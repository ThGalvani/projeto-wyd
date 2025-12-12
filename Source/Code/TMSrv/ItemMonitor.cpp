#include "ItemMonitor.h"
#include "Basedef.h"
#include <ctime>
#include <cstring>
#include <algorithm>

// Declarações externas dos arrays globais do servidor
extern CUser pUser[MAX_USER];
extern CMob pMob[MAX_MOB];
extern CItem pItem[MAX_ITEM];

namespace ItemMonitor {

//==============================================================================
// ItemMonitorSystem Implementation
//==============================================================================

ItemMonitorSystem::ItemMonitorSystem()
	: last_full_scan(0)
	, auto_scan_enabled(true)
{
	// Inicializa contadores para todos itens monitorados
	for (int i = 0; i < NUM_MONITORED_ITEMS; i++)
	{
		ItemCount count;
		count.item_id = MONITORED_ITEMS[i].item_id;
		count.item_name = MONITORED_ITEMS[i].name;
		count.count_inventory = 0;
		count.count_cargo = 0;
		count.count_equipped = 0;
		count.count_ground = 0;
		count.total_count = 0;
		count.previous_total = 0;
		count.delta = 0;
		count.last_scan = 0;
		count.alert_triggered = false;

		item_counts[count.item_id] = count;
	}
}

void ItemMonitorSystem::PerformFullScan()
{
	std::lock_guard<std::mutex> lock(monitor_mutex);

	// Salva contagens anteriores
	for (auto& pair : item_counts)
	{
		pair.second.previous_total = pair.second.total_count;
		pair.second.count_inventory = 0;
		pair.second.count_cargo = 0;
		pair.second.count_equipped = 0;
		pair.second.count_ground = 0;
		pair.second.total_count = 0;
	}

	// Varre todos players online
	for (int i = 0; i < MAX_USER; i++)
	{
		if (pUser[i].Mode != USER_PLAY)
			continue;

		// Varre inventário (Carry)
		for (int slot = 0; slot < MAX_CARRY; slot++)
		{
			int itemId = pMob[i].MOB.Carry[slot].sIndex;
			if (itemId > 0 && item_counts.count(itemId) > 0)
			{
				int amount = BASE_GetItemAmount(&pMob[i].MOB.Carry[slot]);
				if (amount <= 0) amount = 1;

				item_counts[itemId].count_inventory += amount;
				item_counts[itemId].total_count += amount;
			}
		}

		// Varre equipamentos
		for (int slot = 0; slot < MAX_EQUIP; slot++)
		{
			int itemId = pMob[i].MOB.Equip[slot].sIndex;
			if (itemId > 0 && item_counts.count(itemId) > 0)
			{
				item_counts[itemId].count_equipped++;
				item_counts[itemId].total_count++;
			}
		}

		// Varre cargo (baú)
		for (int slot = 0; slot < MAX_CARGO; slot++)
		{
			int itemId = pUser[i].Cargo[slot].sIndex;
			if (itemId > 0 && item_counts.count(itemId) > 0)
			{
				int amount = BASE_GetItemAmount(&pUser[i].Cargo[slot]);
				if (amount <= 0) amount = 1;

				item_counts[itemId].count_cargo += amount;
				item_counts[itemId].total_count += amount;
			}
		}
	}

	// Varre itens no chão
	for (int i = 0; i < MAX_ITEM; i++)
	{
		if (pItem[i].Mode == 0)
			continue;

		int itemId = pItem[i].item.sIndex;
		if (itemId > 0 && item_counts.count(itemId) > 0)
		{
			int amount = BASE_GetItemAmount(&pItem[i].item);
			if (amount <= 0) amount = 1;

			item_counts[itemId].count_ground += amount;
			item_counts[itemId].total_count += amount;
		}
	}

	// Calcula deltas e gera alertas
	time_t now = time(nullptr);
	for (auto& pair : item_counts)
	{
		ItemCount& current = pair.second;
		current.delta = current.total_count - current.previous_total;
		current.last_scan = now;

		// Verifica se deve gerar alerta
		if (current.delta != 0)
		{
			// Busca configuração do item
			const ItemInfo* config = nullptr;
			for (int i = 0; i < NUM_MONITORED_ITEMS; i++)
			{
				if (MONITORED_ITEMS[i].item_id == current.item_id)
				{
					config = &MONITORED_ITEMS[i];
					break;
				}
			}

			if (config && config->alert_on_increase)
			{
				// Gera alerta se aumento >= threshold
				if (current.delta >= config->alert_threshold)
				{
					ItemAlert alert;
					alert.timestamp = now;

					struct tm timeinfo;
					localtime_s(&timeinfo, &alert.timestamp);
					strftime(alert.timestamp_str, sizeof(alert.timestamp_str),
						"%Y-%m-%d %H:%M:%S", &timeinfo);

					alert.item_id = current.item_id;
					alert.item_name = current.item_name;
					alert.old_count = current.previous_total;
					alert.new_count = current.total_count;
					alert.delta = current.delta;

					if (current.delta >= config->alert_threshold * 3)
						alert.alert_type = "SPIKE";
					else if (current.delta > 0)
						alert.alert_type = "INCREASE";
					else
						alert.alert_type = "DECREASE";

					char msg[512];
					snprintf(msg, sizeof(msg),
						"[%s] %s: %d -> %d (delta: %+d) - POSSIBLE DUPE!",
						alert.alert_type.c_str(),
						alert.item_name.c_str(),
						alert.old_count,
						alert.new_count,
						alert.delta);
					alert.message = msg;

					recent_alerts.push_back(alert);
					current.alert_triggered = true;

					// Log no servidor
					char logBuf[1024];
					snprintf(logBuf, sizeof(logBuf),
						"ITEM ALERT: %s", msg);
					// SystemLog poderia ser chamado aqui se disponível
				}
			}
		}
	}

	// Limpa alertas antigos
	CleanOldAlerts();

	last_full_scan = now;
}

void ItemMonitorSystem::CleanOldAlerts()
{
	// Mantém apenas últimos 100 alertas
	if (recent_alerts.size() > 100)
	{
		recent_alerts.erase(
			recent_alerts.begin(),
			recent_alerts.begin() + (recent_alerts.size() - 100)
		);
	}
}

ItemCount ItemMonitorSystem::GetItemCount(int item_id)
{
	std::lock_guard<std::mutex> lock(monitor_mutex);

	if (item_counts.count(item_id) > 0)
		return item_counts[item_id];

	// Retorna vazio se não monitorado
	ItemCount empty;
	empty.item_id = item_id;
	empty.item_name = "Unknown";
	empty.count_inventory = 0;
	empty.count_cargo = 0;
	empty.count_equipped = 0;
	empty.count_ground = 0;
	empty.total_count = 0;
	empty.previous_total = 0;
	empty.delta = 0;
	empty.last_scan = 0;
	empty.alert_triggered = false;

	return empty;
}

std::vector<ItemCount> ItemMonitorSystem::GetAllItemCounts()
{
	std::lock_guard<std::mutex> lock(monitor_mutex);

	std::vector<ItemCount> result;
	for (const auto& pair : item_counts)
	{
		result.push_back(pair.second);
	}

	// Ordena por total_count (maior primeiro)
	std::sort(result.begin(), result.end(),
		[](const ItemCount& a, const ItemCount& b) {
			return a.total_count > b.total_count;
		});

	return result;
}

std::vector<ItemAlert> ItemMonitorSystem::GetRecentAlerts(int limit)
{
	std::lock_guard<std::mutex> lock(monitor_mutex);

	std::vector<ItemAlert> result;

	int start = (int)recent_alerts.size() - limit;
	if (start < 0) start = 0;

	for (size_t i = start; i < recent_alerts.size(); i++)
	{
		result.push_back(recent_alerts[i]);
	}

	// Inverte para mostrar mais recentes primeiro
	std::reverse(result.begin(), result.end());

	return result;
}

void ItemMonitorSystem::ClearAlerts()
{
	std::lock_guard<std::mutex> lock(monitor_mutex);
	recent_alerts.clear();

	// Reseta flags de alerta
	for (auto& pair : item_counts)
	{
		pair.second.alert_triggered = false;
	}
}

//==============================================================================
// Funções Auxiliares
//==============================================================================

int CountItemInServer(int item_id)
{
	int total = 0;

	// Players online
	for (int i = 0; i < MAX_USER; i++)
	{
		if (pUser[i].Mode != USER_PLAY)
			continue;

		// Inventário
		for (int slot = 0; slot < MAX_CARRY; slot++)
		{
			if (pMob[i].MOB.Carry[slot].sIndex == item_id)
			{
				int amount = BASE_GetItemAmount(&pMob[i].MOB.Carry[slot]);
				total += (amount <= 0 ? 1 : amount);
			}
		}

		// Equipado
		for (int slot = 0; slot < MAX_EQUIP; slot++)
		{
			if (pMob[i].MOB.Equip[slot].sIndex == item_id)
				total++;
		}

		// Cargo
		for (int slot = 0; slot < MAX_CARGO; slot++)
		{
			if (pUser[i].Cargo[slot].sIndex == item_id)
			{
				int amount = BASE_GetItemAmount(&pUser[i].Cargo[slot]);
				total += (amount <= 0 ? 1 : amount);
			}
		}
	}

	// Chão
	for (int i = 0; i < MAX_ITEM; i++)
	{
		if (pItem[i].Mode == 0)
			continue;

		if (pItem[i].item.sIndex == item_id)
		{
			int amount = BASE_GetItemAmount(&pItem[i].item);
			total += (amount <= 0 ? 1 : amount);
		}
	}

	return total;
}

void OnItemCreated(int item_id, int conn)
{
	// Placeholder para integração futura
	// Pode ser usado para tracking em tempo real
}

void OnItemDeleted(int item_id, int conn)
{
	// Placeholder para integração futura
}

void OnItemMoved(int item_id, int conn)
{
	// Placeholder para integração futura
}

// Global instance
ItemMonitorSystem g_ItemMonitor;

} // namespace ItemMonitor

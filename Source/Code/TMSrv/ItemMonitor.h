#ifndef _ITEMMONITOR_H_
#define _ITEMMONITOR_H_

#include <mutex>
#include <map>
#include <vector>
#include <string>
#include <ctime>

//==============================================================================
// FASE 3 - Sistema de Monitoramento de Itens Raros
//
// Este sistema rastreia quantidades de itens específicos no servidor para:
// - Detectar possíveis dupes (aumento súbito de quantidade)
// - Monitorar economia do servidor
// - Gerar alertas de anomalias
//==============================================================================

namespace ItemMonitor {

// Estrutura para informações de um item monitorado
struct ItemInfo {
	int item_id;
	const char* name;
	bool alert_on_increase;  // Gera alerta se quantidade aumentar
	int alert_threshold;     // Threshold de aumento para alerta
};

// Lista de itens a serem monitorados
static const ItemInfo MONITORED_ITEMS[] = {
	// Itens raros de crafting
	{ 415, "Laktolerium", true, 5 },
	{ 413, "Elenium", true, 10 },
	{ 412, "Bizotium", true, 10 },
	{ 419, "Lingote de Ouro", true, 20 },
	{ 420, "Pedra Lunar", true, 20 },

	// Armas raras
	{ 1774, "Espada Lendária", true, 1 },
	{ 777, "Arco Celestial", true, 1 },
	{ 787, "Cajado Supremo", true, 1 },

	// Montarias
	{ 3989, "Gullfaxi (30d)", true, 3 },
	{ 3988, "Klazedale (30d)", true, 3 },
	{ 3987, "Thoroughbred (30d)", true, 3 },
	{ 3986, "Shire (30d)", true, 3 },

	// Itens de evento
	{ 4011, "Ficha de Evento", true, 50 },
	{ 4097, "Token Premium", true, 20 },

	// Gemas
	{ 3200, "Rubi", false, 0 },
	{ 3201, "Safira", false, 0 },
	{ 3202, "Esmeralda", false, 0 },
	{ 3203, "Diamante", false, 0 },
};

static const int NUM_MONITORED_ITEMS = sizeof(MONITORED_ITEMS) / sizeof(ItemInfo);

// Estrutura para contagem de um item
struct ItemCount {
	int item_id;
	std::string item_name;
	int count_inventory;      // Em inventários de players
	int count_cargo;          // Em cargo (baú)
	int count_equipped;       // Equipado
	int count_ground;         // No chão
	int total_count;          // Total no servidor
	int previous_total;       // Total da varredura anterior
	int delta;                // Diferença desde última varredura
	time_t last_scan;         // Timestamp da última varredura
	bool alert_triggered;     // Se alerta foi disparado
};

// Estrutura de alerta
struct ItemAlert {
	time_t timestamp;
	char timestamp_str[64];
	int item_id;
	std::string item_name;
	int old_count;
	int new_count;
	int delta;
	std::string alert_type;   // "INCREASE", "DECREASE", "SPIKE"
	std::string message;
};

// Classe principal de monitoramento
class ItemMonitorSystem {
private:
	std::mutex monitor_mutex;
	std::map<int, ItemCount> item_counts;
	std::vector<ItemAlert> recent_alerts;
	time_t last_full_scan;
	bool auto_scan_enabled;

	// Limpa alertas antigos (mantém apenas últimos 100)
	void CleanOldAlerts();

	// Gera alerta se necessário
	void CheckAndGenerateAlert(const ItemCount& current, const ItemCount& previous);

public:
	ItemMonitorSystem();

	// Executa varredura completa do servidor
	void PerformFullScan();

	// Obtém contagem de um item específico
	ItemCount GetItemCount(int item_id);

	// Obtém contagens de todos itens monitorados
	std::vector<ItemCount> GetAllItemCounts();

	// Obtém alertas recentes
	std::vector<ItemAlert> GetRecentAlerts(int limit = 10);

	// Limpa todos alertas
	void ClearAlerts();

	// Enable/disable auto-scan
	void SetAutoScan(bool enabled) { auto_scan_enabled = enabled; }
	bool IsAutoScanEnabled() const { return auto_scan_enabled; }

	// Get last scan time
	time_t GetLastScanTime() const { return last_full_scan; }
};

// Global instance
extern ItemMonitorSystem g_ItemMonitor;

// Função auxiliar para contar item específico no servidor
int CountItemInServer(int item_id);

// Funções para integração com operações do servidor
void OnItemCreated(int item_id, int conn);
void OnItemDeleted(int item_id, int conn);
void OnItemMoved(int item_id, int conn);

} // namespace ItemMonitor

#endif

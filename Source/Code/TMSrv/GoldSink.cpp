#include "GoldSink.h"
#include "EconomyTracker.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cmath>

namespace Economy {

// Inst�ncia global
GoldSinkManager g_GoldSink;

// ============================================================================
// CONSTRUTOR / DESTRUTOR
// ============================================================================

GoldSinkManager::GoldSinkManager() {
    recent_sinks.reserve(MAX_RECENT_SINKS);
}

GoldSinkManager::~GoldSinkManager() {
    // Cleanup
}

// ============================================================================
// FUN��ES AUXILIARES
// ============================================================================

void GoldSinkManager::RecordSink(SinkType type, int player_id, int amount, const char* description) {
    // Registra sink
    SinkRecord record;
    record.type = type;
    record.player_id = player_id;
    record.amount = amount;
    record.timestamp = time(nullptr);
    strncpy_s(record.description, description, sizeof(record.description) - 1);

    {
        std::lock_guard<std::mutex> lock(sink_mutex);
        recent_sinks.push_back(record);

        // Limita tamanho
        if (recent_sinks.size() > MAX_RECENT_SINKS) {
            recent_sinks.erase(recent_sinks.begin());
        }

        // Atualiza estat�sticas
        stats_by_type[type].total_removed.fetch_add(amount);
        stats_by_type[type].transaction_count.fetch_add(1);
    }

    // Registra no EconomyTracker
    g_EconomyTracker.RecordGoldSink(player_id, amount, description, "", "");
}

// ============================================================================
// TAXAS AUTOM�TICAS
// ============================================================================

int GoldSinkManager::ApplyTradeTax(int player_id, int trade_amount) {
    int tax = (int)(trade_amount * TRADE_TAX_RATE);

    if (tax > 0) {
        char desc[64];
        sprintf_s(desc, "Trade Tax: %d gold (5%%)", tax);
        RecordSink(SinkType::TRADE_TAX, player_id, tax, desc);
    }

    return tax;
}

int GoldSinkManager::ApplyAuctionTax(int player_id, int auction_amount) {
    int tax = (int)(auction_amount * AUCTION_TAX_RATE);

    if (tax > 0) {
        char desc[64];
        sprintf_s(desc, "Auction Tax: %d gold (10%%)", tax);
        RecordSink(SinkType::AUCTION_TAX, player_id, tax, desc);
    }

    return tax;
}

// ============================================================================
// SERVI�OS PAGOS
// ============================================================================

bool GoldSinkManager::ChargeTeleport(int player_id, int& player_gold) {
    if (player_gold < TELEPORT_COST) {
        return false;  // Sem gold suficiente
    }

    player_gold -= TELEPORT_COST;
    RecordSink(SinkType::TELEPORT, player_id, TELEPORT_COST, "Teleport Service");

    return true;
}

int GoldSinkManager::CalculateRepairCost(int durability_lost) {
    return durability_lost * REPAIR_COST_PER_DURABILITY;
}

bool GoldSinkManager::ChargeRepair(int player_id, int durability_lost, int& player_gold) {
    int cost = CalculateRepairCost(durability_lost);

    if (player_gold < cost) {
        return false;  // Sem gold suficiente
    }

    player_gold -= cost;

    char desc[64];
    sprintf_s(desc, "Repair: %d durability (%d gold)", durability_lost, cost);
    RecordSink(SinkType::REPAIR, player_id, cost, desc);

    return true;
}

bool GoldSinkManager::ChargeStorageFee(int player_id, int& player_gold) {
    if (player_gold < STORAGE_FEE) {
        return false;  // Sem gold suficiente
    }

    player_gold -= STORAGE_FEE;
    RecordSink(SinkType::STORAGE, player_id, STORAGE_FEE, "Storage Fee");

    return true;
}

bool GoldSinkManager::ChargeSkillReset(int player_id, int& player_gold) {
    if (player_gold < SKILL_RESET_COST) {
        return false;  // Sem gold suficiente
    }

    player_gold -= SKILL_RESET_COST;
    RecordSink(SinkType::SKILL_RESET, player_id, SKILL_RESET_COST, "Skill Reset");

    return true;
}

// ============================================================================
// PREMIUM FEATURES
// ============================================================================

bool GoldSinkManager::ChargeNameChange(int player_id, int& player_gold) {
    if (player_gold < PREMIUM_NAME_CHANGE) {
        return false;
    }

    player_gold -= PREMIUM_NAME_CHANGE;
    RecordSink(SinkType::NAME_CHANGE, player_id, PREMIUM_NAME_CHANGE, "Name Change");

    return true;
}

bool GoldSinkManager::ChargeGuildCreate(int player_id, int& player_gold) {
    if (player_gold < PREMIUM_GUILD_CREATE) {
        return false;
    }

    player_gold -= PREMIUM_GUILD_CREATE;
    RecordSink(SinkType::GUILD_CREATE, player_id, PREMIUM_GUILD_CREATE, "Guild Creation");

    return true;
}

bool GoldSinkManager::ChargeAppearanceChange(int player_id, int& player_gold) {
    if (player_gold < PREMIUM_APPEARANCE_CHANGE) {
        return false;
    }

    player_gold -= PREMIUM_APPEARANCE_CHANGE;
    RecordSink(SinkType::APPEARANCE_CHANGE, player_id, PREMIUM_APPEARANCE_CHANGE, "Appearance Change");

    return true;
}

// ============================================================================
// UPGRADE SYSTEM
// ============================================================================

int GoldSinkManager::CalculateUpgradeCost(int current_level) {
    // Custo aumenta exponencialmente: base * (2 ^ level)
    // Level 1: 1000
    // Level 2: 2000
    // Level 3: 4000
    // Level 4: 8000
    // Level 10: 512,000
    return (int)(UPGRADE_BASE_COST * pow(2.0, current_level));
}

bool GoldSinkManager::ChargeUpgrade(int player_id, int current_level, int& player_gold) {
    int cost = CalculateUpgradeCost(current_level);

    if (player_gold < cost) {
        return false;
    }

    player_gold -= cost;

    char desc[64];
    sprintf_s(desc, "Upgrade to +%d (%d gold)", current_level + 1, cost);
    RecordSink(SinkType::UPGRADE, player_id, cost, desc);

    return true;
}

// ============================================================================
// COMPRAS DE NPC
// ============================================================================

bool GoldSinkManager::ChargeNPCPurchase(int player_id, int item_cost, int& player_gold) {
    if (player_gold < item_cost) {
        return false;
    }

    player_gold -= item_cost;

    char desc[64];
    sprintf_s(desc, "NPC Purchase: %d gold", item_cost);
    RecordSink(SinkType::NPC_PURCHASE, player_id, item_cost, desc);

    return true;
}

// ============================================================================
// ESTAT�STICAS
// ============================================================================

long long GoldSinkManager::GetTotalGoldRemoved() const {
    long long total = 0;

    std::lock_guard<std::mutex> lock(sink_mutex);
    for (const auto& pair : stats_by_type) {
        total += pair.second.total_removed.load();
    }

    return total;
}

long long GoldSinkManager::GetGoldRemovedByType(SinkType type) const {
    std::lock_guard<std::mutex> lock(sink_mutex);

    auto it = stats_by_type.find(type);
    if (it != stats_by_type.end()) {
        return it->second.total_removed.load();
    }

    return 0;
}

int GoldSinkManager::GetTransactionCountByType(SinkType type) const {
    std::lock_guard<std::mutex> lock(sink_mutex);

    auto it = stats_by_type.find(type);
    if (it != stats_by_type.end()) {
        return it->second.transaction_count.load();
    }

    return 0;
}

std::vector<SinkRecord> GoldSinkManager::GetRecentSinks(int count) const {
    std::lock_guard<std::mutex> lock(sink_mutex);

    std::vector<SinkRecord> result;
    int start = std::max(0, (int)recent_sinks.size() - count);

    for (size_t i = start; i < recent_sinks.size(); i++) {
        result.push_back(recent_sinks[i]);
    }

    return result;
}

std::string GoldSinkManager::GenerateSinkReport() const {
    std::ostringstream report;

    report << "=== RELAT�RIO DE GOLD SINKS ===\n\n";

    long long total = GetTotalGoldRemoved();
    report << "Total de Gold Removido: " << total << "\n\n";

    report << "POR TIPO:\n";
    report << std::left << std::setw(25) << "Tipo"
           << std::setw(20) << "Gold Removido"
           << std::setw(15) << "Transa��es" << "\n";
    report << std::string(60, '-') << "\n";

    // Tipos
    const char* type_names[] = {
        "Trade Tax",
        "Auction Tax",
        "Teleport",
        "Repair",
        "Storage",
        "Upgrade",
        "Skill Reset",
        "Name Change",
        "Guild Create",
        "Appearance Change",
        "NPC Purchase",
        "Other"
    };

    std::lock_guard<std::mutex> lock(sink_mutex);

    for (int i = 0; i < 12; i++) {
        SinkType type = (SinkType)i;
        auto it = stats_by_type.find(type);

        if (it != stats_by_type.end()) {
            long long removed = it->second.total_removed.load();
            int count = it->second.transaction_count.load();

            report << std::left << std::setw(25) << type_names[i]
                   << std::setw(20) << removed
                   << std::setw(15) << count << "\n";
        }
    }

    report << "\n";

    // Sinks recentes
    int recent_count = std::min(5, (int)recent_sinks.size());
    report << "�LTIMOS SINKS (" << recent_count << "):\n";

    for (int i = recent_sinks.size() - recent_count; i < recent_sinks.size(); i++) {
        const SinkRecord& record = recent_sinks[i];
        report << "  - Player " << record.player_id << ": "
               << record.description << "\n";
    }

    return report.str();
}

float GoldSinkManager::GetRemovalRate(long long gold_generated) const {
    long long removed = GetTotalGoldRemoved();

    if (gold_generated == 0) return 0.0f;

    return (float)removed / (float)gold_generated;
}

} // namespace Economy

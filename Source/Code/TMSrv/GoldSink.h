#ifndef GOLD_SINK_H
#define GOLD_SINK_H

#include <mutex>
#include <unordered_map>
#include <atomic>
#include <string>

// FASE 5: Gold Sink Systems (Anti-Infla��o)
// M�ltiplos sistemas para remover gold da economia

namespace Economy {

// ============================================================================
// CONSTANTES
// ============================================================================

// Taxas base (%)
const float TRADE_TAX_RATE = 0.05f;          // 5% tax em trades
const float AUCTION_TAX_RATE = 0.10f;        // 10% tax em auction house
const float UPGRADE_BASE_COST = 1000;        // Custo base de upgrade

// Servi�os
const int TELEPORT_COST = 500;               // Custo de teleport
const int REPAIR_COST_PER_DURABILITY = 10;   // Custo de repair
const int STORAGE_FEE = 100;                 // Taxa de storage di�ria
const int SKILL_RESET_COST = 10000;          // Reset de skills

// Premium Features
const int PREMIUM_NAME_CHANGE = 50000;       // Mudar nome
const int PREMIUM_GUILD_CREATE = 100000;     // Criar guild
const int PREMIUM_APPEARANCE_CHANGE = 20000; // Mudar apar�ncia

// ============================================================================
// ESTRUTURAS
// ============================================================================

// Tipos de gold sink
enum class SinkType {
    TRADE_TAX,           // Taxa de trade
    AUCTION_TAX,         // Taxa de auction
    TELEPORT,            // Servi�o de teleport
    REPAIR,              // Repair de items
    STORAGE,             // Taxa de storage
    UPGRADE,             // Upgrade de items
    SKILL_RESET,         // Reset de skills
    NAME_CHANGE,         // Mudan�a de nome
    GUILD_CREATE,        // Cria��o de guild
    APPEARANCE_CHANGE,   // Mudan�a de apar�ncia
    NPC_PURCHASE,        // Compra de NPC
    OTHER                // Outros
};

// Registro de gold sink
struct SinkRecord {
    SinkType type;
    int player_id;
    int amount;
    time_t timestamp;
    char description[64];

    SinkRecord() : type(SinkType::OTHER), player_id(0), amount(0), timestamp(0) {
        memset(description, 0, sizeof(description));
    }
};

// Estat�sticas por tipo
struct SinkStats {
    std::atomic<long long> total_removed;
    std::atomic<int> transaction_count;

    SinkStats() : total_removed(0), transaction_count(0) {}
};

// ============================================================================
// CLASSE PRINCIPAL
// ============================================================================

class GoldSinkManager {
private:
    std::mutex sink_mutex;
    std::unordered_map<SinkType, SinkStats> stats_by_type;
    std::vector<SinkRecord> recent_sinks;

    const int MAX_RECENT_SINKS = 500;

    // Fun��es auxiliares
    void RecordSink(SinkType type, int player_id, int amount, const char* description);

public:
    GoldSinkManager();
    ~GoldSinkManager();

    // ========================================================================
    // TAXAS AUTOM�TICAS
    // ========================================================================

    // Calcula e cobra taxa de trade (retorna taxa cobrada)
    int ApplyTradeTax(int player_id, int trade_amount);

    // Calcula e cobra taxa de auction (retorna taxa cobrada)
    int ApplyAuctionTax(int player_id, int auction_amount);

    // ========================================================================
    // SERVI�OS PAGOS
    // ========================================================================

    // Teleport
    bool ChargeTeleport(int player_id, int& player_gold);

    // Repair (baseado em durabilidade perdida)
    int CalculateRepairCost(int durability_lost);
    bool ChargeRepair(int player_id, int durability_lost, int& player_gold);

    // Storage
    bool ChargeStorageFee(int player_id, int& player_gold);

    // Skill Reset
    bool ChargeSkillReset(int player_id, int& player_gold);

    // ========================================================================
    // PREMIUM FEATURES
    // ========================================================================

    // Mudar nome
    bool ChargeNameChange(int player_id, int& player_gold);

    // Criar guild
    bool ChargeGuildCreate(int player_id, int& player_gold);

    // Mudar apar�ncia
    bool ChargeAppearanceChange(int player_id, int& player_gold);

    // ========================================================================
    // UPGRADE SYSTEM
    // ========================================================================

    // Calcula custo de upgrade (aumenta exponencialmente)
    int CalculateUpgradeCost(int current_level);

    // Cobra upgrade
    bool ChargeUpgrade(int player_id, int current_level, int& player_gold);

    // ========================================================================
    // COMPRAS DE NPC
    // ========================================================================

    // Compra gen�rica de NPC
    bool ChargeNPCPurchase(int player_id, int item_cost, int& player_gold);

    // ========================================================================
    // ESTAT�STICAS
    // ========================================================================

    // Total de gold removido
    long long GetTotalGoldRemoved() const;

    // Gold removido por tipo
    long long GetGoldRemovedByType(SinkType type) const;

    // Transa��es por tipo
    int GetTransactionCountByType(SinkType type) const;

    // Sinks recentes
    std::vector<SinkRecord> GetRecentSinks(int count = 50) const;

    // Relat�rio
    std::string GenerateSinkReport() const;

    // Taxa de remo��o (gold removido vs gerado)
    float GetRemovalRate(long long gold_generated) const;
};

// Inst�ncia global
extern GoldSinkManager g_GoldSink;

} // namespace Economy

#endif // GOLD_SINK_H

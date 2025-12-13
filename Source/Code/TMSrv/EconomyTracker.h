#ifndef ECONOMY_TRACKER_H
#define ECONOMY_TRACKER_H

#include <mutex>
#include <unordered_map>
#include <string>
#include <vector>
#include <atomic>
#include <ctime>

// FASE 5: Sistema de Rastreamento de Economia
// Monitora fluxo de gold e itens valiosos para detectar anomalias e infla��o

namespace Economy {

// ============================================================================
// CONSTANTES
// ============================================================================

const int MAX_TRACKED_ITEMS = 50;           // N�mero de itens rastreados
const int ANOMALY_THRESHOLD_GOLD = 100000;  // 100k gold em uma transa��o
const int ANOMALY_THRESHOLD_MULTIPLIER = 5; // 5x a m�dia � anomalia
const int STATS_WINDOW_HOURS = 24;          // Janela de estat�sticas: 24h

// ============================================================================
// ESTRUTURAS
// ============================================================================

// Tipos de transa��o
enum class TransactionType {
    GOLD_DROP,          // Mob dropou gold
    GOLD_PICKUP,        // Player pegou gold
    GOLD_TRADE,         // Trade entre players
    GOLD_NPC_SELL,      // Venda para NPC
    GOLD_NPC_BUY,       // Compra de NPC
    GOLD_SINK,          // Gold destru�do (tax, servi�os)
    ITEM_DROP,          // Mob dropou item
    ITEM_PICKUP,        // Player pegou item
    ITEM_TRADE,         // Trade de item
    ITEM_DESTROY,       // Item destru�do
    ITEM_CRAFT          // Item criado via craft
};

// Registro de transa��o
struct Transaction {
    TransactionType type;
    int player_id;           // ID do player envolvido
    int amount;              // Quantidade (gold ou item ID)
    int secondary_player;    // Para trades
    time_t timestamp;
    char account_name[32];
    char ip[16];

    Transaction() : type(TransactionType::GOLD_DROP), player_id(0), amount(0),
                    secondary_player(0), timestamp(0) {
        memset(account_name, 0, sizeof(account_name));
        memset(ip, 0, sizeof(ip));
    }
};

// Estat�sticas de gold
struct GoldStats {
    std::atomic<long long> total_generated;    // Gold criado (drops)
    std::atomic<long long> total_destroyed;    // Gold destru�do (sinks)
    std::atomic<long long> total_circulating;  // Gold em circula��o
    std::atomic<int> avg_transaction;          // M�dia de transa��es
    std::atomic<int> peak_transaction;         // Maior transa��o
    std::atomic<int> transaction_count;        // Total de transa��es

    GoldStats() : total_generated(0), total_destroyed(0), total_circulating(0),
                  avg_transaction(0), peak_transaction(0), transaction_count(0) {}
};

// Estat�sticas de item
struct ItemStats {
    int item_id;
    std::atomic<int> total_dropped;      // Quantidade dropada
    std::atomic<int> total_destroyed;    // Quantidade destru�da
    std::atomic<int> total_circulating;  // Quantidade em circula��o
    std::atomic<int> avg_price;          // Pre�o m�dio em trades
    std::atomic<int> peak_price;         // Pre�o m�ximo

    ItemStats() : item_id(0), total_dropped(0), total_destroyed(0),
                  total_circulating(0), avg_price(0), peak_price(0) {}
};

// Anomalia detectada
struct EconomyAnomaly {
    TransactionType type;
    int player_id;
    int amount;
    int expected_amount;
    time_t timestamp;
    char reason[128];

    EconomyAnomaly() : type(TransactionType::GOLD_DROP), player_id(0),
                       amount(0), expected_amount(0), timestamp(0) {
        memset(reason, 0, sizeof(reason));
    }
};

// ============================================================================
// CLASSE PRINCIPAL
// ============================================================================

class Tracker {
private:
    std::mutex transaction_mutex;
    std::mutex stats_mutex;
    std::mutex anomaly_mutex;

    GoldStats gold_stats;
    std::unordered_map<int, ItemStats> item_stats;  // item_id -> stats

    std::vector<Transaction> recent_transactions;   // �ltimas 1000 transa��es
    std::vector<EconomyAnomaly> anomalies;          // �ltimas 100 anomalias

    const int MAX_RECENT_TRANSACTIONS = 1000;
    const int MAX_ANOMALIES = 100;

    // Fun��es auxiliares
    bool IsAnomaly(TransactionType type, int amount);
    void UpdateAverages();
    void CleanOldData();

public:
    Tracker();
    ~Tracker();

    // Registro de transa��es de gold
    void RecordGoldDrop(int player_id, int amount, const char* account, const char* ip);
    void RecordGoldPickup(int player_id, int amount, const char* account, const char* ip);
    void RecordGoldTrade(int player_id, int target_id, int amount, const char* account, const char* ip);
    void RecordGoldNPCSell(int player_id, int amount, const char* account, const char* ip);
    void RecordGoldNPCBuy(int player_id, int amount, const char* account, const char* ip);
    void RecordGoldSink(int player_id, int amount, const char* reason, const char* account, const char* ip);

    // Registro de transa��es de itens
    void RecordItemDrop(int player_id, int item_id, const char* account, const char* ip);
    void RecordItemPickup(int player_id, int item_id, const char* account, const char* ip);
    void RecordItemTrade(int player_id, int target_id, int item_id, int price, const char* account, const char* ip);
    void RecordItemDestroy(int player_id, int item_id, const char* account, const char* ip);
    void RecordItemCraft(int player_id, int item_id, const char* account, const char* ip);

    // Consulta de estat�sticas
    GoldStats GetGoldStats() const;
    ItemStats GetItemStats(int item_id) const;
    std::vector<EconomyAnomaly> GetRecentAnomalies(int count = 10) const;
    std::vector<Transaction> GetRecentTransactions(int count = 50) const;

    // An�lise
    float GetInflationRate() const;  // Taxa de infla��o (generated/destroyed)
    int GetGoldVelocity() const;     // Velocidade de circula��o

    // Snapshot para m�tricas
    std::string GenerateEconomyReport() const;
};

// Inst�ncia global
extern Tracker g_EconomyTracker;

} // namespace Economy

#endif // ECONOMY_TRACKER_H

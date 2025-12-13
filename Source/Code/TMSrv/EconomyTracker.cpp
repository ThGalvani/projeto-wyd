#include "EconomyTracker.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace Economy {

// Inst�ncia global
Tracker g_EconomyTracker;

// ============================================================================
// CONSTRUTOR / DESTRUTOR
// ============================================================================

Tracker::Tracker() {
    recent_transactions.reserve(MAX_RECENT_TRANSACTIONS);
    anomalies.reserve(MAX_ANOMALIES);
}

Tracker::~Tracker() {
    // Cleanup autom�tico
}

// ============================================================================
// FUN��ES AUXILIARES
// ============================================================================

bool Tracker::IsAnomaly(TransactionType type, int amount) {
    // Verifica se a transa��o � uma anomalia

    switch (type) {
        case TransactionType::GOLD_DROP:
        case TransactionType::GOLD_PICKUP:
        case TransactionType::GOLD_TRADE: {
            // Gold > 100k ou > 5x a m�dia
            if (amount > ANOMALY_THRESHOLD_GOLD) return true;

            int avg = gold_stats.avg_transaction.load();
            if (avg > 0 && amount > avg * ANOMALY_THRESHOLD_MULTIPLIER) {
                return true;
            }
            break;
        }

        default:
            break;
    }

    return false;
}

void Tracker::UpdateAverages() {
    std::lock_guard<std::mutex> lock(transaction_mutex);

    // Calcula m�dia de transa��es de gold
    if (!recent_transactions.empty()) {
        long long total = 0;
        int count = 0;

        for (const auto& t : recent_transactions) {
            if (t.type == TransactionType::GOLD_TRADE ||
                t.type == TransactionType::GOLD_NPC_BUY ||
                t.type == TransactionType::GOLD_NPC_SELL) {
                total += t.amount;
                count++;
            }
        }

        if (count > 0) {
            gold_stats.avg_transaction.store((int)(total / count));
        }
    }
}

void Tracker::CleanOldData() {
    std::lock_guard<std::mutex> lock(transaction_mutex);

    // Remove transa��es antigas (mant�m �ltimas MAX_RECENT_TRANSACTIONS)
    if (recent_transactions.size() > MAX_RECENT_TRANSACTIONS) {
        recent_transactions.erase(
            recent_transactions.begin(),
            recent_transactions.begin() + (recent_transactions.size() - MAX_RECENT_TRANSACTIONS)
        );
    }

    // Remove anomalias antigas (mant�m �ltimas MAX_ANOMALIES)
    {
        std::lock_guard<std::mutex> anom_lock(anomaly_mutex);
        if (anomalies.size() > MAX_ANOMALIES) {
            anomalies.erase(
                anomalies.begin(),
                anomalies.begin() + (anomalies.size() - MAX_ANOMALIES)
            );
        }
    }
}

// ============================================================================
// REGISTRO DE TRANSA��ES DE GOLD
// ============================================================================

void Tracker::RecordGoldDrop(int player_id, int amount, const char* account, const char* ip) {
    // Registra gold dropado por mob
    gold_stats.total_generated.fetch_add(amount);
    gold_stats.total_circulating.fetch_add(amount);

    Transaction t;
    t.type = TransactionType::GOLD_DROP;
    t.player_id = player_id;
    t.amount = amount;
    t.timestamp = time(nullptr);
    strncpy_s(t.account_name, account, sizeof(t.account_name) - 1);
    strncpy_s(t.ip, ip, sizeof(t.ip) - 1);

    {
        std::lock_guard<std::mutex> lock(transaction_mutex);
        recent_transactions.push_back(t);
    }

    CleanOldData();
}

void Tracker::RecordGoldPickup(int player_id, int amount, const char* account, const char* ip) {
    // Registra gold coletado
    Transaction t;
    t.type = TransactionType::GOLD_PICKUP;
    t.player_id = player_id;
    t.amount = amount;
    t.timestamp = time(nullptr);
    strncpy_s(t.account_name, account, sizeof(t.account_name) - 1);
    strncpy_s(t.ip, ip, sizeof(t.ip) - 1);

    {
        std::lock_guard<std::mutex> lock(transaction_mutex);
        recent_transactions.push_back(t);
    }

    // Detecta anomalia
    if (IsAnomaly(TransactionType::GOLD_PICKUP, amount)) {
        EconomyAnomaly anomaly;
        anomaly.type = TransactionType::GOLD_PICKUP;
        anomaly.player_id = player_id;
        anomaly.amount = amount;
        anomaly.expected_amount = gold_stats.avg_transaction.load();
        anomaly.timestamp = time(nullptr);
        sprintf_s(anomaly.reason, "Pickup de %d gold (m�dia: %d)", amount, anomaly.expected_amount);

        std::lock_guard<std::mutex> lock(anomaly_mutex);
        anomalies.push_back(anomaly);
    }
}

void Tracker::RecordGoldTrade(int player_id, int target_id, int amount, const char* account, const char* ip) {
    // Registra trade de gold entre players
    Transaction t;
    t.type = TransactionType::GOLD_TRADE;
    t.player_id = player_id;
    t.secondary_player = target_id;
    t.amount = amount;
    t.timestamp = time(nullptr);
    strncpy_s(t.account_name, account, sizeof(t.account_name) - 1);
    strncpy_s(t.ip, ip, sizeof(t.ip) - 1);

    {
        std::lock_guard<std::mutex> lock(transaction_mutex);
        recent_transactions.push_back(t);
    }

    gold_stats.transaction_count.fetch_add(1);

    // Atualiza pico
    int current_peak = gold_stats.peak_transaction.load();
    if (amount > current_peak) {
        gold_stats.peak_transaction.store(amount);
    }

    UpdateAverages();

    // Detecta anomalia
    if (IsAnomaly(TransactionType::GOLD_TRADE, amount)) {
        EconomyAnomaly anomaly;
        anomaly.type = TransactionType::GOLD_TRADE;
        anomaly.player_id = player_id;
        anomaly.amount = amount;
        anomaly.expected_amount = gold_stats.avg_transaction.load();
        anomaly.timestamp = time(nullptr);
        sprintf_s(anomaly.reason, "Trade de %d gold (m�dia: %d)", amount, anomaly.expected_amount);

        std::lock_guard<std::mutex> lock(anomaly_mutex);
        anomalies.push_back(anomaly);
    }
}

void Tracker::RecordGoldNPCSell(int player_id, int amount, const char* account, const char* ip) {
    // Player vendeu item para NPC e recebeu gold
    gold_stats.total_generated.fetch_add(amount);
    gold_stats.total_circulating.fetch_add(amount);

    Transaction t;
    t.type = TransactionType::GOLD_NPC_SELL;
    t.player_id = player_id;
    t.amount = amount;
    t.timestamp = time(nullptr);
    strncpy_s(t.account_name, account, sizeof(t.account_name) - 1);
    strncpy_s(t.ip, ip, sizeof(t.ip) - 1);

    {
        std::lock_guard<std::mutex> lock(transaction_mutex);
        recent_transactions.push_back(t);
    }
}

void Tracker::RecordGoldNPCBuy(int player_id, int amount, const char* account, const char* ip) {
    // Player comprou item de NPC e gastou gold
    gold_stats.total_destroyed.fetch_add(amount);
    gold_stats.total_circulating.fetch_sub(amount);

    Transaction t;
    t.type = TransactionType::GOLD_NPC_BUY;
    t.player_id = player_id;
    t.amount = amount;
    t.timestamp = time(nullptr);
    strncpy_s(t.account_name, account, sizeof(t.account_name) - 1);
    strncpy_s(t.ip, ip, sizeof(t.ip) - 1);

    {
        std::lock_guard<std::mutex> lock(transaction_mutex);
        recent_transactions.push_back(t);
    }
}

void Tracker::RecordGoldSink(int player_id, int amount, const char* reason, const char* account, const char* ip) {
    // Gold destru�do (tax, servi�os, etc)
    gold_stats.total_destroyed.fetch_add(amount);
    gold_stats.total_circulating.fetch_sub(amount);

    Transaction t;
    t.type = TransactionType::GOLD_SINK;
    t.player_id = player_id;
    t.amount = amount;
    t.timestamp = time(nullptr);
    strncpy_s(t.account_name, account, sizeof(t.account_name) - 1);
    strncpy_s(t.ip, ip, sizeof(t.ip) - 1);

    {
        std::lock_guard<std::mutex> lock(transaction_mutex);
        recent_transactions.push_back(t);
    }
}

// ============================================================================
// REGISTRO DE TRANSA��ES DE ITENS
// ============================================================================

void Tracker::RecordItemDrop(int player_id, int item_id, const char* account, const char* ip) {
    // Mob dropou item
    {
        std::lock_guard<std::mutex> lock(stats_mutex);
        item_stats[item_id].item_id = item_id;
        item_stats[item_id].total_dropped.fetch_add(1);
        item_stats[item_id].total_circulating.fetch_add(1);
    }

    Transaction t;
    t.type = TransactionType::ITEM_DROP;
    t.player_id = player_id;
    t.amount = item_id;
    t.timestamp = time(nullptr);
    strncpy_s(t.account_name, account, sizeof(t.account_name) - 1);
    strncpy_s(t.ip, ip, sizeof(t.ip) - 1);

    {
        std::lock_guard<std::mutex> lock(transaction_mutex);
        recent_transactions.push_back(t);
    }
}

void Tracker::RecordItemPickup(int player_id, int item_id, const char* account, const char* ip) {
    // Player pegou item
    Transaction t;
    t.type = TransactionType::ITEM_PICKUP;
    t.player_id = player_id;
    t.amount = item_id;
    t.timestamp = time(nullptr);
    strncpy_s(t.account_name, account, sizeof(t.account_name) - 1);
    strncpy_s(t.ip, ip, sizeof(t.ip) - 1);

    {
        std::lock_guard<std::mutex> lock(transaction_mutex);
        recent_transactions.push_back(t);
    }
}

void Tracker::RecordItemTrade(int player_id, int target_id, int item_id, int price, const char* account, const char* ip) {
    // Trade de item entre players
    Transaction t;
    t.type = TransactionType::ITEM_TRADE;
    t.player_id = player_id;
    t.secondary_player = target_id;
    t.amount = item_id;
    t.timestamp = time(nullptr);
    strncpy_s(t.account_name, account, sizeof(t.account_name) - 1);
    strncpy_s(t.ip, ip, sizeof(t.ip) - 1);

    {
        std::lock_guard<std::mutex> lock(transaction_mutex);
        recent_transactions.push_back(t);
    }

    // Atualiza pre�o m�dio
    {
        std::lock_guard<std::mutex> lock(stats_mutex);
        auto& stats = item_stats[item_id];
        stats.item_id = item_id;

        int current_avg = stats.avg_price.load();
        int new_avg = (current_avg + price) / 2;
        stats.avg_price.store(new_avg);

        int current_peak = stats.peak_price.load();
        if (price > current_peak) {
            stats.peak_price.store(price);
        }
    }
}

void Tracker::RecordItemDestroy(int player_id, int item_id, const char* account, const char* ip) {
    // Item destru�do
    {
        std::lock_guard<std::mutex> lock(stats_mutex);
        item_stats[item_id].item_id = item_id;
        item_stats[item_id].total_destroyed.fetch_add(1);
        item_stats[item_id].total_circulating.fetch_sub(1);
    }

    Transaction t;
    t.type = TransactionType::ITEM_DESTROY;
    t.player_id = player_id;
    t.amount = item_id;
    t.timestamp = time(nullptr);
    strncpy_s(t.account_name, account, sizeof(t.account_name) - 1);
    strncpy_s(t.ip, ip, sizeof(t.ip) - 1);

    {
        std::lock_guard<std::mutex> lock(transaction_mutex);
        recent_transactions.push_back(t);
    }
}

void Tracker::RecordItemCraft(int player_id, int item_id, const char* account, const char* ip) {
    // Item criado via craft
    {
        std::lock_guard<std::mutex> lock(stats_mutex);
        item_stats[item_id].item_id = item_id;
        item_stats[item_id].total_circulating.fetch_add(1);
    }

    Transaction t;
    t.type = TransactionType::ITEM_CRAFT;
    t.player_id = player_id;
    t.amount = item_id;
    t.timestamp = time(nullptr);
    strncpy_s(t.account_name, account, sizeof(t.account_name) - 1);
    strncpy_s(t.ip, ip, sizeof(t.ip) - 1);

    {
        std::lock_guard<std::mutex> lock(transaction_mutex);
        recent_transactions.push_back(t);
    }
}

// ============================================================================
// CONSULTA DE ESTAT�STICAS
// ============================================================================

GoldStats Tracker::GetGoldStats() const {
    return gold_stats;
}

ItemStats Tracker::GetItemStats(int item_id) const {
    std::lock_guard<std::mutex> lock(stats_mutex);

    auto it = item_stats.find(item_id);
    if (it != item_stats.end()) {
        return it->second;
    }

    return ItemStats();
}

std::vector<EconomyAnomaly> Tracker::GetRecentAnomalies(int count) const {
    std::lock_guard<std::mutex> lock(anomaly_mutex);

    std::vector<EconomyAnomaly> result;
    int start = std::max(0, (int)anomalies.size() - count);

    for (size_t i = start; i < anomalies.size(); i++) {
        result.push_back(anomalies[i]);
    }

    return result;
}

std::vector<Transaction> Tracker::GetRecentTransactions(int count) const {
    std::lock_guard<std::mutex> lock(transaction_mutex);

    std::vector<Transaction> result;
    int start = std::max(0, (int)recent_transactions.size() - count);

    for (size_t i = start; i < recent_transactions.size(); i++) {
        result.push_back(recent_transactions[i]);
    }

    return result;
}

// ============================================================================
// AN�LISE
// ============================================================================

float Tracker::GetInflationRate() const {
    // Taxa de infla��o = gold gerado / gold destru�do
    long long generated = gold_stats.total_generated.load();
    long long destroyed = gold_stats.total_destroyed.load();

    if (destroyed == 0) return 999.0f;  // Infla��o extrema

    return (float)generated / (float)destroyed;
}

int Tracker::GetGoldVelocity() const {
    // Velocidade = n�mero de transa��es / gold em circula��o
    int transactions = gold_stats.transaction_count.load();
    long long circulating = gold_stats.total_circulating.load();

    if (circulating == 0) return 0;

    return (int)(transactions * 1000 / circulating);  // Normalizado por 1000
}

std::string Tracker::GenerateEconomyReport() const {
    std::ostringstream report;

    report << "=== RELAT�RIO DE ECONOMIA ===\n\n";

    // Gold Stats
    report << "GOLD STATS:\n";
    report << "  Total Gerado: " << gold_stats.total_generated.load() << "\n";
    report << "  Total Destru�do: " << gold_stats.total_destroyed.load() << "\n";
    report << "  Em Circula��o: " << gold_stats.total_circulating.load() << "\n";
    report << "  M�dia de Transa��o: " << gold_stats.avg_transaction.load() << "\n";
    report << "  Pico de Transa��o: " << gold_stats.peak_transaction.load() << "\n";
    report << "  Total de Transa��es: " << gold_stats.transaction_count.load() << "\n\n";

    // An�lise
    report << "AN�LISE:\n";
    report << "  Taxa de Infla��o: " << std::fixed << std::setprecision(2)
           << GetInflationRate() << "x\n";
    report << "  Velocidade de Gold: " << GetGoldVelocity() << "\n\n";

    // Anomalias recentes
    auto recent_anomalies = GetRecentAnomalies(5);
    report << "�LTIMAS ANOMALIAS (" << recent_anomalies.size() << "):\n";
    for (const auto& a : recent_anomalies) {
        report << "  - Player " << a.player_id << ": " << a.reason << "\n";
    }

    return report.str();
}

} // namespace Economy

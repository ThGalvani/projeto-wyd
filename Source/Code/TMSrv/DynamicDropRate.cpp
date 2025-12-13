#include "DynamicDropRate.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cmath>

namespace Economy {

// Inst�ncia global
DynamicDropManager g_DynamicDrop;

// ============================================================================
// CONSTRUTOR / DESTRUTOR
// ============================================================================

DynamicDropManager::DynamicDropManager() {
    // Inicializa��o
}

DynamicDropManager::~DynamicDropManager() {
    // Cleanup
}

// ============================================================================
// FUN��ES AUXILIARES
// ============================================================================

float DynamicDropManager::CalculateAdjustment(int circulating, int desired) {
    // Calcula ajuste baseado em oferta vs demanda

    if (desired == 0) return BASE_DROP_RATE;

    float ratio = (float)circulating / (float)desired;

    // Oversupply: item muito comum, reduz drop
    if (circulating > OVERSUPPLY_THRESHOLD) {
        return MIN_DROP_RATE;  // 30% da taxa base
    }

    // Undersupply: item muito raro, aumenta drop
    if (circulating < UNDERSUPPLY_THRESHOLD) {
        return MAX_DROP_RATE;  // 300% da taxa base
    }

    // Ajuste suave baseado em ratio
    // ratio = 1.0 (perfeito) -> rate = 1.0
    // ratio > 1.0 (oversupply) -> rate < 1.0 (reduz drop)
    // ratio < 1.0 (undersupply) -> rate > 1.0 (aumenta drop)

    float adjustment = 1.0f / ratio;  // Inverso do ratio

    // Clamp entre MIN e MAX
    if (adjustment < MIN_DROP_RATE) adjustment = MIN_DROP_RATE;
    if (adjustment > MAX_DROP_RATE) adjustment = MAX_DROP_RATE;

    return adjustment;
}

void DynamicDropManager::AdjustDropRate(int item_id) {
    std::lock_guard<std::mutex> lock(drop_mutex);

    auto it = drop_rates.find(item_id);
    if (it == drop_rates.end()) return;

    DropInfo& info = it->second;

    // N�o ajusta items premium
    if (info.is_premium) return;

    // Calcula nova taxa
    float new_rate = CalculateAdjustment(info.circulating_count, info.desired_count);

    // Aplica ajuste gradual (evita mudan�as bruscas)
    float current = info.current_rate;
    float diff = new_rate - current;
    float step = diff * 0.2f;  // Ajusta 20% da diferen�a por vez

    info.current_rate = current + step;

    // Clamp final
    if (info.current_rate < MIN_DROP_RATE) info.current_rate = MIN_DROP_RATE;
    if (info.current_rate > MAX_DROP_RATE) info.current_rate = MAX_DROP_RATE;

    info.last_adjustment = time(nullptr);
}

bool DynamicDropManager::ShouldAdjust(time_t last_adjustment) {
    time_t now = time(nullptr);
    return (now - last_adjustment) >= ADJUSTMENT_INTERVAL;
}

// ============================================================================
// INICIALIZA��O
// ============================================================================

void DynamicDropManager::RegisterItem(int item_id, float base_rate, int desired_count, bool is_premium) {
    std::lock_guard<std::mutex> lock(drop_mutex);

    DropInfo info;
    info.item_id = item_id;
    info.base_rate = base_rate;
    info.current_rate = base_rate;
    info.desired_count = desired_count;
    info.is_premium = is_premium;
    info.last_adjustment = time(nullptr);
    info.circulating_count = 0;

    drop_rates[item_id] = info;
}

// ============================================================================
// CONSULTA DE TAXA ATUAL
// ============================================================================

float DynamicDropManager::GetDropRate(int item_id) {
    std::lock_guard<std::mutex> lock(drop_mutex);

    auto it = drop_rates.find(item_id);
    if (it != drop_rates.end()) {
        return it->second.current_rate;
    }

    // Item n�o registrado: retorna taxa base
    return BASE_DROP_RATE;
}

// ============================================================================
// ATUALIZA��O DE ESTAT�STICAS
// ============================================================================

void DynamicDropManager::UpdateCirculatingCount(int item_id, int count) {
    std::lock_guard<std::mutex> lock(drop_mutex);

    auto it = drop_rates.find(item_id);
    if (it != drop_rates.end()) {
        it->second.circulating_count = count;
    }
}

void DynamicDropManager::OnItemDropped(int item_id) {
    std::lock_guard<std::mutex> lock(drop_mutex);

    auto it = drop_rates.find(item_id);
    if (it != drop_rates.end()) {
        it->second.circulating_count++;
    }
}

void DynamicDropManager::OnItemDestroyed(int item_id) {
    std::lock_guard<std::mutex> lock(drop_mutex);

    auto it = drop_rates.find(item_id);
    if (it != drop_rates.end()) {
        if (it->second.circulating_count > 0) {
            it->second.circulating_count--;
        }
    }
}

// ============================================================================
// PROCESSAMENTO PERI�DICO
// ============================================================================

void DynamicDropManager::ProcessAdjustments() {
    std::lock_guard<std::mutex> lock(drop_mutex);

    for (auto& pair : drop_rates) {
        int item_id = pair.first;
        DropInfo& info = pair.second;

        // Verifica se deve ajustar (a cada hora)
        if (ShouldAdjust(info.last_adjustment)) {
            // Unlock para chamar AdjustDropRate (que tamb�m usa lock)
            drop_mutex.unlock();
            AdjustDropRate(item_id);
            drop_mutex.lock();
        }
    }
}

// ============================================================================
// RELAT�RIO
// ============================================================================

std::string DynamicDropManager::GenerateDropReport() const {
    std::ostringstream report;

    report << "=== RELAT�RIO DE DROP RATES DIN�MICOS ===\n\n";

    std::lock_guard<std::mutex> lock(drop_mutex);

    report << "Total de Itens Rastreados: " << drop_rates.size() << "\n\n";

    report << std::left << std::setw(10) << "Item ID"
           << std::setw(12) << "Base Rate"
           << std::setw(15) << "Current Rate"
           << std::setw(15) << "Circulating"
           << std::setw(12) << "Desired"
           << std::setw(10) << "Status" << "\n";
    report << std::string(80, '-') << "\n";

    for (const auto& pair : drop_rates) {
        const DropInfo& info = pair.second;

        std::string status;
        float ratio = (float)info.circulating_count / (float)info.desired_count;

        if (info.is_premium) {
            status = "PREMIUM";
        } else if (ratio > 1.5f) {
            status = "OVER";
        } else if (ratio < 0.5f) {
            status = "UNDER";
        } else {
            status = "OK";
        }

        report << std::left << std::setw(10) << info.item_id
               << std::setw(12) << std::fixed << std::setprecision(2) << info.base_rate
               << std::setw(15) << std::fixed << std::setprecision(2) << info.current_rate
               << std::setw(15) << info.circulating_count
               << std::setw(12) << info.desired_count
               << std::setw(10) << status << "\n";
    }

    return report.str();
}

// ============================================================================
// CONFIGURA��O MANUAL (GM)
// ============================================================================

void DynamicDropManager::SetDropRate(int item_id, float rate) {
    std::lock_guard<std::mutex> lock(drop_mutex);

    auto it = drop_rates.find(item_id);
    if (it != drop_rates.end()) {
        it->second.current_rate = rate;
    }
}

void DynamicDropManager::SetDesiredCount(int item_id, int count) {
    std::lock_guard<std::mutex> lock(drop_mutex);

    auto it = drop_rates.find(item_id);
    if (it != drop_rates.end()) {
        it->second.desired_count = count;
    }
}

void DynamicDropManager::ResetToBase(int item_id) {
    std::lock_guard<std::mutex> lock(drop_mutex);

    auto it = drop_rates.find(item_id);
    if (it != drop_rates.end()) {
        it->second.current_rate = it->second.base_rate;
    }
}

} // namespace Economy

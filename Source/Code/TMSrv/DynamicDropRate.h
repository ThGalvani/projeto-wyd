#ifndef DYNAMIC_DROP_RATE_H
#define DYNAMIC_DROP_RATE_H

#include <mutex>
#include <unordered_map>
#include <atomic>

// FASE 5: Sistema de Drop Rate Din�mico
// Auto-balanceamento baseado em oferta e demanda

namespace Economy {

// ============================================================================
// CONSTANTES
// ============================================================================

const float BASE_DROP_RATE = 1.0f;          // Taxa base (100%)
const float MIN_DROP_RATE = 0.3f;           // Taxa m�nima (30%)
const float MAX_DROP_RATE = 3.0f;           // Taxa m�xima (300%)
const int ADJUSTMENT_INTERVAL = 3600;       // Ajuste a cada 1 hora
const int OVERSUPPLY_THRESHOLD = 1000;      // Item muito comum
const int UNDERSUPPLY_THRESHOLD = 50;       // Item muito raro

// ============================================================================
// ESTRUTURAS
// ============================================================================

// Informa��o de drop de item
struct DropInfo {
    int item_id;
    float current_rate;              // Taxa atual (multiplicador)
    float base_rate;                 // Taxa base original
    int circulating_count;           // Quantidade em circula��o
    int desired_count;               // Quantidade desejada
    time_t last_adjustment;          // �ltimo ajuste
    bool is_premium;                 // Item premium (n�o ajusta)

    DropInfo() : item_id(0), current_rate(BASE_DROP_RATE), base_rate(BASE_DROP_RATE),
                 circulating_count(0), desired_count(100), last_adjustment(0),
                 is_premium(false) {}
};

// ============================================================================
// CLASSE PRINCIPAL
// ============================================================================

class DynamicDropManager {
private:
    std::mutex drop_mutex;
    std::unordered_map<int, DropInfo> drop_rates;  // item_id -> DropInfo

    // Fun��es auxiliares
    float CalculateAdjustment(int circulating, int desired);
    void AdjustDropRate(int item_id);
    bool ShouldAdjust(time_t last_adjustment);

public:
    DynamicDropManager();
    ~DynamicDropManager();

    // Inicializa��o
    void RegisterItem(int item_id, float base_rate, int desired_count, bool is_premium = false);

    // Consulta de taxa atual
    float GetDropRate(int item_id);

    // Atualiza��o de estat�sticas
    void UpdateCirculatingCount(int item_id, int count);
    void OnItemDropped(int item_id);
    void OnItemDestroyed(int item_id);

    // Processamento peri�dico
    void ProcessAdjustments();  // Chamar a cada hora

    // Relat�rio
    std::string GenerateDropReport() const;

    // Configura��o manual (GM)
    void SetDropRate(int item_id, float rate);
    void SetDesiredCount(int item_id, int count);
    void ResetToBase(int item_id);
};

// Inst�ncia global
extern DynamicDropManager g_DynamicDrop;

// ============================================================================
// FUN��ES HELPER PARA INTEGRA��O
// ============================================================================

// Usar ao dropar item de mob
inline bool ShouldDropItem(int item_id, float base_chance) {
    float dynamic_rate = g_DynamicDrop.GetDropRate(item_id);
    float final_chance = base_chance * dynamic_rate;

    // Clamp entre 0% e 100%
    if (final_chance > 1.0f) final_chance = 1.0f;
    if (final_chance < 0.0f) final_chance = 0.0f;

    float roll = (float)rand() / (float)RAND_MAX;
    bool should_drop = roll < final_chance;

    if (should_drop) {
        g_DynamicDrop.OnItemDropped(item_id);
    }

    return should_drop;
}

} // namespace Economy

#endif // DYNAMIC_DROP_RATE_H

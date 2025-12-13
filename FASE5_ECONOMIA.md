# FASE 5: SISTEMA DE ECONOMIA

**Data:** 2025-12-13
**Prioridade:** Alta (P1)

---

## 📋 VISÃO GERAL

A Fase 5 implementa um sistema completo de gestão econômica para o servidor WYD, incluindo:

1. **EconomyTracker**: Rastreamento de todas as transações de gold e itens
2. **DynamicDropRate**: Ajuste automático de drop rates baseado em oferta/demanda
3. **GoldSink**: Múltiplos sistemas para remover gold da economia (anti-inflação)

---

## 🎯 OBJETIVOS

- ✅ Monitorar fluxo de gold (geração vs destruição)
- ✅ Detectar anomalias econômicas (dupes, exploits)
- ✅ Auto-balancear economia (drop rates dinâmicos)
- ✅ Prevenir hiperinflação (gold sinks)
- ✅ Fornecer dados para análise econômica

---

## 🗂️ ARQUIVOS CRIADOS

### 1. EconomyTracker.h / EconomyTracker.cpp

**Localização:** `Source/Code/TMSrv/`

**Funcionalidades:**
- Rastreamento de transações de gold (drop, pickup, trade, NPC, sinks)
- Rastreamento de transações de itens (drop, pickup, trade, destroy, craft)
- Detecção automática de anomalias (valores anormais)
- Estatísticas em tempo real (gold gerado/destruído/circulando)
- Análise de inflação e velocidade de circulação

**Estruturas Principais:**
```cpp
enum class TransactionType {
    GOLD_DROP, GOLD_PICKUP, GOLD_TRADE,
    GOLD_NPC_SELL, GOLD_NPC_BUY, GOLD_SINK,
    ITEM_DROP, ITEM_PICKUP, ITEM_TRADE,
    ITEM_DESTROY, ITEM_CRAFT
};

struct GoldStats {
    std::atomic<long long> total_generated;
    std::atomic<long long> total_destroyed;
    std::atomic<long long> total_circulating;
    std::atomic<int> avg_transaction;
    std::atomic<int> peak_transaction;
};

struct ItemStats {
    int item_id;
    std::atomic<int> total_dropped;
    std::atomic<int> total_destroyed;
    std::atomic<int> total_circulating;
    std::atomic<int> avg_price;
};
```

**API Principal:**
```cpp
// Gold
Economy::g_EconomyTracker.RecordGoldDrop(player_id, amount, account, ip);
Economy::g_EconomyTracker.RecordGoldPickup(player_id, amount, account, ip);
Economy::g_EconomyTracker.RecordGoldTrade(p1, p2, amount, account, ip);
Economy::g_EconomyTracker.RecordGoldSink(player_id, amount, reason, account, ip);

// Items
Economy::g_EconomyTracker.RecordItemDrop(player_id, item_id, account, ip);
Economy::g_EconomyTracker.RecordItemTrade(p1, p2, item_id, price, account, ip);

// Análise
float inflation_rate = Economy::g_EconomyTracker.GetInflationRate();
std::string report = Economy::g_EconomyTracker.GenerateEconomyReport();
```

---

### 2. DynamicDropRate.h / DynamicDropRate.cpp

**Localização:** `Source/Code/TMSrv/`

**Funcionalidades:**
- Ajuste automático de drop rates (30% a 300% da taxa base)
- Balanceamento baseado em oferta vs demanda
- Registra items com quantidade desejada
- Processa ajustes a cada hora
- Suporta items premium (não ajusta)

**Lógica de Ajuste:**
```
Se circulating > 1000 (OVERSUPPLY):
    drop_rate = 30% (MIN_DROP_RATE)

Se circulating < 50 (UNDERSUPPLY):
    drop_rate = 300% (MAX_DROP_RATE)

Caso contrário:
    drop_rate = (desired / circulating) * base_rate
```

**API Principal:**
```cpp
// Registrar item para controle dinâmico
Economy::g_DynamicDrop.RegisterItem(
    item_id,
    base_rate,      // Taxa base (ex: 0.05 = 5%)
    desired_count,  // Quantidade desejada em circulação
    is_premium      // Se é item premium (não ajusta)
);

// Usar ao dropar item
if (Economy::ShouldDropItem(item_id, base_chance)) {
    // Dropa o item
}

// Processamento periódico (chamar a cada hora)
Economy::g_DynamicDrop.ProcessAdjustments();

// Relatório
std::string report = Economy::g_DynamicDrop.GenerateDropReport();
```

**Exemplo de Registro:**
```cpp
// Na inicialização do servidor
Economy::g_DynamicDrop.RegisterItem(4360, 0.01f, 100, false);  // Montaria: 1% base, 100 desejadas
Economy::g_DynamicDrop.RegisterItem(4670, 0.05f, 500, false);  // Item comum: 5% base, 500 desejados
Economy::g_DynamicDrop.RegisterItem(9999, 0.001f, 10, true);   // Item premium: não ajusta
```

---

### 3. GoldSink.h / GoldSink.cpp

**Localização:** `Source/Code/TMSrv/`

**Funcionalidades:**
- Taxas automáticas (5% trade, 10% auction)
- Serviços pagos (teleport, repair, storage, skill reset)
- Premium features (name change, guild create, appearance change)
- Sistema de upgrade exponencial
- Rastreamento de todo gold removido

**Sistemas de Sink:**

#### Taxas Automáticas:
- **Trade Tax**: 5% do valor tradado
- **Auction Tax**: 10% do valor vendido

#### Serviços:
- **Teleport**: 500 gold
- **Repair**: 10 gold por durabilidade
- **Storage Fee**: 100 gold/dia
- **Skill Reset**: 10,000 gold

#### Premium:
- **Name Change**: 50,000 gold
- **Guild Create**: 100,000 gold
- **Appearance Change**: 20,000 gold

#### Upgrade:
- **Fórmula**: `cost = 1000 * (2 ^ level)`
- Level +1: 1,000 gold
- Level +2: 2,000 gold
- Level +3: 4,000 gold
- Level +10: 512,000 gold

**API Principal:**
```cpp
// Taxas
int tax = Economy::g_GoldSink.ApplyTradeTax(player_id, trade_amount);
player_gold -= tax;

// Serviços
if (Economy::g_GoldSink.ChargeTeleport(player_id, player_gold)) {
    // Teleporta player
}

if (Economy::g_GoldSink.ChargeRepair(player_id, durability_lost, player_gold)) {
    // Restaura durabilidade
}

// Upgrade
int cost = Economy::g_GoldSink.CalculateUpgradeCost(current_level);
if (Economy::g_GoldSink.ChargeUpgrade(player_id, current_level, player_gold)) {
    // Aplica upgrade
}

// Estatísticas
long long total_removed = Economy::g_GoldSink.GetTotalGoldRemoved();
float removal_rate = Economy::g_GoldSink.GetRemovalRate(gold_generated);
```

---

## 🔧 INTEGRAÇÃO

### 1. Rastreamento de Gold

**Em _MSG_DropGold.cpp:**
```cpp
void Exec_MSG_DropGold(int conn, char* pMsg) {
    // ... lógica de drop ...

    // FASE 5: Registra gold dropado
    Economy::g_EconomyTracker.RecordGoldDrop(
        conn, gold_amount,
        pUser[conn].AccountName,
        pUser[conn].IP
    );
}
```

**Em _MSG_PickupGold.cpp:**
```cpp
void Exec_MSG_PickupGold(int conn, char* pMsg) {
    // ... lógica de pickup ...

    // FASE 5: Registra pickup
    Economy::g_EconomyTracker.RecordGoldPickup(
        conn, gold_amount,
        pUser[conn].AccountName,
        pUser[conn].IP
    );
}
```

**Em _MSG_Trade.cpp:**
```cpp
void Exec_MSG_Trade(int conn, char* pMsg) {
    int target_id = msg->TargetID;
    int gold_amount = msg->Gold;

    // FASE 5: Aplica taxa de trade (5%)
    int tax = Economy::g_GoldSink.ApplyTradeTax(conn, gold_amount);
    gold_amount -= tax;  // Deduz taxa

    // ... executa trade ...

    // FASE 5: Registra trade
    Economy::g_EconomyTracker.RecordGoldTrade(
        conn, target_id, gold_amount,
        pUser[conn].AccountName,
        pUser[conn].IP
    );
}
```

---

### 2. Drop Rate Dinâmico

**Em MobAI.cpp (ao dropar item):**
```cpp
void DropItemFromMob(int mob_id, int item_id) {
    float base_chance = GetBaseDropChance(item_id);  // Ex: 0.05 (5%)

    // FASE 5: Usa drop rate dinâmico
    if (Economy::ShouldDropItem(item_id, base_chance)) {
        // Cria item no mundo
        CreateItemOnGround(item_id, mob_x, mob_y);
    }
}
```

**Em Server.cpp (processamento periódico):**
```cpp
void ServerLoop() {
    static time_t last_adjustment = time(nullptr);

    while (running) {
        // ... lógica do servidor ...

        // FASE 5: Ajusta drop rates a cada hora
        time_t now = time(nullptr);
        if (now - last_adjustment >= 3600) {  // 1 hora
            Economy::g_DynamicDrop.ProcessAdjustments();
            last_adjustment = now;
        }
    }
}
```

---

### 3. Gold Sinks

**Em _MSG_Teleport.cpp:**
```cpp
void Exec_MSG_Teleport(int conn, char* pMsg) {
    int player_gold = pMob[conn].MOB.Gold;

    // FASE 5: Cobra teleport (500 gold)
    if (!Economy::g_GoldSink.ChargeTeleport(conn, player_gold)) {
        SendClientMessage(conn, "Gold insuficiente! Teleport custa 500 gold.");
        return;
    }

    pMob[conn].MOB.Gold = player_gold;  // Atualiza gold

    // Executa teleport
    // ...
}
```

**Em _MSG_RepairItem.cpp:**
```cpp
void Exec_MSG_RepairItem(int conn, char* pMsg) {
    int durability_lost = 100 - item.Durability;
    int player_gold = pMob[conn].MOB.Gold;

    // FASE 5: Calcula e cobra repair
    int cost = Economy::g_GoldSink.CalculateRepairCost(durability_lost);

    if (!Economy::g_GoldSink.ChargeRepair(conn, durability_lost, player_gold)) {
        SendClientMessage(conn, "Gold insuficiente! Repair custa %d gold.", cost);
        return;
    }

    pMob[conn].MOB.Gold = player_gold;
    item.Durability = 100;  // Restaura
}
```

---

## 📊 ANÁLISE ECONÔMICA

### Detectar Inflação:
```cpp
float inflation_rate = Economy::g_EconomyTracker.GetInflationRate();

if (inflation_rate > 2.0f) {
    // ALERTA: Inflação alta! Gold gerado >> Gold destruído
    // Ação: Aumentar custos dos gold sinks
    // OU: Reduzir drop de gold de mobs
}
```

### Balancear Items:
```cpp
// Registra todos os items valiosos
Economy::g_DynamicDrop.RegisterItem(4360, 0.01f, 100, false);  // Montaria
Economy::g_DynamicDrop.RegisterItem(4670, 0.05f, 500, false);  // Poção
Economy::g_DynamicDrop.RegisterItem(4680, 0.02f, 200, false);  // Armor

// Sistema ajusta automaticamente:
// - Se muitos players têm montaria → reduz drop
// - Se poucos players têm poção → aumenta drop
```

### Monitorar Anomalias:
```cpp
auto anomalies = Economy::g_EconomyTracker.GetRecentAnomalies(10);

for (const auto& anomaly : anomalies) {
    if (anomaly.amount > 100000) {
        // ALERTA: Player transferiu 100k+ gold
        // Possível RMT ou exploit
        SystemLog("ECONOMY", "ANOMALY DETECTED: Player %d - %s",
            anomaly.player_id, anomaly.reason);
    }
}
```

---

## 🧪 TESTES

### Teste 1: Rastreamento de Gold
1. Mate mobs e colete gold
2. Faça trade de gold com outro player
3. Compre item de NPC
4. Verifique relatório: `Economy::g_EconomyTracker.GenerateEconomyReport()`
5. Confirme que todas as transações foram registradas

### Teste 2: Drop Rate Dinâmico
1. Registre item com desired_count = 10
2. Drope 50+ deste item
3. Verifique relatório: `Economy::g_DynamicDrop.GenerateDropReport()`
4. Confirme que drop_rate diminuiu (oversupply)

### Teste 3: Gold Sinks
1. Use teleport (deve custar 500 gold)
2. Repare item (deve custar 10 gold/durabilidade)
3. Faça trade (deve cobrar 5% de taxa)
4. Verifique relatório: `Economy::g_GoldSink.GenerateSinkReport()`
5. Confirme que gold foi removido

---

## 📈 MÉTRICAS

**Métricas Principais:**
- Gold gerado/hora
- Gold destruído/hora
- Taxa de inflação (gerado/destruído)
- Velocidade de circulação
- Anomalias detectadas/dia
- Efetividade dos sinks (%)

**Meta Ideal:**
- Inflação: 1.0x a 1.2x (equilíbrio)
- Remoção via sinks: 40-60% do gold gerado
- Anomalias: < 5 por dia

---

## ⚙️ CONFIGURAÇÃO

### Ajustar Limiares de Anomalia:
```cpp
// Em EconomyTracker.h
const int ANOMALY_THRESHOLD_GOLD = 200000;  // 200k em vez de 100k
const int ANOMALY_THRESHOLD_MULTIPLIER = 10;  // 10x a média
```

### Ajustar Drop Rates:
```cpp
// Em DynamicDropRate.h
const float MIN_DROP_RATE = 0.1f;   // Mínimo 10% (em vez de 30%)
const float MAX_DROP_RATE = 5.0f;   // Máximo 500% (em vez de 300%)
```

### Ajustar Gold Sinks:
```cpp
// Em GoldSink.h
const float TRADE_TAX_RATE = 0.10f;      // 10% (em vez de 5%)
const int TELEPORT_COST = 1000;          // 1000 gold
const int UPGRADE_BASE_COST = 2000;      // 2000 gold base
```

---

## 🎯 PRÓXIMOS PASSOS

1. **Integrar com Dashboard** (FASE3_INTEGRACAO.md)
   - Adicionar endpoint `/api/economy`
   - Mostrar gráficos de inflação no dashboard

2. **Integrar com ServerMetrics**
   - Adicionar métricas econômicas ao sistema de monitoramento

3. **Criar Comandos GM**
   - `/economy report` - Relatório completo
   - `/economy inflation` - Taxa de inflação
   - `/economy sinks` - Estatísticas de gold sinks
   - `/droprate <item_id>` - Ver/ajustar drop rate

4. **Alertas Automáticos**
   - Notificar GMs quando inflação > 2.0x
   - Notificar quando anomalia detectada
   - Notificar quando item está em undersupply

---

## ✅ CHECKLIST DE IMPLEMENTAÇÃO

- [x] EconomyTracker.h/cpp criados
- [x] DynamicDropRate.h/cpp criados
- [x] GoldSink.h/cpp criados
- [ ] Integrar RecordGoldDrop/Pickup em código existente
- [ ] Integrar ShouldDropItem em MobAI
- [ ] Integrar Gold Sinks em NPCs e serviços
- [ ] Adicionar processamento periódico (ProcessAdjustments)
- [ ] Testar rastreamento de gold
- [ ] Testar drop rate dinâmico
- [ ] Testar gold sinks
- [ ] Criar comandos GM
- [ ] Integrar com dashboard

---

**FASE 5 COMPLETA!** 🎉

Sistema de economia implementado e pronto para integração!

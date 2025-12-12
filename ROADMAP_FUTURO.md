# 🗺️ ROADMAP FUTURO - Servidor WYD

**Projeto:** WYD Server Security & Features
**Autor:** Claude AI
**Data:** 12/12/2025
**Status:** Documento Estratégico

---

## 📋 Sumário Executivo

Este documento define o roadmap de desenvolvimento para o servidor WYD após conclusão das Fases 1-3.

**Trabalho Completado:**
- ✅ **Fase 1:** Correções críticas de segurança (anti-dupe)
- ✅ **Fase 2:** SaveUserSync + Rollback + Estabilização
- ✅ **Fase 3:** Sistema de métricas e dashboard

**Próximas Fases:**
- 🔵 **Fase 4:** Segurança Avançada & Anti-Cheat
- 🟢 **Fase 5:** Economia & Balanceamento
- 🟡 **Fase 6:** Sistema de Eventos Automatizados
- 🟠 **Fase 7:** Backup & Disaster Recovery
- 🔴 **Fase 8:** Performance & Escalabilidade

---

## 🎯 Critérios de Priorização

Cada feature é avaliada por:

| Critério | Peso | Descrição |
|----------|------|-----------|
| **Impacto** | 40% | Benefício para jogadores/servidor |
| **Complexidade** | 30% | Tempo e dificuldade de implementação |
| **Risco** | 20% | Probabilidade de bugs/problemas |
| **Dependências** | 10% | Precisa de outras features primeiro |

**Níveis:**
- 🟢 **P0 - Crítico:** Implementar ASAP
- 🟡 **P1 - Alto:** Implementar em 1-2 meses
- 🟠 **P2 - Médio:** Implementar em 3-6 meses
- 🔵 **P3 - Baixo:** Implementar quando possível

---

## 🔵 FASE 4: Segurança Avançada & Anti-Cheat

**Prioridade:** 🟢 P0 (Crítico)
**Tempo Estimado:** 40-60 horas
**Complexidade:** Alta

### Objetivos:
1. Detectar e prevenir cheats comuns
2. Validar operações críticas do lado do servidor
3. Implementar rate limiting
4. Log e ban automático de suspeitos

---

### 4.1. Sistema Anti-Speed Hack 🟢 P0

**Problema:** Players usando speed hack para farm/PvP injusto.

**Solução:**

```cpp
// AntiCheat.h
class SpeedMonitor {
private:
    struct PlayerMovement {
        int last_x, last_y;
        time_t last_move_time;
        int suspicious_moves;
        double avg_speed;
    };

    std::map<int, PlayerMovement> player_data;

public:
    bool ValidateMove(int conn, int new_x, int new_y);
    void UpdatePlayerPosition(int conn, int x, int y);
    bool IsSpeedHacking(int conn);
    void BanPlayer(int conn, const char* reason);
};
```

**Implementação:**
1. Calcular distância euclidiana entre movimentos
2. Validar contra velocidade máxima permitida
3. Acumular "suspicious_moves" counter
4. Se > threshold → Log + Kick/Ban

**Threshold:**
- Max speed: 10 tiles/segundo (configurável)
- Suspicious threshold: 5 violações em 30s

**Integração:**
- Hook em `_MSG_MovePlayer.cpp`
- Log em arquivo "speedhack.log"

**Estimativa:** 12 horas

---

### 4.2. Validação de Dano Server-Side 🟢 P0

**Problema:** Players podem manipular dano via packet injection.

**Solução:**

```cpp
// DamageValidator.h
class DamageValidator {
public:
    // Calcula dano esperado
    int CalculateExpectedDamage(
        CMob* attacker,
        CMob* target,
        int skill_id,
        bool is_critical
    );

    // Valida dano reportado pelo cliente
    bool ValidateDamage(
        int reported_damage,
        int expected_damage,
        float tolerance = 0.15f  // 15% margem
    );

    // Reverte dano inválido
    void RevertInvalidDamage(int conn, int target_conn);
};
```

**Fórmula de Dano:**
```
Base Damage = (ATK - DEF) * Skill Multiplier
Critical = Base * 1.5
Variance = ±10%
```

**Implementação:**
1. Calcular dano esperado no servidor
2. Comparar com dano reportado pelo cliente
3. Se diferença > 15% → Rejeitar + Log
4. Restaurar HP do target

**Integração:**
- Hook em `_MSG_Attack.cpp`
- Validar ANTES de aplicar dano

**Estimativa:** 20 horas

---

### 4.3. Rate Limiting de Operações 🟡 P1

**Problema:** Spam de packets pode causar lag/exploits.

**Solução:**

```cpp
// RateLimiter.h
class RateLimiter {
private:
    struct RateLimit {
        int count;
        time_t window_start;
        int max_requests;
        int window_seconds;
    };

    std::map<std::string, RateLimit> limits;

public:
    bool AllowRequest(int conn, const char* operation);
    void ResetLimit(const char* operation);
};
```

**Limites Recomendados:**
```cpp
// Operações por segundo
RateLimits = {
    {"trade", 1},        // 1 trade/s
    {"drop", 5},         // 5 drops/s
    {"get", 10},         // 10 gets/s
    {"attack", 20},      // 20 ataques/s
    {"move", 50},        // 50 movimentos/s
    {"chat", 3}          // 3 mensagens/s
};
```

**Penalidades:**
- 1ª violação: Warning
- 2ª violação: Kick
- 3ª violação: Ban temporário (1h)

**Estimativa:** 8 horas

---

### 4.4. Sistema de Ban Automatizado 🟡 P1

**Features:**
- **Ban permanente** (never unban)
- **Ban temporário** (1h, 24h, 7d, 30d)
- **Ban de IP**
- **Ban de MAC address**
- **Ban de hardware ID** (se disponível)

```cpp
struct BanRecord {
    int id;
    char account_name[ACCOUNTNAME_LENGTH];
    char ip_address[16];
    char mac_address[18];
    char reason[256];
    time_t ban_start;
    time_t ban_end;  // 0 = permanent
    int admin_id;
    bool is_permanent;
};
```

**Banco de Dados:**
```sql
CREATE TABLE bans (
    id INT PRIMARY KEY AUTO_INCREMENT,
    account_name VARCHAR(16),
    ip_address VARCHAR(16),
    mac_address VARCHAR(18),
    reason VARCHAR(256),
    ban_start DATETIME,
    ban_end DATETIME NULL,
    admin_id INT,
    is_permanent BOOLEAN DEFAULT 0
);
```

**Estimativa:** 15 horas

---

## 🟢 FASE 5: Economia & Balanceamento

**Prioridade:** 🟡 P1 (Alto)
**Tempo Estimado:** 50-70 horas
**Complexidade:** Média-Alta

### Objetivos:
1. Monitorar inflação da economia
2. Ajustar drop rates dinamicamente
3. Implementar sink de gold
4. Balancear mercado

---

### 5.1. Sistema de Economia - Rastreamento 🟡 P1

**Métricas para Rastrear:**
- Total de gold no servidor
- Gold criado (drops) vs destruído (NPCs)
- Distribuição de riqueza (top 1%, 10%, 50%)
- Preço médio de itens em trades
- Inflação mensal (%)

```cpp
// EconomyTracker.h
class EconomyTracker {
public:
    // Rastreia criação de gold
    void OnGoldCreated(int amount, const char* source);

    // Rastreia destruição de gold
    void OnGoldDestroyed(int amount, const char* sink);

    // Estatísticas
    struct EconomySnapshot {
        int64_t total_gold_in_game;
        int64_t gold_created_today;
        int64_t gold_destroyed_today;
        int64_t net_gold;
        float inflation_rate;
        int unique_accounts_with_gold;
    };

    EconomySnapshot GetSnapshot();
};
```

**Implementação:**
- Hook em drops de gold
- Hook em compras de NPCs
- Hook em taxas de trade
- Dashboard dedicado

**Estimativa:** 15 horas

---

### 5.2. Drop Rate Dinâmico 🟠 P2

**Conceito:** Ajustar drop rates baseado em economia.

**Exemplo:**
```cpp
float CalculateDynamicDropRate(int item_id)
{
    float base_rate = GetBaseDropRate(item_id);
    int current_count = ItemMonitor::CountItemInServer(item_id);
    int target_count = GetTargetItemCount(item_id);

    float scarcity_multiplier = (float)target_count / current_count;

    // Se item está escasso (scarcity > 1.5), aumenta drop
    if (scarcity_multiplier > 1.5f)
        return base_rate * 1.25f;

    // Se item está em excesso (scarcity < 0.7), diminui drop
    if (scarcity_multiplier < 0.7f)
        return base_rate * 0.75f;

    return base_rate;
}
```

**Benefícios:**
- Auto-balanceamento de economia
- Mantém itens raros valiosos
- Previne inflação de itens

**Estimativa:** 20 horas

---

### 5.3. Gold Sink - Sistemas de Destruição 🟠 P2

**Problema:** Gold acumula infinitamente, causando inflação.

**Soluções:**

#### A. Taxa de Trade (Já Comum)
```cpp
// 5% de taxa em trades de gold
int trade_tax = (trade_amount * 5) / 100;
int final_amount = trade_amount - trade_tax;

// Gold da taxa é DESTRUÍDO (não vai para NPC)
ServerMetrics::EconomyTracker::OnGoldDestroyed(trade_tax, "trade_tax");
```

#### B. Repair de Equipamentos
```cpp
// Durabilidade de itens diminui com uso
// Repair custa gold baseado em valor do item
int repair_cost = (item_value * durability_lost) / 100;
```

#### C. Teleport Caro
```cpp
// Teleport entre cidades custa gold
int teleport_cost = 1000 * player_level;
```

#### D. Enchant com Chance de Falha
```cpp
// Enchant +7 → +8 tem 30% chance
// Se falha, consome GOLD mas não aplica upgrade
// Gold é destruído
```

**Estimativa:** 15 horas

---

## 🟡 FASE 6: Sistema de Eventos Automatizados

**Prioridade:** 🟠 P2 (Médio)
**Tempo Estimado:** 40-50 horas
**Complexidade:** Média

### Objetivos:
1. Eventos automáticos (sem GM)
2. Calendário de eventos
3. Recompensas automáticas
4. Variedade de tipos de evento

---

### 6.1. Framework de Eventos 🟠 P2

```cpp
// EventManager.h
class Event {
public:
    virtual void Start() = 0;
    virtual void Update(float delta_time) = 0;
    virtual void End() = 0;
    virtual bool ShouldStart() = 0;
};

class EventManager {
private:
    std::vector<Event*> registered_events;
    Event* current_event;

public:
    void RegisterEvent(Event* event);
    void Update();
    void ForceStart(const char* event_name);
    void ForceStop();
};
```

**Tipos de Evento:**

#### A. Boss Invasion (Invasão de Boss)
```cpp
class BossInvasionEvent : public Event {
    // Spawna boss poderoso em cidade aleatória
    // Players devem derrotar em 30 minutos
    // Recompensa: Gold + Item raro
};
```

#### B. Double EXP Weekend
```cpp
class DoubleEXPEvent : public Event {
    // Sexta 18h → Domingo 23h
    // EXP multiplicado por 2
    // Anúncio automático
};
```

#### C. Treasure Hunt (Caça ao Tesouro)
```cpp
class TreasureHuntEvent : public Event {
    // Spawna 10 baús aleatórios no mapa
    // Primeiro a abrir ganha prêmio
    // Resetado a cada 4 horas
};
```

#### D. PvP Tournament (Torneio PvP)
```cpp
class PvPTournamentEvent : public Event {
    // Cadastro: 30 minutos
    // Brackets automáticos
    // Lutas 1v1 em arena especial
    // Top 3 ganham prêmios
};
```

**Estimativa:** 30 horas

---

### 6.2. Calendário de Eventos 🟠 P2

**Formato JSON:**
```json
{
  "events": [
    {
      "name": "Double EXP Weekend",
      "type": "passive",
      "schedule": "0 18 * * 5-0",  // Cron: Sexta 18h até Domingo
      "duration_hours": 53,
      "multipliers": {
        "exp": 2.0
      }
    },
    {
      "name": "Boss Invasion",
      "type": "active",
      "schedule": "0 */6 * * *",  // A cada 6 horas
      "duration_minutes": 30,
      "boss_id": 123,
      "reward": {
        "gold": 10000,
        "item_id": 415,
        "item_amount": 5
      }
    }
  ]
}
```

**Anúncios Automáticos:**
```cpp
void Event::AnnounceStart()
{
    SendGlobalMessage("=== EVENTO INICIADO ===");
    SendGlobalMessage(GetEventName());
    SendGlobalMessage(GetEventDescription());
    SendGlobalMessage("=======================");
}
```

**Estimativa:** 10 horas

---

## 🟠 FASE 7: Backup & Disaster Recovery

**Prioridade:** 🟢 P0 (Crítico para produção)
**Tempo Estimado:** 25-35 horas
**Complexidade:** Média

### Objetivos:
1. Backup automático de dados
2. Recuperação rápida de desastres
3. Rollback de dados
4. Replicação de banco

---

### 7.1. Sistema de Backup Automático 🟢 P0

```cpp
// BackupManager.h
class BackupManager {
public:
    // Backup completo (DB + logs + configs)
    bool CreateFullBackup();

    // Backup incremental (apenas mudanças)
    bool CreateIncrementalBackup();

    // Restaurar de backup
    bool RestoreFromBackup(const char* backup_name);

    // Listar backups disponíveis
    std::vector<BackupInfo> ListBackups();

    // Deletar backups antigos
    void CleanOldBackups(int keep_days = 30);
};
```

**Estratégia 3-2-1:**
- **3 cópias:** Original + 2 backups
- **2 mídias:** HDD local + Cloud (AWS S3/Azure)
- **1 offsite:** Backup na nuvem

**Agendamento:**
```
Full Backup:  Domingo 03:00 (semanal)
Incremental:  Diariamente 03:00
Logs:         A cada 1 hora
```

**Compressão:**
```cpp
// Comprimir com gzip para economizar espaço
system("tar -czf backup_2025-12-12.tar.gz /path/to/data");

// Média: 10GB de dados → 1GB comprimido (90% economia)
```

**Estimativa:** 20 horas

---

### 7.2. Replicação de Banco de Dados 🟡 P1

**MySQL Master-Slave Replication:**

**Setup:**
```sql
-- Master (servidor principal)
[mysqld]
server-id=1
log-bin=mysql-bin
binlog-do-db=wyd_server

-- Slave (servidor backup)
[mysqld]
server-id=2
relay-log=mysql-relay-bin
```

**Benefícios:**
- ✅ Backup em tempo real
- ✅ Leitura distribuída (slaves podem servir SELECT)
- ✅ Failover automático (se master cai, promove slave)

**Estimativa:** 8 horas (configuração)

---

## 🔴 FASE 8: Performance & Escalabilidade

**Prioridade:** 🔵 P3 (Baixo, para >500 players)
**Tempo Estimado:** 60-80 horas
**Complexidade:** Muito Alta

### Objetivos:
1. Otimizar código crítico
2. Suportar +1000 players simultâneos
3. Reduzir latência
4. Load balancing

---

### 8.1. Otimização de Grid System 🔵 P3

**Problema Atual:** Grid 2D linear (O(n) busca).

**Solução:** Quadtree ou Grid Hash.

```cpp
// QuadTree para posicionamento espacial
class QuadTree {
    struct Node {
        int x, y, w, h;
        std::vector<CMob*> mobs;
        Node* children[4];
    };

public:
    void Insert(CMob* mob, int x, int y);
    std::vector<CMob*> QueryRange(int x, int y, int radius);
};
```

**Benefício:**
- Busca de vizinhos: O(n) → O(log n)
- Crítico para >500 players

**Estimativa:** 25 horas

---

### 8.2. Thread Pool para I/O 🔵 P3

**Problema:** SaveUser bloqueia thread principal.

**Solução:**
```cpp
class ThreadPool {
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;

public:
    void EnqueueTask(std::function<void()> task);
    void WaitAll();
};

// Uso:
g_ThreadPool.EnqueueTask([conn]() {
    SaveUser(conn, 1);
});
```

**Benefício:**
- I/O não bloqueia game loop
- Pode processar múltiplos saves em paralelo

**Estimativa:** 15 horas

---

### 8.3. Load Balancer para Múltiplos TMSrv 🔵 P3

**Arquitetura:**
```
Players
   ↓
[HAProxy Load Balancer]
   ↓
[TMSrv 1] [TMSrv 2] [TMSrv 3]
   ↓         ↓         ↓
      [DBSrv Shared]
```

**Desafios:**
- Sincronizar estado entre TMSrv
- Cross-server trades/parties
- Session persistence

**Estimativa:** 40 horas (complexo!)

---

## 📊 Resumo de Prioridades

### 🟢 Implementar Agora (P0 - 0-3 meses):
1. **Anti-Speed Hack** (12h)
2. **Validação de Dano** (20h)
3. **Sistema de Ban** (15h)
4. **Backup Automático** (20h)

**Total P0:** ~67 horas (~2 semanas de trabalho)

---

### 🟡 Implementar em Breve (P1 - 3-6 meses):
1. **Rate Limiting** (8h)
2. **Economia Tracker** (15h)
3. **Replicação de DB** (8h)

**Total P1:** ~31 horas (~1 semana)

---

### 🟠 Implementar Depois (P2 - 6-12 meses):
1. **Drop Rate Dinâmico** (20h)
2. **Gold Sink** (15h)
3. **Sistema de Eventos** (40h)

**Total P2:** ~75 horas (~2 semanas)

---

### 🔵 Implementar se Necessário (P3 - >12 meses):
1. **QuadTree** (25h)
2. **Thread Pool** (15h)
3. **Load Balancer** (40h)

**Total P3:** ~80 horas (~2 semanas)

---

## 🛠️ Ferramentas Recomendadas

### Desenvolvimento:
- **Visual Studio 2022** (IDE)
- **Git** (controle de versão)
- **CMake** (build system alternativo)

### Monitoramento:
- **Grafana** (dashboards avançados)
- **Prometheus** (time-series DB)
- **ELK Stack** (logs)

### Backup:
- **AWS S3** (storage cloud)
- **rclone** (sync automático)
- **rsync** (backup incremental)

### Anti-Cheat:
- **WireShark** (análise de packets)
- **Cheat Engine** (teste de vulnerabilidades)
- **OllyDbg** (reverse engineering)

---

## 📈 Estimativas de Crescimento

### Com Roadmap Completo:

**Segurança:**
- Anti-dupe: 99.9% efetivo
- Anti-speed: 95% efetivo
- Anti-damage-hack: 90% efetivo

**Economia:**
- Inflação controlada: <5% ao mês
- Gold sink efetivo: 30% do gold criado é destruído
- Balanceamento automático

**Disponibilidade:**
- Uptime: 99.5% (backup + replicação)
- Recovery Time: <1 hora
- Data Loss: <5 minutos (replicação em tempo real)

**Performance:**
- Suporta 500 players: ✅ Garantido
- Suporta 1000 players: ✅ Com otimizações P3
- Latência média: <50ms (rede permitting)

---

## 🎯 Conclusão

Este roadmap fornece direção clara para os próximos 12-24 meses de desenvolvimento.

**Priorize:**
1. **Segurança primeiro** (Fase 4 P0)
2. **Estabilidade depois** (Fase 7 P0)
3. **Features em seguida** (Fases 5-6 P1-P2)
4. **Performance se necessário** (Fase 8 P3)

**Mantra:**
> "Make it work, make it right, make it fast." - Kent Beck

**Próximo Passo:**
Implementar **Anti-Speed Hack + Validação de Dano** da Fase 4.

---

**Documento Vivo:** Este roadmap deve ser revisado a cada 3 meses e ajustado conforme necessidades da comunidade.

**Feedback:** Implemente features baseado em feedback real de players. Dados do Dashboard (Fase 3) vão guiar decisões.

🚀 **Bom desenvolvimento!**

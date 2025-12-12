# FASE 3 - GUIA DE INTEGRAÇÃO

Este guia mostra exatamente onde adicionar as chamadas do sistema de métricas no código existente.

---

## 📋 Passo 1: Adicionar Includes

### Em `Server.cpp` (no topo do arquivo):

```cpp
#include "ServerMetrics.h"
#include "ItemMonitor.h"
#include "MetricsHTTP.h"
```

---

## 📋 Passo 2: Inicializar Sistema HTTP

### Em `Server.cpp`, na função de inicialização do servidor (procure por "InitServer" ou "main"):

```cpp
// Após inicializar rede e DB, adicione:

// Inicializa servidor HTTP de métricas (porta 8080)
if (MetricsHTTP::InitMetricsHTTP(8080))
{
    printf("Metrics HTTP Server started on port 8080\n");
    printf("Dashboard: http://localhost:8080/\n");
}
else
{
    printf("Failed to start Metrics HTTP Server\n");
}
```

---

## 📋 Passo 3: Loop de Atualização

### Em `Server.cpp`, no loop principal do servidor:

```cpp
// Adicione no loop principal (geralmente onde tem while(1) ou similar):

static time_t last_metrics_update = 0;
static time_t last_item_scan = 0;
time_t current_time = time(nullptr);

// Atualiza saúde do servidor a cada 10 segundos
if (current_time - last_metrics_update >= 10)
{
    int active_players = 0;
    int active_mobs = 0;
    int items_ground = 0;

    // Conta players
    for (int i = 0; i < MAX_USER; i++)
    {
        if (pUser[i].Mode == USER_PLAY)
            active_players++;
    }

    // Conta mobs
    for (int i = 0; i < MAX_MOB; i++)
    {
        if (pMob[i].Mode != MOB_EMPTY)
            active_mobs++;
    }

    // Conta itens no chão
    for (int i = 0; i < MAX_ITEM; i++)
    {
        if (pItem[i].Mode != 0)
            items_ground++;
    }

    ServerMetrics::g_Metrics.UpdateServerHealth(active_players, active_mobs, items_ground);
    last_metrics_update = current_time;
}

// Varredura de itens raros a cada 5 minutos
if (current_time - last_item_scan >= 300)
{
    ItemMonitor::g_ItemMonitor.PerformFullScan();
    last_item_scan = current_time;
    printf("Item scan completed\n");
}
```

---

## 📋 Passo 4: Integrar SaveUserSync()

### Em `Server.cpp`, na função `SaveUserSync()` (já existe, apenas adicione métricas):

Encontre estas linhas (aproximadamente linha 493):
```cpp
if (confirmed && pending.success)
{
    // ADICIONE AQUI:
    auto end = std::chrono::steady_clock::now();
    double elapsed_ms = std::chrono::duration<double, std::milli>(end - pending.timestamp).count();
    ServerMetrics::g_Metrics.RecordSaveSuccess(elapsed_ms);

    // ... resto do código existente ...
}
else if (!confirmed)
{
    // ADICIONE AQUI:
    ServerMetrics::g_Metrics.RecordSaveTimeout();

    // ... resto do código ...
}
else
{
    // ADICIONE AQUI:
    ServerMetrics::g_Metrics.RecordSaveFailure();

    // ... resto do código ...
}
```

---

## 📋 Passo 5: Integrar Trade

### Em `_MSG_Trade.cpp`, após trade bem-sucedido (linha ~444):

```cpp
// SUCESSO: Save confirmado pelo DBSrv
snprintf(temp, sizeof(temp), "Trade SAVE CONFIRMED by DBSrv: [%s] and [%s]",
    pUser[conn].AccountName, pUser[OpponentID].AccountName);
SystemLog("TRADE-SYSTEM", "00:00:00:00:00:00", 0, temp);

// ADICIONE AQUI:
ServerMetrics::g_Metrics.RecordTradeSuccess();
```

### No rollback (linha ~420):

```cpp
// FALHA NO SAVE: ROLLBACK COMPLETO
snprintf(temp, sizeof(temp), "Trade SAVE FAILED - ROLLBACK: conn:%d save:%d, opp:%d save:%d",
    conn, save1_success, OpponentID, save2_success);
SystemLog("TRADE-SYSTEM", "00:00:00:00:00:00", 0, temp);

// ADICIONE AQUI:
ServerMetrics::g_Metrics.RecordTradeFailure();
ServerMetrics::g_Metrics.RecordTradeRollback();
```

---

## 📋 Passo 6: Integrar DropItem

### Em `_MSG_DropItem.cpp`, após CreateItem (linha ~205):

```cpp
int drop = CreateItem(m->GridX, m->GridY, &itemBackup, m->Rotate, 1);

// ADICIONE AQUI:
ServerMetrics::g_Metrics.RecordDrop(drop > 0 && drop < MAX_ITEM);

// PASSO 4: ROLLBACK if failed
if (drop <= 0 || drop >= MAX_ITEM)
{
    // ... código de rollback ...
}
```

---

## 📋 Passo 7: Integrar GetItem

### Em `_MSG_GetItem.cpp`, após pegar item com sucesso (linha ~350):

```cpp
// Atualiza inventário do player
memcpy(bItem, ditem, sizeof(STRUCT_ITEM));
SendItem(conn, Vol, bPos, bItem);

// ADICIONE AQUI:
ServerMetrics::g_Metrics.RecordGet(true);
```

### E quando falha (onde tem returns de erro):

```cpp
// Quando falhar validação:
if (bItem->sIndex != 0)
{
    // ROLLBACK
    pItemGrid[itemY][itemX] = itemID;
    pItem[itemID].Mode = 1;

    // ADICIONE:
    ServerMetrics::g_Metrics.RecordGet(false);
    return;
}
```

---

## 📋 Passo 8: Integrar SplitItem

### Em `_MSG_SplitItem.cpp`, quando split for bem-sucedido (linha ~168):

```cpp
// SUCCESS: Now reduce original quantity
if (amount > 1)
    BASE_SetItemAmount(&pMob[conn].MOB.Carry[slot], amount - m->Num);
else
    memset(&pMob[conn].MOB.Carry[slot], 0, sizeof(STRUCT_ITEM));

SendCarry(conn);

// ADICIONE AQUI:
ServerMetrics::g_Metrics.RecordSplit();
```

---

## 📋 Passo 9: Shutdown

### Em `Server.cpp`, na função de shutdown do servidor:

```cpp
void ShutdownServer()
{
    // ... código existente ...

    // Shutdown metrics HTTP server
    MetricsHTTP::ShutdownMetricsHTTP();

    printf("Server shutdown complete\n");
}
```

---

## 🧪 Como Testar

### 1. Compile o servidor com os novos arquivos:

Adicione ao projeto Visual Studio:
- `ServerMetrics.h`
- `ServerMetrics.cpp`
- `ItemMonitor.h`
- `ItemMonitor.cpp`
- `MetricsHTTP.h`
- `MetricsHTTP.cpp`

### 2. Execute o servidor:

```bash
DBSrv.exe
TMSrv.exe
```

Você deve ver:
```
Metrics HTTP Server started on port 8080
Dashboard: http://localhost:8080/
```

### 3. Acesse o Dashboard:

Abra no navegador:
```
http://localhost:8080/
```

OU abra diretamente o arquivo HTML:
```
Dashboard/index.html
```

E configure a API base para:
```javascript
const API_BASE = 'http://localhost:8080/api';
```

### 4. Faça operações no jogo:

- Entre no jogo
- Faça trades
- Drope e pegue itens
- Veja as métricas atualizando em tempo real!

### 5. Teste a detecção de dupes:

- Crie vários Laktolerium (item 415)
- Aguarde 5 minutos (varredura automática)
- Vá na aba "Alertas de Dupes"
- Deve aparecer um alerta de SPIKE!

---

## 🌐 Endpoints da API

```
GET http://localhost:8080/api/metrics
GET http://localhost:8080/api/items
GET http://localhost:8080/api/alerts
```

Teste direto no navegador ou com curl:
```bash
curl http://localhost:8080/api/metrics
curl http://localhost:8080/api/items
curl http://localhost:8080/api/alerts
```

---

## ⚙️ Configuração Avançada

### Mudar porta do servidor HTTP:

Em `Server.cpp`:
```cpp
MetricsHTTP::InitMetricsHTTP(9000);  // Porta 9000 em vez de 8080
```

### Intervalo de varredura de itens:

Em `Server.cpp`:
```cpp
// De 5 minutos para 1 minuto:
if (current_time - last_item_scan >= 60)  // Era 300
```

### Adicionar novos itens para monitorar:

Em `ItemMonitor.h`, no array `MONITORED_ITEMS`:
```cpp
{ 999, "Meu Item Raro", true, 10 },
```

Parâmetros:
- `999` = ID do item
- `"Meu Item Raro"` = Nome
- `true` = Gera alerta em aumento
- `10` = Threshold de delta para alerta

---

## 🎯 Resumo dos Arquivos

**Criados:**
- `Source/Code/TMSrv/ServerMetrics.h`
- `Source/Code/TMSrv/ServerMetrics.cpp`
- `Source/Code/TMSrv/ItemMonitor.h`
- `Source/Code/TMSrv/ItemMonitor.cpp`
- `Source/Code/TMSrv/MetricsHTTP.h`
- `Source/Code/TMSrv/MetricsHTTP.cpp`
- `Dashboard/index.html`

**Modificados:**
- `Source/Code/TMSrv/Server.cpp` (adicionar includes + loop)
- `Source/Code/TMSrv/_MSG_Trade.cpp` (adicionar métricas)
- `Source/Code/TMSrv/_MSG_DropItem.cpp` (adicionar métricas)
- `Source/Code/TMSrv/_MSG_GetItem.cpp` (adicionar métricas)
- `Source/Code/TMSrv/_MSG_SplitItem.cpp` (adicionar métricas)

---

**FASE 3 IMPLEMENTADA! 🎉**

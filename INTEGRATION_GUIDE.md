# 🔧 Guia de Integração - Sistema RedDragon ao Visual Studio

## 📋 Pré-requisitos

- Visual Studio 2015 ou superior
- MySQL Connector C++ 6.1
- Projeto TMSrv.vcxproj já configurado
- Código RedDragon já no diretório Source/Code/TMSrv/

---

## 🚀 Passo a Passo de Integração

### Passo 1: Adicionar Arquivo ao Projeto Visual Studio

#### Opção A: Via Interface Visual Studio
```
1. Abrir projeto TMSrv.sln no Visual Studio
2. No Solution Explorer, clicar com botão direito em "Source Files"
3. Add → Existing Item...
4. Navegar até: Source/Code/TMSrv/_MSG_CombineItemDragonForge.cpp
5. Selecionar e clicar "Add"
```

#### Opção B: Editar .vcxproj Diretamente
```xml
<!-- Abrir TMSrv.vcxproj em editor de texto -->
<!-- Localizar seção <ItemGroup> com outros _MSG_Combine*.cpp -->
<!-- Adicionar esta linha: -->

<ClCompile Include="Source\Code\TMSrv\_MSG_CombineItemDragonForge.cpp" />
```

---

### Passo 2: Adicionar Declaração no Header

**Arquivo**: `Source/Code/TMSrv/ProcessClientMessage.h`

Localizar onde estão declaradas outras funções `Exec_MSG_Combine*` e adicionar:

```cpp
// Existing declarations...
void Exec_MSG_CombineItemOdin(int conn, char* pMsg);
void Exec_MSG_CombineItemEhre(int conn, char* pMsg);
void Exec_MSG_CombineItemDedekinto(int conn, char* pMsg);

// ADD THIS LINE:
void Exec_MSG_CombineItemDragonForge(int conn, char* pMsg);
```

---

### Passo 3: Adicionar Message ID (Se Necessário)

**Arquivo**: `Source/Code/Basedef.h`

Se não existir um `MSG_DRAGONFORGE`, adicionar na seção de message defines:

```cpp
// Existing message defines...
#define MSG_COMBINEODIN     0x123  // Example existing ID
#define MSG_COMBINEEHRE     0x124  // Example existing ID

// ADD THIS (escolher um ID não utilizado):
#define MSG_DRAGONFORGE     0x1F5  // Exemplo: 0x1F5 = 501
```

**Como encontrar ID disponível**:
```cpp
// Pesquisar no código por "_MSG_" para ver IDs já usados
// Escolher um número que não conflite
// Recomendado: usar range 0x1F0-0x1FF para custom combines
```

---

### Passo 4: Adicionar ao Switch Case Principal

**Arquivo**: `Source/Code/TMSrv/ProcessClientMessage.cpp`

Localizar a função que processa mensagens do cliente (geralmente `ProcessClientMessage` ou similar) e adicionar:

```cpp
void ProcessClientMessage(int conn, char* pMsg)
{
    // ... código existente ...

    switch(msgType)
    {
        // ... cases existentes ...

        case MSG_COMBINEODIN:
            Exec_MSG_CombineItemOdin(conn, pMsg);
            break;

        case MSG_COMBINEEHRE:
            Exec_MSG_CombineItemEhre(conn, pMsg);
            break;

        // ADD THIS CASE:
        case MSG_DRAGONFORGE:
            Exec_MSG_CombineItemDragonForge(conn, pMsg);
            break;

        // ... outros cases ...
    }
}
```

---

### Passo 5: Configurar NPC para Usar o Sistema

**Opção A: Configuração via Banco de Dados**

```sql
-- Adicionar NPC Dragon Forge Master
INSERT INTO npcs (npc_id, name, x, y, map_id, function_type)
VALUES (9600, 'Mestre da Forja Dracônica', 2100, 1900, 1, 'DRAGONFORGE');

-- Configurar diálogo do NPC
INSERT INTO npc_dialogs (npc_id, dialog_text, action_type, action_param)
VALUES (
    9600,
    'Bem-vindo à Dragon Infusion! Traga um item Destruição +15 e os materiais necessários.',
    'OPEN_COMBINE',
    'MSG_DRAGONFORGE'
);
```

**Opção B: Configuração via Arquivo**

Criar/editar arquivo `Server/TMSrv/run/NPCConfig.txt`:

```
[NPC_9600]
Name=Mestre_da_Forja_Draconica
PosX=2100
PosY=1900
Map=Armia
FunctionType=COMBINE
CombineType=DRAGONFORGE
MessageID=0x1F5

Dialog1=Bem-vindo à Dragon Infusion!
Dialog2=Traga os seguintes materiais:
Dialog3=- Item Destruição +15
Dialog4=- 3x Escama RedDragon
Dialog5=- 1x Alma do Dragão
Dialog6=- 2x Chama Infernal
Dialog7=- 1x Essência Elemental
Dialog8=- 150.000.000 Gold
```

---

### Passo 6: Compilar o Projeto

#### No Visual Studio
```
1. Build → Rebuild Solution (ou Ctrl+Shift+B)
2. Verificar Output window para erros
3. Se houver erros de linking, verificar includes
```

#### Possíveis Erros e Soluções

**Erro 1**: `unresolved external symbol BASE_GetItemData`
```cpp
// Solução: Adicionar includes necessários
#include "ItemFunctions.h"  // ou arquivo que contém BASE_GetItemData
```

**Erro 2**: `'GetFirstEmptySlot' was not declared`
```cpp
// Solução: Implementar função helper ou usar função existente
// Procurar por funções similares no código como:
// - FindEmptySlot
// - GetEmptyCarrySlot
// - Etc.

// Ou implementar:
int GetFirstEmptySlot(int conn)
{
    for (int i = 0; i < pMob[conn].MaxCarry; i++)
    {
        if (pMob[conn].MOB.Carry[i].sIndex == 0)
            return i;
    }
    return -1;
}
```

**Erro 3**: `'BASE_SetItemEffect' is not a member of...`
```cpp
// Solução: Verificar como outros combines setam efeitos
// Geralmente algo como:
newItem.stEffect[slot].cEffect = EF_DAMAGE;
newItem.stEffect[slot].cValue = 72;
```

---

### Passo 7: Testar In-Game

#### Testes Básicos
```
1. Compilar servidor
2. Iniciar TMSrv.exe
3. Conectar com cliente
4. Usar comandos GM para testar:
```

**Comandos de Teste**:
```
/spawn 5752 10    // Spawnar 10x Alma do Dragão
/spawn 5753 30    // Spawnar 30x Escama RedDragon
/spawn 5754 20    // Spawnar 20x Chama Infernal
/spawn 5757 5     // Spawnar 5x Essência do Fogo
/spawn 1506 1     // Spawnar Armadura da Destruição

// Refinar item até +15
/refine 15        // (se comando existir)

// Adicionar gold
/gold 200000000   // 200M gold para testes
```

#### Fluxo de Teste Completo
```
1. Equipar item Destruição +15
2. Pegar materiais do inventário
3. Ir ao NPC Dragon Forge Master (ID 9600)
4. Abrir janela de combine
5. Colocar materiais nos slots:
   - Slot 1: Item Destruição +15
   - Slot 2-4: 3x Escama RedDragon
   - Slot 5: 1x Alma do Dragão
   - Slot 6-7: 2x Chama Infernal
   - Slot 8: 1x Essência Elemental
6. Confirmar crafting
7. Verificar resultado (65% chance sucesso)
```

**Logs para Debug**:
```cpp
// No código, já existem logs:
Combinelogsitem(pUser[conn].AccountName, pUser[conn].MacAddress, pUser[conn].IP,
               "success,DragonForge - RedDragon item created");

// Verificar logs em:
Server/TMSrv/logs/combines.txt  (ou similar)
```

---

### Passo 8: Verificações Finais

#### Checklist de Validação
```
[ ] Código compila sem erros
[ ] Servidor inicia sem crashes
[ ] NPC aparece in-game
[ ] Janela de combine abre
[ ] Validação de materiais funciona
[ ] Gold é debitado corretamente
[ ] Item é criado com stats corretos
[ ] Essência aplica adicional escolhido
[ ] Falha retorna item em +0
[ ] Logs são gravados corretamente
[ ] Não há memory leaks
[ ] Múltiplos players podem usar simultaneamente
```

---

## 🔧 Troubleshooting Avançado

### Problema: Servidor Crasha ao Abrir Combine

**Possível Causa**: Acesso a memória inválida

**Debug**:
```cpp
// Adicionar logs de debug:
void Exec_MSG_CombineItemDragonForge(int conn, char* pMsg)
{
    printf("[DEBUG] DragonForge called for conn=%d\n", conn);

    MSG_CombineItem* m = (MSG_CombineItem*)pMsg;
    printf("[DEBUG] Message parsed, size=%d\n", m->Size);

    // ... resto do código com mais logs ...
}
```

**Solução**: Verificar se `pMsg` não é NULL e size é válido

---

### Problema: Items Não Aparecem no Inventário

**Possível Causa**: ItemList.csv não foi recarregado

**Solução**:
```
1. Parar servidor
2. Verificar se ItemList.csv tem os items (IDs 5752-5771)
3. Deletar ItemList.bin (se existir)
4. Reiniciar servidor (vai recriar .bin a partir do .csv)
```

---

### Problema: Stats dos Items Estão Errados

**Possível Causa**: Ordem dos efeitos no ItemList.csv

**Verificação**:
```csv
# Verificar formato correto:
5762,Garra_RedDragon,35.0,170.232.0.0.200,45,180000,64,0,0,EF_CLASS,4,EF_GRID,0,EF_DAMAGE,385...

# Formato:
ID, Nome, Weight, Mesh.Texture.0.0.0, Level, Price, Slot, ?, ?, EF_*, Value, ...
```

---

### Problema: Essência Não Aplica Adicional

**Debug**:
```cpp
// No código, adicionar log:
void ApplyEssenceBonus(STRUCT_ITEM* item, int essenceID, bool isWeapon)
{
    printf("[DEBUG] Applying essence %d to item %d (weapon=%d)\n",
           essenceID, item->sIndex, isWeapon);

    if (isWeapon)
    {
        switch(essenceID)
        {
            case ESSENCE_FIRE_ID:
                printf("[DEBUG] Adding +72 damage\n");
                BASE_SetItemEffect(item, EF_DAMAGE, 72);
                break;
            // ...
        }
    }
}
```

---

## 📝 Configurações Adicionais

### Ajustar Taxa de Sucesso

No arquivo `Source/Code/TMSrv/_MSG_CombineItemDragonForge.cpp`:

```cpp
// Linha ~250, modificar:
#define INFUSION_SUCCESS_RATE   65  // Padrão: 65%

// Para aumentar chance:
#define INFUSION_SUCCESS_RATE   80  // 80% chance

// Para diminuir:
#define INFUSION_SUCCESS_RATE   50  // 50% chance
```

### Ajustar Custo de Gold

```cpp
// Linha ~15, modificar:
#define INFUSION_COST           150000000  // 150M gold

// Para baratear:
#define INFUSION_COST           50000000   // 50M gold

// Para encarecer:
#define INFUSION_COST           300000000  // 300M gold
```

### Modificar Materiais Necessários

No código, modificar função `ValidateInfusionMaterials`:

```cpp
// Exemplo: reduzir de 3 para 2 escamas
if (scaleCount >= 2)  // Original: >= 3
    hasScale = true;

// Exemplo: remover necessidade de Chama Infernal
// Comentar/remover verificação:
// else if (itemID == INFERNAL_FLAME_ID)
// {
//     flameCount++;
//     if (flameCount >= 2)
//         hasFlame = true;
// }
```

---

## 🎓 Dicas de Performance

### Otimização 1: Cache de Item Data
```cpp
// Ao invés de carregar item data toda vez:
static STRUCT_ITEM g_RedDragonCache[20];
static bool g_CacheInitialized = false;

void InitRedDragonCache()
{
    if (!g_CacheInitialized)
    {
        for (int i = 0; i < 20; i++)
        {
            BASE_GetItemData(&g_RedDragonCache[i], 5752 + i);
        }
        g_CacheInitialized = true;
    }
}
```

### Otimização 2: Validação Rápida
```cpp
// Adicionar early returns:
if (pMob[conn].MOB.Status.Level < 380)
{
    SendClientMessage(conn, "Level mínimo: Celestial 380.");
    return; // Early return
}

if (pMob[conn].MOB.Coin < INFUSION_COST)
{
    SendClientMessage(conn, "Gold insuficiente.");
    return; // Early return
}
```

---

## 📊 Monitoramento Pós-Deploy

### Logs a Monitorar
```
Server/TMSrv/logs/combines.txt       - Logs de crafting
Server/TMSrv/logs/errors.txt         - Erros críticos
Server/TMSrv/logs/duplication.txt    - Tentativas de dup
Server/DBSrv/logs/transactions.txt   - Transações de gold
```

### Métricas Importantes
```sql
-- Quantos items RedDragon foram criados
SELECT COUNT(*) FROM items WHERE item_id BETWEEN 5762 AND 5771;

-- Taxa de sucesso real
SELECT
    COUNT(CASE WHEN result='success' THEN 1 END) as sucessos,
    COUNT(CASE WHEN result='fail' THEN 1 END) as falhas,
    ROUND(COUNT(CASE WHEN result='success' THEN 1 END) * 100.0 / COUNT(*), 2) as taxa_sucesso
FROM combine_logs
WHERE combine_type = 'DragonForge';

-- Materiais mais usados
SELECT essence_type, COUNT(*) as usos
FROM combine_logs
WHERE combine_type = 'DragonForge' AND result='success'
GROUP BY essence_type
ORDER BY usos DESC;
```

---

## 🎯 Próximos Passos Após Integração

1. **Testar Extensivamente**
   - 50+ crafts de teste
   - Verificar todos os tipos de essências
   - Testar com diferentes items Destruição

2. **Beta Testing**
   - Selecionar 20-30 jogadores
   - Coletar feedback
   - Ajustar balanceamento

3. **Implementar Assets Visuais**
   - Adicionar modelos 3D
   - Texturas ao cliente
   - Efeitos de partículas

4. **Launch**
   - Anunciar oficialmente
   - Monitorar estabilidade
   - Estar pronto para hotfixes

---

**Versão**: 1.0
**Data**: 2025-01-31
**Status**: Guia Completo para Integração

🔧 **Ready to Build and Deploy!** 🔧

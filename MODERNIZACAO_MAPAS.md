# MODERNIZAÇÃO: SISTEMA DE MAPAS E CONFIGURAÇÃO

**Data:** 2025-12-13
**Branch:** claude/analyze-wyd-server-014nKEkMreWMYjH99kuRW2RP

---

## 🎯 PROBLEMA IDENTIFICADO

O sistema antigo de mapas do WYD apresenta várias limitações:

### ❌ Problemas do Sistema Antigo:

1. **Hardcoded e Limitado:**
   - Mapas definidos em `Regions.txt` com formato texto rígido
   - Spawns em `NPCGener.txt` com coordenadas manuais
   - Teleportes em `PotalPos.txt` sem validação
   - Difícil adicionar novas cidades/regiões

2. **Sem Validação:**
   - Nenhuma verificação de overlapping de regiões
   - Spawns podem ficar fora dos limites
   - Sem detecção de erros de configuração

3. **Difícil de Manter:**
   - Formato proprietário e não documentado
   - Precisa editar múltiplos arquivos para um mapa
   - Sem ferramentas de edição
   - Sem hot-reload (precisa reiniciar servidor)

4. **Sem Flexibilidade:**
   - Impossível definir propriedades por região (PvP, exp bonus, etc)
   - Sem sistema de templates reutilizáveis
   - Sem suporte a metadados (música, clima, descrição)

---

## ✨ SOLUÇÃO: SISTEMA MODERNIZADO

Implementei um sistema **completamente novo** baseado em JSON:

### Arquivos Criados:

1. **MapConfigSystem.h** (420 linhas)
   - Novas estruturas de dados
   - API moderna e type-safe
   - Suporte a templates

2. **MapConfigSystem.cpp** (670 linhas)
   - Carregamento/salvamento JSON
   - Conversão de formato antigo
   - Validação automática

3. **Tools/MapEditor.cpp** (350 linhas)
   - Ferramenta de linha de comando
   - Conversão, validação, edição

4. **Config/Maps/example_azran.json**
   - Exemplo completo de configuração
   - Demonstração de features

**Total:** ~1440 linhas de código

---

## 🗂️ NOVA ESTRUTURA DE DADOS

### MapRegion (Região/Mapa)

```cpp
struct MapRegion {
    int id;                          // ID único
    std::string name;                // Nome interno (azran_kingdom)
    std::string displayName;         // Nome exibido (Reino de Azran)
    RegionType type;                 // CITY, FIELD, DUNGEON, PVP, BOSS, EVENT
    BoundingBox bounds;              // Coordenadas (minX, minY, maxX, maxY)
    RegionProperties properties;     // Propriedades da região

    std::vector<SpawnPoint> spawns;      // Spawns de mobs
    std::vector<TeleportPoint> teleports; // Pontos de teleporte
    std::vector<NPCData> npcs;           // NPCs

    // Metadados
    std::string description;
    std::string backgroundMusic;
    std::string weatherEffect;
    int recommendedLevel;
};
```

### RegionProperties (Propriedades)

```cpp
struct RegionProperties {
    bool isPvPEnabled;        // PvP ativo?
    bool isSafeZone;          // Zona segura?
    bool canTeleportIn;       // Pode teleportar para cá?
    bool canTeleportOut;      // Pode teleportar daqui?
    bool canTrade;            // Pode trocar?
    bool canMount;            // Pode usar montaria?
    int minLevel;             // Nível mínimo
    int maxLevel;             // Nível máximo
    float expBonus;           // Multiplicador de EXP (1.0 = 100%)
    float dropBonus;          // Multiplicador de drop (1.0 = 100%)
};
```

### SpawnPoint (Spawn de Mob)

```cpp
struct SpawnPoint {
    int x, y;                 // Coordenadas centrais
    int radius;               // Raio de variação
    std::string mobName;      // Nome do mob
    int minGroup;             // Tamanho mínimo do grupo
    int maxGroup;             // Tamanho máximo do grupo
    int minuteRespawn;        // Tempo de respawn
};
```

### TeleportPoint (Portal)

```cpp
struct TeleportPoint {
    std::string name;         // Nome do portal
    int x, y;                 // Posição
    int targetRegionId;       // ID da região destino
    int targetX, targetY;     // Coordenadas destino
    int requiredLevel;        // Nível necessário
    int requiredGold;         // Gold necessário
    std::string requiredItem; // Item necessário
};
```

---

## 📝 FORMATO JSON

### Exemplo Completo:

```json
{
  "regions": [
    {
      "id": 1,
      "name": "azran_kingdom",
      "displayName": "Reino de Azran",
      "description": "Capital do reino humano",
      "type": "CITY",
      "recommendedLevel": 1,
      "backgroundMusic": "azran_theme.mp3",
      "weatherEffect": "sunny",
      "bounds": {
        "minX": 1788,
        "minY": 1538,
        "maxX": 2185,
        "maxY": 1785
      },
      "properties": {
        "isPvPEnabled": false,
        "isSafeZone": true,
        "canTeleportIn": true,
        "canTeleportOut": true,
        "canTrade": true,
        "canMount": true,
        "minLevel": 1,
        "maxLevel": 400,
        "expBonus": 1.0,
        "dropBonus": 1.0
      },
      "spawns": [],
      "teleports": [
        {
          "name": "Portal para Erion",
          "x": 1900,
          "y": 1650,
          "targetRegionId": 2,
          "targetX": 2500,
          "targetY": 2000,
          "requiredLevel": 1,
          "requiredGold": 0,
          "requiredItem": ""
        }
      ],
      "npcs": [
        {
          "name": "Mercador de Azran",
          "npcId": 1001,
          "x": 1920,
          "y": 1660,
          "type": "merchant",
          "dialogue": "Bem-vindo à minha loja!"
        }
      ]
    }
  ],
  "spawnTemplates": [
    {
      "templateName": "weak_mob_spawn",
      "mobName": "Orc",
      "minGroup": 2,
      "maxGroup": 4,
      "minuteRespawn": 3,
      "defaultRadius": 15
    }
  ]
}
```

---

## 🛠️ FERRAMENTA: MAP EDITOR

### Compilar:

```bash
cd Tools
g++ -std=c++17 MapEditor.cpp ../Source/Code/TMSrv/MapConfigSystem.cpp -o MapEditor.exe -I../Source/Code/TMSrv
```

### Comandos Disponíveis:

#### 1. Converter Arquivos Antigos

```bash
MapEditor.exe convert Regions.txt NPCGener.txt
```

**Resultado:** Cria `maps_converted.json` com todas as regiões convertidas

---

#### 2. Validar Configuração

```bash
MapEditor.exe validate maps.json
```

**Saída:**
```
Validando configuração...

✓ VALIDAÇÃO PASSOU!
Nenhum erro encontrado.
```

**Ou se houver erros:**
```
✗ VALIDAÇÃO FALHOU!
Encontrados 2 erros:

  - OVERLAP: Region azran_kingdom overlaps with erion
  - SPAWN OUT OF BOUNDS: Ciclope in azran_garden
```

---

#### 3. Gerar Relatório

```bash
MapEditor.exe report maps.json
```

**Saída:**
```
=== MAP CONFIGURATION REPORT ===

Total Regions: 15
Total Spawn Templates: 5

BY TYPE:
  CITY: 3
  FIELD: 7
  DUNGEON: 4
  PVP: 1

VALIDATION: PASSED
```

---

#### 4. Listar Regiões

```bash
MapEditor.exe list maps.json
```

**Saída:**
```
=== LISTA DE REGIÕES ===

ID   | Nome                    | Tipo       | Bounds
-----+-------------------------+------------+------------------------
1    | azran_kingdom           | CITY       | (1788,1538) -> (2185,1785)
2    | azran_garden            | FIELD      | (2180,1533) -> (2435,1787)
10   | dungeon_hydra           | DUNGEON    | (127,3710) -> (767,6841)
```

---

#### 5. Adicionar Nova Região (Interativo)

```bash
MapEditor.exe add-region maps.json
```

**Interação:**
```
=== ADICIONAR NOVA REGIÃO ===

ID da região: 50
Nome interno (ex: azran_castle): new_city
Nome para exibição (ex: Castelo de Azran): Nova Cidade
Descrição: Uma cidade recém descoberta
Tipo (CITY/FIELD/DUNGEON/PVP/BOSS/EVENT): CITY
Coordenadas:
  MinX: 3000
  MinY: 3000
  MaxX: 3500
  MaxY: 3500
Nível recomendado: 100
PvP ativo? (1=sim, 0=não): 0
Safe Zone? (1=sim, 0=não): 1

✓ Região adicionada com sucesso!
```

---

#### 6. Adicionar Spawn a Região

```bash
MapEditor.exe add-spawn maps.json 2
```

**Interação:**
```
=== ADICIONAR SPAWN À REGIÃO azran_garden ===

Nome do Mob: Dragon_Vermelho
Coordenadas:
  X: 2300
  Y: 1650
Raio de spawn: 25
Grupo mínimo: 1
Grupo máximo: 2
Tempo de respawn (minutos): 30

✓ Spawn adicionado com sucesso!
```

---

## 💻 INTEGRAÇÃO NO SERVIDOR

### 1. Inicialização do Servidor

Em `Server.cpp`:

```cpp
#include "MapConfigSystem.h"

void ServerInit() {
    // ... inicialização existente ...

    // Carrega configuração de mapas
    if (!MapConfig::g_MapConfig.LoadFromJSON("maps.json")) {
        // Fallback: tenta converter arquivos antigos
        MapConfig::g_MapConfig.ConvertLegacyFiles("Regions.txt", "NPCGener.txt");
        MapConfig::g_MapConfig.SaveToJSON("maps.json");
    }

    // Valida configuração
    auto errors = MapConfig::g_MapConfig.ValidateConfig();
    if (!errors.empty()) {
        for (const auto& error : errors) {
            printf("[MAP CONFIG ERROR] %s\n", error.c_str());
        }
    }
}
```

---

### 2. Consultas em Tempo Real

#### Verificar se Posição é PvP:

```cpp
void OnPlayerAttack(int attacker, int target) {
    int x = pMob[attacker].TargetX;
    int y = pMob[attacker].TargetY;

    // Verifica se PvP é permitido
    if (!MapConfig::g_MapConfig.IsPvPEnabled(x, y)) {
        SendClientMessage(attacker, "PvP não é permitido nesta área!");
        return;
    }

    // Continua com ataque
    // ...
}
```

#### Aplicar Bonus de EXP por Região:

```cpp
void GiveExperience(int conn, int base_exp) {
    int x = pMob[conn].TargetX;
    int y = pMob[conn].TargetY;

    // Obtém região
    MapConfig::MapRegion* region = MapConfig::g_MapConfig.FindRegionAt(x, y);

    if (region) {
        float bonus = region->properties.expBonus;
        int final_exp = (int)(base_exp * bonus);

        pMob[conn].MOB.Exp += final_exp;

        if (bonus > 1.0f) {
            SendClientMessage(conn, "Bonus de EXP de %.0f%% nesta região!",
                             (bonus - 1.0f) * 100.0f);
        }
    } else {
        pMob[conn].MOB.Exp += base_exp;
    }
}
```

#### Spawnar Mobs da Configuração:

```cpp
void SpawnMobsFromConfig() {
    // Para cada região
    for (int region_id = 0; region_id < 100; region_id++) {
        MapConfig::MapRegion* region = MapConfig::g_MapConfig.GetRegion(region_id);

        if (!region) continue;

        // Para cada spawn definido
        for (const auto& spawn : region->spawns) {
            // Cria mob
            int mob_index = CreateMob(spawn.mobName.c_str());

            if (mob_index >= 0) {
                // Posição aleatória dentro do raio
                int offset_x = (rand() % (spawn.radius * 2)) - spawn.radius;
                int offset_y = (rand() % (spawn.radius * 2)) - spawn.radius;

                pMob[mob_index].TargetX = spawn.x + offset_x;
                pMob[mob_index].TargetY = spawn.y + offset_y;

                pMob[mob_index].RespawnTime = spawn.minuteRespawn;
            }
        }
    }
}
```

#### Teleporte via Portal:

```cpp
void OnPlayerUseTeleport(int conn, int portal_x, int portal_y) {
    MapConfig::MapRegion* region = MapConfig::g_MapConfig.FindRegionAt(portal_x, portal_y);

    if (!region) return;

    // Procura teleporte próximo
    for (const auto& tp : region->teleports) {
        int dist = Distance(portal_x, portal_y, tp.x, tp.y);

        if (dist <= 10) {  // Dentro do raio do portal
            // Verifica requisitos
            if (pMob[conn].MOB.Level < tp.requiredLevel) {
                SendClientMessage(conn, "Você precisa de nível %d!", tp.requiredLevel);
                return;
            }

            if (pMob[conn].MOB.Gold < tp.requiredGold) {
                SendClientMessage(conn, "Você precisa de %d gold!", tp.requiredGold);
                return;
            }

            // Teleporta
            pMob[conn].TargetX = tp.targetX;
            pMob[conn].TargetY = tp.targetY;
            pMob[conn].MOB.Gold -= tp.requiredGold;

            SendClientMessage(conn, "Teleportando para %s...", tp.name.c_str());
            TeleportPlayer(conn, tp.targetX, tp.targetY);
            break;
        }
    }
}
```

---

### 3. Hot-Reload (Sem Reiniciar Servidor)

```cpp
void OnGMCommand(int conn, const char* command) {
    if (strcmp(command, "/reloadmaps") == 0) {
        // Recarrega configuração de mapas
        if (MapConfig::g_MapConfig.Reload()) {
            SendClientMessage(conn, "Configuração de mapas recarregada!");
        } else {
            SendClientMessage(conn, "Falha ao recarregar mapas!");
        }
    }
}
```

---

## 🎨 COMO ADICIONAR UM NOVO MAPA/CIDADE

### Método 1: Editar JSON Manualmente

1. Abra `Config/Maps/maps.json`
2. Adicione nova região ao array `regions`:

```json
{
  "id": 100,
  "name": "new_kingdom",
  "displayName": "Novo Reino",
  "description": "Um reino recém descoberto",
  "type": "CITY",
  "recommendedLevel": 150,
  "backgroundMusic": "new_kingdom.mp3",
  "weatherEffect": "rain",
  "bounds": {
    "minX": 4000,
    "minY": 4000,
    "maxX": 4500,
    "maxY": 4500
  },
  "properties": {
    "isPvPEnabled": false,
    "isSafeZone": true,
    "expBonus": 1.5,
    "dropBonus": 1.2
  },
  "spawns": [],
  "teleports": [],
  "npcs": []
}
```

3. Salve e valide:
```bash
MapEditor.exe validate maps.json
```

4. Recarregue no servidor:
```
/reloadmaps
```

---

### Método 2: Usar Map Editor (Interativo)

```bash
MapEditor.exe add-region maps.json
```

Siga as instruções interativas!

---

### Método 3: Programaticamente

```cpp
void CreateNewRegion() {
    MapConfig::MapRegion newRegion;
    newRegion.id = 100;
    newRegion.name = "new_kingdom";
    newRegion.displayName = "Novo Reino";
    newRegion.type = MapConfig::RegionType::CITY;
    newRegion.bounds = MapConfig::BoundingBox(4000, 4000, 4500, 4500);
    newRegion.recommendedLevel = 150;

    newRegion.properties.isPvPEnabled = false;
    newRegion.properties.isSafeZone = true;
    newRegion.properties.expBonus = 1.5f;
    newRegion.properties.dropBonus = 1.2f;

    MapConfig::g_MapConfig.AddRegion(newRegion);
    MapConfig::g_MapConfig.SaveToJSON("maps.json");
}
```

---

## 🔍 VALIDAÇÃO AUTOMÁTICA

O sistema detecta automaticamente:

### 1. Overlapping de Regiões

```
OVERLAP: Region azran_kingdom overlaps with azran_garden
```

**Fix:** Ajustar coordenadas para não sobrepor

---

### 2. Coordenadas Inválidas

```
INVALID BOUNDS: new_city
```

**Fix:** Garantir que maxX > minX e maxY > minY

---

### 3. Spawns Fora dos Limites

```
SPAWN OUT OF BOUNDS: Dragon_Vermelho in azran_garden
```

**Fix:** Ajustar coordenadas do spawn ou expandir bounds da região

---

## ⚙️ COMPATIBILIDADE

### Retrocompatibilidade:

O sistema **mantém compatibilidade** com arquivos antigos:

```cpp
// Se maps.json não existir, converte automaticamente
if (!MapConfig::g_MapConfig.LoadFromJSON("maps.json")) {
    MapConfig::g_MapConfig.ConvertLegacyFiles("Regions.txt", "NPCGener.txt");
    MapConfig::g_MapConfig.SaveToJSON("maps.json");
}
```

### Migração Gradual:

Você pode:
1. Continuar usando `Regions.txt` temporariamente
2. Converter para JSON quando quiser
3. Editar JSON e voltar ao formato antigo se necessário

---

## 📊 BENEFÍCIOS

### ✅ Facilidade:
- Adicionar novos mapas em **minutos** (vs horas)
- Interface JSON legível e editável
- Ferramentas de validação automática

### ✅ Flexibilidade:
- Propriedades por região (PvP, bonuses, etc)
- Templates reutilizáveis
- Hot-reload sem reiniciar

### ✅ Segurança:
- Validação automática de overlaps
- Detecção de erros antes de rodar
- Type-safe C++ API

### ✅ Produtividade:
- Map Editor interativo
- Conversão automática de formato antigo
- Relatórios detalhados

---

## 📋 CHECKLIST DE MIGRAÇÃO

- [ ] Compilar MapEditor.exe
- [ ] Converter arquivos antigos: `MapEditor.exe convert Regions.txt NPCGener.txt`
- [ ] Validar resultado: `MapEditor.exe validate maps_converted.json`
- [ ] Revisar e ajustar JSON manualmente
- [ ] Integrar MapConfigSystem no servidor
- [ ] Testar hot-reload
- [ ] Adicionar primeiro mapa novo!

---

## 🎯 PRÓXIMOS PASSOS

### Sistema de Interface (Futuro):

- Editor visual de mapas (GUI)
- Sistema de UI configurável (XML/JSON)
- Hot-reload de interface do cliente
- Template system para janelas

**Ver:** `INTERFACE_MODERNIZACAO.md` (a criar)

---

**SISTEMA DE MAPAS MODERNIZADO! 🎉**

Agora é fácil adicionar novos mapas, cidades e dungeons!

#ifndef MAP_CONFIG_SYSTEM_H
#define MAP_CONFIG_SYSTEM_H

#include <string>
#include <vector>
#include <unordered_map>
#include <nlohmann/json.hpp>  // JSON library (https://github.com/nlohmann/json)

// MODERNIZA��O: Sistema de Configura��o de Mapas Baseado em JSON
// Facilita adi��o de novos mapas, cidades, e zonas

using json = nlohmann::json;

namespace MapConfig {

// ============================================================================
// ESTRUTURAS MODERNIZADAS
// ============================================================================

// Bounding Box (ret�ngulo de �rea)
struct BoundingBox {
    int minX;
    int minY;
    int maxX;
    int maxY;

    BoundingBox() : minX(0), minY(0), maxX(0), maxY(0) {}
    BoundingBox(int x1, int y1, int x2, int y2) : minX(x1), minY(y1), maxX(x2), maxY(y2) {}

    bool Contains(int x, int y) const {
        return (x >= minX && x <= maxX && y >= minY && y <= maxY);
    }

    int GetWidth() const { return maxX - minX; }
    int GetHeight() const { return maxY - minY; }
    int GetCenterX() const { return (minX + maxX) / 2; }
    int GetCenterY() const { return (minY + maxY) / 2; }
};

// Tipo de regi�o
enum class RegionType {
    CITY,           // Cidade (safe zone)
    FIELD,          // Campo aberto
    DUNGEON,        // Dungeon/Masmorra
    PVP,            // �rea PvP
    BOSS,           // �rea de boss
    EVENT,          // �rea de evento
    NEUTRAL,        // Zona neutra
    GUILD_WAR       // Guerra de guildas
};

// Propriedades de regi�o (flags)
struct RegionProperties {
    bool isPvPEnabled;
    bool isRespawnEnabled;
    bool isSafeZone;
    bool canTeleportIn;
    bool canTeleportOut;
    bool canTrade;
    bool canMount;
    int  minLevel;
    int  maxLevel;
    float expBonus;          // Multiplicador de EXP (1.0 = 100%)
    float dropBonus;         // Multiplicador de drop (1.0 = 100%)

    RegionProperties()
        : isPvPEnabled(false), isRespawnEnabled(true), isSafeZone(false),
          canTeleportIn(true), canTeleportOut(true), canTrade(true),
          canMount(true), minLevel(1), maxLevel(400), expBonus(1.0f), dropBonus(1.0f) {}
};

// Ponto de Spawn
struct SpawnPoint {
    int x;
    int y;
    int radius;              // Raio de varia��o
    std::string mobName;
    int minGroup;
    int maxGroup;
    int minuteRespawn;

    SpawnPoint() : x(0), y(0), radius(10), minGroup(1), maxGroup(1), minuteRespawn(5) {}
};

// Ponto de Teleporte (Portal)
struct TeleportPoint {
    std::string name;
    int x;
    int y;
    int targetRegionId;      // ID da regi�o de destino
    int targetX;
    int targetY;
    int requiredLevel;
    int requiredGold;
    std::string requiredItem;

    TeleportPoint()
        : x(0), y(0), targetRegionId(0), targetX(0), targetY(0),
          requiredLevel(1), requiredGold(0) {}
};

// NPC
struct NPCData {
    std::string name;
    int npcId;
    int x;
    int y;
    std::string type;        // "merchant", "quest", "teleporter", etc
    std::string dialogue;

    NPCData() : npcId(0), x(0), y(0) {}
};

// Regi�o/Mapa completo
struct MapRegion {
    int id;
    std::string name;
    std::string displayName;
    RegionType type;
    BoundingBox bounds;
    RegionProperties properties;

    std::vector<SpawnPoint> spawns;
    std::vector<TeleportPoint> teleports;
    std::vector<NPCData> npcs;

    // Metadados
    std::string description;
    std::string backgroundMusic;
    std::string weatherEffect;
    int recommendedLevel;

    MapRegion() : id(0), type(RegionType::FIELD), recommendedLevel(1) {}
};

// Template de Spawn (reutiliz�vel)
struct SpawnTemplate {
    std::string templateName;
    std::string mobName;
    int minGroup;
    int maxGroup;
    int minuteRespawn;
    int defaultRadius;

    SpawnTemplate() : minGroup(1), maxGroup(1), minuteRespawn(5), defaultRadius(10) {}
};

// ============================================================================
// CLASSE DE GERENCIAMENTO
// ============================================================================

class MapConfigManager {
private:
    std::unordered_map<int, MapRegion> regions;
    std::unordered_map<std::string, SpawnTemplate> spawnTemplates;
    std::string config_path;

    // Parsing JSON
    MapRegion ParseRegionFromJSON(const json& j);
    SpawnTemplate ParseTemplateFromJSON(const json& j);

    // Serializa��o JSON
    json RegionToJSON(const MapRegion& region);
    json TemplateToJSON(const SpawnTemplate& templ);

    // Convers�o de formato antigo
    void ConvertOldRegionsFile(const std::string& old_file_path);
    void ConvertOldNPCGenerFile(const std::string& old_file_path);

public:
    MapConfigManager(const std::string& config_dir = "Config/Maps/");
    ~MapConfigManager();

    // ========================================================================
    // CARREGAR/SALVAR
    // ========================================================================

    // Carrega todas as regi�es de arquivo JSON
    bool LoadFromJSON(const std::string& filename);

    // Salva todas as regi�es em JSON
    bool SaveToJSON(const std::string& filename);

    // Hot-reload (recarrega sem reiniciar servidor)
    bool Reload();

    // Converte arquivos antigos para JSON
    bool ConvertLegacyFiles(const std::string& regions_txt, const std::string& npcgener_txt);

    // ========================================================================
    // CONSULTAS
    // ========================================================================

    // Obt�m regi�o por ID
    MapRegion* GetRegion(int region_id);

    // Obt�m regi�o por nome
    MapRegion* GetRegionByName(const std::string& name);

    // Encontra regi�o que cont�m ponto (x, y)
    MapRegion* FindRegionAt(int x, int y);

    // Lista todas as regi�es de um tipo
    std::vector<MapRegion*> GetRegionsByType(RegionType type);

    // Verifica se posi��o � v�lida
    bool IsValidPosition(int x, int y);

    // Verifica se PvP � permitido em posi��o
    bool IsPvPEnabled(int x, int y);

    // ========================================================================
    // EDI��O
    // ========================================================================

    // Adiciona nova regi�o
    void AddRegion(const MapRegion& region);

    // Remove regi�o
    void RemoveRegion(int region_id);

    // Atualiza regi�o
    void UpdateRegion(int region_id, const MapRegion& region);

    // Adiciona spawn a regi�o
    void AddSpawnToRegion(int region_id, const SpawnPoint& spawn);

    // Adiciona teleporte a regi�o
    void AddTeleportToRegion(int region_id, const TeleportPoint& teleport);

    // ========================================================================
    // TEMPLATES
    // ========================================================================

    // Carrega template de spawn
    SpawnTemplate* GetSpawnTemplate(const std::string& template_name);

    // Adiciona template
    void AddSpawnTemplate(const SpawnTemplate& templ);

    // Cria spawn a partir de template
    SpawnPoint CreateSpawnFromTemplate(const std::string& template_name, int x, int y);

    // ========================================================================
    // VALIDA��O
    // ========================================================================

    // Valida configura��o (detecta overlaps, coordenadas inv�lidas, etc)
    std::vector<std::string> ValidateConfig();

    // Relat�rio de estat�sticas
    std::string GenerateReport();
};

// Inst�ncia global
extern MapConfigManager g_MapConfig;

// ============================================================================
// HELPERS
// ============================================================================

// Converte string para RegionType
RegionType StringToRegionType(const std::string& str);

// Converte RegionType para string
std::string RegionTypeToString(RegionType type);

} // namespace MapConfig

#endif // MAP_CONFIG_SYSTEM_H

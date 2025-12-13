#include "MapConfigSystem.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

namespace MapConfig {

// Inst�ncia global
MapConfigManager g_MapConfig;

// ============================================================================
// CONSTRUTOR / DESTRUTOR
// ============================================================================

MapConfigManager::MapConfigManager(const std::string& config_dir)
    : config_path(config_dir) {
    // Cria diret�rio se n�o existir
    // CreateDirectory(config_dir.c_str(), NULL);  // Windows
}

MapConfigManager::~MapConfigManager() {
    // Cleanup autom�tico
}

// ============================================================================
// HELPERS
// ============================================================================

RegionType StringToRegionType(const std::string& str) {
    if (str == "CITY") return RegionType::CITY;
    if (str == "FIELD") return RegionType::FIELD;
    if (str == "DUNGEON") return RegionType::DUNGEON;
    if (str == "PVP") return RegionType::PVP;
    if (str == "BOSS") return RegionType::BOSS;
    if (str == "EVENT") return RegionType::EVENT;
    if (str == "NEUTRAL") return RegionType::NEUTRAL;
    if (str == "GUILD_WAR") return RegionType::GUILD_WAR;
    return RegionType::FIELD;
}

std::string RegionTypeToString(RegionType type) {
    switch (type) {
        case RegionType::CITY: return "CITY";
        case RegionType::FIELD: return "FIELD";
        case RegionType::DUNGEON: return "DUNGEON";
        case RegionType::PVP: return "PVP";
        case RegionType::BOSS: return "BOSS";
        case RegionType::EVENT: return "EVENT";
        case RegionType::NEUTRAL: return "NEUTRAL";
        case RegionType::GUILD_WAR: return "GUILD_WAR";
        default: return "FIELD";
    }
}

// ============================================================================
// PARSING JSON
// ============================================================================

MapRegion MapConfigManager::ParseRegionFromJSON(const json& j) {
    MapRegion region;

    region.id = j.value("id", 0);
    region.name = j.value("name", "");
    region.displayName = j.value("displayName", "");
    region.description = j.value("description", "");
    region.type = StringToRegionType(j.value("type", "FIELD"));
    region.recommendedLevel = j.value("recommendedLevel", 1);
    region.backgroundMusic = j.value("backgroundMusic", "");
    region.weatherEffect = j.value("weatherEffect", "");

    // Bounds
    if (j.contains("bounds")) {
        auto& bounds = j["bounds"];
        region.bounds.minX = bounds.value("minX", 0);
        region.bounds.minY = bounds.value("minY", 0);
        region.bounds.maxX = bounds.value("maxX", 0);
        region.bounds.maxY = bounds.value("maxY", 0);
    }

    // Properties
    if (j.contains("properties")) {
        auto& props = j["properties"];
        region.properties.isPvPEnabled = props.value("isPvPEnabled", false);
        region.properties.isRespawnEnabled = props.value("isRespawnEnabled", true);
        region.properties.isSafeZone = props.value("isSafeZone", false);
        region.properties.canTeleportIn = props.value("canTeleportIn", true);
        region.properties.canTeleportOut = props.value("canTeleportOut", true);
        region.properties.canTrade = props.value("canTrade", true);
        region.properties.canMount = props.value("canMount", true);
        region.properties.minLevel = props.value("minLevel", 1);
        region.properties.maxLevel = props.value("maxLevel", 400);
        region.properties.expBonus = props.value("expBonus", 1.0f);
        region.properties.dropBonus = props.value("dropBonus", 1.0f);
    }

    // Spawns
    if (j.contains("spawns")) {
        for (const auto& spawn_json : j["spawns"]) {
            SpawnPoint spawn;
            spawn.x = spawn_json.value("x", 0);
            spawn.y = spawn_json.value("y", 0);
            spawn.radius = spawn_json.value("radius", 10);
            spawn.mobName = spawn_json.value("mobName", "");
            spawn.minGroup = spawn_json.value("minGroup", 1);
            spawn.maxGroup = spawn_json.value("maxGroup", 1);
            spawn.minuteRespawn = spawn_json.value("minuteRespawn", 5);
            region.spawns.push_back(spawn);
        }
    }

    // Teleports
    if (j.contains("teleports")) {
        for (const auto& tp_json : j["teleports"]) {
            TeleportPoint tp;
            tp.name = tp_json.value("name", "");
            tp.x = tp_json.value("x", 0);
            tp.y = tp_json.value("y", 0);
            tp.targetRegionId = tp_json.value("targetRegionId", 0);
            tp.targetX = tp_json.value("targetX", 0);
            tp.targetY = tp_json.value("targetY", 0);
            tp.requiredLevel = tp_json.value("requiredLevel", 1);
            tp.requiredGold = tp_json.value("requiredGold", 0);
            tp.requiredItem = tp_json.value("requiredItem", "");
            region.teleports.push_back(tp);
        }
    }

    // NPCs
    if (j.contains("npcs")) {
        for (const auto& npc_json : j["npcs"]) {
            NPCData npc;
            npc.name = npc_json.value("name", "");
            npc.npcId = npc_json.value("npcId", 0);
            npc.x = npc_json.value("x", 0);
            npc.y = npc_json.value("y", 0);
            npc.type = npc_json.value("type", "merchant");
            npc.dialogue = npc_json.value("dialogue", "");
            region.npcs.push_back(npc);
        }
    }

    return region;
}

json MapConfigManager::RegionToJSON(const MapRegion& region) {
    json j;

    j["id"] = region.id;
    j["name"] = region.name;
    j["displayName"] = region.displayName;
    j["description"] = region.description;
    j["type"] = RegionTypeToString(region.type);
    j["recommendedLevel"] = region.recommendedLevel;
    j["backgroundMusic"] = region.backgroundMusic;
    j["weatherEffect"] = region.weatherEffect;

    // Bounds
    j["bounds"] = {
        {"minX", region.bounds.minX},
        {"minY", region.bounds.minY},
        {"maxX", region.bounds.maxX},
        {"maxY", region.bounds.maxY}
    };

    // Properties
    j["properties"] = {
        {"isPvPEnabled", region.properties.isPvPEnabled},
        {"isRespawnEnabled", region.properties.isRespawnEnabled},
        {"isSafeZone", region.properties.isSafeZone},
        {"canTeleportIn", region.properties.canTeleportIn},
        {"canTeleportOut", region.properties.canTeleportOut},
        {"canTrade", region.properties.canTrade},
        {"canMount", region.properties.canMount},
        {"minLevel", region.properties.minLevel},
        {"maxLevel", region.properties.maxLevel},
        {"expBonus", region.properties.expBonus},
        {"dropBonus", region.properties.dropBonus}
    };

    // Spawns
    j["spawns"] = json::array();
    for (const auto& spawn : region.spawns) {
        json spawn_json = {
            {"x", spawn.x},
            {"y", spawn.y},
            {"radius", spawn.radius},
            {"mobName", spawn.mobName},
            {"minGroup", spawn.minGroup},
            {"maxGroup", spawn.maxGroup},
            {"minuteRespawn", spawn.minuteRespawn}
        };
        j["spawns"].push_back(spawn_json);
    }

    // Teleports
    j["teleports"] = json::array();
    for (const auto& tp : region.teleports) {
        json tp_json = {
            {"name", tp.name},
            {"x", tp.x},
            {"y", tp.y},
            {"targetRegionId", tp.targetRegionId},
            {"targetX", tp.targetX},
            {"targetY", tp.targetY},
            {"requiredLevel", tp.requiredLevel},
            {"requiredGold", tp.requiredGold},
            {"requiredItem", tp.requiredItem}
        };
        j["teleports"].push_back(tp_json);
    }

    // NPCs
    j["npcs"] = json::array();
    for (const auto& npc : region.npcs) {
        json npc_json = {
            {"name", npc.name},
            {"npcId", npc.npcId},
            {"x", npc.x},
            {"y", npc.y},
            {"type", npc.type},
            {"dialogue", npc.dialogue}
        };
        j["npcs"].push_back(npc_json);
    }

    return j;
}

// ============================================================================
// CARREGAR/SALVAR
// ============================================================================

bool MapConfigManager::LoadFromJSON(const std::string& filename) {
    std::ifstream file(config_path + filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open: " << filename << std::endl;
        return false;
    }

    try {
        json j;
        file >> j;

        // Carrega regi�es
        if (j.contains("regions")) {
            for (const auto& region_json : j["regions"]) {
                MapRegion region = ParseRegionFromJSON(region_json);
                regions[region.id] = region;
            }
        }

        // Carrega templates
        if (j.contains("spawnTemplates")) {
            for (const auto& template_json : j["spawnTemplates"]) {
                SpawnTemplate templ = ParseTemplateFromJSON(template_json);
                spawnTemplates[templ.templateName] = templ;
            }
        }

        std::cout << "Loaded " << regions.size() << " regions and "
                  << spawnTemplates.size() << " spawn templates" << std::endl;

        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return false;
    }
}

bool MapConfigManager::SaveToJSON(const std::string& filename) {
    json j;

    // Serializa regi�es
    j["regions"] = json::array();
    for (const auto& pair : regions) {
        j["regions"].push_back(RegionToJSON(pair.second));
    }

    // Serializa templates
    j["spawnTemplates"] = json::array();
    for (const auto& pair : spawnTemplates) {
        j["spawnTemplates"].push_back(TemplateToJSON(pair.second));
    }

    // Salva arquivo
    std::ofstream file(config_path + filename);
    if (!file.is_open()) {
        std::cerr << "Failed to create: " << filename << std::endl;
        return false;
    }

    file << j.dump(4);  // Pretty-print com indenta��o de 4 espa�os
    file.close();

    std::cout << "Saved " << regions.size() << " regions to " << filename << std::endl;
    return true;
}

bool MapConfigManager::Reload() {
    regions.clear();
    spawnTemplates.clear();
    return LoadFromJSON("maps.json");
}

// ============================================================================
// CONVERS�O DE FORMATO ANTIGO
// ============================================================================

void MapConfigManager::ConvertOldRegionsFile(const std::string& old_file_path) {
    FILE* fp = fopen(old_file_path.c_str(), "r");
    if (!fp) {
        std::cerr << "Failed to open old Regions.txt" << std::endl;
        return;
    }

    char line[1024];
    int region_id = 0;

    while (fgets(line, sizeof(line), fp)) {
        int minX, minY, maxX, maxY;
        char regionName[128];

        // Formato antigo: 129, 137, 253, 243 = Guerra_de_Cidades
        if (sscanf(line, "%d, %d, %d, %d = %127s", &minX, &minY, &maxX, &maxY, regionName) == 5) {
            MapRegion region;
            region.id = region_id++;
            region.name = regionName;
            region.displayName = regionName;
            region.bounds = BoundingBox(minX, minY, maxX, maxY);
            region.type = RegionType::FIELD;  // Assumir field por padr�o

            // Detectar tipo por nome
            std::string name_lower = regionName;
            std::transform(name_lower.begin(), name_lower.end(), name_lower.begin(), ::tolower);

            if (name_lower.find("city") != std::string::npos ||
                name_lower.find("cidade") != std::string::npos) {
                region.type = RegionType::CITY;
                region.properties.isSafeZone = true;
                region.properties.isPvPEnabled = false;
            }
            else if (name_lower.find("dungeon") != std::string::npos) {
                region.type = RegionType::DUNGEON;
            }
            else if (name_lower.find("guerra") != std::string::npos ||
                     name_lower.find("war") != std::string::npos) {
                region.type = RegionType::GUILD_WAR;
                region.properties.isPvPEnabled = true;
            }

            regions[region.id] = region;
        }
    }

    fclose(fp);

    std::cout << "Converted " << regions.size() << " regions from old format" << std::endl;
}

bool MapConfigManager::ConvertLegacyFiles(const std::string& regions_txt, const std::string& npcgener_txt) {
    // Converte Regions.txt
    ConvertOldRegionsFile(regions_txt);

    // TODO: Implementar convers�o de NPCGener.txt

    // Salva em formato novo
    return SaveToJSON("maps_converted.json");
}

// ============================================================================
// CONSULTAS
// ============================================================================

MapRegion* MapConfigManager::GetRegion(int region_id) {
    auto it = regions.find(region_id);
    if (it != regions.end()) {
        return &it->second;
    }
    return nullptr;
}

MapRegion* MapConfigManager::GetRegionByName(const std::string& name) {
    for (auto& pair : regions) {
        if (pair.second.name == name) {
            return &pair.second;
        }
    }
    return nullptr;
}

MapRegion* MapConfigManager::FindRegionAt(int x, int y) {
    for (auto& pair : regions) {
        if (pair.second.bounds.Contains(x, y)) {
            return &pair.second;
        }
    }
    return nullptr;
}

std::vector<MapRegion*> MapConfigManager::GetRegionsByType(RegionType type) {
    std::vector<MapRegion*> result;

    for (auto& pair : regions) {
        if (pair.second.type == type) {
            result.push_back(&pair.second);
        }
    }

    return result;
}

bool MapConfigManager::IsValidPosition(int x, int y) {
    return FindRegionAt(x, y) != nullptr;
}

bool MapConfigManager::IsPvPEnabled(int x, int y) {
    MapRegion* region = FindRegionAt(x, y);
    return region && region->properties.isPvPEnabled;
}

// ============================================================================
// EDI��O
// ============================================================================

void MapConfigManager::AddRegion(const MapRegion& region) {
    regions[region.id] = region;
}

void MapConfigManager::RemoveRegion(int region_id) {
    regions.erase(region_id);
}

void MapConfigManager::UpdateRegion(int region_id, const MapRegion& region) {
    regions[region_id] = region;
}

void MapConfigManager::AddSpawnToRegion(int region_id, const SpawnPoint& spawn) {
    auto it = regions.find(region_id);
    if (it != regions.end()) {
        it->second.spawns.push_back(spawn);
    }
}

void MapConfigManager::AddTeleportToRegion(int region_id, const TeleportPoint& teleport) {
    auto it = regions.find(region_id);
    if (it != regions.end()) {
        it->second.teleports.push_back(teleport);
    }
}

// ============================================================================
// TEMPLATES
// ============================================================================

SpawnTemplate MapConfigManager::ParseTemplateFromJSON(const json& j) {
    SpawnTemplate templ;
    templ.templateName = j.value("templateName", "");
    templ.mobName = j.value("mobName", "");
    templ.minGroup = j.value("minGroup", 1);
    templ.maxGroup = j.value("maxGroup", 1);
    templ.minuteRespawn = j.value("minuteRespawn", 5);
    templ.defaultRadius = j.value("defaultRadius", 10);
    return templ;
}

json MapConfigManager::TemplateToJSON(const SpawnTemplate& templ) {
    return {
        {"templateName", templ.templateName},
        {"mobName", templ.mobName},
        {"minGroup", templ.minGroup},
        {"maxGroup", templ.maxGroup},
        {"minuteRespawn", templ.minuteRespawn},
        {"defaultRadius", templ.defaultRadius}
    };
}

SpawnTemplate* MapConfigManager::GetSpawnTemplate(const std::string& template_name) {
    auto it = spawnTemplates.find(template_name);
    if (it != spawnTemplates.end()) {
        return &it->second;
    }
    return nullptr;
}

void MapConfigManager::AddSpawnTemplate(const SpawnTemplate& templ) {
    spawnTemplates[templ.templateName] = templ;
}

SpawnPoint MapConfigManager::CreateSpawnFromTemplate(const std::string& template_name, int x, int y) {
    SpawnPoint spawn;
    SpawnTemplate* templ = GetSpawnTemplate(template_name);

    if (templ) {
        spawn.x = x;
        spawn.y = y;
        spawn.radius = templ->defaultRadius;
        spawn.mobName = templ->mobName;
        spawn.minGroup = templ->minGroup;
        spawn.maxGroup = templ->maxGroup;
        spawn.minuteRespawn = templ->minuteRespawn;
    }

    return spawn;
}

// ============================================================================
// VALIDA��O
// ============================================================================

std::vector<std::string> MapConfigManager::ValidateConfig() {
    std::vector<std::string> errors;

    // Verifica overlaps
    for (const auto& pair1 : regions) {
        for (const auto& pair2 : regions) {
            if (pair1.first >= pair2.first) continue;

            const auto& r1 = pair1.second.bounds;
            const auto& r2 = pair2.second.bounds;

            // Verifica interse��o
            bool overlap = !(r2.minX > r1.maxX || r2.maxX < r1.minX ||
                            r2.minY > r1.maxY || r2.maxY < r1.minY);

            if (overlap) {
                std::string error = "OVERLAP: Region " + pair1.second.name +
                                   " overlaps with " + pair2.second.name;
                errors.push_back(error);
            }
        }
    }

    // Verifica coordenadas inv�lidas
    for (const auto& pair : regions) {
        const auto& region = pair.second;

        if (region.bounds.minX >= region.bounds.maxX ||
            region.bounds.minY >= region.bounds.maxY) {
            errors.push_back("INVALID BOUNDS: " + region.name);
        }

        // Verifica spawns fora dos limites
        for (const auto& spawn : region.spawns) {
            if (!region.bounds.Contains(spawn.x, spawn.y)) {
                errors.push_back("SPAWN OUT OF BOUNDS: " + spawn.mobName +
                               " in " + region.name);
            }
        }
    }

    return errors;
}

std::string MapConfigManager::GenerateReport() {
    std::ostringstream report;

    report << "=== MAP CONFIGURATION REPORT ===\n\n";
    report << "Total Regions: " << regions.size() << "\n";
    report << "Total Spawn Templates: " << spawnTemplates.size() << "\n\n";

    // Por tipo
    std::unordered_map<RegionType, int> type_count;
    for (const auto& pair : regions) {
        type_count[pair.second.type]++;
    }

    report << "BY TYPE:\n";
    for (const auto& pair : type_count) {
        report << "  " << RegionTypeToString(pair.first) << ": " << pair.second << "\n";
    }

    report << "\n";

    // Valida��o
    auto errors = ValidateConfig();
    if (errors.empty()) {
        report << "VALIDATION: PASSED\n";
    } else {
        report << "VALIDATION: " << errors.size() << " ERRORS\n";
        for (const auto& error : errors) {
            report << "  - " << error << "\n";
        }
    }

    return report.str();
}

} // namespace MapConfig

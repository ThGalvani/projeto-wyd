// MAP EDITOR TOOL - Ferramenta de Edi��o de Mapas WYD
// Uso: MapEditor.exe [command] [options]

#include "../Source/Code/TMSrv/MapConfigSystem.h"
#include <iostream>
#include <string>
#include <vector>

using namespace MapConfig;

void PrintHelp() {
    std::cout << "\n=== WYD MAP EDITOR TOOL ===\n\n";
    std::cout << "COMANDOS:\n\n";
    std::cout << "  convert <regions.txt> <npcgener.txt>  - Converte arquivos antigos para JSON\n";
    std::cout << "  validate <maps.json>                  - Valida configura��o\n";
    std::cout << "  report <maps.json>                    - Gera relat�rio de mapas\n";
    std::cout << "  add-region <maps.json>                - Adiciona nova regi�o (interativo)\n";
    std::cout << "  add-spawn <maps.json> <region_id>     - Adiciona spawn a regi�o\n";
    std::cout << "  list <maps.json>                      - Lista todas as regi�es\n";
    std::cout << "  export-old <maps.json>                - Exporta para formato antigo\n";
    std::cout << "  help                                  - Mostra esta ajuda\n\n";
    std::cout << "EXEMPLOS:\n\n";
    std::cout << "  MapEditor.exe convert Regions.txt NPCGener.txt\n";
    std::cout << "  MapEditor.exe validate maps.json\n";
    std::cout << "  MapEditor.exe add-region maps.json\n";
    std::cout << "  MapEditor.exe list maps.json\n\n";
}

void CommandConvert(const std::vector<std::string>& args) {
    if (args.size() < 3) {
        std::cerr << "Uso: convert <regions.txt> <npcgener.txt>\n";
        return;
    }

    MapConfigManager manager("./");

    std::cout << "Convertendo arquivos antigos...\n";

    if (manager.ConvertLegacyFiles(args[1], args[2])) {
        std::cout << "Convers�o conclu�da! Arquivo gerado: maps_converted.json\n";
        std::cout << "\nVOC� PODE AGORA:\n";
        std::cout << "  1. Revisar o arquivo maps_converted.json\n";
        std::cout << "  2. Validar com: MapEditor.exe validate maps_converted.json\n";
        std::cout << "  3. Editar manualmente ou usar comandos add-*\n";
    } else {
        std::cerr << "Falha na convers�o!\n";
    }
}

void CommandValidate(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cerr << "Uso: validate <maps.json>\n";
        return;
    }

    MapConfigManager manager("./");

    std::cout << "Carregando " << args[1] << "...\n";

    if (!manager.LoadFromJSON(args[1])) {
        std::cerr << "Falha ao carregar arquivo!\n";
        return;
    }

    std::cout << "Validando configura��o...\n\n";

    auto errors = manager.ValidateConfig();

    if (errors.empty()) {
        std::cout << "✓ VALIDA��O PASSOU!\n";
        std::cout << "Nenhum erro encontrado.\n";
    } else {
        std::cout << "✗ VALIDA��O FALHOU!\n";
        std::cout << "Encontrados " << errors.size() << " erros:\n\n";

        for (const auto& error : errors) {
            std::cout << "  - " << error << "\n";
        }
    }
}

void CommandReport(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cerr << "Uso: report <maps.json>\n";
        return;
    }

    MapConfigManager manager("./");

    if (!manager.LoadFromJSON(args[1])) {
        std::cerr << "Falha ao carregar arquivo!\n";
        return;
    }

    std::cout << manager.GenerateReport() << "\n";
}

void CommandList(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cerr << "Uso: list <maps.json>\n";
        return;
    }

    MapConfigManager manager("./");

    if (!manager.LoadFromJSON(args[1])) {
        std::cerr << "Falha ao carregar arquivo!\n";
        return;
    }

    std::cout << "\n=== LISTA DE REGI�ES ===\n\n";
    std::cout << "ID   | Nome                    | Tipo       | Bounds\n";
    std::cout << "-----+--------------------------+------------+------------------------\n";

    // Listar todas as regi�es (implementar itera��o)
    for (int i = 0; i < 100; i++) {  // Assumir max 100 regi�es
        MapRegion* region = manager.GetRegion(i);
        if (region) {
            printf("%-4d | %-23s | %-10s | (%d,%d) -> (%d,%d)\n",
                region->id,
                region->name.c_str(),
                RegionTypeToString(region->type).c_str(),
                region->bounds.minX,
                region->bounds.minY,
                region->bounds.maxX,
                region->bounds.maxY
            );
        }
    }

    std::cout << "\n";
}

void CommandAddRegion(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cerr << "Uso: add-region <maps.json>\n";
        return;
    }

    MapConfigManager manager("./");

    if (!manager.LoadFromJSON(args[1])) {
        std::cerr << "Falha ao carregar arquivo!\n";
        return;
    }

    std::cout << "\n=== ADICIONAR NOVA REGI�O ===\n\n";

    MapRegion region;

    std::cout << "ID da regi�o: ";
    std::cin >> region.id;
    std::cin.ignore();

    std::cout << "Nome interno (ex: azran_castle): ";
    std::getline(std::cin, region.name);

    std::cout << "Nome para exibi��o (ex: Castelo de Azran): ";
    std::getline(std::cin, region.displayName);

    std::cout << "Descri��o: ";
    std::getline(std::cin, region.description);

    std::cout << "Tipo (CITY/FIELD/DUNGEON/PVP/BOSS/EVENT): ";
    std::string type_str;
    std::cin >> type_str;
    region.type = StringToRegionType(type_str);

    std::cout << "Coordenadas:\n";
    std::cout << "  MinX: ";
    std::cin >> region.bounds.minX;
    std::cout << "  MinY: ";
    std::cin >> region.bounds.minY;
    std::cout << "  MaxX: ";
    std::cin >> region.bounds.maxX;
    std::cout << "  MaxY: ";
    std::cin >> region.bounds.maxY;

    std::cout << "N�vel recomendado: ";
    std::cin >> region.recommendedLevel;

    std::cout << "PvP ativo? (1=sim, 0=n�o): ";
    int pvp;
    std::cin >> pvp;
    region.properties.isPvPEnabled = (pvp == 1);

    std::cout << "Safe Zone? (1=sim, 0=n�o): ";
    int safe;
    std::cin >> safe;
    region.properties.isSafeZone = (safe == 1);

    // Adiciona regi�o
    manager.AddRegion(region);

    // Salva
    if (manager.SaveToJSON(args[1])) {
        std::cout << "\n✓ Regi�o adicionada com sucesso!\n";
    } else {
        std::cerr << "\n✗ Falha ao salvar arquivo!\n";
    }
}

void CommandAddSpawn(const std::vector<std::string>& args) {
    if (args.size() < 3) {
        std::cerr << "Uso: add-spawn <maps.json> <region_id>\n";
        return;
    }

    MapConfigManager manager("./");

    if (!manager.LoadFromJSON(args[1])) {
        std::cerr << "Falha ao carregar arquivo!\n";
        return;
    }

    int region_id = std::stoi(args[2]);

    MapRegion* region = manager.GetRegion(region_id);
    if (!region) {
        std::cerr << "Regi�o " << region_id << " n�o encontrada!\n";
        return;
    }

    std::cout << "\n=== ADICIONAR SPAWN � REGI�O " << region->name << " ===\n\n";

    SpawnPoint spawn;

    std::cout << "Nome do Mob: ";
    std::cin.ignore();
    std::getline(std::cin, spawn.mobName);

    std::cout << "Coordenadas:\n";
    std::cout << "  X: ";
    std::cin >> spawn.x;
    std::cout << "  Y: ";
    std::cin >> spawn.y;

    std::cout << "Raio de spawn: ";
    std::cin >> spawn.radius;

    std::cout << "Grupo m�nimo: ";
    std::cin >> spawn.minGroup;

    std::cout << "Grupo m�ximo: ";
    std::cin >> spawn.maxGroup;

    std::cout << "Tempo de respawn (minutos): ";
    std::cin >> spawn.minuteRespawn;

    // Valida coordenadas
    if (!region->bounds.Contains(spawn.x, spawn.y)) {
        std::cerr << "\nAVISO: Spawn fora dos limites da regi�o!\n";
        std::cout << "Continuar mesmo assim? (1=sim, 0=n�o): ";
        int cont;
        std::cin >> cont;
        if (cont == 0) {
            std::cout << "Opera��o cancelada.\n";
            return;
        }
    }

    // Adiciona spawn
    manager.AddSpawnToRegion(region_id, spawn);

    // Salva
    if (manager.SaveToJSON(args[1])) {
        std::cout << "\n✓ Spawn adicionado com sucesso!\n";
    } else {
        std::cerr << "\n✗ Falha ao salvar arquivo!\n";
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        PrintHelp();
        return 0;
    }

    std::string command = argv[1];
    std::vector<std::string> args;

    for (int i = 0; i < argc; i++) {
        args.push_back(argv[i]);
    }

    if (command == "convert") {
        CommandConvert(args);
    }
    else if (command == "validate") {
        CommandValidate(args);
    }
    else if (command == "report") {
        CommandReport(args);
    }
    else if (command == "list") {
        CommandList(args);
    }
    else if (command == "add-region") {
        CommandAddRegion(args);
    }
    else if (command == "add-spawn") {
        CommandAddSpawn(args);
    }
    else if (command == "help") {
        PrintHelp();
    }
    else {
        std::cerr << "Comando desconhecido: " << command << "\n";
        PrintHelp();
        return 1;
    }

    return 0;
}

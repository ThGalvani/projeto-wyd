#include "LoadBalancer.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <random>

namespace Scalability {

// Inst�ncias globais
LoadBalancer g_LoadBalancer;
SessionManager g_SessionManager;

// ============================================================================
// LOAD BALANCER - IMPLEMENTA��O
// ============================================================================

LoadBalancer::LoadBalancer(BalancingPolicy default_policy)
    : policy(default_policy), round_robin_index(0) {
    servers.reserve(MAX_SERVERS);
}

LoadBalancer::~LoadBalancer() {
    // Cleanup
}

// ============================================================================
// FUN��ES AUXILIARES
// ============================================================================

int LoadBalancer::SelectServerRoundRobin() {
    std::lock_guard<std::mutex> lock(balancer_mutex);

    int attempts = 0;
    int start_index = round_robin_index.load();

    while (attempts < servers.size()) {
        int index = (start_index + attempts) % servers.size();

        if (IsServerAvailable(servers[index])) {
            round_robin_index.store((index + 1) % servers.size());
            return servers[index].server_id;
        }

        attempts++;
    }

    return -1;  // Nenhum servidor dispon�vel
}

int LoadBalancer::SelectServerLeastLoaded() {
    std::lock_guard<std::mutex> lock(balancer_mutex);

    int best_server = -1;
    int min_load = INT_MAX;

    for (const auto& server : servers) {
        if (IsServerAvailable(server)) {
            if (server.current_players < min_load) {
                min_load = server.current_players;
                best_server = server.server_id;
            }
        }
    }

    return best_server;
}

int LoadBalancer::SelectServerRandom() {
    std::lock_guard<std::mutex> lock(balancer_mutex);

    std::vector<int> available_servers;

    for (const auto& server : servers) {
        if (IsServerAvailable(server)) {
            available_servers.push_back(server.server_id);
        }
    }

    if (available_servers.empty()) {
        return -1;
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, available_servers.size() - 1);

    return available_servers[dis(gen)];
}

int LoadBalancer::SelectServerByChannel(int preferred_channel) {
    std::lock_guard<std::mutex> lock(balancer_mutex);

    int best_server = -1;
    int min_load = INT_MAX;

    // Tenta encontrar servidor no canal preferido
    for (const auto& server : servers) {
        if (server.channel == preferred_channel && IsServerAvailable(server)) {
            if (server.current_players < min_load) {
                min_load = server.current_players;
                best_server = server.server_id;
            }
        }
    }

    // Se n�o encontrou no canal preferido, usa qualquer um
    if (best_server == -1) {
        best_server = SelectServerLeastLoaded();
    }

    return best_server;
}

bool LoadBalancer::IsServerAvailable(const ServerInfo& server) const {
    return (server.status == ServerStatus::ONLINE &&
            server.current_players < server.max_players);
}

// ============================================================================
// GERENCIAMENTO DE SERVIDORES
// ============================================================================

void LoadBalancer::RegisterServer(int server_id, const std::string& host, int port, int channel) {
    std::lock_guard<std::mutex> lock(balancer_mutex);

    ServerInfo info;
    info.server_id = server_id;
    info.host = host;
    info.port = port;
    info.channel = channel;
    info.status = ServerStatus::ONLINE;
    info.current_players = 0;
    info.max_players = MAX_PLAYERS_PER_SERVER;
    info.last_heartbeat = time(nullptr);

    servers.push_back(info);
}

void LoadBalancer::UnregisterServer(int server_id) {
    std::lock_guard<std::mutex> lock(balancer_mutex);

    servers.erase(
        std::remove_if(servers.begin(), servers.end(),
            [server_id](const ServerInfo& s) { return s.server_id == server_id; }),
        servers.end()
    );
}

void LoadBalancer::UpdateServerStatus(int server_id, int player_count, float cpu, float memory) {
    std::lock_guard<std::mutex> lock(balancer_mutex);

    for (auto& server : servers) {
        if (server.server_id == server_id) {
            server.current_players = player_count;
            server.cpu_usage = cpu;
            server.memory_usage = memory;
            server.last_heartbeat = time(nullptr);

            // Auto-detecta sobrecarga
            if (player_count >= server.max_players * 0.9f || cpu >= 90.0f) {
                server.status = ServerStatus::OVERLOADED;
            } else if (server.status == ServerStatus::OVERLOADED) {
                server.status = ServerStatus::ONLINE;
            }

            break;
        }
    }
}

void LoadBalancer::SetServerStatus(int server_id, ServerStatus status) {
    std::lock_guard<std::mutex> lock(balancer_mutex);

    for (auto& server : servers) {
        if (server.server_id == server_id) {
            server.status = status;
            break;
        }
    }
}

ServerInfo LoadBalancer::GetServerInfo(int server_id) const {
    std::lock_guard<std::mutex> lock(balancer_mutex);

    for (const auto& server : servers) {
        if (server.server_id == server_id) {
            return server;
        }
    }

    return ServerInfo();
}

std::vector<ServerInfo> LoadBalancer::GetAllServers() const {
    std::lock_guard<std::mutex> lock(balancer_mutex);
    return servers;
}

// ============================================================================
// BALANCEAMENTO DE CARGA
// ============================================================================

int LoadBalancer::SelectBestServer(int preferred_channel) {
    switch (policy) {
        case BalancingPolicy::ROUND_ROBIN:
            return SelectServerRoundRobin();

        case BalancingPolicy::LEAST_LOADED:
            return SelectServerLeastLoaded();

        case BalancingPolicy::RANDOM:
            return SelectServerRandom();

        case BalancingPolicy::CHANNEL_BASED:
            return SelectServerByChannel(preferred_channel);

        default:
            return SelectServerLeastLoaded();
    }
}

bool LoadBalancer::AssignPlayer(int player_id, int server_id, int channel) {
    std::lock_guard<std::mutex> lock(balancer_mutex);

    // Verifica se servidor existe e est� dispon�vel
    bool found = false;
    for (auto& server : servers) {
        if (server.server_id == server_id) {
            if (IsServerAvailable(server)) {
                server.current_players++;

                // Cria sess�o
                PlayerSession session;
                session.player_id = player_id;
                session.assigned_server = server_id;
                session.channel = channel;
                session.login_time = time(nullptr);
                session.is_migrating = false;

                active_sessions[player_id] = session;

                found = true;
            }
            break;
        }
    }

    return found;
}

void LoadBalancer::RemovePlayer(int player_id) {
    std::lock_guard<std::mutex> lock(balancer_mutex);

    auto it = active_sessions.find(player_id);
    if (it != active_sessions.end()) {
        int server_id = it->second.assigned_server;

        // Decrementa contador do servidor
        for (auto& server : servers) {
            if (server.server_id == server_id) {
                if (server.current_players > 0) {
                    server.current_players--;
                }
                break;
            }
        }

        active_sessions.erase(it);
    }
}

bool LoadBalancer::MigratePlayer(int player_id, int target_server) {
    std::lock_guard<std::mutex> lock(balancer_mutex);

    auto it = active_sessions.find(player_id);
    if (it == active_sessions.end()) {
        return false;  // Player n�o encontrado
    }

    int old_server = it->second.assigned_server;

    // Verifica se servidor alvo est� dispon�vel
    bool target_available = false;
    for (const auto& server : servers) {
        if (server.server_id == target_server && IsServerAvailable(server)) {
            target_available = true;
            break;
        }
    }

    if (!target_available) {
        return false;
    }

    // Atualiza contadores
    for (auto& server : servers) {
        if (server.server_id == old_server && server.current_players > 0) {
            server.current_players--;
        } else if (server.server_id == target_server) {
            server.current_players++;
        }
    }

    // Atualiza sess�o
    it->second.assigned_server = target_server;
    it->second.is_migrating = true;

    return true;
}

int LoadBalancer::GetPlayerServer(int player_id) const {
    std::lock_guard<std::mutex> lock(balancer_mutex);

    auto it = active_sessions.find(player_id);
    if (it != active_sessions.end()) {
        return it->second.assigned_server;
    }

    return -1;
}

bool LoadBalancer::IsPlayerOnline(int player_id) const {
    std::lock_guard<std::mutex> lock(balancer_mutex);
    return active_sessions.find(player_id) != active_sessions.end();
}

// ============================================================================
// HEALTH CHECK
// ============================================================================

void LoadBalancer::CheckServerHealth() {
    MarkTimeoutServers();
}

void LoadBalancer::MarkTimeoutServers() {
    std::lock_guard<std::mutex> lock(balancer_mutex);

    time_t now = time(nullptr);

    for (auto& server : servers) {
        if (server.status == ServerStatus::ONLINE ||
            server.status == ServerStatus::OVERLOADED) {

            time_t elapsed = now - server.last_heartbeat;

            if (elapsed > SERVER_TIMEOUT) {
                server.status = ServerStatus::OFFLINE;
            }
        }
    }
}

// ============================================================================
// POL�TICA DE BALANCEAMENTO
// ============================================================================

void LoadBalancer::SetBalancingPolicy(BalancingPolicy new_policy) {
    policy = new_policy;
}

// ============================================================================
// ESTAT�STICAS
// ============================================================================

int LoadBalancer::GetTotalPlayers() const {
    std::lock_guard<std::mutex> lock(balancer_mutex);
    return active_sessions.size();
}

int LoadBalancer::GetOnlineServerCount() const {
    std::lock_guard<std::mutex> lock(balancer_mutex);

    int count = 0;
    for (const auto& server : servers) {
        if (server.status == ServerStatus::ONLINE) {
            count++;
        }
    }

    return count;
}

int LoadBalancer::GetMostLoadedServer() const {
    std::lock_guard<std::mutex> lock(balancer_mutex);

    int max_load = -1;
    int server_id = -1;

    for (const auto& server : servers) {
        if (server.current_players > max_load) {
            max_load = server.current_players;
            server_id = server.server_id;
        }
    }

    return server_id;
}

int LoadBalancer::GetLeastLoadedServer() const {
    std::lock_guard<std::mutex> lock(balancer_mutex);

    int min_load = INT_MAX;
    int server_id = -1;

    for (const auto& server : servers) {
        if (server.status == ServerStatus::ONLINE &&
            server.current_players < min_load) {
            min_load = server.current_players;
            server_id = server.server_id;
        }
    }

    return server_id;
}

std::vector<float> LoadBalancer::GetLoadDistribution() const {
    std::lock_guard<std::mutex> lock(balancer_mutex);

    std::vector<float> distribution;

    for (const auto& server : servers) {
        float load = (float)server.current_players / (float)server.max_players * 100.0f;
        distribution.push_back(load);
    }

    return distribution;
}

std::string LoadBalancer::GenerateReport() const {
    std::ostringstream report;

    report << "=== RELAT�RIO DE LOAD BALANCING ===\n\n";

    std::lock_guard<std::mutex> lock(balancer_mutex);

    report << "Total de Players: " << active_sessions.size() << "\n";
    report << "Servidores Online: " << GetOnlineServerCount() << "/" << servers.size() << "\n\n";

    report << "SERVIDORES:\n";
    report << std::left << std::setw(5) << "ID"
           << std::setw(20) << "Host:Port"
           << std::setw(8) << "Canal"
           << std::setw(12) << "Status"
           << std::setw(12) << "Players"
           << std::setw(10) << "CPU"
           << std::setw(10) << "Mem" << "\n";
    report << std::string(80, '-') << "\n";

    for (const auto& server : servers) {
        std::string status_str;
        switch (server.status) {
            case ServerStatus::ONLINE: status_str = "ONLINE"; break;
            case ServerStatus::OFFLINE: status_str = "OFFLINE"; break;
            case ServerStatus::MAINTENANCE: status_str = "MAINT"; break;
            case ServerStatus::OVERLOADED: status_str = "OVERLOAD"; break;
        }

        std::string host_port = server.host + ":" + std::to_string(server.port);
        std::string players = std::to_string(server.current_players) + "/" +
                              std::to_string(server.max_players);

        report << std::left << std::setw(5) << server.server_id
               << std::setw(20) << host_port
               << std::setw(8) << server.channel
               << std::setw(12) << status_str
               << std::setw(12) << players
               << std::setw(10) << std::fixed << std::setprecision(1) << server.cpu_usage << "%"
               << std::setw(10) << std::fixed << std::setprecision(1) << server.memory_usage << "%" << "\n";
    }

    return report.str();
}

// ============================================================================
// SESSION MANAGER - IMPLEMENTA��O
// ============================================================================

SessionManager::SessionManager() {
    // Inicializa��o
}

SessionManager::~SessionManager() {
    // Cleanup
}

void SessionManager::CreateSession(int player_id, int server_id, int channel) {
    std::lock_guard<std::mutex> lock(session_mutex);

    PlayerSession session;
    session.player_id = player_id;
    session.assigned_server = server_id;
    session.channel = channel;
    session.login_time = time(nullptr);
    session.is_migrating = false;

    global_sessions[player_id] = session;
}

void SessionManager::RemoveSession(int player_id) {
    std::lock_guard<std::mutex> lock(session_mutex);
    global_sessions.erase(player_id);
}

PlayerSession* SessionManager::GetSession(int player_id) {
    std::lock_guard<std::mutex> lock(session_mutex);

    auto it = global_sessions.find(player_id);
    if (it != global_sessions.end()) {
        return &it->second;
    }

    return nullptr;
}

bool SessionManager::IsPlayerOnline(int player_id) const {
    std::lock_guard<std::mutex> lock(session_mutex);
    return global_sessions.find(player_id) != global_sessions.end();
}

void SessionManager::MigrateSession(int player_id, int new_server) {
    std::lock_guard<std::mutex> lock(session_mutex);

    auto it = global_sessions.find(player_id);
    if (it != global_sessions.end()) {
        it->second.assigned_server = new_server;
        it->second.is_migrating = true;
    }
}

} // namespace Scalability

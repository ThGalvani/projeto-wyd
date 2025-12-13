#ifndef LOAD_BALANCER_H
#define LOAD_BALANCER_H

#include <vector>
#include <string>
#include <mutex>
#include <atomic>
#include <unordered_map>
#include <memory>

// FASE 8: Load Balancing para M�ltiplos TMSrv
// Sistema de m�ltiplos servidores de jogo com balanceamento de carga

namespace Scalability {

// ============================================================================
// CONSTANTES
// ============================================================================

const int MAX_SERVERS = 16;                   // M�ximo de TMSrv
const int MAX_PLAYERS_PER_SERVER = 1000;      // Capacidade por servidor
const int HEALTH_CHECK_INTERVAL = 5;          // Segundos entre health checks
const int SERVER_TIMEOUT = 30;                // Segundos para considerar offline

// ============================================================================
// ESTRUTURAS
// ============================================================================

// Status do servidor
enum class ServerStatus {
    ONLINE,          // Servidor ativo
    OFFLINE,         // Servidor offline
    MAINTENANCE,     // Em manuten��o
    OVERLOADED       // Sobrecarga
};

// Informa��es de servidor
struct ServerInfo {
    int server_id;
    std::string host;
    int port;
    ServerStatus status;
    int current_players;
    int max_players;
    float cpu_usage;
    float memory_usage;
    time_t last_heartbeat;
    int channel;  // Canal do servidor (1, 2, 3, etc)

    ServerInfo()
        : server_id(0), port(0), status(ServerStatus::OFFLINE),
          current_players(0), max_players(MAX_PLAYERS_PER_SERVER),
          cpu_usage(0.0f), memory_usage(0.0f), last_heartbeat(0), channel(1) {}
};

// Pol�tica de balanceamento
enum class BalancingPolicy {
    ROUND_ROBIN,         // Rotativo
    LEAST_LOADED,        // Menos carregado
    RANDOM,              // Aleat�rio
    CHANNEL_BASED        // Baseado em canal
};

// Sess�o de player
struct PlayerSession {
    int player_id;
    int assigned_server;
    int channel;
    time_t login_time;
    bool is_migrating;

    PlayerSession()
        : player_id(0), assigned_server(0), channel(1),
          login_time(0), is_migrating(false) {}
};

// ============================================================================
// CLASSE LOAD BALANCER
// ============================================================================

class LoadBalancer {
private:
    mutable std::mutex balancer_mutex;
    std::vector<ServerInfo> servers;
    std::unordered_map<int, PlayerSession> active_sessions;  // player_id -> session

    BalancingPolicy policy;
    std::atomic<int> round_robin_index;

    // Fun��es auxiliares
    int SelectServerRoundRobin();
    int SelectServerLeastLoaded();
    int SelectServerRandom();
    int SelectServerByChannel(int preferred_channel);
    bool IsServerAvailable(const ServerInfo& server) const;

public:
    LoadBalancer(BalancingPolicy default_policy = BalancingPolicy::LEAST_LOADED);
    ~LoadBalancer();

    // ========================================================================
    // GERENCIAMENTO DE SERVIDORES
    // ========================================================================

    // Registra novo servidor
    void RegisterServer(int server_id, const std::string& host, int port, int channel = 1);

    // Remove servidor
    void UnregisterServer(int server_id);

    // Atualiza status do servidor (heartbeat)
    void UpdateServerStatus(int server_id, int player_count, float cpu, float memory);

    // Marca servidor como offline/online
    void SetServerStatus(int server_id, ServerStatus status);

    // Obt�m informa��es do servidor
    ServerInfo GetServerInfo(int server_id) const;

    // Lista todos os servidores
    std::vector<ServerInfo> GetAllServers() const;

    // ========================================================================
    // BALANCEAMENTO DE CARGA
    // ========================================================================

    // Seleciona melhor servidor para novo player
    int SelectBestServer(int preferred_channel = 1);

    // Atribui player a servidor
    bool AssignPlayer(int player_id, int server_id, int channel = 1);

    // Remove player de servidor
    void RemovePlayer(int player_id);

    // Migra player para outro servidor
    bool MigratePlayer(int player_id, int target_server);

    // Obt�m servidor do player
    int GetPlayerServer(int player_id) const;

    // Verifica se player est� online
    bool IsPlayerOnline(int player_id) const;

    // ========================================================================
    // HEALTH CHECK
    // ========================================================================

    // Verifica servidores offline (chamar periodicamente)
    void CheckServerHealth();

    // Marca servidores sem heartbeat como offline
    void MarkTimeoutServers();

    // ========================================================================
    // POL�TICA DE BALANCEAMENTO
    // ========================================================================

    // Define pol�tica
    void SetBalancingPolicy(BalancingPolicy new_policy);

    // Obt�m pol�tica atual
    BalancingPolicy GetBalancingPolicy() const { return policy; }

    // ========================================================================
    // ESTAT�STICAS
    // ========================================================================

    // Total de players online
    int GetTotalPlayers() const;

    // N�mero de servidores online
    int GetOnlineServerCount() const;

    // Servidor mais carregado
    int GetMostLoadedServer() const;

    // Servidor menos carregado
    int GetLeastLoadedServer() const;

    // Distribui��o de carga
    std::vector<float> GetLoadDistribution() const;

    // Relat�rio
    std::string GenerateReport() const;
};

// ============================================================================
// SESSION MANAGER (COMPARTILHADO ENTRE SERVIDORES)
// ============================================================================

class SessionManager {
private:
    std::mutex session_mutex;
    std::unordered_map<int, PlayerSession> global_sessions;

public:
    SessionManager();
    ~SessionManager();

    // Cria sess�o global
    void CreateSession(int player_id, int server_id, int channel);

    // Remove sess�o
    void RemoveSession(int player_id);

    // Obt�m sess�o
    PlayerSession* GetSession(int player_id);

    // Verifica se player est� online globalmente
    bool IsPlayerOnline(int player_id) const;

    // Migra sess�o
    void MigrateSession(int player_id, int new_server);
};

// ============================================================================
// GERENCIADOR GLOBAL
// ============================================================================

// Inst�ncias globais
extern LoadBalancer g_LoadBalancer;
extern SessionManager g_SessionManager;

} // namespace Scalability

#endif // LOAD_BALANCER_H

#ifndef _ANTICHEAT_H_
#define _ANTICHEAT_H_

#include <mutex>
#include <map>
#include <ctime>
#include <cmath>

//==============================================================================
// FASE 4 - Sistema Anti-Cheat
//
// Detecta e previne:
// - Speed Hack (movimento muito rápido)
// - Teleport Hack (movimento impossível)
// - No-Clip (atravessar paredes)
//==============================================================================

namespace AntiCheat {

// Configuração
const float MAX_SPEED_TILES_PER_SECOND = 12.0f;  // Velocidade máxima permitida
const int SUSPICIOUS_MOVES_THRESHOLD = 5;         // Violações para kick
const int BAN_MOVES_THRESHOLD = 10;               // Violações para ban
const int VIOLATION_RESET_TIME = 60;              // Reseta violações após 60s

// Estrutura de movimento do player
struct PlayerMovement {
	int last_x;
	int last_y;
	time_t last_move_time;
	int suspicious_moves;
	int total_violations;
	double avg_speed;
	bool is_banned;
	time_t ban_time;
};

// Sistema de monitoramento de velocidade
class SpeedMonitor {
private:
	std::mutex monitor_mutex;
	std::map<int, PlayerMovement> player_data;

	// Calcula distância euclidiana
	float CalculateDistance(int x1, int y1, int x2, int y2);

	// Calcula velocidade (tiles/segundo)
	float CalculateSpeed(int conn, int new_x, int new_y, time_t current_time);

public:
	SpeedMonitor();

	// Valida movimento
	bool ValidateMove(int conn, int new_x, int new_y);

	// Atualiza posição do player
	void UpdatePlayerPosition(int conn, int x, int y);

	// Verifica se player está speed hacking
	bool IsSpeedHacking(int conn);

	// Reseta violações (chamado quando player morre, etc)
	void ResetViolations(int conn);

	// Remove player do monitoramento (disconnect)
	void RemovePlayer(int conn);

	// Ban temporário
	void BanPlayer(int conn, const char* reason);

	// Verifica se está banido
	bool IsBanned(int conn);

	// Estatísticas
	int GetViolationCount(int conn);
	double GetAverageSpeed(int conn);
};

// Instância global
extern SpeedMonitor g_SpeedMonitor;

} // namespace AntiCheat

#endif

#include "AntiCheat.h"
#include "Basedef.h"
#include <cstring>

// Declarações externas
extern CUser pUser[MAX_USER];
extern CMob pMob[MAX_MOB];
extern void SystemLog(const char* account, const char* mac, int ip, const char* msg);
extern void CloseUser(int conn);

namespace AntiCheat {

//==============================================================================
// SpeedMonitor Implementation
//==============================================================================

SpeedMonitor::SpeedMonitor()
{
}

float SpeedMonitor::CalculateDistance(int x1, int y1, int x2, int y2)
{
	int dx = x2 - x1;
	int dy = y2 - y1;
	return std::sqrt((float)(dx * dx + dy * dy));
}

float SpeedMonitor::CalculateSpeed(int conn, int new_x, int new_y, time_t current_time)
{
	std::lock_guard<std::mutex> lock(monitor_mutex);

	auto it = player_data.find(conn);
	if (it == player_data.end())
		return 0.0f;

	PlayerMovement& data = it->second;

	// Calcula distância
	float distance = CalculateDistance(data.last_x, data.last_y, new_x, new_y);

	// Calcula tempo decorrido
	time_t time_diff = current_time - data.last_move_time;
	if (time_diff <= 0)
		time_diff = 1;  // Evita divisão por zero

	// Velocidade = distância / tempo
	float speed = distance / (float)time_diff;

	// Atualiza média móvel
	data.avg_speed = (data.avg_speed * 0.7f) + (speed * 0.3f);

	return speed;
}

bool SpeedMonitor::ValidateMove(int conn, int new_x, int new_y)
{
	if (conn < 0 || conn >= MAX_USER)
		return true;

	if (pUser[conn].Mode != USER_PLAY)
		return true;

	std::lock_guard<std::mutex> lock(monitor_mutex);

	time_t current_time = time(nullptr);

	// Primeiro movimento - inicializa
	auto it = player_data.find(conn);
	if (it == player_data.end())
	{
		PlayerMovement data;
		data.last_x = new_x;
		data.last_y = new_y;
		data.last_move_time = current_time;
		data.suspicious_moves = 0;
		data.total_violations = 0;
		data.avg_speed = 0.0f;
		data.is_banned = false;
		data.ban_time = 0;

		player_data[conn] = data;
		return true;
	}

	PlayerMovement& data = it->second;

	// Verifica se está banido
	if (data.is_banned)
	{
		if (current_time - data.ban_time < 3600)  // Ban de 1 hora
			return false;
		else
		{
			// Ban expirou
			data.is_banned = false;
			data.suspicious_moves = 0;
			data.total_violations = 0;
		}
	}

	// Calcula distância e tempo
	float distance = CalculateDistance(data.last_x, data.last_y, new_x, new_y);
	time_t time_diff = current_time - data.last_move_time;

	// Se muito pouco tempo, ignora (evita spam de packets)
	if (time_diff == 0)
	{
		// Movimento na mesma timestamp - suspeito mas tolerado
		return true;
	}

	// Calcula velocidade
	float speed = distance / (float)time_diff;

	// Reseta violações antigas (após 60s sem problemas)
	if (time_diff > VIOLATION_RESET_TIME && data.suspicious_moves > 0)
	{
		data.suspicious_moves = 0;
	}

	// Valida velocidade
	if (speed > MAX_SPEED_TILES_PER_SECOND)
	{
		// SPEED HACK DETECTADO!
		data.suspicious_moves++;
		data.total_violations++;

		char logMsg[512];
		snprintf(logMsg, sizeof(logMsg),
			"SPEED HACK DETECTED: speed=%.2f tiles/s (max=%.2f) dist=%.2f time=%d violations=%d/%d",
			speed, MAX_SPEED_TILES_PER_SECOND, distance, (int)time_diff,
			data.suspicious_moves, SUSPICIOUS_MOVES_THRESHOLD);

		SystemLog(pUser[conn].AccountName, pUser[conn].MacAddress, pUser[conn].IP, logMsg);

		// Ações progressivas
		if (data.suspicious_moves >= BAN_MOVES_THRESHOLD)
		{
			// BAN de 1 hora
			BanPlayer(conn, "Speed Hack - Auto Ban");
			return false;
		}
		else if (data.suspicious_moves >= SUSPICIOUS_MOVES_THRESHOLD)
		{
			// KICK
			char kickMsg[256];
			snprintf(kickMsg, sizeof(kickMsg),
				"You have been kicked for Speed Hack. Violations: %d",
				data.suspicious_moves);

			SystemLog(pUser[conn].AccountName, pUser[conn].MacAddress, pUser[conn].IP,
				"KICKED for Speed Hack");

			CloseUser(conn);
			return false;
		}

		// Warning - recusa movimento
		return false;
	}

	// Movimento válido - atualiza posição
	data.last_x = new_x;
	data.last_y = new_y;
	data.last_move_time = current_time;

	return true;
}

void SpeedMonitor::UpdatePlayerPosition(int conn, int x, int y)
{
	std::lock_guard<std::mutex> lock(monitor_mutex);

	auto it = player_data.find(conn);
	if (it != player_data.end())
	{
		it->second.last_x = x;
		it->second.last_y = y;
		it->second.last_move_time = time(nullptr);
	}
}

bool SpeedMonitor::IsSpeedHacking(int conn)
{
	std::lock_guard<std::mutex> lock(monitor_mutex);

	auto it = player_data.find(conn);
	if (it == player_data.end())
		return false;

	return it->second.suspicious_moves >= SUSPICIOUS_MOVES_THRESHOLD;
}

void SpeedMonitor::ResetViolations(int conn)
{
	std::lock_guard<std::mutex> lock(monitor_mutex);

	auto it = player_data.find(conn);
	if (it != player_data.end())
	{
		it->second.suspicious_moves = 0;
	}
}

void SpeedMonitor::RemovePlayer(int conn)
{
	std::lock_guard<std::mutex> lock(monitor_mutex);
	player_data.erase(conn);
}

void SpeedMonitor::BanPlayer(int conn, const char* reason)
{
	std::lock_guard<std::mutex> lock(monitor_mutex);

	auto it = player_data.find(conn);
	if (it != player_data.end())
	{
		it->second.is_banned = true;
		it->second.ban_time = time(nullptr);

		char logMsg[512];
		snprintf(logMsg, sizeof(logMsg),
			"AUTO-BAN (1h): %s - Total violations: %d",
			reason, it->second.total_violations);

		SystemLog(pUser[conn].AccountName, pUser[conn].MacAddress, pUser[conn].IP, logMsg);
	}

	// Kick player
	CloseUser(conn);
}

bool SpeedMonitor::IsBanned(int conn)
{
	std::lock_guard<std::mutex> lock(monitor_mutex);

	auto it = player_data.find(conn);
	if (it == player_data.end())
		return false;

	if (!it->second.is_banned)
		return false;

	time_t current_time = time(nullptr);
	if (current_time - it->second.ban_time >= 3600)  // 1 hora
	{
		// Ban expirou
		it->second.is_banned = false;
		return false;
	}

	return true;
}

int SpeedMonitor::GetViolationCount(int conn)
{
	std::lock_guard<std::mutex> lock(monitor_mutex);

	auto it = player_data.find(conn);
	if (it == player_data.end())
		return 0;

	return it->second.total_violations;
}

double SpeedMonitor::GetAverageSpeed(int conn)
{
	std::lock_guard<std::mutex> lock(monitor_mutex);

	auto it = player_data.find(conn);
	if (it == player_data.end())
		return 0.0;

	return it->second.avg_speed;
}

// Global instance
SpeedMonitor g_SpeedMonitor;

} // namespace AntiCheat

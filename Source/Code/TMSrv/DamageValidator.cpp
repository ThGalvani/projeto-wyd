#include "DamageValidator.h"
#include "Basedef.h"
#include <cstdlib>
#include <ctime>

// Declarações externas
extern CUser pUser[MAX_USER];
extern CMob pMob[MAX_MOB];
extern void SystemLog(const char* account, const char* mac, int ip, const char* msg);
extern void CloseUser(int conn);

namespace DamageValidator {

//==============================================================================
// Validator Implementation
//==============================================================================

Validator::Validator()
{
}

int Validator::CalculateBaseDamage(
	int attacker_atk,
	int target_def,
	int attacker_level,
	int target_level)
{
	// Fórmula básica: ATK - DEF
	int base_damage = attacker_atk - target_def;

	// Ajuste por diferença de nível
	float level_factor = 1.0f;
	int level_diff = attacker_level - target_level;

	if (level_diff > 0)
	{
		// Atacante mais forte
		level_factor = 1.0f + (level_diff * 0.01f);  // +1% por nível
	}
	else if (level_diff < 0)
	{
		// Defensor mais forte
		level_factor = 1.0f + (level_diff * 0.01f);  // -1% por nível
	}

	base_damage = (int)(base_damage * level_factor);

	// Dano mínimo de 1
	if (base_damage < 1)
		base_damage = 1;

	return base_damage;
}

int Validator::ApplySkillMultiplier(int base_damage, int skill_id)
{
	// Multiplicadores de skill (exemplos - ajuste conforme seu jogo)
	float multiplier = 1.0f;

	if (skill_id == 0)
	{
		// Ataque normal
		multiplier = 1.0f;
	}
	else if (skill_id >= 1 && skill_id <= 10)
	{
		// Skills básicas (1.5x - 2.5x)
		multiplier = 1.5f + (skill_id * 0.1f);
	}
	else if (skill_id >= 11 && skill_id <= 20)
	{
		// Skills intermediárias (2.5x - 4.0x)
		multiplier = 2.5f + ((skill_id - 10) * 0.15f);
	}
	else if (skill_id >= 21 && skill_id <= 30)
	{
		// Skills avançadas (4.0x - 6.0x)
		multiplier = 4.0f + ((skill_id - 20) * 0.2f);
	}
	else
	{
		// Skills desconhecidas - usa 1.0x para segurança
		multiplier = 1.0f;
	}

	return (int)(base_damage * multiplier);
}

int Validator::ApplyCritical(int damage, bool is_critical)
{
	if (is_critical)
		return (int)(damage * 1.5f);  // Critical = 1.5x

	return damage;
}

void Validator::AddVariance(int& min_damage, int& max_damage)
{
	// Variância natural: ±10%
	int base = min_damage;
	min_damage = (int)(base * 0.90f);
	max_damage = (int)(base * 1.10f);

	// Garante mínimo de 1
	if (min_damage < 1)
		min_damage = 1;
}

void Validator::CalculateExpectedDamageRange(
	CMob* attacker,
	CMob* target,
	int skill_id,
	bool is_critical,
	int& min_damage,
	int& max_damage)
{
	if (attacker == nullptr || target == nullptr)
	{
		min_damage = 0;
		max_damage = 999999;  // Sem validação
		return;
	}

	// Extrai stats
	int atk = attacker->MOB.CurrentScore.Str + attacker->MOB.CurrentScore.Int;
	int def = target->MOB.CurrentScore.Con;
	int attacker_level = attacker->MOB.BaseScore.Level;
	int target_level = target->MOB.BaseScore.Level;

	// Calcula dano base
	int base_damage = CalculateBaseDamage(atk, def, attacker_level, target_level);

	// Aplica multiplicador de skill
	base_damage = ApplySkillMultiplier(base_damage, skill_id);

	// Aplica critical
	base_damage = ApplyCritical(base_damage, is_critical);

	// Define range
	min_damage = base_damage;
	max_damage = base_damage;

	// Adiciona variância
	AddVariance(min_damage, max_damage);

	// Adiciona tolerância extra
	min_damage = (int)(min_damage * (1.0f - DAMAGE_TOLERANCE));
	max_damage = (int)(max_damage * (1.0f + DAMAGE_TOLERANCE));

	// Garante mínimo
	if (min_damage < 1)
		min_damage = 1;
}

bool Validator::ValidateDamage(
	int conn,
	int reported_damage,
	int min_expected,
	int max_expected)
{
	if (conn < 0 || conn >= MAX_USER)
		return true;

	// Dano zero é sempre válido (miss)
	if (reported_damage == 0)
		return true;

	// Valida se está dentro do range
	if (reported_damage >= min_expected && reported_damage <= max_expected)
	{
		// Dano válido
		return true;
	}

	// DANO INVÁLIDO DETECTADO!
	std::lock_guard<std::mutex> lock(validator_mutex);

	// Registra violação
	auto it = violations.find(conn);
	if (it == violations.end())
	{
		DamageViolation violation;
		violation.total_violations = 1;
		violation.consecutive_violations = 1;
		violation.last_violation_time = time(nullptr);
		violation.last_reset_time = time(nullptr);

		violations[conn] = violation;
		it = violations.find(conn);
	}
	else
	{
		it->second.total_violations++;
		it->second.consecutive_violations++;
		it->second.last_violation_time = time(nullptr);
	}

	// Log
	char logMsg[512];
	snprintf(logMsg, sizeof(logMsg),
		"DAMAGE HACK DETECTED: reported=%d expected=[%d-%d] violations=%d/%d",
		reported_damage, min_expected, max_expected,
		it->second.consecutive_violations, MAX_DAMAGE_VIOLATIONS);

	SystemLog(pUser[conn].AccountName, pUser[conn].MacAddress, pUser[conn].IP, logMsg);

	// Kick se atingiu threshold
	if (it->second.consecutive_violations >= MAX_DAMAGE_VIOLATIONS)
	{
		SystemLog(pUser[conn].AccountName, pUser[conn].MacAddress, pUser[conn].IP,
			"KICKED for Damage Hack");

		CloseUser(conn);
		return false;
	}

	// Recusa dano
	return false;
}

void Validator::RecordViolation(int conn)
{
	std::lock_guard<std::mutex> lock(validator_mutex);

	auto it = violations.find(conn);
	if (it != violations.end())
	{
		it->second.total_violations++;
		it->second.consecutive_violations++;
		it->second.last_violation_time = time(nullptr);
	}
}

void Validator::ResetViolations(int conn)
{
	std::lock_guard<std::mutex> lock(validator_mutex);

	auto it = violations.find(conn);
	if (it != violations.end())
	{
		it->second.consecutive_violations = 0;
		it->second.last_reset_time = time(nullptr);
	}
}

void Validator::RemovePlayer(int conn)
{
	std::lock_guard<std::mutex> lock(validator_mutex);
	violations.erase(conn);
}

int Validator::GetViolationCount(int conn)
{
	std::lock_guard<std::mutex> lock(validator_mutex);

	auto it = violations.find(conn);
	if (it == violations.end())
		return 0;

	return it->second.total_violations;
}

// Global instance
Validator g_DamageValidator;

} // namespace DamageValidator

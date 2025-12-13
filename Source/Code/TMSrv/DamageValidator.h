#ifndef _DAMAGEVALIDATOR_H_
#define _DAMAGEVALIDATOR_H_

#include <mutex>
#include <map>

//==============================================================================
// FASE 4 - Sistema de Validação de Dano Server-Side
//
// Calcula dano esperado no servidor e compara com dano reportado pelo cliente.
// Previne packet injection de dano inflado.
//==============================================================================

namespace DamageValidator {

// Configuração
const float DAMAGE_TOLERANCE = 0.20f;  // 20% de tolerância (para variância natural)
const int MAX_DAMAGE_VIOLATIONS = 3;   // Máximo de violações antes de kick

// Estrutura de violações de dano
struct DamageViolation {
	int total_violations;
	int consecutive_violations;
	time_t last_violation_time;
	time_t last_reset_time;
};

// Sistema de validação
class Validator {
private:
	std::mutex validator_mutex;
	std::map<int, DamageViolation> violations;

	// Calcula dano base (ATK - DEF)
	int CalculateBaseDamage(
		int attacker_atk,
		int target_def,
		int attacker_level,
		int target_level
	);

	// Aplica multiplicador de skill
	int ApplySkillMultiplier(int base_damage, int skill_id);

	// Aplica critical
	int ApplyCritical(int damage, bool is_critical);

	// Adiciona variância natural (±10%)
	void AddVariance(int& min_damage, int& max_damage);

public:
	Validator();

	// Calcula range de dano esperado [min, max]
	void CalculateExpectedDamageRange(
		CMob* attacker,
		CMob* target,
		int skill_id,
		bool is_critical,
		int& min_damage,
		int& max_damage
	);

	// Valida dano reportado pelo cliente
	bool ValidateDamage(
		int conn,
		int reported_damage,
		int min_expected,
		int max_expected
	);

	// Registra violação
	void RecordViolation(int conn);

	// Reseta violações
	void ResetViolations(int conn);

	// Remove player
	void RemovePlayer(int conn);

	// Estatísticas
	int GetViolationCount(int conn);
};

// Instância global
extern Validator g_DamageValidator;

} // namespace DamageValidator

#endif

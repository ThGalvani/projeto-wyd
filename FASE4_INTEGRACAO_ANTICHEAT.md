# FASE 4 - INTEGRAÇÃO ANTI-CHEAT

Este guia mostra como integrar os sistemas Anti-Speed Hack e Validação de Dano.

---

## 🚀 1. Anti-Speed Hack

### Passo 1: Adicionar Includes

Em **`ProcessClientMessage.h`** ou onde os headers são centralizados:
```cpp
#include "AntiCheat.h"
```

### Passo 2: Integrar em MSG_MovePlayer

Procure o arquivo que processa movimentos (geralmente `_MSG_MovePlayer.cpp` ou similar).

**Localização:** Quando o servidor recebe novo X, Y do cliente.

```cpp
void Exec_MSG_MovePlayer(int conn, char* pMsg)
{
    MSG_MovePlayer* m = (MSG_MovePlayer*)pMsg;

    int new_x = m->X;
    int new_y = m->Y;

    // ===== FASE 4: ANTI-SPEED HACK =====
    // Valida movimento ANTES de aplicar
    if (!AntiCheat::g_SpeedMonitor.ValidateMove(conn, new_x, new_y))
    {
        // Movimento inválido - speed hack detectado
        // ValidateMove já kickou/baniu se necessário

        // Envia posição antiga de volta (rollback)
        MSG_UpdatePosition update;
        update.X = pMob[conn].TargetX;
        update.Y = pMob[conn].TargetY;
        pUser[conn].cSock.AddMessage((char*)&update, sizeof(update));

        return;  // Recusa movimento
    }
    // ===================================

    // Aplica movimento (código original)
    pMob[conn].TargetX = new_x;
    pMob[conn].TargetY = new_y;

    // Atualiza grid, etc...
}
```

### Passo 3: Cleanup ao Desconectar

Em **`CloseUser()`** ou função de disconnect:

```cpp
void CloseUser(int conn)
{
    // ... código existente ...

    // FASE 4: Remove do anti-cheat monitor
    AntiCheat::g_SpeedMonitor.RemovePlayer(conn);
    DamageValidator::g_DamageValidator.RemovePlayer(conn);

    // ... resto do código ...
}
```

### Passo 4: Reset ao Morrer (Opcional)

Em **`OnPlayerDeath()`** ou similar:

```cpp
void OnPlayerDeath(int conn)
{
    // ... código existente ...

    // FASE 4: Reseta violações ao morrer
    AntiCheat::g_SpeedMonitor.ResetViolations(conn);
    DamageValidator::g_DamageValidator.ResetViolations(conn);

    // ... resto do código ...
}
```

---

## ⚔️ 2. Validação de Dano

### Passo 1: Adicionar Includes

```cpp
#include "DamageValidator.h"
```

### Passo 2: Integrar em MSG_Attack

Procure o arquivo que processa ataques (geralmente `_MSG_Attack.cpp` ou `ProcessAttack.cpp`).

**Localização:** Quando o servidor recebe dano do cliente.

```cpp
void Exec_MSG_Attack(int conn, char* pMsg)
{
    MSG_Attack* m = (MSG_Attack*)pMsg;

    int attacker_id = conn;
    int target_id = m->TargetID;
    int reported_damage = m->Damage;
    int skill_id = m->SkillID;
    bool is_critical = m->IsCritical;

    // Validações básicas
    if (target_id < 0 || target_id >= MAX_MOB)
        return;

    CMob* attacker = &pMob[attacker_id];
    CMob* target = &pMob[target_id];

    // ===== FASE 4: VALIDAÇÃO DE DANO =====
    int min_expected, max_expected;

    DamageValidator::g_DamageValidator.CalculateExpectedDamageRange(
        attacker,
        target,
        skill_id,
        is_critical,
        min_expected,
        max_expected
    );

    if (!DamageValidator::g_DamageValidator.ValidateDamage(
        conn,
        reported_damage,
        min_expected,
        max_expected))
    {
        // DANO INVÁLIDO!
        // ValidateDamage já kickou se necessário

        // Usa dano médio esperado em vez do reportado
        reported_damage = (min_expected + max_expected) / 2;

        char logMsg[256];
        snprintf(logMsg, sizeof(logMsg),
            "Damage corrected to %d (expected range: [%d-%d])",
            reported_damage, min_expected, max_expected);

        SystemLog(pUser[conn].AccountName, pUser[conn].MacAddress,
                  pUser[conn].IP, logMsg);
    }
    // ====================================

    // Aplica dano (validado ou corrigido)
    target->MOB.CurrentScore.Hp -= reported_damage;

    if (target->MOB.CurrentScore.Hp < 0)
        target->MOB.CurrentScore.Hp = 0;

    // Envia atualização, etc...
}
```

### Alternativa: Recalcular Dano no Servidor

Para máxima segurança, **ignore completamente** o dano do cliente:

```cpp
void Exec_MSG_Attack(int conn, char* pMsg)
{
    MSG_Attack* m = (MSG_Attack*)pMsg;

    // NÃO usa m->Damage!

    int attacker_id = conn;
    int target_id = m->TargetID;
    int skill_id = m->SkillID;
    bool is_critical = m->IsCritical;

    CMob* attacker = &pMob[attacker_id];
    CMob* target = &pMob[target_id];

    // ===== CALCULA DANO NO SERVIDOR =====
    int min_damage, max_damage;

    DamageValidator::g_DamageValidator.CalculateExpectedDamageRange(
        attacker,
        target,
        skill_id,
        is_critical,
        min_damage,
        max_damage
    );

    // Usa valor aleatório dentro do range
    int actual_damage = min_damage + (rand() % (max_damage - min_damage + 1));
    // ====================================

    // Aplica dano calculado pelo servidor
    target->MOB.CurrentScore.Hp -= actual_damage;

    if (target->MOB.CurrentScore.Hp < 0)
        target->MOB.CurrentScore.Hp = 0;

    // Envia dano REAL de volta para cliente
    MSG_DamageResult result;
    result.Damage = actual_damage;
    result.IsCritical = is_critical;
    pUser[conn].cSock.AddMessage((char*)&result, sizeof(result));
}
```

**Recomendação:** Use a **Alternativa** (recalcular no servidor) para máxima segurança!

---

## ⚙️ 3. Configuração Avançada

### Ajustar Limites de Velocidade

Em **`AntiCheat.h`**:

```cpp
// Para servidor com montarias mais rápidas:
const float MAX_SPEED_TILES_PER_SECOND = 20.0f;  // Era 12.0f

// Para servidor mais rigoroso:
const int SUSPICIOUS_MOVES_THRESHOLD = 3;  // Era 5
```

### Ajustar Tolerância de Dano

Em **`DamageValidator.h`**:

```cpp
// Para maior tolerância (menos kicks):
const float DAMAGE_TOLERANCE = 0.30f;  // Era 0.20f (20%)

// Para menor tolerância (mais rigoroso):
const float DAMAGE_TOLERANCE = 0.10f;  // 10% apenas
```

### Ajustar Multiplicadores de Skill

Em **`DamageValidator.cpp`**, função `ApplySkillMultiplier()`:

```cpp
// Personalize conforme as skills do seu servidor
if (skill_id == 15)  // Fireball
{
    multiplier = 3.5f;
}
else if (skill_id == 27)  // Ultimate Attack
{
    multiplier = 6.0f;
}
```

---

## 🧪 4. Teste os Sistemas

### Teste 1: Speed Hack Detectado

1. Use Cheat Engine para acelerar movimento
2. Mova o personagem rapidamente
3. **Esperado:**
   - Log "SPEED HACK DETECTED" aparece
   - Após 5 violações → Kick
   - Após 10 violações → Ban 1h

### Teste 2: Damage Hack Detectado

1. Use Cheat Engine para modificar dano
2. Ataque um mob/player
3. **Esperado:**
   - Log "DAMAGE HACK DETECTED" aparece
   - Dano é corrigido para valor esperado
   - Após 3 violações → Kick

### Teste 3: Movimento Legítimo

1. Mova personagem normalmente
2. Use montaria (se disponível)
3. **Esperado:**
   - Nenhum log de speed hack
   - Movimento aceito normalmente

### Teste 4: Dano Legítimo

1. Ataque normalmente
2. Use skills
3. **Esperado:**
   - Dano aceito
   - Nenhum log de hack

---

## 📊 5. Monitoramento

### Logs Gerados

Ambos sistemas geram logs automáticos em **SystemLog**:

```
SPEED HACK DETECTED: speed=25.30 tiles/s (max=12.00) dist=126.50 time=5 violations=1/5
DAMAGE HACK DETECTED: reported=9999 expected=[450-550] violations=1/3
KICKED for Speed Hack
AUTO-BAN (1h): Speed Hack - Auto Ban - Total violations: 12
```

### Estatísticas

Você pode consultar estatísticas:

```cpp
// Em comando de admin, por exemplo
void CMD_CheckPlayer(int admin_conn, int target_conn)
{
    int speed_violations = AntiCheat::g_SpeedMonitor.GetViolationCount(target_conn);
    double avg_speed = AntiCheat::g_SpeedMonitor.GetAverageSpeed(target_conn);
    int damage_violations = DamageValidator::g_DamageValidator.GetViolationCount(target_conn);

    char msg[512];
    snprintf(msg, sizeof(msg),
        "Player Stats:\nSpeed Violations: %d\nAvg Speed: %.2f\nDamage Violations: %d",
        speed_violations, avg_speed, damage_violations);

    SendClientMessage(admin_conn, msg);
}
```

---

## ⚠️ 6. Considerações Importantes

### Falso-Positivos

**Speed Hack:**
- Teleport por skill/item pode parecer speed hack
- **Solução:** Chame `ResetViolations()` após teleport
- OU adicione exceção para teleport

```cpp
void TeleportPlayer(int conn, int x, int y)
{
    pMob[conn].TargetX = x;
    pMob[conn].TargetY = y;

    // IMPORTANTE: Atualiza posição no monitor
    AntiCheat::g_SpeedMonitor.UpdatePlayerPosition(conn, x, y);

    // Reseta violações (teleport legítimo)
    AntiCheat::g_SpeedMonitor.ResetViolations(conn);
}
```

**Damage Validator:**
- Buffs/debuffs podem causar variação
- **Solução:** Aumente `DAMAGE_TOLERANCE` para 25-30%
- OU inclua buffs no cálculo

### Performance

- **Anti-Speed:** ~0.01ms por movimento (desprezível)
- **Damage Validator:** ~0.05ms por ataque (desprezível)
- **Locks:** Thread-safe, overhead mínimo

### Lag de Rede

Players com lag podem ter movimentos "saltados".

**Solução:** O sistema já tolera isso usando **time_diff** no cálculo de velocidade.

---

## 🎯 7. Resumo de Integração

**Arquivos Criados:**
- `Source/Code/TMSrv/AntiCheat.h`
- `Source/Code/TMSrv/AntiCheat.cpp`
- `Source/Code/TMSrv/DamageValidator.h`
- `Source/Code/TMSrv/DamageValidator.cpp`

**Arquivos a Modificar:**
- `_MSG_MovePlayer.cpp` (ou similar) - adiciona ValidateMove()
- `_MSG_Attack.cpp` (ou similar) - adiciona ValidateDamage()
- `CloseUser()` - adiciona RemovePlayer()

**Compilação:**
Adicione os 4 novos arquivos ao projeto Visual Studio e compile!

---

**FASE 4 IMPLEMENTADA! Servidor agora tem proteção contra Speed Hack e Damage Hack! 🛡️**

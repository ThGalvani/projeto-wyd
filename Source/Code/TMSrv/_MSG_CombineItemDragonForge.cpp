/*
*   Dragon Infusion System - RedDragon Item Crafting
*   Created: 2025-12-15
*   NPC: Mestre da Forja Dracônica (Dragon Forge Master)
*
*   Sistema inovador de crafting que permite ao jogador escolher
*   adicionais através de Essências Elementais
*/

#include "ProcessClientMessage.h"

// Define constants for RedDragon system
#define DRAGON_SOUL_ID          5752
#define REDDRAGON_SCALE_ID      5753
#define INFERNAL_FLAME_ID       5754
#define ESSENCE_FIRE_ID         5757
#define ESSENCE_ICE_ID          5758
#define ESSENCE_THUNDER_ID      5759
#define ESSENCE_EARTH_ID        5760
#define ESSENCE_WIND_ID         5761

#define DESTRUCTION_ITEMLEVEL   6
#define REDDRAGON_ITEMLEVEL     7
#define INFUSION_COST           150000000  // 150M gold

// RedDragon Item IDs
#define REDDRAGON_CLAW_ID       5762
#define REDDRAGON_STAFF_ID      5763
#define REDDRAGON_HELM_ID       5764
#define REDDRAGON_ARMOR_ID      5765
#define REDDRAGON_PANTS_ID      5766
#define REDDRAGON_GLOVES_ID     5767
#define REDDRAGON_BOOTS_ID      5768

// Success rates
#define INFUSION_SUCCESS_RATE   65  // 65%

// Structure to hold infusion recipe
struct InfusionRecipe {
    int DestructionItemID;
    int RedDragonItemID;
    int RequiredSlot;      // Position slot (2=helm, 4=armor, 8=pants, 16=gloves, 32=boots, 64=weapon)
};

// Mapping table: Destruction items -> RedDragon items
InfusionRecipe g_InfusionRecipes[] = {
    // BeastMaster weapons (garra = ID unknown, using slot check)
    // BeastMaster armor set "Destruição" IDs from analysis: 1505-1509
    {1505, REDDRAGON_HELM_ID, 2},      // Elmo da Destruição -> Elmo RedDragon
    {1506, REDDRAGON_ARMOR_ID, 4},     // Armadura da Destruição -> Armadura RedDragon
    {1507, REDDRAGON_PANTS_ID, 8},     // Calça da Destruição -> Calça RedDragon
    {1508, REDDRAGON_GLOVES_ID, 16},   // Manoplas da Destruição -> Manoplas RedDragon
    {1509, REDDRAGON_BOOTS_ID, 32},    // Botas da Destruição -> Botas RedDragon
    {0, 0, 0} // Terminator
};

// Get essence bonus based on essence type and item type
void ApplyEssenceBonus(STRUCT_ITEM* item, int essenceID, bool isWeapon)
{
    if (isWeapon)
    {
        switch(essenceID)
        {
            case ESSENCE_FIRE_ID:    // +72 Physical Damage
                BASE_SetItemEffect(item, EF_DAMAGE, 72);
                break;
            case ESSENCE_ICE_ID:     // +56% Magic Attack
                BASE_SetItemEffect(item, EF_MAGIC, 56);
                break;
            case ESSENCE_THUNDER_ID: // +35 Penetration
                BASE_SetItemEffect(item, EF_PER, 35);
                break;
            case ESSENCE_EARTH_ID:   // +40 Absorption
                BASE_SetItemEffect(item, EF_ABS, 40);
                break;
            case ESSENCE_WIND_ID:    // +8% Attack Speed
                BASE_SetItemEffect(item, EF_ATTSPD, 8);
                break;
        }
    }
    else // Armor
    {
        switch(essenceID)
        {
            case ESSENCE_FIRE_ID:    // +5% Critical Rate
                BASE_SetItemEffect(item, EF_CRITICAL, 5);
                break;
            case ESSENCE_ICE_ID:     // +100 Defense
                BASE_SetItemEffect(item, EF_AC, 100);
                break;
            case ESSENCE_THUNDER_ID: // +80 HP
                BASE_SetItemEffect(item, EF_HPADD, 80);
                break;
            case ESSENCE_EARTH_ID:   // +15% Damage Reduction
                BASE_SetItemEffect(item, EF_ACADD, 15);
                break;
            case ESSENCE_WIND_ID:    // +3 Movement Speed
                BASE_SetItemEffect(item, EF_RUNSPEED, 3);
                break;
        }
    }
}

// Find RedDragon item ID from Destruction item
int GetRedDragonItemID(int destructionID, int slot)
{
    for (int i = 0; g_InfusionRecipes[i].DestructionItemID != 0; i++)
    {
        if (g_InfusionRecipes[i].DestructionItemID == destructionID)
            return g_InfusionRecipes[i].RedDragonItemID;
    }

    // If not found by ID, try by slot (for weapons)
    for (int i = 0; g_InfusionRecipes[i].DestructionItemID != 0; i++)
    {
        if (g_InfusionRecipes[i].RequiredSlot == slot)
            return g_InfusionRecipes[i].RedDragonItemID;
    }

    return 0; // Not found
}

// Validate infusion materials
bool ValidateInfusionMaterials(MSG_CombineItem* m, int& essenceID, int& destructionSlot)
{
    bool hasDestruction = false;
    bool hasScale = false;
    int scaleCount = 0;
    bool hasSoul = false;
    bool hasFlame = false;
    int flameCount = 0;
    bool hasEssence = false;

    for (int i = 0; i < MAX_COMBINE; i++)
    {
        if (m->Item[i].sIndex == 0)
            continue;

        int itemID = m->Item[i].sIndex;

        // Check Destruction item (ItemLevel 6, +15)
        if (BASE_GetItemAbility(&m->Item[i], EF_ITEMLEVEL) == DESTRUCTION_ITEMLEVEL)
        {
            if (BASE_GetItemSanc(&m->Item[i]) == REF_15)
            {
                hasDestruction = true;
                destructionSlot = m->Item[i].stEffect[0].cEffect; // Get item slot
            }
        }

        // Check materials
        else if (itemID == REDDRAGON_SCALE_ID)
        {
            scaleCount++;
            if (scaleCount >= 3)
                hasScale = true;
        }
        else if (itemID == DRAGON_SOUL_ID)
        {
            hasSoul = true;
        }
        else if (itemID == INFERNAL_FLAME_ID)
        {
            flameCount++;
            if (flameCount >= 2)
                hasFlame = true;
        }
        // Check essence
        else if (itemID >= ESSENCE_FIRE_ID && itemID <= ESSENCE_WIND_ID)
        {
            if (!hasEssence) // Only first essence counts
            {
                hasEssence = true;
                essenceID = itemID;
            }
        }
    }

    return (hasDestruction && hasScale && hasSoul && hasFlame && hasEssence);
}

// Main Dragon Infusion handler
void Exec_MSG_CombineItemDragonForge(int conn, char* pMsg)
{
    MSG_CombineItem* m = (MSG_CombineItem*)pMsg;

    int Size = m->Size;

    // Size validation
    if (Size > sizeof(MSG_CombineItem))
    {
        SendClientMessage(conn, "Impossível executar ação, tente mais tarde.");
        return;
    }

    // Packet delay control
    if (pUser[conn].Atraso != 0)
    {
        int isTime = GetTickCount64() - pUser[conn].Atraso;

        if (isTime < 1000) // 1 second delay
        {
            SendClientMessage(conn, "Aguarde 1 segundo para uma nova tentativa.");
            return;
        }
    }
    pUser[conn].Atraso = GetTickCount64();

    // Level requirement check
    if (pMob[conn].MOB.Status.Level < 380)
    {
        SendClientMessage(conn, "Você precisa ser Celestial 380+ para usar Dragon Infusion.");
        SendClientSignalParm(conn, ESCENE_FIELD, _MSG_CombineComplete, 0);
        return;
    }

    // Inventory validation
    for (int i = 0; i < MAX_COMBINE; i++)
    {
        int invPos = m->InvenPos[i];

        if (m->Item[i].sIndex == 0)
            continue;

        if (invPos == -1)
        {
            Combinelogsitem(pUser[conn].AccountName, pUser[conn].MacAddress, pUser[conn].IP,
                           "dup,Tentativa de Dup DragonForge");
            continue;
        }

        if (invPos < 0 || invPos >= pMob[conn].MaxCarry)
        {
            RemoveTrade(conn);
            return;
        }

        if (memcmp(&pMob[conn].MOB.Carry[invPos], &m->Item[i], sizeof(STRUCT_ITEM)))
        {
            Combinelogsitem(pUser[conn].AccountName, pUser[conn].MacAddress, pUser[conn].IP,
                           "err,msg_DragonForge - item removed or changed.");
            SendClientSignalParm(conn, ESCENE_FIELD, _MSG_CombineComplete, 0);
            return;
        }
    }

    // Gold check
    if (pMob[conn].MOB.Coin < INFUSION_COST)
    {
        SendClientMessage(conn, "Você precisa de 150.000.000 Gold para realizar a Dragon Infusion.");
        SendClientSignalParm(conn, ESCENE_FIELD, _MSG_CombineComplete, 0);
        return;
    }

    // Validate materials
    int essenceID = 0;
    int destructionSlot = 0;

    if (!ValidateInfusionMaterials(m, essenceID, destructionSlot))
    {
        SendClientMessage(conn, "Materiais incompletos! Necessário: Item Destruição +15, 3x Escama RedDragon, 1x Alma do Dragão, 2x Chama Infernal, 1x Essência Elemental.");
        SendClientSignalParm(conn, ESCENE_FIELD, _MSG_CombineComplete, 0);
        return;
    }

    // Find target RedDragon item
    int redDragonID = GetRedDragonItemID(m->Item[0].sIndex, destructionSlot);

    if (redDragonID == 0)
    {
        SendClientMessage(conn, "Item Destruição inválido para conversão.");
        SendClientSignalParm(conn, ESCENE_FIELD, _MSG_CombineComplete, 0);
        return;
    }

    // Roll success/failure (65% success rate)
    int random = rand() % 100;
    bool success = (random < INFUSION_SUCCESS_RATE);

    // Remove materials from inventory
    for (int i = 0; i < MAX_COMBINE; i++)
    {
        if (m->InvenPos[i] != -1 && m->Item[i].sIndex != 0)
        {
            // Keep Destruction item if failed (but reset refinement)
            if (!success && BASE_GetItemAbility(&m->Item[i], EF_ITEMLEVEL) == DESTRUCTION_ITEMLEVEL)
            {
                // Reset refinement to +0
                pMob[conn].MOB.Carry[m->InvenPos[i]].stEffect[2].cEffect = 0;
                pMob[conn].MOB.Carry[m->InvenPos[i]].stEffect[2].cValue = 0;
                continue; // Don't delete
            }

            // Delete all other materials
            memset(&pMob[conn].MOB.Carry[m->InvenPos[i]], 0, sizeof(STRUCT_ITEM));
            SendItem(conn, ITEM_PLACE_CARRY, m->InvenPos[i], &pMob[conn].MOB.Carry[m->InvenPos[i]]);
        }
    }

    // Deduct gold
    pMob[conn].MOB.Coin -= INFUSION_COST;
    SendClientSignalParm(conn, ESCENE_FIELD, _MSG_SendCoin, pMob[conn].MOB.Coin);

    if (success)
    {
        // Create RedDragon item
        STRUCT_ITEM newItem;
        memset(&newItem, 0, sizeof(STRUCT_ITEM));

        // Load base RedDragon item from database
        BASE_GetItemData(&newItem, redDragonID);

        // Copy some effects from old Destruction item (optional: keep some additionals)
        // For now, start fresh with base stats

        // Apply essence bonus
        bool isWeapon = (destructionSlot == 64); // Weapon slot
        ApplyEssenceBonus(&newItem, essenceID, isWeapon);

        // Find empty inventory slot
        int emptySlot = GetFirstEmptySlot(conn);

        if (emptySlot == -1)
        {
            SendClientMessage(conn, "Inventário cheio! Dragon Infusion bem-sucedida mas não há espaço.");
            SendClientSignalParm(conn, ESCENE_FIELD, _MSG_CombineComplete, 0);
            SaveUser(conn, 0);
            return;
        }

        // Add to inventory
        memcpy(&pMob[conn].MOB.Carry[emptySlot], &newItem, sizeof(STRUCT_ITEM));
        SendItem(conn, ITEM_PLACE_CARRY, emptySlot, &newItem);

        // Success message
        char successMsg[128];
        sprintf(successMsg, "Dragon Infusion bem-sucedida! Você criou: %s com bônus elemental!",
                BASE_GetItemName(redDragonID));
        SendClientMessage(conn, successMsg);

        // Log success
        Combinelogsitem(pUser[conn].AccountName, pUser[conn].MacAddress, pUser[conn].IP,
                       "success,DragonForge - RedDragon item created");
    }
    else
    {
        // Failure message
        SendClientMessage(conn, "Dragon Infusion falhou! O item Destruição retornou em +0. Materiais foram perdidos.");

        // Log failure
        Combinelogsitem(pUser[conn].AccountName, pUser[conn].MacAddress, pUser[conn].IP,
                       "fail,DragonForge - Infusion failed");
    }

    SendClientSignalParm(conn, ESCENE_FIELD, _MSG_CombineComplete, 1);
    SaveUser(conn, 0);
}

// Helper function to get first empty inventory slot
int GetFirstEmptySlot(int conn)
{
    for (int i = 0; i < pMob[conn].MaxCarry; i++)
    {
        if (pMob[conn].MOB.Carry[i].sIndex == 0)
            return i;
    }
    return -1;
}

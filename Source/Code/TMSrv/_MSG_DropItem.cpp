/*
*   Copyright (C) {2015}  {Victor Klafke, Charles TheHouse}
*
*   This program is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program.  If not, see [http://www.gnu.org/licenses/].
*
*   Contact at: victor.klafke@ecomp.ufsm.br
*/
#include "ProcessClientMessage.h"

void Exec_MSG_DropItem(int conn, char *pMsg)
{

	MSG_DropItem* m = (MSG_DropItem*)pMsg;

	if (m->SourPos < 0 || m->SourPos > 60)
		return;

	if (m->SourType < 0 || m->SourType > 2)
		return;

	if (m->GridX < 0 || m->GridX > CARGOGRIDX)
		return;

	if (m->GridY < 0 || m->GridY > CARGOGRIDY)
		return;

	if (m->ItemID < 0 || m->ItemID > MAX_ITEMLIST)
		return;

	int Size = m->Size;

	if (Size > sizeof(MSG_DropItem)) //CONTROLE DE SIZE
	{
		SendClientMessage(conn, "Imposs�vel executar a��o27, tente mais tarde. ");
		return;
	}

	if (pMob[conn].MOB.CurrentScore.Hp <= 0 || pUser[conn].Mode != USER_PLAY)
	{
		AddCrackError(conn, 1, 14);
		SendHpMode(conn);
		return;
	}

	if (pUser[conn].Trade.OpponentID)
	{
		RemoveTrade(pUser[conn].Trade.OpponentID);
		RemoveTrade(conn);
		return;
	}

	if (pUser[conn].TradeMode)
	{
		SendClientMessage(conn, g_pMessageStringTable[_NN_CantWhenAutoTrade]);
		return;
	}

	if (m->GridX >= MAX_GRIDX || m->GridY >= MAX_GRIDY)
	{
		sprintf_s(temp, "err,wrong drop pos %d %d", m->GridX, m->GridY);
		SystemLog(pUser[conn].AccountName, pUser[conn].MacAddress, pUser[conn].IP, temp);
		return;
	}

	if (isDropItem == 0)
		return;

	int gridx = m->GridX;
	int gridy = m->GridY;

	int titem = GetEmptyItemGrid(&gridx, &gridy);

	m->GridX = gridx;
	m->GridY = gridy;

	if (titem == 0)
	{
		SendClientMessage(conn, g_pMessageStringTable[_NN_Cant_Drop_Here]);
		return;
	}

	if (m->SourType == ITEM_PLACE_EQUIP)
	{
		//Log("err,dropitem - sourtype", pUser[conn].AccountName, pUser[conn].IP);
		return;
	}

	if (m->SourType)
	{
		if (m->SourType == ITEM_PLACE_CARRY)
		{
			if (m->SourPos < 0 || m->SourPos >= pMob[conn].MaxCarry)
			{
				//Log("err,dropitem - carry equip", pUser[conn].AccountName, pUser[conn].IP);
				return;
			}
		}
		else
		{
			if (m->SourType != ITEM_PLACE_CARGO)
			{
				//Log("err,dropitem - sourtype", pUser[conn].AccountName, pUser[conn].IP);
				return;
			}
			if (m->SourPos < 0 || m->SourPos >= MAX_CARGO)
			{
				//Log("err,dropitem - sourpos cargo", pUser[conn].AccountName, pUser[conn].IP);
				return;
			}
		}
	}

	//==============================================================================
	// FASE 1 EMERGENCIA - FIX VULNERABILIDADE CRITICA #1
	// Correcao de race condition em DropItem
	//
	// PROBLEMA ORIGINAL:
	// - Item era criado no chao ANTES de ser removido do inventario
	// - Janela de vulnerabilidade permitia disconnect timing dupe
	//
	// SOLUCAO:
	// 1. Lock do player (previne operacoes concorrentes)
	// 2. Backup completo do item (nao apenas ponteiro)
	// 3. REMOVE do inventario PRIMEIRO
	// 4. Tenta criar no chao
	// 5. Se falhar, ROLLBACK (restaura do backup)
	//==============================================================================

	// Lock do player para operacao atomica
	PlayerLockGuard lock(conn);

	STRUCT_ITEM* SrcItem = GetItemPointer(&pMob[conn].MOB, pUser[conn].Cargo, m->SourType, m->SourPos);

	if (SrcItem == NULL)
		return;

	if (SrcItem->sIndex <= 0 || SrcItem->sIndex >= MAX_ITEMLIST)
		return;

	// Validacao: itens especiais nao podem ser dropados
	if (SrcItem->sIndex == 508 || SrcItem->sIndex == 509 || SrcItem->sIndex == 522 ||
		(SrcItem->sIndex >= 526 && SrcItem->sIndex <= 537) ||
		SrcItem->sIndex == 446 || SrcItem->sIndex == 747 ||
		SrcItem->sIndex == 3993 || SrcItem->sIndex == 3994)
	{
		SendClientMessage(conn, g_pMessageStringTable[_NN_Guild_Medal_Cant_Be_Droped]);
		return;
	}

	// PASSO 1: BACKUP COMPLETO do item (nao apenas ponteiro)
	STRUCT_ITEM itemBackup;
	memcpy(&itemBackup, SrcItem, sizeof(STRUCT_ITEM));

	char tmplog[2048];
	BASE_GetItemCode(&itemBackup, tmplog);
	sprintf(temp, "dropitem, %s", tmplog);
	SystemLog(pUser[conn].AccountName, pUser[conn].MacAddress, pUser[conn].IP, temp);

	// PASSO 2: REMOVE do inventario PRIMEIRO (operacao atomica)
	memset(SrcItem, 0, sizeof(STRUCT_ITEM));

	// Envia atualizacao para o cliente imediatamente
	SendItem(conn, m->SourType, m->SourPos, SrcItem);

	// PASSO 3: Tenta criar item no chao
	int drop = CreateItem(m->GridX, m->GridY, &itemBackup, m->Rotate, 1);

	if (drop <= 0 || drop >= MAX_ITEM)
	{
		// PASSO 4: ROLLBACK - Falhou ao criar no chao, restaura no inventario
		memcpy(SrcItem, &itemBackup, sizeof(STRUCT_ITEM));
		SendItem(conn, m->SourType, m->SourPos, SrcItem);

		sprintf(temp, "dropitem FAILED - rollback, item restored to inventory");
		SystemLog(pUser[conn].AccountName, pUser[conn].MacAddress, pUser[conn].IP, temp);

		SendClientMessage(conn, "Can't create object(item)");
		return;
	}

	// PASSO 5: SUCESSO - Confirma drop para o cliente
	MSG_CNFDropItem sm_ditem;
	memset(&sm_ditem, 0, sizeof(MSG_CNFDropItem));

	sm_ditem.Type = _MSG_CNFDropItem;
	sm_ditem.Size = sizeof(MSG_CNFDropItem);
	sm_ditem.SourType = m->SourType;
	sm_ditem.SourPos = m->SourPos;
	sm_ditem.Rotate = m->Rotate;
	sm_ditem.GridX = m->GridX;
	sm_ditem.GridY = m->GridY;

	pUser[conn].cSock.SendOneMessage((char*)&sm_ditem, sizeof(MSG_CNFDropItem));

	//==============================================================================
	// END FASE 1 - DropItem agora e atomico e seguro contra dupes
	//==============================================================================
}
#include "stdafx.h"
#include "CustomJewelBank.h"
#include "GameMain.h"
#include "DSProtocol.h"
#include "ItemManager.h"
#include "ServerInfo.h"
#include "Notice.h"
#include "Util.h"
#include "Message.h"
#include "MemScript.h"


CBankEx gBankEx;

CBankEx::CBankEx()
{
	this->Init();
}

CBankEx::~CBankEx()
{
	
}

void CBankEx::Init() // OK
{
	for(int n=0;n < MAX_CUSTOM_BANK;n++)
	{
		this->m_JewelBank[n].Index = -1;
	}
}

void CBankEx::Load(char* path) // OK
{
	CMemScript* lpMemScript = new CMemScript;

	if(lpMemScript == 0)
	{
		ErrorMessageBox(MEM_SCRIPT_ALLOC_ERROR,path);
		return;
	}

	if(lpMemScript->SetBuffer(path) == 0)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
		delete lpMemScript;
		return;
	}
	
	this->Init();

	try
	{
		while(true)
		{
			if(lpMemScript->GetToken() == TOKEN_END)
			{
				break;
			}

			if(strcmp("end",lpMemScript->GetString()) == 0)
			{
				break;
			}

			JEWEL_BANK info;

			memset(&info,0,sizeof(info));
			static int CustomItemIndexCount = 0;

			info.Index = CustomItemIndexCount++;

			info.ID = lpMemScript->GetNumber();

			info.ItemID = lpMemScript->GetAsNumber();
		
			strcpy_s(info.ItemName,lpMemScript->GetAsString());

			this->SetInfo(info);
		}
	}
	catch(...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;
}

void CBankEx::SetInfo(JEWEL_BANK info) // OK
{
	if(info.Index < 0 || info.Index >= MAX_CUSTOM_BANK)
	{
		return;
	}

	this->m_JewelBank[info.Index] = info;
}

void CBankEx::InitUser(LPOBJ lpObj)
{
	lpObj->BankEx.Money			= 0;
	lpObj->BankEx.JBless		= 0;
	lpObj->BankEx.JSoul			= 0;
	lpObj->BankEx.JChaos		= 0;
	lpObj->BankEx.JCreation		= 0;
	lpObj->BankEx.JLife			= 0;
	lpObj->BankEx.JHarmony		= 0;
	lpObj->BankEx.JGuardian		= 0;
	lpObj->BankEx.JGemstone		= 0;
	lpObj->BankEx.JLowRef		= 0;
	lpObj->BankEx.JHigRef		= 0;
	//new jewels
/*	lpObj->BankEx.JCustomJ1		= 0;
	lpObj->BankEx.JCustomJ2		= 0;
	lpObj->BankEx.JCustomJ3		= 0;
	lpObj->BankEx.JCustomJ4		= 0;
	lpObj->BankEx.JCustomJ5		= 0;
	lpObj->BankEx.JCustomJ6		= 0;
	lpObj->BankEx.JCustomJ7		= 0;
	lpObj->BankEx.JCustomJ8		= 0;
	lpObj->BankEx.JCustomJ9		= 0;
	lpObj->BankEx.JCustomJ10	= 0;
	lpObj->BankEx.JCustomJ11	= 0;*/
}

void CBankEx::GDSavePoint(int aIndex)
{
	if( !gObjIsConnected(aIndex) )
	{
		return;
	}
	// ----
	LPOBJ lpUser = &gObj[aIndex];
	// ----
	BANKEX_GD_SAVE_POINT pRequest;
	pRequest.h.set(0xFB, 0x06, sizeof(pRequest));
	memcpy(pRequest.AccountID, lpUser->Account, sizeof(lpUser->Account));
	pRequest.UserIndex	= aIndex;
	pRequest.Money		= lpUser->BankEx.Money;
	pRequest.Bless		= lpUser->BankEx.JBless;
	pRequest.Soul		= lpUser->BankEx.JSoul;
	pRequest.Chaos		= lpUser->BankEx.JChaos;
	pRequest.Creation	= lpUser->BankEx.JCreation;
	pRequest.Life		= lpUser->BankEx.JLife;
	pRequest.Harmony	= lpUser->BankEx.JHarmony;
	pRequest.Guardian	= lpUser->BankEx.JGuardian;
	pRequest.Gemstone	= lpUser->BankEx.JGemstone;
	pRequest.LowRefine	= lpUser->BankEx.JLowRef;
	pRequest.HigRefine	= lpUser->BankEx.JHigRef;
	//new jewels
/*	pRequest.Custom001	= lpUser->BankEx.JCustomJ1;
	pRequest.Custom002	= lpUser->BankEx.JCustomJ2;
	pRequest.Custom003	= lpUser->BankEx.JCustomJ3;
	pRequest.Custom004	= lpUser->BankEx.JCustomJ4;
	pRequest.Custom005	= lpUser->BankEx.JCustomJ5;
	pRequest.Custom006	= lpUser->BankEx.JCustomJ6;
	pRequest.Custom007	= lpUser->BankEx.JCustomJ7;
	pRequest.Custom008	= lpUser->BankEx.JCustomJ8;
	pRequest.Custom009	= lpUser->BankEx.JCustomJ9;
	pRequest.Custom010	= lpUser->BankEx.JCustomJ10;
	pRequest.Custom011	= lpUser->BankEx.JCustomJ11;*/

	gDataServerConnection.DataSend((BYTE*)&pRequest,pRequest.h.size);
}
// -------------------------------------------------------------------------------

void CBankEx::GDReqPoint(int aIndex)
{
	// ----
	LPOBJ lpUser = &gObj[aIndex];
	// ----
	BANKEX_GD_REQ_POINT pRequest;
	pRequest.h.set(0xFB, 0x05, sizeof(pRequest));
	memcpy(pRequest.AccountID, lpUser->Account, sizeof(lpUser->Account));
	pRequest.UserIndex = aIndex;
	gDataServerConnection.DataSend((BYTE*)&pRequest,pRequest.h.size);
}
// -------------------------------------------------------------------------------

void CBankEx::DGGetPoint(BANKEX_DG_GET_POINT * aRecv)
{
	// ----
	LPOBJ lpUser = &gObj[aRecv->UserIndex];
	// ----

	if( aRecv->Bless < 0 )
	{
		aRecv->Bless = 0;
	}
	if( aRecv->Soul < 0 )
	{
		aRecv->Soul = 0;
	}
	if (aRecv->Chaos < 0)
	{
		aRecv->Chaos = 0;
	}
	if (aRecv->Creation < 0)
	{
		aRecv->Creation = 0;
	}
	if( aRecv->Life < 0 )
	{
		aRecv->Life = 0;
	}
	if( aRecv->Harmony < 0 )
	{
		aRecv->Harmony = 0;
	}
	if( aRecv->Guardian < 0 )
	{
		aRecv->Guardian = 0;
	}
	if( aRecv->Gemstone < 0 )
	{
		aRecv->Gemstone = 0;
	}
	if( aRecv->LowRefine < 0 )
	{
		aRecv->LowRefine = 0;
	}
	if( aRecv->HigRefine < 0 )
	{
		aRecv->HigRefine = 0;
	}
//novas customs
/*	if( aRecv->Custom001 < 0 )
	{
		aRecv->Custom001 = 0;
	}
	
	if( aRecv->Custom002 < 0 )
	{
		aRecv->Custom002 = 0;
	}
	
	if( aRecv->Custom003 < 0 )
	{
		aRecv->Custom003 = 0;
	}
	
	if( aRecv->Custom004 < 0 )
	{
		aRecv->Custom004 = 0;
	}
	
	if( aRecv->Custom005 < 0 )
	{
		aRecv->Custom005 = 0;
	}
	
	if( aRecv->Custom006 < 0 )
	{
		aRecv->Custom006 = 0;
	}
	
	if( aRecv->Custom007 < 0 )
	{
		aRecv->Custom007 = 0;
	}
	
	if( aRecv->Custom008 < 0 )
	{
		aRecv->Custom008 = 0;
	}
	
	if( aRecv->Custom009 < 0 )
	{
		aRecv->Custom009 = 0;
	}
	
	if( aRecv->Custom010 < 0 )
	{
		aRecv->Custom010 = 0;
	}
	
	if( aRecv->Custom011 < 0 )
	{
		aRecv->Custom011 = 0;
	}*/
	// ----
	lpUser->BankEx.Money		= aRecv->Money;
	lpUser->BankEx.JBless		= aRecv->Bless;
	lpUser->BankEx.JSoul		= aRecv->Soul;
	lpUser->BankEx.JChaos		= aRecv->Chaos;
	lpUser->BankEx.JCreation	= aRecv->Creation;
	lpUser->BankEx.JLife		= aRecv->Life;
	lpUser->BankEx.JHarmony		= aRecv->Harmony;
	lpUser->BankEx.JGuardian	= aRecv->Guardian;
	lpUser->BankEx.JGemstone	= aRecv->Gemstone;
	lpUser->BankEx.JLowRef		= aRecv->LowRefine;
	lpUser->BankEx.JHigRef		= aRecv->HigRefine;
	//news
/*	lpUser->BankEx.JCustomJ1	= aRecv->Custom001;
	lpUser->BankEx.JCustomJ2	= aRecv->Custom002;
	lpUser->BankEx.JCustomJ3	= aRecv->Custom003;
	lpUser->BankEx.JCustomJ4	= aRecv->Custom004;
	lpUser->BankEx.JCustomJ5	= aRecv->Custom005;
	lpUser->BankEx.JCustomJ6	= aRecv->Custom006;
	lpUser->BankEx.JCustomJ7	= aRecv->Custom007;
	lpUser->BankEx.JCustomJ8	= aRecv->Custom008;
	lpUser->BankEx.JCustomJ9	= aRecv->Custom009;
	lpUser->BankEx.JCustomJ10	= aRecv->Custom010;
	lpUser->BankEx.JCustomJ11	= aRecv->Custom011;*/

	gBankEx.GCUpdateBankEx(lpUser->Index);
}
// -------------------------------------------------------------------------------

void CBankEx::GCUpdateBankEx(int aIndex)
{
	if (!gObjIsConnectedGP(aIndex))
	{
		return;
	}

	LPOBJ lpObj = &gObj[aIndex];

	PMSG_BANKEX_UPDATE_SEND pMsg;
	pMsg.header.set(0xFB, 0x06, sizeof(pMsg));
	// ----
	pMsg.Money = lpObj->BankEx.Money;
	pMsg.Bless = lpObj->BankEx.JBless;
	pMsg.Soul = lpObj->BankEx.JSoul;
	pMsg.Chaos = lpObj->BankEx.JChaos;
	pMsg.Creation = lpObj->BankEx.JCreation;
	pMsg.Life = lpObj->BankEx.JLife;
	pMsg.Harmony = lpObj->BankEx.JHarmony;
	pMsg.Guardian = lpObj->BankEx.JGuardian;
	pMsg.Gemstone = lpObj->BankEx.JGemstone;
	pMsg.LowRefine = lpObj->BankEx.JLowRef;
	pMsg.HigRefine = lpObj->BankEx.JHigRef;
	// ----
/*	pMsg.Custom001 = lpObj->BankEx.JCustomJ1;
	pMsg.Custom002 = lpObj->BankEx.JCustomJ2;
	pMsg.Custom003 = lpObj->BankEx.JCustomJ3;
	pMsg.Custom004 = lpObj->BankEx.JCustomJ4;
	pMsg.Custom005 = lpObj->BankEx.JCustomJ5;
	pMsg.Custom006 = lpObj->BankEx.JCustomJ6;
	pMsg.Custom007 = lpObj->BankEx.JCustomJ7;
	pMsg.Custom008 = lpObj->BankEx.JCustomJ8;
	pMsg.Custom009 = lpObj->BankEx.JCustomJ9;
	pMsg.Custom010 = lpObj->BankEx.JCustomJ10;
	pMsg.Custom011 = lpObj->BankEx.JCustomJ11;*/
	DataSend(aIndex, (LPBYTE)&pMsg, sizeof(pMsg));
}

//comandos
// ----------------------
//->JewelBank System
// ----------------------
void CBankEx::SendZenCustomVault(LPOBJ lpObj,char* arg) // OK
{
	int Zen = atoi(arg);

	if (Zen < 0)
	{
		Zen = -Zen;
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(874));
		return;
	}

	if ( lpObj->Money >= Zen )
	{
		lpObj->Money -= Zen;
		lpObj->BankEx.Money += Zen;

		GCMoneySend(lpObj->Index, lpObj->Money);
		gBankEx.GDSavePoint(lpObj->Index);
		gBankEx.GCUpdateBankEx(lpObj->Index);
	}
	else
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(875));
	}
}

void CBankEx::SendBlessCustomVault(LPOBJ lpObj,char* arg)
{
	int Count = atoi(arg);

	if (Count < 0)
	{
		Count = -Count;
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(874));
		return;
	}

	int BlessCount = gItemManager.GetInventoryItemCount(lpObj, GET_ITEM(14, 13), 0);

	if ( Count > BlessCount )
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(874));
		return;
	}

	gItemManager.DeleteInventoryItemCount(lpObj, GET_ITEM(14, 13), 0, Count);

	lpObj->BankEx.JBless += Count;

	gBankEx.GDSavePoint(lpObj->Index);
	gBankEx.GCUpdateBankEx(lpObj->Index);
}

void CBankEx::SendCreationCustomVault(LPOBJ lpObj,char* arg)
{
	int Count = atoi(arg);

	if (Count < 0)
	{
		Count = -Count;
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(874));
		return;
	}

	int BlessCount = gItemManager.GetInventoryItemCount(lpObj, GET_ITEM(14, 22), 0);

	if ( Count > BlessCount )
	{
		gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, gMessage.GetMessage(874));
		return;
	}


	gItemManager.DeleteInventoryItemCount(lpObj, GET_ITEM(14, 22), 0, Count);

	lpObj->BankEx.JCreation += Count;

	gBankEx.GDSavePoint(lpObj->Index);
	gBankEx.GCUpdateBankEx(lpObj->Index);
}

void CBankEx::SendSoulCustomVault(LPOBJ lpObj,char* arg)
{
	int Count = atoi(arg);

	if (Count < 0)
	{
		Count = -Count;
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(874));
		return;
	}

	int BlessCount = gItemManager.GetInventoryItemCount(lpObj, GET_ITEM(14, 14), 0);

	if ( Count > BlessCount )
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(874));
		return;
	}


	gItemManager.DeleteInventoryItemCount(lpObj, GET_ITEM(14, 14), 0, Count);


	lpObj->BankEx.JSoul += Count;

	gBankEx.GDSavePoint(lpObj->Index);
	gBankEx.GCUpdateBankEx(lpObj->Index);
}

void CBankEx::SendChaosCustomVault(LPOBJ lpObj,char* arg)
{
	int Count = atoi(arg);

	if (Count < 0)
	{
		Count = -Count;
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(874));
		return;
	}

	int BlessCount = gItemManager.GetInventoryItemCount(lpObj, GET_ITEM(12, 15), 0);

	if ( Count > BlessCount )
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(874));
		return;
	}

	gItemManager.DeleteInventoryItemCount(lpObj, GET_ITEM(12, 15), 0, Count);

	lpObj->BankEx.JChaos += Count;

	gBankEx.GDSavePoint(lpObj->Index);
	gBankEx.GCUpdateBankEx(lpObj->Index);
}

void CBankEx::SendLifeCustomVault(LPOBJ lpObj,char* arg)
{
	int Count = atoi(arg);

	if (Count < 0)
	{
		Count = -Count;
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(874));
		return;
	}

	int BlessCount = gItemManager.GetInventoryItemCount(lpObj, GET_ITEM(14, 16), 0);

	if ( Count > BlessCount )
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(874));
		return;
	}


	gItemManager.DeleteInventoryItemCount(lpObj, GET_ITEM(14, 16), 0, Count);

	lpObj->BankEx.JLife += Count;

	gBankEx.GDSavePoint(lpObj->Index);
	gBankEx.GCUpdateBankEx(lpObj->Index);
}
// ----------------------
//->JewelBank Recv
// ----------------------
void CBankEx::RecvSoulCustomVault(LPOBJ lpObj,char* arg)
{
	int Count = atoi(arg);

	if (Count < 0)
	{
		Count = -Count;
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(874));
		return;
	}

	if ( Count > lpObj->BankEx.JSoul )
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(875));
		return;
	}

	int EmptySlot = gItemManager.GetInventoryEmptySlotCount(lpObj);

	if ( EmptySlot < Count )
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(876));
		return;
	}

	for(int n=0;n < Count;n++)
	{
		GDCreateItemSend(lpObj->Index,0xEB,0,0,GET_ITEM(14, 14),0,0,0,0,0,-1,0,0,0,0,0,0xFF,0);
	}

	lpObj->BankEx.JSoul -= Count;

	gBankEx.GDSavePoint(lpObj->Index);
	gBankEx.GCUpdateBankEx(lpObj->Index);
}

void CBankEx::RecvBlessCustomVault(LPOBJ lpObj,char* arg)
{
	int Count = atoi(arg);

	if (Count < 0)
	{
		Count = -Count;
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(874));
		return;
	}

	if ( Count > lpObj->BankEx.JBless )
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(875));
		return;
	}

	int EmptySlot = gItemManager.GetInventoryEmptySlotCount(lpObj);

	if ( EmptySlot < Count )
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(876));
		return;
	}

	for(int n=0;n < Count;n++)
	{
		GDCreateItemSend(lpObj->Index,0xEB,0,0,GET_ITEM(14, 13),0,0,0,0,0,-1,0,0,0,0,0,0xFF,0);
	}

	lpObj->BankEx.JBless -= Count;

	gBankEx.GDSavePoint(lpObj->Index);
	gBankEx.GCUpdateBankEx(lpObj->Index);
}

void CBankEx::RecvChaosCustomVault(LPOBJ lpObj,char* arg)
{
	int Count = atoi(arg);

	if (Count < 0)
	{
		Count = -Count;
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(874));
		return;
	}

	if ( Count > lpObj->BankEx.JChaos )
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(875));
		return;
	}

	int EmptySlot = gItemManager.GetInventoryEmptySlotCount(lpObj);

	if ( EmptySlot < Count )
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(876));
		return;
	}


	for(int n=0;n < Count;n++)
	{
		GDCreateItemSend(lpObj->Index,0xEB,0,0,GET_ITEM(12, 15),0,0,0,0,0,-1,0,0,0,0,0,0xFF,0);
	}

	lpObj->BankEx.JChaos -= Count;

	gBankEx.GDSavePoint(lpObj->Index);
	gBankEx.GCUpdateBankEx(lpObj->Index);
}

void CBankEx::RecvLifeCustomVault(LPOBJ lpObj,char* arg)
{
	int Count = atoi(arg);

	if (Count < 0)
	{
		Count = -Count;
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(874));
		return;
	}

	if ( Count > lpObj->BankEx.JLife )
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(875));
		return;
	}

	int EmptySlot = gItemManager.GetInventoryEmptySlotCount(lpObj);

	if ( EmptySlot < Count )
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(876));
		return;
	}

	for(int n=0;n < Count;n++)
	{
		GDCreateItemSend(lpObj->Index, 0xEB, 0, 0, GET_ITEM(14, 16), 0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0xFF, 0);
	}

	lpObj->BankEx.JLife -= Count;

	gBankEx.GDSavePoint(lpObj->Index);
	gBankEx.GCUpdateBankEx(lpObj->Index);
}

void CBankEx::RecvCreationCustomVault(LPOBJ lpObj,char* arg)
{
	int Count = atoi(arg);

	if (Count < 0)
	{
		Count = -Count;
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(874));
		return;
	}

	if ( Count > lpObj->BankEx.JCreation )
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(875));
		return;
	}

	int EmptySlot = gItemManager.GetInventoryEmptySlotCount(lpObj);

	if ( EmptySlot < Count )
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(876));
		return;
	}

	for(int n=0;n < Count;n++)
	{
		GDCreateItemSend(lpObj->Index, 0xEB, 0, 0, GET_ITEM(14, 22), 0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0xFF, 0);
	}

	lpObj->BankEx.JCreation -= Count;

	gBankEx.GDSavePoint(lpObj->Index);
	gBankEx.GCUpdateBankEx(lpObj->Index);
}

void CBankEx::RecvZenCustomVault(LPOBJ lpObj,char* arg) // OK
{
	int Zen = atoi(arg);

	if (Zen < 0)
	{
		Zen = -Zen;
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(874));
		return;
	}
	if ( lpObj->BankEx.Money >= Zen )
	{

		if ( lpObj->Money + Zen > MAX_MONEY )
		{
			gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(877), MAX_MONEY);
			return;
		}

		lpObj->Money += Zen;
		lpObj->BankEx.Money -= Zen;

		GCMoneySend(lpObj->Index, lpObj->Money);
		gBankEx.GDSavePoint(lpObj->Index);
		gBankEx.GCUpdateBankEx(lpObj->Index);
	}
	else
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(875));
	}
}

void CBankEx::RecvHarmonyCustomVault(LPOBJ lpObj,char* arg)
{
	int Count = atoi(arg);

	if (Count < 0)
	{
		Count = -Count;
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(874));
		return;
	}

	if ( Count > lpObj->BankEx.JHarmony )
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(875));
		return;
	}

	int EmptySlot = gItemManager.GetInventoryEmptySlotCount(lpObj);

	if ( EmptySlot < Count )
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(876));
		return;
	}

	for(int n=0;n < Count;n++)
	{
		GDCreateItemSend(lpObj->Index, 0xEB, 0, 0, GET_ITEM(14, 42), 0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0xFF, 0);
	}

	lpObj->BankEx.JHarmony -= Count;

	gBankEx.GDSavePoint(lpObj->Index);
	gBankEx.GCUpdateBankEx(lpObj->Index);
}

void CBankEx::SendHarmonyCustomVault(LPOBJ lpObj,char* arg)
{
	int Count = atoi(arg);

	if (Count < 0)
	{
		Count = -Count;
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(874));
		return;
	}

	int HarmonyCount = gItemManager.GetInventoryItemCount(lpObj, GET_ITEM(14, 42), 0);

	if ( Count > HarmonyCount )
	{
		gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, gMessage.GetMessage(874));
		return;
	}


	gItemManager.DeleteInventoryItemCount(lpObj, GET_ITEM(14, 42), 0, Count);

	lpObj->BankEx.JHarmony += Count;

	gBankEx.GDSavePoint(lpObj->Index);
	gBankEx.GCUpdateBankEx(lpObj->Index);
}

void CBankEx::RecvGuardianCustomVault(LPOBJ lpObj,char* arg)
{
	int Count = atoi(arg);

	if (Count < 0)
	{
		Count = -Count;
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(874));
		return;
	}

	if ( Count > lpObj->BankEx.JGuardian )
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(875));
		return;
	}

	int EmptySlot = gItemManager.GetInventoryEmptySlotCount(lpObj);

	if ( EmptySlot < Count )
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(876));
		return;
	}

	for(int n=0;n < Count;n++)
	{
		GDCreateItemSend(lpObj->Index, 0xEB, 0, 0, GET_ITEM(14, 31), 0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0xFF, 0);
	}

	lpObj->BankEx.JGuardian -= Count;

	gBankEx.GDSavePoint(lpObj->Index);
	gBankEx.GCUpdateBankEx(lpObj->Index);
}

void CBankEx::SendGuardianCustomVault(LPOBJ lpObj,char* arg)
{
	int Count = atoi(arg);

	if (Count < 0)
	{
		Count = -Count;
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(874));
		return;
	}

	int HarmonyCount = gItemManager.GetInventoryItemCount(lpObj, GET_ITEM(14, 31), 0);

	if ( Count > HarmonyCount )
	{
		gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, gMessage.GetMessage(874));
		return;
	}


	gItemManager.DeleteInventoryItemCount(lpObj, GET_ITEM(14, 31), 0, Count);

	lpObj->BankEx.JGuardian += Count;

	gBankEx.GDSavePoint(lpObj->Index);
	gBankEx.GCUpdateBankEx(lpObj->Index);
}

void CBankEx::RecvGemstoneCustomVault(LPOBJ lpObj,char* arg)
{
	int Count = atoi(arg);

	if (Count < 0)
	{
		Count = -Count;
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(874));
		return;
	}

	if ( Count > lpObj->BankEx.JGemstone )
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(875));
		return;
	}

	int EmptySlot = gItemManager.GetInventoryEmptySlotCount(lpObj);

	if ( EmptySlot < Count )
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(876));
		return;
	}

	for(int n=0;n < Count;n++)
	{
		GDCreateItemSend(lpObj->Index, 0xEB, 0, 0, GET_ITEM(14, 41), 0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0xFF, 0);
	}

	lpObj->BankEx.JGemstone -= Count;

	gBankEx.GDSavePoint(lpObj->Index);
	gBankEx.GCUpdateBankEx(lpObj->Index);
}

void CBankEx::SendGemstoneCustomVault(LPOBJ lpObj,char* arg)
{
	int Count = atoi(arg);

	if (Count < 0)
	{
		Count = -Count;
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(874));
		return;
	}

	int HarmonyCount = gItemManager.GetInventoryItemCount(lpObj, GET_ITEM(14, 41), 0);

	if ( Count > HarmonyCount )
	{
		gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, gMessage.GetMessage(874));
		return;
	}


	gItemManager.DeleteInventoryItemCount(lpObj, GET_ITEM(14, 41), 0, Count);

	lpObj->BankEx.JGemstone += Count;

	gBankEx.GDSavePoint(lpObj->Index);
	gBankEx.GCUpdateBankEx(lpObj->Index);
}

void CBankEx::RecvLowerRefinCustomVault(LPOBJ lpObj,char* arg)
{
	int Count = atoi(arg);

	if (Count < 0)
	{
		Count = -Count;
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(874));
		return;
	}

	if ( Count > lpObj->BankEx.JLowRef )
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(875));
		return;
	}

	int EmptySlot = gItemManager.GetInventoryEmptySlotCount(lpObj);

	if ( EmptySlot < Count )
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(876));
		return;
	}

	for(int n=0;n < Count;n++)
	{
		GDCreateItemSend(lpObj->Index, 0xEB, 0, 0, GET_ITEM(14, 43), 0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0xFF, 0);
	}

	lpObj->BankEx.JLowRef -= Count;

	gBankEx.GDSavePoint(lpObj->Index);
	gBankEx.GCUpdateBankEx(lpObj->Index);
}

void CBankEx::SendLowerRefinCustomVault(LPOBJ lpObj,char* arg)
{
	int Count = atoi(arg);

	if (Count < 0)
	{
		Count = -Count;
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(874));
		return;
	}

	int HarmonyCount = gItemManager.GetInventoryItemCount(lpObj, GET_ITEM(14, 43), 0);

	if ( Count > HarmonyCount )
	{
		gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, gMessage.GetMessage(874));
		return;
	}


	gItemManager.DeleteInventoryItemCount(lpObj, GET_ITEM(14, 43), 0, Count);

	lpObj->BankEx.JLowRef += Count;

	gBankEx.GDSavePoint(lpObj->Index);
	gBankEx.GCUpdateBankEx(lpObj->Index);
}

void CBankEx::RecvHighRefinCustomVault(LPOBJ lpObj,char* arg)
{
	int Count = atoi(arg);

	if (Count < 0)
	{
		Count = -Count;
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(874));
		return;
	}

	if ( Count > lpObj->BankEx.JHigRef )
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(875));
		return;
	}

	int EmptySlot = gItemManager.GetInventoryEmptySlotCount(lpObj);

	if ( EmptySlot < Count )
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(876));
		return;
	}

	for(int n=0;n < Count;n++)
	{
		GDCreateItemSend(lpObj->Index, 0xEB, 0, 0, GET_ITEM(14, 44), 0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0xFF, 0);
	}

	lpObj->BankEx.JHigRef-= Count;

	gBankEx.GDSavePoint(lpObj->Index);
	gBankEx.GCUpdateBankEx(lpObj->Index);
}

void CBankEx::SendHighRefinCustomVault(LPOBJ lpObj,char* arg)
{
	int Count = atoi(arg);

	if (Count < 0)
	{
		Count = -Count;
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(874));
		return;
	}

	int HarmonyCount = gItemManager.GetInventoryItemCount(lpObj, GET_ITEM(14, 44), 0);

	if ( Count > HarmonyCount )
	{
		gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, gMessage.GetMessage(874));
		return;
	}


	gItemManager.DeleteInventoryItemCount(lpObj, GET_ITEM(14, 44), 0, Count);

	lpObj->BankEx.JHigRef += Count;

	gBankEx.GDSavePoint(lpObj->Index);
	gBankEx.GCUpdateBankEx(lpObj->Index);
}

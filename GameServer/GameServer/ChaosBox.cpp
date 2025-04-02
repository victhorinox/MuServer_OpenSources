// ChaosBox.cpp: implementation of the CChaosBox class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ChaosBox.h"
#include "CastleSiegeSync.h"
#include "CustomWingMix.h"
#include "DSProtocol.h"
#include "ItemBagManager.h"
#include "ItemOptionRate.h"
#include "JewelOfHarmonyOption.h"
#include "JewelOfHarmonyType.h"
#include "Log.h"
#include "LuckyItem.h"
#include "Notice.h"
#include "PentagramSystem.h"
#include "RandomManager.h"
#include "ServerInfo.h"
#include "SetItemType.h"
#include "Shop.h"
#include "SocketItemOption.h"
#include "SocketItemType.h"
#include "Util.h"
#include "CustomMix.h"

CChaosBox gChaosBox;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CChaosBox::CChaosBox() // OK
{
	this->m_SeniorMixLimitDay = 0;

	this->m_SeniorMixLimitMonth = 0;

	this->m_SeniorMixLimitYear = 0;
}

CChaosBox::~CChaosBox() // OK
{

}

void CChaosBox::ChaosBoxInit(LPOBJ lpObj) // OK
{
	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		lpObj->ChaosBox[n].Clear();
		lpObj->ChaosBoxMap[n] = 0xFF;
	}
}

void CChaosBox::ChaosBoxItemDown(LPOBJ lpObj,int slot) // OK
{
	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(n != slot)
		{
			lpObj->ChaosBox[n].Clear();
			lpObj->ChaosBoxMap[n] = 0xFF;
			continue;
		}

		CItem* lpItem = &lpObj->ChaosBox[n];
		
		lpItem->m_Level = 0;

		float dur = (float)gItemManager.GetItemDurability(lpItem->m_Index,lpItem->m_Level,lpItem->IsExcItem(),lpItem->IsSetItem());

		lpItem->m_Durability = dur*(lpItem->m_Durability/lpItem->m_BaseDurability);

		lpItem->Convert(lpItem->m_Index,lpItem->m_Option1,lpItem->m_Option2,lpItem->m_Option3,lpItem->m_NewOption,lpItem->m_SetOption,lpItem->m_JewelOfHarmonyOption,lpItem->m_ItemOptionEx,lpItem->m_SocketOption,lpItem->m_SocketOptionBonus);
	}
}

void CChaosBox::ChaosBoxItemKeep(LPOBJ lpObj,int slot) // OK
{
	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(n != slot)
		{
			lpObj->ChaosBox[n].Clear();
			lpObj->ChaosBoxMap[n] = 0xFF;
			continue;
		}
	}
}

void CChaosBox::ChaosBoxItemSave(LPOBJ lpObj) // OK
{
	if(gObjInventoryCommit(lpObj->Index) != 0)
	{
		for(int n=0;n < CHAOS_BOX_SIZE;n++)
		{
			if(lpObj->ChaosBox[n].IsItem() != 0)
			{
				gItemManager.InventoryInsertItem(lpObj->Index,lpObj->ChaosBox[n]);
			}
		}
	}
}

bool CChaosBox::GetTalismanOfLuckRate(LPOBJ lpObj,int* rate) // OK
{
	int count = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,53)) // Talisman of Luck
		{
			count += (int)lpObj->ChaosBox[n].m_Durability;
		}
	}

	(*rate) += count;

	return ((count>MAX_TALISMAN_OF_LUCK)?0:1);
}

bool CChaosBox::GetElementalTalismanOfLuckRate(LPOBJ lpObj,int* rate) // OK
{
	int count = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,189)) // Elemental Talisman of Luck
		{
			count += (int)lpObj->ChaosBox[n].m_Durability;
		}
	}

	(*rate) += count;

	return ((count>MAX_TALISMAN_OF_LUCK)?0:1);
}

void CChaosBox::ChaosItemMix(LPOBJ lpObj) // OK
{
	int ChaosCount = 0;
	int ItemCount = 0;
	int ItemMoney = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
			lpObj->ChaosBox[n].OldValue();
			ItemMoney += lpObj->ChaosBox[n].m_OldBuyMoney;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,13) || lpObj->ChaosBox[n].m_Index == GET_ITEM(14,14))
		{
			ItemCount++;
			lpObj->ChaosBox[n].OldValue();
			ItemMoney += lpObj->ChaosBox[n].m_OldBuyMoney;
		}
		else if(lpObj->ChaosBox[n].m_Level >= 4 && lpObj->ChaosBox[n].m_Option3 >= 1)
		{
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
	}

	if(ChaosCount == 0 || ItemCount == 0)
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	if(gServerInfo.m_ChaosItemMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney/20000);
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_ChaosItemMixRate[lpObj->AccountLevel];
	}

	if(this->GetTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	lpObj->ChaosMoney = lpObj->ChaosSuccessRate*10000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemLevel = 0;
		BYTE ItemOption1 = 0;
		BYTE ItemOption2 = 0;
		BYTE ItemOption3 = 0;

		CRandomManager RandomManager;

		RandomManager.AddElement(GET_ITEM(2,6),1);

		RandomManager.AddElement(GET_ITEM(4,6),1);

		RandomManager.AddElement(GET_ITEM(5,7),1);

		RandomManager.GetRandomElement(&ItemIndex);

		gItemOptionRate.GetItemOption0(3,&ItemLevel);

		gItemOptionRate.GetItemOption1(3,&ItemOption1);

		gItemOptionRate.GetItemOption2(3,&ItemOption2);

		gItemOptionRate.GetItemOption3(3,&ItemOption3);

		GDCreateItemSend(lpObj->Index,0xFF,0,0,ItemIndex,ItemLevel,0,ItemOption1,ItemOption2,ItemOption3,-1,0,0,0,0,0,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[ChaosItemMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[ChaosItemMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
}

void CChaosBox::DevilSquareMix(LPOBJ lpObj) // OK
{
	int ChaosCount = 0;
	int EyeCount = 0;
	int EyeLevel = 0;
	int KeyCount = 0;
	int KeyLevel = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,17))
		{
			EyeCount++;
			EyeLevel = lpObj->ChaosBox[n].m_Level;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,18))
		{
			KeyCount++;
			KeyLevel = lpObj->ChaosBox[n].m_Level;
		}
	}

	if(ChaosCount != 1 || EyeCount != 1 || KeyCount != 1 || EyeLevel != KeyLevel)
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	if(EyeLevel > 7 || KeyLevel > 7)
	{
		this->GCChaosMixSend(lpObj->Index,8,0);
		return;
	}

	switch(EyeLevel)
	{
		case 1:
			lpObj->ChaosSuccessRate = gServerInfo.m_DevilSquareMixRate1[lpObj->AccountLevel];
			lpObj->ChaosMoney = 100000;
			break;
		case 2:
			lpObj->ChaosSuccessRate = gServerInfo.m_DevilSquareMixRate2[lpObj->AccountLevel];
			lpObj->ChaosMoney = 200000;
			break;
		case 3:
			lpObj->ChaosSuccessRate = gServerInfo.m_DevilSquareMixRate3[lpObj->AccountLevel];
			lpObj->ChaosMoney = 400000;
			break;
		case 4:
			lpObj->ChaosSuccessRate = gServerInfo.m_DevilSquareMixRate4[lpObj->AccountLevel];
			lpObj->ChaosMoney = 700000;
			break;
		case 5:
			lpObj->ChaosSuccessRate = gServerInfo.m_DevilSquareMixRate5[lpObj->AccountLevel];
			lpObj->ChaosMoney = 1100000;
			break;
		case 6:
			lpObj->ChaosSuccessRate = gServerInfo.m_DevilSquareMixRate6[lpObj->AccountLevel];
			lpObj->ChaosMoney = 1600000;
			break;
		case 7:
			lpObj->ChaosSuccessRate = gServerInfo.m_DevilSquareMixRate7[lpObj->AccountLevel];
			lpObj->ChaosMoney = 2000000;
			break;
	}

	if(this->GetTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		GDCreateItemSend(lpObj->Index,0xFF,0,0,GET_ITEM(14,19),EyeLevel,1,0,0,0,-1,0,0,0,0,0,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[DevilSquareMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[DevilSquareMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
}

void CChaosBox::PlusItemLevelMix(LPOBJ lpObj,int type) // OK
{
	int ChaosCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int ItemCount = 0;
	int ItemSlot = 0;
	int ChaosAmulet = 0;
	int ElementalChaosAmulet = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,13))
		{
			BlessCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,14))
		{
			SoulCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,96))
		{
			ChaosAmulet++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,190))
		{
			ElementalChaosAmulet++;
		}
		else if(lpObj->ChaosBox[n].m_Level == (9+type))
		{
			ItemCount++;
			ItemSlot = n;
		}
	}

	if(ChaosCount != 1 || SoulCount < (type+1) || BlessCount < (type+1) || ItemCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	switch(type)
	{
		case 0:
			lpObj->ChaosSuccessRate = gServerInfo.m_PlusItemLevelMixRate1[lpObj->AccountLevel];
			break;
		case 1:
			lpObj->ChaosSuccessRate = gServerInfo.m_PlusItemLevelMixRate2[lpObj->AccountLevel];
			break;
		case 2:
			lpObj->ChaosSuccessRate = gServerInfo.m_PlusItemLevelMixRate3[lpObj->AccountLevel];
			break;
		case 3:
			lpObj->ChaosSuccessRate = gServerInfo.m_PlusItemLevelMixRate4[lpObj->AccountLevel];
			break;
		case 4:
			lpObj->ChaosSuccessRate = gServerInfo.m_PlusItemLevelMixRate5[lpObj->AccountLevel];
			break;
		case 5:
			lpObj->ChaosSuccessRate = gServerInfo.m_PlusItemLevelMixRate6[lpObj->AccountLevel];
			break;
	}

	if(lpObj->ChaosBox[ItemSlot].m_Option2 != 0)
	{
		lpObj->ChaosSuccessRate += gServerInfo.m_AddLuckSuccessRate2[lpObj->AccountLevel];
	}

	if(lpObj->ChaosBox[ItemSlot].IsPentagramItem() == 0 && this->GetTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	if(lpObj->ChaosBox[ItemSlot].IsPentagramItem() != 0 && this->GetElementalTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	lpObj->ChaosMoney = 2000000*(type+1);

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		CItem item = lpObj->ChaosBox[ItemSlot];

		item.m_Level++;

		item.m_Durability = (float)gItemManager.GetItemDurability(item.m_Index,item.m_Level,item.IsExcItem(),item.IsSetItem());

		item.m_Durability = (item.m_Durability*lpObj->ChaosBox[ItemSlot].m_Durability)/item.m_BaseDurability;

		this->ChaosBoxInit(lpObj);

		gItemManager.ChaosBoxAddItem(lpObj->Index,item,0);

		this->GCChaosMixSend(lpObj->Index,1,&item);

		//gObjCustomLogPlusChaosMix(lpObj,type,item.m_Index);

		gObjCustomLogPlusChaosMix10(lpObj,type,item.m_Index);

		gObjCustomLogPlusChaosMix11(lpObj,type,item.m_Index);

		gObjCustomLogPlusChaosMix12(lpObj,type,item.m_Index);

		gObjCustomLogPlusChaosMix13(lpObj,type,item.m_Index);

		gObjCustomLogPlusChaosMix14(lpObj,type,item.m_Index);

		gObjCustomLogPlusChaosMix15(lpObj,type,item.m_Index);

		gLog.Output(LOG_CHAOS_MIX,"[PlusItemLevelMix][Success][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,type,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		if(ChaosAmulet == 0 && ElementalChaosAmulet == 0)
		{
			#if(GAMESERVER_UPDATE>=701)

			gPentagramSystem.DelAllPentagramJewelInfo(lpObj,&lpObj->ChaosBox[ItemSlot],0);

			#endif

			this->ChaosBoxInit(lpObj);

			this->GCChaosBoxSend(lpObj,0);

			this->GCChaosMixSend(lpObj->Index,0,0);

			gLog.Output(LOG_CHAOS_MIX,"[PlusItemLevelMix][Failure][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d, ChaosAmulet: %d)",lpObj->Account,lpObj->Name,type,lpObj->ChaosSuccessRate,lpObj->ChaosMoney,(ChaosAmulet+ElementalChaosAmulet));
		}
		else
		{
			this->ChaosBoxItemDown(lpObj,ItemSlot);

			this->GCChaosBoxSend(lpObj,0);

			this->GCChaosMixSend(lpObj->Index,0,0);

			gLog.Output(LOG_CHAOS_MIX,"[PlusItemLevelMix][Failure][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d, ChaosAmulet: %d)",lpObj->Account,lpObj->Name,type,lpObj->ChaosSuccessRate,lpObj->ChaosMoney,(ChaosAmulet+ElementalChaosAmulet));
		}
	}
}

void CChaosBox::DinorantMix(LPOBJ lpObj) // OK
{
	int ChaosCount = 0;
	int UniriaCount = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,2) && lpObj->ChaosBox[n].m_Durability == 255)
		{
			UniriaCount++;
		}
	}

	if(ChaosCount != 1 || UniriaCount != 10)
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	lpObj->ChaosSuccessRate = gServerInfo.m_DinorantMixRate[lpObj->AccountLevel];

	if(this->GetTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	lpObj->ChaosMoney = 500000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = GET_ITEM(13,3);
		BYTE ItemNewOption = 0;

		gItemOptionRate.GetItemOption4(3,&ItemNewOption);

		gItemOptionRate.MakeNewOption(ItemIndex,ItemNewOption,&ItemNewOption);

		GDCreateItemSend(lpObj->Index,0xFF,0,0,ItemIndex,0,255,0,0,0,-1,ItemNewOption,0,0,0,0,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[DinorantMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[DinorantMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
}

void CChaosBox::FruitMix(LPOBJ lpObj) // OK
{
	int ChaosCount = 0;
	int CreationCount = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,22))
		{
			CreationCount++;
		}
	}

	if(ChaosCount != 1 || CreationCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	lpObj->ChaosSuccessRate = gServerInfo.m_FruitMixRate[lpObj->AccountLevel];

	if(this->GetTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	lpObj->ChaosMoney = 3000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		BYTE ItemLevel = GetLargeRand()%5;

		GDCreateItemSend(lpObj->Index,0xFF,0,0,GET_ITEM(13,15),ItemLevel,0,0,0,0,-1,0,0,0,0,0,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[FruitMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[FruitMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
}

void CChaosBox::Wing2Mix(LPOBJ lpObj,int type) // OK
{
	int ChaosCount = 0;
	int FeatherCount = 0;
	int SleeveCount = 0;
	int WingItemCount = 0;
	int WingItemMoney = 0;
	int ItemCount = 0;
	int ItemMoney = 0;
	int TalismanOfWingType = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,14) && lpObj->ChaosBox[n].m_Level == 0)
		{
			FeatherCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,14) && lpObj->ChaosBox[n].m_Level == 1)
		{
			SleeveCount++;
		}
		else if((lpObj->ChaosBox[n].m_Index >= GET_ITEM(12,0) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(12,2)) || lpObj->ChaosBox[n].m_Index == GET_ITEM(12,41))
		{
			WingItemCount++;
			WingItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
		else if(lpObj->ChaosBox[n].IsExcItem() != 0 && lpObj->ChaosBox[n].m_Level >= 4)
		{
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
		else if(lpObj->ChaosBox[n].m_Index >= GET_ITEM(13,88) && lpObj->ChaosBox[n].m_Index >= GET_ITEM(13,92))
		{
			TalismanOfWingType = lpObj->ChaosBox[n].m_Index-GET_ITEM(13,87);
		}
	}

	if(ChaosCount != 1 || (type == 0 && FeatherCount != 1) || (type == 1 && SleeveCount != 1) || WingItemCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	if(gServerInfo.m_Wing2MixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (WingItemMoney/4000000)+(ItemMoney/40000);
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_Wing2MixRate[lpObj->AccountLevel];
	}

	if(this->GetTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	if(gServerInfo.m_Wing2MixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>90)?90:lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 5000000;
	
	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		if(type == 0)
		{
			WORD ItemIndex = 0;
			BYTE ItemOption2 = 0;
			BYTE ItemOption3 = 0;
			BYTE ItemNewOption = 0;

			CRandomManager RandomManager;

			RandomManager.AddElement(GET_ITEM(12,3),1);

			RandomManager.AddElement(GET_ITEM(12,4),1);

			RandomManager.AddElement(GET_ITEM(12,5),1);

			RandomManager.AddElement(GET_ITEM(12,6),1);

			if (gServerInfo.m_DisableWingMixSU != 1)
			{
				RandomManager.AddElement(GET_ITEM(12,42),1);
			}

			RandomManager.GetRandomElement(&ItemIndex);

			switch(TalismanOfWingType)
			{
				case 1:
					ItemIndex = GET_ITEM(12,5);
					break;
				case 2:
					ItemIndex = GET_ITEM(12,4);
					break;
				case 3:
					ItemIndex = GET_ITEM(12,3);
					break;
				case 4:
					if (gServerInfo.m_DisableWingMixSU != 1)
					{
						ItemIndex = GET_ITEM(12,42); // Asa de SU
					}
					else 
					{
						ItemIndex = GET_ITEM(12,5); // Asa de SM
					}
					break;
				case 5:
					ItemIndex = GET_ITEM(12,6);
					break;
			}

			gItemOptionRate.GetItemOption2(4,&ItemOption2);

			gItemOptionRate.GetItemOption3(4,&ItemOption3);

			gItemOptionRate.GetItemOption4(4,&ItemNewOption);

			gItemOptionRate.MakeNewOption(ItemIndex,ItemNewOption,&ItemNewOption);

			GDCreateItemSend(lpObj->Index,0xFF,0,0,ItemIndex,0,0,0,ItemOption2,ItemOption3,-1,(ItemNewOption+(32*(GetLargeRand()%2))),0,0,0,0,0xFF,0);

			gLog.Output(LOG_CHAOS_MIX,"[Wing2Mix][Success][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,type,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
		}
		else
		{
			WORD ItemIndex = 0;
			BYTE ItemOption2 = 0;
			BYTE ItemOption3 = 0;
			BYTE ItemNewOption = 0;

			CRandomManager RandomManager;

			#if(GAMESERVER_UPDATE>=601)

			if (gServerInfo.m_DisableWingMixRF != 1)
			{
				RandomManager.AddElement(GET_ITEM(12,49),1);
			}

			#endif

			if (gServerInfo.m_DisableWingMixDL != 1)
			{
				RandomManager.AddElement(GET_ITEM(13,30),1);
			}
			else
			{
				RandomManager.AddElement(GET_ITEM(12,3),1);
			}

			RandomManager.GetRandomElement(&ItemIndex);

			gItemOptionRate.GetItemOption2(4,&ItemOption2);

			gItemOptionRate.GetItemOption3(4,&ItemOption3);

			gItemOptionRate.GetItemOption4(5,&ItemNewOption);

			gItemOptionRate.MakeNewOption(ItemIndex,ItemNewOption,&ItemNewOption);

			GDCreateItemSend(lpObj->Index,0xFF,0,0,ItemIndex,0,0,0,ItemOption2,ItemOption3,-1,(ItemNewOption+32),0,0,0,0,0xFF,0);

			gLog.Output(LOG_CHAOS_MIX,"[Wing2Mix][Success][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,type,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
		}
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[Wing2Mix][Failure][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,type,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
}

void CChaosBox::BloodCastleMix(LPOBJ lpObj) // OK
{
	int ChaosCount = 0;
	int ScrollCount = 0;
	int ScrollLevel = 0;
	int BoneCount = 0;
	int BoneLevel = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,16))
		{
			ScrollCount++;
			ScrollLevel = lpObj->ChaosBox[n].m_Level;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,17))
		{
			BoneCount++;
			BoneLevel = lpObj->ChaosBox[n].m_Level;
		}
	}

	if(ChaosCount != 1 || ScrollCount != 1 || BoneCount != 1 || ScrollLevel != BoneLevel)
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	if(ScrollLevel > 8 || BoneLevel > 8)
	{
		this->GCChaosMixSend(lpObj->Index,8,0);
		return;
	}

	switch(ScrollLevel)
	{
		case 1:
			lpObj->ChaosSuccessRate = gServerInfo.m_BloodCastleMixRate1[lpObj->AccountLevel];
			lpObj->ChaosMoney = 50000;
			break;
		case 2:
			lpObj->ChaosSuccessRate = gServerInfo.m_BloodCastleMixRate2[lpObj->AccountLevel];
			lpObj->ChaosMoney = 80000;
			break;
		case 3:
			lpObj->ChaosSuccessRate = gServerInfo.m_BloodCastleMixRate3[lpObj->AccountLevel];
			lpObj->ChaosMoney = 150000;
			break;
		case 4:
			lpObj->ChaosSuccessRate = gServerInfo.m_BloodCastleMixRate4[lpObj->AccountLevel];
			lpObj->ChaosMoney = 250000;
			break;
		case 5:
			lpObj->ChaosSuccessRate = gServerInfo.m_BloodCastleMixRate5[lpObj->AccountLevel];
			lpObj->ChaosMoney = 400000;
			break;
		case 6:
			lpObj->ChaosSuccessRate = gServerInfo.m_BloodCastleMixRate6[lpObj->AccountLevel];
			lpObj->ChaosMoney = 600000;
			break;
		case 7:
			lpObj->ChaosSuccessRate = gServerInfo.m_BloodCastleMixRate7[lpObj->AccountLevel];
			lpObj->ChaosMoney = 850000;
			break;
		case 8:
			lpObj->ChaosSuccessRate = gServerInfo.m_BloodCastleMixRate8[lpObj->AccountLevel];
			lpObj->ChaosMoney = 1050000;
			break;
	}

	if(this->GetTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) <= lpObj->ChaosSuccessRate)
	{
		GDCreateItemSend(lpObj->Index,0xFF,0,0,GET_ITEM(13,18),ScrollLevel,1,0,0,0,-1,0,0,0,0,0,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[BloodCastleMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[BloodCastleMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
}

void CChaosBox::Wing1Mix(LPOBJ lpObj) // OK
{
	int ChaosCount = 0;
	int ChaosItem = 0;
	int ItemCount = 0;
	int ItemMoney = 0;
	int TalismanOfWingType = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
			lpObj->ChaosBox[n].OldValue();
			ItemMoney += lpObj->ChaosBox[n].m_OldBuyMoney;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,13) || lpObj->ChaosBox[n].m_Index == GET_ITEM(14,14))
		{
			ItemCount++;
			lpObj->ChaosBox[n].OldValue();
			ItemMoney += lpObj->ChaosBox[n].m_OldBuyMoney;
		}
		else if((lpObj->ChaosBox[n].m_Index == GET_ITEM(2,6) || lpObj->ChaosBox[n].m_Index == GET_ITEM(4,6) || lpObj->ChaosBox[n].m_Index == GET_ITEM(5,7)) && lpObj->ChaosBox[n].m_Level >= 4 && lpObj->ChaosBox[n].m_Option3 >= 1)
		{
			ChaosItem++;
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
		else if(lpObj->ChaosBox[n].m_Level >= 4 && lpObj->ChaosBox[n].m_Option3 >= 1)
		{
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
		else if(lpObj->ChaosBox[n].m_Index >= GET_ITEM(13,83) && lpObj->ChaosBox[n].m_Index >= GET_ITEM(13,86))
		{
			TalismanOfWingType = lpObj->ChaosBox[n].m_Index-GET_ITEM(13,82);
		}
	}

	if(ChaosCount == 0 || ChaosItem == 0)
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	if(gServerInfo.m_Wing1MixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney/20000);
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_Wing1MixRate[lpObj->AccountLevel];
	}

	if(this->GetTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	lpObj->ChaosMoney = lpObj->ChaosSuccessRate*10000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;
		BYTE ItemOption3 = 0;

		CRandomManager RandomManager;

		RandomManager.AddElement(GET_ITEM(12,0),1);

		RandomManager.AddElement(GET_ITEM(12,1),1);

		RandomManager.AddElement(GET_ITEM(12,2),1);

		if (gServerInfo.m_DisableWingMixSU != 1)
		{
			RandomManager.AddElement(GET_ITEM(12,41),1);
		}

		RandomManager.GetRandomElement(&ItemIndex);

		switch(TalismanOfWingType)
		{
			case 1:
				ItemIndex = GET_ITEM(12,2);
				break;
			case 2:
				ItemIndex = GET_ITEM(12,1);
				break;
			case 3:
				ItemIndex = GET_ITEM(12,0);
				break;
			case 4:
				if (gServerInfo.m_DisableWingMixSU != 1)
				{
					ItemIndex = GET_ITEM(12,41); //asa de SU
				}
				else
				{
					ItemIndex = GET_ITEM(12,0); // Asa de SM
				}
				break;
		}

		gItemOptionRate.GetItemOption2(5,&ItemOption2);

		gItemOptionRate.GetItemOption3(5,&ItemOption3);

		GDCreateItemSend(lpObj->Index,0xFF,0,0,ItemIndex,0,0,0,ItemOption2,ItemOption3,-1,0,0,0,0,0,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[Wing1Mix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[Wing1Mix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
}

void CChaosBox::PetMix(LPOBJ lpObj,int type) // OK
{
	int ChaosCount = 0;
	int SoulOfDarkHorseCount = 0;
	int SoulOfDarkSpiritCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int CreationCount = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,31) && lpObj->ChaosBox[n].m_Level == 0)
		{
			SoulOfDarkHorseCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,31) && lpObj->ChaosBox[n].m_Level == 1)
		{
			SoulOfDarkSpiritCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,13))
		{
			BlessCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,14))
		{
			SoulCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,22))
		{
			CreationCount++;
		}
	}

	if(ChaosCount != 1 || (type == 0 && SoulOfDarkHorseCount != 1) || (type == 1 && SoulOfDarkSpiritCount != 1) || BlessCount != (5-(type*3)) || SoulCount != (5-(type*3)) || CreationCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	lpObj->ChaosSuccessRate = gServerInfo.m_PetMixRate[lpObj->AccountLevel];

	if(this->GetTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	lpObj->ChaosMoney = 5000000-(4000000*type);

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		GDCreateItemSend(lpObj->Index,0xFE,0,0,(GET_ITEM(13,4)+type),0,0,0,0,0,-1,0,0,0,0,0,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[PetMix][Success][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,type,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,1);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[PetMix][Failure][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,type,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
}

void CChaosBox::SiegePotionMix(LPOBJ lpObj,int type) // OK
{
	int BlessCount = 0;
	int SoulCount = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,13))
		{
			BlessCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,14))
		{
			SoulCount++;
		}
	}

	if((type == 0 && (BlessCount == 0 || BlessCount > 25)) || (type == 1 && (SoulCount == 0 || SoulCount > 25)))
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	lpObj->ChaosMoney = 100000-(50000*type);

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	GDCreateItemSend(lpObj->Index,0xFF,0,0,GET_ITEM(14,7),type,((BlessCount+SoulCount)*10),0,0,0,-1,0,0,0,0,0,0xFF,0);

	gLog.Output(LOG_CHAOS_MIX,"[SiegePotionMix][Success][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,type,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
}

void CChaosBox::LifeStoneMix(LPOBJ lpObj) // OK
{
	int ChaosCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int GuardianCount = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,13))
		{
			BlessCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,14))
		{
			SoulCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,31))
		{
			GuardianCount++;
		}
	}

	if(ChaosCount != 1 || BlessCount != 5 || SoulCount != 5 || GuardianCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	lpObj->ChaosMoney = 5000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	GDCreateItemSend(lpObj->Index,0xFF,0,0,GET_ITEM(13,11),1,0,0,0,0,-1,0,0,0,0,0,0xFF,0);

	gLog.Output(LOG_CHAOS_MIX,"[LifeStoneMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
}

void CChaosBox::SeniorMix(LPOBJ lpObj) // OK
{
	if(gCastleSiegeSync.CheckCastleOwnerMember(lpObj->Index) == 0 || lpObj->GuildStatus != 0x80)
	{
		this->GCChaosMixSend(lpObj->Index,0,0);
		return;
	}

	SYSTEMTIME SystemTime;

	GetSystemTime(&SystemTime);

	//- Castle Siege SeniorMix unlimited option
	if(gServerInfo.m_CastleSiegeSeniorMixUnlimitedUse == 0)
	{
		if(this->m_SeniorMixLimitDay == SystemTime.wDay && this->m_SeniorMixLimitMonth == SystemTime.wMonth && this->m_SeniorMixLimitYear == SystemTime.wYear)
		{
			this->GCChaosMixSend(lpObj->Index,0,0);
			return;
		}
	}

	int SoulPack10 = 0;
	int BlessPack10 = 0;
	int GuardianCount = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,30))
		{
			BlessPack10 += lpObj->ChaosBox[n].m_Level+1;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,31))
		{
			SoulPack10 += lpObj->ChaosBox[n].m_Level+1;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,31))
		{
			GuardianCount++;
		}
	}

	if(SoulPack10 != 3 || BlessPack10 != 3 || GuardianCount != 30)
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	lpObj->ChaosMoney = 1000000000;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	CItem item;

	if(gItemBagManager.GetItemBySpecialValue(ITEM_BAG_SENIOR_MIX,lpObj,&item) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,0,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	//- Castle Siege SeniorMix unlimited option
	if(gServerInfo.m_CastleSiegeSeniorMixUnlimitedUse == 0)
	{
		this->m_SeniorMixLimitDay = SystemTime.wDay;

		this->m_SeniorMixLimitMonth = SystemTime.wMonth;

		this->m_SeniorMixLimitYear = SystemTime.wYear;
	}

	GDCreateItemSend(lpObj->Index,0xFF,0,0,item.m_Index,(BYTE)item.m_Level,0,item.m_Option1,item.m_Option2,item.m_Option3,-1,item.m_NewOption,item.m_SetOption,item.m_JewelOfHarmonyOption,item.m_ItemOptionEx,item.m_SocketOption,item.m_SocketOptionBonus,0);

	gLog.Output(LOG_CHAOS_MIX,"[SeniorMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
}

void CChaosBox::PieceOfHornMix(LPOBJ lpObj) // OK
{
	int ChaosCount = 0;
	int SplinterOfArmorCount = 0;
	int BlessOfGuardianCount = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,32))
		{
			SplinterOfArmorCount += (int)lpObj->ChaosBox[n].m_Durability;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,33))
		{
			BlessOfGuardianCount += (int)lpObj->ChaosBox[n].m_Durability;
		}
	}

	if(ChaosCount != 1 || SplinterOfArmorCount != 20 || BlessOfGuardianCount != 20)
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	lpObj->ChaosSuccessRate = gServerInfo.m_PieceOfHornMixRate[lpObj->AccountLevel];

	if(this->GetTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		GDCreateItemSend(lpObj->Index,0xFF,0,0,GET_ITEM(13,35),0,1,0,0,0,-1,0,0,0,0,0,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[PieceOfHornMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[PieceOfHornMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
}

void CChaosBox::BrokenHornMix(LPOBJ lpObj) // OK
{
	int ChaosCount = 0;
	int ClawOfBeastCount = 0;
	int PieceOfHornCount = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,34))
		{
			ClawOfBeastCount += (int)lpObj->ChaosBox[n].m_Durability;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,35))
		{
			PieceOfHornCount++;
		}
	}

	if(ChaosCount != 1 || ClawOfBeastCount != 10 || PieceOfHornCount != 5)
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	lpObj->ChaosSuccessRate = gServerInfo.m_BrokenHornMixRate[lpObj->AccountLevel];

	if(this->GetTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		GDCreateItemSend(lpObj->Index,0xFF,0,0,GET_ITEM(13,36),0,1,0,0,0,-1,0,0,0,0,0,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[BrokenHornMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[BrokenHornMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
}

void CChaosBox::HornOfFenrirMix(LPOBJ lpObj) // OK
{
	int ChaosCount = 0;
	int BrokenHornCount = 0;
	int LifeCount = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,36))
		{
			BrokenHornCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,16))
		{
			LifeCount++;
		}
	}

	if(ChaosCount != 1 || BrokenHornCount != 1 || LifeCount != 3)
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	lpObj->ChaosSuccessRate = gServerInfo.m_HornOfFenrirMixRate[lpObj->AccountLevel];

	if(this->GetTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	lpObj->ChaosMoney = 10000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		GDCreateItemSend(lpObj->Index,0xFF,0,0,GET_ITEM(13,37),0,255,1,0,0,-1,0,0,0,0,0,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[HornOfFenrirMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[HornOfFenrirMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
}

void CChaosBox::HornOfFenrirUpgradeMix(LPOBJ lpObj) // OK
{
	int ChaosCount = 0;
	int HornOfFenrirCount = 0;
	int LifeCount = 0;
	int WeaponCount = 0;
	int WeaponMoney = 0;
	int ArmorCount = 0;
	int ArmorMoney = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,37))
		{
			HornOfFenrirCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,16))
		{
			LifeCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index >= GET_ITEM(0,0) && lpObj->ChaosBox[n].m_Index < GET_ITEM(6,0) && lpObj->ChaosBox[n].m_Level >= 4 && lpObj->ChaosBox[n].m_Option3 >= 1)
		{
			WeaponCount++;
			WeaponMoney = lpObj->ChaosBox[n].m_BuyMoney;
		}
		else if(lpObj->ChaosBox[n].m_Index >= GET_ITEM(6,0) && lpObj->ChaosBox[n].m_Index < GET_ITEM(12,0) && lpObj->ChaosBox[n].m_Level >= 4 && lpObj->ChaosBox[n].m_Option3 >= 1)
		{
			ArmorCount++;
			ArmorMoney = lpObj->ChaosBox[n].m_BuyMoney;
		}
	}

	if(ChaosCount != 1 || HornOfFenrirCount != 1 || LifeCount != 5 || (WeaponCount == 0 && ArmorCount == 0) || (WeaponCount != 0 && ArmorCount != 0))
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	if(gServerInfo.m_HornOfFenrirUpgradeMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (WeaponMoney+ArmorMoney)/10000;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_HornOfFenrirUpgradeMixRate[lpObj->AccountLevel];
	}

	if(this->GetTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	if(gServerInfo.m_HornOfFenrirUpgradeMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>80)?80:lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 10000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		BYTE ItemNewOption = ((WeaponCount==0)?((ArmorCount==0)?0:2):1);

		GDCreateItemSend(lpObj->Index,0xFF,0,0,GET_ITEM(13,37),0,255,1,0,0,-1,ItemNewOption,0,0,0,0,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[HornOfFenrirUpgradeMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[HornOfFenrirUpgradeMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
}

void CChaosBox::ShieldPotionMix(LPOBJ lpObj,int type) // OK
{
	int LargeHealingPotionCount = 0;
	int SmallCompoundPotionCount = 0;
	int MediumCompoundPotionCount = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,3))
		{
			LargeHealingPotionCount += (int)lpObj->ChaosBox[n].m_Durability;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,38))
		{
			SmallCompoundPotionCount += (int)lpObj->ChaosBox[n].m_Durability;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,39))
		{
			MediumCompoundPotionCount += (int)lpObj->ChaosBox[n].m_Durability;
		}
	}

	if((type == 0 && LargeHealingPotionCount != 3) || (type == 1 && SmallCompoundPotionCount != 3) || (type == 2 && MediumCompoundPotionCount != 3))
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	switch(type)
	{
		case 0:
			lpObj->ChaosSuccessRate = gServerInfo.m_ShieldPotionMixRate1[lpObj->AccountLevel];
			break;
		case 1:
			lpObj->ChaosSuccessRate = gServerInfo.m_ShieldPotionMixRate2[lpObj->AccountLevel];
			break;
		case 2:
			lpObj->ChaosSuccessRate = gServerInfo.m_ShieldPotionMixRate3[lpObj->AccountLevel];
			break;
	}

	if(this->GetTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	lpObj->ChaosMoney = ((type==0)?100000:(500000*type));

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		GDCreateItemSend(lpObj->Index,0xFF,0,0,(GET_ITEM(14,35)+type),0,1,0,0,0,-1,0,0,0,0,0,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[ShieldPotionMix][Success][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,type,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[ShieldPotionMix][Failure][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,type,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
}

void CChaosBox::JewelOfHarmonyItemPurityMix(LPOBJ lpObj) // OK
{
	int GemStoneCount = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,41))
		{
			GemStoneCount++;
		}
	}

	if(GemStoneCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	lpObj->ChaosSuccessRate = gServerInfo.m_JewelOfHarmonyItemPurityMixRate[lpObj->AccountLevel];

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		GDCreateItemSend(lpObj->Index,0xFF,0,0,GET_ITEM(14,42),0,1,0,0,0,-1,0,0,0,0,0,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[JewelOfHarmonyItemPurityMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[JewelOfHarmonyItemPurityMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
}

void CChaosBox::JewelOfHarmonyItemSmeltMix(LPOBJ lpObj) // OK
{
	int ItemCount = 0;
	int ItemSlot = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(gJewelOfHarmonyType.CheckJewelOfHarmonyItemType(&lpObj->ChaosBox[n]) != 0 && lpObj->ChaosBox[n].IsSetItem() == 0 && lpObj->ChaosBox[n].IsJewelOfHarmonyItem() == 0)
		{
			ItemCount++;
			ItemSlot = n;
		}
	}

	if(ItemCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	if(lpObj->ChaosBox[ItemSlot].IsExcItem() == 0)
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_JewelOfHarmonyItemSmeltMixRate1[lpObj->AccountLevel];
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_JewelOfHarmonyItemSmeltMixRate2[lpObj->AccountLevel];
	}

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = ((lpObj->ChaosBox[ItemSlot].IsExcItem()==0)?GET_ITEM(14,43):GET_ITEM(14,44));

		GDCreateItemSend(lpObj->Index,0xFF,0,0,ItemIndex,0,1,0,0,0,-1,0,0,0,0,0,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[JewelOfHarmonyItemSmeltMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[JewelOfHarmonyItemSmeltMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
}

void CChaosBox::JewelOfHarmonyItemRestoreMix(LPOBJ lpObj) // OK
{
	int ItemCount = 0;
	int ItemSlot = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].IsJewelOfHarmonyItem() != 0)
		{
			ItemCount++;
			ItemSlot = n;
		}
	}

	if(ItemCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	lpObj->ChaosMoney = gJewelOfHarmonyOption.GetJewelOfHarmonyItemRestoreMoney(&lpObj->ChaosBox[ItemSlot]);

	if(lpObj->ChaosMoney == -1)
	{
		this->GCChaosMixSend(lpObj->Index,0,0);
		return;
	}

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	CItem item = lpObj->ChaosBox[ItemSlot];

	item.m_JewelOfHarmonyOption = JEWEL_OF_HARMONY_OPTION_NONE;

	this->ChaosBoxInit(lpObj);

	gItemManager.ChaosBoxAddItem(lpObj->Index,item,0);

	this->GCChaosMixSend(lpObj->Index,1,&item);

	gLog.Output(LOG_CHAOS_MIX,"[JewelOfHarmonyItemRestoreMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
}

void CChaosBox::Item380Mix(LPOBJ lpObj) // OK
{
	int GuardianCount = 0;
	int HarmonyCount = 0;
	int ItemCount = 0;
	int ItemSlot = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,31))
		{
			GuardianCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,42))
		{
			HarmonyCount++;
		}
		else if(lpObj->ChaosBox[n].Is380Item() == 0 && lpObj->ChaosBox[n].m_Level >= 4 && lpObj->ChaosBox[n].m_Option3 >= 1)
		{
			ItemCount++;
			ItemSlot = n;
		}
	}

	if(GuardianCount != 1 || HarmonyCount != 1 || ItemCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	if(lpObj->ChaosBox[ItemSlot].m_Level >= 7)
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_Item380MixRate2[lpObj->AccountLevel];
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_Item380MixRate1[lpObj->AccountLevel];
	}

	if(this->GetTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	lpObj->ChaosMoney = 10000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		CItem item = lpObj->ChaosBox[ItemSlot];

		item.m_ItemOptionEx |= 0x80;

		this->ChaosBoxInit(lpObj);

		gItemManager.ChaosBoxAddItem(lpObj->Index,item,0);

		this->GCChaosMixSend(lpObj->Index,1,&item);

		gLog.Output(LOG_CHAOS_MIX,"[Item380Mix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxItemKeep(lpObj,ItemSlot);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[Item380Mix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
}

void CChaosBox::IllusionTempleMix(LPOBJ lpObj) // OK
{
	int ChaosCount = 0;
	int ScrollCount = 0;
	int ScrollLevel = 0;
	int PotionCount = 0;
	int PotionLevel = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,49))
		{
			ScrollCount++;
			ScrollLevel = lpObj->ChaosBox[n].m_Level;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,50))
		{
			PotionCount++;
			PotionLevel = lpObj->ChaosBox[n].m_Level;
		}
	}

	if(ChaosCount != 1 || ScrollCount != 1 || PotionCount != 1 || ScrollLevel != PotionLevel)
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	if(ScrollLevel > 6 || PotionLevel > 6)
	{
		this->GCChaosMixSend(lpObj->Index,8,0);
		return;
	}

	switch(ScrollLevel)
	{
		case 1:
			lpObj->ChaosSuccessRate = gServerInfo.m_IllusionTempleMixRate1[lpObj->AccountLevel];
			lpObj->ChaosMoney = 3000000;
			break;
		case 2:
			lpObj->ChaosSuccessRate = gServerInfo.m_IllusionTempleMixRate2[lpObj->AccountLevel];
			lpObj->ChaosMoney = 5000000;
			break;
		case 3:
			lpObj->ChaosSuccessRate = gServerInfo.m_IllusionTempleMixRate3[lpObj->AccountLevel];
			lpObj->ChaosMoney = 7000000;
			break;
		case 4:
			lpObj->ChaosSuccessRate = gServerInfo.m_IllusionTempleMixRate4[lpObj->AccountLevel];
			lpObj->ChaosMoney = 9000000;
			break;
		case 5:
			lpObj->ChaosSuccessRate = gServerInfo.m_IllusionTempleMixRate5[lpObj->AccountLevel];
			lpObj->ChaosMoney = 11000000;
			break;
		case 6:
			lpObj->ChaosSuccessRate = gServerInfo.m_IllusionTempleMixRate6[lpObj->AccountLevel];
			lpObj->ChaosMoney = 13000000;
			break;
	}

	if(this->GetTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		GDCreateItemSend(lpObj->Index,0xFF,0,0,GET_ITEM(13,51),ScrollLevel,1,0,0,0,-1,0,0,0,0,0,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[IllusionTempleMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[IllusionTempleMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
}

void CChaosBox::FeatherOfCondorMix(LPOBJ lpObj) // OK
{
	int ChaosCount = 0;
	int CreationCount = 0;
	int SoulPack10 = 0;
	int WingCount = 0;
	int ItemCount = 0;
	int ItemMoney = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,22))
		{
			CreationCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,31) && lpObj->ChaosBox[n].m_Level == 0)
		{
			SoulPack10++;
		}
		else if(((lpObj->ChaosBox[n].m_Index >= GET_ITEM(12,3) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(12,6)) || lpObj->ChaosBox[n].m_Index == GET_ITEM(12,42) || lpObj->ChaosBox[n].m_Index == GET_ITEM(12,49) || lpObj->ChaosBox[n].m_Index == GET_ITEM(13,30)) && (lpObj->ChaosBox[n].m_Level >= 9 && lpObj->ChaosBox[n].m_Option3 >= 1))
		{
			WingCount++;
		}
		else if(lpObj->ChaosBox[n].IsSetItem() != 0 && lpObj->ChaosBox[n].m_Level >= 7 && lpObj->ChaosBox[n].m_Option3 >= 1)
		{
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
	}

	if(ChaosCount != 1 || CreationCount != 1 || SoulPack10 != 1 || WingCount != 1 || ItemCount == 0)
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	if(gServerInfo.m_FeatherOfCondorMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney/300000)+1;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_FeatherOfCondorMixRate[lpObj->AccountLevel];
	}

	if(this->GetTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	if(gServerInfo.m_FeatherOfCondorMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>60)?60:lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = lpObj->ChaosSuccessRate*200000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		GDCreateItemSend(lpObj->Index,0xFF,0,0,GET_ITEM(13,53),0,0,0,0,0,-1,0,0,0,0,0,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[FeatherOfCondorMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[FeatherOfCondorMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
}

void CChaosBox::Wing3Mix(LPOBJ lpObj) // OK
{
	int ChaosCount = 0;
	int CreationCount = 0;
	int SoulPack10 = 0;
	int BlessPack10 = 0;
	int FlameCount = 0;
	int FeatherCount = 0;
	int ItemCount = 0;
	int ItemMoney = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,22))
		{
			CreationCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,31) && lpObj->ChaosBox[n].m_Level == 0)
		{
			SoulPack10++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,30) && lpObj->ChaosBox[n].m_Level == 0)
		{
			BlessPack10++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,52))
		{
			FlameCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,53))
		{
			FeatherCount++;
		}
		else if(lpObj->ChaosBox[n].IsExcItem() != 0 && lpObj->ChaosBox[n].m_Level >= 9 && lpObj->ChaosBox[n].m_Option3 >= 1)
		{
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
	}

	if(ChaosCount != 1 || CreationCount != 1 || SoulPack10 != 1 || BlessPack10 != 1 || FlameCount != 1 || FeatherCount != 1 || ItemCount == 0)
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	if(gServerInfo.m_Wing3MixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney/3000000)+1;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_Wing3MixRate[lpObj->AccountLevel];
	}

	if(this->GetTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	if(gServerInfo.m_Wing3MixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40)?40:lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = lpObj->ChaosSuccessRate*200000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;
		BYTE ItemOption3 = 0;
		BYTE ItemNewOption = 0;

		CRandomManager RandomManager;

		RandomManager.AddElement(GET_ITEM(12,36),1);

		RandomManager.AddElement(GET_ITEM(12,37),1);

		RandomManager.AddElement(GET_ITEM(12,38),1);

		RandomManager.AddElement(GET_ITEM(12,39),1);

		for (int i=0; i < gCustomWingMix.m_count; i++)
		{
			RandomManager.AddElement(GET_ITEM(gCustomWingMix.m_Data[i].m_Category,gCustomWingMix.m_Data[i].m_Index),1);
		}

		if (gServerInfo.m_DisableWingMixDL != 1)
		{
			RandomManager.AddElement(GET_ITEM(12,40),1);
		}
		
		if (gServerInfo.m_DisableWingMixSU != 1)
		{
			RandomManager.AddElement(GET_ITEM(12,43),1);
		}

		#if(GAMESERVER_UPDATE>=601)

		if (gServerInfo.m_DisableWingMixRF != 1)
		{
			RandomManager.AddElement(GET_ITEM(12,50),1);
		}

		#endif

		RandomManager.GetRandomElement(&ItemIndex);

		gItemOptionRate.GetItemOption2(6,&ItemOption2);

		gItemOptionRate.GetItemOption3(6,&ItemOption3);

		gItemOptionRate.GetItemOption4(6,&ItemNewOption);

		gItemOptionRate.MakeNewOption(ItemIndex,ItemNewOption,&ItemNewOption);

		GDCreateItemSend(lpObj->Index,0xFF,0,0,ItemIndex,0,0,0,ItemOption2,ItemOption3,-1,(ItemNewOption+(16*(GetLargeRand()%3))),0,0,0,0,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[Wing3Mix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[Wing3Mix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
}

void CChaosBox::ChaosCardMix(LPOBJ lpObj,int type) // OK
{
	int ChaosCardCount = 0;
	int ChaosCardGoldCount = 0;
	int ChaosCardRareCount = 0;
	int ChaosCardMiniCount = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,54))
		{
			type = 1;
			ChaosCardCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,92))
		{
			type = 2;
			ChaosCardGoldCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,93))
		{
			type = 3;
			ChaosCardRareCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,95))
		{
			type = 4;
			ChaosCardMiniCount++;
		}
	}

	if(type == 0 || (type == 1 && ChaosCardCount != 1) || (type == 2 && ChaosCardGoldCount != 1) || (type == 3 && ChaosCardRareCount != 1) || (type == 4 && ChaosCardMiniCount != 1))
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	CItem item;

	if(gItemBagManager.GetItemBySpecialValue((ITEM_BAG_CHAOS_CARD_MIX1+(type-1)),lpObj,&item) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,0,0);
		return;
	}

	GDCreateItemSend(lpObj->Index,0xFF,0,0,item.m_Index,(BYTE)item.m_Level,0,item.m_Option1,item.m_Option2,item.m_Option3,-1,item.m_NewOption,item.m_SetOption,item.m_JewelOfHarmonyOption,item.m_ItemOptionEx,item.m_SocketOption,item.m_SocketOptionBonus,0);

	gLog.Output(LOG_CHAOS_MIX,"[ChaosCardMix][Success][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,type,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
}

void CChaosBox::CherryBlossomMix(LPOBJ lpObj,int type) // OK
{
	int WhiteCherryBlossomCount = 0;
	int RedCherryBlossomCount = 0;
	int GoldenCherryBlossomCount = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,88))
		{
			type = 1;
			WhiteCherryBlossomCount += (int)lpObj->ChaosBox[n].m_Durability;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,89))
		{
			type = 2;
			RedCherryBlossomCount += (int)lpObj->ChaosBox[n].m_Durability;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,90))
		{
			type = 3;
			GoldenCherryBlossomCount += (int)lpObj->ChaosBox[n].m_Durability;
		}
	}

	if(type == 0 || (type == 1 && WhiteCherryBlossomCount != 10) || (type == 2 && RedCherryBlossomCount != 30) || (type == 3 && GoldenCherryBlossomCount != 255))
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	CItem item;

	if(gItemBagManager.GetItemBySpecialValue((ITEM_BAG_CHERRY_BLOSSOM_MIX1+(type-1)),lpObj,&item) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,0,0);
		return;
	}

	GDCreateItemSend(lpObj->Index,0xFF,0,0,item.m_Index,(BYTE)item.m_Level,0,item.m_Option1,item.m_Option2,item.m_Option3,-1,item.m_NewOption,item.m_SetOption,item.m_JewelOfHarmonyOption,item.m_ItemOptionEx,item.m_SocketOption,item.m_SocketOptionBonus,0);

	gLog.Output(LOG_CHAOS_MIX,"[CherryBlossomMix][Success][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,type,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
}

void CChaosBox::SocketItemCreateSeedMix(LPOBJ lpObj) // OK
{
	int ChaosCount = 0;
	int CreationCount = 0;
	int HarmonyCount = 0;
	int ExcItemCount = 0;
	int SetItemCount = 0;
	int ItemMoney = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,22))
		{
			CreationCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,42))
		{
			HarmonyCount++;
		}
		else if(lpObj->ChaosBox[n].IsExcItem() != 0 && lpObj->ChaosBox[n].m_Level >= 4 && ExcItemCount == 0)
		{
			ExcItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
		else if(lpObj->ChaosBox[n].IsSetItem() != 0 && lpObj->ChaosBox[n].m_Level >= 4 && SetItemCount == 0)
		{
			SetItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
	}

	if(ChaosCount != 1 || CreationCount != 1 || HarmonyCount != 1 || ExcItemCount != 1 || SetItemCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	if(gServerInfo.m_SocketItemCreateSeedMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney/2000000)+80;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_SocketItemCreateSeedMixRate[lpObj->AccountLevel];
	}

	if(gServerInfo.m_SocketItemCreateSeedMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>90)?90:lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 1000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		int SocketOptionType = 1+(GetLargeRand()%MAX_SOCKET_ITEM_OPTION_TYPE);

		int SocketOptionIndex = gSocketItemOption.GetSocketItemRandomOption(SocketOptionType);

		int SocketOptionValue = gSocketItemOption.GetSocketItemSeedOption(SocketOptionIndex,SocketOptionType);

		if(SocketOptionIndex == -1 || SocketOptionValue == -1)
		{
			this->GCChaosMixSend(lpObj->Index,0,0);
			return;
		}

		GDCreateItemSend(lpObj->Index,0xFF,0,0,(GET_ITEM(12,60)+(SocketOptionType-1)),SocketOptionValue,0,0,0,0,-1,0,0,0,0,0,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[SocketItemCreateSeedMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[SocketItemCreateSeedMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
}

void CChaosBox::SocketItemCreateSeedSphereMix(LPOBJ lpObj) // OK
{
	int ChaosCount = 0;
	int CreationCount = 0;
	int SeedCount = 0;
	int SeedType = 0;
	int SeedLevel = 0;
	int SphereCount = 0;
	int SphereType = 0;
	int ItemMoney = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,22))
		{
			CreationCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index >= GET_ITEM(12,60) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(12,66))
		{
			SeedCount++;
			SeedType = lpObj->ChaosBox[n].m_Index-GET_ITEM(12,60);
			SeedLevel = lpObj->ChaosBox[n].m_Level;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
		else if(lpObj->ChaosBox[n].m_Index >= GET_ITEM(12,70) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(12,74))
		{
			SphereCount++;
			SphereType = lpObj->ChaosBox[n].m_Index-GET_ITEM(12,70);
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
	}

	if(ChaosCount != 1 || CreationCount != 1 || SeedCount != 1 || SphereCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	if(gServerInfo.m_SocketItemCreateSeedSphereMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney/200000)+80;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_SocketItemCreateSeedSphereMixRate[lpObj->AccountLevel];
	}

	if(gServerInfo.m_SocketItemCreateSeedSphereMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>90)?90:lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 1000000;
	
	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;
	
	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		GDCreateItemSend(lpObj->Index,0xFF,0,0,(GET_ITEM(12,100)+(SphereType*6)+SeedType),SeedLevel,0,0,0,0,-1,0,0,0,0,0,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[SocketItemCreateSeedSphereMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[SocketItemCreateSeedSphereMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
}

void CChaosBox::SocketItemMountSeedSphereMix(LPOBJ lpObj,BYTE info) // OK
{
	if(info >= MAX_SOCKET_ITEM_OPTION_TABLE)
	{
		this->GCChaosMixSend(lpObj->Index,0,0);
		return;
	}

	int ChaosCount = 0;
	int CreationCount = 0;
	int SeedSphereCount = 0;
	int SeedSphereType = 0;
	int SeedSphereLevel = 0;
	int SeedSphereOption = 0;
	int ItemCount = 0;
	int ItemSlot = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,22))
		{
			CreationCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index >= GET_ITEM(12,100) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(12,129))
		{
			SeedSphereCount++;
			SeedSphereType = (lpObj->ChaosBox[n].m_Index-GET_ITEM(12,100))%MAX_SOCKET_ITEM_OPTION_TYPE;
			SeedSphereLevel = (lpObj->ChaosBox[n].m_Index-GET_ITEM(12,100))/MAX_SOCKET_ITEM_OPTION_TYPE;
			SeedSphereOption = lpObj->ChaosBox[n].m_Level;
		}
		else if(gSocketItemType.CheckSocketItemType(lpObj->ChaosBox[n].m_Index) != 0 && lpObj->ChaosBox[n].m_SocketOption[info] == SOCKET_ITEM_OPTION_EMPTY)
		{
			ItemCount++;
			ItemSlot = n;
		}
	}

	if(ChaosCount != 1 || CreationCount != 1 || SeedSphereCount != 1 || ItemCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	//limite socket
	if(info >= gSocketItemType.GetSocketItemMaxSocket(lpObj->ChaosBox[ItemSlot].m_Index))
	{
		this->GCChaosMixSend(lpObj->Index,0,0);
		return;
	}

	lpObj->ChaosMoney = 1000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	BYTE SocketOption = gSocketItemOption.GetSocketItemOption(&lpObj->ChaosBox[ItemSlot],(SeedSphereType+1),SeedSphereOption,SeedSphereLevel);

	if(SocketOption == SOCKET_ITEM_OPTION_NONE)
	{
		this->GCChaosMixSend(lpObj->Index,0,0);
		return;
	}

	CItem item = lpObj->ChaosBox[ItemSlot];

	item.m_SocketOption[info] = SocketOption;

	item.m_SocketOptionBonus = gSocketItemOption.GetSocketItemBonusOption(&item);

	this->ChaosBoxInit(lpObj);

	gItemManager.ChaosBoxAddItem(lpObj->Index,item,0);

	this->GCChaosMixSend(lpObj->Index,1,&item);

	gLog.Output(LOG_CHAOS_MIX,"[SocketItemMountSeedSphereMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
}

void CChaosBox::SocketItemUnMountSeedSphereMix(LPOBJ lpObj,BYTE info) // OK
{
	if(info >= MAX_SOCKET_ITEM_OPTION_TABLE)
	{
		this->GCChaosMixSend(lpObj->Index,0,0);
		return;
	}

	int ItemCount = 0;
	int ItemSlot = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_SocketOption[info] != SOCKET_ITEM_OPTION_NONE && lpObj->ChaosBox[n].m_SocketOption[info] != SOCKET_ITEM_OPTION_EMPTY)
		{
			ItemCount++;
			ItemSlot = n;
		}
	}

	if(ItemCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	lpObj->ChaosMoney = 1000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	CItem item = lpObj->ChaosBox[ItemSlot];

	item.m_SocketOption[info] = SOCKET_ITEM_OPTION_EMPTY;

	item.m_SocketOptionBonus = gSocketItemOption.GetSocketItemBonusOption(&item);

	this->ChaosBoxInit(lpObj);

	gItemManager.ChaosBoxAddItem(lpObj->Index,item,0);

	this->GCChaosMixSend(lpObj->Index,1,&item);
}

void CChaosBox::ImperialGuardianMix(LPOBJ lpObj) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	int DimensionalPart1Count = 0;
	int DimensionalPart2Count = 0;
	int DimensionalPart3Count = 0;
	int DimensionalPart4Count = 0;
	int DimensionalPart5Count = 0;
	int DimensionalPart6Count = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,103))
		{
			DimensionalPart1Count++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,104))
		{
			DimensionalPart2Count++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,105))
		{
			DimensionalPart3Count++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,106))
		{
			DimensionalPart4Count++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,107))
		{
			DimensionalPart5Count++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,108))
		{
			DimensionalPart6Count++;
		}
	}

	if(DimensionalPart1Count != 1 && DimensionalPart2Count != 1 && DimensionalPart3Count != 1 && DimensionalPart4Count != 1 && DimensionalPart5Count != 1 && DimensionalPart6Count != 1)
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	lpObj->ChaosMoney = 1000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	GDCreateItemSend(lpObj->Index,0xFF,0,0,GET_ITEM(14,109),0,1,0,0,0,-1,0,0,0,0,0,0xFF,0);

	gLog.Output(LOG_CHAOS_MIX,"[ImperialGuardianMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);

	#endif
}

void CChaosBox::ChestMix(LPOBJ lpObj) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	int GoldKeyCount = 0;
	int GoldChestCount = 0;
	int SilverKeyCount = 0;
	int SilverChestCount = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,112))
		{
			SilverKeyCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,113))
		{
			GoldKeyCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,121))
		{
			GoldChestCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,122))
		{
			SilverChestCount++;
		}
	}

	if((GoldKeyCount != 1 && SilverKeyCount != 1) || (GoldKeyCount == 1 && GoldChestCount != 1) || (SilverKeyCount == 1 && SilverChestCount != 1))
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	GDCreateItemSend(lpObj->Index,0xFF,0,0,(GET_ITEM(14,123)+SilverKeyCount),0,0,0,0,0,-1,0,0,0,0,0,0xFF,0);

	gLog.Output(LOG_CHAOS_MIX,"[ChestMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);

	#endif
}

void CChaosBox::SummonScrollMix(LPOBJ lpObj,int type) // OK
{
	//#if(GAMESERVER_UPDATE>=801)

	int SummonScroll1Count = 0;
	int SummonScroll2Count = 0;
	int SummonScroll3Count = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,210))
		{
			type = 1;
			SummonScroll1Count++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,211))
		{
			type = 2;
			SummonScroll2Count++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,212))
		{
			type = 3;
			SummonScroll3Count++;
		}
	}

	if(type == 0 || (type == 1 && SummonScroll1Count != 2) || (type == 2 && (SummonScroll1Count != 1 || SummonScroll2Count != 1)) || (type == 3 && (SummonScroll1Count != 1 || SummonScroll3Count != 1)))
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	switch(type)
	{
		case 1:
			lpObj->ChaosSuccessRate = gServerInfo.m_SummonScrollMixRate1[lpObj->AccountLevel];
			lpObj->ChaosMoney = 100000;
			break;
		case 2:
			lpObj->ChaosSuccessRate = gServerInfo.m_SummonScrollMixRate2[lpObj->AccountLevel];
			lpObj->ChaosMoney = 200000;
			break;
		case 3:
			lpObj->ChaosSuccessRate = gServerInfo.m_SummonScrollMixRate3[lpObj->AccountLevel];
			lpObj->ChaosMoney = 300000;
			break;
	}

	if(this->GetTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		GDCreateItemSend(lpObj->Index,0xFF,0,0,(GET_ITEM(14,210)+type),0,1,0,0,0,-1,0,0,0,0,0,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[SummonScrollMix][Success][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,type,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[SummonScrollMix][Failure][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,type,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}

	//#endif
}

void CChaosBox::LuckyItemCreateMix(LPOBJ lpObj) // OK
{
	#if(GAMESERVER_UPDATE>=602)

	int LuckyTicketCount = 0;
	int LuckyTicketGroup = 0;
	int LuckyTicketSection = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,135))
		{
			LuckyTicketCount++;
			LuckyTicketGroup = 0;
			LuckyTicketSection = 8;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,136))
		{
			LuckyTicketCount++;
			LuckyTicketGroup = 0;
			LuckyTicketSection = 9;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,137))
		{
			LuckyTicketCount++;
			LuckyTicketGroup = 0;
			LuckyTicketSection = 7;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,138))
		{
			LuckyTicketCount++;
			LuckyTicketGroup = 0;
			LuckyTicketSection = 10;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,139))
		{
			LuckyTicketCount++;
			LuckyTicketGroup = 0;
			LuckyTicketSection = 11;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,140))
		{
			LuckyTicketCount++;
			LuckyTicketGroup = 1;
			LuckyTicketSection = 8;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,141))
		{
			LuckyTicketCount++;
			LuckyTicketGroup = 1;
			LuckyTicketSection = 9;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,142))
		{
			LuckyTicketCount++;
			LuckyTicketGroup = 1;
			LuckyTicketSection = 7;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,143))
		{
			LuckyTicketCount++;
			LuckyTicketGroup = 1;
			LuckyTicketSection = 10;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,144))
		{
			LuckyTicketCount++;
			LuckyTicketGroup = 1;
			LuckyTicketSection = 11;
		}
	}

	if(LuckyTicketCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	WORD ItemIndex = 0;
	BYTE ItemLevel = 0;
	BYTE ItemOption1 = 0;
	BYTE ItemOption2 = 0;
	BYTE ItemOption3 = 0;
	BYTE ItemNewOption = 0;
	BYTE ItemSetOption = 0;
	BYTE ItemSocketOption[MAX_SOCKET_OPTION] = {0xFF,0xFF,0xFF,0xFF,0xFF};

	if(gLuckyItem.GetLuckyItemIndex(lpObj,LuckyTicketSection,LuckyTicketGroup,&ItemIndex) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,0,0);
		return;
	}

	gLuckyItem.GetLuckyItemOption0(ItemIndex,&ItemLevel);

	gLuckyItem.GetLuckyItemOption1(ItemIndex,&ItemOption1);

	gLuckyItem.GetLuckyItemOption2(ItemIndex,&ItemOption2);

	gLuckyItem.GetLuckyItemOption3(ItemIndex,&ItemOption3);

	gLuckyItem.GetLuckyItemOption4(ItemIndex,&ItemNewOption);

	gLuckyItem.GetLuckyItemOption5(ItemIndex,&ItemSetOption);

	gLuckyItem.GetLuckyItemOption6(ItemIndex,&ItemSocketOption[0]);

	gItemOptionRate.MakeNewOption(ItemIndex,ItemNewOption,&ItemNewOption);

	gItemOptionRate.MakeSetOption(ItemIndex,ItemSetOption,&ItemSetOption);

	gItemOptionRate.MakeSocketOption(ItemIndex,ItemSocketOption[0],&ItemSocketOption[0]);

	GDCreateItemSend(lpObj->Index,0xFF,0,0,ItemIndex,ItemLevel,255,ItemOption1,ItemOption2,ItemOption3,-1,ItemNewOption,ItemSetOption,0,0,ItemSocketOption,0xFF,0);

	gLog.Output(LOG_CHAOS_MIX,"[LuckyItemCreateMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);

	#endif
}

void CChaosBox::LuckyItemRefineMix(LPOBJ lpObj) // OK
{
	#if(GAMESERVER_UPDATE>=602)

	int LuckyItemCount = 0;
	int LuckyItemSlot = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].IsLuckyItem() != 0)
		{
			LuckyItemCount++;
			LuckyItemSlot = n;
		}
	}

	if(LuckyItemCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	if(lpObj->ChaosBox[LuckyItemSlot].m_Durability == 255)
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_LuckyItemRefineMixRate1[lpObj->AccountLevel];
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_LuckyItemRefineMixRate2[lpObj->AccountLevel];
	}

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		GDCreateItemSend(lpObj->Index,0xFF,0,0,GET_ITEM(14,160),0,1,0,0,0,-1,0,0,0,0,0,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[LuckyItemRefineMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[LuckyItemRefineMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}

	#endif
}

void CChaosBox::MonsterWingMix(LPOBJ lpObj) // OK
{
	//#if(GAMESERVER_UPDATE>=701)

	int ChaosCount = 0;
	int CreationCount = 0;
	int MaterialType = 0;
	int WingItemCount = 0;
	int WingItemMoney = 0;
	int WingItemType = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,22))
		{
			CreationCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,176))
		{
			MaterialType = 1;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,177))
		{
			MaterialType = 2;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,178))
		{
			MaterialType = 3;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,179))
		{
			MaterialType = 4;
		}
		else if((lpObj->ChaosBox[n].m_Index >= GET_ITEM(12,3) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(12,6)) || lpObj->ChaosBox[n].m_Index == GET_ITEM(12,42))
		{
			WingItemCount++;
			WingItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
			WingItemType = 0;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,49) || lpObj->ChaosBox[n].m_Index == GET_ITEM(13,30))
		{
			WingItemCount++;
			WingItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
			WingItemType = 1;
		}
	}

	if(ChaosCount != 1 || CreationCount != 1 || MaterialType == 0 || WingItemCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	if(gServerInfo.m_MonsterWingMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = WingItemMoney/((WingItemType==0)?9000000:500000);
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_MonsterWingMixRate[lpObj->AccountLevel];
	}

	if(this->GetTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	if(gServerInfo.m_MonsterWingMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>60)?60:lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 7000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((rand()%100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;
		BYTE ItemOption3 = 0;
		BYTE ItemNewOption = 0;

		switch(MaterialType)
		{
			case 1:
				ItemIndex = GET_ITEM(12,200);
				break;
			case 2:
				ItemIndex = GET_ITEM(12,201);
				break;
			case 3:
				ItemIndex = GET_ITEM(12,202);
				break;
			case 4:
				ItemIndex = GET_ITEM(12,203);
				break;
		}

		gItemOptionRate.GetItemOption2(7,&ItemOption2);

		gItemOptionRate.GetItemOption3(7,&ItemOption3);

		gItemOptionRate.GetItemOption4(7,&ItemNewOption);

		gItemOptionRate.MakeNewOption(ItemIndex,ItemNewOption,&ItemNewOption);

		GDCreateItemSend(lpObj->Index,0xFF,0,0,ItemIndex,0,0,0,ItemOption2,ItemOption3,-1,(ItemNewOption+(16*(rand()%2))),0,0,0,0,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[MonsterWingMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[MonsterWingMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}

	//#endif
}

void CChaosBox::SocketWeaponMix(LPOBJ lpObj) // OK
{
	#if(GAMESERVER_UPDATE>=401)

	int IceWalkerManeCount = 0;
	int MammothBundleOfFurCount = 0;
	int GiantIcicleCount = 0;
	int CoolutinPoisonCount = 0;
	int IronKnightHeartCount = 0;
	int DarkMammothHornCount = 0;
	int DarkGiantAxeCount = 0;
	int DarkCoolutinBloodCount = 0;
	int DarkIronKnightSwordCount = 0;
	int CombinationNoteType = 0;
	int SocketItemCount = 0;
	int SocketItemMoney = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,180))
		{
			IceWalkerManeCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,181))
		{
			MammothBundleOfFurCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,182))
		{
			GiantIcicleCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,183))
		{
			CoolutinPoisonCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,184))
		{
			IronKnightHeartCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,185))
		{
			DarkMammothHornCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,186))
		{
			DarkGiantAxeCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,187))
		{
			DarkCoolutinBloodCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,188))
		{
			DarkIronKnightSwordCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index >= GET_ITEM(14,191) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(14,208))
		{
			CombinationNoteType = lpObj->ChaosBox[n].m_Index-GET_ITEM(14,190);
		}
		else if(gSocketItemType.CheckSocketItemType(lpObj->ChaosBox[n].m_Index) != 0 && lpObj->ChaosBox[n].m_Level >= 7 && lpObj->ChaosBox[n].m_Option3 >= 1)
		{
			SocketItemCount++;
			SocketItemMoney += lpObj->ChaosBox[n].m_Value;
		}
	}

	if(CombinationNoteType == 0 || SocketItemCount == 0 || (CombinationNoteType == 1 && (IceWalkerManeCount != 2 || DarkIronKnightSwordCount != 1)) || (CombinationNoteType == 2 && (MammothBundleOfFurCount != 2 || DarkMammothHornCount != 1)) || (CombinationNoteType == 3 && (GiantIcicleCount != 2 || DarkIronKnightSwordCount != 1)) || (CombinationNoteType == 4 && (CoolutinPoisonCount != 2 || DarkMammothHornCount != 1)) || (CombinationNoteType == 5 && (GiantIcicleCount != 2 || DarkGiantAxeCount != 1)) || (CombinationNoteType == 6 && (IceWalkerManeCount != 2 || DarkMammothHornCount != 1)) || (CombinationNoteType == 7 && (IronKnightHeartCount != 2 || DarkCoolutinBloodCount != 1)) || (CombinationNoteType == 8 && (GiantIcicleCount != 2 || DarkMammothHornCount != 1)) || (CombinationNoteType == 9 && (IronKnightHeartCount != 2 || DarkGiantAxeCount != 1)) || (CombinationNoteType == 10 && (IronKnightHeartCount != 2 || DarkIronKnightSwordCount != 1)) || (CombinationNoteType == 11 && (MammothBundleOfFurCount != 2 || DarkCoolutinBloodCount != 1)) || (CombinationNoteType == 12 && (IronKnightHeartCount != 2 || DarkMammothHornCount != 1)) || (CombinationNoteType == 13 && (GiantIcicleCount != 2 || DarkCoolutinBloodCount != 1)) || (CombinationNoteType == 14 && (CoolutinPoisonCount != 2 || DarkMammothHornCount != 1)) || (CombinationNoteType == 15 && (IceWalkerManeCount != 2 || DarkMammothHornCount != 1)) || (CombinationNoteType == 16 && (IronKnightHeartCount != 2 || DarkGiantAxeCount != 1)) || (CombinationNoteType == 17 && (IronKnightHeartCount != 2 || DarkIronKnightSwordCount != 1)) || (CombinationNoteType == 18 && (MammothBundleOfFurCount != 2 || DarkCoolutinBloodCount != 1)))
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	if(gServerInfo.m_SocketWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = SocketItemMoney/360000;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_SocketWeaponMixRate[lpObj->AccountLevel];
	}

	if(this->GetTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	if(gServerInfo.m_SocketWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40)?40:lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 1000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;
		BYTE ItemSocketOption[MAX_SOCKET_OPTION] = {0xFF,0xFF,0xFF,0xFF,0xFF};

		switch(CombinationNoteType)
		{
			case 1:
				ItemIndex = GET_ITEM(0,29);
				break;
			case 2:
				ItemIndex = GET_ITEM(0,36);
				break;
			case 3:
				ItemIndex = GET_ITEM(0,37);
				break;
			case 4:
				ItemIndex = GET_ITEM(3,12);
				break;
			case 5:
				ItemIndex = GET_ITEM(2,20);
				break;
			case 6:
				ItemIndex = GET_ITEM(4,25);
				break;
			case 7:
				ItemIndex = GET_ITEM(4,26);
				break;
			case 8:
				ItemIndex = GET_ITEM(5,35);
				break;
			case 9:
				ItemIndex = GET_ITEM(5,37);
				break;
			case 10:
				ItemIndex = GET_ITEM(0,30);
				break;
			case 11:
				ItemIndex = GET_ITEM(2,19);
				break;
			case 12:
				ItemIndex = GET_ITEM(5,42); //Soulbringer
				break;
			case 13:
				ItemIndex = GET_ITEM(0,38); //Prickle Glove
				break;
			//Add Socket Shields
			case 14:
				ItemIndex = GET_ITEM(6,22);
				break;
			case 15:
				ItemIndex = GET_ITEM(6,23);
				break;
			case 16:
				ItemIndex = GET_ITEM(6,24);
				break;
			case 17:
				ItemIndex = GET_ITEM(6,25);
				break;
			case 18:
				ItemIndex = GET_ITEM(6,26);
				break;
		}

		gItemOptionRate.GetItemOption2(8,&ItemOption2);

		gItemOptionRate.GetItemOption6(((gItemManager.GetItemTwoHand(ItemIndex)==0)?6:7),&ItemSocketOption[0]);

		gItemOptionRate.MakeSocketOption(ItemIndex,ItemSocketOption[0],&ItemSocketOption[0]);

		GDCreateItemSend(lpObj->Index,0xFF,0,0,ItemIndex,0,0,1,ItemOption2,0,-1,0,0,0,0,ItemSocketOption,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[SocketWeaponMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[SocketWeaponMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}

	#endif
}

void CChaosBox::PentagramMithrilMix(LPOBJ lpObj) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	int ChaosCount = 0;
	int MithrilFragmentBunchCount = 0;
	int MithrilFragmentBunchType = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,148))
		{
			MithrilFragmentBunchCount++;
			MithrilFragmentBunchType = lpObj->ChaosBox[n].m_SocketOptionBonus;
		}
	}

	if(ChaosCount != 1 || MithrilFragmentBunchCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index,250,0);
		return;
	}

	if(CHECK_RANGE((MithrilFragmentBunchType-1),MAX_PENTAGRAM_ELEMENTAL_ATTRIBUTE) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,248,0);
		return;
	}

	lpObj->ChaosSuccessRate = gServerInfo.m_PentagramMithrilMixRate[lpObj->AccountLevel];

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	lpObj->ChaosMoney = 100000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,249,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = GET_ITEM(12,145);
		BYTE SocketOption[MAX_SOCKET_OPTION] = {0xFF,0xFF,0xFF,0xFF,0xFF};
		BYTE SocketOptionBonus = MithrilFragmentBunchType;

		GDCreateItemSend(lpObj->Index,0xFF,0,0,ItemIndex,0,0,0,0,0,-1,0,0,0,0,SocketOption,SocketOptionBonus,0);

		gLog.Output(LOG_CHAOS_MIX,"[PentagramMithrilMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,226,0);

		gLog.Output(LOG_CHAOS_MIX,"[PentagramMithrilMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}

	#endif
}

void CChaosBox::PentagramElixirMix(LPOBJ lpObj) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	int ChaosCount = 0;
	int ElixirFragmentBunchCount = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,149))
		{
			ElixirFragmentBunchCount++;
		}
	}

	if(ChaosCount != 1 || ElixirFragmentBunchCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index,250,0);
		return;
	}

	lpObj->ChaosSuccessRate = gServerInfo.m_PentagramElixirMixRate[lpObj->AccountLevel];

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	lpObj->ChaosMoney = 100000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,249,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = GET_ITEM(12,147);
		BYTE SocketOption[MAX_SOCKET_OPTION] = {0xFF,0xFF,0xFF,0xFF,0xFF};
		BYTE SocketOptionBonus = 0xFF;

		GDCreateItemSend(lpObj->Index,0xFF,0,0,ItemIndex,0,0,0,0,0,-1,0,0,0,0,SocketOption,SocketOptionBonus,0);

		gLog.Output(LOG_CHAOS_MIX,"[PentagramElixirMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,226,0);

		gLog.Output(LOG_CHAOS_MIX,"[PentagramElixirMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}

	#endif
}

void CChaosBox::PentagramJewelMix(LPOBJ lpObj) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	int BlessCount = 0;
	int MithrilCount = 0;
	int MithrilType = 0;
	int ElixirCount = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,13))
		{
			BlessCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,145))
		{
			MithrilCount++;
			MithrilType = lpObj->ChaosBox[n].m_SocketOptionBonus;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,147))
		{
			ElixirCount++;
		}
	}

	if(BlessCount != 1 || MithrilCount != 1 || ElixirCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index,250,0);
		return;
	}

	if(CHECK_RANGE((MithrilType-1),MAX_PENTAGRAM_ELEMENTAL_ATTRIBUTE) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,251,0);
		return;
	}

	lpObj->ChaosSuccessRate = gServerInfo.m_PentagramJewelMixRate[lpObj->AccountLevel];

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	lpObj->ChaosMoney = 100000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,249,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE SocketOption[MAX_SOCKET_OPTION] = {0xFF,0xFF,0xFF,0xFF,0xFF};
		BYTE SocketOptionBonus = MithrilType | 16;

		CRandomManager RandomManager;

		RandomManager.AddElement(GET_ITEM(12,221),4000);

		RandomManager.AddElement(GET_ITEM(12,231),3000);

		RandomManager.AddElement(GET_ITEM(12,241),1500);

		RandomManager.AddElement(GET_ITEM(12,251),1000);

		RandomManager.AddElement(GET_ITEM(12,261),500);

		RandomManager.GetRandomElement(&ItemIndex);

		if(gPentagramSystem.GetPentagramRandomJewelOption(ItemIndex,1,&SocketOption[0]) == 0)
		{
			this->GCChaosMixSend(lpObj->Index,248,0);
			return;
		}

		GDCreateItemSend(lpObj->Index,0xFF,0,0,ItemIndex,0,0,0,0,0,-1,0,0,0,0,SocketOption,SocketOptionBonus,0);

		gLog.Output(LOG_CHAOS_MIX,"[PentagramJewelMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,226,0);

		gLog.Output(LOG_CHAOS_MIX,"[PentagramJewelMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}

	#endif
}

void CChaosBox::PentagramDecompositeMix(LPOBJ lpObj,int type) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	int HarmonyCount = 0;
	int PentagramItemType = 0;
	int PentagramItemSlot = 0;
	int PentagramDecompositeType = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,42))
		{
			HarmonyCount++;
		}
		if(lpObj->ChaosBox[n].IsPentagramItem() != 0)
		{
			PentagramItemType = lpObj->ChaosBox[n].m_SocketOptionBonus%16;
			PentagramItemSlot = n;
			PentagramDecompositeType = lpObj->ChaosBox[n].m_Index-GET_ITEM(12,199);
		}
	}

	if(HarmonyCount != 1 || PentagramDecompositeType == 0 || (PentagramDecompositeType != (type+1)))
	{
		this->GCChaosMixSend(lpObj->Index,250,0);
		return;
	}

	if(CHECK_RANGE((PentagramItemType-1),MAX_PENTAGRAM_ELEMENTAL_ATTRIBUTE) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,251,0);
		return;
	}

	lpObj->ChaosSuccessRate = gServerInfo.m_PentagramDecompositeMixRate[lpObj->AccountLevel];

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	lpObj->ChaosMoney = 100000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,249,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	gPentagramSystem.DelAllPentagramJewelInfo(lpObj,&lpObj->ChaosBox[PentagramItemSlot],0);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE SocketOption[MAX_SOCKET_OPTION] = {0xFF,0xFF,0xFF,0xFF,0xFF};
		BYTE SocketOptionBonus = 0xFF;

		CRandomManager RandomManager;

		RandomManager.AddElement(GET_ITEM(12,144),5000);

		RandomManager.AddElement(GET_ITEM(12,145),1500);

		RandomManager.AddElement(GET_ITEM(12,146),2500);

		RandomManager.AddElement(GET_ITEM(12,147),500);

		RandomManager.AddElement(GET_ITEM(12,150),500);

		RandomManager.GetRandomElement(&ItemIndex);

		SocketOptionBonus = ((ItemIndex==GET_ITEM(12,144))?PentagramItemType:SocketOptionBonus);

		SocketOptionBonus = ((ItemIndex==GET_ITEM(12,145))?PentagramItemType:SocketOptionBonus);

		GDCreateItemSend(lpObj->Index,0xFF,0,0,ItemIndex,0,0,0,0,0,-1,0,0,0,0,SocketOption,SocketOptionBonus,0);

		gLog.Output(LOG_CHAOS_MIX,"[PentagramDecompositeMix][Success][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,type,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,226,0);

		gLog.Output(LOG_CHAOS_MIX,"[PentagramDecompositeMix][Failure][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,type,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}

	#endif
}

void CChaosBox::PentagramJewelUpgradeLevelMix(LPOBJ lpObj,BYTE info) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	int BlessCount = 0;
	int JewelCombinationFrameCount = 0;
	int PentagramJewelCount = 0;
	int PentagramJewelSlot = 0;
	int ElementalChaosAmulet = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,13))
		{
			BlessCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,150))
		{
			JewelCombinationFrameCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,190))
		{
			ElementalChaosAmulet++;
		}
		else if(lpObj->ChaosBox[n].IsPentagramJewel() != 0)
		{
			PentagramJewelCount++;
			PentagramJewelSlot = n;
		}
	}

	if(BlessCount != 1 || JewelCombinationFrameCount != 1 || PentagramJewelCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index,250,0);
		return;
	}

	if(CHECK_RANGE(((lpObj->ChaosBox[PentagramJewelSlot].m_SocketOptionBonus/16)-1),MAX_PENTAGRAM_JEWEL_RANK) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,252,0);
		return;
	}

	if(CHECK_RANGE(lpObj->ChaosBox[PentagramJewelSlot].m_Level,(MAX_PENTAGRAM_JEWEL_LEVEL-1)) == 0 || lpObj->ChaosBox[PentagramJewelSlot].m_Level != (info-1))
	{
		this->GCChaosMixSend(lpObj->Index,252,0);
		return;
	}

	PENTAGRAM_JEWEL_UPGRADE_LEVEL_INFO PentagramJewelUpgradeLevelInfo;

	if(gPentagramSystem.GetPentagramJewelUpgradeLevelInfo((info-1),&PentagramJewelUpgradeLevelInfo) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,248,0);
		return;
	}

	lpObj->ChaosSuccessRate = PentagramJewelUpgradeLevelInfo.MixRate[((lpObj->ChaosBox[PentagramJewelSlot].m_SocketOptionBonus/16)-1)]/100;

	if(this->GetElementalTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	lpObj->ChaosMoney = PentagramJewelUpgradeLevelInfo.MixMoney;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,249,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		CItem item = lpObj->ChaosBox[PentagramJewelSlot];

		item.m_Level++;

		item.m_SocketOption[((item.m_SocketOptionBonus/16)-1)] += 16;

		this->ChaosBoxInit(lpObj);

		gItemManager.ChaosBoxAddItem(lpObj->Index,item,0);

		this->GCChaosMixSend(lpObj->Index,1,&item);

		gLog.Output(LOG_CHAOS_MIX,"[PentagramJewelUpgradeLevelMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		if(ElementalChaosAmulet == 0)
		{
			this->ChaosBoxInit(lpObj);

			this->GCChaosBoxSend(lpObj,0);

			this->GCChaosMixSend(lpObj->Index,225,0);

			gLog.Output(LOG_CHAOS_MIX,"[PentagramJewelUpgradeLevelMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d, ChaosAmulet: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney,ElementalChaosAmulet);
		}
		else
		{
			this->ChaosBoxItemDown(lpObj,PentagramJewelSlot);

			this->GCChaosBoxSend(lpObj,0);

			this->GCChaosMixSend(lpObj->Index,225,0);

			gLog.Output(LOG_CHAOS_MIX,"[PentagramJewelUpgradeLevelMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d, ChaosAmulet: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney,ElementalChaosAmulet);
		}
	}

	#endif
}

void CChaosBox::PentagramJewelUpgradeRankMix(LPOBJ lpObj,BYTE info) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	int BlessCount = 0;
	int SoulCount = 0;
	int SpiritPowderCount = 0;
	int PentagramJewelCount = 0;
	int PentagramJewelSlot = 0;
	int SetItemCount = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,13))
		{
			BlessCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,14))
		{
			SoulCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,151))
		{
			SpiritPowderCount++;
		}
		else if(lpObj->ChaosBox[n].IsPentagramJewel() != 0 && lpObj->ChaosBox[n].m_Level >= 7)
		{
			PentagramJewelCount++;
			PentagramJewelSlot = n;
		}
		else if(lpObj->ChaosBox[n].IsSetItem() != 0 && lpObj->ChaosBox[n].m_Level >= 7 && lpObj->ChaosBox[n].m_Option3 >= 1)
		{
			SetItemCount++;
		}
	}

	if(BlessCount != (info-1) || SoulCount != (info-1) || SpiritPowderCount != (info-1) || PentagramJewelCount != 1 || SetItemCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index,250,0);
		return;
	}

	if(CHECK_RANGE(((lpObj->ChaosBox[PentagramJewelSlot].m_SocketOptionBonus/16)-1),(MAX_PENTAGRAM_JEWEL_RANK-1)) == 0 || (lpObj->ChaosBox[PentagramJewelSlot].m_SocketOptionBonus/16) != (info-1))
	{
		this->GCChaosMixSend(lpObj->Index,252,0);
		return;
	}

	PENTAGRAM_JEWEL_UPGRADE_RANK_INFO PentagramJewelUpgradeRankInfo;

	if(gPentagramSystem.GetPentagramJewelUpgradeRankInfo((info-1),&PentagramJewelUpgradeRankInfo) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,248,0);
		return;
	}

	lpObj->ChaosSuccessRate = PentagramJewelUpgradeRankInfo.MixRate/100;

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	lpObj->ChaosMoney = PentagramJewelUpgradeRankInfo.MixMoney;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,249,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		CItem item = lpObj->ChaosBox[PentagramJewelSlot];

		item.m_Level = 0;

		item.m_SocketOptionBonus += 16;

		if(gPentagramSystem.GetPentagramRandomJewelOption(item.m_Index,(item.m_SocketOptionBonus/16),&item.m_SocketOption[((item.m_SocketOptionBonus/16)-1)]) == 0)
		{
			this->GCChaosMixSend(lpObj->Index,248,0);
			return;
		}

		this->ChaosBoxInit(lpObj);

		gItemManager.ChaosBoxAddItem(lpObj->Index,item,0);

		this->GCChaosMixSend(lpObj->Index,1,&item);

		gLog.Output(LOG_CHAOS_MIX,"[PentagramJewelUpgradeRankMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,225,0);

		gLog.Output(LOG_CHAOS_MIX,"[PentagramJewelUpgradeRankMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}

	#endif
}

void CChaosBox::CGChaosMixRecv(PMSG_CHAOS_MIX_RECV* lpMsg,int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	if(lpObj->ChaosLock != 0)
	{
		return;
	}

	if(lpObj->PShopOpen != 0)
	{
		return;
	}

	lpObj->ChaosLock = 1;

	lpObj->ChaosMoney = 0;

	lpObj->ChaosSuccessRate = 0;

	lpObj->IsChaosMixCompleted = 1;

	switch(lpMsg->type)
	{
		case CHAOS_MIX_CHAOS_ITEM:
			this->ChaosItemMix(lpObj);
			break;
		case CHAOS_MIX_DEVIL_SQUARE:
			this->DevilSquareMix(lpObj);
			break;
		case CHAOS_MIX_PLUS_ITEM_LEVEL1:
			this->PlusItemLevelMix(lpObj,0);
			break;
		case CHAOS_MIX_PLUS_ITEM_LEVEL2:
			this->PlusItemLevelMix(lpObj,1);
			break;
		case CHAOS_MIX_DINORANT:
			this->DinorantMix(lpObj);
			break;
		case CHAOS_MIX_FRUIT:
			this->FruitMix(lpObj);
			break;
		case CHAOS_MIX_WING1:
			this->Wing2Mix(lpObj,0);
			break;
		case CHAOS_MIX_BLOOD_CASTLE:
			this->BloodCastleMix(lpObj);
			break;
		case CHAOS_MIX_WING2:
			this->Wing1Mix(lpObj);
			break;
		case CHAOS_MIX_PET1:
			this->PetMix(lpObj,0);
			break;
		case CHAOS_MIX_PET2:
			this->PetMix(lpObj,1);
			break;
		case CHAOS_MIX_SIEGE_POTION1:
			this->SiegePotionMix(lpObj,0);
			break;
		case CHAOS_MIX_SIEGE_POTION2:
			this->SiegePotionMix(lpObj,1);
			break;
		case CHAOS_MIX_LIFE_STONE:
			this->LifeStoneMix(lpObj);
			break;
		case CHAOS_MIX_SENIOR:
			this->SeniorMix(lpObj);
			break;
		case CHAOS_MIX_PLUS_ITEM_LEVEL3:
			this->PlusItemLevelMix(lpObj,2);
			break;
		case CHAOS_MIX_PLUS_ITEM_LEVEL4:
			this->PlusItemLevelMix(lpObj,3);
			break;
		case CHAOS_MIX_WING3:
			this->Wing2Mix(lpObj,1);
			break;
		case CHAOS_MIX_PIECE_OF_HORN:
			this->PieceOfHornMix(lpObj);
			break;
		case CHAOS_MIX_BROKEN_HORN:
			this->BrokenHornMix(lpObj);
			break;
		case CHAOS_MIX_HORN_OF_FENRIR:
			this->HornOfFenrirMix(lpObj);
			break;
		case CHAOS_MIX_HORN_OF_FENRIR_UPGRADE:
			this->HornOfFenrirUpgradeMix(lpObj);
			break;
		case CHAOS_MIX_SHIELD_POTION1:
			this->ShieldPotionMix(lpObj,0);
			break;
		case CHAOS_MIX_SHIELD_POTION2:
			this->ShieldPotionMix(lpObj,1);
			break;
		case CHAOS_MIX_SHIELD_POTION3:
			this->ShieldPotionMix(lpObj,2);
			break;
		case CHAOS_MIX_JEWEL_OF_HARMONY_ITEM_PURITY:
			this->JewelOfHarmonyItemPurityMix(lpObj);
			break;
		case CHAOS_MIX_JEWEL_OF_HARMONY_ITEM_SMELT:
			this->JewelOfHarmonyItemSmeltMix(lpObj);
			break;
		case CHAOS_MIX_JEWEL_OF_HARMONY_ITEM_RESTORE:
			this->JewelOfHarmonyItemRestoreMix(lpObj);
			break;
		case CHAOS_MIX_ITEM_380:
			this->Item380Mix(lpObj);
			break;
		case CHAOS_MIX_ILLUSION_TEMPLE:
			this->IllusionTempleMix(lpObj);
			break;
		case CHAOS_MIX_FEATHER_OF_CONDOR:
			this->FeatherOfCondorMix(lpObj);
			break;
		case CHAOS_MIX_WING4:
			this->Wing3Mix(lpObj);
			break;
		case CHAOS_MIX_CHAOS_CARD:
			this->ChaosCardMix(lpObj,0);
			break;
		case CHAOS_MIX_CHERRY_BLOSSOM:
			this->CherryBlossomMix(lpObj,0);
			break;
		case CHAOS_MIX_SOCKET_ITEM_CREATE_SEED:
			this->SocketItemCreateSeedMix(lpObj);
			break;
		case CHAOS_MIX_SOCKET_ITEM_CREATE_SEED_SPHERE:
			this->SocketItemCreateSeedSphereMix(lpObj);
			break;
		case CHAOS_MIX_SOCKET_ITEM_MOUNT_SEED_SPHERE:
			this->SocketItemMountSeedSphereMix(lpObj,lpMsg->info);
			break;
		case CHAOS_MIX_SOCKET_ITEM_UN_MOUNT_SEED_SPHERE:
			this->SocketItemUnMountSeedSphereMix(lpObj,lpMsg->info);
			break;
		case CHAOS_MIX_IMPERIAL_GUARDIAN:
			this->ImperialGuardianMix(lpObj);
			break;
		case CHAOS_MIX_CHEST:
			this->ChestMix(lpObj);
			break;
		case CHAOS_MIX_SUMMON_SCROLL:
			this->SummonScrollMix(lpObj,0);
			break;
		case CHAOS_MIX_PLUS_ITEM_LEVEL5:
			this->PlusItemLevelMix(lpObj,4);
			break;
		case CHAOS_MIX_PLUS_ITEM_LEVEL6:
			this->PlusItemLevelMix(lpObj,5);
			break;
		case CHAOS_MIX_LUCKY_ITEM_CREATE:
			this->LuckyItemCreateMix(lpObj);
			break;
		case CHAOS_MIX_LUCKY_ITEM_REFINE:
			this->LuckyItemRefineMix(lpObj);
			break;
		case CHAOS_MIX_MONSTER_WING:
			this->MonsterWingMix(lpObj);
			break;
		case CHAOS_MIX_SOCKET_ITEM:
			this->SocketWeaponMix(lpObj);
			break;
		case CHAOS_MIX_SOCKET_SET:
			this->SocketHelmMix(lpObj);
			this->SocketArmorMix(lpObj);
			this->SocketPantMix(lpObj);
			this->SocketGloveMix(lpObj);
			this->SocketBootMix(lpObj);
			//--
			this->SocketRFHelmMix(lpObj);
			this->SocketRFArmorMix(lpObj);
			this->SocketRFPantMix(lpObj);
			this->SocketRFBootMix(lpObj);
			break;
		//Talisman Wing 1 Mix
		case CHAOS_MIX_WING1DK:
			this->Wing1DKMix(lpObj);
			break;
		case CHAOS_MIX_WING1DW:
			this->Wing1DWMix(lpObj);
			break;
		case CHAOS_MIX_WING1FE:
			this->Wing1FEMix(lpObj);
			break;
		case CHAOS_MIX_WING1SU:
			this->Wing1SUMix(lpObj);
			break;
		//Talisman Wing 2 Mix
		case CHAOS_MIX_WING2DL:
			this->Wing2DLMix(lpObj);
			break;
		case CHAOS_MIX_WING2BK:
			this->Wing2BKMix(lpObj);
			break;
		case CHAOS_MIX_WING2SM:
			this->Wing2SMMix(lpObj);
			break;
		case CHAOS_MIX_WING2ME:
			this->Wing2MEMix(lpObj);
			break;
		case CHAOS_MIX_WING2BS:
			this->Wing2BSMix(lpObj);
			break;
		case CHAOS_MIX_WING2MG:
			this->Wing2MGMix(lpObj);
			break;
		case CHAOS_MIX_WING2RF:
			this->Wing2RFMix(lpObj);
			break;
		//Talisman Wing 3 Mix
		case CHAOS_MIX_WING3LE:
			this->Wing3LEMix(lpObj);
			break;	
		case CHAOS_MIX_WING3BM:
			this->Wing3BMMix(lpObj);
			break;	
		case CHAOS_MIX_WING3GM:
			this->Wing3GMMix(lpObj);
			break;	
		case CHAOS_MIX_WING3HE:
			this->Wing3HEMix(lpObj);
			break;	
		case CHAOS_MIX_WING3DM:
			this->Wing3DMMix(lpObj);
			break;	
		case CHAOS_MIX_WING3SU:
			this->Wing3SUMix(lpObj);
			break;	
		case CHAOS_MIX_WING3FM:
			this->Wing3FMMix(lpObj);
			break;
		//Classic Pet Mix
		case CHAOS_MIX_SPIRIT_GUARDIAN:
			this->SpiritGuardianMix(lpObj);
			break;
		case CHAOS_MIX_DEMON:
			this->DemonMix(lpObj);
			break;
		case CHAOS_MIX_GOLD_FENRIR:
			this->GoldFenrirMix(lpObj);
			break;
		//New Refine Harmony
		case CHAOS_MIX_JEWEL_OF_HARMONY_ITEM_PURITY_10:
			this->JewelOfHarmonyItemPurity10Mix(lpObj);
			break;
		case CHAOS_MIX_JEWEL_OF_HARMONY_ITEM_PURITY_20:
			this->JewelOfHarmonyItemPurity20Mix(lpObj);
			break;
		case CHAOS_MIX_JEWEL_OF_HARMONY_ITEM_PURITY_30:
			this->JewelOfHarmonyItemPurity30Mix(lpObj);
			break;
		//Conqueror Wings
		case CHAOS_MIX_WINGS_OF_CONQUEROR:
			this->WingsOfConquerorMix(lpObj);
			break;
		case CHAOS_MIX_ANCIENT_HERO_SOUL:
			this->AncientHeroSoulMix(lpObj);
			break;
		case CHAOS_MIX_BLOODANGEL_HELM:
			this->BloodangelHelmMix(lpObj);
			break;
		case CHAOS_MIX_BLOODANGEL_ARMOR:
			this->BloodangelArmorMix(lpObj);
			break;
		case CHAOS_MIX_BLOODANGEL_PANTS:
			this->BloodangelPantsMix(lpObj);
			break;
		case CHAOS_MIX_BLOODANGEL_GLOVES:
			this->BloodangelGlovesMix(lpObj);
			break;
		case CHAOS_MIX_BLOODANGEL_BOOTS:
			this->BloodangelBootsMix(lpObj);
			break;
		case CHAOS_MIX_DARKANGEL_HELM:
			this->DarkangelHelmMix(lpObj);
			break;
		case CHAOS_MIX_DARKANGEL_ARMOR:
			this->DarkangelArmorMix(lpObj);
			break;
		case CHAOS_MIX_DARKANGEL_PANTS:
			this->DarkangelPantsMix(lpObj);
			break;
		case CHAOS_MIX_DARKANGEL_GLOVES:
			this->DarkangelGlovesMix(lpObj);
			break;
		case CHAOS_MIX_DARKANGEL_BOOTS:
			this->DarkangelBootsMix(lpObj);
			break;
		case CHAOS_MIX_DARKANGEL_HELM_EE:
			this->DarkangelEnergyElfHelmMix(lpObj);
			break;
		case CHAOS_MIX_DARKANGEL_ARMOR_EE:
			this->DarkangelEnergyElfArmorMix(lpObj);
			break;
		case CHAOS_MIX_DARKANGEL_PANTS_EE:
			this->DarkangelEnergyElfPantsMix(lpObj);
			break;
		case CHAOS_MIX_DARKANGEL_GLOVES_EE:
			this->DarkangelEnergyElfGlovesMix(lpObj);
			break;
		case CHAOS_MIX_DARKANGEL_ARMOR_EMG:
			this->DarkangelEnergyMagicArmorMix(lpObj);
			break;
		case CHAOS_MIX_DARKANGEL_PANTS_EMG:
			this->DarkangelEnergyMagicPantsMix(lpObj);
			break;
		case CHAOS_MIX_DARKANGEL_GLOVES_EMG:
			this->DarkangelEnergyMagicGlovesMix(lpObj);
			break;
		case CHAOS_MIX_DARKANGEL_BOOTS_EMG:
			this->DarkangelEnergyMagicBootsMix(lpObj);
			break;
		case CHAOS_MIX_HOLYANGEL_HELM:
			this->HolyangelHelmMix(lpObj);
			break;
		case CHAOS_MIX_HOLYANGEL_ARMOR:
			this->HolyangelArmorMix(lpObj);
			break;
		case CHAOS_MIX_HOLYANGEL_PANTS:
			this->HolyangelPantsMix(lpObj);
			break;
		case CHAOS_MIX_HOLYANGEL_GLOVES:
			this->HolyangelGlovesMix(lpObj);
			break;
		case CHAOS_MIX_HOLYANGEL_BOOTS:
			this->HolyangelBootsMix(lpObj);
			break;
		case CHAOS_MIX_HOLYANGEL_HELM_EE:
			this->HolyangelEnergyElfHelmMix(lpObj);
			break;
		case CHAOS_MIX_HOLYANGEL_ARMOR_EE:
			this->HolyangelEnergyElfArmorMix(lpObj);
			break;
		case CHAOS_MIX_HOLYANGEL_PANTS_EE:
			this->HolyangelEnergyElfPantsMix(lpObj);
			break;
		case CHAOS_MIX_HOLYANGEL_GLOVES_EE:
			this->HolyangelEnergyElfGlovesMix(lpObj);
			break;
		case CHAOS_MIX_HOLYANGEL_ARMOR_EMG:
			this->HolyangelEnergyMagicArmorMix(lpObj);
			break;
		case CHAOS_MIX_HOLYANGEL_PANTS_EMG:
			this->HolyangelEnergyMagicPantsMix(lpObj);
			break;
		case CHAOS_MIX_HOLYANGEL_GLOVES_EMG:
			this->HolyangelEnergyMagicGlovesMix(lpObj);
			break;
		case CHAOS_MIX_HOLYANGEL_BOOTS_EMG:
			this->HolyangelEnergyMagicBootsMix(lpObj);
			break;
		case CHAOS_MIX_AWAKENING_HELM:
			this->AwakeningHelmMix(lpObj);
			break;
		case CHAOS_MIX_AWAKENING_ARMOR:
			this->AwakeningArmorMix(lpObj);
			break;
		case CHAOS_MIX_AWAKENING_PANTS:
			this->AwakeningPantsMix(lpObj);
			break;
		case CHAOS_MIX_AWAKENING_GLOVES:
			this->AwakeningGlovesMix(lpObj);
			break;
		case CHAOS_MIX_AWAKENING_BOOTS:
			this->AwakeningBootsMix(lpObj);
			break;
		case CHAOS_MIX_AWAKENING_HELM_EE:
			this->AwakeningEnergyElfHelmMix(lpObj);
			break;
		case CHAOS_MIX_AWAKENING_ARMOR_EE:
			this->AwakeningEnergyElfArmorMix(lpObj);
			break;
		case CHAOS_MIX_AWAKENING_PANTS_EE:
			this->AwakeningEnergyElfPantsMix(lpObj);
			break;
		case CHAOS_MIX_AWAKENING_GLOVES_EE:
			this->AwakeningEnergyElfGlovesMix(lpObj);
			break;
		case CHAOS_MIX_AWAKENING_ARMOR_EMG:
			this->AwakeningEnergyMagicArmorMix(lpObj);
			break;
		case CHAOS_MIX_AWAKENING_PANTS_EMG:
			this->AwakeningEnergyMagicPantsMix(lpObj);
			break;
		case CHAOS_MIX_AWAKENING_GLOVES_EMG:
			this->AwakeningEnergyMagicGlovesMix(lpObj);
			break;
		case CHAOS_MIX_AWAKENING_BOOTS_EMG:
			this->AwakeningEnergyMagicBootsMix(lpObj);
			break;
		case CHAOS_MIX_BLUE_EYE_HELM:
			this->BlueEyeHelmMix(lpObj);
			break;
		case CHAOS_MIX_BLUE_EYE_ARMOR:
			this->BlueEyeArmorMix(lpObj);
			break;
		case CHAOS_MIX_BLUE_EYE_PANTS:
			this->BlueEyePantsMix(lpObj);
			break;
		case CHAOS_MIX_BLUE_EYE_GLOVES:
			this->BlueEyeGlovesMix(lpObj);
			break;
		case CHAOS_MIX_BLUE_EYE_BOOTS:
			this->BlueEyeBootsMix(lpObj);
			break;
		case CHAOS_MIX_FLEET_SILVER_HEART_HELM:
			this->FleetSilverHeartHelmMix(lpObj);
			break;
		case CHAOS_MIX_FLEET_SILVER_HEART_ARMOR:
			this->FleetSilverHeartArmorMix(lpObj);
			break;
		case CHAOS_MIX_FLEET_SILVER_HEART_PANTS:
			this->FleetSilverHeartPantsMix(lpObj);
			break;
		case CHAOS_MIX_FLEET_SILVER_HEART_GLOVES:
			this->FleetSilverHeartGlovesMix(lpObj);
			break;
		case CHAOS_MIX_FLEET_SILVER_HEART_BOOTS:
			this->FleetSilverHeartBootsMix(lpObj);
			break;
		case CHAOS_MIX_ROARING_MANTICORE_HELM:
			this->RoaringManticoreHelmMix(lpObj);
			break;
		case CHAOS_MIX_ROARING_MANTICORE_ARMOR:
			this->RoaringManticoreArmorMix(lpObj);
			break;
		case CHAOS_MIX_ROARING_MANTICORE_PANTS:
			this->RoaringManticorePantsMix(lpObj);
			break;
		case CHAOS_MIX_ROARING_MANTICORE_GLOVES:
			this->RoaringManticoreGlovesMix(lpObj);
			break;
		case CHAOS_MIX_ROARING_MANTICORE_BOOTS:
			this->RoaringManticoreBootsMix(lpObj);
			break;
		case CHAOS_MIX_HAMMER_OF_ARCHANGEL:
			this->HammerArchangelMix(lpObj,0);
			break;
		case CHAOS_MIX_ABSOLUTE_SWORD_OF_ARCHANGEL:
			this->AbsoluteArchangelSwordMix(lpObj,0);
			break;
		case CHAOS_MIX_ABSOLUTE_SCEPTER_OF_ARCHANGEL:
			this->AbsoluteArchangelScepterMix(lpObj,0);
			break;
		case CHAOS_MIX_ABSOLUTE_CROSSBOW_OF_ARCHANGEL:
			this->AbsoluteArchangelCrossbowMix(lpObj,0);
			break;
		case CHAOS_MIX_ABSOLUTE_STAFF_OF_ARCHANGEL:
			this->AbsoluteArchangelStaffMix(lpObj,0);
			break;
		case CHAOS_MIX_ABSOLUTE_STICK_OF_ARCHANGEL:
			this->AbsoluteArchangelStickMix(lpObj,0);
			break;
		case CHAOS_MIX_ABSOLUTE_CLAW_OF_ARCHANGEL:
			this->AbsoluteArchangelClawMix(lpObj,0);
			break;
		case CHAOS_MIX_DARKANGEL_SWORD:
			this->DarkangelSwordMix(lpObj,0);
			break;
		case CHAOS_MIX_DARKANGEL_MAGIC_SWORD:
			this->DarkangelMagicSwordMix(lpObj,0);
			break;
		case CHAOS_MIX_DARKANGEL_CLAW:
			this->DarkangelClawMix(lpObj,0);
			break;
		case CHAOS_MIX_DARKANGEL_SCEPTER:
			this->DarkangelScepterMix(lpObj,0);
			break;
		case CHAOS_MIX_DARKANGEL_BOW:
			this->DarkangelBowMix(lpObj,0);
			break;
		case CHAOS_MIX_DARKANGEL_STAFF:
			this->DarkangelStaffMix(lpObj,0);
			break;
		case CHAOS_MIX_DARKANGEL_STICK:
			this->DarkangelStickMix(lpObj,0);
			break;
		case CHAOS_MIX_HOLYANGEL_SWORD:
			this->HolyangelSwordMix(lpObj,0);
			break;
		case CHAOS_MIX_HOLYANGEL_MAGIC_SWORD:
			this->HolyangelMagicSwordMix(lpObj,0);
			break;
		case CHAOS_MIX_HOLYANGEL_CLAW:
			this->HolyangelClawMix(lpObj,0);
			break;
		case CHAOS_MIX_HOLYANGEL_SCEPTER:
			this->HolyangelScepterMix(lpObj,0);
			break;
		case CHAOS_MIX_HOLYANGEL_BOW:
			this->HolyangelBowMix(lpObj,0);
			break;
		case CHAOS_MIX_HOLYANGEL_STAFF:
			this->HolyangelStaffMix(lpObj,0);
			break;
		case CHAOS_MIX_HOLYANGEL_STICK:
			this->HolyangelStickMix(lpObj,0);
			break;
		case CHAOS_MIX_AWAKENING_SOUL_SWORD:
			this->SoulSwordMix(lpObj,0);
			break;
		case CHAOS_MIX_AWAKENING_SOUL_MAGIC_SWORD:
			this->SoulMagicSwordMix(lpObj,0);
			break;
		case CHAOS_MIX_AWAKENING_SOUL_CLAW:
			this->SoulClawMix(lpObj,0);
			break;
		case CHAOS_MIX_AWAKENING_SOUL_SCEPTER:
			this->SoulScepterMix(lpObj,0);
			break;
		case CHAOS_MIX_AWAKENING_SOUL_BOW:
			this->SoulBowMix(lpObj,0);
			break;
		case CHAOS_MIX_AWAKENING_SOUL_STAFF:
			this->SoulStaffMix(lpObj,0);
			break;
		case CHAOS_MIX_AWAKENING_SOUL_STICK:
			this->SoulStickMix(lpObj,0);
			break;
		case CHAOS_MIX_BLUE_EYE_SWORD:
			this->BlueEyeSwordMix(lpObj,0);
			break;
		case CHAOS_MIX_BLUE_EYE_MAGIC_SWORD:
			this->BlueEyeMagicSwordMix(lpObj,0);
			break;
		case CHAOS_MIX_BLUE_EYE_CLAW:
			this->BlueEyeClawMix(lpObj,0);
			break;
		case CHAOS_MIX_BLUE_EYE_SCEPTER:
			this->BlueEyeScepterMix(lpObj,0);
			break;
		case CHAOS_MIX_BLUE_EYE_BOW:
			this->BlueEyeBowMix(lpObj,0);
			break;
		case CHAOS_MIX_BLUE_EYE_STAFF:
			this->BlueEyeStaffMix(lpObj,0);
			break;
		case CHAOS_MIX_BLUE_EYE_STICK:
			this->BlueEyeStickMix(lpObj,0);
			break;	
		default:
			this->CustomItemMix(lpObj,lpMsg->type);
			break;
	}
}

void CChaosBox::CGChaosMixCloseRecv(int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	if(lpObj->Interface.use == 0 || lpObj->Interface.type != INTERFACE_CHAOS_BOX)
	{
		return;
	}

	lpObj->Interface.use = 0;
	lpObj->Interface.type = INTERFACE_NONE;
	lpObj->Interface.state = 0;

	this->ChaosBoxInit(lpObj);

	gObjInventoryCommit(aIndex);

	lpObj->ChaosLock = 0;

	lpObj->IsChaosMixCompleted = 0;

	lpObj->IsCastleNPCUpgradeCompleted = 0;

	PBMSG_HEAD pMsg;

	pMsg.set(0x87,sizeof(pMsg));

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.size);
}

void CChaosBox::GCChaosBoxSend(LPOBJ lpObj,BYTE type) // OK
{
	if((type == 0 && (lpObj->Interface.type != INTERFACE_CHAOS_BOX || lpObj->Interface.state == 1)) || (type != 0 && (lpObj->Interface.type != INTERFACE_TRAINER || lpObj->Interface.state == 1)))
	{
		return;
	}

	BYTE send[2048];

	PMSG_SHOP_ITEM_LIST_SEND pMsg;

	pMsg.header.set(0x31,0);

	int size = sizeof(pMsg);

	pMsg.type = ((type==0)?3:5);

	pMsg.count = 0;

	PMSG_SHOP_ITEM_LIST info;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		info.slot = n;

		gItemManager.ItemByteConvert(info.ItemInfo,lpObj->ChaosBox[n]);

		memcpy(&send[size],&info,sizeof(info));
		size += sizeof(info);

		pMsg.count++;
	}

	pMsg.header.size[0] = SET_NUMBERHB(size);
	pMsg.header.size[1] = SET_NUMBERLB(size);

	memcpy(send,&pMsg,sizeof(pMsg));

	DataSend(lpObj->Index,send,size);
}

void CChaosBox::GCChaosMixSend(int aIndex,BYTE result,CItem* lpItem) // OK
{
	PMSG_CHAOS_MIX_SEND pMsg;

	pMsg.header.set(0x86,sizeof(pMsg));

	pMsg.result = result;

	if(lpItem == 0)
	{
		memset(pMsg.ItemInfo,0xFF,sizeof(pMsg.ItemInfo));
	}
	else
	{
		gItemManager.ItemByteConvert(pMsg.ItemInfo,(*lpItem));
	}

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);

	gObj[aIndex].ChaosLock = 0;
}

void CChaosBox::CustomItemMix(LPOBJ lpObj, int type) // OK
{
	CUSTOM_MIX* CustomMix = gCustomMix.GetCustomMix(type);

	if(CustomMix){

		int itemMixCount,groupMix = -1;

		int ItemBagMixCount = 0;

		int itemCountComplet = CustomMix->m_CountItem;

		int ItemMixGroupCount = itemCountComplet - gCustomMix.GetCountItemBagMix(CustomMix->m_Index);

		//-- primera comprobación de item bag no cambiantes
		for(int slot = 0;slot < CHAOS_BOX_SIZE;slot++)
		{
			if(lpObj->ChaosBox[slot].IsItem() == 0)
			{
				continue;
			}

			if(gCustomMix.istItemBagMix(CustomMix->m_Index,lpObj->ChaosBox[slot])){
				ItemBagMixCount++;
			}
		}

		//validación de item por grupos existentes
		for(int Group = 0 ; Group < CustomMix->m_CountGroup ; Group++){
			
			itemMixCount = 0;

			for(int slot = 0;slot < CHAOS_BOX_SIZE;slot++)
			{
				if(lpObj->ChaosBox[slot].IsItem() == 0)
				{
					continue;
				}

				if(gCustomMix.istItemMix(CustomMix->m_Index, Group,lpObj->ChaosBox[slot]))	// validación si existe el item
				{
					itemMixCount++;
				}
			}

			if(itemMixCount == ItemMixGroupCount){
				groupMix = Group;
				Group = CustomMix->m_CountGroup;
			}
		}
	
		if((itemMixCount + ItemBagMixCount) != itemCountComplet)	// falló la combinación
		{
			this->GCChaosMixSend(lpObj->Index,7,0);
			return;
		}
	
		if(this->GetTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
		{
			this->GCChaosMixSend(lpObj->Index,240,0);
			return;
		}

		switch(lpObj->AccountLevel){
			case 1:
				lpObj->ChaosSuccessRate =  ((CustomMix->m_MixRate_AL1 < 0 || CustomMix->m_MixRate_AL1 > 100)?90:CustomMix->m_MixRate_AL1);
				break;
			case 2:
				lpObj->ChaosSuccessRate =  ((CustomMix->m_MixRate_AL2 < 0 || CustomMix->m_MixRate_AL2 > 100)?90:CustomMix->m_MixRate_AL2);
				break;
			case 3:
				lpObj->ChaosSuccessRate =  ((CustomMix->m_MixRate_AL3 < 0 || CustomMix->m_MixRate_AL3 > 100)?90:CustomMix->m_MixRate_AL3);
				break;
			default:
				lpObj->ChaosSuccessRate =  ((CustomMix->m_MixRate_AL0 < 0 || CustomMix->m_MixRate_AL0 > 100)?90:CustomMix->m_MixRate_AL0);
				break;
		}
	
		lpObj->ChaosMoney = lpObj->ChaosSuccessRate * ((CustomMix->m_MixMoney < 0)?1000:CustomMix->m_MixMoney);

		int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

		lpObj->ChaosMoney += TaxMoney;

		if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
		{
			this->GCChaosMixSend(lpObj->Index,2,0);
			return;
		}

		lpObj->Money -= lpObj->ChaosMoney;

		GCMoneySend(lpObj->Index,lpObj->Money);

		gCastleSiegeSync.AddTributeMoney(TaxMoney);

		if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
		{
		
			WORD ItemIndex = 0;
			BYTE ItemOption1 = 0;
			BYTE ItemOption2 = 0;
			BYTE ItemOption3 = 0;
			BYTE ItemNewOption = 0;
			BYTE ItemSetOption = 0;

			CRandomManager RandomManager = gCustomMix.RandomManager(CustomMix->m_Index,groupMix);

			RandomManager.GetRandomElement(&ItemIndex);

			CUSTOM_ITEM_MIX_RESULT* ItemCreate = gCustomMix.GetItemResult(CustomMix->m_Index,groupMix,ItemIndex);

			if(ItemCreate->m_Skill == -1){
				gItemOptionRate.GetItemOption1(6,&ItemOption1);
			}else{
				ItemOption1 = ItemCreate->m_Skill;
			}
		
			if(ItemCreate->m_Luck == -1){
				gItemOptionRate.GetItemOption2(6,&ItemOption2);
			}else{
				ItemOption2 = ItemCreate->m_Luck;
			}

			if(ItemCreate->m_Opcion == -1){
				gItemOptionRate.GetItemOption3(6,&ItemOption3);
			}else{
				ItemOption3 = ItemCreate->m_Opcion;
			}

			if(ItemCreate->m_Excelent == -1){

				gItemOptionRate.GetItemOption4(6,&ItemNewOption);

				gItemOptionRate.MakeNewOption(ItemIndex,ItemNewOption,&ItemNewOption);

			}else{
				ItemNewOption = ItemCreate->m_Excelent;
			}

			if(ItemCreate->m_SetACC == -1){

				gItemOptionRate.GetItemOption5(6,&ItemSetOption);

				gItemOptionRate.MakeSetOption(ItemIndex,ItemSetOption,&ItemSetOption);

			}else{
				ItemSetOption = ItemCreate->m_SetACC;
			}

			if(ItemNewOption == 0){
				GDCreateItemSend(lpObj->Index,0xFF,0,0,ItemIndex,0,0,0,ItemOption2,ItemOption3,-1,ItemNewOption,ItemSetOption,0,0,0,0xFF,0);
			}else{
				GDCreateItemSend(lpObj->Index,0xFF,0,0,ItemIndex,0,0,0,ItemOption2,ItemOption3,-1,(ItemNewOption+(16*(GetLargeRand()%3))),ItemSetOption,0,0,0,0xFF,0);
			}
			gLog.Output(LOG_CHAOS_MIX,"[CustomMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,100,lpObj->ChaosMoney);
		}
		else
		{
			this->ChaosBoxInit(lpObj);

			this->GCChaosBoxSend(lpObj,0);

			this->GCChaosMixSend(lpObj->Index,0,0);

			gLog.Output(LOG_CHAOS_MIX,"[CustomMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
		}
	}
	else{
		gLog.Output(LOG_CHAOS_MIX,"[CustomMix][Failure][%s][%s] - CustomMix No Existente",lpObj->Account,lpObj->Name);		
	}
}


//Socket Set Upgrade

void CChaosBox::SocketHelmMix(LPOBJ lpObj) // OK
{
	int CombinationNote = 0;
	int SocketType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 364))
		{
			CombinationNote++;
		}

		else if (lpObj->ChaosBox[n].m_Index >= GET_ITEM(7, 44) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(7, 53) || lpObj->ChaosBox[n].m_Index >= GET_ITEM(7, 146) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(7, 147))
		{
			SocketType = lpObj->ChaosBox[n].m_Index - GET_ITEM(7, 43);
		}
	}

	if ( SocketType == 0 || CombinationNote == 0 || (SocketType == 1 && (CombinationNote != 1)) || (SocketType == 2 && (CombinationNote != 1)) || (SocketType == 3 && (CombinationNote != 1)) || (SocketType == 4 && (CombinationNote != 1)) || (SocketType == 5 && (CombinationNote != 1)) || (SocketType == 6 && (CombinationNote != 1)) || (SocketType == 7 && (CombinationNote != 1)) || (SocketType == 8 && (CombinationNote != 1)) || (SocketType == 9 && (CombinationNote != 1)))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_SocketSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_SocketSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_SocketSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 75000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;
		BYTE ItemSocketOption[MAX_SOCKET_OPTION] = {0xFF,0xFF,0xFF,0xFF,0xFF};

		switch (SocketType)
		{
		case 1:
			ItemIndex = GET_ITEM(7, 90);
			break;
		case 2:
			ItemIndex = GET_ITEM(7, 74);
			break;
		case 3:
			ItemIndex = GET_ITEM(7, 54);
			break;
		case 4:
			ItemIndex = GET_ITEM(7, 57);
			break;
		case 5:
			ItemIndex = GET_ITEM(7, 76);
			break;
		case 6:
			ItemIndex = GET_ITEM(7, 56);
			break;
		case 7:
			ItemIndex = GET_ITEM(7, 75);
			break;
		case 8:
			ItemIndex = GET_ITEM(7, 58);
			break;
		case 9:
			ItemIndex = GET_ITEM(7, 55);
			break;
		case 10:
			ItemIndex = GET_ITEM(7, 90);
			break;
		}

		gItemOptionRate.GetItemOption2(8,&ItemOption2);

		gItemOptionRate.GetItemOption6(((gItemManager.GetItemTwoHand(ItemIndex)==0)?6:7),&ItemSocketOption[0]);

		gItemOptionRate.MakeSocketOption(ItemIndex,ItemSocketOption[0],&ItemSocketOption[0]);

		GDCreateItemSend(lpObj->Index,0xFF,0,0,ItemIndex,0,0,1,ItemOption2,0,-1,0,0,0,0,ItemSocketOption,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[SocketHelmMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[SocketHelmMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::SocketArmorMix(LPOBJ lpObj) // OK
{
	int CombinationNote = 0;
	int SocketType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 364))
		{
			CombinationNote++;
		}

		else if (lpObj->ChaosBox[n].m_Index >= GET_ITEM(8, 44) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(8, 53) || lpObj->ChaosBox[n].m_Index >= GET_ITEM(8, 146) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(8, 147))
		{
			SocketType = lpObj->ChaosBox[n].m_Index - GET_ITEM(8, 43);
		}
	}

	if ( SocketType == 0 || CombinationNote == 0 || (SocketType == 1 && (CombinationNote != 1)) || (SocketType == 2 && (CombinationNote != 1)) || (SocketType == 3 && (CombinationNote != 1)) || (SocketType == 4 && (CombinationNote != 1)) || (SocketType == 5 && (CombinationNote != 1)) || (SocketType == 6 && (CombinationNote != 1)) || (SocketType == 7 && (CombinationNote != 1)) || (SocketType == 8 && (CombinationNote != 1)) || (SocketType == 9 && (CombinationNote != 1)) || (SocketType == 10 && (CombinationNote != 1)))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_SocketSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_SocketSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_SocketSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 75000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;
		BYTE ItemSocketOption[MAX_SOCKET_OPTION] = {0xFF,0xFF,0xFF,0xFF,0xFF};

		switch (SocketType)
		{
		case 1:
			ItemIndex = GET_ITEM(8, 90);
			break;
		case 2:
			ItemIndex = GET_ITEM(8, 74);
			break;
		case 3:
			ItemIndex = GET_ITEM(8, 54);
			break;
		case 4:
			ItemIndex = GET_ITEM(8, 57);
			break;
		case 5:
			ItemIndex = GET_ITEM(8, 76);
			break;
		case 6:
			ItemIndex = GET_ITEM(8, 56);
			break;
		case 7:
			ItemIndex = GET_ITEM(8, 75);
			break;
		case 8:
			ItemIndex = GET_ITEM(8, 58);
			break;
		case 9:
			ItemIndex = GET_ITEM(8, 55);
			break;
		case 10:
			ItemIndex = GET_ITEM(8, 90);
			break;
		}

		gItemOptionRate.GetItemOption2(8,&ItemOption2);

		gItemOptionRate.GetItemOption6(((gItemManager.GetItemTwoHand(ItemIndex)==0)?6:7),&ItemSocketOption[0]);

		gItemOptionRate.MakeSocketOption(ItemIndex,ItemSocketOption[0],&ItemSocketOption[0]);

		GDCreateItemSend(lpObj->Index,0xFF,0,0,ItemIndex,0,0,1,ItemOption2,0,-1,0,0,0,0,ItemSocketOption,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[SocketArmorMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[SocketArmorMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::SocketPantMix(LPOBJ lpObj) // OK
{
	int CombinationNote = 0;
	int SocketType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 364))
		{
			CombinationNote++;
		}

		else if (lpObj->ChaosBox[n].m_Index >= GET_ITEM(9, 44) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(9, 53) || lpObj->ChaosBox[n].m_Index >= GET_ITEM(9, 146) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(9, 147))
		{
			SocketType = lpObj->ChaosBox[n].m_Index - GET_ITEM(9, 43);
		}
	}

	if ( SocketType == 0 || CombinationNote == 0 || (SocketType == 1 && (CombinationNote != 1)) || (SocketType == 2 && (CombinationNote != 1)) || (SocketType == 3 && (CombinationNote != 1)) || (SocketType == 4 && (CombinationNote != 1)) || (SocketType == 5 && (CombinationNote != 1)) || (SocketType == 6 && (CombinationNote != 1)) || (SocketType == 7 && (CombinationNote != 1)) || (SocketType == 8 && (CombinationNote != 1)) || (SocketType == 9 && (CombinationNote != 1)))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_SocketSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_SocketSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_SocketSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 75000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;
		BYTE ItemSocketOption[MAX_SOCKET_OPTION] = {0xFF,0xFF,0xFF,0xFF,0xFF};

		switch (SocketType)
		{
		case 1:
			ItemIndex = GET_ITEM(9, 90);
			break;
		case 2:
			ItemIndex = GET_ITEM(9, 74);
			break;
		case 3:
			ItemIndex = GET_ITEM(9, 54);
			break;
		case 4:
			ItemIndex = GET_ITEM(9, 57);
			break;
		case 5:
			ItemIndex = GET_ITEM(9, 76);
			break;
		case 6:
			ItemIndex = GET_ITEM(9, 56);
			break;
		case 7:
			ItemIndex = GET_ITEM(9, 75);
			break;
		case 8:
			ItemIndex = GET_ITEM(9, 58);
			break;
		case 9:
			ItemIndex = GET_ITEM(9, 55);
			break;
		case 10:
			ItemIndex = GET_ITEM(9, 90);
			break;
		}

		gItemOptionRate.GetItemOption2(8,&ItemOption2);

		gItemOptionRate.GetItemOption6(((gItemManager.GetItemTwoHand(ItemIndex)==0)?6:7),&ItemSocketOption[0]);

		gItemOptionRate.MakeSocketOption(ItemIndex,ItemSocketOption[0],&ItemSocketOption[0]);

		GDCreateItemSend(lpObj->Index,0xFF,0,0,ItemIndex,0,0,1,ItemOption2,0,-1,0,0,0,0,ItemSocketOption,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[SocketPantMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[SocketPantMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::SocketGloveMix(LPOBJ lpObj) // OK
{
	int CombinationNote = 0;
	int SocketType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 364))
		{
			CombinationNote++;
		}

		else if (lpObj->ChaosBox[n].m_Index >= GET_ITEM(10, 44) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(10, 53) || lpObj->ChaosBox[n].m_Index >= GET_ITEM(10, 146) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(10, 147))
		{
			SocketType = lpObj->ChaosBox[n].m_Index - GET_ITEM(10, 43);
		}
	}

	if ( SocketType == 0 || CombinationNote == 0 || (SocketType == 1 && (CombinationNote != 1)) || (SocketType == 2 && (CombinationNote != 1)) || (SocketType == 3 && (CombinationNote != 1)) || (SocketType == 4 && (CombinationNote != 1)) || (SocketType == 5 && (CombinationNote != 1)) || (SocketType == 6 && (CombinationNote != 1)) || (SocketType == 7 && (CombinationNote != 1)) || (SocketType == 8 && (CombinationNote != 1)) || (SocketType == 9 && (CombinationNote != 1)) || (SocketType == 10 && (CombinationNote != 1)))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_SocketSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_SocketSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_SocketSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 75000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;
		BYTE ItemSocketOption[MAX_SOCKET_OPTION] = {0xFF,0xFF,0xFF,0xFF,0xFF};

		switch (SocketType)
		{
		case 1:
			ItemIndex = GET_ITEM(10, 90);
			break;
		case 2:
			ItemIndex = GET_ITEM(10, 74);
			break;
		case 3:
			ItemIndex = GET_ITEM(10, 54);
			break;
		case 4:
			ItemIndex = GET_ITEM(10, 57);
			break;
		case 5:
			ItemIndex = GET_ITEM(10, 76);
			break;
		case 6:
			ItemIndex = GET_ITEM(10, 56);
			break;
		case 7:
			ItemIndex = GET_ITEM(10, 75);
			break;
		case 8:
			ItemIndex = GET_ITEM(10, 58);
			break;
		case 9:
			ItemIndex = GET_ITEM(10, 55);
			break;
		case 10:
			ItemIndex = GET_ITEM(10, 90);
			break;

		}

		gItemOptionRate.GetItemOption2(8,&ItemOption2);

		gItemOptionRate.GetItemOption6(((gItemManager.GetItemTwoHand(ItemIndex)==0)?6:7),&ItemSocketOption[0]);

		gItemOptionRate.MakeSocketOption(ItemIndex,ItemSocketOption[0],&ItemSocketOption[0]);

		GDCreateItemSend(lpObj->Index,0xFF,0,0,ItemIndex,0,0,1,ItemOption2,0,-1,0,0,0,0,ItemSocketOption,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[SocketGloveMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[SocketGloveMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::SocketBootMix(LPOBJ lpObj) // OK
{
	int CombinationNote = 0;
	int SocketType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 364))
		{
			CombinationNote++;
		}

		else if (lpObj->ChaosBox[n].m_Index >= GET_ITEM(11, 44) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(11, 53))
		{
			SocketType = lpObj->ChaosBox[n].m_Index - GET_ITEM(11, 43);
		}
	}

	if ( SocketType == 0 || CombinationNote == 0 || (SocketType == 1 && (CombinationNote != 1)) || (SocketType == 2 && (CombinationNote != 1)) || (SocketType == 3 && (CombinationNote != 1)) || (SocketType == 4 && (CombinationNote != 1)) || (SocketType == 5 && (CombinationNote != 1)) || (SocketType == 6 && (CombinationNote != 1)) || (SocketType == 7 && (CombinationNote != 1)) || (SocketType == 8 && (CombinationNote != 1)) || (SocketType == 9 && (CombinationNote != 1)) || (SocketType == 10 && (CombinationNote != 1)))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_SocketSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_SocketSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_SocketSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 75000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;
		BYTE ItemSocketOption[MAX_SOCKET_OPTION] = {0xFF,0xFF,0xFF,0xFF,0xFF};

		switch (SocketType)
		{
		case 1:
			ItemIndex = GET_ITEM(11, 90);
			break;
		case 2:
			ItemIndex = GET_ITEM(11, 74);
			break;
		case 3:
			ItemIndex = GET_ITEM(11, 54);
			break;
		case 4:
			ItemIndex = GET_ITEM(11, 57);
			break;
		case 5:
			ItemIndex = GET_ITEM(11, 76);
			break;
		case 6:
			ItemIndex = GET_ITEM(11, 56);
			break;
		case 7:
			ItemIndex = GET_ITEM(11, 75);
			break;
		case 8:
			ItemIndex = GET_ITEM(11, 58);
			break;
		case 9:
			ItemIndex = GET_ITEM(11, 55);
			break;
		case 10:
			ItemIndex = GET_ITEM(11, 90);
			break;

		}

		gItemOptionRate.GetItemOption2(8,&ItemOption2);

		gItemOptionRate.GetItemOption6(((gItemManager.GetItemTwoHand(ItemIndex)==0)?6:7),&ItemSocketOption[0]);

		gItemOptionRate.MakeSocketOption(ItemIndex,ItemSocketOption[0],&ItemSocketOption[0]);

		GDCreateItemSend(lpObj->Index,0xFF,0,0,ItemIndex,0,0,1,ItemOption2,0,-1,0,0,0,0,ItemSocketOption,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[SocketBootMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[SocketBootMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

//Socket Set Upgrade

void CChaosBox::SocketRFHelmMix(LPOBJ lpObj) // OK
{
	int CombinationNote = 0;
	int SocketType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 364))
		{
			CombinationNote++;
		}

		else if (lpObj->ChaosBox[n].m_Index >= GET_ITEM(7, 146) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(7, 147))
		{
			SocketType = lpObj->ChaosBox[n].m_Index - GET_ITEM(7, 145);
		}
	}

	if ( SocketType == 0 || CombinationNote == 0 || (SocketType == 1 && (CombinationNote != 1)) || (SocketType == 2 && (CombinationNote != 1)))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_SocketSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_SocketSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_SocketSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 75000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;
		BYTE ItemSocketOption[MAX_SOCKET_OPTION] = {0xFF,0xFF,0xFF,0xFF,0xFF};

		switch (SocketType)
		{
		case 1:
			ItemIndex = GET_ITEM(7, 77);
			break;
		case 2:
			ItemIndex = GET_ITEM(7, 77);
			break;
		}

		gItemOptionRate.GetItemOption2(8,&ItemOption2);

		gItemOptionRate.GetItemOption6(((gItemManager.GetItemTwoHand(ItemIndex)==0)?6:7),&ItemSocketOption[0]);

		gItemOptionRate.MakeSocketOption(ItemIndex,ItemSocketOption[0],&ItemSocketOption[0]);

		GDCreateItemSend(lpObj->Index,0xFF,0,0,ItemIndex,0,0,1,ItemOption2,0,-1,0,0,0,0,ItemSocketOption,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[SocketHelmMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[SocketHelmMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::SocketRFArmorMix(LPOBJ lpObj) // OK
{
	int CombinationNote = 0;
	int SocketType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 364))
		{
			CombinationNote++;
		}

		else if (lpObj->ChaosBox[n].m_Index >= GET_ITEM(8, 146) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(8, 147))
		{
			SocketType = lpObj->ChaosBox[n].m_Index - GET_ITEM(8, 145);
		}
	}

	if ( SocketType == 0 || CombinationNote == 0 || (SocketType == 1 && (CombinationNote != 1)) || (SocketType == 2 && (CombinationNote != 1)))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_SocketSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_SocketSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_SocketSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 75000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;
		BYTE ItemSocketOption[MAX_SOCKET_OPTION] = {0xFF,0xFF,0xFF,0xFF,0xFF};

		switch (SocketType)
		{
		case 1:
			ItemIndex = GET_ITEM(8, 77);
			break;
		case 2:
			ItemIndex = GET_ITEM(8, 77);
			break;
		}

		gItemOptionRate.GetItemOption2(8,&ItemOption2);

		gItemOptionRate.GetItemOption6(((gItemManager.GetItemTwoHand(ItemIndex)==0)?6:7),&ItemSocketOption[0]);

		gItemOptionRate.MakeSocketOption(ItemIndex,ItemSocketOption[0],&ItemSocketOption[0]);

		GDCreateItemSend(lpObj->Index,0xFF,0,0,ItemIndex,0,0,1,ItemOption2,0,-1,0,0,0,0,ItemSocketOption,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[SocketArmorMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[SocketArmorMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::SocketRFPantMix(LPOBJ lpObj) // OK
{
	int CombinationNote = 0;
	int SocketType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 364))
		{
			CombinationNote++;
		}

		else if (lpObj->ChaosBox[n].m_Index >= GET_ITEM(9, 146) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(9, 147))
		{
			SocketType = lpObj->ChaosBox[n].m_Index - GET_ITEM(9, 145);
		}
	}

	if ( SocketType == 0 || CombinationNote == 0 || (SocketType == 1 && (CombinationNote != 1)) || (SocketType == 2 && (CombinationNote != 1)))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_SocketSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_SocketSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_SocketSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 75000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;
		BYTE ItemSocketOption[MAX_SOCKET_OPTION] = {0xFF,0xFF,0xFF,0xFF,0xFF};

		switch (SocketType)
		{
		case 1:
			ItemIndex = GET_ITEM(9, 77);
			break;
		case 2:
			ItemIndex = GET_ITEM(9, 77);
			break;
		}

		gItemOptionRate.GetItemOption2(8,&ItemOption2);

		gItemOptionRate.GetItemOption6(((gItemManager.GetItemTwoHand(ItemIndex)==0)?6:7),&ItemSocketOption[0]);

		gItemOptionRate.MakeSocketOption(ItemIndex,ItemSocketOption[0],&ItemSocketOption[0]);

		GDCreateItemSend(lpObj->Index,0xFF,0,0,ItemIndex,0,0,1,ItemOption2,0,-1,0,0,0,0,ItemSocketOption,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[SocketPantMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[SocketPantMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::SocketRFBootMix(LPOBJ lpObj) // OK
{
	int CombinationNote = 0;
	int SocketType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 364))
		{
			CombinationNote++;
		}

		else if (lpObj->ChaosBox[n].m_Index >= GET_ITEM(11, 146) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(11, 147))
		{
			SocketType = lpObj->ChaosBox[n].m_Index - GET_ITEM(11, 145);
		}
	}

	if ( SocketType == 0 || CombinationNote == 0 || (SocketType == 1 && (CombinationNote != 1)) || (SocketType == 2 && (CombinationNote != 1)))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_SocketSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_SocketSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_SocketSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 75000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;
		BYTE ItemSocketOption[MAX_SOCKET_OPTION] = {0xFF,0xFF,0xFF,0xFF,0xFF};

		switch (SocketType)
		{
		case 1:
			ItemIndex = GET_ITEM(11, 77);
			break;
		case 2:
			ItemIndex = GET_ITEM(11, 77);
			break;
		}

		gItemOptionRate.GetItemOption2(8,&ItemOption2);

		gItemOptionRate.GetItemOption6(((gItemManager.GetItemTwoHand(ItemIndex)==0)?6:7),&ItemSocketOption[0]);

		gItemOptionRate.MakeSocketOption(ItemIndex,ItemSocketOption[0],&ItemSocketOption[0]);

		GDCreateItemSend(lpObj->Index,0xFF,0,0,ItemIndex,0,0,1,ItemOption2,0,-1,0,0,0,0,ItemSocketOption,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[SocketBootMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[SocketBootMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

//Conqueror Wings Mix
void CChaosBox::WingsOfConquerorMix(LPOBJ lpObj) // OK
{
	int ItemCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int LifeCount = 0;
	int ChaosCount = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,276))
		{
			ItemCount++;
		}

		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,30) && lpObj->ChaosBox[n].m_Level == 0)
		{
			BlessCount++;
		}

		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,31) && lpObj->ChaosBox[n].m_Level == 0)
		{
			SoulCount++;
		}

		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,136) && lpObj->ChaosBox[n].m_Level == 0)
		{
			LifeCount++;
		}

		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
		}
	}

	if(ItemCount != 1 || BlessCount != 1 || SoulCount != 1 || LifeCount != 1 || ChaosCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	lpObj->ChaosMoney = 10000000;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	lpObj->ChaosSuccessRate = gServerInfo.m_ConquerorWingMixRate[lpObj->AccountLevel];

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	if((rand()%100) < lpObj->ChaosSuccessRate)
	{
		
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;
		BYTE ItemOption3 = 0;
		BYTE ItemNewOption = 0;

		CRandomManager RandomManager;

		RandomManager.AddElement(GET_ITEM(12,204),1);

		RandomManager.GetRandomElement(&ItemIndex);

		gItemOptionRate.GetItemOption2(6,&ItemOption2);

		gItemOptionRate.GetItemOption3(6,&ItemOption3);

		gItemOptionRate.GetItemOption4(6,&ItemNewOption);

		gItemOptionRate.MakeNewOption(ItemIndex,ItemNewOption,&ItemNewOption);

		GDCreateItemSend(lpObj->Index,0xFF,0,0,ItemIndex,0,0,0,ItemOption2,ItemOption3,-1,(ItemNewOption+(16*(GetLargeRand()%3))),0,0,0,0,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[ConquerorWingMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[ConquerorWingMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
}

//New Refine Harmony
void CChaosBox::JewelOfHarmonyItemPurity10Mix(LPOBJ lpObj) // OK
{
	int GemStoneCount = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,139))
		{
			GemStoneCount++;
		}
	}

	if(GemStoneCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	lpObj->ChaosSuccessRate = gServerInfo.m_JewelOfHarmonyItemPurityMixRate[lpObj->AccountLevel];

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		GDCreateItemSend(lpObj->Index,0xFF,0,0,GET_ITEM(12,140),0,1,0,0,0,-1,0,0,0,0,0,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[JewelOfHarmonyItemPurity10Mix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[JewelOfHarmonyItemPurity10Mix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
}

void CChaosBox::JewelOfHarmonyItemPurity20Mix(LPOBJ lpObj) // OK
{
	int GemStoneCount = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,139) && lpObj->ChaosBox[n].m_Level == 1)
		{
			GemStoneCount++;
		}
	}

	if(GemStoneCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	lpObj->ChaosSuccessRate = gServerInfo.m_JewelOfHarmonyItemPurityMixRate[lpObj->AccountLevel];

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		GDCreateItemSend(lpObj->Index,0xFF,0,0,GET_ITEM(12,140),1,1,0,0,0,-1,0,0,0,0,0,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[JewelOfHarmonyItemPurity20Mix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[JewelOfHarmonyItemPurity20Mix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
}

void CChaosBox::JewelOfHarmonyItemPurity30Mix(LPOBJ lpObj) // OK
{
	int GemStoneCount = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,139) && lpObj->ChaosBox[n].m_Level == 2)
		{
			GemStoneCount++;
		}
	}

	if(GemStoneCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	lpObj->ChaosSuccessRate = gServerInfo.m_JewelOfHarmonyItemPurityMixRate[lpObj->AccountLevel];

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		GDCreateItemSend(lpObj->Index,0xFF,0,0,GET_ITEM(12,140),2,1,0,0,0,-1,0,0,0,0,0,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[JewelOfHarmonyItemPurity30Mix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[JewelOfHarmonyItemPurity30Mix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
}

//Mix Wings Level 1 + Talisman

void CChaosBox::Wing1DKMix(LPOBJ lpObj) // OK
{
	int ChaosCount = 0;
	int ChaosItem = 0;
	int ItemCount = 0;
	int ItemMoney = 0;
	int TalismanOfWingType = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
			lpObj->ChaosBox[n].OldValue();
			ItemMoney += lpObj->ChaosBox[n].m_OldBuyMoney;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,13) || lpObj->ChaosBox[n].m_Index == GET_ITEM(14,14))
		{
			ItemCount++;
			lpObj->ChaosBox[n].OldValue();
			ItemMoney += lpObj->ChaosBox[n].m_OldBuyMoney;
		}
		else if((lpObj->ChaosBox[n].m_Index == GET_ITEM(2,6) || lpObj->ChaosBox[n].m_Index == GET_ITEM(4,6) || lpObj->ChaosBox[n].m_Index == GET_ITEM(5,7)) && lpObj->ChaosBox[n].m_Level >= 4 && lpObj->ChaosBox[n].m_Option3 >= 1)
		{
			ChaosItem++;
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,83))
		{
			TalismanOfWingType++;
			lpObj->ChaosBox[n].OldValue();
			ItemMoney += lpObj->ChaosBox[n].m_OldBuyMoney;
		}
	}

	if(ChaosCount == 0 || ChaosItem == 0)
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	if(gServerInfo.m_Wing1MixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney/20000);
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_Wing1MixRate[lpObj->AccountLevel];
	}

	if(this->GetTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	lpObj->ChaosMoney = 100000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;
		BYTE ItemOption3 = 0;

		CRandomManager RandomManager;

		RandomManager.AddElement(GET_ITEM(12,2),1);

		RandomManager.GetRandomElement(&ItemIndex);

		gItemOptionRate.GetItemOption2(5,&ItemOption2);

		gItemOptionRate.GetItemOption3(5,&ItemOption3);

		GDCreateItemSend(lpObj->Index,0xFF,0,0,ItemIndex,0,0,0,ItemOption2,ItemOption3,-1,0,0,0,0,0,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[Wing1DKMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[Wing1DKMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
}

void CChaosBox::Wing1DWMix(LPOBJ lpObj) // OK
{
	int ChaosCount = 0;
	int ChaosItem = 0;
	int ItemCount = 0;
	int ItemMoney = 0;
	int TalismanOfWingType = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
			lpObj->ChaosBox[n].OldValue();
			ItemMoney += lpObj->ChaosBox[n].m_OldBuyMoney;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,13) || lpObj->ChaosBox[n].m_Index == GET_ITEM(14,14))
		{
			ItemCount++;
			lpObj->ChaosBox[n].OldValue();
			ItemMoney += lpObj->ChaosBox[n].m_OldBuyMoney;
		}
		else if((lpObj->ChaosBox[n].m_Index == GET_ITEM(2,6) || lpObj->ChaosBox[n].m_Index == GET_ITEM(4,6) || lpObj->ChaosBox[n].m_Index == GET_ITEM(5,7)) && lpObj->ChaosBox[n].m_Level >= 4 && lpObj->ChaosBox[n].m_Option3 >= 1)
		{
			ChaosItem++;
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,84))
		{
			TalismanOfWingType++;
			lpObj->ChaosBox[n].OldValue();
			ItemMoney += lpObj->ChaosBox[n].m_OldBuyMoney;
		}
	}

	if(ChaosCount == 0 || ChaosItem == 0)
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	if(gServerInfo.m_Wing1MixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney/20000);
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_Wing1MixRate[lpObj->AccountLevel];
	}

	if(this->GetTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	lpObj->ChaosMoney = 100000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;
		BYTE ItemOption3 = 0;

		CRandomManager RandomManager;

		RandomManager.AddElement(GET_ITEM(12,1),1);

		RandomManager.GetRandomElement(&ItemIndex);

		gItemOptionRate.GetItemOption2(5,&ItemOption2);

		gItemOptionRate.GetItemOption3(5,&ItemOption3);

		GDCreateItemSend(lpObj->Index,0xFF,0,0,ItemIndex,0,0,0,ItemOption2,ItemOption3,-1,0,0,0,0,0,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[Wing1DWMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[Wing1DWMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
}

void CChaosBox::Wing1FEMix(LPOBJ lpObj) // OK
{
	int ChaosCount = 0;
	int ChaosItem = 0;
	int ItemCount = 0;
	int ItemMoney = 0;
	int TalismanOfWingType = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
			lpObj->ChaosBox[n].OldValue();
			ItemMoney += lpObj->ChaosBox[n].m_OldBuyMoney;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,13) || lpObj->ChaosBox[n].m_Index == GET_ITEM(14,14))
		{
			ItemCount++;
			lpObj->ChaosBox[n].OldValue();
			ItemMoney += lpObj->ChaosBox[n].m_OldBuyMoney;
		}
		else if((lpObj->ChaosBox[n].m_Index == GET_ITEM(2,6) || lpObj->ChaosBox[n].m_Index == GET_ITEM(4,6) || lpObj->ChaosBox[n].m_Index == GET_ITEM(5,7)) && lpObj->ChaosBox[n].m_Level >= 4 && lpObj->ChaosBox[n].m_Option3 >= 1)
		{
			ChaosItem++;
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,85))
		{
			TalismanOfWingType++;
			lpObj->ChaosBox[n].OldValue();
			ItemMoney += lpObj->ChaosBox[n].m_OldBuyMoney;
		}
	}

	if(ChaosCount == 0 || ChaosItem == 0)
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	if(gServerInfo.m_Wing1MixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney/20000);
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_Wing1MixRate[lpObj->AccountLevel];
	}

	if(this->GetTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	lpObj->ChaosMoney = 100000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;
		BYTE ItemOption3 = 0;

		CRandomManager RandomManager;

		RandomManager.AddElement(GET_ITEM(12,0),1);

		RandomManager.GetRandomElement(&ItemIndex);

		gItemOptionRate.GetItemOption2(5,&ItemOption2);

		gItemOptionRate.GetItemOption3(5,&ItemOption3);

		GDCreateItemSend(lpObj->Index,0xFF,0,0,ItemIndex,0,0,0,ItemOption2,ItemOption3,-1,0,0,0,0,0,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[Wing1FEMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[Wing1FEMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
}

void CChaosBox::Wing1SUMix(LPOBJ lpObj) // OK
{
	int ChaosCount = 0;
	int ChaosItem = 0;
	int ItemCount = 0;
	int ItemMoney = 0;
	int TalismanOfWingType = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
			lpObj->ChaosBox[n].OldValue();
			ItemMoney += lpObj->ChaosBox[n].m_OldBuyMoney;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,13) || lpObj->ChaosBox[n].m_Index == GET_ITEM(14,14))
		{
			ItemCount++;
			lpObj->ChaosBox[n].OldValue();
			ItemMoney += lpObj->ChaosBox[n].m_OldBuyMoney;
		}
		else if((lpObj->ChaosBox[n].m_Index == GET_ITEM(2,6) || lpObj->ChaosBox[n].m_Index == GET_ITEM(4,6) || lpObj->ChaosBox[n].m_Index == GET_ITEM(5,7)) && lpObj->ChaosBox[n].m_Level >= 4 && lpObj->ChaosBox[n].m_Option3 >= 1)
		{
			ChaosItem++;
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,86))
		{
			TalismanOfWingType++;
			lpObj->ChaosBox[n].OldValue();
			ItemMoney += lpObj->ChaosBox[n].m_OldBuyMoney;
		}
	}

	if(ChaosCount == 0 || ChaosItem == 0)
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	if(gServerInfo.m_Wing1MixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney/20000);
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_Wing1MixRate[lpObj->AccountLevel];
	}

	if(this->GetTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	lpObj->ChaosMoney = 100000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;
		BYTE ItemOption3 = 0;

		CRandomManager RandomManager;

		RandomManager.AddElement(GET_ITEM(12,41),1);

		RandomManager.GetRandomElement(&ItemIndex);

		gItemOptionRate.GetItemOption2(5,&ItemOption2);

		gItemOptionRate.GetItemOption3(5,&ItemOption3);

		GDCreateItemSend(lpObj->Index,0xFF,0,0,ItemIndex,0,0,0,ItemOption2,ItemOption3,-1,0,0,0,0,0,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[Wing1DSUix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[Wing1SUMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
}

//Mix Wings Level 2 + Talisman

void CChaosBox::Wing2DLMix(LPOBJ lpObj) // OK
{
	int ChaosCount = 0;
	int ChaosItem = 0;
	int ItemCount = 0;
	int ItemMoney = 0;
	int TalismanOfWingType = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
			lpObj->ChaosBox[n].OldValue();
			ItemMoney += lpObj->ChaosBox[n].m_OldBuyMoney;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,22))
		{
			ItemCount++;
			lpObj->ChaosBox[n].OldValue();
			ItemMoney += lpObj->ChaosBox[n].m_OldBuyMoney;
		}
		else if((lpObj->ChaosBox[n].m_Index >= GET_ITEM(12,0) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(12,2)) || lpObj->ChaosBox[n].m_Index == GET_ITEM(12,41))
		{
			ChaosItem++;
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,87))
		{
			TalismanOfWingType++;
			lpObj->ChaosBox[n].OldValue();
			ItemMoney += lpObj->ChaosBox[n].m_OldBuyMoney;
		}
	}

	if(ChaosCount == 0 || ChaosItem == 0)
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	if(gServerInfo.m_Wing2MixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney/20000);
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_Wing2MixRate[lpObj->AccountLevel];
	}

	if(this->GetTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	lpObj->ChaosMoney = 5000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
			WORD ItemIndex = 0;
			BYTE ItemOption2 = 0;
			BYTE ItemOption3 = 0;
			BYTE ItemNewOption = 0;

			CRandomManager RandomManager;

			RandomManager.AddElement(GET_ITEM(13,30),1);

			RandomManager.GetRandomElement(&ItemIndex);

			gItemOptionRate.GetItemOption2(4,&ItemOption2);

			gItemOptionRate.GetItemOption3(4,&ItemOption3);

			gItemOptionRate.GetItemOption4(4,&ItemNewOption);

			gItemOptionRate.MakeNewOption(ItemIndex,ItemNewOption,&ItemNewOption);

			GDCreateItemSend(lpObj->Index,0xFF,0,0,ItemIndex,0,0,0,ItemOption2,ItemOption3,-1,(ItemNewOption+(32*(GetLargeRand()%2))),0,0,0,0,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[Wing2DLMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[Wing2DLMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
}

void CChaosBox::Wing2BKMix(LPOBJ lpObj) // OK
{
	int ChaosCount = 0;
	int ChaosItem = 0;
	int ItemCount = 0;
	int ItemMoney = 0;
	int TalismanOfWingType = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
			lpObj->ChaosBox[n].OldValue();
			ItemMoney += lpObj->ChaosBox[n].m_OldBuyMoney;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,22))
		{
			ItemCount++;
			lpObj->ChaosBox[n].OldValue();
			ItemMoney += lpObj->ChaosBox[n].m_OldBuyMoney;
		}
		else if((lpObj->ChaosBox[n].m_Index >= GET_ITEM(12,0) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(12,2)) || lpObj->ChaosBox[n].m_Index == GET_ITEM(12,41))
		{
			ChaosItem++;
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,88))
		{
			TalismanOfWingType++;
			lpObj->ChaosBox[n].OldValue();
			ItemMoney += lpObj->ChaosBox[n].m_OldBuyMoney;
		}
	}

	if(ChaosCount == 0 || ChaosItem == 0)
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	if(gServerInfo.m_Wing2MixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney/20000);
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_Wing2MixRate[lpObj->AccountLevel];
	}

	if(this->GetTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	lpObj->ChaosMoney = 5000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
			WORD ItemIndex = 0;
			BYTE ItemOption2 = 0;
			BYTE ItemOption3 = 0;
			BYTE ItemNewOption = 0;

			CRandomManager RandomManager;

			RandomManager.AddElement(GET_ITEM(12,5),1);

			RandomManager.GetRandomElement(&ItemIndex);

			gItemOptionRate.GetItemOption2(4,&ItemOption2);

			gItemOptionRate.GetItemOption3(4,&ItemOption3);

			gItemOptionRate.GetItemOption4(4,&ItemNewOption);

			gItemOptionRate.MakeNewOption(ItemIndex,ItemNewOption,&ItemNewOption);

			GDCreateItemSend(lpObj->Index,0xFF,0,0,ItemIndex,0,0,0,ItemOption2,ItemOption3,-1,(ItemNewOption+(32*(GetLargeRand()%2))),0,0,0,0,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[Wing2BKMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[Wing2BKMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
}

void CChaosBox::Wing2SMMix(LPOBJ lpObj) // OK
{
	int ChaosCount = 0;
	int ChaosItem = 0;
	int ItemCount = 0;
	int ItemMoney = 0;
	int TalismanOfWingType = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
			lpObj->ChaosBox[n].OldValue();
			ItemMoney += lpObj->ChaosBox[n].m_OldBuyMoney;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,22))
		{
			ItemCount++;
			lpObj->ChaosBox[n].OldValue();
			ItemMoney += lpObj->ChaosBox[n].m_OldBuyMoney;
		}
		else if((lpObj->ChaosBox[n].m_Index >= GET_ITEM(12,0) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(12,2)) || lpObj->ChaosBox[n].m_Index == GET_ITEM(12,41))
		{
			ChaosItem++;
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,89))
		{
			TalismanOfWingType++;
			lpObj->ChaosBox[n].OldValue();
			ItemMoney += lpObj->ChaosBox[n].m_OldBuyMoney;
		}
	}

	if(ChaosCount == 0 || ChaosItem == 0)
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	if(gServerInfo.m_Wing2MixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney/20000);
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_Wing2MixRate[lpObj->AccountLevel];
	}

	if(this->GetTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	lpObj->ChaosMoney = 5000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
			WORD ItemIndex = 0;
			BYTE ItemOption2 = 0;
			BYTE ItemOption3 = 0;
			BYTE ItemNewOption = 0;

			CRandomManager RandomManager;

			RandomManager.AddElement(GET_ITEM(12,4),1);

			RandomManager.GetRandomElement(&ItemIndex);

			gItemOptionRate.GetItemOption2(4,&ItemOption2);

			gItemOptionRate.GetItemOption3(4,&ItemOption3);

			gItemOptionRate.GetItemOption4(4,&ItemNewOption);

			gItemOptionRate.MakeNewOption(ItemIndex,ItemNewOption,&ItemNewOption);

			GDCreateItemSend(lpObj->Index,0xFF,0,0,ItemIndex,0,0,0,ItemOption2,ItemOption3,-1,(ItemNewOption+(32*(GetLargeRand()%2))),0,0,0,0,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[Wing2SMMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[Wing2SMMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
}

void CChaosBox::Wing2MEMix(LPOBJ lpObj) // OK
{
	int ChaosCount = 0;
	int ChaosItem = 0;
	int ItemCount = 0;
	int ItemMoney = 0;
	int TalismanOfWingType = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
			lpObj->ChaosBox[n].OldValue();
			ItemMoney += lpObj->ChaosBox[n].m_OldBuyMoney;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,22))
		{
			ItemCount++;
			lpObj->ChaosBox[n].OldValue();
			ItemMoney += lpObj->ChaosBox[n].m_OldBuyMoney;
		}
		else if((lpObj->ChaosBox[n].m_Index >= GET_ITEM(12,0) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(12,2)) || lpObj->ChaosBox[n].m_Index == GET_ITEM(12,41))
		{
			ChaosItem++;
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,90))
		{
			TalismanOfWingType++;
			lpObj->ChaosBox[n].OldValue();
			ItemMoney += lpObj->ChaosBox[n].m_OldBuyMoney;
		}
	}

	if(ChaosCount == 0 || ChaosItem == 0)
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	if(gServerInfo.m_Wing2MixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney/20000);
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_Wing2MixRate[lpObj->AccountLevel];
	}

	if(this->GetTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	lpObj->ChaosMoney = 5000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
			WORD ItemIndex = 0;
			BYTE ItemOption2 = 0;
			BYTE ItemOption3 = 0;
			BYTE ItemNewOption = 0;

			CRandomManager RandomManager;

			RandomManager.AddElement(GET_ITEM(12,3),1);

			RandomManager.GetRandomElement(&ItemIndex);

			gItemOptionRate.GetItemOption2(4,&ItemOption2);

			gItemOptionRate.GetItemOption3(4,&ItemOption3);

			gItemOptionRate.GetItemOption4(4,&ItemNewOption);

			gItemOptionRate.MakeNewOption(ItemIndex,ItemNewOption,&ItemNewOption);

			GDCreateItemSend(lpObj->Index,0xFF,0,0,ItemIndex,0,0,0,ItemOption2,ItemOption3,-1,(ItemNewOption+(32*(GetLargeRand()%2))),0,0,0,0,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[Wing2MEMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[Wing2MEMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
}

void CChaosBox::Wing2BSMix(LPOBJ lpObj) // OK
{
	int ChaosCount = 0;
	int ChaosItem = 0;
	int ItemCount = 0;
	int ItemMoney = 0;
	int TalismanOfWingType = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
			lpObj->ChaosBox[n].OldValue();
			ItemMoney += lpObj->ChaosBox[n].m_OldBuyMoney;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,22))
		{
			ItemCount++;
			lpObj->ChaosBox[n].OldValue();
			ItemMoney += lpObj->ChaosBox[n].m_OldBuyMoney;
		}
		else if((lpObj->ChaosBox[n].m_Index >= GET_ITEM(12,0) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(12,2)) || lpObj->ChaosBox[n].m_Index == GET_ITEM(12,41))
		{
			ChaosItem++;
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,91))
		{
			TalismanOfWingType++;
			lpObj->ChaosBox[n].OldValue();
			ItemMoney += lpObj->ChaosBox[n].m_OldBuyMoney;
		}
	}

	if(ChaosCount == 0 || ChaosItem == 0)
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	if(gServerInfo.m_Wing2MixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney/20000);
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_Wing2MixRate[lpObj->AccountLevel];
	}

	if(this->GetTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	lpObj->ChaosMoney = 5000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
			WORD ItemIndex = 0;
			BYTE ItemOption2 = 0;
			BYTE ItemOption3 = 0;
			BYTE ItemNewOption = 0;

			CRandomManager RandomManager;

			RandomManager.AddElement(GET_ITEM(12,42),1);

			RandomManager.GetRandomElement(&ItemIndex);

			gItemOptionRate.GetItemOption2(4,&ItemOption2);

			gItemOptionRate.GetItemOption3(4,&ItemOption3);

			gItemOptionRate.GetItemOption4(4,&ItemNewOption);

			gItemOptionRate.MakeNewOption(ItemIndex,ItemNewOption,&ItemNewOption);

			GDCreateItemSend(lpObj->Index,0xFF,0,0,ItemIndex,0,0,0,ItemOption2,ItemOption3,-1,(ItemNewOption+(32*(GetLargeRand()%2))),0,0,0,0,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[Wing2BSMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[Wing2BSMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
}

void CChaosBox::Wing2MGMix(LPOBJ lpObj) // OK
{
	int ChaosCount = 0;
	int ChaosItem = 0;
	int ItemCount = 0;
	int ItemMoney = 0;
	int TalismanOfWingType = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
			lpObj->ChaosBox[n].OldValue();
			ItemMoney += lpObj->ChaosBox[n].m_OldBuyMoney;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,22))
		{
			ItemCount++;
			lpObj->ChaosBox[n].OldValue();
			ItemMoney += lpObj->ChaosBox[n].m_OldBuyMoney;
		}
		else if((lpObj->ChaosBox[n].m_Index >= GET_ITEM(12,0) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(12,2)) || lpObj->ChaosBox[n].m_Index == GET_ITEM(12,41))
		{
			ChaosItem++;
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,92))
		{
			TalismanOfWingType++;
			lpObj->ChaosBox[n].OldValue();
			ItemMoney += lpObj->ChaosBox[n].m_OldBuyMoney;
		}
	}

	if(ChaosCount == 0 || ChaosItem == 0)
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	if(gServerInfo.m_Wing2MixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney/20000);
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_Wing2MixRate[lpObj->AccountLevel];
	}

	if(this->GetTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	lpObj->ChaosMoney = 5000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
			WORD ItemIndex = 0;
			BYTE ItemOption2 = 0;
			BYTE ItemOption3 = 0;
			BYTE ItemNewOption = 0;

			CRandomManager RandomManager;

			RandomManager.AddElement(GET_ITEM(12,6),1);

			RandomManager.GetRandomElement(&ItemIndex);

			gItemOptionRate.GetItemOption2(4,&ItemOption2);

			gItemOptionRate.GetItemOption3(4,&ItemOption3);

			gItemOptionRate.GetItemOption4(4,&ItemNewOption);

			gItemOptionRate.MakeNewOption(ItemIndex,ItemNewOption,&ItemNewOption);

			GDCreateItemSend(lpObj->Index,0xFF,0,0,ItemIndex,0,0,0,ItemOption2,ItemOption3,-1,(ItemNewOption+(32*(GetLargeRand()%2))),0,0,0,0,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[Wing2MGMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[Wing2MGMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
}

void CChaosBox::Wing2RFMix(LPOBJ lpObj) // OK
{
	int ChaosCount = 0;
	int ChaosItem = 0;
	int ItemCount = 0;
	int ItemMoney = 0;
	int TalismanOfWingType = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
			lpObj->ChaosBox[n].OldValue();
			ItemMoney += lpObj->ChaosBox[n].m_OldBuyMoney;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,22))
		{
			ItemCount++;
			lpObj->ChaosBox[n].OldValue();
			ItemMoney += lpObj->ChaosBox[n].m_OldBuyMoney;
		}
		else if((lpObj->ChaosBox[n].m_Index >= GET_ITEM(12,0) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(12,2)) || lpObj->ChaosBox[n].m_Index == GET_ITEM(12,41))
		{
			ChaosItem++;
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,145))
		{
			TalismanOfWingType++;
			lpObj->ChaosBox[n].OldValue();
			ItemMoney += lpObj->ChaosBox[n].m_OldBuyMoney;
		}
	}

	if(ChaosCount == 0 || ChaosItem == 0)
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	if(gServerInfo.m_Wing2MixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney/20000);
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_Wing2MixRate[lpObj->AccountLevel];
	}

	if(this->GetTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	lpObj->ChaosMoney = 5000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
			WORD ItemIndex = 0;
			BYTE ItemOption2 = 0;
			BYTE ItemOption3 = 0;
			BYTE ItemNewOption = 0;

			CRandomManager RandomManager;

			RandomManager.AddElement(GET_ITEM(12,49),1);

			RandomManager.GetRandomElement(&ItemIndex);

			gItemOptionRate.GetItemOption2(4,&ItemOption2);

			gItemOptionRate.GetItemOption3(4,&ItemOption3);

			gItemOptionRate.GetItemOption4(4,&ItemNewOption);

			gItemOptionRate.MakeNewOption(ItemIndex,ItemNewOption,&ItemNewOption);

			GDCreateItemSend(lpObj->Index,0xFF,0,0,ItemIndex,0,0,0,ItemOption2,ItemOption3,-1,(ItemNewOption+(32*(GetLargeRand()%2))),0,0,0,0,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[Wing2RFMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[Wing2RFMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
}

//Mix Wings Level 3 + Talisman
void CChaosBox::Wing3LEMix(LPOBJ lpObj) // OK
{
	int ChaosCount = 0;
	int CreationCount = 0;
	int SoulPack10 = 0;
	int BlessPack10 = 0;
	int FlameCount = 0;
	int FeatherCount = 0;
	int ItemCount = 0;
	int ItemMoney = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,22))
		{
			CreationCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,31) && lpObj->ChaosBox[n].m_Level == 0)
		{
			SoulPack10++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,30) && lpObj->ChaosBox[n].m_Level == 0)
		{
			BlessPack10++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,52))
		{
			FlameCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,53))
		{
			FeatherCount++;
		}
		else if(lpObj->ChaosBox[n].IsExcItem() != 0 && lpObj->ChaosBox[n].m_Level >= 9 && lpObj->ChaosBox[n].m_Option3 >= 1)
		{
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
	}

	if(ChaosCount != 1 || CreationCount != 1 || SoulPack10 != 1 || BlessPack10 != 1 || FlameCount != 1 || FeatherCount != 1 || ItemCount == 0)
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	if(gServerInfo.m_Wing3MixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney/3000000)+1;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_Wing3MixRate[lpObj->AccountLevel];
	}

	if(this->GetTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	if(gServerInfo.m_Wing3MixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40)?40:lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 7500000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;
		BYTE ItemOption3 = 0;
		BYTE ItemNewOption = 0;

		CRandomManager RandomManager;

		RandomManager.AddElement(GET_ITEM(12,40),1);

		RandomManager.GetRandomElement(&ItemIndex);

		gItemOptionRate.GetItemOption2(6,&ItemOption2);

		gItemOptionRate.GetItemOption3(6,&ItemOption3);

		gItemOptionRate.GetItemOption4(6,&ItemNewOption);

		gItemOptionRate.MakeNewOption(ItemIndex,ItemNewOption,&ItemNewOption);

		GDCreateItemSend(lpObj->Index,0xFF,0,0,ItemIndex,0,0,0,ItemOption2,ItemOption3,-1,(ItemNewOption+(16*(GetLargeRand()%3))),0,0,0,0,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[Wing3LEMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[Wing3LEMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
}

void CChaosBox::Wing3BMMix(LPOBJ lpObj) // OK
{
	int ChaosCount = 0;
	int CreationCount = 0;
	int SoulPack10 = 0;
	int BlessPack10 = 0;
	int FlameCount = 0;
	int FeatherCount = 0;
	int ItemCount = 0;
	int ItemMoney = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,22))
		{
			CreationCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,31) && lpObj->ChaosBox[n].m_Level == 0)
		{
			SoulPack10++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,30) && lpObj->ChaosBox[n].m_Level == 0)
		{
			BlessPack10++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,52))
		{
			FlameCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,53))
		{
			FeatherCount++;
		}
		else if(lpObj->ChaosBox[n].IsExcItem() != 0 && lpObj->ChaosBox[n].m_Level >= 9 && lpObj->ChaosBox[n].m_Option3 >= 1)
		{
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
	}

	if(ChaosCount != 1 || CreationCount != 1 || SoulPack10 != 1 || BlessPack10 != 1 || FlameCount != 1 || FeatherCount != 1 || ItemCount == 0)
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	if(gServerInfo.m_Wing3MixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney/3000000)+1;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_Wing3MixRate[lpObj->AccountLevel];
	}

	if(this->GetTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	if(gServerInfo.m_Wing3MixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40)?40:lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 7500000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;
		BYTE ItemOption3 = 0;
		BYTE ItemNewOption = 0;

		CRandomManager RandomManager;

		RandomManager.AddElement(GET_ITEM(12,36),1);

		RandomManager.GetRandomElement(&ItemIndex);

		gItemOptionRate.GetItemOption2(6,&ItemOption2);

		gItemOptionRate.GetItemOption3(6,&ItemOption3);

		gItemOptionRate.GetItemOption4(6,&ItemNewOption);

		gItemOptionRate.MakeNewOption(ItemIndex,ItemNewOption,&ItemNewOption);

		GDCreateItemSend(lpObj->Index,0xFF,0,0,ItemIndex,0,0,0,ItemOption2,ItemOption3,-1,(ItemNewOption+(16*(GetLargeRand()%3))),0,0,0,0,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[Wing3BMMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[Wing3BMMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
}

void CChaosBox::Wing3GMMix(LPOBJ lpObj) // OK
{
	int ChaosCount = 0;
	int CreationCount = 0;
	int SoulPack10 = 0;
	int BlessPack10 = 0;
	int FlameCount = 0;
	int FeatherCount = 0;
	int ItemCount = 0;
	int ItemMoney = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,22))
		{
			CreationCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,31) && lpObj->ChaosBox[n].m_Level == 0)
		{
			SoulPack10++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,30) && lpObj->ChaosBox[n].m_Level == 0)
		{
			BlessPack10++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,52))
		{
			FlameCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,53))
		{
			FeatherCount++;
		}
		else if(lpObj->ChaosBox[n].IsExcItem() != 0 && lpObj->ChaosBox[n].m_Level >= 9 && lpObj->ChaosBox[n].m_Option3 >= 1)
		{
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
	}

	if(ChaosCount != 1 || CreationCount != 1 || SoulPack10 != 1 || BlessPack10 != 1 || FlameCount != 1 || FeatherCount != 1 || ItemCount == 0)
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	if(gServerInfo.m_Wing3MixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney/3000000)+1;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_Wing3MixRate[lpObj->AccountLevel];
	}

	if(this->GetTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	if(gServerInfo.m_Wing3MixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40)?40:lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 7500000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;
		BYTE ItemOption3 = 0;
		BYTE ItemNewOption = 0;

		CRandomManager RandomManager;

		RandomManager.AddElement(GET_ITEM(12,37),1);

		RandomManager.GetRandomElement(&ItemIndex);

		gItemOptionRate.GetItemOption2(6,&ItemOption2);

		gItemOptionRate.GetItemOption3(6,&ItemOption3);

		gItemOptionRate.GetItemOption4(6,&ItemNewOption);

		gItemOptionRate.MakeNewOption(ItemIndex,ItemNewOption,&ItemNewOption);

		GDCreateItemSend(lpObj->Index,0xFF,0,0,ItemIndex,0,0,0,ItemOption2,ItemOption3,-1,(ItemNewOption+(16*(GetLargeRand()%3))),0,0,0,0,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[Wing3GMMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[Wing3GMMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
}

void CChaosBox::Wing3HEMix(LPOBJ lpObj) // OK
{
	int ChaosCount = 0;
	int CreationCount = 0;
	int SoulPack10 = 0;
	int BlessPack10 = 0;
	int FlameCount = 0;
	int FeatherCount = 0;
	int ItemCount = 0;
	int ItemMoney = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,22))
		{
			CreationCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,31) && lpObj->ChaosBox[n].m_Level == 0)
		{
			SoulPack10++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,30) && lpObj->ChaosBox[n].m_Level == 0)
		{
			BlessPack10++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,52))
		{
			FlameCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,53))
		{
			FeatherCount++;
		}
		else if(lpObj->ChaosBox[n].IsExcItem() != 0 && lpObj->ChaosBox[n].m_Level >= 9 && lpObj->ChaosBox[n].m_Option3 >= 1)
		{
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
	}

	if(ChaosCount != 1 || CreationCount != 1 || SoulPack10 != 1 || BlessPack10 != 1 || FlameCount != 1 || FeatherCount != 1 || ItemCount == 0)
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	if(gServerInfo.m_Wing3MixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney/3000000)+1;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_Wing3MixRate[lpObj->AccountLevel];
	}

	if(this->GetTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	if(gServerInfo.m_Wing3MixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40)?40:lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 7500000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;
		BYTE ItemOption3 = 0;
		BYTE ItemNewOption = 0;

		CRandomManager RandomManager;

		RandomManager.AddElement(GET_ITEM(12,38),1);

		RandomManager.GetRandomElement(&ItemIndex);

		gItemOptionRate.GetItemOption2(6,&ItemOption2);

		gItemOptionRate.GetItemOption3(6,&ItemOption3);

		gItemOptionRate.GetItemOption4(6,&ItemNewOption);

		gItemOptionRate.MakeNewOption(ItemIndex,ItemNewOption,&ItemNewOption);

		GDCreateItemSend(lpObj->Index,0xFF,0,0,ItemIndex,0,0,0,ItemOption2,ItemOption3,-1,(ItemNewOption+(16*(GetLargeRand()%3))),0,0,0,0,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[Wing3HEMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[Wing3HEMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
}

void CChaosBox::Wing3DMMix(LPOBJ lpObj) // OK
{
	int ChaosCount = 0;
	int CreationCount = 0;
	int SoulPack10 = 0;
	int BlessPack10 = 0;
	int FlameCount = 0;
	int FeatherCount = 0;
	int ItemCount = 0;
	int ItemMoney = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,22))
		{
			CreationCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,31) && lpObj->ChaosBox[n].m_Level == 0)
		{
			SoulPack10++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,30) && lpObj->ChaosBox[n].m_Level == 0)
		{
			BlessPack10++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,52))
		{
			FlameCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,53))
		{
			FeatherCount++;
		}
		else if(lpObj->ChaosBox[n].IsExcItem() != 0 && lpObj->ChaosBox[n].m_Level >= 9 && lpObj->ChaosBox[n].m_Option3 >= 1)
		{
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
	}

	if(ChaosCount != 1 || CreationCount != 1 || SoulPack10 != 1 || BlessPack10 != 1 || FlameCount != 1 || FeatherCount != 1 || ItemCount == 0)
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	if(gServerInfo.m_Wing3MixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney/3000000)+1;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_Wing3MixRate[lpObj->AccountLevel];
	}

	if(this->GetTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	if(gServerInfo.m_Wing3MixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40)?40:lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 7500000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;
		BYTE ItemOption3 = 0;
		BYTE ItemNewOption = 0;

		CRandomManager RandomManager;

		RandomManager.AddElement(GET_ITEM(12,39),1);

		RandomManager.GetRandomElement(&ItemIndex);

		gItemOptionRate.GetItemOption2(6,&ItemOption2);

		gItemOptionRate.GetItemOption3(6,&ItemOption3);

		gItemOptionRate.GetItemOption4(6,&ItemNewOption);

		gItemOptionRate.MakeNewOption(ItemIndex,ItemNewOption,&ItemNewOption);

		GDCreateItemSend(lpObj->Index,0xFF,0,0,ItemIndex,0,0,0,ItemOption2,ItemOption3,-1,(ItemNewOption+(16*(GetLargeRand()%3))),0,0,0,0,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[Wing3DMMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[Wing3DMMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
}

void CChaosBox::Wing3SUMix(LPOBJ lpObj) // OK
{
	int ChaosCount = 0;
	int CreationCount = 0;
	int SoulPack10 = 0;
	int BlessPack10 = 0;
	int FlameCount = 0;
	int FeatherCount = 0;
	int ItemCount = 0;
	int ItemMoney = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,22))
		{
			CreationCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,31) && lpObj->ChaosBox[n].m_Level == 0)
		{
			SoulPack10++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,30) && lpObj->ChaosBox[n].m_Level == 0)
		{
			BlessPack10++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,52))
		{
			FlameCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,53))
		{
			FeatherCount++;
		}
		else if(lpObj->ChaosBox[n].IsExcItem() != 0 && lpObj->ChaosBox[n].m_Level >= 9 && lpObj->ChaosBox[n].m_Option3 >= 1)
		{
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
	}

	if(ChaosCount != 1 || CreationCount != 1 || SoulPack10 != 1 || BlessPack10 != 1 || FlameCount != 1 || FeatherCount != 1 || ItemCount == 0)
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	if(gServerInfo.m_Wing3MixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney/3000000)+1;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_Wing3MixRate[lpObj->AccountLevel];
	}

	if(this->GetTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	if(gServerInfo.m_Wing3MixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40)?40:lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 7500000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;
		BYTE ItemOption3 = 0;
		BYTE ItemNewOption = 0;

		CRandomManager RandomManager;

		RandomManager.AddElement(GET_ITEM(12,43),1);

		RandomManager.GetRandomElement(&ItemIndex);

		gItemOptionRate.GetItemOption2(6,&ItemOption2);

		gItemOptionRate.GetItemOption3(6,&ItemOption3);

		gItemOptionRate.GetItemOption4(6,&ItemNewOption);

		gItemOptionRate.MakeNewOption(ItemIndex,ItemNewOption,&ItemNewOption);

		GDCreateItemSend(lpObj->Index,0xFF,0,0,ItemIndex,0,0,0,ItemOption2,ItemOption3,-1,(ItemNewOption+(16*(GetLargeRand()%3))),0,0,0,0,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[Wing3SUMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[Wing3SUMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
}

void CChaosBox::Wing3FMMix(LPOBJ lpObj) // OK
{
	int ChaosCount = 0;
	int CreationCount = 0;
	int SoulPack10 = 0;
	int BlessPack10 = 0;
	int FlameCount = 0;
	int FeatherCount = 0;
	int ItemCount = 0;
	int ItemMoney = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,22))
		{
			CreationCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,31) && lpObj->ChaosBox[n].m_Level == 0)
		{
			SoulPack10++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,30) && lpObj->ChaosBox[n].m_Level == 0)
		{
			BlessPack10++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,52))
		{
			FlameCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,53))
		{
			FeatherCount++;
		}
		else if(lpObj->ChaosBox[n].IsExcItem() != 0 && lpObj->ChaosBox[n].m_Level >= 9 && lpObj->ChaosBox[n].m_Option3 >= 1)
		{
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
	}

	if(ChaosCount != 1 || CreationCount != 1 || SoulPack10 != 1 || BlessPack10 != 1 || FlameCount != 1 || FeatherCount != 1 || ItemCount == 0)
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	if(gServerInfo.m_Wing3MixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney/3000000)+1;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_Wing3MixRate[lpObj->AccountLevel];
	}

	if(this->GetTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	if(gServerInfo.m_Wing3MixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40)?40:lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 7500000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;
		BYTE ItemOption3 = 0;
		BYTE ItemNewOption = 0;

		CRandomManager RandomManager;

		RandomManager.AddElement(GET_ITEM(12,50),1);

		RandomManager.GetRandomElement(&ItemIndex);

		gItemOptionRate.GetItemOption2(6,&ItemOption2);

		gItemOptionRate.GetItemOption3(6,&ItemOption3);

		gItemOptionRate.GetItemOption4(6,&ItemNewOption);

		gItemOptionRate.MakeNewOption(ItemIndex,ItemNewOption,&ItemNewOption);

		GDCreateItemSend(lpObj->Index,0xFF,0,0,ItemIndex,0,0,0,ItemOption2,ItemOption3,-1,(ItemNewOption+(16*(GetLargeRand()%3))),0,0,0,0,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[Wing3FMMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[Wing3FMMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
}

//Classic Pet Mix
void CChaosBox::SpiritGuardianMix(LPOBJ lpObj) // OK
{
	int ChaosCount = 0;
	int UniriaCount = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,0))
		{
			UniriaCount++;
		}
	}

	if(ChaosCount != 1 || UniriaCount != 10)
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	lpObj->ChaosSuccessRate = gServerInfo.m_SpiritGuardianMixRate[lpObj->AccountLevel];

	if(this->GetTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	lpObj->ChaosMoney = 50000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = GET_ITEM(13,65);
		
		GDCreateItemSend(lpObj->Index,0xFF,0,0,ItemIndex,0,255,0,0,0,-1,0,0,0,0,0,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[SpiritGuardianMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[SpiritGuardianMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
}

void CChaosBox::DemonMix(LPOBJ lpObj) // OK
{
	int ChaosCount = 0;
	int UniriaCount = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,1) && lpObj->ChaosBox[n].m_Durability == 255)
		{
			UniriaCount++;
		}
	}

	if(ChaosCount != 1 || UniriaCount != 10)
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	lpObj->ChaosSuccessRate = gServerInfo.m_DemonMixRate[lpObj->AccountLevel];

	if(this->GetTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	lpObj->ChaosMoney = 50000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = GET_ITEM(13,64);

		GDCreateItemSend(lpObj->Index,0xFF,0,0,ItemIndex,0,255,0,0,0,-1,0,0,0,0,0,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[DemonMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[DemonMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
}

//Gold Fenrir Mix
void CChaosBox::GoldFenrirMix(LPOBJ lpObj) // OK
{
	int HornOfFenrirCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int LifeCount = 0;
	int CreationCount = 0;
	int ChaosCount = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,37))
		{
			HornOfFenrirCount++;
		}

		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,30) && lpObj->ChaosBox[n].m_Level == 2)
		{
			BlessCount++;
		}

		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,31) && lpObj->ChaosBox[n].m_Level == 2)
		{
			SoulCount++;
		}

		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,136) && lpObj->ChaosBox[n].m_Level == 2)
		{
			LifeCount++;
		}

		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,137) && lpObj->ChaosBox[n].m_Level == 2)
		{
			CreationCount++;
		}

		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,141) && lpObj->ChaosBox[n].m_Level == 2)
		{
			ChaosCount++;
		}
	}

	if(HornOfFenrirCount != 2 || BlessCount != 1 || SoulCount != 1 || LifeCount != 1 || CreationCount != 1 || ChaosCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	lpObj->ChaosMoney = 150000000;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	lpObj->ChaosSuccessRate = gServerInfo.m_GoldFenrirMixRate[lpObj->AccountLevel];

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	if((rand()%100) < lpObj->ChaosSuccessRate)
	{
		GDCreateItemSend(lpObj->Index,0xFF,0,0,GET_ITEM(13,37),0,255,0,0,0,-1,4,0,0,0,0,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[GoldFenrirMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[GoldFenrirMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
}

//New Season's Mixes
void CChaosBox::AncientHeroSoulMix(LPOBJ lpObj) // OK
{
	int ItemCount = 0;
	int ItemMoney = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].IsSetItem() != 0 && lpObj->ChaosBox[n].m_Level >= 11)
		{
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
	}

	if(ItemCount == 0)
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	if(gServerInfo.m_AncientHeroSoulMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney/300000)+1;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_AncientHeroSoulMixRate[lpObj->AccountLevel];
	}

	if(this->GetTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	if(gServerInfo.m_AncientHeroSoulMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>60)?60:lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 10000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((rand()%100) < lpObj->ChaosSuccessRate)
	{
		GDCreateItemSend(lpObj->Index,0xFF,0,0,GET_ITEM(14,269),0,0,0,0,0,-1,0,0,0,0,0,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[AncientHeroSoulMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[AncientHeroSoulMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
}

void CChaosBox::BloodangelHelmMix(LPOBJ lpObj) // OK
{
	int AncientHeroSoulCount = 0;

	int BloodangelType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 269))
		{
			AncientHeroSoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index >= GET_ITEM(7, 498) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(7, 506))
		{
			BloodangelType = lpObj->ChaosBox[n].m_Index - GET_ITEM(7, 497);
		}
	}

	if (BloodangelType == 0 || (BloodangelType == 1 && (AncientHeroSoulCount != 1)) || (BloodangelType == 2 && (AncientHeroSoulCount != 1)) || (BloodangelType == 3 && (AncientHeroSoulCount != 1)) || (BloodangelType == 4 && (AncientHeroSoulCount != 1)) || (BloodangelType == 5 && (AncientHeroSoulCount != 1)) || (BloodangelType == 6 && (AncientHeroSoulCount != 1)) || (BloodangelType == 7 && (AncientHeroSoulCount != 1)) || (BloodangelType == 8 && (AncientHeroSoulCount != 1)) || (BloodangelType == 9 && (AncientHeroSoulCount != 1)))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_BloodangelSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_BloodangelSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_BloodangelSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 100000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;

		switch (BloodangelType)
		{
		case 1:
			ItemIndex = GET_ITEM(7, 98);
			break;
		case 2:
			ItemIndex = GET_ITEM(7, 99);
			break;
		case 3:
			ItemIndex = GET_ITEM(7, 100);
			break;
		case 4:
			ItemIndex = GET_ITEM(7, 101);
			break;
		case 5:
			ItemIndex = GET_ITEM(7, 102);
			break;
		case 6:
			ItemIndex = GET_ITEM(7, 103);
			break;
		case 7:
			ItemIndex = GET_ITEM(7, 104);
			break;
		case 8:
			ItemIndex = GET_ITEM(7, 158);
			break;
		case 9:
			ItemIndex = GET_ITEM(7, 161);
			break;
		}

		gItemOptionRate.GetItemOption2(8, &ItemOption2);

		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, ItemOption2, 0, -1, 0, 5, 0, 0, 0, 0xFF, 0);

		gLog.Output(LOG_CHAOS_MIX, "[BloodangelSetMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[BloodangelSetMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::BloodangelArmorMix(LPOBJ lpObj) // OK
{
	int AncientHeroSoulCount = 0;

	int BloodangelType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 269))
		{
			AncientHeroSoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index >= GET_ITEM(8, 498) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(8, 506))
		{
			BloodangelType = lpObj->ChaosBox[n].m_Index - GET_ITEM(8, 497);
		}
	}

	if (BloodangelType == 0 || (BloodangelType == 1 && (AncientHeroSoulCount != 1)) || (BloodangelType == 2 && (AncientHeroSoulCount != 1)) || (BloodangelType == 3 && (AncientHeroSoulCount != 1)) || (BloodangelType == 4 && (AncientHeroSoulCount != 1)) || (BloodangelType == 5 && (AncientHeroSoulCount != 1)) || (BloodangelType == 6 && (AncientHeroSoulCount != 1)) || (BloodangelType == 7 && (AncientHeroSoulCount != 1)) || (BloodangelType == 8 && (AncientHeroSoulCount != 1)) || (BloodangelType == 9 && (AncientHeroSoulCount != 1)))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_BloodangelSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_BloodangelSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_BloodangelSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 100000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;

		switch (BloodangelType)
		{
		case 1:
			ItemIndex = GET_ITEM(8, 98);
			break;
		case 2:
			ItemIndex = GET_ITEM(8, 99);
			break;
		case 3:
			ItemIndex = GET_ITEM(8, 100);
			break;
		case 4:
			ItemIndex = GET_ITEM(8, 101);
			break;
		case 5:
			ItemIndex = GET_ITEM(8, 102);
			break;
		case 6:
			ItemIndex = GET_ITEM(8, 103);
			break;
		case 7:
			ItemIndex = GET_ITEM(8, 104);
			break;
		case 8:
			ItemIndex = GET_ITEM(8, 158);
			break;
		case 9:
			ItemIndex = GET_ITEM(8, 161);
			break;
		}

		gItemOptionRate.GetItemOption2(8, &ItemOption2);

		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, ItemOption2, 0, -1, 0, 5, 0, 0, 0, 0xFF, 0);

		gLog.Output(LOG_CHAOS_MIX, "[BloodangelSetMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[BloodangelSetMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::BloodangelPantsMix(LPOBJ lpObj) // OK
{
	int AncientHeroSoulCount = 0;

	int BloodangelType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 269))
		{
			AncientHeroSoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index >= GET_ITEM(9, 498) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(9, 506))
		{
			BloodangelType = lpObj->ChaosBox[n].m_Index - GET_ITEM(9, 497);
		}
	}

	if (BloodangelType == 0 || (BloodangelType == 1 && (AncientHeroSoulCount != 1)) || (BloodangelType == 2 && (AncientHeroSoulCount != 1)) || (BloodangelType == 3 && (AncientHeroSoulCount != 1)) || (BloodangelType == 4 && (AncientHeroSoulCount != 1)) || (BloodangelType == 5 && (AncientHeroSoulCount != 1)) || (BloodangelType == 6 && (AncientHeroSoulCount != 1)) || (BloodangelType == 7 && (AncientHeroSoulCount != 1)) || (BloodangelType == 8 && (AncientHeroSoulCount != 1)) || (BloodangelType == 9 && (AncientHeroSoulCount != 1)))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_BloodangelSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_BloodangelSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_BloodangelSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 100000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;

		switch (BloodangelType)
		{
		case 1:
			ItemIndex = GET_ITEM(9, 98);
			break;
		case 2:
			ItemIndex = GET_ITEM(9, 99);
			break;
		case 3:
			ItemIndex = GET_ITEM(9, 100);
			break;
		case 4:
			ItemIndex = GET_ITEM(9, 101);
			break;
		case 5:
			ItemIndex = GET_ITEM(9, 102);
			break;
		case 6:
			ItemIndex = GET_ITEM(9, 103);
			break;
		case 7:
			ItemIndex = GET_ITEM(9, 104);
			break;
		case 8:
			ItemIndex = GET_ITEM(9, 158);
			break;
		case 9:
			ItemIndex = GET_ITEM(9, 161);
			break;
		}

		gItemOptionRate.GetItemOption2(8, &ItemOption2);

		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, ItemOption2, 0, -1, 0, 5, 0, 0, 0, 0xFF, 0);

		gLog.Output(LOG_CHAOS_MIX, "[BloodangelSetMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[BloodangelSetMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::BloodangelGlovesMix(LPOBJ lpObj) // OK
{
	int AncientHeroSoulCount = 0;

	int BloodangelType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 269))
		{
			AncientHeroSoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index >= GET_ITEM(10, 498) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(10, 506))
		{
			BloodangelType = lpObj->ChaosBox[n].m_Index - GET_ITEM(10, 497);
		}
	}

	if (BloodangelType == 0 || (BloodangelType == 1 && (AncientHeroSoulCount != 1)) || (BloodangelType == 2 && (AncientHeroSoulCount != 1)) || (BloodangelType == 3 && (AncientHeroSoulCount != 1)) || (BloodangelType == 4 && (AncientHeroSoulCount != 1)) || (BloodangelType == 5 && (AncientHeroSoulCount != 1)) || (BloodangelType == 6 && (AncientHeroSoulCount != 1)) || (BloodangelType == 7 && (AncientHeroSoulCount != 1)) || (BloodangelType == 8 && (AncientHeroSoulCount != 1)) || (BloodangelType == 9 && (AncientHeroSoulCount != 1)))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_BloodangelSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_BloodangelSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_BloodangelSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 100000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;

		switch (BloodangelType)
		{
		case 1:
			ItemIndex = GET_ITEM(10, 98);
			break;
		case 2:
			ItemIndex = GET_ITEM(10, 99);
			break;
		case 3:
			ItemIndex = GET_ITEM(10, 100);
			break;
		case 4:
			ItemIndex = GET_ITEM(10, 101);
			break;
		case 5:
			ItemIndex = GET_ITEM(10, 102);
			break;
		case 6:
			ItemIndex = GET_ITEM(10, 103);
			break;
		case 7:
			ItemIndex = GET_ITEM(10, 104);
			break;
		case 8:
			ItemIndex = GET_ITEM(10, 158);
			break;
		case 9:
			ItemIndex = GET_ITEM(10, 161);
			break;
		}

		gItemOptionRate.GetItemOption2(8, &ItemOption2);

		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, ItemOption2, 0, -1, 0, 5, 0, 0, 0, 0xFF, 0);

		gLog.Output(LOG_CHAOS_MIX, "[BloodangelSetMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[BloodangelSetMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::BloodangelBootsMix(LPOBJ lpObj) // OK
{
	int AncientHeroSoulCount = 0;

	int BloodangelType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 269))
		{
			AncientHeroSoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index >= GET_ITEM(11, 498) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(11, 506))
		{
			BloodangelType = lpObj->ChaosBox[n].m_Index - GET_ITEM(11, 497);
		}
	}

	if (BloodangelType == 0 || (BloodangelType == 1 && (AncientHeroSoulCount != 1)) || (BloodangelType == 2 && (AncientHeroSoulCount != 1)) || (BloodangelType == 3 && (AncientHeroSoulCount != 1)) || (BloodangelType == 4 && (AncientHeroSoulCount != 1)) || (BloodangelType == 5 && (AncientHeroSoulCount != 1)) || (BloodangelType == 6 && (AncientHeroSoulCount != 1)) || (BloodangelType == 7 && (AncientHeroSoulCount != 1)) || (BloodangelType == 8 && (AncientHeroSoulCount != 1)) || (BloodangelType == 9 && (AncientHeroSoulCount != 1)))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_BloodangelSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_BloodangelSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_BloodangelSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 100000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;

		switch (BloodangelType)
		{
		case 1:
			ItemIndex = GET_ITEM(11, 98);
			break;
		case 2:
			ItemIndex = GET_ITEM(11, 99);
			break;
		case 3:
			ItemIndex = GET_ITEM(11, 100);
			break;
		case 4:
			ItemIndex = GET_ITEM(11, 101);
			break;
		case 5:
			ItemIndex = GET_ITEM(11, 102);
			break;
		case 6:
			ItemIndex = GET_ITEM(11, 103);
			break;
		case 7:
			ItemIndex = GET_ITEM(11, 104);
			break;
		case 8:
			ItemIndex = GET_ITEM(11, 158);
			break;
		case 9:
			ItemIndex = GET_ITEM(11, 161);
			break;
		}

		gItemOptionRate.GetItemOption2(8, &ItemOption2);

		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, ItemOption2, 0, -1, 0, 5, 0, 0, 0, 0xFF, 0);

		gLog.Output(LOG_CHAOS_MIX, "[BloodangelSetMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[BloodangelSetMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::DarkangelHelmMix(LPOBJ lpObj) // OK
{
	int SealedStoneShardCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int ChaosCount = 0;
	int CreationCount = 0;
	int DarkangelType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 341))
		{
			SealedStoneShardCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 30))
		{
			BlessCount++;
		}
	
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 31))
		{
			SoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 15))
		{
			ChaosCount++;
		}
		
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 22))
		{
			CreationCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index >= GET_ITEM(7, 98) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(7, 104))
		{
			DarkangelType = lpObj->ChaosBox[n].m_Index - GET_ITEM(7, 97);
		}
	}

	if (DarkangelType == 0 || 
		(DarkangelType == 1 && (SealedStoneShardCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) ||
		(DarkangelType == 2 && (SealedStoneShardCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(DarkangelType == 3 && (SealedStoneShardCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(DarkangelType == 4 && (SealedStoneShardCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(DarkangelType == 5 && (SealedStoneShardCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(DarkangelType == 6 && (SealedStoneShardCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(DarkangelType == 7 && (SealedStoneShardCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_DarkangelSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_DarkangelSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_DarkangelSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 250000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;

		switch (DarkangelType)
		{
		case 1:
			ItemIndex = GET_ITEM(7, 138);
			break;
		case 2:
			ItemIndex = GET_ITEM(7, 139);
			break;
		case 3:
			ItemIndex = GET_ITEM(7, 140);
			break;
		case 4:
			ItemIndex = GET_ITEM(7, 141);
			break;
		case 5:
			ItemIndex = GET_ITEM(7, 142);
			break;
		case 6:
			ItemIndex = GET_ITEM(7, 143);
			break;
		case 7:
			ItemIndex = GET_ITEM(7, 144);
			break;
		}

		gItemOptionRate.GetItemOption2(8, &ItemOption2);

		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, ItemOption2, 0, -1, 0, 5, 0, 0, 0, 0xFF, 0);

		gLog.Output(LOG_CHAOS_MIX, "[DarkangelSetMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[DarkangelSetMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::DarkangelArmorMix(LPOBJ lpObj) // OK
{
	int SealedStoneShardCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int ChaosCount = 0;
	int CreationCount = 0;
	int DarkangelType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 341))
		{
			SealedStoneShardCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 30))
		{
			BlessCount++;
		}
	
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 31))
		{
			SoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 15))
		{
			ChaosCount++;
		}
		
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 22))
		{
			CreationCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index >= GET_ITEM(8, 98) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(8, 104))
		{
			DarkangelType = lpObj->ChaosBox[n].m_Index - GET_ITEM(8, 97);
		}
	}

	if (DarkangelType == 0 || 
		(DarkangelType == 1 && (SealedStoneShardCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) ||
		(DarkangelType == 2 && (SealedStoneShardCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(DarkangelType == 3 && (SealedStoneShardCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(DarkangelType == 4 && (SealedStoneShardCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(DarkangelType == 5 && (SealedStoneShardCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(DarkangelType == 6 && (SealedStoneShardCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(DarkangelType == 7 && (SealedStoneShardCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_DarkangelSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_DarkangelSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_DarkangelSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 250000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;

		switch (DarkangelType)
		{
		case 1:
			ItemIndex = GET_ITEM(8, 138);
			break;
		case 2:
			ItemIndex = GET_ITEM(8, 139);
			break;
		case 3:
			ItemIndex = GET_ITEM(8, 140);
			break;
		case 4:
			ItemIndex = GET_ITEM(8, 141);
			break;
		case 5:
			ItemIndex = GET_ITEM(8, 142);
			break;
		case 6:
			ItemIndex = GET_ITEM(8, 143);
			break;
		case 7:
			ItemIndex = GET_ITEM(8, 144);
			break;
		}

		gItemOptionRate.GetItemOption2(8, &ItemOption2);

		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, ItemOption2, 0, -1, 0, 5, 0, 0, 0, 0xFF, 0);

		gLog.Output(LOG_CHAOS_MIX, "[DarkangelSetMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[DarkangelSetMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::DarkangelPantsMix(LPOBJ lpObj) // OK
{
	int SealedStoneShardCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int ChaosCount = 0;
	int CreationCount = 0;
	int DarkangelType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 341))
		{
			SealedStoneShardCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 30))
		{
			BlessCount++;
		}
	
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 31))
		{
			SoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 15))
		{
			ChaosCount++;
		}
		
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 22))
		{
			CreationCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index >= GET_ITEM(9, 98) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(9, 104))
		{
			DarkangelType = lpObj->ChaosBox[n].m_Index - GET_ITEM(9, 97);
		}
	}

	if (DarkangelType == 0 || 
		(DarkangelType == 1 && (SealedStoneShardCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) ||
		(DarkangelType == 2 && (SealedStoneShardCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(DarkangelType == 3 && (SealedStoneShardCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(DarkangelType == 4 && (SealedStoneShardCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(DarkangelType == 5 && (SealedStoneShardCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(DarkangelType == 6 && (SealedStoneShardCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(DarkangelType == 7 && (SealedStoneShardCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_DarkangelSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_DarkangelSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_DarkangelSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 250000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;

		switch (DarkangelType)
		{
		case 1:
			ItemIndex = GET_ITEM(9, 138);
			break;
		case 2:
			ItemIndex = GET_ITEM(9, 139);
			break;
		case 3:
			ItemIndex = GET_ITEM(9, 140);
			break;
		case 4:
			ItemIndex = GET_ITEM(9, 141);
			break;
		case 5:
			ItemIndex = GET_ITEM(9, 142);
			break;
		case 6:
			ItemIndex = GET_ITEM(9, 143);
			break;
		case 7:
			ItemIndex = GET_ITEM(9, 144);
			break;
		}

		gItemOptionRate.GetItemOption2(8, &ItemOption2);

		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, ItemOption2, 0, -1, 0, 5, 0, 0, 0, 0xFF, 0);

		gLog.Output(LOG_CHAOS_MIX, "[DarkangelSetMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[DarkangelSetMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::DarkangelGlovesMix(LPOBJ lpObj) // OK
{
	int SealedStoneShardCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int ChaosCount = 0;
	int CreationCount = 0;
	int DarkangelType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 341))
		{
			SealedStoneShardCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 30))
		{
			BlessCount++;
		}
	
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 31))
		{
			SoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 15))
		{
			ChaosCount++;
		}
		
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 22))
		{
			CreationCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index >= GET_ITEM(10, 98) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(10, 104))
		{
			DarkangelType = lpObj->ChaosBox[n].m_Index - GET_ITEM(10, 97);
		}
	}

	if (DarkangelType == 0 || 
		(DarkangelType == 1 && (SealedStoneShardCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) ||
		(DarkangelType == 2 && (SealedStoneShardCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(DarkangelType == 3 && (SealedStoneShardCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(DarkangelType == 4 && (SealedStoneShardCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(DarkangelType == 5 && (SealedStoneShardCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(DarkangelType == 6 && (SealedStoneShardCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(DarkangelType == 7 && (SealedStoneShardCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_DarkangelSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_DarkangelSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_DarkangelSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 250000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;

		switch (DarkangelType)
		{
		case 1:
			ItemIndex = GET_ITEM(10, 138);
			break;
		case 2:
			ItemIndex = GET_ITEM(10, 139);
			break;
		case 3:
			ItemIndex = GET_ITEM(10, 140);
			break;
		case 4:
			ItemIndex = GET_ITEM(10, 141);
			break;
		case 5:
			ItemIndex = GET_ITEM(10, 142);
			break;
		case 6:
			ItemIndex = GET_ITEM(10, 143);
			break;
		case 7:
			ItemIndex = GET_ITEM(10, 144);
			break;
		}

		gItemOptionRate.GetItemOption2(8, &ItemOption2);

		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, ItemOption2, 0, -1, 0, 5, 0, 0, 0, 0xFF, 0);

		gLog.Output(LOG_CHAOS_MIX, "[DarkangelSetMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[DarkangelSetMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::DarkangelBootsMix(LPOBJ lpObj) // OK
{
	int SealedStoneShardCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int ChaosCount = 0;
	int CreationCount = 0;
	int DarkangelType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 341))
		{
			SealedStoneShardCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 30))
		{
			BlessCount++;
		}
	
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 31))
		{
			SoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 15))
		{
			ChaosCount++;
		}
		
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 22))
		{
			CreationCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index >= GET_ITEM(11, 98) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(11, 104))
		{
			DarkangelType = lpObj->ChaosBox[n].m_Index - GET_ITEM(11, 97);
		}
	}

	if (DarkangelType == 0 || 
		(DarkangelType == 1 && (SealedStoneShardCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) ||
		(DarkangelType == 2 && (SealedStoneShardCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(DarkangelType == 3 && (SealedStoneShardCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(DarkangelType == 4 && (SealedStoneShardCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(DarkangelType == 5 && (SealedStoneShardCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(DarkangelType == 6 && (SealedStoneShardCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(DarkangelType == 7 && (SealedStoneShardCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_DarkangelSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_DarkangelSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_DarkangelSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 250000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;

		switch (DarkangelType)
		{
		case 1:
			ItemIndex = GET_ITEM(11, 138);
			break;
		case 2:
			ItemIndex = GET_ITEM(11, 139);
			break;
		case 3:
			ItemIndex = GET_ITEM(11, 140);
			break;
		case 4:
			ItemIndex = GET_ITEM(11, 141);
			break;
		case 5:
			ItemIndex = GET_ITEM(11, 142);
			break;
		case 6:
			ItemIndex = GET_ITEM(11, 143);
			break;
		case 7:
			ItemIndex = GET_ITEM(11, 144);
			break;
		}

		gItemOptionRate.GetItemOption2(8, &ItemOption2);

		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, ItemOption2, 0, -1, 0, 5, 0, 0, 0, 0xFF, 0);

		gLog.Output(LOG_CHAOS_MIX, "[DarkangelSetMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[DarkangelSetMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::DarkangelEnergyElfHelmMix(LPOBJ lpObj) // OK
{
	int SealedStoneShardCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int ChaosCount = 0;
	int CreationCount = 0;
	int DarkangelType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 341))
		{
			SealedStoneShardCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 30))
		{
			BlessCount++;
		}
	
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 31))
		{
			SoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 15))
		{
			ChaosCount++;
		}
		
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 22))
		{
			CreationCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(7, 158))
		{
			DarkangelType++;
		}
	}

	if ((SealedStoneShardCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1) && (DarkangelType != 1))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_DarkangelSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_DarkangelSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_DarkangelSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 250000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;
		
		gItemOptionRate.GetItemOption2(8, &ItemOption2);

		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, GET_ITEM(7, 159), 0, 0, 1, ItemOption2, 0, -1, 0, 5, 0, 0, 0, 0xFF, 0);

		gLog.Output(LOG_CHAOS_MIX,"[DarkangelSetMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[DarkangelSetMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::DarkangelEnergyElfArmorMix(LPOBJ lpObj) // OK
{
	int SealedStoneShardCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int ChaosCount = 0;
	int CreationCount = 0;
	int DarkangelType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 341))
		{
			SealedStoneShardCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 30))
		{
			BlessCount++;
		}
	
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 31))
		{
			SoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 15))
		{
			ChaosCount++;
		}
		
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 22))
		{
			CreationCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(8, 158))
		{
			DarkangelType++;
		}
	}

	if ((SealedStoneShardCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1) && (DarkangelType != 1))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_DarkangelSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_DarkangelSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_DarkangelSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 250000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;
		
		gItemOptionRate.GetItemOption2(8, &ItemOption2);

		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, GET_ITEM(8, 159), 0, 0, 1, ItemOption2, 0, -1, 0, 5, 0, 0, 0, 0xFF, 0);

		gLog.Output(LOG_CHAOS_MIX,"[DarkangelSetMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[DarkangelSetMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::DarkangelEnergyElfPantsMix(LPOBJ lpObj) // OK
{
	int SealedStoneShardCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int ChaosCount = 0;
	int CreationCount = 0;
	int DarkangelType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 341))
		{
			SealedStoneShardCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 30))
		{
			BlessCount++;
		}
	
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 31))
		{
			SoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 15))
		{
			ChaosCount++;
		}
		
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 22))
		{
			CreationCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(9, 158))
		{
			DarkangelType++;
		}
	}

	if ((SealedStoneShardCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1) && (DarkangelType != 1))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_DarkangelSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_DarkangelSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_DarkangelSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 250000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;
		
		gItemOptionRate.GetItemOption2(8, &ItemOption2);

		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, GET_ITEM(9, 159), 0, 0, 1, ItemOption2, 0, -1, 0, 5, 0, 0, 0, 0xFF, 0);

		gLog.Output(LOG_CHAOS_MIX,"[DarkangelSetMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[DarkangelSetMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::DarkangelEnergyElfGlovesMix(LPOBJ lpObj) // OK
{
	int SealedStoneShardCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int ChaosCount = 0;
	int CreationCount = 0;
	int DarkangelType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 341))
		{
			SealedStoneShardCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 30))
		{
			BlessCount++;
		}
	
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 31))
		{
			SoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 15))
		{
			ChaosCount++;
		}
		
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 22))
		{
			CreationCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(10, 158))
		{
			DarkangelType++;
		}
	}

	if ((SealedStoneShardCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1) && (DarkangelType != 1))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_DarkangelSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_DarkangelSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_DarkangelSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 250000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;
		
		gItemOptionRate.GetItemOption2(8, &ItemOption2);

		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, GET_ITEM(10, 159), 0, 0, 1, ItemOption2, 0, -1, 0, 5, 0, 0, 0, 0xFF, 0);

		gLog.Output(LOG_CHAOS_MIX,"[DarkangelSetMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[DarkangelSetMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::DarkangelEnergyMagicArmorMix(LPOBJ lpObj) // OK
{
	int SealedStoneShardCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int ChaosCount = 0;
	int CreationCount = 0;
	int DarkangelType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 341))
		{
			SealedStoneShardCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 30))
		{
			BlessCount++;
		}
	
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 31))
		{
			SoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 15))
		{
			ChaosCount++;
		}
		
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 22))
		{
			CreationCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(8, 161))
		{
			DarkangelType++;
		}
	}

	if ((SealedStoneShardCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1) && (DarkangelType != 1))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_DarkangelSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_DarkangelSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_DarkangelSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 250000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;
		
		gItemOptionRate.GetItemOption2(8, &ItemOption2);

		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, GET_ITEM(8, 162), 0, 0, 1, ItemOption2, 0, -1, 0, 5, 0, 0, 0, 0xFF, 0);

		gLog.Output(LOG_CHAOS_MIX,"[DarkangelSetMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[DarkangelSetMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::DarkangelEnergyMagicPantsMix(LPOBJ lpObj) // OK
{
	int SealedStoneShardCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int ChaosCount = 0;
	int CreationCount = 0;
	int DarkangelType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 341))
		{
			SealedStoneShardCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 30))
		{
			BlessCount++;
		}
	
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 31))
		{
			SoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 15))
		{
			ChaosCount++;
		}
		
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 22))
		{
			CreationCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(9, 161))
		{
			DarkangelType++;
		}
	}

	if ((SealedStoneShardCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1) && (DarkangelType != 1))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_DarkangelSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_DarkangelSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_DarkangelSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 250000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;
		
		gItemOptionRate.GetItemOption2(8, &ItemOption2);

		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, GET_ITEM(9, 162), 0, 0, 1, ItemOption2, 0, -1, 0, 5, 0, 0, 0, 0xFF, 0);

		gLog.Output(LOG_CHAOS_MIX,"[DarkangelSetMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[DarkangelSetMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::DarkangelEnergyMagicGlovesMix(LPOBJ lpObj) // OK
{
	int SealedStoneShardCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int ChaosCount = 0;
	int CreationCount = 0;
	int DarkangelType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 341))
		{
			SealedStoneShardCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 30))
		{
			BlessCount++;
		}
	
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 31))
		{
			SoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 15))
		{
			ChaosCount++;
		}
		
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 22))
		{
			CreationCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(10, 161))
		{
			DarkangelType++;
		}
	}

	if ((SealedStoneShardCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1) && (DarkangelType != 1))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_DarkangelSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_DarkangelSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_DarkangelSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 250000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;
		
		gItemOptionRate.GetItemOption2(8, &ItemOption2);

		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, GET_ITEM(10, 162), 0, 0, 1, ItemOption2, 0, -1, 0, 5, 0, 0, 0, 0xFF, 0);

		gLog.Output(LOG_CHAOS_MIX,"[DarkangelSetMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[DarkangelSetMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::DarkangelEnergyMagicBootsMix(LPOBJ lpObj) // OK
{
	int SealedStoneShardCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int ChaosCount = 0;
	int CreationCount = 0;
	int DarkangelType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 341))
		{
			SealedStoneShardCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 30))
		{
			BlessCount++;
		}
	
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 31))
		{
			SoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 15))
		{
			ChaosCount++;
		}
		
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 22))
		{
			CreationCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(11, 161))
		{
			DarkangelType++;
		}
	}

	if ((SealedStoneShardCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1) && (DarkangelType != 1))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_DarkangelSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_DarkangelSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_DarkangelSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 250000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;
		
		gItemOptionRate.GetItemOption2(8, &ItemOption2);

		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, GET_ITEM(11, 162), 0, 0, 1, ItemOption2, 0, -1, 0, 5, 0, 0, 0, 0xFF, 0);

		gLog.Output(LOG_CHAOS_MIX,"[DarkangelSetMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[DarkangelSetMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::HolyangelHelmMix(LPOBJ lpObj) // OK
{
	int HolyangelSpiritCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int ChaosCount = 0;
	int CreationCount = 0;
	int HolyangelType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 389))
		{
			HolyangelSpiritCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 30))
		{
			BlessCount++;
		}
	
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 31))
		{
			SoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 15))
		{
			ChaosCount++;
		}
		
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 22))
		{
			CreationCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index >= GET_ITEM(7, 138) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(7, 144))
		{
			HolyangelType = lpObj->ChaosBox[n].m_Index - GET_ITEM(7, 137);
		}
	}

	if (HolyangelType == 0 || 
		(HolyangelType == 1 && (HolyangelSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) ||
		(HolyangelType == 2 && (HolyangelSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(HolyangelType == 3 && (HolyangelSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(HolyangelType == 4 && (HolyangelSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(HolyangelType == 5 && (HolyangelSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(HolyangelType == 6 && (HolyangelSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(HolyangelType == 7 && (HolyangelSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_HolyangelSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_HolyangelSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_HolyangelSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 500000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;

		switch (HolyangelType)
		{
		case 1:
			ItemIndex = GET_ITEM(7, 150);
			break;
		case 2:
			ItemIndex = GET_ITEM(7, 151);
			break;
		case 3:
			ItemIndex = GET_ITEM(7, 152);
			break;
		case 4:
			ItemIndex = GET_ITEM(7, 153);
			break;
		case 5:
			ItemIndex = GET_ITEM(7, 154);
			break;
		case 6:
			ItemIndex = GET_ITEM(7, 155);
			break;
		case 7:
			ItemIndex = GET_ITEM(7, 156);
			break;
		}

		gItemOptionRate.GetItemOption2(8, &ItemOption2);

		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, ItemOption2, 0, -1, 0, 5, 0, 0, 0, 0xFF, 0);

		gLog.Output(LOG_CHAOS_MIX, "[HolyangelSetMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[HolyangelSetMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::HolyangelArmorMix(LPOBJ lpObj) // OK
{
	int HolyangelSpiritCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int ChaosCount = 0;
	int CreationCount = 0;
	int HolyangelType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 389))
		{
			HolyangelSpiritCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 30))
		{
			BlessCount++;
		}
	
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 31))
		{
			SoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 15))
		{
			ChaosCount++;
		}
		
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 22))
		{
			CreationCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index >= GET_ITEM(8, 138) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(8, 144))
		{
			HolyangelType = lpObj->ChaosBox[n].m_Index - GET_ITEM(8, 137);
		}
	}

	if (HolyangelType == 0 || 
		(HolyangelType == 1 && (HolyangelSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) ||
		(HolyangelType == 2 && (HolyangelSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(HolyangelType == 3 && (HolyangelSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(HolyangelType == 4 && (HolyangelSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(HolyangelType == 5 && (HolyangelSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(HolyangelType == 6 && (HolyangelSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(HolyangelType == 7 && (HolyangelSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_HolyangelSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_HolyangelSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_HolyangelSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 500000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;

		switch (HolyangelType)
		{
		case 1:
			ItemIndex = GET_ITEM(8, 150);
			break;
		case 2:
			ItemIndex = GET_ITEM(8, 151);
			break;
		case 3:
			ItemIndex = GET_ITEM(8, 152);
			break;
		case 4:
			ItemIndex = GET_ITEM(8, 153);
			break;
		case 5:
			ItemIndex = GET_ITEM(8, 154);
			break;
		case 6:
			ItemIndex = GET_ITEM(8, 155);
			break;
		case 7:
			ItemIndex = GET_ITEM(8, 156);
			break;
		}

		gItemOptionRate.GetItemOption2(8, &ItemOption2);

		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, ItemOption2, 0, -1, 0, 5, 0, 0, 0, 0xFF, 0);

		gLog.Output(LOG_CHAOS_MIX, "[HolyangelSetMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[HolyangelSetMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::HolyangelPantsMix(LPOBJ lpObj) // OK
{
	int HolyangelSpiritCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int ChaosCount = 0;
	int CreationCount = 0;
	int HolyangelType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 389))
		{
			HolyangelSpiritCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 30))
		{
			BlessCount++;
		}
	
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 31))
		{
			SoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 15))
		{
			ChaosCount++;
		}
		
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 22))
		{
			CreationCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index >= GET_ITEM(9, 138) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(9, 144))
		{
			HolyangelType = lpObj->ChaosBox[n].m_Index - GET_ITEM(9, 137);
		}
	}

	if (HolyangelType == 0 || 
		(HolyangelType == 1 && (HolyangelSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) ||
		(HolyangelType == 2 && (HolyangelSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(HolyangelType == 3 && (HolyangelSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(HolyangelType == 4 && (HolyangelSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(HolyangelType == 5 && (HolyangelSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(HolyangelType == 6 && (HolyangelSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(HolyangelType == 7 && (HolyangelSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_HolyangelSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_HolyangelSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_HolyangelSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 500000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;

		switch (HolyangelType)
		{
		case 1:
			ItemIndex = GET_ITEM(9, 150);
			break;
		case 2:
			ItemIndex = GET_ITEM(9, 151);
			break;
		case 3:
			ItemIndex = GET_ITEM(9, 152);
			break;
		case 4:
			ItemIndex = GET_ITEM(9, 153);
			break;
		case 5:
			ItemIndex = GET_ITEM(9, 154);
			break;
		case 6:
			ItemIndex = GET_ITEM(9, 155);
			break;
		case 7:
			ItemIndex = GET_ITEM(9, 156);
			break;
		}

		gItemOptionRate.GetItemOption2(8, &ItemOption2);

		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, ItemOption2, 0, -1, 0, 5, 0, 0, 0, 0xFF, 0);

		gLog.Output(LOG_CHAOS_MIX, "[HolyangelSetMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[HolyangelSetMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::HolyangelGlovesMix(LPOBJ lpObj) // OK
{
	int HolyangelSpiritCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int ChaosCount = 0;
	int CreationCount = 0;
	int HolyangelType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 389))
		{
			HolyangelSpiritCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 30))
		{
			BlessCount++;
		}
	
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 31))
		{
			SoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 15))
		{
			ChaosCount++;
		}
		
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 22))
		{
			CreationCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index >= GET_ITEM(10, 138) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(10, 144))
		{
			HolyangelType = lpObj->ChaosBox[n].m_Index - GET_ITEM(10, 137);
		}
	}

	if (HolyangelType == 0 || 
		(HolyangelType == 1 && (HolyangelSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) ||
		(HolyangelType == 2 && (HolyangelSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(HolyangelType == 3 && (HolyangelSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(HolyangelType == 4 && (HolyangelSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(HolyangelType == 5 && (HolyangelSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(HolyangelType == 6 && (HolyangelSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(HolyangelType == 7 && (HolyangelSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_HolyangelSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_HolyangelSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_HolyangelSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 500000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;

		switch (HolyangelType)
		{
		case 1:
			ItemIndex = GET_ITEM(10, 150);
			break;
		case 2:
			ItemIndex = GET_ITEM(10, 151);
			break;
		case 3:
			ItemIndex = GET_ITEM(10, 152);
			break;
		case 4:
			ItemIndex = GET_ITEM(10, 153);
			break;
		case 5:
			ItemIndex = GET_ITEM(10, 154);
			break;
		case 6:
			ItemIndex = GET_ITEM(10, 155);
			break;
		case 7:
			ItemIndex = GET_ITEM(10, 156);
			break;
		}

		gItemOptionRate.GetItemOption2(8, &ItemOption2);

		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, ItemOption2, 0, -1, 0, 5, 0, 0, 0, 0xFF, 0);

		gLog.Output(LOG_CHAOS_MIX, "[HolyangelSetMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[HolyangelSetMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::HolyangelBootsMix(LPOBJ lpObj) // OK
{
	int HolyangelSpiritCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int ChaosCount = 0;
	int CreationCount = 0;
	int HolyangelType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 389))
		{
			HolyangelSpiritCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 30))
		{
			BlessCount++;
		}
	
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 31))
		{
			SoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 15))
		{
			ChaosCount++;
		}
		
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 22))
		{
			CreationCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index >= GET_ITEM(11, 138) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(11, 144))
		{
			HolyangelType = lpObj->ChaosBox[n].m_Index - GET_ITEM(11, 137);
		}
	}

	if (HolyangelType == 0 || 
		(HolyangelType == 1 && (HolyangelSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) ||
		(HolyangelType == 2 && (HolyangelSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(HolyangelType == 3 && (HolyangelSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(HolyangelType == 4 && (HolyangelSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(HolyangelType == 5 && (HolyangelSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(HolyangelType == 6 && (HolyangelSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(HolyangelType == 7 && (HolyangelSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_HolyangelSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_HolyangelSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_HolyangelSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 500000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;

		switch (HolyangelType)
		{
		case 1:
			ItemIndex = GET_ITEM(11, 150);
			break;
		case 2:
			ItemIndex = GET_ITEM(11, 151);
			break;
		case 3:
			ItemIndex = GET_ITEM(11, 152);
			break;
		case 4:
			ItemIndex = GET_ITEM(11, 153);
			break;
		case 5:
			ItemIndex = GET_ITEM(11, 154);
			break;
		case 6:
			ItemIndex = GET_ITEM(11, 155);
			break;
		case 7:
			ItemIndex = GET_ITEM(11, 156);
			break;
		}

		gItemOptionRate.GetItemOption2(8, &ItemOption2);

		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, ItemOption2, 0, -1, 0, 5, 0, 0, 0, 0xFF, 0);

		gLog.Output(LOG_CHAOS_MIX, "[HolyangelSetMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[HolyangelSetMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::HolyangelEnergyElfHelmMix(LPOBJ lpObj) // OK
{
	int SealedStoneShardCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int ChaosCount = 0;
	int CreationCount = 0;
	int HolyangelType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 389))
		{
			SealedStoneShardCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 30))
		{
			BlessCount++;
		}
	
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 31))
		{
			SoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 15))
		{
			ChaosCount++;
		}
		
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 22))
		{
			CreationCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(7, 159))
		{
			HolyangelType++;
		}
	}

	if ((SealedStoneShardCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1) && (HolyangelType != 1))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_HolyangelSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_HolyangelSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_HolyangelSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 500000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;
		
		gItemOptionRate.GetItemOption2(8, &ItemOption2);

		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, GET_ITEM(7, 160), 0, 0, 1, ItemOption2, 0, -1, 0, 5, 0, 0, 0, 0xFF, 0);

		gLog.Output(LOG_CHAOS_MIX,"[HolyangelSetMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[HolyangelSetMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::HolyangelEnergyElfArmorMix(LPOBJ lpObj) // OK
{
	int SealedStoneShardCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int ChaosCount = 0;
	int CreationCount = 0;
	int HolyangelType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 389))
		{
			SealedStoneShardCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 30))
		{
			BlessCount++;
		}
	
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 31))
		{
			SoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 15))
		{
			ChaosCount++;
		}
		
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 22))
		{
			CreationCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(8, 159))
		{
			HolyangelType++;
		}
	}

	if ((SealedStoneShardCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1) && (HolyangelType != 1))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_HolyangelSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_HolyangelSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_HolyangelSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 500000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;
		
		gItemOptionRate.GetItemOption2(8, &ItemOption2);

		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, GET_ITEM(8, 160), 0, 0, 1, ItemOption2, 0, -1, 0, 5, 0, 0, 0, 0xFF, 0);

		gLog.Output(LOG_CHAOS_MIX,"[HolyangelSetMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[HolyangelSetMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::HolyangelEnergyElfPantsMix(LPOBJ lpObj) // OK
{
	int SealedStoneShardCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int ChaosCount = 0;
	int CreationCount = 0;
	int HolyangelType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 389))
		{
			SealedStoneShardCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 30))
		{
			BlessCount++;
		}
	
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 31))
		{
			SoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 15))
		{
			ChaosCount++;
		}
		
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 22))
		{
			CreationCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(9, 159))
		{
			HolyangelType++;
		}
	}

	if ((SealedStoneShardCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1) && (HolyangelType != 1))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_HolyangelSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_HolyangelSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_HolyangelSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 500000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;
		
		gItemOptionRate.GetItemOption2(8, &ItemOption2);

		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, GET_ITEM(9, 160), 0, 0, 1, ItemOption2, 0, -1, 0, 5, 0, 0, 0, 0xFF, 0);

		gLog.Output(LOG_CHAOS_MIX,"[HolyangelSetMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[HolyangelSetMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::HolyangelEnergyElfGlovesMix(LPOBJ lpObj) // OK
{
	int SealedStoneShardCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int ChaosCount = 0;
	int CreationCount = 0;
	int HolyangelType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 389))
		{
			SealedStoneShardCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 30))
		{
			BlessCount++;
		}
	
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 31))
		{
			SoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 15))
		{
			ChaosCount++;
		}
		
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 22))
		{
			CreationCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(10, 159))
		{
			HolyangelType++;
		}
	}

	if ((SealedStoneShardCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1) && (HolyangelType != 1))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_HolyangelSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_HolyangelSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_HolyangelSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 500000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;
		
		gItemOptionRate.GetItemOption2(8, &ItemOption2);

		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, GET_ITEM(10, 160), 0, 0, 1, ItemOption2, 0, -1, 0, 5, 0, 0, 0, 0xFF, 0);

		gLog.Output(LOG_CHAOS_MIX,"[HolyangelSetMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[HolyangelSetMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::HolyangelEnergyMagicArmorMix(LPOBJ lpObj) // OK
{
	int SealedStoneShardCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int ChaosCount = 0;
	int CreationCount = 0;
	int HolyangelType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 389))
		{
			SealedStoneShardCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 30))
		{
			BlessCount++;
		}
	
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 31))
		{
			SoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 15))
		{
			ChaosCount++;
		}
		
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 22))
		{
			CreationCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(8, 162))
		{
			HolyangelType++;
		}
	}

	if ((SealedStoneShardCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1) && (HolyangelType != 1))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_HolyangelSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_HolyangelSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_HolyangelSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 500000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;
		
		gItemOptionRate.GetItemOption2(8, &ItemOption2);

		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, GET_ITEM(8, 163), 0, 0, 1, ItemOption2, 0, -1, 0, 5, 0, 0, 0, 0xFF, 0);

		gLog.Output(LOG_CHAOS_MIX,"[HolyangelSetMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[HolyangelSetMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::HolyangelEnergyMagicPantsMix(LPOBJ lpObj) // OK
{
	int SealedStoneShardCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int ChaosCount = 0;
	int CreationCount = 0;
	int HolyangelType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 389))
		{
			SealedStoneShardCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 30))
		{
			BlessCount++;
		}
	
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 31))
		{
			SoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 15))
		{
			ChaosCount++;
		}
		
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 22))
		{
			CreationCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(9, 162))
		{
			HolyangelType++;
		}
	}

	if ((SealedStoneShardCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1) && (HolyangelType != 1))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_HolyangelSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_HolyangelSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_HolyangelSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 500000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;
		
		gItemOptionRate.GetItemOption2(8, &ItemOption2);

		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, GET_ITEM(9, 163), 0, 0, 1, ItemOption2, 0, -1, 0, 5, 0, 0, 0, 0xFF, 0);

		gLog.Output(LOG_CHAOS_MIX,"[HolyangelSetMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[HolyangelSetMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::HolyangelEnergyMagicGlovesMix(LPOBJ lpObj) // OK
{
	int SealedStoneShardCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int ChaosCount = 0;
	int CreationCount = 0;
	int HolyangelType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 389))
		{
			SealedStoneShardCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 30))
		{
			BlessCount++;
		}
	
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 31))
		{
			SoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 15))
		{
			ChaosCount++;
		}
		
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 22))
		{
			CreationCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(10, 162))
		{
			HolyangelType++;
		}
	}

	if ((SealedStoneShardCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1) && (HolyangelType != 1))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_HolyangelSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_HolyangelSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_HolyangelSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 500000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;
		
		gItemOptionRate.GetItemOption2(8, &ItemOption2);

		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, GET_ITEM(10, 163), 0, 0, 1, ItemOption2, 0, -1, 0, 5, 0, 0, 0, 0xFF, 0);

		gLog.Output(LOG_CHAOS_MIX,"[HolyangelSetMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[HolyangelSetMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::HolyangelEnergyMagicBootsMix(LPOBJ lpObj) // OK
{
	int SealedStoneShardCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int ChaosCount = 0;
	int CreationCount = 0;
	int HolyangelType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 389))
		{
			SealedStoneShardCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 30))
		{
			BlessCount++;
		}
	
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 31))
		{
			SoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 15))
		{
			ChaosCount++;
		}
		
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 22))
		{
			CreationCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(11, 162))
		{
			HolyangelType++;
		}
	}

	if ((SealedStoneShardCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1) && (HolyangelType != 1))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_HolyangelSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_HolyangelSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_HolyangelSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 500000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;
		
		gItemOptionRate.GetItemOption2(8, &ItemOption2);

		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, GET_ITEM(11, 163), 0, 0, 1, ItemOption2, 0, -1, 0, 5, 0, 0, 0, 0xFF, 0);

		gLog.Output(LOG_CHAOS_MIX,"[HolyangelSetMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[HolyangelSetMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::AwakeningHelmMix(LPOBJ lpObj) // OK
{
	int AwakeningSoulCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int ChaosCount = 0;
	int CreationCount = 0;
	int AwakeningType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 415))
		{
			AwakeningSoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 30))
		{
			BlessCount++;
		}
	
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 31))
		{
			SoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 15))
		{
			ChaosCount++;
		}
		
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 22))
		{
			CreationCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index >= GET_ITEM(7, 150) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(7, 156))
		{
			AwakeningType = lpObj->ChaosBox[n].m_Index - GET_ITEM(7, 149);
		}
	}

	if (AwakeningType == 0 || 
		(AwakeningType == 1 && (AwakeningSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) ||
		(AwakeningType == 2 && (AwakeningSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(AwakeningType == 3 && (AwakeningSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(AwakeningType == 4 && (AwakeningSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(AwakeningType == 5 && (AwakeningSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(AwakeningType == 6 && (AwakeningSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(AwakeningType == 7 && (AwakeningSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_AwakeningSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_AwakeningSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_AwakeningSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 750000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;

		switch (AwakeningType)
		{
		case 1:
			ItemIndex = GET_ITEM(7, 106);
			break;
		case 2:
			ItemIndex = GET_ITEM(7, 107);
			break;
		case 3:
			ItemIndex = GET_ITEM(7, 108);
			break;
		case 4:
			ItemIndex = GET_ITEM(7, 109);
			break;
		case 5:
			ItemIndex = GET_ITEM(7, 110);
			break;
		case 6:
			ItemIndex = GET_ITEM(7, 111);
			break;
		case 7:
			ItemIndex = GET_ITEM(7, 112);
			break;
		}

		gItemOptionRate.GetItemOption2(8, &ItemOption2);

		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, ItemOption2, 0, -1, 0, 5, 0, 0, 0, 0xFF, 0);

		gLog.Output(LOG_CHAOS_MIX, "[AwakeningSetMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[AwakeningSetMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::AwakeningArmorMix(LPOBJ lpObj) // OK
{
	int AwakeningSoulCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int ChaosCount = 0;
	int CreationCount = 0;
	int AwakeningType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 415))
		{
			AwakeningSoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 30))
		{
			BlessCount++;
		}
	
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 31))
		{
			SoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 15))
		{
			ChaosCount++;
		}
		
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 22))
		{
			CreationCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index >= GET_ITEM(8, 150) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(8, 156))
		{
			AwakeningType = lpObj->ChaosBox[n].m_Index - GET_ITEM(8, 149);
		}
	}

	if (AwakeningType == 0 || 
		(AwakeningType == 1 && (AwakeningSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) ||
		(AwakeningType == 2 && (AwakeningSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(AwakeningType == 3 && (AwakeningSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(AwakeningType == 4 && (AwakeningSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(AwakeningType == 5 && (AwakeningSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(AwakeningType == 6 && (AwakeningSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(AwakeningType == 7 && (AwakeningSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_AwakeningSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_AwakeningSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_AwakeningSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 750000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;

		switch (AwakeningType)
		{
		case 1:
			ItemIndex = GET_ITEM(8, 106);
			break;
		case 2:
			ItemIndex = GET_ITEM(8, 107);
			break;
		case 3:
			ItemIndex = GET_ITEM(8, 108);
			break;
		case 4:
			ItemIndex = GET_ITEM(8, 109);
			break;
		case 5:
			ItemIndex = GET_ITEM(8, 110);
			break;
		case 6:
			ItemIndex = GET_ITEM(8, 111);
			break;
		case 7:
			ItemIndex = GET_ITEM(8, 112);
			break;
		}

		gItemOptionRate.GetItemOption2(8, &ItemOption2);

		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, ItemOption2, 0, -1, 0, 5, 0, 0, 0, 0xFF, 0);

		gLog.Output(LOG_CHAOS_MIX, "[AwakeningSetMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[AwakeningSetMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::AwakeningPantsMix(LPOBJ lpObj) // OK
{
	int AwakeningSoulCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int ChaosCount = 0;
	int CreationCount = 0;
	int AwakeningType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 415))
		{
			AwakeningSoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 30))
		{
			BlessCount++;
		}
	
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 31))
		{
			SoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 15))
		{
			ChaosCount++;
		}
		
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 22))
		{
			CreationCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index >= GET_ITEM(9, 150) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(9, 156))
		{
			AwakeningType = lpObj->ChaosBox[n].m_Index - GET_ITEM(9, 149);
		}
	}

	if (AwakeningType == 0 || 
		(AwakeningType == 1 && (AwakeningSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) ||
		(AwakeningType == 2 && (AwakeningSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(AwakeningType == 3 && (AwakeningSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(AwakeningType == 4 && (AwakeningSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(AwakeningType == 5 && (AwakeningSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(AwakeningType == 6 && (AwakeningSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(AwakeningType == 7 && (AwakeningSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_AwakeningSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_AwakeningSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_AwakeningSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 750000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;

		switch (AwakeningType)
		{
		case 1:
			ItemIndex = GET_ITEM(9, 106);
			break;
		case 2:
			ItemIndex = GET_ITEM(9, 107);
			break;
		case 3:
			ItemIndex = GET_ITEM(9, 108);
			break;
		case 4:
			ItemIndex = GET_ITEM(9, 109);
			break;
		case 5:
			ItemIndex = GET_ITEM(9, 110);
			break;
		case 6:
			ItemIndex = GET_ITEM(9, 111);
			break;
		case 7:
			ItemIndex = GET_ITEM(9, 112);
			break;
		}

		gItemOptionRate.GetItemOption2(8, &ItemOption2);

		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, ItemOption2, 0, -1, 0, 5, 0, 0, 0, 0xFF, 0);

		gLog.Output(LOG_CHAOS_MIX, "[AwakeningSetMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[AwakeningSetMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::AwakeningGlovesMix(LPOBJ lpObj) // OK
{
	int AwakeningSoulCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int ChaosCount = 0;
	int CreationCount = 0;
	int AwakeningType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 415))
		{
			AwakeningSoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 30))
		{
			BlessCount++;
		}
	
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 31))
		{
			SoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 15))
		{
			ChaosCount++;
		}
		
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 22))
		{
			CreationCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index >= GET_ITEM(10, 150) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(10, 156))
		{
			AwakeningType = lpObj->ChaosBox[n].m_Index - GET_ITEM(10, 149);
		}
	}

	if (AwakeningType == 0 || 
		(AwakeningType == 1 && (AwakeningSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) ||
		(AwakeningType == 2 && (AwakeningSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(AwakeningType == 3 && (AwakeningSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(AwakeningType == 4 && (AwakeningSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(AwakeningType == 5 && (AwakeningSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(AwakeningType == 6 && (AwakeningSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(AwakeningType == 7 && (AwakeningSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_AwakeningSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_AwakeningSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_AwakeningSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 750000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;

		switch (AwakeningType)
		{
		case 1:
			ItemIndex = GET_ITEM(10, 106);
			break;
		case 2:
			ItemIndex = GET_ITEM(10, 107);
			break;
		case 3:
			ItemIndex = GET_ITEM(10, 108);
			break;
		case 4:
			ItemIndex = GET_ITEM(10, 109);
			break;
		case 5:
			ItemIndex = GET_ITEM(10, 110);
			break;
		case 6:
			ItemIndex = GET_ITEM(10, 111);
			break;
		case 7:
			ItemIndex = GET_ITEM(10, 112);
			break;
		}

		gItemOptionRate.GetItemOption2(8, &ItemOption2);

		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, ItemOption2, 0, -1, 0, 5, 0, 0, 0, 0xFF, 0);

		gLog.Output(LOG_CHAOS_MIX, "[AwakeningSetMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[AwakeningSetMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::AwakeningBootsMix(LPOBJ lpObj) // OK
{
	int AwakeningSoulCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int ChaosCount = 0;
	int CreationCount = 0;
	int AwakeningType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 415))
		{
			AwakeningSoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 30))
		{
			BlessCount++;
		}
	
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 31))
		{
			SoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 15))
		{
			ChaosCount++;
		}
		
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 22))
		{
			CreationCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index >= GET_ITEM(11, 150) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(11, 156))
		{
			AwakeningType = lpObj->ChaosBox[n].m_Index - GET_ITEM(11, 149);
		}
	}

	if (AwakeningType == 0 || 
		(AwakeningType == 1 && (AwakeningSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) ||
		(AwakeningType == 2 && (AwakeningSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(AwakeningType == 3 && (AwakeningSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(AwakeningType == 4 && (AwakeningSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(AwakeningType == 5 && (AwakeningSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(AwakeningType == 6 && (AwakeningSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(AwakeningType == 7 && (AwakeningSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_AwakeningSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_AwakeningSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_AwakeningSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 750000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;

		switch (AwakeningType)
		{
		case 1:
			ItemIndex = GET_ITEM(11, 106);
			break;
		case 2:
			ItemIndex = GET_ITEM(11, 107);
			break;
		case 3:
			ItemIndex = GET_ITEM(11, 108);
			break;
		case 4:
			ItemIndex = GET_ITEM(11, 109);
			break;
		case 5:
			ItemIndex = GET_ITEM(11, 110);
			break;
		case 6:
			ItemIndex = GET_ITEM(11, 111);
			break;
		case 7:
			ItemIndex = GET_ITEM(11, 112);
			break;
		}

		gItemOptionRate.GetItemOption2(8, &ItemOption2);

		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, ItemOption2, 0, -1, 0, 5, 0, 0, 0, 0xFF, 0);

		gLog.Output(LOG_CHAOS_MIX, "[AwakeningSetMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[AwakeningSetMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::AwakeningEnergyElfHelmMix(LPOBJ lpObj) // OK
{
	int AwakeningSoulCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int ChaosCount = 0;
	int CreationCount = 0;
	int AwakeningType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 415))
		{
			AwakeningSoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 30))
		{
			BlessCount++;
		}
	
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 31))
		{
			SoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 15))
		{
			ChaosCount++;
		}
		
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 22))
		{
			CreationCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(7, 160))
		{
			AwakeningType++;
		}
	}

	if ((AwakeningSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1) && (AwakeningType != 1))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_AwakeningSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_AwakeningSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_AwakeningSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 750000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;
		
		gItemOptionRate.GetItemOption2(8, &ItemOption2);

		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, GET_ITEM(7, 114), 0, 0, 1, ItemOption2, 0, -1, 0, 5, 0, 0, 0, 0xFF, 0);

		gLog.Output(LOG_CHAOS_MIX,"[AwakeningSetMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[AwakeningSetMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::AwakeningEnergyElfArmorMix(LPOBJ lpObj) // OK
{
	int AwakeningSoulCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int ChaosCount = 0;
	int CreationCount = 0;
	int AwakeningType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 415))
		{
			AwakeningSoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 30))
		{
			BlessCount++;
		}
	
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 31))
		{
			SoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 15))
		{
			ChaosCount++;
		}
		
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 22))
		{
			CreationCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(8, 160))
		{
			AwakeningType++;
		}
	}

	if ((AwakeningSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1) && (AwakeningType != 1))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_AwakeningSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_AwakeningSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_AwakeningSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 750000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;
		
		gItemOptionRate.GetItemOption2(8, &ItemOption2);

		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, GET_ITEM(8, 115), 0, 0, 1, ItemOption2, 0, -1, 0, 5, 0, 0, 0, 0xFF, 0);

		gLog.Output(LOG_CHAOS_MIX,"[AwakeningSetMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[AwakeningSetMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::AwakeningEnergyElfPantsMix(LPOBJ lpObj) // OK
{
	int AwakeningSoulCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int ChaosCount = 0;
	int CreationCount = 0;
	int AwakeningType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 415))
		{
			AwakeningSoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 30))
		{
			BlessCount++;
		}
	
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 31))
		{
			SoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 15))
		{
			ChaosCount++;
		}
		
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 22))
		{
			CreationCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(9, 160))
		{
			AwakeningType++;
		}
	}

	if ((AwakeningSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1) && (AwakeningType != 1))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_AwakeningSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_AwakeningSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_AwakeningSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 750000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;
		
		gItemOptionRate.GetItemOption2(8, &ItemOption2);

		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, GET_ITEM(9, 114), 0, 0, 1, ItemOption2, 0, -1, 0, 5, 0, 0, 0, 0xFF, 0);

		gLog.Output(LOG_CHAOS_MIX,"[AwakeningSetMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[AwakeningSetMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::AwakeningEnergyElfGlovesMix(LPOBJ lpObj) // OK
{
	int AwakeningSoulCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int ChaosCount = 0;
	int CreationCount = 0;
	int AwakeningType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 415))
		{
			AwakeningSoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 30))
		{
			BlessCount++;
		}
	
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 31))
		{
			SoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 15))
		{
			ChaosCount++;
		}
		
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 22))
		{
			CreationCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(10, 160))
		{
			AwakeningType++;
		}
	}

	if ((AwakeningSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1) && (AwakeningType != 1))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_AwakeningSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_AwakeningSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_AwakeningSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 750000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;
		
		gItemOptionRate.GetItemOption2(8, &ItemOption2);

		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, GET_ITEM(10, 114), 0, 0, 1, ItemOption2, 0, -1, 0, 5, 0, 0, 0, 0xFF, 0);

		gLog.Output(LOG_CHAOS_MIX,"[AwakeningSetMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[AwakeningSetMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::AwakeningEnergyMagicArmorMix(LPOBJ lpObj) // OK
{
	int AwakeningSoulCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int ChaosCount = 0;
	int CreationCount = 0;
	int AwakeningType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 415))
		{
			AwakeningSoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 30))
		{
			BlessCount++;
		}
	
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 31))
		{
			SoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 15))
		{
			ChaosCount++;
		}
		
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 22))
		{
			CreationCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(8, 163))
		{
			AwakeningType++;
		}
	}

	if ((AwakeningSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1) && (AwakeningType != 1))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_AwakeningSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_AwakeningSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_AwakeningSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 750000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;
		
		gItemOptionRate.GetItemOption2(8, &ItemOption2);

		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, GET_ITEM(8, 115), 0, 0, 1, ItemOption2, 0, -1, 0, 5, 0, 0, 0, 0xFF, 0);

		gLog.Output(LOG_CHAOS_MIX,"[AwakeningSetMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[AwakeningSetMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::AwakeningEnergyMagicPantsMix(LPOBJ lpObj) // OK
{
	int AwakeningSoulCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int ChaosCount = 0;
	int CreationCount = 0;
	int AwakeningType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 415))
		{
			AwakeningSoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 30))
		{
			BlessCount++;
		}
	
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 31))
		{
			SoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 15))
		{
			ChaosCount++;
		}
		
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 22))
		{
			CreationCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(9, 163))
		{
			AwakeningType++;
		}
	}

	if ((AwakeningSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1) && (AwakeningType != 1))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_AwakeningSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_AwakeningSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_AwakeningSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 750000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;
		
		gItemOptionRate.GetItemOption2(8, &ItemOption2);

		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, GET_ITEM(9, 115), 0, 0, 1, ItemOption2, 0, -1, 0, 5, 0, 0, 0, 0xFF, 0);

		gLog.Output(LOG_CHAOS_MIX,"[AwakeningSetMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[AwakeningSetMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::AwakeningEnergyMagicGlovesMix(LPOBJ lpObj) // OK
{
	int AwakeningSoulCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int ChaosCount = 0;
	int CreationCount = 0;
	int AwakeningType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 415))
		{
			AwakeningSoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 30))
		{
			BlessCount++;
		}
	
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 31))
		{
			SoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 15))
		{
			ChaosCount++;
		}
		
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 22))
		{
			CreationCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(10, 163))
		{
			AwakeningType++;
		}
	}

	if ((AwakeningSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1) && (AwakeningType != 1))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_AwakeningSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_AwakeningSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_AwakeningSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 750000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;
		
		gItemOptionRate.GetItemOption2(8, &ItemOption2);

		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, GET_ITEM(10, 115), 0, 0, 1, ItemOption2, 0, -1, 0, 5, 0, 0, 0, 0xFF, 0);

		gLog.Output(LOG_CHAOS_MIX,"[AwakeningSetMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[AwakeningSetMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::AwakeningEnergyMagicBootsMix(LPOBJ lpObj) // OK
{
	int AwakeningSoulCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int ChaosCount = 0;
	int CreationCount = 0;
	int AwakeningType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 415))
		{
			AwakeningSoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 30))
		{
			BlessCount++;
		}
	
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 31))
		{
			SoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 15))
		{
			ChaosCount++;
		}
		
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 22))
		{
			CreationCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(11, 163))
		{
			AwakeningType++;
		}
	}

	if ((AwakeningSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1) && (AwakeningType != 1))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_AwakeningSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_AwakeningSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_AwakeningSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 750000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;
		
		gItemOptionRate.GetItemOption2(8, &ItemOption2);

		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, GET_ITEM(11, 115), 0, 0, 1, ItemOption2, 0, -1, 0, 5, 0, 0, 0, 0xFF, 0);

		gLog.Output(LOG_CHAOS_MIX,"[AwakeningSetMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[AwakeningSetMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::BlueEyeHelmMix(LPOBJ lpObj) // OK
{
	int FrostSpiritCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int ChaosCount = 0;
	int CreationCount = 0;
	int BlueEyeType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 449))
		{
			FrostSpiritCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 30))
		{
			BlessCount++;
		}
	
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 31))
		{
			SoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 15))
		{
			ChaosCount++;
		}
		
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 22))
		{
			CreationCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index >= GET_ITEM(7, 106) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(7, 115))
		{
			BlueEyeType = lpObj->ChaosBox[n].m_Index - GET_ITEM(7, 105);
		}
	}

	if (BlueEyeType == 0 || 
		(BlueEyeType == 1 && (FrostSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) ||
		(BlueEyeType == 2 && (FrostSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(BlueEyeType == 3 && (FrostSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(BlueEyeType == 4 && (FrostSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(BlueEyeType == 5 && (FrostSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(BlueEyeType == 6 && (FrostSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(BlueEyeType == 7 && (FrostSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(BlueEyeType == 8 && (FrostSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(BlueEyeType == 9 && (FrostSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(BlueEyeType == 10 && (FrostSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_BlueEyeSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_BlueEyeSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_BlueEyeSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 1000000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;

		switch (BlueEyeType)
		{
		case 1:
			ItemIndex = GET_ITEM(7, 164);
			break;
		case 2:
			ItemIndex = GET_ITEM(7, 165);
			break;
		case 3:
			ItemIndex = GET_ITEM(7, 166);
			break;
		case 4:
			ItemIndex = GET_ITEM(7, 167);
			break;
		case 5:
			ItemIndex = GET_ITEM(7, 168);
			break;
		case 6:
			ItemIndex = GET_ITEM(7, 169);
			break;
		case 7:
			ItemIndex = GET_ITEM(7, 170);
			break;
		case 8:
			ItemIndex = GET_ITEM(7, 171);
			break;
		case 9:
			ItemIndex = GET_ITEM(7, 172);
			break;
		case 10:
			ItemIndex = GET_ITEM(7, 173);
			break;
		}

		gItemOptionRate.GetItemOption2(8, &ItemOption2);

		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, ItemOption2, 0, -1, 0, 5, 0, 0, 0, 0xFF, 0);

		gLog.Output(LOG_CHAOS_MIX, "[BlueEyeSetMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[BlueEyeSetMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::BlueEyeArmorMix(LPOBJ lpObj) // OK
{
	int FrostSpiritCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int ChaosCount = 0;
	int CreationCount = 0;
	int BlueEyeType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 449))
		{
			FrostSpiritCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 30))
		{
			BlessCount++;
		}
	
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 31))
		{
			SoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 15))
		{
			ChaosCount++;
		}
		
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 22))
		{
			CreationCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index >= GET_ITEM(8, 106) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(8, 115))
		{
			BlueEyeType = lpObj->ChaosBox[n].m_Index - GET_ITEM(8, 105);
		}
	}

	if (BlueEyeType == 0 || 
		(BlueEyeType == 1 && (FrostSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) ||
		(BlueEyeType == 2 && (FrostSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(BlueEyeType == 3 && (FrostSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(BlueEyeType == 4 && (FrostSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(BlueEyeType == 5 && (FrostSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(BlueEyeType == 6 && (FrostSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(BlueEyeType == 7 && (FrostSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(BlueEyeType == 8 && (FrostSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(BlueEyeType == 9 && (FrostSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(BlueEyeType == 10 && (FrostSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_BlueEyeSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_BlueEyeSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_BlueEyeSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 1000000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;

		switch (BlueEyeType)
		{
		case 1:
			ItemIndex = GET_ITEM(8, 164);
			break;
		case 2:
			ItemIndex = GET_ITEM(8, 165);
			break;
		case 3:
			ItemIndex = GET_ITEM(8, 166);
			break;
		case 4:
			ItemIndex = GET_ITEM(8, 167);
			break;
		case 5:
			ItemIndex = GET_ITEM(8, 168);
			break;
		case 6:
			ItemIndex = GET_ITEM(8, 169);
			break;
		case 7:
			ItemIndex = GET_ITEM(8, 170);
			break;
		case 8:
			ItemIndex = GET_ITEM(8, 171);
			break;
		case 9:
			ItemIndex = GET_ITEM(8, 172);
			break;
		case 10:
			ItemIndex = GET_ITEM(8, 173);
			break;
		}

		gItemOptionRate.GetItemOption2(8, &ItemOption2);

		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, ItemOption2, 0, -1, 0, 5, 0, 0, 0, 0xFF, 0);

		gLog.Output(LOG_CHAOS_MIX, "[BlueEyeSetMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[BlueEyeSetMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::BlueEyePantsMix(LPOBJ lpObj) // OK
{
	int FrostSpiritCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int ChaosCount = 0;
	int CreationCount = 0;
	int BlueEyeType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 449))
		{
			FrostSpiritCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 30))
		{
			BlessCount++;
		}
	
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 31))
		{
			SoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 15))
		{
			ChaosCount++;
		}
		
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 22))
		{
			CreationCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index >= GET_ITEM(9, 106) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(9, 115))
		{
			BlueEyeType = lpObj->ChaosBox[n].m_Index - GET_ITEM(9, 105);
		}
	}

	if (BlueEyeType == 0 || 
		(BlueEyeType == 1 && (FrostSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) ||
		(BlueEyeType == 2 && (FrostSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(BlueEyeType == 3 && (FrostSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(BlueEyeType == 4 && (FrostSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(BlueEyeType == 5 && (FrostSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(BlueEyeType == 6 && (FrostSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(BlueEyeType == 7 && (FrostSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(BlueEyeType == 8 && (FrostSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(BlueEyeType == 9 && (FrostSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(BlueEyeType == 10 && (FrostSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_BlueEyeSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_BlueEyeSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_BlueEyeSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 1000000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;

		switch (BlueEyeType)
		{
		case 1:
			ItemIndex = GET_ITEM(9, 164);
			break;
		case 2:
			ItemIndex = GET_ITEM(9, 165);
			break;
		case 3:
			ItemIndex = GET_ITEM(9, 166);
			break;
		case 4:
			ItemIndex = GET_ITEM(9, 167);
			break;
		case 5:
			ItemIndex = GET_ITEM(9, 168);
			break;
		case 6:
			ItemIndex = GET_ITEM(9, 169);
			break;
		case 7:
			ItemIndex = GET_ITEM(9, 170);
			break;
		case 8:
			ItemIndex = GET_ITEM(9, 171);
			break;
		case 9:
			ItemIndex = GET_ITEM(9, 172);
			break;
		case 10:
			ItemIndex = GET_ITEM(9, 173);
			break;
		}

		gItemOptionRate.GetItemOption2(8, &ItemOption2);

		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, ItemOption2, 0, -1, 0, 5, 0, 0, 0, 0xFF, 0);

		gLog.Output(LOG_CHAOS_MIX, "[BlueEyeSetMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[BlueEyeSetMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::BlueEyeGlovesMix(LPOBJ lpObj) // OK
{
	int FrostSpiritCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int ChaosCount = 0;
	int CreationCount = 0;
	int BlueEyeType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 449))
		{
			FrostSpiritCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 30))
		{
			BlessCount++;
		}
	
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 31))
		{
			SoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 15))
		{
			ChaosCount++;
		}
		
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 22))
		{
			CreationCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index >= GET_ITEM(10, 106) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(10, 115))
		{
			BlueEyeType = lpObj->ChaosBox[n].m_Index - GET_ITEM(10, 105);
		}
	}

	if (BlueEyeType == 0 || 
		(BlueEyeType == 1 && (FrostSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) ||
		(BlueEyeType == 2 && (FrostSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(BlueEyeType == 3 && (FrostSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(BlueEyeType == 4 && (FrostSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(BlueEyeType == 5 && (FrostSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(BlueEyeType == 6 && (FrostSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(BlueEyeType == 7 && (FrostSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(BlueEyeType == 8 && (FrostSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(BlueEyeType == 9 && (FrostSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(BlueEyeType == 10 && (FrostSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_BlueEyeSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_BlueEyeSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_BlueEyeSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 1000000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;

		switch (BlueEyeType)
		{
		case 1:
			ItemIndex = GET_ITEM(10, 164);
			break;
		case 2:
			ItemIndex = GET_ITEM(10, 165);
			break;
		case 3:
			ItemIndex = GET_ITEM(10, 166);
			break;
		case 4:
			ItemIndex = GET_ITEM(10, 167);
			break;
		case 5:
			ItemIndex = GET_ITEM(10, 168);
			break;
		case 6:
			ItemIndex = GET_ITEM(10, 169);
			break;
		case 7:
			ItemIndex = GET_ITEM(10, 170);
			break;
		case 8:
			ItemIndex = GET_ITEM(10, 171);
			break;
		case 9:
			ItemIndex = GET_ITEM(10, 172);
			break;
		case 10:
			ItemIndex = GET_ITEM(10, 173);
			break;
		}

		gItemOptionRate.GetItemOption2(8, &ItemOption2);

		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, ItemOption2, 0, -1, 0, 5, 0, 0, 0, 0xFF, 0);

		gLog.Output(LOG_CHAOS_MIX, "[BlueEyeSetMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[BlueEyeSetMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::BlueEyeBootsMix(LPOBJ lpObj) // OK
{
	int FrostSpiritCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int ChaosCount = 0;
	int CreationCount = 0;
	int BlueEyeType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 449))
		{
			FrostSpiritCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 30))
		{
			BlessCount++;
		}
	
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 31))
		{
			SoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 15))
		{
			ChaosCount++;
		}
		
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 22))
		{
			CreationCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index >= GET_ITEM(11, 106) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(11, 115))
		{
			BlueEyeType = lpObj->ChaosBox[n].m_Index - GET_ITEM(11, 105);
		}
	}

	if (BlueEyeType == 0 || 
		(BlueEyeType == 1 && (FrostSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) ||
		(BlueEyeType == 2 && (FrostSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(BlueEyeType == 3 && (FrostSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(BlueEyeType == 4 && (FrostSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(BlueEyeType == 5 && (FrostSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(BlueEyeType == 6 && (FrostSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(BlueEyeType == 7 && (FrostSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_BlueEyeSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_BlueEyeSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_BlueEyeSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 1000000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;

		switch (BlueEyeType)
		{
		case 1:
			ItemIndex = GET_ITEM(11, 164);
			break;
		case 2:
			ItemIndex = GET_ITEM(11, 165);
			break;
		case 3:
			ItemIndex = GET_ITEM(11, 166);
			break;
		case 4:
			ItemIndex = GET_ITEM(11, 167);
			break;
		case 5:
			ItemIndex = GET_ITEM(11, 168);
			break;
		case 6:
			ItemIndex = GET_ITEM(11, 169);
			break;
		case 7:
			ItemIndex = GET_ITEM(11, 170);
			break;
		case 8:
			ItemIndex = GET_ITEM(11, 171);
			break;
		case 9:
			ItemIndex = GET_ITEM(11, 172);
			break;
		case 10:
			ItemIndex = GET_ITEM(11, 173);
			break;
		}

		gItemOptionRate.GetItemOption2(8, &ItemOption2);

		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, ItemOption2, 0, -1, 0, 5, 0, 0, 0, 0xFF, 0);

		gLog.Output(LOG_CHAOS_MIX, "[BlueEyeSetMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[BlueEyeSetMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::FleetSilverHeartHelmMix(LPOBJ lpObj) // OK
{
	int AncestralSpiritCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int ChaosCount = 0;
	int CreationCount = 0;
	int FleetSilverHeartType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 494))
		{
			AncestralSpiritCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 30))
		{
			BlessCount++;
		}
	
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 31))
		{
			SoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 15))
		{
			ChaosCount++;
		}
		
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 22))
		{
			CreationCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index >= GET_ITEM(7, 164) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(7, 173))
		{
			FleetSilverHeartType = lpObj->ChaosBox[n].m_Index - GET_ITEM(7, 163);
		}
	}

	if (FleetSilverHeartType == 0 || 
		(FleetSilverHeartType == 1 && (AncestralSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) ||
		(FleetSilverHeartType == 2 && (AncestralSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(FleetSilverHeartType == 3 && (AncestralSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(FleetSilverHeartType == 4 && (AncestralSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(FleetSilverHeartType == 5 && (AncestralSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(FleetSilverHeartType == 6 && (AncestralSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(FleetSilverHeartType == 7 && (AncestralSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(FleetSilverHeartType == 8 && (AncestralSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(FleetSilverHeartType == 9 && (AncestralSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(FleetSilverHeartType == 10 && (AncestralSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_FleetSilverHeartSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_FleetSilverHeartSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_FleetSilverHeartSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 1000000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;

		switch (FleetSilverHeartType)
		{
		case 1:
			ItemIndex = GET_ITEM(7, 189);
			break;
		case 2:
			ItemIndex = GET_ITEM(7, 190);
			break;
		case 3:
			ItemIndex = GET_ITEM(7, 191);
			break;
		case 4:
			ItemIndex = GET_ITEM(7, 192);
			break;
		case 5:
			ItemIndex = GET_ITEM(7, 193);
			break;
		case 6:
			ItemIndex = GET_ITEM(7, 194);
			break;
		case 7:
			ItemIndex = GET_ITEM(7, 195);
			break;
		case 8:
			ItemIndex = GET_ITEM(7, 196);
			break;
		case 9:
			ItemIndex = GET_ITEM(7, 197);
			break;
		case 10:
			ItemIndex = GET_ITEM(7, 198);
			break;
		}

		gItemOptionRate.GetItemOption2(8, &ItemOption2);

		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, ItemOption2, 0, -1, 0, 5, 0, 0, 0, 0xFF, 0);

		gLog.Output(LOG_CHAOS_MIX, "[FleetSilverHeartSetMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[FleetSilverHeartSetMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::FleetSilverHeartArmorMix(LPOBJ lpObj) // OK
{
	int AncestralSpiritCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int ChaosCount = 0;
	int CreationCount = 0;
	int FleetSilverHeartType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 494))
		{
			AncestralSpiritCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 30))
		{
			BlessCount++;
		}
	
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 31))
		{
			SoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 15))
		{
			ChaosCount++;
		}
		
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 22))
		{
			CreationCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index >= GET_ITEM(8, 164) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(8, 173))
		{
			FleetSilverHeartType = lpObj->ChaosBox[n].m_Index - GET_ITEM(8, 163);
		}
	}

	if (FleetSilverHeartType == 0 || 
		(FleetSilverHeartType == 1 && (AncestralSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) ||
		(FleetSilverHeartType == 2 && (AncestralSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(FleetSilverHeartType == 3 && (AncestralSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(FleetSilverHeartType == 4 && (AncestralSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(FleetSilverHeartType == 5 && (AncestralSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(FleetSilverHeartType == 6 && (AncestralSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(FleetSilverHeartType == 7 && (AncestralSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(FleetSilverHeartType == 8 && (AncestralSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(FleetSilverHeartType == 9 && (AncestralSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(FleetSilverHeartType == 10 && (AncestralSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_FleetSilverHeartSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_FleetSilverHeartSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_FleetSilverHeartSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 1000000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;

		switch (FleetSilverHeartType)
		{
		case 1:
			ItemIndex = GET_ITEM(8, 189);
			break;
		case 2:
			ItemIndex = GET_ITEM(8, 190);
			break;
		case 3:
			ItemIndex = GET_ITEM(8, 191);
			break;
		case 4:
			ItemIndex = GET_ITEM(8, 192);
			break;
		case 5:
			ItemIndex = GET_ITEM(8, 193);
			break;
		case 6:
			ItemIndex = GET_ITEM(8, 194);
			break;
		case 7:
			ItemIndex = GET_ITEM(8, 195);
			break;
		case 8:
			ItemIndex = GET_ITEM(8, 196);
			break;
		case 9:
			ItemIndex = GET_ITEM(8, 197);
			break;
		case 10:
			ItemIndex = GET_ITEM(8, 198);
			break;
		}

		gItemOptionRate.GetItemOption2(8, &ItemOption2);

		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, ItemOption2, 0, -1, 0, 5, 0, 0, 0, 0xFF, 0);

		gLog.Output(LOG_CHAOS_MIX, "[FleetSilverHeartSetMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[FleetSilverHeartSetMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::FleetSilverHeartPantsMix(LPOBJ lpObj) // OK
{
	int AncestralSpiritCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int ChaosCount = 0;
	int CreationCount = 0;
	int FleetSilverHeartType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 494))
		{
			AncestralSpiritCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 30))
		{
			BlessCount++;
		}
	
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 31))
		{
			SoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 15))
		{
			ChaosCount++;
		}
		
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 22))
		{
			CreationCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index >= GET_ITEM(9, 164) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(9, 173))
		{
			FleetSilverHeartType = lpObj->ChaosBox[n].m_Index - GET_ITEM(9, 163);
		}
	}

	if (FleetSilverHeartType == 0 || 
		(FleetSilverHeartType == 1 && (AncestralSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) ||
		(FleetSilverHeartType == 2 && (AncestralSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(FleetSilverHeartType == 3 && (AncestralSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(FleetSilverHeartType == 4 && (AncestralSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(FleetSilverHeartType == 5 && (AncestralSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(FleetSilverHeartType == 6 && (AncestralSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(FleetSilverHeartType == 7 && (AncestralSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(FleetSilverHeartType == 8 && (AncestralSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(FleetSilverHeartType == 9 && (AncestralSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(FleetSilverHeartType == 10 && (AncestralSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_FleetSilverHeartSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_FleetSilverHeartSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_FleetSilverHeartSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 1000000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;

		switch (FleetSilverHeartType)
		{
		case 1:
			ItemIndex = GET_ITEM(9, 189);
			break;
		case 2:
			ItemIndex = GET_ITEM(9, 190);
			break;
		case 3:
			ItemIndex = GET_ITEM(9, 191);
			break;
		case 4:
			ItemIndex = GET_ITEM(9, 192);
			break;
		case 5:
			ItemIndex = GET_ITEM(9, 193);
			break;
		case 6:
			ItemIndex = GET_ITEM(9, 194);
			break;
		case 7:
			ItemIndex = GET_ITEM(9, 195);
			break;
		case 8:
			ItemIndex = GET_ITEM(9, 196);
			break;
		case 9:
			ItemIndex = GET_ITEM(9, 197);
			break;
		case 10:
			ItemIndex = GET_ITEM(9, 198);
			break;
		}

		gItemOptionRate.GetItemOption2(8, &ItemOption2);

		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, ItemOption2, 0, -1, 0, 5, 0, 0, 0, 0xFF, 0);

		gLog.Output(LOG_CHAOS_MIX, "[FleetSilverHeartSetMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[FleetSilverHeartSetMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::FleetSilverHeartGlovesMix(LPOBJ lpObj) // OK
{
	int AncestralSpiritCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int ChaosCount = 0;
	int CreationCount = 0;
	int FleetSilverHeartType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 494))
		{
			AncestralSpiritCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 30))
		{
			BlessCount++;
		}
	
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 31))
		{
			SoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 15))
		{
			ChaosCount++;
		}
		
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 22))
		{
			CreationCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index >= GET_ITEM(10, 164) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(10, 173))
		{
			FleetSilverHeartType = lpObj->ChaosBox[n].m_Index - GET_ITEM(10, 163);
		}
	}

	if (FleetSilverHeartType == 0 || 
		(FleetSilverHeartType == 1 && (AncestralSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) ||
		(FleetSilverHeartType == 2 && (AncestralSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(FleetSilverHeartType == 3 && (AncestralSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(FleetSilverHeartType == 4 && (AncestralSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(FleetSilverHeartType == 5 && (AncestralSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(FleetSilverHeartType == 6 && (AncestralSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(FleetSilverHeartType == 7 && (AncestralSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(FleetSilverHeartType == 8 && (AncestralSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(FleetSilverHeartType == 9 && (AncestralSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(FleetSilverHeartType == 10 && (AncestralSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_FleetSilverHeartSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_FleetSilverHeartSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_FleetSilverHeartSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 1000000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;

		switch (FleetSilverHeartType)
		{
		case 1:
			ItemIndex = GET_ITEM(10, 189);
			break;
		case 2:
			ItemIndex = GET_ITEM(10, 190);
			break;
		case 3:
			ItemIndex = GET_ITEM(10, 191);
			break;
		case 4:
			ItemIndex = GET_ITEM(10, 192);
			break;
		case 5:
			ItemIndex = GET_ITEM(10, 193);
			break;
		case 6:
			ItemIndex = GET_ITEM(10, 194);
			break;
		case 7:
			ItemIndex = GET_ITEM(10, 195);
			break;
		case 8:
			ItemIndex = GET_ITEM(10, 196);
			break;
		case 9:
			ItemIndex = GET_ITEM(10, 197);
			break;
		case 10:
			ItemIndex = GET_ITEM(10, 198);
			break;
		}

		gItemOptionRate.GetItemOption2(8, &ItemOption2);

		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, ItemOption2, 0, -1, 0, 5, 0, 0, 0, 0xFF, 0);

		gLog.Output(LOG_CHAOS_MIX, "[FleetSilverHeartSetMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[FleetSilverHeartSetMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::FleetSilverHeartBootsMix(LPOBJ lpObj) // OK
{
	int AncestralSpiritCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int ChaosCount = 0;
	int CreationCount = 0;
	int FleetSilverHeartType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 494))
		{
			AncestralSpiritCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 30))
		{
			BlessCount++;
		}
	
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 31))
		{
			SoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 15))
		{
			ChaosCount++;
		}
		
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 22))
		{
			CreationCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index >= GET_ITEM(11, 164) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(11, 173))
		{
			FleetSilverHeartType = lpObj->ChaosBox[n].m_Index - GET_ITEM(11, 163);
		}
	}

	if (FleetSilverHeartType == 0 || 
		(FleetSilverHeartType == 1 && (AncestralSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) ||
		(FleetSilverHeartType == 2 && (AncestralSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(FleetSilverHeartType == 3 && (AncestralSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(FleetSilverHeartType == 4 && (AncestralSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(FleetSilverHeartType == 5 && (AncestralSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(FleetSilverHeartType == 6 && (AncestralSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(FleetSilverHeartType == 7 && (AncestralSpiritCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_FleetSilverHeartSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_FleetSilverHeartSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_FleetSilverHeartSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 1000000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;

		switch (FleetSilverHeartType)
		{
		case 1:
			ItemIndex = GET_ITEM(11, 189);
			break;
		case 2:
			ItemIndex = GET_ITEM(11, 190);
			break;
		case 3:
			ItemIndex = GET_ITEM(11, 191);
			break;
		case 4:
			ItemIndex = GET_ITEM(11, 192);
			break;
		case 5:
			ItemIndex = GET_ITEM(11, 193);
			break;
		case 6:
			ItemIndex = GET_ITEM(11, 194);
			break;
		case 7:
			ItemIndex = GET_ITEM(11, 195);
			break;
		case 8:
			ItemIndex = GET_ITEM(11, 196);
			break;
		case 9:
			ItemIndex = GET_ITEM(11, 197);
			break;
		case 10:
			ItemIndex = GET_ITEM(11, 198);
			break;
		}

		gItemOptionRate.GetItemOption2(8, &ItemOption2);

		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, ItemOption2, 0, -1, 0, 5, 0, 0, 0, 0xFF, 0);

		gLog.Output(LOG_CHAOS_MIX, "[FleetSilverHeartSetMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[FleetSilverHeartSetMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::RoaringManticoreHelmMix(LPOBJ lpObj) // OK
{
	int ManticoreSoulCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int ChaosCount = 0;
	int CreationCount = 0;
	int RoaringManticoreType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 501))
		{
			ManticoreSoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 30))
		{
			BlessCount++;
		}
	
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 31))
		{
			SoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 15))
		{
			ChaosCount++;
		}
		
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 22))
		{
			CreationCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index >= GET_ITEM(7, 189) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(7, 198))
		{
			RoaringManticoreType = lpObj->ChaosBox[n].m_Index - GET_ITEM(7, 188);
		}
	}

	if (RoaringManticoreType == 0 || 
		(RoaringManticoreType == 1 && (ManticoreSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) ||
		(RoaringManticoreType == 2 && (ManticoreSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(RoaringManticoreType == 3 && (ManticoreSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(RoaringManticoreType == 4 && (ManticoreSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(RoaringManticoreType == 5 && (ManticoreSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(RoaringManticoreType == 6 && (ManticoreSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(RoaringManticoreType == 7 && (ManticoreSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(RoaringManticoreType == 8 && (ManticoreSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(RoaringManticoreType == 9 && (ManticoreSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(RoaringManticoreType == 10 && (ManticoreSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_RoaringManticoreSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_RoaringManticoreSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_RoaringManticoreSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 1000000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;

		switch (RoaringManticoreType)
		{
		case 1:
			ItemIndex = GET_ITEM(7, 221);
			break;
		case 2:
			ItemIndex = GET_ITEM(7, 222);
			break;
		case 3:
			ItemIndex = GET_ITEM(7, 223);
			break;
		case 4:
			ItemIndex = GET_ITEM(7, 224);
			break;
		case 5:
			ItemIndex = GET_ITEM(7, 225);
			break;
		case 6:
			ItemIndex = GET_ITEM(7, 226);
			break;
		case 7:
			ItemIndex = GET_ITEM(7, 227);
			break;
		case 8:
			ItemIndex = GET_ITEM(7, 228);
			break;
		case 9:
			ItemIndex = GET_ITEM(7, 229);
			break;
		case 10:
			ItemIndex = GET_ITEM(7, 230);
			break;
		}

		gItemOptionRate.GetItemOption2(8, &ItemOption2);

		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, ItemOption2, 0, -1, 0, 5, 0, 0, 0, 0xFF, 0);

		gLog.Output(LOG_CHAOS_MIX, "[RoaringManticoreSetMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[RoaringManticoreSetMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::RoaringManticoreArmorMix(LPOBJ lpObj) // OK
{
	int ManticoreSoulCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int ChaosCount = 0;
	int CreationCount = 0;
	int RoaringManticoreType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 501))
		{
			ManticoreSoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 30))
		{
			BlessCount++;
		}
	
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 31))
		{
			SoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 15))
		{
			ChaosCount++;
		}
		
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 22))
		{
			CreationCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index >= GET_ITEM(8, 189) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(8, 198))
		{
			RoaringManticoreType = lpObj->ChaosBox[n].m_Index - GET_ITEM(8, 188);
		}
	}

	if (RoaringManticoreType == 0 || 
		(RoaringManticoreType == 1 && (ManticoreSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) ||
		(RoaringManticoreType == 2 && (ManticoreSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(RoaringManticoreType == 3 && (ManticoreSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(RoaringManticoreType == 4 && (ManticoreSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(RoaringManticoreType == 5 && (ManticoreSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(RoaringManticoreType == 6 && (ManticoreSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(RoaringManticoreType == 7 && (ManticoreSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(RoaringManticoreType == 8 && (ManticoreSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(RoaringManticoreType == 9 && (ManticoreSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(RoaringManticoreType == 10 && (ManticoreSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_RoaringManticoreSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_RoaringManticoreSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_RoaringManticoreSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 1000000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;

		switch (RoaringManticoreType)
		{
		case 1:
			ItemIndex = GET_ITEM(8, 221);
			break;
		case 2:
			ItemIndex = GET_ITEM(8, 222);
			break;
		case 3:
			ItemIndex = GET_ITEM(8, 223);
			break;
		case 4:
			ItemIndex = GET_ITEM(8, 224);
			break;
		case 5:
			ItemIndex = GET_ITEM(8, 225);
			break;
		case 6:
			ItemIndex = GET_ITEM(8, 226);
			break;
		case 7:
			ItemIndex = GET_ITEM(8, 227);
			break;
		case 8:
			ItemIndex = GET_ITEM(8, 228);
			break;
		case 9:
			ItemIndex = GET_ITEM(8, 229);
			break;
		case 10:
			ItemIndex = GET_ITEM(8, 230);
			break;
		}

		gItemOptionRate.GetItemOption2(8, &ItemOption2);

		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, ItemOption2, 0, -1, 0, 5, 0, 0, 0, 0xFF, 0);

		gLog.Output(LOG_CHAOS_MIX, "[RoaringManticoreSetMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[RoaringManticoreSetMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::RoaringManticorePantsMix(LPOBJ lpObj) // OK
{
	int ManticoreSoulCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int ChaosCount = 0;
	int CreationCount = 0;
	int RoaringManticoreType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 501))
		{
			ManticoreSoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 30))
		{
			BlessCount++;
		}
	
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 31))
		{
			SoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 15))
		{
			ChaosCount++;
		}
		
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 22))
		{
			CreationCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index >= GET_ITEM(9, 189) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(9, 198))
		{
			RoaringManticoreType = lpObj->ChaosBox[n].m_Index - GET_ITEM(9, 188);
		}
	}

	if (RoaringManticoreType == 0 || 
		(RoaringManticoreType == 1 && (ManticoreSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) ||
		(RoaringManticoreType == 2 && (ManticoreSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(RoaringManticoreType == 3 && (ManticoreSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(RoaringManticoreType == 4 && (ManticoreSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(RoaringManticoreType == 5 && (ManticoreSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(RoaringManticoreType == 6 && (ManticoreSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(RoaringManticoreType == 7 && (ManticoreSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(RoaringManticoreType == 8 && (ManticoreSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(RoaringManticoreType == 9 && (ManticoreSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(RoaringManticoreType == 10 && (ManticoreSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_RoaringManticoreSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_RoaringManticoreSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_RoaringManticoreSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 1000000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;

		switch (RoaringManticoreType)
		{
		case 1:
			ItemIndex = GET_ITEM(9, 221);
			break;
		case 2:
			ItemIndex = GET_ITEM(9, 222);
			break;
		case 3:
			ItemIndex = GET_ITEM(9, 223);
			break;
		case 4:
			ItemIndex = GET_ITEM(9, 224);
			break;
		case 5:
			ItemIndex = GET_ITEM(9, 225);
			break;
		case 6:
			ItemIndex = GET_ITEM(9, 226);
			break;
		case 7:
			ItemIndex = GET_ITEM(9, 227);
			break;
		case 8:
			ItemIndex = GET_ITEM(9, 228);
			break;
		case 9:
			ItemIndex = GET_ITEM(9, 229);
			break;
		case 10:
			ItemIndex = GET_ITEM(9, 230);
			break;
		}

		gItemOptionRate.GetItemOption2(8, &ItemOption2);

		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, ItemOption2, 0, -1, 0, 5, 0, 0, 0, 0xFF, 0);

		gLog.Output(LOG_CHAOS_MIX, "[RoaringManticoreSetMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[RoaringManticoreSetMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::RoaringManticoreGlovesMix(LPOBJ lpObj) // OK
{
	int ManticoreSoulCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int ChaosCount = 0;
	int CreationCount = 0;
	int RoaringManticoreType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 501))
		{
			ManticoreSoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 30))
		{
			BlessCount++;
		}
	
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 31))
		{
			SoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 15))
		{
			ChaosCount++;
		}
		
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 22))
		{
			CreationCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index >= GET_ITEM(10, 189) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(10, 198))
		{
			RoaringManticoreType = lpObj->ChaosBox[n].m_Index - GET_ITEM(10, 188);
		}
	}

	if (RoaringManticoreType == 0 || 
		(RoaringManticoreType == 1 && (ManticoreSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) ||
		(RoaringManticoreType == 2 && (ManticoreSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(RoaringManticoreType == 3 && (ManticoreSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(RoaringManticoreType == 4 && (ManticoreSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(RoaringManticoreType == 5 && (ManticoreSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(RoaringManticoreType == 6 && (ManticoreSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(RoaringManticoreType == 7 && (ManticoreSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(RoaringManticoreType == 8 && (ManticoreSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(RoaringManticoreType == 9 && (ManticoreSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(RoaringManticoreType == 10 && (ManticoreSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_RoaringManticoreSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_RoaringManticoreSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_RoaringManticoreSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 1000000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;

		switch (RoaringManticoreType)
		{
		case 1:
			ItemIndex = GET_ITEM(10, 221);
			break;
		case 2:
			ItemIndex = GET_ITEM(10, 222);
			break;
		case 3:
			ItemIndex = GET_ITEM(10, 223);
			break;
		case 4:
			ItemIndex = GET_ITEM(10, 224);
			break;
		case 5:
			ItemIndex = GET_ITEM(10, 225);
			break;
		case 6:
			ItemIndex = GET_ITEM(10, 226);
			break;
		case 7:
			ItemIndex = GET_ITEM(10, 227);
			break;
		case 8:
			ItemIndex = GET_ITEM(10, 228);
			break;
		case 9:
			ItemIndex = GET_ITEM(10, 229);
			break;
		case 10:
			ItemIndex = GET_ITEM(10, 230);
			break;
		}

		gItemOptionRate.GetItemOption2(8, &ItemOption2);

		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, ItemOption2, 0, -1, 0, 5, 0, 0, 0, 0xFF, 0);

		gLog.Output(LOG_CHAOS_MIX, "[RoaringManticoreSetMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[RoaringManticoreSetMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::RoaringManticoreBootsMix(LPOBJ lpObj) // OK
{
	int ManticoreSoulCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int ChaosCount = 0;
	int CreationCount = 0;
	int RoaringManticoreType = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 501))
		{
			ManticoreSoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 30))
		{
			BlessCount++;
		}
	
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 31))
		{
			SoulCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 15))
		{
			ChaosCount++;
		}
		
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 22))
		{
			CreationCount++;
		}

		else if (lpObj->ChaosBox[n].m_Index >= GET_ITEM(11, 189) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(11, 198))
		{
			RoaringManticoreType = lpObj->ChaosBox[n].m_Index - GET_ITEM(11, 188);
		}
	}

	if (RoaringManticoreType == 0 || 
		(RoaringManticoreType == 1 && (ManticoreSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) ||
		(RoaringManticoreType == 2 && (ManticoreSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(RoaringManticoreType == 3 && (ManticoreSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(RoaringManticoreType == 4 && (ManticoreSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(RoaringManticoreType == 5 && (ManticoreSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(RoaringManticoreType == 6 && (ManticoreSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)) || 
		(RoaringManticoreType == 7 && (ManticoreSoulCount != 1) && (BlessCount != 1)  && (SoulCount != 1) && (ChaosCount != 1) && (CreationCount != 1)))
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_RoaringManticoreSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ItemMoney ;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_RoaringManticoreSetMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_RoaringManticoreSetMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40) ? 40 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 1000000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;

		switch (RoaringManticoreType)
		{
		case 1:
			ItemIndex = GET_ITEM(11, 221);
			break;
		case 2:
			ItemIndex = GET_ITEM(11, 222);
			break;
		case 3:
			ItemIndex = GET_ITEM(11, 223);
			break;
		case 4:
			ItemIndex = GET_ITEM(11, 224);
			break;
		case 5:
			ItemIndex = GET_ITEM(11, 225);
			break;
		case 6:
			ItemIndex = GET_ITEM(11, 226);
			break;
		case 7:
			ItemIndex = GET_ITEM(11, 227);
			break;
		case 8:
			ItemIndex = GET_ITEM(11, 228);
			break;
		case 9:
			ItemIndex = GET_ITEM(11, 229);
			break;
		case 10:
			ItemIndex = GET_ITEM(11, 230);
			break;
		}

		gItemOptionRate.GetItemOption2(8, &ItemOption2);

		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, ItemOption2, 0, -1, 0, 5, 0, 0, 0, 0xFF, 0);

		gLog.Output(LOG_CHAOS_MIX, "[RoaringManticoreSetMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[RoaringManticoreSetMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::HammerArchangelMix(LPOBJ lpObj, int type) // OK
{
	int ItemCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int ChaosCount = 0;
	int CreationCount = 0;
	int GuardianCount = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 30))
		{
			BlessCount++;
		}
		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 31))
		{
			SoulCount++;
		}
		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 15))
		{
			ChaosCount++;
		}
		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 22))
		{
			CreationCount++;
		}
		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 31))
		{
			GuardianCount++;
		}
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 342))
		{
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
	}

	if (ChaosCount != 1 || (type == 0 && BlessCount != 1) || (type == 0 && SoulCount != 1) || (type == 0 && GuardianCount != 1) || (type == 0 && ItemCount != 1) || CreationCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_HammerArchangelMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney / 4000000) + (ItemMoney / 40000);
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_HammerArchangelMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_HammerArchangelMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>90) ? 90 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 100000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		if (type == 0)
		{
			WORD ItemIndex = 0;

			CRandomManager RandomManager;

			RandomManager.AddElement(GET_ITEM(14, 343), 1);

			RandomManager.GetRandomElement(&ItemIndex);

			GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0xFF, 0);

			gLog.Output(LOG_CHAOS_MIX, "[HammerArchangelSoulMix][Success][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
		}
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[HammerArchangelMix][Failure][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}


void CChaosBox::AbsoluteArchangelSwordMix(LPOBJ lpObj, int type) // OK
{
	int ChaosCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int LifeCount = 0;
	int WeaponCount = 0;
	int ItemCount = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 15))
		{
			ChaosCount++;
		}
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 30) && lpObj->ChaosBox[n].m_Level == 2)
		{
			BlessCount++;
		}
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 31) && lpObj->ChaosBox[n].m_Level == 2)
		{
			SoulCount++;
		}
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 136) && lpObj->ChaosBox[n].m_Level == 2)
		{
			LifeCount++;
		}
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(0, 19) && lpObj->ChaosBox[n].m_Level == 15)
		{
			WeaponCount++;
		}
		else if ((lpObj->ChaosBox[n].m_Index >= GET_ITEM(14, 343)))
		{
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
	}

	if (ChaosCount != 1 || BlessCount != 1 || SoulCount != 1 || LifeCount != 1 || WeaponCount != 1 || ItemCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_AbsoluteArchangelMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney / 4000000) + (ItemMoney / 40000);
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_AbsoluteArchangelMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_AbsoluteArchangelMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>90) ? 90 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 1000000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		if (type == 0)
		{
			WORD ItemIndex = 0;

			CRandomManager RandomManager;

			RandomManager.AddElement(GET_ITEM(0, 51), 1);

			RandomManager.GetRandomElement(&ItemIndex);

			GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, 1, 1, -1, 63, 0, 0, 0, 0, 0xFF, 0);

			gLog.Output(LOG_CHAOS_MIX, "[AbsoluteArchangelMix][Success][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
		}
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[AbsoluteArchangelMix][Failure][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::AbsoluteArchangelScepterMix(LPOBJ lpObj, int type) // OK
{
	int ChaosCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int LifeCount = 0;
	int WeaponCount = 0;
	int ItemCount = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 15))
		{
			ChaosCount++;
		}
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 30) && lpObj->ChaosBox[n].m_Level == 2)
		{
			BlessCount++;
		}
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 31) && lpObj->ChaosBox[n].m_Level == 2)
		{
			SoulCount++;
		}
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 136) && lpObj->ChaosBox[n].m_Level == 2)
		{
			LifeCount++;
		}
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(2, 13) && lpObj->ChaosBox[n].m_Level == 15)
		{
			WeaponCount++;
		}
		else if ((lpObj->ChaosBox[n].m_Index >= GET_ITEM(14, 343)))
		{
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
	}

	if (ChaosCount != 1 || BlessCount != 1 || SoulCount != 1 || LifeCount != 1 || WeaponCount != 1 || ItemCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_AbsoluteArchangelMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney / 4000000) + (ItemMoney / 40000);
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_AbsoluteArchangelMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_AbsoluteArchangelMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>90) ? 90 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 1000000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		if (type == 0)
		{
			WORD ItemIndex = 0;

			CRandomManager RandomManager;

			RandomManager.AddElement(GET_ITEM(2, 25), 1);

			RandomManager.GetRandomElement(&ItemIndex);

			GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, 1, 1, -1, 63, 0, 0, 0, 0, 0xFF, 0);

			gLog.Output(LOG_CHAOS_MIX, "[AbsoluteArchangelMix][Success][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
		}
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[AbsoluteArchangelMix][Failure][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::AbsoluteArchangelCrossbowMix(LPOBJ lpObj, int type) // OK
{
	int ChaosCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int LifeCount = 0;
	int WeaponCount = 0;
	int ItemCount = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 15))
		{
			ChaosCount++;
		}
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 30) && lpObj->ChaosBox[n].m_Level == 2)
		{
			BlessCount++;
		}
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 31) && lpObj->ChaosBox[n].m_Level == 2)
		{
			SoulCount++;
		}
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 136) && lpObj->ChaosBox[n].m_Level == 2)
		{
			LifeCount++;
		}
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(4, 18) && lpObj->ChaosBox[n].m_Level == 15)
		{
			WeaponCount++;
		}
		else if ((lpObj->ChaosBox[n].m_Index >= GET_ITEM(14, 343)))
		{
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
	}

	if (ChaosCount != 1 || BlessCount != 1 || SoulCount != 1 || LifeCount != 1 || WeaponCount != 1 || ItemCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_AbsoluteArchangelMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney / 4000000) + (ItemMoney / 40000);
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_AbsoluteArchangelMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_AbsoluteArchangelMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>90) ? 90 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 1000000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		if (type == 0)
		{
			WORD ItemIndex = 0;

			CRandomManager RandomManager;

			RandomManager.AddElement(GET_ITEM(4, 30), 1);

			RandomManager.GetRandomElement(&ItemIndex);

			GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, 1, 1, -1, 63, 0, 0, 0, 0, 0xFF, 0);

			gLog.Output(LOG_CHAOS_MIX, "[AbsoluteArchangelMix][Success][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
		}
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[AbsoluteArchangelMix][Failure][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::AbsoluteArchangelStaffMix(LPOBJ lpObj, int type) // OK
{
	int ChaosCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int LifeCount = 0;
	int WeaponCount = 0;
	int ItemCount = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 15))
		{
			ChaosCount++;
		}
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 30) && lpObj->ChaosBox[n].m_Level == 2)
		{
			BlessCount++;
		}
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 31) && lpObj->ChaosBox[n].m_Level == 2)
		{
			SoulCount++;
		}
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 136) && lpObj->ChaosBox[n].m_Level == 2)
		{
			LifeCount++;
		}
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(5, 10) && lpObj->ChaosBox[n].m_Level == 15)
		{
			WeaponCount++;
		}
		else if ((lpObj->ChaosBox[n].m_Index >= GET_ITEM(14, 343)))
		{
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
	}

	if (ChaosCount != 1 || BlessCount != 1 || SoulCount != 1 || LifeCount != 1 || WeaponCount != 1 || ItemCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_AbsoluteArchangelMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney / 4000000) + (ItemMoney / 40000);
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_AbsoluteArchangelMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_AbsoluteArchangelMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>90) ? 90 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 1000000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		if (type == 0)
		{
			WORD ItemIndex = 0;

			CRandomManager RandomManager;

			RandomManager.AddElement(GET_ITEM(5, 49), 1);

			RandomManager.GetRandomElement(&ItemIndex);

			GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, 1, 1, -1, 63, 0, 0, 0, 0, 0xFF, 0);

			gLog.Output(LOG_CHAOS_MIX, "[AbsoluteArchangelMix][Success][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
		}
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[AbsoluteArchangelMix][Failure][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::AbsoluteArchangelStickMix(LPOBJ lpObj, int type) // OK
{
	int ChaosCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int LifeCount = 0;
	int WeaponCount = 0;
	int ItemCount = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 15))
		{
			ChaosCount++;
		}
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 30) && lpObj->ChaosBox[n].m_Level == 2)
		{
			BlessCount++;
		}
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 31) && lpObj->ChaosBox[n].m_Level == 2)
		{
			SoulCount++;
		}
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 136) && lpObj->ChaosBox[n].m_Level == 2)
		{
			LifeCount++;
		}
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(5, 36) && lpObj->ChaosBox[n].m_Level == 15)
		{
			WeaponCount++;
		}
		else if ((lpObj->ChaosBox[n].m_Index >= GET_ITEM(14, 343)))
		{
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
	}

	if (ChaosCount != 1 || BlessCount != 1 || SoulCount != 1 || LifeCount != 1 || WeaponCount != 1 || ItemCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_AbsoluteArchangelMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney / 4000000) + (ItemMoney / 40000);
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_AbsoluteArchangelMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_AbsoluteArchangelMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>90) ? 90 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 1000000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		if (type == 0)
		{
			WORD ItemIndex = 0;

			CRandomManager RandomManager;

			RandomManager.AddElement(GET_ITEM(5, 50), 1);

			RandomManager.GetRandomElement(&ItemIndex);

			GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, 1, 1, -1, 63, 0, 0, 0, 0, 0xFF, 0);

			gLog.Output(LOG_CHAOS_MIX, "[AbsoluteArchangelMix][Success][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
		}
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[AbsoluteArchangelMix][Failure][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::AbsoluteArchangelClawMix(LPOBJ lpObj, int type) // OK
{
	int ChaosCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int LifeCount = 0;
	int WeaponCount = 0;
	int ItemCount = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 15))
		{
			ChaosCount++;
		}
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 30) && lpObj->ChaosBox[n].m_Level == 2)
		{
			BlessCount++;
		}
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 31) && lpObj->ChaosBox[n].m_Level == 2)
		{
			SoulCount++;
		}
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 136) && lpObj->ChaosBox[n].m_Level == 2)
		{
			LifeCount++;
		}
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(0, 60) && lpObj->ChaosBox[n].m_Level == 15)
		{
			WeaponCount++;
		}
		else if ((lpObj->ChaosBox[n].m_Index >= GET_ITEM(14, 343)))
		{
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
	}

	if (ChaosCount != 1 || BlessCount != 1 || SoulCount != 1 || LifeCount != 1 || WeaponCount != 1 || ItemCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_AbsoluteArchangelMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney / 4000000) + (ItemMoney / 40000);
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_AbsoluteArchangelMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_AbsoluteArchangelMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>90) ? 90 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 1000000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		if (type == 0)
		{
			WORD ItemIndex = 0;

			CRandomManager RandomManager;

			RandomManager.AddElement(GET_ITEM(0, 61), 1);

			RandomManager.GetRandomElement(&ItemIndex);

			GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, 1, 1, -1, 63, 0, 0, 0, 0, 0xFF, 0);

			gLog.Output(LOG_CHAOS_MIX, "[AbsoluteArchangelMix][Success][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
		}
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[AbsoluteArchangelMix][Failure][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::DarkangelSwordMix(LPOBJ lpObj, int type) // OK
{
	int WeaponCount = 0;
	int ItemCount = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(0, 42))
		{
			WeaponCount++;
		}
		else if ((lpObj->ChaosBox[n].m_Index >= GET_ITEM(14, 386)))
		{
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
	}

	if (WeaponCount != 1 || ItemCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_DarkangelWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney / 4000000) + (ItemMoney / 40000);
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_DarkangelWeaponMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_DarkangelWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>90) ? 90 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 250000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		if (type == 0)
		{
			WORD ItemIndex = 0;

			CRandomManager RandomManager;

			RandomManager.AddElement(GET_ITEM(0, 54), 1);

			RandomManager.GetRandomElement(&ItemIndex);

			GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, 1, 1, -1, 56, 0, 0, 0, 0, 0xFF, 0);

			gLog.Output(LOG_CHAOS_MIX, "[DarkangelWeaponMix][Success][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
		}
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[DarkangelWeaponMix][Failure][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::DarkangelMagicSwordMix(LPOBJ lpObj, int type) // OK
{
	int WeaponCount = 0;
	int ItemCount = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(0, 44))
		{
			WeaponCount++;
		}
		else if ((lpObj->ChaosBox[n].m_Index >= GET_ITEM(14, 386)))
		{
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
	}

	if (WeaponCount != 1 || ItemCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_DarkangelWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney / 4000000) + (ItemMoney / 40000);
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_DarkangelWeaponMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_DarkangelWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>90) ? 90 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 250000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		if (type == 0)
		{
			WORD ItemIndex = 0;

			CRandomManager RandomManager;

			RandomManager.AddElement(GET_ITEM(0, 55), 1);

			RandomManager.GetRandomElement(&ItemIndex);

			GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, 1, 1, -1, 56, 0, 0, 0, 0, 0xFF, 0);

			gLog.Output(LOG_CHAOS_MIX, "[DarkangelWeaponMix][Success][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
		}
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[DarkangelWeaponMix][Failure][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::DarkangelClawMix(LPOBJ lpObj, int type) // OK
{
	int WeaponCount = 0;
	int ItemCount = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(0, 46))
		{
			WeaponCount++;
		}
		else if ((lpObj->ChaosBox[n].m_Index >= GET_ITEM(14, 386)))
		{
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
	}

	if (WeaponCount != 1 || ItemCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_DarkangelWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney / 4000000) + (ItemMoney / 40000);
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_DarkangelWeaponMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_DarkangelWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>90) ? 90 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 250000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		if (type == 0)
		{
			WORD ItemIndex = 0;

			CRandomManager RandomManager;

			RandomManager.AddElement(GET_ITEM(0, 56), 1);

			RandomManager.GetRandomElement(&ItemIndex);

			GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, 1, 1, -1, 56, 0, 0, 0, 0, 0xFF, 0);

			gLog.Output(LOG_CHAOS_MIX, "[DarkangelWeaponMix][Success][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
		}
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[DarkangelWeaponMix][Failure][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::DarkangelScepterMix(LPOBJ lpObj, int type) // OK
{
	int WeaponCount = 0;
	int ItemCount = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(2, 22))
		{
			WeaponCount++;
		}
		else if ((lpObj->ChaosBox[n].m_Index >= GET_ITEM(14, 386)))
		{
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
	}

	if (WeaponCount != 1 || ItemCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_DarkangelWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney / 4000000) + (ItemMoney / 40000);
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_DarkangelWeaponMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_DarkangelWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>90) ? 90 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 250000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		if (type == 0)
		{
			WORD ItemIndex = 0;

			CRandomManager RandomManager;

			RandomManager.AddElement(GET_ITEM(2, 26), 1);

			RandomManager.GetRandomElement(&ItemIndex);

			GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, 1, 1, -1, 56, 0, 0, 0, 0, 0xFF, 0);

			gLog.Output(LOG_CHAOS_MIX, "[DarkangelWeaponMix][Success][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
		}
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[DarkangelWeaponMix][Failure][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::DarkangelBowMix(LPOBJ lpObj, int type) // OK
{
	int WeaponCount = 0;
	int ItemCount = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(4, 28))
		{
			WeaponCount++;
		}
		else if ((lpObj->ChaosBox[n].m_Index >= GET_ITEM(14, 386)))
		{
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
	}

	if (WeaponCount != 1 || ItemCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_DarkangelWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney / 4000000) + (ItemMoney / 40000);
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_DarkangelWeaponMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_DarkangelWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>90) ? 90 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 250000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		if (type == 0)
		{
			WORD ItemIndex = 0;

			CRandomManager RandomManager;

			RandomManager.AddElement(GET_ITEM(4, 31), 1);

			RandomManager.GetRandomElement(&ItemIndex);

			GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, 1, 1, -1, 56, 0, 0, 0, 0, 0xFF, 0);

			gLog.Output(LOG_CHAOS_MIX, "[DarkangelWeaponMix][Success][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
		}
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[DarkangelWeaponMix][Failure][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::DarkangelStaffMix(LPOBJ lpObj, int type) // OK
{
	int WeaponCount = 0;
	int ItemCount = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(5, 41))
		{
			WeaponCount++;
		}
		else if ((lpObj->ChaosBox[n].m_Index >= GET_ITEM(14, 386)))
		{
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
	}

	if (WeaponCount != 1 || ItemCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_DarkangelWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney / 4000000) + (ItemMoney / 40000);
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_DarkangelWeaponMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_DarkangelWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>90) ? 90 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 250000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		if (type == 0)
		{
			WORD ItemIndex = 0;

			CRandomManager RandomManager;

			RandomManager.AddElement(GET_ITEM(5, 51), 1);

			RandomManager.GetRandomElement(&ItemIndex);
					
			GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, 1, 1, -1, 56, 0, 0, 0, 0, 0xFF, 0);

			gLog.Output(LOG_CHAOS_MIX, "[DarkangelWeaponMix][Success][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
		}
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[DarkangelWeaponMix][Failure][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::DarkangelStickMix(LPOBJ lpObj, int type) // OK
{
	int WeaponCount = 0;
	int ItemCount = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(5, 43))
		{
			WeaponCount++;
		}
		else if ((lpObj->ChaosBox[n].m_Index >= GET_ITEM(14, 386)))
		{
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
	}

	if (WeaponCount != 1 || ItemCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_DarkangelWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney / 4000000) + (ItemMoney / 40000);
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_DarkangelWeaponMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_DarkangelWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>90) ? 90 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 250000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		if (type == 0)
		{
			WORD ItemIndex = 0;

			CRandomManager RandomManager;

			RandomManager.AddElement(GET_ITEM(5, 52), 1);

			RandomManager.GetRandomElement(&ItemIndex);

			GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, 1, 1, -1, 56, 0, 0, 0, 0, 0xFF, 0);

			gLog.Output(LOG_CHAOS_MIX, "[DarkangelWeaponMix][Success][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
		}
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[DarkangelWeaponMix][Failure][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::HolyangelSwordMix(LPOBJ lpObj, int type) // OK
{
	int WeaponCount = 0;
	int ItemCount = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(0, 54))
		{
			WeaponCount++;
		}
		else if ((lpObj->ChaosBox[n].m_Index >= GET_ITEM(14, 416)))
		{
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
	}

	if (WeaponCount != 1 || ItemCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_HolyangelWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney / 4000000) + (ItemMoney / 40000);
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_HolyangelWeaponMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_HolyangelWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>90) ? 90 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 500000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		if (type == 0)
		{
			WORD ItemIndex = 0;

			CRandomManager RandomManager;

			RandomManager.AddElement(GET_ITEM(0, 57), 1);

			RandomManager.GetRandomElement(&ItemIndex);

			GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, 1, 1, -1, 56, 0, 0, 0, 0, 0xFF, 0);

			gLog.Output(LOG_CHAOS_MIX, "[HolyangelWeaponMix][Success][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
		}
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[HolyangelWeaponMix][Failure][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::HolyangelMagicSwordMix(LPOBJ lpObj, int type) // OK
{
	int WeaponCount = 0;
	int ItemCount = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(0, 55))
		{
			WeaponCount++;
		}
		else if ((lpObj->ChaosBox[n].m_Index >= GET_ITEM(14, 416)))
		{
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
	}

	if (WeaponCount != 1 || ItemCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_HolyangelWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney / 4000000) + (ItemMoney / 40000);
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_HolyangelWeaponMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_HolyangelWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>90) ? 90 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 500000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		if (type == 0)
		{
			WORD ItemIndex = 0;

			CRandomManager RandomManager;

			RandomManager.AddElement(GET_ITEM(0, 58), 1);

			RandomManager.GetRandomElement(&ItemIndex);

			GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, 1, 1, -1, 56, 0, 0, 0, 0, 0xFF, 0);

			gLog.Output(LOG_CHAOS_MIX, "[HolyangelWeaponMix][Success][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
		}
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[HolyangelWeaponMix][Failure][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::HolyangelClawMix(LPOBJ lpObj, int type) // OK
{
	int WeaponCount = 0;
	int ItemCount = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(0, 56))
		{
			WeaponCount++;
		}
		else if ((lpObj->ChaosBox[n].m_Index >= GET_ITEM(14, 416)))
		{
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
	}

	if (WeaponCount != 1 || ItemCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_HolyangelWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney / 4000000) + (ItemMoney / 40000);
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_HolyangelWeaponMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_HolyangelWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>90) ? 90 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 500000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		if (type == 0)
		{
			WORD ItemIndex = 0;

			CRandomManager RandomManager;

			RandomManager.AddElement(GET_ITEM(0, 59), 1);

			RandomManager.GetRandomElement(&ItemIndex);

			GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, 1, 1, -1, 56, 0, 0, 0, 0, 0xFF, 0);

			gLog.Output(LOG_CHAOS_MIX, "[HolyangelWeaponMix][Success][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
		}
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[HolyangelWeaponMix][Failure][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::HolyangelScepterMix(LPOBJ lpObj, int type) // OK
{
	int WeaponCount = 0;
	int ItemCount = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(2, 26))
		{
			WeaponCount++;
		}
		else if ((lpObj->ChaosBox[n].m_Index >= GET_ITEM(14, 416)))
		{
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
	}

	if (WeaponCount != 1 || ItemCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_HolyangelWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney / 4000000) + (ItemMoney / 40000);
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_HolyangelWeaponMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_HolyangelWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>90) ? 90 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 500000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		if (type == 0)
		{
			WORD ItemIndex = 0;

			CRandomManager RandomManager;

			RandomManager.AddElement(GET_ITEM(2, 27), 1);

			RandomManager.GetRandomElement(&ItemIndex);

			GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, 1, 1, -1, 56, 0, 0, 0, 0, 0xFF, 0);

			gLog.Output(LOG_CHAOS_MIX, "[HolyangelWeaponMix][Success][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
		}
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[HolyangelWeaponMix][Failure][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::HolyangelBowMix(LPOBJ lpObj, int type) // OK
{
	int WeaponCount = 0;
	int ItemCount = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(4, 31))
		{
			WeaponCount++;
		}
		else if ((lpObj->ChaosBox[n].m_Index >= GET_ITEM(14, 416)))
		{
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
	}

	if (WeaponCount != 1 || ItemCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_HolyangelWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney / 4000000) + (ItemMoney / 40000);
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_HolyangelWeaponMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_HolyangelWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>90) ? 90 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 500000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		if (type == 0)
		{
			WORD ItemIndex = 0;

			CRandomManager RandomManager;

			RandomManager.AddElement(GET_ITEM(4, 36), 1);

			RandomManager.GetRandomElement(&ItemIndex);

			GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, 1, 1, -1, 56, 0, 0, 0, 0, 0xFF, 0);

			gLog.Output(LOG_CHAOS_MIX, "[HolyangelWeaponMix][Success][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
		}
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[HolyangelWeaponMix][Failure][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::HolyangelStaffMix(LPOBJ lpObj, int type) // OK
{
	int WeaponCount = 0;
	int ItemCount = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(5, 51))
		{
			WeaponCount++;
		}
		else if ((lpObj->ChaosBox[n].m_Index >= GET_ITEM(14, 416)))
		{
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
	}

	if (WeaponCount != 1 || ItemCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_HolyangelWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney / 4000000) + (ItemMoney / 40000);
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_HolyangelWeaponMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_HolyangelWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>90) ? 90 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 500000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		if (type == 0)
		{
			WORD ItemIndex = 0;

			CRandomManager RandomManager;

			RandomManager.AddElement(GET_ITEM(5, 53), 1);

			RandomManager.GetRandomElement(&ItemIndex);
					
			GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, 1, 1, -1, 56, 0, 0, 0, 0, 0xFF, 0);

			gLog.Output(LOG_CHAOS_MIX, "[HolyangelWeaponMix][Success][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
		}
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[HolyangelWeaponMix][Failure][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::HolyangelStickMix(LPOBJ lpObj, int type) // OK
{
	int WeaponCount = 0;
	int ItemCount = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(5, 52))
		{
			WeaponCount++;
		}
		else if ((lpObj->ChaosBox[n].m_Index >= GET_ITEM(14, 416)))
		{
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
	}

	if (WeaponCount != 1 || ItemCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_HolyangelWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney / 4000000) + (ItemMoney / 40000);
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_HolyangelWeaponMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_HolyangelWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>90) ? 90 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 500000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		if (type == 0)
		{
			WORD ItemIndex = 0;

			CRandomManager RandomManager;

			RandomManager.AddElement(GET_ITEM(5, 54), 1);

			RandomManager.GetRandomElement(&ItemIndex);

			GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, 1, 1, -1, 56, 0, 0, 0, 0, 0xFF, 0);

			gLog.Output(LOG_CHAOS_MIX, "[HolyangelWeaponMix][Success][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
		}
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[HolyangelWeaponMix][Failure][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::SoulSwordMix(LPOBJ lpObj, int type) // OK
{
	int WeaponCount = 0;
	int ItemCount = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(0, 57))
		{
			WeaponCount++;
		}
		else if ((lpObj->ChaosBox[n].m_Index >= GET_ITEM(14, 450)))
		{
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
	}

	if (WeaponCount != 1 || ItemCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_SoulWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney / 4000000) + (ItemMoney / 40000);
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_SoulWeaponMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_SoulWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>90) ? 90 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 750000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		if (type == 0)
		{
			WORD ItemIndex = 0;

			CRandomManager RandomManager;

			RandomManager.AddElement(GET_ITEM(0, 62), 1);

			RandomManager.GetRandomElement(&ItemIndex);

			GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, 1, 1, -1, 56, 0, 0, 0, 0, 0xFF, 0);

			gLog.Output(LOG_CHAOS_MIX, "[SoulWeaponMix][Success][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
		}
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[SoulWeaponMix][Failure][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::SoulMagicSwordMix(LPOBJ lpObj, int type) // OK
{
	int WeaponCount = 0;
	int ItemCount = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(0, 58))
		{
			WeaponCount++;
		}
		else if ((lpObj->ChaosBox[n].m_Index >= GET_ITEM(14, 450)))
		{
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
	}

	if (WeaponCount != 1 || ItemCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_SoulWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney / 4000000) + (ItemMoney / 40000);
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_SoulWeaponMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_SoulWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>90) ? 90 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 750000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		if (type == 0)
		{
			WORD ItemIndex = 0;

			CRandomManager RandomManager;

			RandomManager.AddElement(GET_ITEM(0, 63), 1);

			RandomManager.GetRandomElement(&ItemIndex);

			GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, 1, 1, -1, 56, 0, 0, 0, 0, 0xFF, 0);

			gLog.Output(LOG_CHAOS_MIX, "[SoulWeaponMix][Success][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
		}
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[SoulWeaponMix][Failure][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::SoulClawMix(LPOBJ lpObj, int type) // OK
{
	int WeaponCount = 0;
	int ItemCount = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(0, 59))
		{
			WeaponCount++;
		}
		else if ((lpObj->ChaosBox[n].m_Index >= GET_ITEM(14, 450)))
		{
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
	}

	if (WeaponCount != 1 || ItemCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_SoulWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney / 4000000) + (ItemMoney / 40000);
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_SoulWeaponMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_SoulWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>90) ? 90 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 750000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		if (type == 0)
		{
			WORD ItemIndex = 0;

			CRandomManager RandomManager;

			RandomManager.AddElement(GET_ITEM(0, 64), 1);

			RandomManager.GetRandomElement(&ItemIndex);

			GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, 1, 1, -1, 56, 0, 0, 0, 0, 0xFF, 0);

			gLog.Output(LOG_CHAOS_MIX, "[SoulWeaponMix][Success][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
		}
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[SoulWeaponMix][Failure][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::SoulScepterMix(LPOBJ lpObj, int type) // OK
{
	int WeaponCount = 0;
	int ItemCount = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(2, 27))
		{
			WeaponCount++;
		}
		else if ((lpObj->ChaosBox[n].m_Index >= GET_ITEM(14, 450)))
		{
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
	}

	if (WeaponCount != 1 || ItemCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_SoulWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney / 4000000) + (ItemMoney / 40000);
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_SoulWeaponMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_SoulWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>90) ? 90 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 750000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		if (type == 0)
		{
			WORD ItemIndex = 0;

			CRandomManager RandomManager;

			RandomManager.AddElement(GET_ITEM(2, 36), 1);

			RandomManager.GetRandomElement(&ItemIndex);

			GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, 1, 1, -1, 56, 0, 0, 0, 0, 0xFF, 0);

			gLog.Output(LOG_CHAOS_MIX, "[SoulWeaponMix][Success][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
		}
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[SoulWeaponMix][Failure][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::SoulBowMix(LPOBJ lpObj, int type) // OK
{
	int WeaponCount = 0;
	int ItemCount = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(4, 36))
		{
			WeaponCount++;
		}
		else if ((lpObj->ChaosBox[n].m_Index >= GET_ITEM(14, 450)))
		{
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
	}

	if (WeaponCount != 1 || ItemCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_SoulWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney / 4000000) + (ItemMoney / 40000);
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_SoulWeaponMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_SoulWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>90) ? 90 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 750000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		if (type == 0)
		{
			WORD ItemIndex = 0;

			CRandomManager RandomManager;

			RandomManager.AddElement(GET_ITEM(4, 38), 1);

			RandomManager.GetRandomElement(&ItemIndex);

			GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, 1, 1, -1, 56, 0, 0, 0, 0, 0xFF, 0);

			gLog.Output(LOG_CHAOS_MIX, "[SoulWeaponMix][Success][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
		}
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[SoulWeaponMix][Failure][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::SoulStaffMix(LPOBJ lpObj, int type) // OK
{
	int WeaponCount = 0;
	int ItemCount = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(5, 53))
		{
			WeaponCount++;
		}
		else if ((lpObj->ChaosBox[n].m_Index >= GET_ITEM(14, 450)))
		{
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
	}

	if (WeaponCount != 1 || ItemCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_SoulWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney / 4000000) + (ItemMoney / 40000);
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_SoulWeaponMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_SoulWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>90) ? 90 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 750000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		if (type == 0)
		{
			WORD ItemIndex = 0;

			CRandomManager RandomManager;

			RandomManager.AddElement(GET_ITEM(5, 55), 1);

			RandomManager.GetRandomElement(&ItemIndex);
					
			GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, 1, 1, -1, 56, 0, 0, 0, 0, 0xFF, 0);

			gLog.Output(LOG_CHAOS_MIX, "[SoulWeaponMix][Success][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
		}
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[SoulWeaponMix][Failure][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::SoulStickMix(LPOBJ lpObj, int type) // OK
{
	int WeaponCount = 0;
	int ItemCount = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(5, 54))
		{
			WeaponCount++;
		}
		else if ((lpObj->ChaosBox[n].m_Index >= GET_ITEM(14, 450)))
		{
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
	}

	if (WeaponCount != 1 || ItemCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_SoulWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney / 4000000) + (ItemMoney / 40000);
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_SoulWeaponMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_SoulWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>90) ? 90 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 750000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		if (type == 0)
		{
			WORD ItemIndex = 0;

			CRandomManager RandomManager;

			RandomManager.AddElement(GET_ITEM(5, 56), 1);

			RandomManager.GetRandomElement(&ItemIndex);

			GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, 1, 1, -1, 56, 0, 0, 0, 0, 0xFF, 0);

			gLog.Output(LOG_CHAOS_MIX, "[SoulWeaponMix][Success][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
		}
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[SoulWeaponMix][Failure][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::BlueEyeSwordMix(LPOBJ lpObj, int type) // OK
{
	int WeaponCount = 0;
	int ItemCount = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(0, 62))
		{
			WeaponCount++;
		}
		else if ((lpObj->ChaosBox[n].m_Index >= GET_ITEM(14, 495)))
		{
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
	}

	if (WeaponCount != 1 || ItemCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_BlueEyeWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney / 4000000) + (ItemMoney / 40000);
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_BlueEyeWeaponMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_BlueEyeWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>90) ? 90 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 1000000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		if (type == 0)
		{
			WORD ItemIndex = 0;

			CRandomManager RandomManager;

			RandomManager.AddElement(GET_ITEM(0, 78), 1);

			RandomManager.GetRandomElement(&ItemIndex);

			GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, 1, 1, -1, 56, 0, 0, 0, 0, 0xFF, 0);

			gLog.Output(LOG_CHAOS_MIX, "[BlueEyeWeaponMix][Success][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
		}
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[BlueEyeWeaponMix][Failure][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::BlueEyeMagicSwordMix(LPOBJ lpObj, int type) // OK
{
	int WeaponCount = 0;
	int ItemCount = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(0, 63))
		{
			WeaponCount++;
		}
		else if ((lpObj->ChaosBox[n].m_Index >= GET_ITEM(14, 495)))
		{
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
	}

	if (WeaponCount != 1 || ItemCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_BlueEyeWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney / 4000000) + (ItemMoney / 40000);
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_BlueEyeWeaponMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_BlueEyeWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>90) ? 90 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 1000000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		if (type == 0)
		{
			WORD ItemIndex = 0;

			CRandomManager RandomManager;

			RandomManager.AddElement(GET_ITEM(0, 79), 1);

			RandomManager.GetRandomElement(&ItemIndex);

			GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, 1, 1, -1, 56, 0, 0, 0, 0, 0xFF, 0);

			gLog.Output(LOG_CHAOS_MIX, "[BlueEyeWeaponMix][Success][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
		}
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[BlueEyeWeaponMix][Failure][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::BlueEyeClawMix(LPOBJ lpObj, int type) // OK
{
	int WeaponCount = 0;
	int ItemCount = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(0, 64))
		{
			WeaponCount++;
		}
		else if ((lpObj->ChaosBox[n].m_Index >= GET_ITEM(14, 495)))
		{
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
	}

	if (WeaponCount != 1 || ItemCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_BlueEyeWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney / 4000000) + (ItemMoney / 40000);
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_BlueEyeWeaponMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_BlueEyeWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>90) ? 90 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 1000000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		if (type == 0)
		{
			WORD ItemIndex = 0;

			CRandomManager RandomManager;

			RandomManager.AddElement(GET_ITEM(0, 80), 1);

			RandomManager.GetRandomElement(&ItemIndex);

			GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, 1, 1, -1, 56, 0, 0, 0, 0, 0xFF, 0);

			gLog.Output(LOG_CHAOS_MIX, "[BlueEyeWeaponMix][Success][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
		}
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[BlueEyeWeaponMix][Failure][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::BlueEyeScepterMix(LPOBJ lpObj, int type) // OK
{
	int WeaponCount = 0;
	int ItemCount = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(2, 36))
		{
			WeaponCount++;
		}
		else if ((lpObj->ChaosBox[n].m_Index >= GET_ITEM(14, 495)))
		{
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
	}

	if (WeaponCount != 1 || ItemCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_BlueEyeWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney / 4000000) + (ItemMoney / 40000);
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_BlueEyeWeaponMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_BlueEyeWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>90) ? 90 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 1000000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		if (type == 0)
		{
			WORD ItemIndex = 0;

			CRandomManager RandomManager;

			RandomManager.AddElement(GET_ITEM(2, 40), 1);

			RandomManager.GetRandomElement(&ItemIndex);

			GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, 1, 1, -1, 56, 0, 0, 0, 0, 0xFF, 0);

			gLog.Output(LOG_CHAOS_MIX, "[BlueEyeWeaponMix][Success][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
		}
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[BlueEyeWeaponMix][Failure][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::BlueEyeBowMix(LPOBJ lpObj, int type) // OK
{
	int WeaponCount = 0;
	int ItemCount = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(4, 38))
		{
			WeaponCount++;
		}
		else if ((lpObj->ChaosBox[n].m_Index >= GET_ITEM(14, 495)))
		{
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
	}

	if (WeaponCount != 1 || ItemCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_BlueEyeWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney / 4000000) + (ItemMoney / 40000);
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_BlueEyeWeaponMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_BlueEyeWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>90) ? 90 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 1000000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		if (type == 0)
		{
			WORD ItemIndex = 0;

			CRandomManager RandomManager;

			RandomManager.AddElement(GET_ITEM(4, 40), 1);

			RandomManager.GetRandomElement(&ItemIndex);

			GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, 1, 1, -1, 56, 0, 0, 0, 0, 0xFF, 0);

			gLog.Output(LOG_CHAOS_MIX, "[BlueEyeWeaponMix][Success][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
		}
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[BlueEyeWeaponMix][Failure][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::BlueEyeStaffMix(LPOBJ lpObj, int type) // OK
{
	int WeaponCount = 0;
	int ItemCount = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(5, 55))
		{
			WeaponCount++;
		}
		else if ((lpObj->ChaosBox[n].m_Index >= GET_ITEM(14, 495)))
		{
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
	}

	if (WeaponCount != 1 || ItemCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_BlueEyeWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney / 4000000) + (ItemMoney / 40000);
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_BlueEyeWeaponMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_BlueEyeWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>90) ? 90 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 1000000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		if (type == 0)
		{
			WORD ItemIndex = 0;

			CRandomManager RandomManager;

			RandomManager.AddElement(GET_ITEM(5, 61), 1);

			RandomManager.GetRandomElement(&ItemIndex);
					
			GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, 1, 1, -1, 56, 0, 0, 0, 0, 0xFF, 0);

			gLog.Output(LOG_CHAOS_MIX, "[BlueEyeWeaponMix][Success][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
		}
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[BlueEyeWeaponMix][Failure][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::BlueEyeStickMix(LPOBJ lpObj, int type) // OK
{
	int WeaponCount = 0;
	int ItemCount = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(5, 56))
		{
			WeaponCount++;
		}
		else if ((lpObj->ChaosBox[n].m_Index >= GET_ITEM(14, 495)))
		{
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
	}

	if (WeaponCount != 1 || ItemCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_BlueEyeWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney / 4000000) + (ItemMoney / 40000);
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_BlueEyeWeaponMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_BlueEyeWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>90) ? 90 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 1000000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		if (type == 0)
		{
			WORD ItemIndex = 0;

			CRandomManager RandomManager;

			RandomManager.AddElement(GET_ITEM(5, 62), 1);

			RandomManager.GetRandomElement(&ItemIndex);

			GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, 1, 1, -1, 56, 0, 0, 0, 0, 0xFF, 0);

			gLog.Output(LOG_CHAOS_MIX, "[BlueEyeWeaponMix][Success][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
		}
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[BlueEyeWeaponMix][Failure][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}


void CChaosBox::SilverHeartSwordMix(LPOBJ lpObj, int type) // OK
{
	int WeaponCount = 0;
	int ItemCount = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(0, 78))
		{
			WeaponCount++;
		}
		else if ((lpObj->ChaosBox[n].m_Index >= GET_ITEM(14, 502)))
		{
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
	}

	if (WeaponCount != 1 || ItemCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_SilverHeartWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney / 4000000) + (ItemMoney / 40000);
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_SilverHeartWeaponMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_SilverHeartWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>90) ? 90 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 1250000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		if (type == 0)
		{
			WORD ItemIndex = 0;

			CRandomManager RandomManager;

			RandomManager.AddElement(GET_ITEM(0, 81), 1);

			RandomManager.GetRandomElement(&ItemIndex);

			GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, 1, 1, -1, 56, 0, 0, 0, 0, 0xFF, 0);

			gLog.Output(LOG_CHAOS_MIX, "[SilverHeartWeaponMix][Success][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
		}
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[SilverHeartWeaponMix][Failure][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::SilverHeartMagicSwordMix(LPOBJ lpObj, int type) // OK
{
	int WeaponCount = 0;
	int ItemCount = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(0, 79))
		{
			WeaponCount++;
		}
		else if ((lpObj->ChaosBox[n].m_Index >= GET_ITEM(14, 502)))
		{
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
	}

	if (WeaponCount != 1 || ItemCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_SilverHeartWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney / 4000000) + (ItemMoney / 40000);
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_SilverHeartWeaponMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_SilverHeartWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>90) ? 90 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 1250000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		if (type == 0)
		{
			WORD ItemIndex = 0;

			CRandomManager RandomManager;

			RandomManager.AddElement(GET_ITEM(0, 82), 1);

			RandomManager.GetRandomElement(&ItemIndex);

			GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, 1, 1, -1, 56, 0, 0, 0, 0, 0xFF, 0);

			gLog.Output(LOG_CHAOS_MIX, "[SilverHeartWeaponMix][Success][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
		}
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[SilverHeartWeaponMix][Failure][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::SilverHeartClawMix(LPOBJ lpObj, int type) // OK
{
	int WeaponCount = 0;
	int ItemCount = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(0, 80))
		{
			WeaponCount++;
		}
		else if ((lpObj->ChaosBox[n].m_Index >= GET_ITEM(14, 502)))
		{
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
	}

	if (WeaponCount != 1 || ItemCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_SilverHeartWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney / 4000000) + (ItemMoney / 40000);
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_SilverHeartWeaponMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_SilverHeartWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>90) ? 90 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 1250000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		if (type == 0)
		{
			WORD ItemIndex = 0;

			CRandomManager RandomManager;

			RandomManager.AddElement(GET_ITEM(0, 83), 1);

			RandomManager.GetRandomElement(&ItemIndex);

			GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, 1, 1, -1, 56, 0, 0, 0, 0, 0xFF, 0);

			gLog.Output(LOG_CHAOS_MIX, "[SilverHeartWeaponMix][Success][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
		}
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[SilverHeartWeaponMix][Failure][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::SilverHeartScepterMix(LPOBJ lpObj, int type) // OK
{
	int WeaponCount = 0;
	int ItemCount = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(2, 40))
		{
			WeaponCount++;
		}
		else if ((lpObj->ChaosBox[n].m_Index >= GET_ITEM(14, 502)))
		{
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
	}

	if (WeaponCount != 1 || ItemCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_SilverHeartWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney / 4000000) + (ItemMoney / 40000);
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_SilverHeartWeaponMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_SilverHeartWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>90) ? 90 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 1250000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		if (type == 0)
		{
			WORD ItemIndex = 0;

			CRandomManager RandomManager;

			RandomManager.AddElement(GET_ITEM(2, 42), 1);

			RandomManager.GetRandomElement(&ItemIndex);

			GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, 1, 1, -1, 56, 0, 0, 0, 0, 0xFF, 0);

			gLog.Output(LOG_CHAOS_MIX, "[SilverHeartWeaponMix][Success][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
		}
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[SilverHeartWeaponMix][Failure][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::SilverHeartBowMix(LPOBJ lpObj, int type) // OK
{
	int WeaponCount = 0;
	int ItemCount = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(4, 40))
		{
			WeaponCount++;
		}
		else if ((lpObj->ChaosBox[n].m_Index >= GET_ITEM(14, 502)))
		{
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
	}

	if (WeaponCount != 1 || ItemCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_SilverHeartWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney / 4000000) + (ItemMoney / 40000);
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_SilverHeartWeaponMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_SilverHeartWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>90) ? 90 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 1250000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		if (type == 0)
		{
			WORD ItemIndex = 0;

			CRandomManager RandomManager;

			RandomManager.AddElement(GET_ITEM(4, 56), 1);

			RandomManager.GetRandomElement(&ItemIndex);

			GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, 1, 1, -1, 56, 0, 0, 0, 0, 0xFF, 0);

			gLog.Output(LOG_CHAOS_MIX, "[SilverHeartWeaponMix][Success][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
		}
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[SilverHeartWeaponMix][Failure][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::SilverHeartStaffMix(LPOBJ lpObj, int type) // OK
{
	int WeaponCount = 0;
	int ItemCount = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(5, 61))
		{
			WeaponCount++;
		}
		else if ((lpObj->ChaosBox[n].m_Index >= GET_ITEM(14, 502)))
		{
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
	}

	if (WeaponCount != 1 || ItemCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_SilverHeartWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney / 4000000) + (ItemMoney / 40000);
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_SilverHeartWeaponMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_SilverHeartWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>90) ? 90 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 1250000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		if (type == 0)
		{
			WORD ItemIndex = 0;

			CRandomManager RandomManager;

			RandomManager.AddElement(GET_ITEM(5, 64), 1);

			RandomManager.GetRandomElement(&ItemIndex);
					
			GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, 1, 1, -1, 56, 0, 0, 0, 0, 0xFF, 0);

			gLog.Output(LOG_CHAOS_MIX, "[SilverHeartWeaponMix][Success][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
		}
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[SilverHeartWeaponMix][Failure][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::SilverHeartStickMix(LPOBJ lpObj, int type) // OK
{
	int WeaponCount = 0;
	int ItemCount = 0;
	int ItemMoney = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(5, 62))
		{
			WeaponCount++;
		}
		else if ((lpObj->ChaosBox[n].m_Index >= GET_ITEM(14, 502)))
		{
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
	}

	if (WeaponCount != 1 || ItemCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}

	if (gServerInfo.m_SilverHeartWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney / 4000000) + (ItemMoney / 40000);
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_SilverHeartWeaponMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_SilverHeartWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>90) ? 90 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 1250000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if ((rand() % 100) < lpObj->ChaosSuccessRate)
	{
		if (type == 0)
		{
			WORD ItemIndex = 0;

			CRandomManager RandomManager;

			RandomManager.AddElement(GET_ITEM(5, 65), 1);

			RandomManager.GetRandomElement(&ItemIndex);

			GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 1, 1, 1, -1, 56, 0, 0, 0, 0, 0xFF, 0);

			gLog.Output(LOG_CHAOS_MIX, "[SilverHeartWeaponMix][Success][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
		}
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gLog.Output(LOG_CHAOS_MIX, "[SilverHeartWeaponMix][Failure][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

// CustomJewel.h: interface for the CCustomJewel class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Item.h"
#include "User.h"
#include <vector>

#define MAX_CUSTOM_JEWEL 50
#define MAX_CUSTOM_JEWELUPGRADE 2000

struct CUSTOM_JEWEL_SUCCESS_INFO
{
	int Index;
	int Level;
	int Option1;
	int Option2;
	int Option3;
	int NewOption;
	int SetOption;
	int SocketOption;
};

struct CUSTOM_JEWEL_UPGRADE_INFO
{
	int Index;
	int Type;
	int ItemOld;
	int ItemNew;
};

struct CUSTOM_JEWEL_FAILURE_INFO
{
	int Index;
	int Level;
	int Option1;
	int Option2;
	int Option3;
	int NewOption;
	int SetOption;
	int SocketOption;
};

struct CUSTOM_JEWEL_INFO
{
	int Index;
	int ItemIndex;
	int MinItemLevel;
	int MaxItemLevel;
	int MaxItemOption1;
	int MaxItemOption2;
	int MinItemOption3;
	int MaxItemOption3;
	int MinItemNewOption;
	int MaxItemNewOption;
	int MaxItemSetOption;
	int MinItemSocketOption;
	int MaxItemSocketOption;
	int EnableSlotWeapon;
	int EnableSlotArmor;
	int EnableSlotWing;
	int SuccessRate[4];
	int SalePrice;
	char ModelName[32];
	CUSTOM_JEWEL_SUCCESS_INFO SuccessInfo;
	CUSTOM_JEWEL_FAILURE_INFO FailureInfo;
};

class CCustomJewel
{
public:
	CCustomJewel();
	virtual ~CCustomJewel();
	void Init();
	void Load(char* path);
	void SetInfo(CUSTOM_JEWEL_INFO info);
	void SetSuccessInfo(CUSTOM_JEWEL_SUCCESS_INFO info);
	void SetFailureInfo(CUSTOM_JEWEL_FAILURE_INFO info);

	CUSTOM_JEWEL_INFO* GetInfo(int index);
	CUSTOM_JEWEL_INFO* GetInfoByItem(int ItemIndex);
	CUSTOM_JEWEL_SUCCESS_INFO* GetSuccessInfo(int ItemIndex);
	CUSTOM_JEWEL_FAILURE_INFO* GetFailureInfo(int ItemIndex);
	
	bool CheckCustomJewel(int index);
	bool CheckCustomJewelByItem(int ItemIndex);
	bool CheckCustomJewelApplyItem(int ItemIndex,CItem* lpItem);
	bool CheckCustomJewelApplyItemNewOption(CUSTOM_JEWEL_INFO* lpInfo,CItem* lpItem);
	bool CheckCustomJewelApplyItemSetOption(CUSTOM_JEWEL_INFO* lpInfo,CItem* lpItem);
	bool CheckCustomJewelApplyItemSocketOption(CUSTOM_JEWEL_INFO* lpInfo,CItem* lpItem);
	int GetCustomJewelSuccessRate(int ItemIndex,int AccountLevel);
	int GetCustomJewelSalePrice(int ItemIndex);
	int GetCustomJewelNewOption(CItem* lpItem,int value);
	int GetCustomJewelSetOption(CItem* lpItem,int value);
	int GetCustomJewelSocketOption(CItem* lpItem,int value);

	bool CharacterUseCustomJewel(LPOBJ lpObj,int SourceSlot,int TargetSlot);
public:
	CUSTOM_JEWEL_INFO m_CustomJewelInfo[MAX_CUSTOM_JEWEL];
private:
	std::vector<CUSTOM_JEWEL_UPGRADE_INFO> m_UpgradeInfo;
};

extern CCustomJewel gCustomJewel;

#pragma once

#include "Item.h"
#include "User.h"

#define MAX_CUSTOM_PET 512 // 500

struct CUSTOM_PET_INFO
{
	int Index;
	int ItemIndex;
	int IncExperienceRate;
	int IncMasterExperienceRate;
	int IncLife;
	int IncMana;
	int IncDamageRate;
	int IncDefenseRate;
	int IncAttackSpeed;
	int IncDamageReflect;
	int IncFullDamageReflect;
	int IncCriticalDamageRate;
	int IncCriticalDamage;
	int IncExcellentDamageRate;
	int IncExcellentDamage;
	int IncDoubleDamageRate;
	int IncTripleDamageRate;
	int IncDamageReduction;
	int IncIgnoreDefenseRate;
	int IncResistCriticalDamage;
	int IncResistExcellentDamage;
	int IncResistDoubleDamage;
	int IncResistIgnoreDefense;
	int IncResistStun;
	int IncBlockStuck;
	int IncImortalPet;
	char ModelName[32];
};

class CCustomPet
{
public:
	CCustomPet();
	virtual ~CCustomPet();
	void Init();
	void Load(char* path);
	void SetInfo(CUSTOM_PET_INFO info);
	CUSTOM_PET_INFO* GetInfo(int index);
	CUSTOM_PET_INFO* GetInfoByItem(int ItemIndex);
	bool CheckCustomPet(int index);
	bool CheckCustomPetByItem(int ItemIndex);
	int GetCustomPetIndex(int ItemIndex);
	long GetCustomPetExperienceRate(int ItemIndex);
	long GetCustomPetMasterExperienceRate(int ItemIndex);
	int GetCustomPetLife(int ItemIndex);
	int GetCustomPetMana(int ItemIndex);
	int GetCustomPetDamageRate(int ItemIndex);
	int GetCustomPetDefenseRate(int ItemIndex);
	int GetCustomPetAttackSpeed(int ItemIndex);
	int GetCustomPetDamageReflect(int ItemIndex);
	int GetCustomPetFullDamageReflect(int ItemIndex);
	int GetCustomPetCriticalDamageRate(int ItemIndex);
	int GetCustomPetCriticalDamage(int ItemIndex);
	int GetCustomPetExcellentDamageRate(int ItemIndex);
	int GetCustomPetExcellentDamage(int ItemIndex);
	int GetCustomPetDoubleDamageRate(int ItemIndex);
	int GetCustomPetTripleDamageRate(int ItemIndex);
	int GetCustomPetDamageReduction(int ItemIndex);
	int GetCustomPetIgnoreDefenseRate(int ItemIndex);
	int GetCustomPetResistCriticalDamage(int ItemIndex);
	int GetCustomPetResistExcellentDamage(int ItemIndex);
	int GetCustomPetResistDoubleDamage(int ItemIndex);
	int GetCustomPetResistIgnoreDefense(int ItemIndex);
	int GetCustomPetResistStun(int ItemIndex);
	int GetCustomPetBlockStuck(int ItemIndex);
	int GetCustomPetImortalPet(int ItemIndex);
	void CalcCustomPetOption(LPOBJ lpObj, bool flag);
public:
	CUSTOM_PET_INFO m_CustomPetInfo[MAX_CUSTOM_PET];
};

extern CCustomPet gCustomPet;
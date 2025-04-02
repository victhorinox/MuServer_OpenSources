#include "stdafx.h"
#include "CustomPet.h"
#include "MemScript.h"
#include "Util.h"

CCustomPet gCustomPet;

CCustomPet::CCustomPet() // OK
{
	this->Init();
}

CCustomPet::~CCustomPet() // OK
{

}

void CCustomPet::Init() // OK
{
	for (int n = 0; n < MAX_CUSTOM_PET; n++)
	{
		this->m_CustomPetInfo[n].Index = -1;
	}
}

void CCustomPet::Load(char* path) // OK
{
	CMemScript* lpMemScript = new CMemScript;

	if (lpMemScript == 0)
	{
		ErrorMessageBox(MEM_SCRIPT_ALLOC_ERROR, path);
		return;
	}

	if (lpMemScript->SetBuffer(path) == 0)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
		delete lpMemScript;
		return;
	}

	this->Init();

	try
	{
		while (true)
		{
			if (lpMemScript->GetToken() == TOKEN_END)
			{
				break;
			}

			if (strcmp("end", lpMemScript->GetString()) == 0)
			{
				break;
			}

			CUSTOM_PET_INFO info;

			memset(&info, 0, sizeof(info));

			info.Index = lpMemScript->GetNumber();

			info.ItemIndex = lpMemScript->GetAsNumber();

			info.IncExperienceRate = lpMemScript->GetAsNumber();

			info.IncMasterExperienceRate = lpMemScript->GetAsNumber();

			info.IncLife = lpMemScript->GetAsNumber();

			info.IncMana = lpMemScript->GetAsNumber();

			info.IncDamageRate = lpMemScript->GetAsNumber();
			
			info.IncDefenseRate = lpMemScript->GetAsNumber();

			info.IncAttackSpeed = lpMemScript->GetAsNumber();
			
			info.IncDamageReflect = lpMemScript->GetAsNumber();
			
			info.IncFullDamageReflect = lpMemScript->GetAsNumber();

			info.IncCriticalDamageRate = lpMemScript->GetAsNumber();

			info.IncCriticalDamage = lpMemScript->GetAsNumber();
			
			info.IncExcellentDamageRate = lpMemScript->GetAsNumber();

			info.IncExcellentDamage = lpMemScript->GetAsNumber();

			info.IncDoubleDamageRate = lpMemScript->GetAsNumber();

			info.IncTripleDamageRate = lpMemScript->GetAsNumber();
			
			info.IncDamageReduction = lpMemScript->GetAsNumber();
			
			info.IncIgnoreDefenseRate = lpMemScript->GetAsNumber();

			info.IncResistCriticalDamage = lpMemScript->GetAsNumber();

			info.IncResistExcellentDamage = lpMemScript->GetAsNumber();

			info.IncResistDoubleDamage = lpMemScript->GetAsNumber();

			info.IncResistIgnoreDefense = lpMemScript->GetAsNumber();

			info.IncResistStun = lpMemScript->GetAsNumber();

			info.IncBlockStuck = lpMemScript->GetAsNumber();

			info.IncImortalPet = lpMemScript->GetAsNumber();

			strcpy_s(info.ModelName, lpMemScript->GetAsString());

			this->SetInfo(info);
		}
	}
	catch (...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;
}

void CCustomPet::SetInfo(CUSTOM_PET_INFO info) // OK
{
	if (info.Index < 0 || info.Index >= MAX_CUSTOM_PET)
	{
		return;
	}

	this->m_CustomPetInfo[info.Index] = info;
}

CUSTOM_PET_INFO* CCustomPet::GetInfo(int index) // OK
{
	if (index < 0 || index >= MAX_CUSTOM_PET)
	{
		return 0;
	}

	if (this->m_CustomPetInfo[index].Index != index)
	{
		return 0;
	}

	return &this->m_CustomPetInfo[index];
}

CUSTOM_PET_INFO* CCustomPet::GetInfoByItem(int ItemIndex) // OK
{
	for (int n = 0; n < MAX_CUSTOM_PET; n++)
	{
		CUSTOM_PET_INFO* lpInfo = this->GetInfo(n);

		if (lpInfo == 0)
		{
			continue;
		}

		if (lpInfo->ItemIndex == ItemIndex)
		{
			return lpInfo;
		}
	}

	return 0;
}

bool CCustomPet::CheckCustomPet(int index) // OK
{
	if (this->GetInfo(index) != 0)
	{
		return 1;
	}

	return 0;
}

bool CCustomPet::CheckCustomPetByItem(int ItemIndex) // OK
{
	if (this->GetInfoByItem(ItemIndex) != 0)
	{
		return 1;
	}

	return 0;
}

int CCustomPet::GetCustomPetIndex(int ItemIndex) // OK
{
	CUSTOM_PET_INFO* lpInfo = this->GetInfoByItem(ItemIndex);

	if (lpInfo == 0)
	{
		return 0;
	}

	return lpInfo->Index;
}

long CCustomPet::GetCustomPetExperienceRate(int ItemIndex) // OK
{
	CUSTOM_PET_INFO* lpInfo = this->GetInfoByItem(ItemIndex);

	if (lpInfo == 0)
	{
		return 0;
	}

	return (lpInfo->IncExperienceRate);
}

long CCustomPet::GetCustomPetMasterExperienceRate(int ItemIndex) // OK
{
	CUSTOM_PET_INFO* lpInfo = this->GetInfoByItem(ItemIndex);

	if (lpInfo == 0)
	{
		return 0;
	}

	return (lpInfo->IncMasterExperienceRate);
}

int CCustomPet::GetCustomPetLife(int ItemIndex) // OK
{
	CUSTOM_PET_INFO* lpInfo = this->GetInfoByItem(ItemIndex);

	if (lpInfo == 0)
	{
		return 0;
	}

	return (lpInfo->IncLife);
}

int CCustomPet::GetCustomPetMana(int ItemIndex) // OK
{
	CUSTOM_PET_INFO* lpInfo = this->GetInfoByItem(ItemIndex);

	if (lpInfo == 0)
	{
		return 0;
	}

	return (lpInfo->IncMana);
}

int CCustomPet::GetCustomPetDamageRate(int ItemIndex) // OK
{
	CUSTOM_PET_INFO* lpInfo = this->GetInfoByItem(ItemIndex);

	if (lpInfo == 0)
	{
		return 0;
	}

	return (lpInfo->IncDamageRate);
}

int CCustomPet::GetCustomPetDefenseRate(int ItemIndex) // OK
{
	CUSTOM_PET_INFO* lpInfo = this->GetInfoByItem(ItemIndex);

	if (lpInfo == 0)
	{
		return 0;
	}

	return (lpInfo->IncDefenseRate);
}

int CCustomPet::GetCustomPetAttackSpeed(int ItemIndex) // OK
{
	CUSTOM_PET_INFO* lpInfo = this->GetInfoByItem(ItemIndex);

	if (lpInfo == 0)
	{
		return 0;
	}

	return (lpInfo->IncAttackSpeed);
}

int CCustomPet::GetCustomPetDamageReflect(int ItemIndex) // OK
{
	CUSTOM_PET_INFO* lpInfo = this->GetInfoByItem(ItemIndex);

	if (lpInfo == 0)
	{
		return 0;
	}

	return (lpInfo->IncDamageReflect);
}

int CCustomPet::GetCustomPetFullDamageReflect(int ItemIndex) // OK
{
	CUSTOM_PET_INFO* lpInfo = this->GetInfoByItem(ItemIndex);

	if (lpInfo == 0)
	{
		return 0;
	}

	return (lpInfo->IncFullDamageReflect);
}

int CCustomPet::GetCustomPetCriticalDamageRate(int ItemIndex) // OK
{
	CUSTOM_PET_INFO* lpInfo = this->GetInfoByItem(ItemIndex);

	if (lpInfo == 0)
	{
		return 0;
	}

	return (lpInfo->IncCriticalDamageRate);
}

int CCustomPet::GetCustomPetCriticalDamage(int ItemIndex) // OK
{
	CUSTOM_PET_INFO* lpInfo = this->GetInfoByItem(ItemIndex);

	if (lpInfo == 0)
	{
		return 0;
	}

	return (lpInfo->IncCriticalDamage);
}

int CCustomPet::GetCustomPetExcellentDamageRate(int ItemIndex) // OK
{
	CUSTOM_PET_INFO* lpInfo = this->GetInfoByItem(ItemIndex);

	if (lpInfo == 0)
	{
		return 0;
	}

	return (lpInfo->IncExcellentDamageRate);
}

int CCustomPet::GetCustomPetExcellentDamage(int ItemIndex) // OK
{
	CUSTOM_PET_INFO* lpInfo = this->GetInfoByItem(ItemIndex);

	if (lpInfo == 0)
	{
		return 0;
	}

	return (lpInfo->IncExcellentDamage);
}

int CCustomPet::GetCustomPetDoubleDamageRate(int ItemIndex) // OK
{
	CUSTOM_PET_INFO* lpInfo = this->GetInfoByItem(ItemIndex);

	if (lpInfo == 0)
	{
		return 0;
	}

	return (lpInfo->IncDoubleDamageRate);
}

int CCustomPet::GetCustomPetTripleDamageRate(int ItemIndex) // OK
{
	CUSTOM_PET_INFO* lpInfo = this->GetInfoByItem(ItemIndex);

	if (lpInfo == 0)
	{
		return 0;
	}

	return (lpInfo->IncTripleDamageRate);
}

int CCustomPet::GetCustomPetDamageReduction(int ItemIndex) // OK
{
	CUSTOM_PET_INFO* lpInfo = this->GetInfoByItem(ItemIndex);

	if (lpInfo == 0)
	{
		return 0;
	}

	return (lpInfo->IncDamageReduction);
}

int CCustomPet::GetCustomPetIgnoreDefenseRate(int ItemIndex) // OK
{
	CUSTOM_PET_INFO* lpInfo = this->GetInfoByItem(ItemIndex);

	if (lpInfo == 0)
	{
		return 0;
	}

	return (lpInfo->IncIgnoreDefenseRate);
}

int CCustomPet::GetCustomPetResistCriticalDamage(int ItemIndex) // OK
{
	CUSTOM_PET_INFO* lpInfo = this->GetInfoByItem(ItemIndex);

	if (lpInfo == 0)
	{
		return 0;
	}

	return (lpInfo->IncResistCriticalDamage);
}

int CCustomPet::GetCustomPetResistExcellentDamage(int ItemIndex) // OK
{
	CUSTOM_PET_INFO* lpInfo = this->GetInfoByItem(ItemIndex);

	if (lpInfo == 0)
	{
		return 0;
	}

	return (lpInfo->IncResistExcellentDamage);
}

int CCustomPet::GetCustomPetResistDoubleDamage(int ItemIndex) // OK
{
	CUSTOM_PET_INFO* lpInfo = this->GetInfoByItem(ItemIndex);

	if (lpInfo == 0)
	{
		return 0;
	}

	return (lpInfo->IncResistDoubleDamage);
}

int CCustomPet::GetCustomPetResistIgnoreDefense(int ItemIndex) // OK
{
	CUSTOM_PET_INFO* lpInfo = this->GetInfoByItem(ItemIndex);

	if (lpInfo == 0)
	{
		return 0;
	}

	return (lpInfo->IncResistIgnoreDefense);
}

int CCustomPet::GetCustomPetResistStun(int ItemIndex) // OK
{
	CUSTOM_PET_INFO* lpInfo = this->GetInfoByItem(ItemIndex);

	if (lpInfo == 0)
	{
		return 0;
	}

	return (lpInfo->IncResistStun);
}

int CCustomPet::GetCustomPetBlockStuck(int ItemIndex) // OK
{
	CUSTOM_PET_INFO* lpInfo = this->GetInfoByItem(ItemIndex);

	if (lpInfo == 0)
	{
		return 0;
	}

	return (lpInfo->IncBlockStuck);
}

int CCustomPet::GetCustomPetImortalPet(int ItemIndex) // OK
{
	CUSTOM_PET_INFO* lpInfo = this->GetInfoByItem(ItemIndex);

	if (lpInfo == 0)
	{
		return 0;
	}

	return (lpInfo->IncImortalPet);
}

void CCustomPet::CalcCustomPetOption(LPOBJ lpObj, bool flag)
{
	if (flag != 0)
	{
		return;
	}

	CItem* Helper = &lpObj->Inventory[8];

	if (this->CheckCustomPetByItem(Helper->m_Index) != 0)
	{
		lpObj->ExperienceRate += GetCustomPetExperienceRate(Helper->m_Index);
		lpObj->MasterExperienceRate += GetCustomPetMasterExperienceRate(Helper->m_Index);
		lpObj->AddMana += GetCustomPetMana(Helper->m_Index);
		lpObj->AttackSuccessRate += GetCustomPetDamageRate(Helper->m_Index);
		lpObj->DefenseSuccessRate += GetCustomPetDefenseRate(Helper->m_Index);
		lpObj->PhysiSpeed += GetCustomPetAttackSpeed(Helper->m_Index);
		lpObj->MagicSpeed += GetCustomPetAttackSpeed(Helper->m_Index);
		lpObj->DamageReflect += GetCustomPetDamageReflect(Helper->m_Index);
		lpObj->FullDamageReflectRate += GetCustomPetFullDamageReflect(Helper->m_Index);
		lpObj->CriticalDamageRate += GetCustomPetCriticalDamageRate(Helper->m_Index);
		lpObj->CriticalDamage += GetCustomPetCriticalDamage(Helper->m_Index);
		lpObj->ExcellentDamageRate += GetCustomPetExcellentDamageRate(Helper->m_Index);
		lpObj->ExcellentDamage += GetCustomPetExcellentDamage(Helper->m_Index);
		lpObj->DoubleDamageRate += GetCustomPetDoubleDamageRate(Helper->m_Index);
		lpObj->TripleDamageRate += GetCustomPetTripleDamageRate(Helper->m_Index);
		lpObj->DamageReduction[DAMAGE_REDUCTION_EXCELLENT_ITEM] += GetCustomPetDamageReduction(Helper->m_Index);
		lpObj->IgnoreDefenseRate += GetCustomPetIgnoreDefenseRate(Helper->m_Index);
		lpObj->ResistCriticalDamageRate += GetCustomPetResistCriticalDamage(Helper->m_Index);
		lpObj->ResistExcellentDamageRate += GetCustomPetResistExcellentDamage(Helper->m_Index);
		lpObj->ResistDoubleDamageRate += GetCustomPetResistDoubleDamage(Helper->m_Index);
		lpObj->ResistIgnoreDefenseRate += GetCustomPetResistIgnoreDefense(Helper->m_Index);
		lpObj->ResistStunRate += GetCustomPetResistStun(Helper->m_Index);
	}
}
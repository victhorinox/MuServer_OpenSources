#include "stdafx.h"
#include "Util.h"
#include "ItemManager.h"
#include "Message.h"
#include "User.h"
#include "Path.h"
#include "MemScript.h"
#include "RandomManager.h"
#include "CustomMix.h"
#include "Log.h"

CCustomMix gCustomMix;

void CCustomMix::Load(char* path)
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

	this->m_count1 = 0;
	this->m_count2 = 0;
	this->m_count3 = 0;
	this->m_count4 = 0;

	for(int n=0;n < MAX_MIX;n++)
	{
		this->m_Data_Mix[n];
		this->m_Data_Bag[n];
		this->m_Data_Item[n];
		this->m_Data_Result[n];
	}

	try
	{
		while(true)
		{
			if(lpMemScript->GetToken() == TOKEN_END)
			{
				break;
			}
		
			int section = lpMemScript->GetNumber();

			while(true)
			{
				if(section == 0)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					this->m_Data_Mix[this->m_count1].m_Index = lpMemScript->GetNumber();

					this->m_Data_Mix[this->m_count1].m_MixMoney = lpMemScript->GetAsNumber();

					this->m_Data_Mix[this->m_count1].m_MixRate_AL0 = lpMemScript->GetAsNumber();

					this->m_Data_Mix[this->m_count1].m_MixRate_AL1 = lpMemScript->GetAsNumber();

					this->m_Data_Mix[this->m_count1].m_MixRate_AL2 = lpMemScript->GetAsNumber();

					this->m_Data_Mix[this->m_count1].m_MixRate_AL3 = lpMemScript->GetAsNumber();

					this->m_Data_Mix[this->m_count1].m_CountGroup = lpMemScript->GetAsNumber();

					this->m_Data_Mix[this->m_count1].m_CountItem = lpMemScript->GetAsNumber();

					this->m_count1++;
				}
				else if(section == 1)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					this->m_Data_Bag[this->m_count2].m_Index = lpMemScript->GetNumber();

					this->m_Data_Bag[this->m_count2].m_ItemMix = lpMemScript->GetAsNumber();

					this->m_Data_Bag[this->m_count2].m_ItemLevel = lpMemScript->GetAsNumber();

					this->m_Data_Bag[this->m_count2].m_Count = lpMemScript->GetAsNumber();

					this->m_count2++;
				}
				else if(section == 2)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					this->m_Data_Item[this->m_count3].m_Index = lpMemScript->GetNumber();

					this->m_Data_Item[this->m_count3].m_Group = lpMemScript->GetAsNumber();
					
					this->m_Data_Item[this->m_count3].m_Count = lpMemScript->GetAsNumber();

					this->m_Data_Item[this->m_count3].m_ItemIndexMin = lpMemScript->GetAsNumber();

					this->m_Data_Item[this->m_count3].m_ItemIndexMax = lpMemScript->GetAsNumber();

					this->m_Data_Item[this->m_count3].m_ItemLevel = lpMemScript->GetAsNumber();

					this->m_Data_Item[this->m_count3].m_Skill = lpMemScript->GetAsNumber();

					this->m_Data_Item[this->m_count3].m_Luck = lpMemScript->GetAsNumber();

					this->m_Data_Item[this->m_count3].m_Opcion = lpMemScript->GetAsNumber();
					
					this->m_Data_Item[this->m_count3].m_Excelent = lpMemScript->GetAsNumber();

					this->m_Data_Item[this->m_count3].m_SetACC = lpMemScript->GetAsNumber();

					this->m_count3++;
				}
				else if(section == 3)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					this->m_Data_Result[this->m_count4].m_Index = lpMemScript->GetNumber();

					this->m_Data_Result[this->m_count4].m_Group = lpMemScript->GetAsNumber();

					this->m_Data_Result[this->m_count4].m_ItemIndexMin = lpMemScript->GetAsNumber();

					this->m_Data_Result[this->m_count4].m_ItemIndexMax = lpMemScript->GetAsNumber();

					this->m_Data_Result[this->m_count4].m_ItemLevel = lpMemScript->GetAsNumber();

					this->m_Data_Result[this->m_count4].m_Skill = lpMemScript->GetAsNumber();

					this->m_Data_Result[this->m_count4].m_Luck = lpMemScript->GetAsNumber();

					this->m_Data_Result[this->m_count4].m_Opcion = lpMemScript->GetAsNumber();
					
					this->m_Data_Result[this->m_count4].m_Excelent = lpMemScript->GetAsNumber();

					this->m_Data_Result[this->m_count4].m_SetACC = lpMemScript->GetAsNumber();

					this->m_count4++;
				}
				else
				{
					break;
				}
			}
		}
	}
	catch(...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;
}

bool CCustomMix::istItemBagMix(int IndexMix, CItem boxItem){

	for(int n=0;n < MAX_MIX;n++)
	{
		if(this->m_Data_Bag[n].m_Index != IndexMix)
		{
			continue;
		}
		if(this->m_Data_Bag[n].m_Index == IndexMix 
			&& this->m_Data_Bag[n].m_ItemMix == boxItem.m_Index
			&& this->m_Data_Bag[n].m_ItemLevel == boxItem.m_Level)
		{
			return true;
		}
	}
	return false;
}
//---
bool CCustomMix::istItemMix(int IndexMix, int group, CItem boxItem){

	for(int n=0;n < MAX_MIX;n++)
	{
		if(this->m_Data_Item[n].m_Index != IndexMix)
		{
			continue;
		}

		if(this->m_Data_Item[n].m_Index == IndexMix && this->m_Data_Item[n].m_Group == group
			&& (boxItem.m_Index >= this->m_Data_Item[n].m_ItemIndexMin && boxItem.m_Index <= this->m_Data_Item[n].m_ItemIndexMax))
		{
			if(this->m_Data_Item[n].m_ItemLevel != -1){
				if(boxItem.m_Level < this->m_Data_Item[n].m_ItemLevel){
					return false;
				}
			}

			if(this->m_Data_Item[n].m_Skill != -1){
				if(this->m_Data_Item[n].m_Skill != boxItem.m_Option1){
					return false;
				}
			}

			if(this->m_Data_Item[n].m_Luck != -1){
				if(this->m_Data_Item[n].m_Luck != boxItem.m_Option2){
					return false;
				}
			}
			if(this->m_Data_Item[n].m_Opcion != -1){
				if(this->m_Data_Item[n].m_Opcion > boxItem.m_Option3){
					return false;
				}
			}

			if(this->m_Data_Item[n].m_Excelent != -1){
				if(this->m_Data_Item[n].m_Excelent != boxItem.m_NewOption){
					return false;
				}
			}

			if(this->m_Data_Item[n].m_SetACC != -1){
				if(this->m_Data_Item[n].m_SetACC != boxItem.m_SetOption){
					return false;
				}
			}

			return true;
		}	
	}

	return false;
}

int CCustomMix::GetCountItemBagMix(int IndexMix){

	int itemcount = 0;

	for(int n=0;n < MAX_MIX;n++)
	{
		if(this->m_Data_Bag[n].m_Index != IndexMix)
		{
			continue;
		}

		if(this->m_Data_Bag[n].m_Index == IndexMix)
		{
			itemcount+=this->m_Data_Bag[n].m_Count;
		}	
	}
	return itemcount;
}

int CCustomMix::GetCountItemMix(int IndexMix,int Group){

	int itemcount = 0;

	for(int n=0;n < MAX_MIX;n++)
	{
		if(this->m_Data_Item[n].m_Index != IndexMix)
		{
			continue;
		}

		if(this->m_Data_Item[n].m_Index == IndexMix && this->m_Data_Item[n].m_Group == Group)
		{
			itemcount+=this->m_Data_Item[n].m_Count;
		}	
	}
	return itemcount;
}

//---
CUSTOM_MIX* CCustomMix::GetCustomMix(int IndexMix){

	for(int n=0;n < MAX_CUSTOM_MIX ;n++)
	{
		if(this->m_Data_Mix[n].m_Index == IndexMix)
		{
			return &this->m_Data_Mix[n];
		}	
	}

	return 0;
}

CRandomManager CCustomMix::RandomManager(int IndexMix,int group)
{
	CRandomManager RandomManager;

	for(int n = 0 ; n < MAX_MIX ; n++)
	{
		if(this->m_Data_Result[n].m_Index != IndexMix)
		{
			continue;
		}

		if(this->m_Data_Result[n].m_Index == IndexMix && this->m_Data_Result[n].m_Group == group)
		{
			for(int ItemIndex = this->m_Data_Result[n].m_ItemIndexMin; ItemIndex <=this->m_Data_Result[n].m_ItemIndexMax ; ItemIndex++){
				RandomManager.AddElement(ItemIndex,1);
			}			
		}	
	}
	return RandomManager;
}
//---
CUSTOM_ITEM_MIX_RESULT* CCustomMix::GetItemResult(int IndexMix,int group,int ItemIndex)
{
	for(int n=0;n < MAX_MIX;n++)
	{
		if(this->m_Data_Result[n].m_Index != IndexMix)
		{
			continue;
		}

		if(this->m_Data_Result[n].m_Index == IndexMix 
			&& this->m_Data_Result[n].m_Group == group 
			&& (ItemIndex >= this->m_Data_Result[n].m_ItemIndexMin && ItemIndex <= this->m_Data_Result[n].m_ItemIndexMax))
		{
			return &this->m_Data_Result[n];
		}
	}
	return 0;
}
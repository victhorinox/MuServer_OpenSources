#include "stdafx.h"
#include "CustomPKFree.h"
#include "DefaultClassInfo.h"
#include "Map.h"
#include "MemScript.h"
#include "Util.h"

cCPKFree gPKFree;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

cCPKFree::cCPKFree() // OK
{
	this->m_PKFreeInfo.clear();
}

cCPKFree::~cCPKFree() // OK
{

}

void cCPKFree::Load(char* path) // OK
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

	this->m_PKFreeInfo.clear();

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

			MOVE_PKFREE_INFO info;

			info.Map = lpMemScript->GetNumber();

			info.X = lpMemScript->GetAsNumber();

			info.Y = lpMemScript->GetAsNumber();

			info.TX = lpMemScript->GetAsNumber();

			info.TY = lpMemScript->GetAsNumber();

			this->m_PKFreeInfo.push_back(info);
		}
	}
	catch(...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;
}

bool cCPKFree::CheckPKFree(int map,int x,int y) // OK
{


	for(std::vector<MOVE_PKFREE_INFO>::iterator it=this->m_PKFreeInfo.begin();it != this->m_PKFreeInfo.end();it++)
	{
		if(it->Map != map)
		{
			continue;
		}

		if((it->X > x || it->TX < x) || (it->Y > y || it->TY < y))
		{
			continue;
		}
		else
		{
			return 1;
		}
	}

	return 0;
}
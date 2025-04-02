#pragma once
//----
#include "stdafx.h"
#include "User.h"
#include "Protocol.h"
//----
#define MAX_CUSTOM_BANK 11

struct BANKEX_GD_SAVE_POINT
{
	PSBMSG_HEAD	h;
	// ----
	WORD	UserIndex;
	char	AccountID[11];
	long	Money;
	long	Bless;
	long	Soul;
	long	Chaos;
	long	Creation;
	long	Life;
	long	Harmony;
	long	Guardian;
	long	Gemstone;
	long	LowRefine;
	long	HigRefine;
	//customs
	long	Custom001;
	long	Custom002;
	long	Custom003;
	long	Custom004;
	long	Custom005;
	long	Custom006;
	long	Custom007;
	long	Custom008;
	long	Custom009;
	long	Custom010;
	long	Custom011;
};
// -------------------------------------------------------------------------------

struct BANKEX_GD_REQ_POINT
{
	PSBMSG_HEAD	h;
	// ----
	WORD	UserIndex;
	char	AccountID[11];
};
// -------------------------------------------------------------------------------

struct BANKEX_DG_GET_POINT
{
	PSBMSG_HEAD	h;
	// ----
	WORD	UserIndex;
	long	Money;
	long	Bless;
	long	Soul;
	long	Chaos;
	long	Creation;
	long	Life;
	long	Harmony;
	long	Guardian;
	long	Gemstone;
	long	LowRefine;
	long	HigRefine;
	//customs
	long	Custom001;
	long	Custom002;
	long	Custom003;
	long	Custom004;
	long	Custom005;
	long	Custom006;
	long	Custom007;
	long	Custom008;
	long	Custom009;
	long	Custom010;
	long	Custom011;
};

struct PMSG_BANKEX_UPDATE_SEND
{
	PSBMSG_HEAD header;
	long	Money;
	long	Bless;
	long	Soul;
	long	Chaos;
	long	Creation;
	long	Life;
	long	Harmony;
	long	Guardian;
	long	Gemstone;
	long	LowRefine;
	long	HigRefine;
	int		MaxWarehouse;
	//customs
	long	Custom001;
	long	Custom002;
	long	Custom003;
	long	Custom004;
	long	Custom005;
	long	Custom006;
	long	Custom007;
	long	Custom008;
	long	Custom009;
	long	Custom010;
	long	Custom011;
};

struct PMSG_JEWELBANK
{
	PBMSG_HEAD2 h;
	int Result;
};
// -------------------------------------------------------------------------------
struct JEWEL_BANK
{
	int Index;
	int ID;
	int ItemID;
	char ItemName[50];
};

enum eBankExType
{
	BANKEX_BLESS = 0,
	BANKEX_SOUL = 1,
	BANKEX_CHAOS = 2,
	BANKEX_CREATION = 3,
	BANKEX_LIFE = 4,
	BANKEX_HARMONY = 5,
	BANKEX_GUARDIAN = 6,
	BANKEX_GEMSTONE = 7,
	BANKEX_LOWERREF = 8,
	BANKEX_HIGHREF = 9,
};

class CBankEx
{
public:
	CBankEx();
	virtual ~CBankEx();
	void Init();
	void Load(char* path);
	void SetInfo(JEWEL_BANK info);
	void InitUser(LPOBJ lpObj);
	void GDSavePoint(int aIndex);
	void GDReqPoint(int aIndex);
	void DGGetPoint(BANKEX_DG_GET_POINT * aRecv);
	void GCUpdateBankEx(int aIndex);
	//comandos novos
	//CUSTOM JEWELBANK
	void SendZenCustomVault(LPOBJ lpObj,char* arg);
	void RecvZenCustomVault(LPOBJ lpObj,char* arg);
	void SendBlessCustomVault(LPOBJ lpObj,char* arg);
	void RecvBlessCustomVault(LPOBJ lpObj,char* arg);
	void SendSoulCustomVault(LPOBJ lpObj,char* arg);
	void RecvSoulCustomVault(LPOBJ lpObj,char* arg);
	void SendChaosCustomVault(LPOBJ lpObj,char* arg);
	void RecvChaosCustomVault(LPOBJ lpObj,char* arg);
	void SendLifeCustomVault(LPOBJ lpObj,char* arg);
	void RecvLifeCustomVault(LPOBJ lpObj,char* arg);
	void SendCreationCustomVault(LPOBJ lpObj,char* arg);
	void RecvCreationCustomVault(LPOBJ lpObj,char* arg);
	//novas
	void RecvHarmonyCustomVault(LPOBJ lpObj,char* arg);
	void SendHarmonyCustomVault(LPOBJ lpObj,char* arg);
	void RecvGuardianCustomVault(LPOBJ lpObj,char* arg);
	void SendGuardianCustomVault(LPOBJ lpObj,char* arg);
	void RecvGemstoneCustomVault(LPOBJ lpObj,char* arg);
	void SendGemstoneCustomVault(LPOBJ lpObj,char* arg);
	void RecvLowerRefinCustomVault(LPOBJ lpObj,char* arg);
	void SendLowerRefinCustomVault(LPOBJ lpObj,char* arg);
	void RecvHighRefinCustomVault(LPOBJ lpObj,char* arg);
	void SendHighRefinCustomVault(LPOBJ lpObj,char* arg);
private:
	JEWEL_BANK m_JewelBank[MAX_CUSTOM_BANK];
}
; extern CBankEx gBankEx;
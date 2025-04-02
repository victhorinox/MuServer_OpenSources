// CustomAttack.h: interface for the CCustomAttack class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "User.h"
#include "Protocol.h"

struct SDHP_CARESUME_SEND
{
    PBMSG_HEAD header; // C1:F5
    WORD index;
    char name[11];
};

struct SDHP_CARESUME_SAVE_SEND
{
    PBMSG_HEAD header; // C1:F6
    WORD index;
    char name[11];
	WORD active;
	WORD skill;
	WORD map;
	WORD posx;
	WORD posy;
	WORD autobuff;
	WORD offpvp;
	WORD autoreset;
	DWORD autoaddstr;
	DWORD autoaddagi;
	DWORD autoaddvit;
	DWORD autoaddene;
	DWORD autoaddcmd;
};

struct SDHP_CARESUME_RECV
{
    PBMSG_HEAD header; // C1:F5
    WORD index;
    char name[11];
	WORD active;
	WORD skill;
	WORD map;
	WORD posx;
	WORD posy;
	WORD autobuff;
	WORD offpvp;
	WORD autoreset;
	DWORD autoaddstr;
	DWORD autoaddagi;
	DWORD autoaddvit;
	DWORD autoaddene;
	DWORD autoaddcmd;
};

class CCustomAttack
{
public:
	CCustomAttack();
	virtual ~CCustomAttack();
	void ReadCustomAttackInfo(char* section,char* path);
	bool CommandCustomAttack(LPOBJ lpObj,char* arg);
	bool CommandCustomAttackOffline(LPOBJ lpObj,char* arg);
	bool GetAttackSkill(LPOBJ lpObj,int* SkillNumber,int SetSkill);
	bool GetTargetMonster(LPOBJ lpObj,int SkillNumber,int* MonsterIndex);
	void OnAttackClose(LPOBJ lpObj);
	void OnAttackSecondProc(LPOBJ lpObj);
	void OnAttackAlreadyConnected(LPOBJ lpObj);
	void OnAttackMonsterAndMsgProc(LPOBJ lpObj);
	void SendSkillAttack(LPOBJ lpObj,int aIndex,int SkillNumber);
	void SendMultiSkillAttack(LPOBJ lpObj,int aIndex,int SkillNumber);
	void SendDurationSkillAttack(LPOBJ lpObj,int aIndex,int SkillNumber);
	void SendRFSkillAttack(LPOBJ lpObj,int aIndex,int SkillNumber);
	void GetTargetParty(LPOBJ lpObj,int SkillNumber);
	void DGCustomAttackResumeSend(int aIndex);
	void DGCustomAttackResumeRecv(SDHP_CARESUME_RECV* lpMsg);
	void DGCustomAttackResumeSaveSend(int Index);
public:
	//int m_CustomAttackSwitch;
	//int m_CustomAttackEnable[4];
	int m_CustomAttackTime[4];
	//int m_CustomAttackRequireLevel[4];
	//int m_CustomAttackRequireReset[4];
	//char m_CustomAttackCommandSyntax[32];
	//char m_CustomAttackText1[128];
	//char m_CustomAttackText2[128];
	//char m_CustomAttackText3[128];
	//char m_CustomAttackText4[128];
	//char m_CustomAttackText5[128];
	//char m_CustomAttackText6[128];
	//char m_CustomAttackText7[128];
	//char m_CustomAttackText8[128];
	//char m_CustomAttackText9[128];
	//int m_CustomAttackOfflineSwitch;
	int m_CustomAttackOfflineGPGain;
	//int m_CustomAttackOfflineEnable[4];
	//int m_CustomAttackOfflineRequireLevel[4];
	//int m_CustomAttackOfflineRequireReset[4];
	//int m_CustomAttackOfflineRequireMoney[4];
	//char m_CustomAttackOfflineCommandSyntax[32];
	//char m_CustomAttackOfflineText1[128];
	//char m_CustomAttackOfflineText2[128];
	//char m_CustomAttackOfflineText3[128];
	//char m_CustomAttackOfflineText4[128];
	//char m_CustomAttackOfflineText5[128];
	//char m_CustomAttackOfflineText6[128];
	int m_CustomAttackDelay;
	int m_CustomAttackPotionDelay;
	int m_CustomAttackAutoBuff[4];
	int m_CustomAttackAutoBuffDelay;
	int m_CustomAttackAutoResume[4];
};

extern CCustomAttack gCustomAttack;

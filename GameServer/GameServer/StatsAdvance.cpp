#include "StdAfx.h"
#include "user.h"
#include "protocol.h"
#include "StatsAdvance.h"
//#include "SocketManager.h"
#include "Util.h"


CStatsAdvance g_StatsAdvance;

CStatsAdvance::CStatsAdvance()
{
	this->m_Enable = true;
}

CStatsAdvance::~CStatsAdvance()
{
}

void CStatsAdvance::Load()
{

	this->m_Enable = true;

}

void CStatsAdvance::Send(int aIndex)
{
	if (!this->m_Enable)
	{
		return;
	}

	if (!OBJMAX_RANGE(aIndex))
	{
		return;
	}

	LPOBJ lpUser = &gObj[aIndex];

	PMSG_STATS_ADVANCE pMsg;

	pMsg.h.set(0xF3, 0xEE, sizeof(pMsg));
	pMsg.Result = true;
	pMsg.StatInfo.Level = lpUser->Level;
	pMsg.StatInfo.MasterLevel = lpUser->MasterLevel;
	pMsg.StatInfo.Reset = lpUser->Reset;
	pMsg.StatInfo.GrandReset = lpUser->MasterReset;
	pMsg.StatInfo.TotalDamageReflect = lpUser->DamageReflect;
	pMsg.StatInfo.FullDamageReflectRate = lpUser->FullDamageReflectRate;
	pMsg.StatInfo.CriticalDamageRate = lpUser->CriticalDamageRate;
	pMsg.StatInfo.CriticalDamagePower = lpUser->CriticalDamage;
	pMsg.StatInfo.ExellentDamageRate = lpUser->ExcellentDamageRate;
	pMsg.StatInfo.ExellentDamagePower = lpUser->ExcellentDamage;
	pMsg.StatInfo.DoubleDamageRate = lpUser->DoubleDamageRate;
	pMsg.StatInfo.TripleDamageRate = lpUser->TripleDamageRate;
	pMsg.StatInfo.DamageReductionRate = lpUser->DamageReduction[DAMAGE_REDUCTION_EXCELLENT_ITEM] + lpUser->DamageReduction[DAMAGE_REDUCTION_SOCKET_ITEM];
	pMsg.StatInfo.ShieldSkillDamageReductionRate = lpUser->ShieldDamageReduction;
	pMsg.StatInfo.SDDamageReductionRate = lpUser->DamageReduction[DAMAGE_REDUCTION_JOH_ITEM];
	pMsg.StatInfo.SDDecreaseDamageRate = lpUser->DecreaseShieldGaugeRate;
	pMsg.StatInfo.IgnoreDefenceRate = lpUser->IgnoreDefenseRate;
	pMsg.StatInfo.IgnoreSDRate = lpUser->IgnoreShieldGaugeRate;
	pMsg.StatInfo.IncreaseDamagePvP = lpUser->AttackSuccessRatePvP;
	pMsg.StatInfo.IncreaseDefencePvP = lpUser->DefenseSuccessRatePvP;
	pMsg.StatInfo.ResistCriticalDamageRate = lpUser->ResistCriticalDamageRate;
	pMsg.StatInfo.ResistExellentDamageRate = lpUser->ResistExcellentDamageRate;
	pMsg.StatInfo.ResistDoubleDamageRate = lpUser->ResistDoubleDamageRate;
	pMsg.StatInfo.ResistIgnoreDefenceRate = lpUser->ResistIgnoreDefenseRate;
	pMsg.StatInfo.ResistIgnoreSDRate = lpUser->ResistIgnoreShieldGaugeRate;
	pMsg.StatInfo.ResistStumRate = lpUser->ResistStunRate;
	pMsg.StatInfo.ResistIce = lpUser->Resistance[1];
	pMsg.StatInfo.ResistPoison = lpUser->Resistance[2];
	pMsg.StatInfo.ResistLighting = lpUser->Resistance[3];
	pMsg.StatInfo.ResistFire = lpUser->Resistance[4];
	pMsg.StatInfo.ResistEarth = lpUser->Resistance[5];
	pMsg.StatInfo.ResistWind = lpUser->Resistance[6];
	pMsg.StatInfo.ResistWater = lpUser->Resistance[7];
	pMsg.StatInfo.FullHPRestoreRate = lpUser->DefensiveFullHPRestoreRate;
	pMsg.StatInfo.FullMPRestoreRate = lpUser->DefensiveFullMPRestoreRate;
	pMsg.StatInfo.FullSDRestoreRate = lpUser->DefensiveFullSDRestoreRate;
	DataSend(aIndex, (LPBYTE)&pMsg, pMsg.h.size);
}
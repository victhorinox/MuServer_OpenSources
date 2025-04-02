#pragma once

struct STATS_ADVANCE_DATA
{
	//int WX;
	int Level;
	int MasterLevel;
	int Reset;
	int GrandReset;
	//int Defense;
	int TotalDamageReflect;
	int FullDamageReflectRate;
	int CriticalDamageRate;
	int CriticalDamagePower;
	int ExellentDamageRate;
	int ExellentDamagePower;
	int DoubleDamageRate;
	int TripleDamageRate;
	int DamageReductionRate;
	int ShieldSkillDamageReductionRate;
	int SDDamageReductionRate;
	int SDDecreaseDamageRate;
	int IgnoreDefenceRate;
	int IgnoreSDRate;
	int IncreaseDamagePvP;
	int IncreaseDefencePvP;
	int ResistCriticalDamageRate;
	int ResistExellentDamageRate;
	int ResistDoubleDamageRate;
	int ResistIgnoreDefenceRate;
	int ResistIgnoreSDRate;
	int ResistStumRate;
	int ResistIce;
	int ResistPoison;
	int ResistLighting;
	int ResistFire;
	int ResistEarth;
	int ResistWind;
	int ResistWater;
	int FullHPRestoreRate;
	int FullMPRestoreRate;
	int FullSDRestoreRate;
};

struct PMSG_STATS_ADVANCE
{
	PSBMSG_HEAD h;
	BYTE Result;
	STATS_ADVANCE_DATA StatInfo;
};

class CStatsAdvance
{
public:
	CStatsAdvance();
	~CStatsAdvance();

	void Load();
	void Send(int aIndex);

	int m_Enable;
};

extern CStatsAdvance g_StatsAdvance;

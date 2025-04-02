#pragma once


struct CUSTOM_DISABLE_SKILL
{
	int SkillNumber;
	int General;
	int Guild;
	int Duel;
	int Player;
};

class CCustomDisableSkill
{
public:
	CCustomDisableSkill();
	virtual~CCustomDisableSkill();
	void Load(char* path);
	bool GetSkillDisableGeneral(int Skill);
	bool GetSkillDisableGuild(int Skill);
	bool GetSkillDisableDuel(int Skill);
	bool GetSkillDisablePlayer(int Skill);
private:
	std::vector<CUSTOM_DISABLE_SKILL>m_DisableSkillInfo;

};
extern CCustomDisableSkill gCustomDisableSkill;
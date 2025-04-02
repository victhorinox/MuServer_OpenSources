#pragma once

#define MAX_CLASS 7

struct GIFT_INFO
{
	int Class;
	int Session;
	int ItemID;
	int Level;
	int Duration;
	int Skill;
	int Luck;
	int Option;
	int Excellent;
	int	Time;
};

struct BUFF_INFO
{
	int Class;
	int EffectID;
	int Power1;
	int Power2;
	int Time;
};

class CGift
{
public:
	CGift();
	virtual ~CGift();
	void Init();
	void Load(char* path);
	void GiftItem(LPOBJ lpObj);
private:
	std::vector<GIFT_INFO> m_GiftInfo;
	std::vector<BUFF_INFO> m_BuffInfo;
};

extern CGift gGiftNew;

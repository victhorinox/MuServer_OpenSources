// ChaosBox.h: interface for the CChaosMix class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "ItemManager.h"
#include "Protocol.h"
#include "User.h"

#define MAX_TALISMAN_OF_LUCK 10

enum eChaosMixNumber
{
	CHAOS_MIX_CHAOS_ITEM = 1,
	CHAOS_MIX_DEVIL_SQUARE = 2,
	CHAOS_MIX_PLUS_ITEM_LEVEL1 = 3,
	CHAOS_MIX_PLUS_ITEM_LEVEL2 = 4,
	CHAOS_MIX_DINORANT = 5,
	CHAOS_MIX_FRUIT = 6,
	CHAOS_MIX_WING1 = 7,
	CHAOS_MIX_BLOOD_CASTLE = 8,
	CHAOS_MIX_WING2 = 11,
	CHAOS_MIX_PET1 = 13,
	CHAOS_MIX_PET2 = 14,
	CHAOS_MIX_SIEGE_POTION1 = 15,
	CHAOS_MIX_SIEGE_POTION2 = 16,
	CHAOS_MIX_LIFE_STONE = 17,
	CHAOS_MIX_SENIOR = 18,
	CHAOS_MIX_PLUS_ITEM_LEVEL3 = 22,
	CHAOS_MIX_PLUS_ITEM_LEVEL4 = 23,
	CHAOS_MIX_WING3 = 24,
	CHAOS_MIX_PIECE_OF_HORN = 25,
	CHAOS_MIX_BROKEN_HORN = 26,
	CHAOS_MIX_HORN_OF_FENRIR = 27,
	CHAOS_MIX_HORN_OF_FENRIR_UPGRADE = 28,
	CHAOS_MIX_SHIELD_POTION1 = 30,
	CHAOS_MIX_SHIELD_POTION2 = 31,
	CHAOS_MIX_SHIELD_POTION3 = 32,
	CHAOS_MIX_JEWEL_OF_HARMONY_ITEM_PURITY = 33,
	CHAOS_MIX_JEWEL_OF_HARMONY_ITEM_SMELT = 34,
	CHAOS_MIX_JEWEL_OF_HARMONY_ITEM_RESTORE = 35,
	CHAOS_MIX_ITEM_380 = 36,
	CHAOS_MIX_ILLUSION_TEMPLE = 37,
	CHAOS_MIX_FEATHER_OF_CONDOR = 38,
	CHAOS_MIX_WING4 = 39,
	CHAOS_MIX_CHAOS_CARD = 40,
	CHAOS_MIX_CHERRY_BLOSSOM = 41,
	CHAOS_MIX_SOCKET_ITEM_CREATE_SEED = 42,
	CHAOS_MIX_SOCKET_ITEM_CREATE_SEED_SPHERE = 43,
	CHAOS_MIX_SOCKET_ITEM_MOUNT_SEED_SPHERE = 44,
	CHAOS_MIX_SOCKET_ITEM_UN_MOUNT_SEED_SPHERE = 45,
	CHAOS_MIX_IMPERIAL_GUARDIAN = 46,
	CHAOS_MIX_CHEST = 47,
	CHAOS_MIX_SUMMON_SCROLL = 48,
	CHAOS_MIX_PLUS_ITEM_LEVEL5 = 49,
	CHAOS_MIX_PLUS_ITEM_LEVEL6 = 50,
	CHAOS_MIX_LUCKY_ITEM_CREATE = 51,
	CHAOS_MIX_LUCKY_ITEM_REFINE = 52,
	//-> Talisman Wing 1 Mix
	CHAOS_MIX_WING1DK = 53,
	CHAOS_MIX_WING1DW = 54,
	CHAOS_MIX_WING1FE = 55,
	CHAOS_MIX_WING1SU = 56,
	//-> Talisman Wing 2 Mix
	CHAOS_MIX_WING2DL = 57,
	CHAOS_MIX_WING2BK = 58,
	CHAOS_MIX_WING2SM = 59,
	CHAOS_MIX_WING2ME = 60,
	CHAOS_MIX_WING2BS = 61,
	CHAOS_MIX_WING2MG = 62,
	CHAOS_MIX_WING2RF = 63,
	//-> Talisman Wing 3 Mix
	CHAOS_MIX_WING3LE = 64,
	CHAOS_MIX_WING3BM = 65,
	CHAOS_MIX_WING3GM = 66,
	CHAOS_MIX_WING3HE = 67,
	CHAOS_MIX_WING3DM = 68,
	CHAOS_MIX_WING3SU = 69,
	CHAOS_MIX_WING3FM = 70,
	//-> Pet Mix
	CHAOS_MIX_SPIRIT_GUARDIAN = 71,
	CHAOS_MIX_DEMON = 72,
	CHAOS_MIX_GOLD_FENRIR = 73,
	//-> New Refine Harmony
	CHAOS_MIX_JEWEL_OF_HARMONY_ITEM_PURITY_10 = 74,
	CHAOS_MIX_JEWEL_OF_HARMONY_ITEM_PURITY_20 = 75,
	CHAOS_MIX_JEWEL_OF_HARMONY_ITEM_PURITY_30 = 76,
	//-> Wings 2.5
	CHAOS_MIX_MONSTER_WING = 77,
	//-> eX700 Socket Item Mix
	CHAOS_MIX_SOCKET_ITEM = 78,
	CHAOS_MIX_SOCKET_SET = 79,
	//-> Conqueror Wings Mix
	CHAOS_MIX_WINGS_OF_CONQUEROR = 80,
	//-> Create Ancient Hero Soul
	CHAOS_MIX_ANCIENT_HERO_SOUL = 81,
	//-> Bloodangel Set Mix
	CHAOS_MIX_BLOODANGEL_HELM = 82,
	CHAOS_MIX_BLOODANGEL_ARMOR = 83,
	CHAOS_MIX_BLOODANGEL_PANTS = 84,
	CHAOS_MIX_BLOODANGEL_GLOVES = 85,
	CHAOS_MIX_BLOODANGEL_BOOTS = 86,
	//-> Darkangel Set Mix
	CHAOS_MIX_DARKANGEL_HELM = 87,
	CHAOS_MIX_DARKANGEL_ARMOR = 88,
	CHAOS_MIX_DARKANGEL_PANTS = 89,
	CHAOS_MIX_DARKANGEL_GLOVES = 90,
	CHAOS_MIX_DARKANGEL_BOOTS = 91,
	//-> Darkangel Set Mix (Energy Elf)
	CHAOS_MIX_DARKANGEL_HELM_EE = 92,
	CHAOS_MIX_DARKANGEL_ARMOR_EE = 93,
	CHAOS_MIX_DARKANGEL_PANTS_EE = 94,
	CHAOS_MIX_DARKANGEL_GLOVES_EE = 95,
	//-> Darkangel Set Mix (Energy Magic)
	CHAOS_MIX_DARKANGEL_ARMOR_EMG = 96,
	CHAOS_MIX_DARKANGEL_PANTS_EMG = 97,
	CHAOS_MIX_DARKANGEL_GLOVES_EMG = 98,
	CHAOS_MIX_DARKANGEL_BOOTS_EMG = 99,
	//-> Holyangel Set Mix
	CHAOS_MIX_HOLYANGEL_HELM = 100,
	CHAOS_MIX_HOLYANGEL_ARMOR = 101,
	CHAOS_MIX_HOLYANGEL_PANTS = 102,
	CHAOS_MIX_HOLYANGEL_GLOVES = 103,
	CHAOS_MIX_HOLYANGEL_BOOTS = 104,
	//-> Holyangel Set Mix (Energy Elf)
	CHAOS_MIX_HOLYANGEL_HELM_EE = 105,
	CHAOS_MIX_HOLYANGEL_ARMOR_EE = 106,
	CHAOS_MIX_HOLYANGEL_PANTS_EE = 107,
	CHAOS_MIX_HOLYANGEL_GLOVES_EE = 108,
	//-> Holyangel Set Mix (Energy Magic)
	CHAOS_MIX_HOLYANGEL_ARMOR_EMG = 109,
	CHAOS_MIX_HOLYANGEL_PANTS_EMG = 110,
	CHAOS_MIX_HOLYANGEL_GLOVES_EMG = 111,
	CHAOS_MIX_HOLYANGEL_BOOTS_EMG = 112,
	//-> Awakening Set Mix
	CHAOS_MIX_AWAKENING_HELM = 113,
	CHAOS_MIX_AWAKENING_ARMOR = 114,
	CHAOS_MIX_AWAKENING_PANTS = 115,
	CHAOS_MIX_AWAKENING_GLOVES = 116,
	CHAOS_MIX_AWAKENING_BOOTS = 117,
	//-> Awakening Set Mix (Energy Elf)
	CHAOS_MIX_AWAKENING_HELM_EE = 118,
	CHAOS_MIX_AWAKENING_ARMOR_EE = 119,
	CHAOS_MIX_AWAKENING_PANTS_EE = 120,
	CHAOS_MIX_AWAKENING_GLOVES_EE = 121,
	//-> Awakening Set Mix (Energy Magic)
	CHAOS_MIX_AWAKENING_ARMOR_EMG = 122,
	CHAOS_MIX_AWAKENING_PANTS_EMG = 123,
	CHAOS_MIX_AWAKENING_GLOVES_EMG = 124,
	CHAOS_MIX_AWAKENING_BOOTS_EMG = 125,
	//-> Blue Eye Set Mix
	CHAOS_MIX_BLUE_EYE_HELM = 126,
	CHAOS_MIX_BLUE_EYE_ARMOR = 127,
	CHAOS_MIX_BLUE_EYE_PANTS = 128,
	CHAOS_MIX_BLUE_EYE_GLOVES = 129,
	CHAOS_MIX_BLUE_EYE_BOOTS = 130,
	//-> Fleet Silver Heart Set Mix
	CHAOS_MIX_FLEET_SILVER_HEART_HELM = 131,
	CHAOS_MIX_FLEET_SILVER_HEART_ARMOR = 132,
	CHAOS_MIX_FLEET_SILVER_HEART_PANTS = 133,
	CHAOS_MIX_FLEET_SILVER_HEART_GLOVES = 134,
	CHAOS_MIX_FLEET_SILVER_HEART_BOOTS = 135,
	//-> Roaring Manticore Set Mix
	CHAOS_MIX_ROARING_MANTICORE_HELM = 136,
	CHAOS_MIX_ROARING_MANTICORE_ARMOR = 137,
	CHAOS_MIX_ROARING_MANTICORE_PANTS = 138,
	CHAOS_MIX_ROARING_MANTICORE_GLOVES = 139,
	CHAOS_MIX_ROARING_MANTICORE_BOOTS = 140,
	//-> Create Hammer of the Archangel
	CHAOS_MIX_HAMMER_OF_ARCHANGEL = 141,
	//-> Blessed Divine Weapons Mix
	CHAOS_MIX_ABSOLUTE_SWORD_OF_ARCHANGEL = 142,
	CHAOS_MIX_ABSOLUTE_SCEPTER_OF_ARCHANGEL = 143,
	CHAOS_MIX_ABSOLUTE_CROSSBOW_OF_ARCHANGEL = 144,
	CHAOS_MIX_ABSOLUTE_STAFF_OF_ARCHANGEL = 145,
	CHAOS_MIX_ABSOLUTE_STICK_OF_ARCHANGEL = 146,
	CHAOS_MIX_ABSOLUTE_CLAW_OF_ARCHANGEL = 147,
	//-> Darkangel Weapon Mix
	CHAOS_MIX_DARKANGEL_SWORD = 148,
	CHAOS_MIX_DARKANGEL_MAGIC_SWORD = 149,
	CHAOS_MIX_DARKANGEL_CLAW = 150,
	CHAOS_MIX_DARKANGEL_SCEPTER = 151,
	CHAOS_MIX_DARKANGEL_BOW = 152,
	CHAOS_MIX_DARKANGEL_STAFF = 153,
	CHAOS_MIX_DARKANGEL_STICK = 154,
	//-> Holyangel Weapon Mix
	CHAOS_MIX_HOLYANGEL_SWORD = 155,
	CHAOS_MIX_HOLYANGEL_MAGIC_SWORD = 156,
	CHAOS_MIX_HOLYANGEL_CLAW = 157,
	CHAOS_MIX_HOLYANGEL_SCEPTER = 158,
	CHAOS_MIX_HOLYANGEL_BOW = 159,
	CHAOS_MIX_HOLYANGEL_STAFF = 160,
	CHAOS_MIX_HOLYANGEL_STICK = 161,
	//-> Awakening Soul Weapon Mix
	CHAOS_MIX_AWAKENING_SOUL_SWORD = 162,
	CHAOS_MIX_AWAKENING_SOUL_MAGIC_SWORD = 163,
	CHAOS_MIX_AWAKENING_SOUL_CLAW = 164,
	CHAOS_MIX_AWAKENING_SOUL_SCEPTER = 165,
	CHAOS_MIX_AWAKENING_SOUL_BOW = 166,
	CHAOS_MIX_AWAKENING_SOUL_STAFF = 167,
	CHAOS_MIX_AWAKENING_SOUL_STICK = 168,
	//-> Blue Eye Weapon Mix
	CHAOS_MIX_BLUE_EYE_SWORD = 169,
	CHAOS_MIX_BLUE_EYE_MAGIC_SWORD = 170,
	CHAOS_MIX_BLUE_EYE_CLAW = 171,
	CHAOS_MIX_BLUE_EYE_SCEPTER = 172,
	CHAOS_MIX_BLUE_EYE_BOW = 173,
	CHAOS_MIX_BLUE_EYE_STAFF = 174,
	CHAOS_MIX_BLUE_EYE_STICK = 175,
	//-> Silver Heart Weapon Mix
	CHAOS_MIX_SILVER_HEART_SWORD = 176,
	CHAOS_MIX_SILVER_HEART_MAGIC_SWORD = 177,
	CHAOS_MIX_SILVER_HEART_CLAW = 178,
	CHAOS_MIX_SILVER_HEART_SCEPTER = 179,
	CHAOS_MIX_SILVER_HEART_BOW = 180,
	CHAOS_MIX_SILVER_HEART_STAFF = 181,
	CHAOS_MIX_SILVER_HEART_STICK = 182,
	//--

/*
	//-> Sphere Upgrade Mix
	CHAOS_MIX_GREATER_SPHERE4 = 107,
	CHAOS_MIX_STANDARD_SPHERE4 = 108,
	CHAOS_MIX_MINOR_SPHERE4 = 109,
	CHAOS_MIX_GREATER_SPHERE5 = 110,
	CHAOS_MIX_STANDARD_SPHERE5 = 111,
	CHAOS_MIX_MINOR_SPHERE5 = 112,
*/

};

//**********************************************//
//************ Client -> GameServer ************//
//**********************************************//

struct PMSG_CHAOS_MIX_RECV
{
	PBMSG_HEAD header; // C1:86
	BYTE type;
	BYTE info;
};

//**********************************************//
//************ GameServer -> Client ************//
//**********************************************//

struct PMSG_CHAOS_MIX_SEND
{
	PBMSG_HEAD header; // C1:86
	BYTE result;
	BYTE ItemInfo[MAX_ITEM_INFO];
};

//**********************************************//
//**********************************************//
//**********************************************//

class CChaosBox
{
public:
	CChaosBox();
	virtual ~CChaosBox();
	void ChaosBoxInit(LPOBJ lpObj);
	void ChaosBoxItemDown(LPOBJ lpObj,int slot);
	void ChaosBoxItemKeep(LPOBJ lpObj,int slot);
	void ChaosBoxItemSave(LPOBJ lpObj);
	bool GetTalismanOfLuckRate(LPOBJ lpObj,int* rate);
	bool GetElementalTalismanOfLuckRate(LPOBJ lpObj,int* rate);
	void ChaosItemMix(LPOBJ lpObj);
	void DevilSquareMix(LPOBJ lpObj);
	void PlusItemLevelMix(LPOBJ lpObj,int type);
	void DinorantMix(LPOBJ lpObj);
	void FruitMix(LPOBJ lpObj);
	void Wing2Mix(LPOBJ lpObj,int type);
	void BloodCastleMix(LPOBJ lpObj);
	void Wing1Mix(LPOBJ lpObj);
	void PetMix(LPOBJ lpObj,int type);
	void SiegePotionMix(LPOBJ lpObj,int type);
	void LifeStoneMix(LPOBJ lpObj);
	void SeniorMix(LPOBJ lpObj);
	void PieceOfHornMix(LPOBJ lpObj);
	void BrokenHornMix(LPOBJ lpObj);
	void HornOfFenrirMix(LPOBJ lpObj);
	void HornOfFenrirUpgradeMix(LPOBJ lpObj);
	void ShieldPotionMix(LPOBJ lpObj,int type);
	void JewelOfHarmonyItemPurityMix(LPOBJ lpObj);
	void JewelOfHarmonyItemSmeltMix(LPOBJ lpObj);
	void JewelOfHarmonyItemRestoreMix(LPOBJ lpObj);
	void Item380Mix(LPOBJ lpObj);
	void IllusionTempleMix(LPOBJ lpObj);
	void FeatherOfCondorMix(LPOBJ lpObj);
	void Wing3Mix(LPOBJ lpObj);
	void ChaosCardMix(LPOBJ lpObj,int type);
	void CherryBlossomMix(LPOBJ lpObj,int type);
	void SocketItemCreateSeedMix(LPOBJ lpObj);
	void SocketItemCreateSeedSphereMix(LPOBJ lpObj);
	void SocketItemMountSeedSphereMix(LPOBJ lpObj,BYTE info);
	void SocketItemUnMountSeedSphereMix(LPOBJ lpObj,BYTE info);
	void ImperialGuardianMix(LPOBJ lpObj);
	void ChestMix(LPOBJ lpObj);
	void SummonScrollMix(LPOBJ lpObj,int type);
	void LuckyItemCreateMix(LPOBJ lpObj);
	void LuckyItemRefineMix(LPOBJ lpObj);
	//-> Monster Wings 2.5 Mix
	void MonsterWingMix(LPOBJ lpObj);
	//-> Socket Weapon/Shield Mix
	void SocketWeaponMix(LPOBJ lpObj);
	//-> Socket Set Upgrade
	void SocketHelmMix(LPOBJ lpObj);
	void SocketArmorMix(LPOBJ lpObj);
	void SocketPantMix(LPOBJ lpObj);
	void SocketGloveMix(LPOBJ lpObj);
	void SocketBootMix(LPOBJ lpObj);
	//-> Socket Set Upgrade (RF)
	void SocketRFHelmMix(LPOBJ lpObj);
	void SocketRFArmorMix(LPOBJ lpObj);
	void SocketRFPantMix(LPOBJ lpObj);
	void SocketRFBootMix(LPOBJ lpObj);
	//-> Talisman Wing 1 Mix
	void Wing1DKMix(LPOBJ lpObj);
	void Wing1DWMix(LPOBJ lpObj);
	void Wing1FEMix(LPOBJ lpObj);
	void Wing1SUMix(LPOBJ lpObj);
	//-> Talisman Wing 2 Mix
	void Wing2DLMix(LPOBJ lpObj);
	void Wing2BKMix(LPOBJ lpObj);
	void Wing2SMMix(LPOBJ lpObj);
	void Wing2MEMix(LPOBJ lpObj);
	void Wing2BSMix(LPOBJ lpObj);
	void Wing2MGMix(LPOBJ lpObj);
	void Wing2RFMix(LPOBJ lpObj);
	//-> Talisman Wing 3 Mix
	void Wing3LEMix(LPOBJ lpObj);
	void Wing3BMMix(LPOBJ lpObj);
	void Wing3GMMix(LPOBJ lpObj);
	void Wing3HEMix(LPOBJ lpObj);
	void Wing3DMMix(LPOBJ lpObj);
	void Wing3SUMix(LPOBJ lpObj);
	void Wing3FMMix(LPOBJ lpObj);
	//-> Pet Mix Mix
	void SpiritGuardianMix(LPOBJ lpObj);
	void DemonMix(LPOBJ lpObj);
	//-> Gold Fenrir Mix
	void GoldFenrirMix(LPOBJ lpObj);
	//-> Refine Jewel of Harmony Bundle
	void JewelOfHarmonyItemPurity10Mix(LPOBJ lpObj);
	void JewelOfHarmonyItemPurity20Mix(LPOBJ lpObj);
	void JewelOfHarmonyItemPurity30Mix(LPOBJ lpObj);
	//-> Wings of Conqueror Mix
	void WingsOfConquerorMix(LPOBJ lpObj);
	//-> Ancient Hero Soul Mix
	void AncientHeroSoulMix(LPOBJ lpObj);
	//-> Bloodangel Set Mix
	void BloodangelHelmMix(LPOBJ lpObj);
	void BloodangelArmorMix(LPOBJ lpObj);
	void BloodangelPantsMix(LPOBJ lpObj);
	void BloodangelGlovesMix(LPOBJ lpObj);
	void BloodangelBootsMix(LPOBJ lpObj);
	//-> Darkangel Set Mix
	void DarkangelHelmMix(LPOBJ lpObj);
	void DarkangelArmorMix(LPOBJ lpObj);
	void DarkangelPantsMix(LPOBJ lpObj);
	void DarkangelGlovesMix(LPOBJ lpObj);
	void DarkangelBootsMix(LPOBJ lpObj);
	//-> Darkangel Set Mix (Energy Elf)
	void DarkangelEnergyElfHelmMix(LPOBJ lpObj);
	void DarkangelEnergyElfArmorMix(LPOBJ lpObj);
	void DarkangelEnergyElfPantsMix(LPOBJ lpObj);
	void DarkangelEnergyElfGlovesMix(LPOBJ lpObj);
	//-> Darkangel Set Mix (Energy Magic)
	void DarkangelEnergyMagicArmorMix(LPOBJ lpObj);
	void DarkangelEnergyMagicPantsMix(LPOBJ lpObj);
	void DarkangelEnergyMagicGlovesMix(LPOBJ lpObj);
	void DarkangelEnergyMagicBootsMix(LPOBJ lpObj);
	//-> Holyangel Set Mix
	void HolyangelHelmMix(LPOBJ lpObj);
	void HolyangelArmorMix(LPOBJ lpObj);
	void HolyangelPantsMix(LPOBJ lpObj);
	void HolyangelGlovesMix(LPOBJ lpObj);
	void HolyangelBootsMix(LPOBJ lpObj);
	//-> Holyangel Set Mix (Energy Elf)
	void HolyangelEnergyElfHelmMix(LPOBJ lpObj);
	void HolyangelEnergyElfArmorMix(LPOBJ lpObj);
	void HolyangelEnergyElfPantsMix(LPOBJ lpObj);
	void HolyangelEnergyElfGlovesMix(LPOBJ lpObj);
	//-> Holyangel Set Mix (Energy Magic)
	void HolyangelEnergyMagicArmorMix(LPOBJ lpObj);
	void HolyangelEnergyMagicPantsMix(LPOBJ lpObj);
	void HolyangelEnergyMagicGlovesMix(LPOBJ lpObj);
	void HolyangelEnergyMagicBootsMix(LPOBJ lpObj);
	//-> Awakening Set Mix
	void AwakeningHelmMix(LPOBJ lpObj);
	void AwakeningArmorMix(LPOBJ lpObj);
	void AwakeningPantsMix(LPOBJ lpObj);
	void AwakeningGlovesMix(LPOBJ lpObj);
	void AwakeningBootsMix(LPOBJ lpObj);
	//-> Awakening Set Mix (Energy Elf)
	void AwakeningEnergyElfHelmMix(LPOBJ lpObj);
	void AwakeningEnergyElfArmorMix(LPOBJ lpObj);
	void AwakeningEnergyElfPantsMix(LPOBJ lpObj);
	void AwakeningEnergyElfGlovesMix(LPOBJ lpObj);
	//-> Awakening Set Mix (Energy Magic)
	void AwakeningEnergyMagicArmorMix(LPOBJ lpObj);
	void AwakeningEnergyMagicPantsMix(LPOBJ lpObj);
	void AwakeningEnergyMagicGlovesMix(LPOBJ lpObj);
	void AwakeningEnergyMagicBootsMix(LPOBJ lpObj);
	//-> Blue Eye Set Mix
	void BlueEyeHelmMix(LPOBJ lpObj);
	void BlueEyeArmorMix(LPOBJ lpObj);
	void BlueEyePantsMix(LPOBJ lpObj);
	void BlueEyeGlovesMix(LPOBJ lpObj);
	void BlueEyeBootsMix(LPOBJ lpObj);
	//-> Fleet Silver Heart Set Mix
	void FleetSilverHeartHelmMix(LPOBJ lpObj);
	void FleetSilverHeartArmorMix(LPOBJ lpObj);
	void FleetSilverHeartPantsMix(LPOBJ lpObj);
	void FleetSilverHeartGlovesMix(LPOBJ lpObj);
	void FleetSilverHeartBootsMix(LPOBJ lpObj);
	//-> Roaring Manticore Set Mix
	void RoaringManticoreHelmMix(LPOBJ lpObj);
	void RoaringManticoreArmorMix(LPOBJ lpObj);
	void RoaringManticorePantsMix(LPOBJ lpObj);
	void RoaringManticoreGlovesMix(LPOBJ lpObj);
	void RoaringManticoreBootsMix(LPOBJ lpObj);
	//-> Hammer of Archangel Mix
	void HammerArchangelMix(LPOBJ lpObj,int type);
	//-> Blessed Divine Weapons Mix
	void AbsoluteArchangelSwordMix(LPOBJ lpObj,int type);
	void AbsoluteArchangelScepterMix(LPOBJ lpObj,int type);
	void AbsoluteArchangelCrossbowMix(LPOBJ lpObj,int type);
	void AbsoluteArchangelStaffMix(LPOBJ lpObj,int type);
	void AbsoluteArchangelStickMix(LPOBJ lpObj,int type);
	void AbsoluteArchangelClawMix(LPOBJ lpObj,int type);
	//-> Darkangel Weapon Mix
	void DarkangelSwordMix(LPOBJ lpObj,int type);
	void DarkangelMagicSwordMix(LPOBJ lpObj,int type);
	void DarkangelClawMix(LPOBJ lpObj,int type);
	void DarkangelScepterMix(LPOBJ lpObj,int type);
	void DarkangelBowMix(LPOBJ lpObj,int type);
	void DarkangelStaffMix(LPOBJ lpObj,int type);
	void DarkangelStickMix(LPOBJ lpObj,int type);	
	void HolyangelSwordMix(LPOBJ lpObj,int type);
	//-> Holyangel Weapon Mix
	void HolyangelMagicSwordMix(LPOBJ lpObj,int type);
	void HolyangelClawMix(LPOBJ lpObj,int type);
	void HolyangelScepterMix(LPOBJ lpObj,int type);
	void HolyangelBowMix(LPOBJ lpObj,int type);
	void HolyangelStaffMix(LPOBJ lpObj,int type);
	void HolyangelStickMix(LPOBJ lpObj,int type);
	//-> Awakening Soul Weapon Mix
	void SoulSwordMix(LPOBJ lpObj,int type);
	void SoulMagicSwordMix(LPOBJ lpObj,int type);
	void SoulClawMix(LPOBJ lpObj,int type);
	void SoulScepterMix(LPOBJ lpObj,int type);
	void SoulBowMix(LPOBJ lpObj,int type);
	void SoulStaffMix(LPOBJ lpObj,int type);
	void SoulStickMix(LPOBJ lpObj,int type);
	//-> Blue Eye Weapon Mix
	void BlueEyeSwordMix(LPOBJ lpObj,int type);
	void BlueEyeMagicSwordMix(LPOBJ lpObj,int type);
	void BlueEyeClawMix(LPOBJ lpObj,int type);
	void BlueEyeScepterMix(LPOBJ lpObj,int type);
	void BlueEyeBowMix(LPOBJ lpObj,int type);
	void BlueEyeStaffMix(LPOBJ lpObj,int type);
	void BlueEyeStickMix(LPOBJ lpObj,int type);	
	//-> Silver Heart Weapon Mix
	void SilverHeartSwordMix(LPOBJ lpObj,int type);
	void SilverHeartMagicSwordMix(LPOBJ lpObj,int type);
	void SilverHeartClawMix(LPOBJ lpObj,int type);
	void SilverHeartScepterMix(LPOBJ lpObj,int type);
	void SilverHeartBowMix(LPOBJ lpObj,int type);
	void SilverHeartStaffMix(LPOBJ lpObj,int type);
	void SilverHeartStickMix(LPOBJ lpObj,int type);	
	//--
	void PentagramMithrilMix(LPOBJ lpObj);
	void PentagramElixirMix(LPOBJ lpObj);
	void PentagramJewelMix(LPOBJ lpObj);
	void PentagramDecompositeMix(LPOBJ lpObj,int type);
	void PentagramJewelUpgradeLevelMix(LPOBJ lpObj,BYTE info);
	void PentagramJewelUpgradeRankMix(LPOBJ lpObj,BYTE info);
	void CGChaosMixRecv(PMSG_CHAOS_MIX_RECV* lpMsg,int aIndex);
	void CGChaosMixCloseRecv(int aIndex);
	void GCChaosBoxSend(LPOBJ lpObj,BYTE type);
	void GCChaosMixSend(int aIndex,BYTE result,CItem* lpItem);
	void CustomItemMix(LPOBJ lpObj, int type);
public:
	int m_SeniorMixLimitDay;
	int m_SeniorMixLimitMonth;
	int m_SeniorMixLimitYear;
};

extern CChaosBox gChaosBox;

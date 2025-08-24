
#pragma once

#include "GamePtr.h"

namespace gameplay
{
    //------------------------- Skyrim.esm -------------------------------//
    static GamePtr<RE::BGSKeyword> vampire (0x914E5);          // 
    static GamePtr<RE::BGSKeyword> armorDragonscale(0x6BBD6);  //
    static GamePtr<RE::BGSKeyword> armorOrcit(0x6BBE5);        //
    static GamePtr<RE::BGSKeyword> armorLeather(0x6BBDB);      // 
    static GamePtr<RE::BGSKeyword> armorHide(0x6BBDD);         //
    static GamePtr<RE::BGSKeyword> armorScale(0x6BBDE);        //
    static GamePtr<RE::BGSKeyword> armorElven(0x6BBD9);        //
    static GamePtr<RE::BGSKeyword> armorGlass(0x6BBDC);        //
    static GamePtr<RE::BGSSoundDescriptorForm> bluntVsOther(0x3C826);
    static GamePtr<RE::BGSPerk> windRunnerPerk (0x105F22);
    static GamePtr<RE::SpellItem> doomSerpent (0xE5F61);
    // bestiary checks
    static GamePtr<RE::TESRace> FalmerRace(0x131F4);
    static GamePtr<RE::TESRace> SprigganRace(0x13204);
    static GamePtr<RE::TESRace> TrollRace(0x13205);
    static GamePtr<RE::TESRace> TrollFrostRace(0x13206);
    static GamePtr<RE::TESRace> ChaurusRace(0x131EB);
    static GamePtr<RE::TESRace> ChaurusReaperRace(0xA5601);
    static GamePtr<RE::TESRace> FrostbiteSpiderRace(0x131F8);
    static GamePtr<RE::TESRace> FrostbiteSpiderRaceGiant(0x4E507);  // spider fact contain many creatures, so use races
    static GamePtr<RE::TESRace> FrostbiteSpiderRaceLarge(0x53477);
    static GamePtr<RE::TESRace> DraugrRace(0xD53);
    static GamePtr<RE::TESRace> WolfRace(0x1320A);        // wolf fact contain nix-hound etc, so use race
    static GamePtr<RE::TESFaction> BearFaction(0xFBBF3);  // bear races are x3, so use fact
    static GamePtr<RE::TESFaction> SabreCatFaction(0x435A2);
    static GamePtr<RE::BGSKeyword> isBeastRace(0xD61D1);
    // static GamePtr<RE::TESRace> WerewolfBeastRace (0xCDD84);
    //----------------------------------------------------------------------//
    // bestiary checks
    static GamePtr<RE::TESRace> DLC1TrollRaceArmored      (0x117F5, "Dawnguard.esm");
    static GamePtr<RE::TESRace> DLC1TrollFrostRaceArmored (0x117F4, "Dawnguard.esm");
    static GamePtr<RE::TESRace> DLC1ChaurusHunterRace     (0x51FB,  "Dawnguard.esm");
    static GamePtr<RE::TESRace> DLC1SoulCairnMistman      (0x894D,  "Dawnguard.esm");
    static GamePtr<RE::TESFaction> LichFaction     (0xA471FC, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::TESFaction> ArchLichFaction (0xA471FB, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::TESFaction> BerserkFaction  (0xA5653A, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::TESFaction> CairnBGfaction  (0xA60765, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::TESFaction> ValleyGhostFaction (0xFC7A,  "Requiem for a Dream - Kelpie.esp");
    static GamePtr<RE::SpellItem> AbForswornSkinchanges (0x2E576, "Requiem for a Dream - Kelpie.esp");
    static GamePtr<RE::SpellItem> AbSkeletonChampion (0x33812, "Requiem - Beware The Dead.esp");
    static GamePtr<RE::BGSKeyword> DLC2Werebear  (0x34FB7, "Dragonborn.esm");

    // bestiary keywords
    static GamePtr<RE::BGSKeyword> BestiaryFalmer            (0xA420DF, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::BGSKeyword> BestiarySpriggan          (0xA420E0, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::BGSKeyword> BestiaryFrostTroll        (0xA420E1, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::BGSKeyword> BestiarySimpleTroll       (0xA420E2, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::BGSKeyword> BestiaryArmoredTroll      (0xA420E3, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::BGSKeyword> BestiaryArmoredFrostTroll (0xA420E4, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::BGSKeyword> BestiaryWolf              (0xA420E5, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::BGSKeyword> BestiaryBear              (0xA420E6, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::BGSKeyword> BestiarySabrecat          (0xA420E7, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::BGSKeyword> BestiaryChaurus           (0xA420E8, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::BGSKeyword> BestiaryChaurusReaper     (0xA420E9, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::BGSKeyword> BestiaryChaurusHunter     (0xA420EA, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::BGSKeyword> BestiaryFrostbiteSpider   (0xA420EB, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::BGSKeyword> BestiaryLich              (0xA4C312, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::BGSKeyword> BestiaryGrandLich         (0xA4C313, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::BGSKeyword> BestiaryDraugr            (0xA51427, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::BGSKeyword> BestiaryHulkingDraugr     (0xA51428, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::BGSKeyword> BestiaryDraugrBerserk     (0xA51429, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::BGSKeyword> BestiarySkeletonGang      (0xAE4327, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::BGSKeyword> BestiaryMistMan           (0xA60762, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::BGSKeyword> BestiaryBlackGuard        (0xA60763, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::BGSKeyword> BestiaryValleyGhost       (0xA60764, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::BGSKeyword> BestiaryWerewolf          (0xA7EDD4, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::BGSKeyword> BestiaryWerebear          (0xA7EDD5, "RfaD SSE - Awaken.esp");
    // mutagens
    static GamePtr<RE::TESGlobal> sameMutagensGlob (0xC1949E, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::SpellItem> m_red_small(0x9FC, "Alchemy.esp");
    static GamePtr<RE::SpellItem> m_red_mid(0x9FD, "Alchemy.esp");
    static GamePtr<RE::SpellItem> m_red_big(0x9FE, "Alchemy.esp");
    static GamePtr<RE::SpellItem> m_green_small(0x9FF, "Alchemy.esp");
    static GamePtr<RE::SpellItem> m_green_mid(0xA00, "Alchemy.esp");
    static GamePtr<RE::SpellItem> m_green_big(0xA01, "Alchemy.esp");
    static GamePtr<RE::SpellItem> m_blue_small(0xA02, "Alchemy.esp");
    static GamePtr<RE::SpellItem> m_blue_mid(0xA03, "Alchemy.esp");
    static GamePtr<RE::SpellItem> m_blue_big(0xA04, "Alchemy.esp");
    static GamePtr<RE::BGSKeyword> m_red_kw(0x1F9759, "Alchemy.esp");
    static GamePtr<RE::BGSKeyword> m_green_kw(0x1F975B, "Alchemy.esp");
    static GamePtr<RE::BGSKeyword> m_blue_kw(0x1F975C, "Alchemy.esp");
    static GamePtr<RE::BGSKeyword> m_small_kw(0x208C5C, "Alchemy.esp");
    static GamePtr<RE::BGSKeyword> m_med_kw(0x208C5D, "Alchemy.esp");
    static GamePtr<RE::BGSKeyword> m_big_kw(0x208C5E, "Alchemy.esp");
    // knuckles
    static GamePtr<RE::BGSKeyword> sigvaldSet (0x20009E, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::SpellItem> knuckles_univ (0xB852BD, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::SpellItem> fish_bleed (0x59A7, "Requiem for a Dream - Kelpie.esp");
    static GamePtr<RE::SpellItem> knuckles_jab_spell (0x8198FE, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::BGSKeyword> knuckles_bleeding (0x893326, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::BGSKeyword> knuckles_uniq (0x88E216, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::BGSKeyword> knuckles_blunt (0x893327, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::SpellItem>  knuckles_blockedMarkerSelf (0x8890FE, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::EffectSetting> knuckles_afterBlockTimerSelf (0x889100, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::EffectSetting> knuckles_jab (0x8147EE, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::SpellItem> knuckles_combo_ON(0x8147EC, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::BGSPerk> unarmed_20 (0xEF222, "IW_Unarmed_Perktree.esp");
    static GamePtr<RE::BGSPerk> unarmed_50 (0xEF223, "IW_Unarmed_Perktree.esp");
    static GamePtr<RE::BGSPerk> unarmed_80 (0xEF224, "IW_Unarmed_Perktree.esp");
    // mechanist
    static GamePtr<RE::TESGlobal>  currentSummonedSpace (0xE14015, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::BGSKeyword> automaton (0xD1196C, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::BGSKeyword> deploy1Space (0xE14016, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::BGSKeyword> deploy2Space (0xE14017, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::BGSKeyword> deploy3Space (0xE14018, "RfaD SSE - Awaken.esp");     
    static GamePtr<RE::BGSKeyword> deploy4Space (0xE14019, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::BGSKeyword> deploy5Space (0xE1401A, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::BGSPerk> mech_25  (0x636C0, "RFAD_IW_Mechanist.esp");
    static GamePtr<RE::BGSPerk> mech_50  (0x636C1, "RFAD_IW_Mechanist.esp");
    static GamePtr<RE::BGSPerk> mech_75  (0x636C2, "RFAD_IW_Mechanist.esp");
    static GamePtr<RE::BGSPerk> mech_100 (0x636C3, "RFAD_IW_Mechanist.esp");
    // oil
    static GamePtr<RE::SpellItem> silverDust1 (0xFA4B56, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::SpellItem> silverDust2 (0x5DC480, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::SpellItem> oilPoison_onHit1  (0x12AE82, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::SpellItem> oilPoison_onHit2  (0x716CE6, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::SpellItem> oilPoison_overdose(0x6FD6D8, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::SpellItem> oilGarlic_onHit1  (0x171D9A, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::SpellItem> oilGarlic_onHit2  (0xB58BA5, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::SpellItem> oilFrost_onHit1   (0x12FF98, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::SpellItem> oilFrost_onHit2   (0x990A41, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::SpellItem> oilFrost_onHit3   (0xB58BA7, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::EffectSetting> oilPoison_stackCounter (0x6FD6D6, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::EffectSetting> oilFrostDmgDecrease (0x990A43, "RfaD SSE - Awaken.esp");

    static GamePtr<RE::TESGlobal> bossFightID (0x1C7FFB, "RfaD SSE - Awaken.esp");

    // ac + grimoire
    static GamePtr<RE::SpellItem> arcaneCurseExpl (0xDAA, "Requiem for a Dream - ArcaneCurse.esp");
    static GamePtr<RE::SpellItem> arcaneCurseRegenDebuff (0x808, "Requiem for a Dream - ArcaneCurse.esp");
    static GamePtr<RE::SpellItem> arcaneCurseSummonDebuffHP  (0xD810E9, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::SpellItem> arcaneCurseSummonDebuffAll (0xD810EB, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::TESObjectARMO> chemistPotions (0xD8B314, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::TESObjectARMO> grimoireMagDmgOnHit  (0xD9C, "Requiem for a Dream - ArcaneCurse.esp");
    static GamePtr<RE::TESObjectARMO> grimoireDecurse      (0xD9A, "Requiem for a Dream - ArcaneCurse.esp");
    static GamePtr<RE::TESObjectARMO> grimoireMagFury      (0xD8E, "Requiem for a Dream - ArcaneCurse.esp");
    static GamePtr<RE::TESObjectARMO> grimoireConvertInc   (0xD8F, "Requiem for a Dream - ArcaneCurse.esp");
    static GamePtr<RE::TESObjectARMO> grimoirePenetr     (0x9E767, "Requiem for a Dream - ArcaneCurse.esp");
    static GamePtr<RE::BGSKeyword> lessCurseExplDamage   (0xADBD9, "Requiem for a Dream - ArcaneCurse.esp");
    static GamePtr<RE::BGSExplosion> curseExpl (0x995EC, "Requiem for a Dream - ArcaneCurse.esp");
    static GamePtr<RE::EffectSetting> criticalArtEff   (0xDBA, "Requiem for a Dream - ArcaneCurse.esp");
    static GamePtr<RE::SpellItem>     criticalCurseArt (0xDBD, "Requiem for a Dream - ArcaneCurse.esp");
    static GamePtr<RE::BGSArtObject> curseArto (0x4BE34);

    
    static RE::SpellItem* oil_diseaseOnHit;
    static RE::SpellItem* oil_igniteOnHit1;
    static RE::SpellItem* oil_igniteOnHit2;

    static RE::SpellItem* oil_corroseOnHit;
    static RE::EffectSetting *oil_silver, *oil_garlic, *oil_poison, *oil_frost, *oil_ignite, *oil_disease, *oil_corrose;
    static RE::EffectSetting *oilGarlicMarker, *oil_KWHolder;

    static RE::BGSKeyword *oil_improvedKW, *oil_pureKW;
    static RE::BGSKeyword* actorTypeUndead;
    static RE::BGSKeyword* weapMaterialSilver;

    static RE::SpellItem* vampirism;
    static RE::SpellItem* olveShield;

    static RE::BGSPerk *alch_poison_25_perk, *alch_poison_50_perk, *alch_poison_75_perk, *alch_100_perk;

    static RE::BGSSoundDescriptorForm* olvePreShieldVoice;
    
    void set_univ_eff (int id, float mag, uint32_t dur);
    void knuckles_block (RE::Actor* agr, RE::Actor* target, float& damage, float blockMod);
    void knuckles_hit (RE::Actor* agr, RE::Actor* target, RE::TESObjectARMO* hands, float& damage, bool isPwAtk, bool isLeft);
    void oil_proc (RE::Actor* pl, RE::Actor* target, RE::HitData* hit_data, float& damage);
    void windRunnerCheck(RE::Actor *pl);
    void vamp_drain_onHit(RE::Actor* agr, RE::Actor* target);
    void initGameplayData();
    void cast_oil (RE::SpellItem* spell, RE::Actor* target, RE::Actor* pl, RE::AlchemyItem* oil, float ef, float magnOverride = 0);

    void regive_mutagen (RE::SpellItem* spell, RE::Actor* pl, int count);
    void add_mutagen (RE::SpellItem* spell, RE::Actor* pl, int count);
    void remove_other   (std::vector<RE::SpellItem*>& vec, RE::Actor* pl);
    void handle_mutagen (RE::Actor* pl, RE::AlchemyItem* potion);
    void check_bestiary (RE::Actor* actor);

    void increase_arcane_curse (RE::Actor* caster, float amount);
    void handle_player_arcane_curse (RE::Actor* pl);
    void handle_cast_arcane_curse (RE::Actor* caster, RE::SpellItem *spell, bool dualCast);
     
    void handle_bossAppearance (RE::Actor* boss);
    void bossFightHandle (int fight, uint16_t &counter);

    void handle_mechanist (RE::Actor *pl, RE::MagicItem* deployScroll = nullptr);
    bool handle_cast_mechanist (RE::Actor *pl, RE::MagicItem* mitem);

    static RE::SpellItem *olveCast, *olvePreShield, *sephCast, *harkonCast; 
    static RE::Actor *currBoss;
    static int olveState;
};


namespace qst     // newstart quests
{
    static RE::TESDataHandler* handler;

    static GamePtr<RE::BGSPerk> cult_namira_2   (0x2F1B4C, "ChihSkillTree.esp");
    static GamePtr<RE::TESGlobal> altarsTouched (0xC7BF0F, "devFixes.esp");

    static GamePtr<RE::TESObjectARMO> ringOfBeast   (0xE7FD, "Dawnguard.esm");
    static GamePtr<RE::TESObjectARMO> ringOfErudite (0xE7FE, "Dawnguard.esm");
    static GamePtr<RE::TESObjectARMO> amuletOfGarg  (0xF4D5, "Dawnguard.esm");
    static GamePtr<RE::TESObjectARMO> amuletOfBats  (0x68AE, "Dawnguard.esm");

    static RE::TESQuest* startVamp;
    static RE::TESQuest* startCult;
    static RE::TESQuest* startMage;
    static RE::TESQuest* startNecr;
    static RE::TESQuest* startTravel;
    static RE::TESQuest* startMerc;
    static RE::TESQuest* startPal;
    static RE::TESQuest* startVigil;
    static RE::TESQuest* startDB;
    static RE::TESQuest* startSnowElf;

    static RE::TESQuest* MS14;
    static RE::TESQuest *DLC1RV09, *DLC1RV08, *DLC1VQ08, *DLC1VQ07;
    static RE::TESQuest *DLC2TT2, *DLC2RR02, *DLC2MQ05;
    static RE::TESQuest *DBc1,*DBc2,*DBc3;
    static RE::TESQuest *DA01,*DA02,*DA03,*DA04,*DA05,*DA06,*DA07,*DA08,*DA09,*DA10,*DA11,*DA13,*DA14,*DA15,*DA16;
    static RE::TESQuest *BBRegentQuest, *BBWindsQuest, *BBFilamentQuest, *BBSumrakQuest, *BBLegendsQuest;
    static RE::TESQuest *deathBrandQuest, *azidalQuest, *zakrisosQuest, *valokQuest;
    static RE::TESQuest* MG08;
    static RE::TESQuest* TG09;
    static RE::TESQuest* StalleoQuest;
    static RE::TESQuest* DBDestroy;
    static RE::TESQuest* sephirothQuest;

    static RE::SpellItem *mercTraining, *bldHorkerTraining, *boethiaCultistTraining;
    static RE::BGSPerk *aedraToken, *daedraToken;
    static RE::TESObjectARMO* pal_ring;

    static RE::TESGlobal *currGoalVal;

    static RE::TESGlobal *isNamiraBossDead, *isMeridiaBossDead, *isVerminaBossDead;
    static RE::TESGlobal *isVigharDead, *isMarkDead, *isWilhelmDead;
    static RE::TESGlobal *isReaperDead, *isSiltineDead, *isAnachoretDead;
    static RE::TESGlobal *stonesBuffStopDay;
    static RE::TESGlobal *castleMagesKilled;
    static RE::TESGlobal *ambassadorsKilled;
    static RE::TESGlobal *contractsDone;

    static RE::TESFaction* dawnguardHunters;
    static RE::TESFaction* mythicDawnCultist;
    static RE::TESFaction* chihNamiraCult1;
    static RE::TESFaction* chihNamiraCult2;

    static RE::BGSPerk* cult_azura_2;
    static RE::BGSPerk* cult_vermina_2;
    static RE::BGSPerk* cult_meridia_2;
    static RE::BGSPerk* cult_nocturn_2;
    static RE::BGSPerk* cult_boethia_2;
    static RE::BGSPerk* cult_mephala_2;

    static RE::BGSKeyword* conjPhylact;

    void initQuestVars();
    void check_class_achievements(RE::Actor* player);
    void check_onDeath_questGoals(RE::Actor *victim);
};

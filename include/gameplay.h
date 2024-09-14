
#pragma once

#include "GamePtr.h"

namespace gameplay
{

    static GamePtr<RE::BGSKeyword> vampire (0x914E5);               // Skyrim.esm

    static GamePtr<RE::SpellItem> silverDust1 (0xFA4B56, "RfaD SSE - Awaken.esp");   // onHit spells
    static GamePtr<RE::SpellItem> silverDust2 (0x5DC480, "RfaD SSE - Awaken.esp");

    static RE::SpellItem* oil_diseaseOnHit;
    static RE::SpellItem* oil_igniteOnHit1;
    static RE::SpellItem* oil_igniteOnHit2;
    static RE::SpellItem* oil_frostOnHit;
    static RE::SpellItem* oil_poisonOnHit;
    static RE::SpellItem* oil_garlicOnHit;
    static RE::SpellItem* oil_corroseOnHit;
    static RE::EffectSetting *oil_silver, *oil_garlic, *oil_poison, *oil_frost, *oil_ignite, *oil_disease, *oil_corrose;
    static RE::EffectSetting *oilGarlicMarker, *oil_KWHolder;

    static RE::BGSKeyword *oil_improvedKW, *oil_pureKW;
    static RE::BGSKeyword* actorTypeUndead;
    static RE::BGSKeyword* weapMaterialSilver;

    static RE::SpellItem* vampirism;
    static RE::SpellItem* olveShield;

    static RE::BGSPerk *windRunnerPerk;
    static RE::BGSPerk *alch_poison_25_perk, *alch_poison_50_perk, *alch_poison_75_perk, *alch_100_perk;

    static RE::BGSSoundDescriptorForm* olvePreShieldVoice;

    float get_oil_scale_mult (RE::Actor* pl, RE::AlchemyItem* oil);
    bool  oil_proc (RE::Actor* pl, RE::Actor* target, RE::HitData* hit_data, bool isPwatk);
    void  windRunnerCheck(RE::Actor *pl);
    void  initGameplayData();

    void bossFightHandle (RE::Actor* boss, int fightID, uint16_t &counter);

    void gmpl_on_micro_update();

    static RE::SpellItem *olveCast, *olvePreShield, *sephCast, *harkonCast; 
    static RE::Actor *currBoss;
    static int olveState;

};


namespace qst     // newstart quests
{
    static RE::TESDataHandler* handler;

    static GamePtr<RE::BGSPerk> cult_namira_2 (0x2F1B4C, "ChihSkillTree.esp");

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
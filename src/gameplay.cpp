
// SYNCED ++++

#pragma once

#include "utils.h"
#include "core.h"




namespace qst
{
    void initQuestVars()
    {
        handler = RE::TESDataHandler::GetSingleton();

        currGoalVal = handler->LookupForm<RE::TESGlobal>(0xFE1824, "RfaD SSE - Awaken.esp");

        startTravel    = handler->LookupForm<RE::TESQuest>(0x9D016,  "RfaD SSE - Awaken.esp");
        startPal       = handler->LookupForm<RE::TESQuest>(0x37A80,  "RfaD SSE - Awaken.esp");
        startCult      = handler->LookupForm<RE::TESQuest>(0x37A82,  "RfaD SSE - Awaken.esp");
        startVamp      = handler->LookupForm<RE::TESQuest>(0xFC31B4, "RfaD SSE - Awaken.esp");
        startVigil     = handler->LookupForm<RE::TESQuest>(0xFEBA43, "RfaD SSE - Awaken.esp");
        startMage      = handler->LookupForm<RE::TESQuest>(0xFFFE8A, "RfaD SSE - Awaken.esp");
        startNecr      = handler->LookupForm<RE::TESQuest>(0xF1D0,   "RfaD SSE - Awaken.esp");
        startMerc      = handler->LookupForm<RE::TESQuest>(0xF1CF,   "RfaD SSE - Awaken.esp");
        startDB        = handler->LookupForm<RE::TESQuest>(0xF1CE,   "RfaD SSE - Awaken.esp");
        startSnowElf   = handler->LookupForm<RE::TESQuest>(0x46DB4,  "RfaD SSE - Awaken.esp");
        
        MS14      = RE::TESForm::LookupByID<RE::TESQuest>(0x25F3E);                            
        DBc1      = RE::TESForm::LookupByID<RE::TESQuest>(0x1EA5B);    
        DBc2      = RE::TESForm::LookupByID<RE::TESQuest>(0x1EA5E);                // } skyrim.esm
        DBc3      = RE::TESForm::LookupByID<RE::TESQuest>(0x1EA5F);    
        DBDestroy = RE::TESForm::LookupByID<RE::TESQuest>(0x934FB);    
        MG08      = RE::TESForm::LookupByID<RE::TESQuest>(0x1F258);
        TG09      = RE::TESForm::LookupByID<RE::TESQuest>(0x21555);
        StalleoQuest = RE::TESForm::LookupByID<RE::TESQuest>(0x4B2A1);    
        DLC1VQ07 = handler->LookupForm<RE::TESQuest>(0x2853,  "Dawnguard.esm");
        DLC1VQ08 = handler->LookupForm<RE::TESQuest>(0x7C25,  "Dawnguard.esm");
        DLC1RV08 = handler->LookupForm<RE::TESQuest>(0xCE06,  "Dawnguard.esm");
        DLC1RV09 = handler->LookupForm<RE::TESQuest>(0xCE0B,  "Dawnguard.esm");
        DLC2RR02 = handler->LookupForm<RE::TESQuest>(0x18B14, "Dragonborn.esm");
        DLC2TT2  = handler->LookupForm<RE::TESQuest>(0x195A1, "Dragonborn.esm");
        DLC2MQ05 = handler->LookupForm<RE::TESQuest>(0x179DE, "Dragonborn.esm");
        BBLegendsQuest  = handler->LookupForm<RE::TESQuest>(0x3216C, "Dragonborn.esm");
        BBRegentQuest   = handler->LookupForm<RE::TESQuest>(0x3216D, "Dragonborn.esm");
        BBSumrakQuest   = handler->LookupForm<RE::TESQuest>(0x32170, "Dragonborn.esm");
        BBWindsQuest    = handler->LookupForm<RE::TESQuest>(0x3216E, "Dragonborn.esm");
        BBFilamentQuest = handler->LookupForm<RE::TESQuest>(0x3216F, "Dragonborn.esm");
        deathBrandQuest = handler->LookupForm<RE::TESQuest>(0x24002, "Dragonborn.esm");
        azidalQuest     = handler->LookupForm<RE::TESQuest>(0x1810B, "Dragonborn.esm");
        valokQuest        = handler->LookupForm<RE::TESQuest>(0x19B4A, "Dragonborn.esm");
        zakrisosQuest   = handler->LookupForm<RE::TESQuest>(0x18B15, "Dragonborn.esm");
        sephirothQuest  = handler->LookupForm<RE::TESQuest>(0x660ACB, "RfaD SSE - Awaken.esp");
        
        isVigharDead = handler->LookupForm<RE::TESGlobal>(0x764487, "devFixes.esp"); 
        isMarkDead = handler->LookupForm<RE::TESGlobal>(0x92DE7, "RfaD SSE - Awaken.esp");
        isWilhelmDead = handler->LookupForm<RE::TESGlobal>(0x97EFA, "RfaD SSE - Awaken.esp");
        isMeridiaBossDead = handler->LookupForm<RE::TESGlobal>(0xAC348, "RfaD SSE - Awaken.esp");
        isVerminaBossDead = handler->LookupForm<RE::TESGlobal>(0xAC34A, "RfaD SSE - Awaken.esp");
        isReaperDead = handler->LookupForm<RE::TESGlobal>(0xC07B1, "RfaD SSE - Awaken.esp");
        isSiltineDead = handler->LookupForm<RE::TESGlobal>(0xA2129, "RfaD SSE - Awaken.esp");
        isAnachoretDead = handler->LookupForm<RE::TESGlobal>(0xA2128, "RfaD SSE - Awaken.esp");
        isNamiraBossDead = handler->LookupForm<RE::TESGlobal>(0x2B99A6, "ChihSkillTree.esp");

        stonesBuffStopDay = handler->LookupForm<RE::TESGlobal>(0x6D0994, "devFixes.esp");
        aedraToken = handler->LookupForm<RE::BGSPerk>(0xE460B, "Requiem for a Dream - DivineBlessings.esp");
        daedraToken = handler->LookupForm<RE::BGSPerk>(0xE460C, "Requiem for a Dream - DivineBlessings.esp");

        castleMageFaction = handler->LookupForm<RE::TESFaction>(0x90AF43, "devFixes.esp");
        chihNamiraCult1 = handler->LookupForm<RE::TESFaction>(0x2F6CEE, "ChihSkillTree.esp");
        chihNamiraCult2 = handler->LookupForm<RE::TESFaction>(0x2F6CEF, "ChihSkillTree.esp");
        mythicDawnCultist = handler->LookupForm<RE::TESFaction>(0x14C51, "RfaD SSE - Awaken.esp");
        dawnguardHunters = handler->LookupForm<RE::TESFaction>(0x3375, "Dawnguard.esm");

        cult_azura_2 = handler->LookupForm<RE::BGSPerk>(0x54E9C, "ChihSkillTree.esp");
        cult_boethia_2 = handler->LookupForm<RE::BGSPerk>(0x54E9D, "ChihSkillTree.esp");
        cult_meridia_2 = handler->LookupForm<RE::BGSPerk>(0x54E9E, "ChihSkillTree.esp");
        cult_mephala_2 = handler->LookupForm<RE::BGSPerk>(0x54E9A, "ChihSkillTree.esp");
        cult_nocturn_2 = handler->LookupForm<RE::BGSPerk>(0x54E9B, "ChihSkillTree.esp");
        cult_vermina_2 = handler->LookupForm<RE::BGSPerk>(0xCED24, "ChihSkillTree.esp");

        boethiaCultistTraining = handler->LookupForm<RE::SpellItem>(0x2E57C, "Requiem for a Dream - Kelpie.esp");
        mercTraining = handler->LookupForm<RE::SpellItem>(0x47B7B, "Requiem for a Dream - Kelpie.esp");
        bldHorkerTraining = handler->LookupForm<RE::SpellItem>(0x14E95, "Requiem for a Dream - Kelpie.esp");
        
        conjPhylact = handler->LookupForm<RE::BGSKeyword>(0x17BCD1, "devFixes.esp");

        RE::TESQuest** da_quests[15] = {&DA01,&DA02,&DA03,&DA04,&DA05,&DA06,&DA07,&DA08,&DA09,&DA10,&DA11,&DA13,&DA14,&DA15,&DA16};
        RE::FormID da_ids[15] = {0x28AD6,0x4D8D6,0x1BFC4,0x2D512,0x2A49A,0x3B681,0x240B8,0x4A37B,0x4E4E1,0x22F08,0x2C358,0x8998D,0x1BB9B,0x2AC68,0x242AF};
        for (int i = 0; i < 15; i++) {
             *da_quests[i] = RE::TESForm::LookupByID<RE::TESQuest>(da_ids[i]);    // daedra quests from skyrim.esm
        }
    }
        
    void check_onDeath_questGoals (RE::Actor *victim)
    {
        if (qst::startDB->currentStage == 40) {
                 if (victim->IsInFaction(RE::TESForm::LookupByID<RE::TESFaction>(0x86EEE)))  qst::currGoalVal->value += 1;  // guard faction
        }
        else if (qst::startDB->currentStage == 50) {
                 if (victim->IsInFaction(qst::castleMageFaction)) qst::currGoalVal->value += 1;   // castle mages
        }
        else if (qst::startDB->currentStage == 60) {
                 if (victim->formID == 0x133AF || victim->formID == 0x1E7D7 ||    // ambassadors (elenwen, ancarion, ancano, ondolemar)
                     victim->formID == 0x13269 || victim->formID == 0x0401CAF3)  qst::currGoalVal->value += 1;
        }
        else if (qst::startMerc->currentStage == 40) {
                 if (victim->HasSpell(qst::mercTraining) || victim->HasSpell(qst::bldHorkerTraining)) qst::currGoalVal->value += 1;  // mercenaries
        }
        else if (qst::startPal->currentStage == 60) {
                 if (victim->IsInFaction(RE::TESForm::LookupByID<RE::TESFaction>(0x34B74))) qst::currGoalVal->value += 1;  // necromant faction
        }
        else if (qst::startNecr->currentStage == 50) {
                 if (victim->IsInFaction(RE::TESForm::LookupByID<RE::TESFaction>(0x34B74))) qst::currGoalVal->value += 1;  // necromant faction
        }
        else if (qst::startVigil->currentStage == 60) {
                 if (victim->IsInFaction(RE::TESForm::LookupByID<RE::TESFaction>(0x27242))) qst::currGoalVal->value += 1;  // vampire faction
        }
        else if (qst::startVamp->currentStage == 50) {
                 if (victim->IsInFaction(qst::dawnguardHunters)) qst::currGoalVal->value += 1;  // dawnguard faction
        }
        else if (qst::startVamp->currentStage == 40) {
                 if (victim->IsInFaction(RE::TESForm::LookupByID<RE::TESFaction>(0x27242))) qst::currGoalVal->value += 1;  // vampire faction
        }
        else if (qst::startCult->currentStage == 30)
        {
            if (victim->HasSpell(qst::boethiaCultistTraining) || victim->IsInFaction(qst::mythicDawnCultist) ||  // boethia cultist, mythic dawn
                   victim->IsInFaction(RE::TESForm::LookupByID<RE::TESFaction>(0x8942F)) ||  // namira canniball
                   victim->IsInFaction(RE::TESForm::LookupByID<RE::TESFaction>(0xE953D)) ||  // namira canniball attack
                   victim->IsInFaction(RE::TESForm::LookupByID<RE::TESFaction>(0x7A509)) ||  // vaermina nightcaller temple
                   victim->IsInFaction(qst::chihNamiraCult1) || victim->IsInFaction(qst::chihNamiraCult2))  // chih namira canal factions
                   { qst::currGoalVal->value += 1; }
            }
     }
};


int qMisc (std::string statName)    
{
    int val = u_get_game_statistic(statName);
    qst::currGoalVal->value = val;
    return val;
}

void setStage (RE::TESQuest* quest, uint16_t stage) { LOG("called setstage({})", stage); u_setStage(quest, stage); }

float  getContractsDone () { 
    qst::currGoalVal->value = mys::player->GetActorValue(RE::ActorValue::kFavorsPerDay);
    return qst::currGoalVal->value;
}

bool vigharMovarthDead() { return (qst::isVigharDead->value > 0 && qst::MS14->currentStage > 100); }

bool contractsDone() { return (qst::DBc1->currentStage>20 && qst::DBc2->currentStage>20 && qst::DBc3->currentStage>20);}

bool vampArtifacts() { return (qst::DLC1RV08->currentStage >= 200 && qst::DLC1RV09->currentStage >= 200); }

bool learnedFountain() {return mys::player->HasSpell(qst::handler->LookupForm<RE::SpellItem>(0x4FA6, "RfaD SSE - Awaken.esp"));}

bool allArchLichesDead() { return (qst::isAnachoretDead->value && qst::isSiltineDead->value && qst::isReaperDead->value); }

bool conjRitualQuestDone() { return (RE::TESForm::LookupByID<RE::TESQuest>(0x99F27)->currentStage > 100); }

bool equippedConjurPhylactery() { return u_worn_has_keyword(mys::player, qst::conjPhylact); }


bool hasMagicAdeptPerk()
{
    return(mys::player->HasPerk(RE::TESForm::LookupByID<RE::BGSPerk>(0xC44B8)) ||
           mys::player->HasPerk(RE::TESForm::LookupByID<RE::BGSPerk>(0xC44BC)) ||
           mys::player->HasPerk(RE::TESForm::LookupByID<RE::BGSPerk>(0xC44C0)) ||
           mys::player->HasPerk(RE::TESForm::LookupByID<RE::BGSPerk>(0xC44C4)) ||
           mys::player->HasPerk(RE::TESForm::LookupByID<RE::BGSPerk>(0xC44C8))   );
}

int citizenHelped ()
{
    qst::currGoalVal->value = (         // skyrim globs like FriendsCountWhiterun etc.
    RE::TESForm::LookupByID<RE::TESGlobal>(0x87E23)->value + RE::TESForm::LookupByID<RE::TESGlobal>(0x87E22)->value +
    RE::TESForm::LookupByID<RE::TESGlobal>(0x87E21)->value + RE::TESForm::LookupByID<RE::TESGlobal>(0x87E20)->value +
    RE::TESForm::LookupByID<RE::TESGlobal>(0x87E1F)->value + RE::TESForm::LookupByID<RE::TESGlobal>(0x87E1E)->value +
    RE::TESForm::LookupByID<RE::TESGlobal>(0x87E1D)->value + RE::TESForm::LookupByID<RE::TESGlobal>(0x87E1C)->value +
    RE::TESForm::LookupByID<RE::TESGlobal>(0x87E1B)->value );
    return qst::currGoalVal->value;
}

int priestsHelped ()
{
    int x = 0;
    if (RE::TESForm::LookupByID<RE::TESQuest>(0x23B6C)->currentStage > 80) x++;  // T01
    if (RE::TESForm::LookupByID<RE::TESQuest>(0x211D5)->currentStage > 95) x++;     // T02
    if (RE::TESForm::LookupByID<RE::TESQuest>(0x1C48E)->currentStage > 30) x++;  // T03
    if (RE::TESForm::LookupByID<RE::TESQuest>(0x53309)->currentStage > 25) x++;  // FreeformRiften05
    if (RE::TESForm::LookupByID<RE::TESQuest>(0x43E1A)->currentStage > 10) x++;  // FreeformRiften13
    if (RE::TESForm::LookupByID<RE::TESQuest>(0x242AF)->currentStage > 197)x++;  // DA16
    if (RE::TESForm::LookupByID<RE::TESQuest>(0x7D949)->currentStage > 10) x++;  // DA11Intro
    if (u_get_item_count(mys::player, 0x705C3) > 0) x++;    // Runil Journal

    qst::currGoalVal->value = x;
    return x;
}

int tombQuestsDone ()
{
    int x = 0;
    if (qst::deathBrandQuest->currentStage > 90)  x++;                             // DLC2dunHaknirTreasureQST (Haknir)
    if (qst::azidalQuest->currentStage     > 475) x++;                           // DLC2dunKolbjornQST (Azidal)
    if (qst::zakrisosQuest->currentStage   > 50)  x++;                           // DLC2RR03 (Zakrisos)
    if (qst::valokQuest->currentStage       > 50)  x++;                           // DLC2SV01 (Valok)
    if (RE::TESForm::LookupByID<RE::TESQuest>(0x7EDE5)->currentStage > 70) x++;  // dunAngarvundeQST
    if (RE::TESForm::LookupByID<RE::TESQuest>(0x35354)->currentStage > 70) x++;     // dunForelhostQST
    if (RE::TESForm::LookupByID<RE::TESQuest>(0xD9B63)->currentStage > 30) x++;  // FreeformIvarstead01
    if (RE::TESForm::LookupByID<RE::TESQuest>(0x1F252)->currentStage > 70) x++;  // MG02
    if (RE::TESForm::LookupByID<RE::TESQuest>(0x6CD54)->currentStage > 80) x++;  // dunIronbindQST (Gatrik)
    if (RE::TESForm::LookupByID<RE::TESQuest>(0x9F179)->currentStage > 40) x++;  // FreeformRiften21 (Yngvild)
    if (RE::TESForm::LookupByID<RE::TESQuest>(0x3E897)->currentStage > 90) x++;  // dunHillgrundsTombQST (Vals)
    if (RE::TESForm::LookupByID<RE::TESQuest>(0x39645)->currentStage > 40) x++;  // MS13 (Bleakfalls Barrow)
    if (RE::TESForm::LookupByID<RE::TESQuest>(0x3ED82)->currentStage > 50) x++;  // dunValthumeQST (Hevnorak)
    if (RE::TESForm::LookupByID<RE::TESQuest>(0x3763C)->currentStage > 10) x++;  // dunRagnvaldQST (Otar)
    if (RE::TESForm::LookupByID<RE::TESQuest>(0x443F4)->currentStage > 90) x++;  // dunHighGateRuinsQST (Vokun)
    if (RE::TESForm::LookupByID<RE::TESQuest>(0xC1ACC)->currentStage > 22) x++;  // dunRebelsCairnQST (RedEagle)
    if (RE::TESForm::LookupByID<RE::TESQuest>(0x8ADFA)->currentStage > 30) x++;  // dunVolunruudQST (Kvenel)
    if (RE::TESForm::LookupByID<RE::TESQuest>(0x6136B)->currentStage > 20) x++;  // FreeformSoljundsSinkholeA (miners)

    qst::currGoalVal->value = x;
    return x;
}

int solsteimMasksFound()
{
    int x = 0;
    if (u_get_item_count(mys::player, 0x04024037) > 0) x++;  // zahkriisos
    if (u_get_item_count(mys::player, 0x040240FE) > 0) x++;  // ahzidal
    if (u_get_item_count(mys::player, 0x040240FF) > 0) x++;  // dukaan
    qst::currGoalVal->value = x;
    return x;
}

int priestMasksFound()
{
    int x = 0;
    if (u_get_item_count(mys::player, 0x61CC0) > 0) x++;  // 1 rahgot
    if (u_get_item_count(mys::player, 0x61CC1) > 0) x++;  // 2 hevnoraak
    if (u_get_item_count(mys::player, 0x61CC2) > 0) x++;  // 3 otar
    if (u_get_item_count(mys::player, 0x61CC9) > 0) x++;  // 4 vokun
    if (u_get_item_count(mys::player, 0x61CB9) > 0) x++;  // 5 krosis
    if (u_get_item_count(mys::player, 0x61CAB) > 0) x++;  // 6 volsung
    if (u_get_item_count(mys::player, 0x61CA5) > 0) x++;  // 7 nahkrin
    if (u_get_item_count(mys::player, 0x61C8B) > 0) x++;  // 8 morokei
    if (u_get_item_count(mys::player, 0x04024037) > 0) x++;  // 9  [zahkriisos]
    if (u_get_item_count(mys::player, 0x040240FE) > 0) x++;  // 10 [ahzidal]
    if (u_get_item_count(mys::player, 0x040240FF) > 0) x++;  // 11 [dukaan]
    qst::currGoalVal->value = x;
    return x;
}

bool haveMiraakSephirothMasks()
{
    auto sephirothMask = qst::handler->LookupForm<RE::TESObjectARMO>(0xDE012B, "RfaD SSE - Awaken.esp");
    return (u_get_item_count(mys::player, sephirothMask->formID) && u_get_item_count(mys::player, 0x04039D2F));
}

int blackBooksDone()
{
    int x = 0;
    if (qst::BBRegentQuest->currentStage   > 10) x++;
    if (qst::BBLegendsQuest->currentStage  > 10) x++; 
    if (qst::BBSumrakQuest->currentStage   > 10) x++; 
    if (qst::BBWindsQuest->currentStage    > 10) x++; 
    if (qst::BBFilamentQuest->currentStage > 10) x++;
    if (qst::DLC2MQ05->currentStage > 100) x++;
    qst::currGoalVal->value = x;
    return x;
}

int ritualQuests()
{
    int x = 0;
    if (RE::TESForm::LookupByID<RE::TESQuest>(0xA4E82)->currentStage > 100) x++;  // destr
    if (RE::TESForm::LookupByID<RE::TESQuest>(0xB3233)->currentStage > 100) x++;  // illus
    if (RE::TESForm::LookupByID<RE::TESQuest>(0x99F27)->currentStage > 100) x++;  // conjur
    if (RE::TESForm::LookupByID<RE::TESQuest>(0xCD987)->currentStage > 100) x++;  // restore
    if (RE::TESForm::LookupByID<RE::TESQuest>(0xD0755)->currentStage > 100) x++;  // alter
    qst::currGoalVal->value = x;
    return x;
}

int staffsFound()
{
    int x = 0;
    if (u_get_item_count(mys::player, 0x35369) > 0) x++;   // magnus
    if (u_get_item_count(mys::player, 0x1CB36) > 0) x++;   // rose
    if (u_get_item_count(mys::player, 0x35066) > 0) x++;   // skull
    if (u_get_item_count(mys::player, 0x2AC6F) > 0) x++;   // wabbajack
    qst::currGoalVal->value = x;
    return x;
}

int essence_found ()
{
    int x = 0;
    if (u_get_item_count(mys::player, 0xFB8F94) > 0) x++;
    if (u_get_item_count(mys::player, 0x1D7CB9) > 0) x++;
    if (u_get_item_count(mys::player, 0x1D7CBA) > 0) x++;
    qst::currGoalVal->value = x;
    return x;
}

int vampire_dust_have() {
    
    qst::currGoalVal->value = u_get_item_count(mys::player, 0x3AD76);  
    return qst::currGoalVal->value;
}

int daedra_guards_killed ()
{
    int x = 0;
    if (qst::isMeridiaBossDead->value > 0) x++;
    if (qst::isNamiraBossDead->value  > 0) x++;
    if (qst::isVerminaBossDead->value > 0) x++;
    if (qst::sephirothQuest->currentStage > 50) x++;
    if (qst::DA02->currentStage > 30) x++;  // boethia guard
    qst::currGoalVal->value = x;
    return x;
}

int arch_lich_killed ()
{
    int x = 0;
    if (qst::isAnachoretDead->value > 0) x++;
    if (qst::isSiltineDead->value > 0) x++;
    if (qst::isReaperDead->value > 0) x++;
    qst::currGoalVal->value = x;
    return x;
}

int dwarven_ghostBosses_killed ()
{
    float x = qst::handler->LookupForm<RE::TESGlobal>(0x158835, "RfaD SSE - Awaken.esp")->value;
    qst::currGoalVal->value = x;
    return x;
}

int daedra_artifacts_have ()
{
    int x = 0;        
    if (u_get_item_count(mys::player, 0x240D2) > 0) x++;  // razor
    if (u_get_item_count(mys::player, 0x233E3) > 0) x++;  // mace molag bal
    if (u_get_item_count(mys::player, 0x2AC6F) > 0) x++;  // wabbajack
    if (u_get_item_count(mys::player, 0x2ACD2) > 0) x++;  // volendrung
    if (u_get_item_count(mys::player, 0x63B27) > 0) x++;  // azura star white
    if (u_get_item_count(mys::player, 0x63B29) > 0) x++;  // azura star black
    if (u_get_item_count(mys::player, 0x2C37B) > 0) x++;  // ring namira
    if (u_get_item_count(mys::player, 0x2AC60) > 0) x++;  // ring chirsin
    if (u_get_item_count(mys::player, 0xD2846) > 0) x++;  // vile mask
    if (u_get_item_count(mys::player, 0x1A332) > 0) x++;  // ogma
    if (u_get_item_count(mys::player, 0x1CB36) > 0) x++;  // sanguine rose
    if (u_get_item_count(mys::player, 0x45F96) > 0) x++;  // spellbreaker
    if (u_get_item_count(mys::player, 0x4E4EE) > 0) x++;  // dawnbreaker
    if (u_get_item_count(mys::player, 0x1C4E6) > 0) x++;  // rueful axe
    if (u_get_item_count(mys::player, 0x35066) > 0) x++;  // skull staff
    if (u_get_item_count(mys::player, 0x2AC61) > 0) x++;  // savior hide
    if (u_get_item_count(mys::player, 0x52794) > 0) x++;  // ebony mail
    if (u_get_item_count(mys::player, 0x4A38F) > 0) x++;  // ebony blade
    qst::currGoalVal->value = x;
    return x;
}

bool chosenDaedra_fullPerks_n_quest()
{
    bool ok_ = false;
    if (mys::player->HasPerk(qst::cult_boethia_2) && qst::DA02->currentStage > 30)   ok_ = true;
    if (mys::player->HasPerk(qst::cult_mephala_2) && qst::DA08->currentStage > 40)   ok_ = true;
    if (mys::player->HasPerk(qst::cult_vermina_2) && qst::DA16->currentStage > 197)  ok_ = true;
    if (mys::player->HasPerk(qst::cult_nocturn_2) && qst::TG09->currentStage > 50)   ok_ = true;
    if (mys::player->HasPerk(qst::cult_meridia_2) && qst::DA09->currentStage > 350)  ok_ = true;
    if (mys::player->HasPerk(qst::cult_azura_2)   && qst::DA01->currentStage > 85)   ok_ = true;
    return ok_;
}

void check_horseCert()
{
    auto certificate = qst::handler->LookupForm<RE::TESObjectBOOK>(0x8C3A8A, "devFixes.esp");
    if (u_get_item_count(mys::player, certificate->formID) > 0)  // if have cert
    {
        if (u_get_game_statistic("Horses Owned") > 0)        // and bought horse
    {
             RE::TESForm::LookupByID<RE::TESGlobal>(0xF49BD)->value = 2500.f;                                // price = 2500
             mys::player->RemoveItem(certificate, 1, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);     // remove cert
        }
        else RE::TESForm::LookupByID<RE::TESGlobal>(0xF49BD)->value = 0.f;  // have cert but haven't bought a horse yet = price 0
    }
    else RE::TESForm::LookupByID<RE::TESGlobal>(0xF49BD)->value = 2500.f;   // have no cert = price 2500
}

namespace qst
{
  void check_class_achievements(RE::Actor* player)
  {

    if (qst::startTravel->currentStage > 0)            //  [TRAVELER]
    {
        auto this_ = qst::startTravel;
        auto stage = qst::startTravel->currentStage;
        if    (stage == 1  && qst::stonesBuffStopDay->value)                setStage(this_, 10);  // [1] touch stone
        else if (stage == 10 && u_is_in_city(player))                    setStage(this_, 20);  // [2] travel to city     
        else if (stage == 20 && qMisc("Misc Objectives Completed") > 9) setStage(this_, 30);  // [3] misc quests x10     
        else if (stage == 30 && qMisc("Ingredients Harvested") > 149)   setStage(this_, 40);  // [4] harvested  x150
        else if (stage == 40 && qMisc("Side Quests Completed") > 2)     setStage(this_, 50);  // [5] side quests x3
        else if (stage == 50 && qMisc("Nirnroots Found") > 29)            setStage(this_, 60);  // [6] nirnroots x30
        else if (stage == 60 && qMisc("Side Quests Completed") > 5)     setStage(this_, 70);  // [7] side quests x6 
        else if (stage == 70 && qMisc("Words Of Power Learned") > 14)   setStage(this_, 80);  // [8] words of power x15
        else if (stage == 80 && priestMasksFound() > 9)                    setStage(this_, 90);  // [9] priest masks    x10
        else if (stage == 90 && blackBooksDone() > 5)                    setStage(this_, 100); // [10] black books x6
        u_updQuestTextGlob(this_, qst::currGoalVal);
    }
    else if (qst::startMerc->currentStage > 0)            //  [MERCENARY]
    {
        auto this_ = qst::startMerc;
        auto stage = qst::startMerc->currentStage;
        //        (stage == 1  && witch reward_taken)                                             // [1] reward for witch
        if        (stage == 10 && getContractsDone() > 4)                   setStage(this_, 20);  // [2] contrtacts x5
        else if (stage == 20 && qst::StalleoQuest->currentStage > 60)  setStage(this_, 30);  // [3] stalleo quest
        else if (stage == 30 && getContractsDone()   > 19)               setStage(this_, 40);  // [4] contracts x15
        else if (stage == 40 && qst::currGoalVal->value > 14)           setStage(this_, 50);  // [5] slay mercs x15
        else if (stage == 50 && qMisc("Dungeons Cleared") > 39)        setStage(this_, 60);  // [6] dungeons clear x40
        else if (stage == 60 && getContractsDone() > 44)               setStage(this_, 70);  // [7] contracts x45
        else if (stage == 70 && qst::DLC2RR02->currentStage > 110)     setStage(this_, 80);  // [8] raven_rock
        else if (stage == 80 && tombQuestsDone() > 11)                   setStage(this_, 90);  // [9] thomb quests x12
        u_updQuestTextGlob(this_, qst::currGoalVal);
    }
    else if (qst::startPal->currentStage > 0)        //  [PALADIN]
    {
        auto this_ = qst::startPal;
        auto stage = qst::startPal->currentStage;
        if        (stage == 1  && player->HasPerk(qst::aedraToken))   setStage(this_, 10);   // [1] take bless
        else if (stage == 10 && citizenHelped() > 9)                setStage(this_, 20);   // [2] help citizen x10
        else if (stage == 20 && qst::DBDestroy->currentStage > 30)  setStage(this_, 30);   // [3] destroy DB
        else if (stage == 30 && priestsHelped() > 4)                setStage(this_, 40);   // [4] help priests x5
        else if (stage == 40 && qst::DA10->currentStage > 65)        setStage(this_, 50);   // [5] help tyranus
      //else if (stage == 50 && ring_paladin_papyrus)                setStage(this_, 60);   // [6] ring of paladin
        else if (stage == 60 && qst::currGoalVal->value > 14)        setStage(this_, 70);   // [7] slay necrs x15
        else if (stage == 70 && qst::isAnachoretDead->value > 0)    setStage(this_, 80);   // [8] anachoret
        else if (stage == 80 && essence_found() > 2)                setStage(this_, 90);   // [9] find 3 essence
        u_updQuestTextGlob(this_, qst::currGoalVal);
    }
    else if (qst::startVigil->currentStage > 0)        //  [VIGILANT]
    {
            auto this_ = qst::startVigil;
            auto stage = qst::startVigil->currentStage;
         //        (stage == 1  && take_amulet_papyrus))                  setStage(this_, 10);   // [1] amulet
         //        (stage == 10 && deliver_note_papyrus)                  setStage(this_, 20);   // [2] note to carcetta
        if      (stage == 20 && qMisc("Undead Killed") > 9)           setStage(this_, 30);     // [3] undead x10
        else if (stage == 30 && qMisc("Daedra Killed") > 4)              setStage(this_, 40);      // [4] daedra x5
        else if (stage == 40 && qst::DA11->currentStage > 100)          setStage(this_, 50);     // [5] slay cannibals
        else if (stage == 50 && qst::DA10->currentStage > 65)          setStage(this_, 60);   // [6] tyranus
        else if (stage == 60 && qst::currGoalVal->value > 19)          setStage(this_, 70);   // [7] vampires x20
        else if (stage == 70 && qst::isNamiraBossDead->value)          setStage(this_, 80);   // [8] namira canal boss
        else if (stage == 80 && vigharMovarthDead())                  setStage(this_, 90);   // [9] vighar & movart
        else if (stage == 90 && qst::sephirothQuest->currentStage>50) setStage(this_, 100);  // [10] sephiroth
        u_updQuestTextGlob(this_, qst::currGoalVal);
    }
    else if (qst::startDB->currentStage > 0)        //  [ASSASSIN]
    {
        auto this_ = qst::startDB;
        auto stage = qst::startDB->currentStage;
        if        (stage == 1  && contractsDone())                  setStage(this_, 10);  // [1] contracts
        else if (stage == 10 && qMisc("Poisons Mixed") > 29)      setStage(this_, 20);  // [2] poisons x30
        else if (stage == 20 && qMisc("Murders") > 9)              setStage(this_, 30);  // [3] peaceful x10
        else if (stage == 30 && qMisc("People Killed") > 59)      setStage(this_, 40);  // [4] enemies x60
        else if (stage == 40 && qst::currGoalVal->value > 29)      setStage(this_, 50);  // [5] guards x30
        else if (stage == 50 && qst::currGoalVal->value > 3)      setStage(this_, 60);  // [6] castle mage x4
        else if (stage == 60 && qst::currGoalVal->value > 2)      setStage(this_, 70);  // [7] ambassador x3
        else if (stage == 70 && qst::DLC2RR02->currentStage >90)  setStage(this_, 80);  // [8] morag tong
        u_updQuestTextGlob(this_, qst::currGoalVal);
    }
    else if (qst::startVamp->currentStage > 0)        //  [VAMPIRE]
    {
            auto this_ = qst::startVamp;
            auto stage = qst::startVamp->currentStage;
      //if      (stage == 1  && phials papyrus)                      setStage(this_, 10);   // [1] phials
        if        (stage == 10 && qst::currGoalVal->value > 9)      setStage(this_, 20);   // [2] feed x10
        else if (stage == 20 && qst::isMarkDead->value > 0)          setStage(this_, 30);   // [3] slay Mark
        else if (stage == 30 && qst::isWilhelmDead->value > 0)      setStage(this_, 40);   // [4] slay Wilhelm
        else if (stage == 40 && qst::currGoalVal->value > 19)      setStage(this_, 50);   // [5] vampires x20
        else if (stage == 50 && qst::currGoalVal->value > 9)      setStage(this_, 60);   // [6] dawnguards x10
        else if (stage == 60 && vampArtifacts())                  setStage(this_, 70);   // [7] artifacts x4
        else if (stage == 70 && vigharMovarthDead())              setStage(this_, 80);   // [8] vighar & movart
        else if (stage == 80 && qst::DLC1VQ08->currentStage > 40) setStage(this_, 90);   // [9] harkon dead
        u_updQuestTextGlob(this_, qst::currGoalVal);
    }
    else if (qst::startMage->currentStage > 0)        //  [MAGE]
    {
        auto this_ = qst::startMage;
        auto stage = qst::startMage->currentStage;
        if        (stage == 1  && learnedFountain())                  setStage(this_, 10);  // [1] learn fountain
        else if (stage == 10 && qMisc("Potions Mixed") > 29)      setStage(this_, 20);  // [2] potions x30
        else if (stage == 20 && hasMagicAdeptPerk())              setStage(this_, 30);    // [3] adept
        else if (stage == 30 && qMisc("Spells Learned") > 29)      setStage(this_, 40);    // [4] spells x30
        else if (stage == 40 && qst::DLC2TT2->currentStage >400)  setStage(this_, 50);    // [5] neloth - kill ildari
        else if (stage == 50 && staffsFound() > 3)                  setStage(this_, 60);    // [6] staffs x4
        else if (stage == 60 && solsteimMasksFound() > 2)          setStage(this_, 70);  // [7] solsteim masks
        else if (stage == 70 && qst::MG08->currentStage > 50)      setStage(this_, 80);  // [8] arch-mage
        else if (stage == 80 && ritualQuests() > 4)                  setStage(this_, 90);  // [9] ritual quest x5
        u_updQuestTextGlob(this_, qst::currGoalVal);
    }
    else if (qst::startCult->currentStage > 0)        //  [CULTIST]
    {
        auto this_ = qst::startCult;
        auto stage = qst::startCult->currentStage;
        if        (stage == 1  && player->HasPerk(qst::daedraToken))setStage(this_, 10);  // [1] daedra bless
      //else if (stage == 10 && heart_scroll_papyrus)              setStage(this_, 20);  // [2] heart scroll
        else if (stage == 20 && qMisc("People Killed") > 29)      setStage(this_, 30);    // [3] humanoid x30
        else if (stage == 30 && qst::currGoalVal->value > 14)      setStage(this_, 40);    // [4] cultist  x15
        else if (stage == 40 && chosenDaedra_fullPerks_n_quest()) setStage(this_, 50);    // [5] chosen daedra full
        else if (stage == 50 && blackBooksDone() > 2)              setStage(this_, 60);    // [6] black books x3
        else if (stage == 60 && daedra_guards_killed() > 2)          setStage(this_, 70);  // [7] daedra guards x3
        else if (stage == 70 && daedra_artifacts_have() > 14)      setStage(this_, 80);  // [8] daedra artifact x15
        else if (stage == 80 && haveMiraakSephirothMasks())          setStage(this_, 90);  // [9] miraak & seph masks
        u_updQuestTextGlob(this_, qst::currGoalVal);
    }
    else if (qst::startNecr->currentStage > 0)            //  [NECR]
    {
        auto this_ = qst::startNecr;
        auto stage = qst::startNecr->currentStage;
      //if        (stage == 1  && body_parts_papyrus)                setStage(this_, 10);  // [1] body parts
      //if        (stage == 10 && ritual_scroll_papyrus)            setStage(this_, 20);  // [2] scroll
        if        (stage == 20 && qMisc("Souls Trapped") > 19)    setStage(this_, 30);  // [3] souls x20
        else if (stage == 30 && qMisc("People Killed") > 79)    setStage(this_, 40);  // [4] humanoid x80
        else if (stage == 40 && vampire_dust_have() > 19)        setStage(this_, 50);  // [5] vampire dust x20
        else if (stage == 50 && qst::currGoalVal->value > 14)    setStage(this_, 60);  // [6] necromants x15
        else if (stage == 60 && conjRitualQuestDone())            setStage(this_, 70);  // [7] ritual quest conj
        else if (stage == 70 && equippedConjurPhylactery())        setStage(this_, 80);  // [8] conj phylactery
        else if (stage == 80 && arch_lich_killed() > 2)            setStage(this_, 90);  // [9] arch liches x3
        u_updQuestTextGlob(this_, qst::currGoalVal);
    }
    else if (qst::startSnowElf->currentStage > 0)        //  [SNOW ELF]
    {
        auto this_ = qst::startSnowElf;
        auto stage = qst::startSnowElf->currentStage;
        if      (stage == 40 && dwarven_ghostBosses_killed() > 4)    setStage(this_, 50);  // dwarven boss x5
      //else if (stage == 50 && olve_white_phial_papyrus)            setStage(this_, 60);  // olve white phial
        else if (stage == 60 && qst::DLC1VQ07->currentStage > 80)    setStage(this_, 70);  // find valley
      //else if (stage == 70 && note_in_valley_papyrus)              setStage(this_, 80);  // note in valley
        else if (stage == 80 && qst::DLC1VQ07->currentStage > 110)   setStage(this_, 90);  // kill virtur
        u_updQuestTextGlob(this_, qst::currGoalVal);
    }

    check_horseCert();

  }//enc achieves

};//end namespace

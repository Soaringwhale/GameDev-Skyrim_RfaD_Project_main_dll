
#pragma once

#include "core.h"
#include "utils.h"
#include "ItemCardFixer.h"
#include "gameplay.h"
#include <iostream>
#include <fstream>
#include <windows.h>
#include <shlobj.h>  // to get path to app folder

#ifdef _UNICODE
    #define SetConsoleTitle SetConsoleTitleW            // to avoid char / lpwstr trouble 
#else
    #define SetConsoleTitle SetConsoleTitleA        
#endif

using std::ios;

static std::map <uint32_t, uint16_t>     sf_map;
static std::map <uint16_t, std::string>  sf_tier1_names;
static std::map <uint16_t, std::string>  sf_tier2_names;
static std::map <uint16_t, std::string>  sf_tier3_names;

static std::map<RE::FormID, std::string> x_descriptions;
//static std::map<RE::FormID, std::string> old_descriptions;

static std::vector <RE::TESGlobal*> bindGlobs;
static std::vector <float> keyCodes;

namespace my {        //   namespace with static variables used only in this file, so no need for their extra declarations

    void initGameData()
    {
        LOG("called initObjects()");
        handler = RE::TESDataHandler::GetSingleton();
        fill_gamePointers();
        fill_data();
        fill_translation();
        qst::initQuestVars();
        gameplay::initGameplayData();
        LOG("finished initObjects()");
    }
}

//  struct with static variables that must be visible in any file, requiring extra declarations

float               mys::reserved_MP;        
float               mys::time_delta;
float               mys::micro_timer;
float               mys::ms_compensator;
float               mys::eq_weight;
float               mys::dodge_timer;
uint16_t            mys::vamp_C_state;
uint16_t            mys::nb_hold_state;
uint16_t            mys::xdescr_state;
uint16_t            mys::bossUpdMeter;
bool                mys::xdescr_on;
bool                mys::hasHeavyArmor;
bool                mys::attackKeyHolding;
RE::Actor*          mys::player;
RE::BGSKeyword*     mys::armorHeavyKw;
RE::EffectSetting*  mys::dodge_KD_eff;
RE::SpellItem*      mys::dodge_KD_spl;
RE::BGSPerk*        mys::dodgePerk;
RE::TESGlobal*      mys::speed_cast_glob;
RE::TESGlobal*      mys::gameProcessed;

void mys::init_globs()
{ 
    player = RE::PlayerCharacter::GetSingleton();

    armorHeavyKw    = RE::TESForm::LookupByID<RE::BGSKeyword>(0x6BBD2);
    dodgePerk       = RE::TESForm::LookupByID<RE::BGSPerk>(0x79376);
    dodge_KD_eff    = my::handler->LookupForm<RE::EffectSetting>(0x15D3F4, "devFixes.esp");
    dodge_KD_spl    = my::handler->LookupForm<RE::SpellItem>(0x6AA965, "Requiem.esp");
    speed_cast_glob = my::handler->LookupForm<RE::TESGlobal>(0xBA02F2, "RfaD SSE - Awaken.esp");
    gameProcessed   = my::handler->LookupForm<RE::TESGlobal>(0x7E992,  "RfaD SSE - Awaken.esp");
    
    reserved_MP = 0.f;
    ms_compensator = 0.f;
    eq_weight = 0.f;
    dodge_timer = 0.f;
    hasHeavyArmor = false;
    attackKeyHolding = false;
    xdescr_on = false;
    xdescr_state = 0;
    vamp_C_state = 0;
    nb_hold_state = 0;
    bossUpdMeter = 0;
    time_delta = 0.0166f;            // initial value of delta, later it will recount for fps
    micro_timer = 1.f;

    SKSE::log::info("mys::init_globs() finished");
}


template <typename T>
T* GetFormFromString (const std::string &formIDstr, const std::string &modname)      // to utils
{
    auto form = RE::TESForm::LookupByEditorID(formIDstr);      // skyrim.esm
    if (form && form->As<T>()) {
        return form->As<T>();
    }

    RE::FormID formID = std::stoul(formIDstr, nullptr, 16);    // string to -> hex number

    form = RE::TESDataHandler::GetSingleton()->LookupForm(formID, modname.c_str());
    if (form == nullptr) {
        LOG("GetFormFromString() - can't get form {} , {}", formIDstr, modname);
        return nullptr;
    }

    if (form->As<T>()) {
        return form->As<T>();
    }
    return nullptr;
}

std::string trimEnd (const std::string& str) {
    std::string result = str;
    result.erase(std::find_if(result.rbegin(), result.rend(), [](int ch) { return !std::isspace(ch); }).base(), result.end());
    return result;
}

namespace x_desc
{
    void parseFileLine (std::string line)
    {
        if (line.empty() || line[0] != '0') return;

        // delete spaces at the end
        line.erase(std::find_if(line.rbegin(), line.rend(), [](int i){return !std::isspace(i);}).base(), line.end());

        std::istringstream iss(line);
        std::string token;
        std::vector<std::string> tokens;

        while (std::getline(iss, token, '~')) {     
            tokens.push_back(token);             // split into substrings by ~
        }

        if (tokens.size() < 3) {
            LOG ("parseDescriptionLine() - incorrect format for string - {}", line);
            return;
        }

        auto form = GetFormFromString<RE::TESForm>(tokens[0], tokens[1]);  // find object by (string formID, esp)
        if (form) x_descriptions.emplace(form->formID, tokens[2]);         // add to map - (formID, descr)
    }

    void parseFile()
    {
        LOG("called parseFile()");
        using string = std::string;
        char path[MAX_PATH];
        GetModuleFileNameA(NULL, path, MAX_PATH);  // path to app folder, in our case RfaD SE/
        string dllPath = path;
        size_t lastSlashPos = dllPath.rfind('\\');
        string folderPath = dllPath.substr(0, lastSlashPos + 1);
        string filePath = folderPath + "\\Data\\xItemDescriptions.ini";  // RfaD SE / Data

        std::fstream file (filePath.c_str(), std::ios::in);
        if (!file.is_open()) {
            LOG("Couldn't open description file...");
            return;
        }
        string line;
        while (std::getline(file, line))
        {
            try        { parseFileLine(line); }
            catch(...) { LOG("error parsing line - {}", line); }
        }
    }
}

void on_item_card_upd (RE::ItemCard* itemCard, RE::TESForm* item)
{
    //LOG("called on_item_card_upd()");
    if (x_descriptions.contains(item->formID))
    {
        if (mys::xdescr_on)
        {
           auto newDescrGfx = RE::GFxValue(x_descriptions[item->formID].c_str());          // получаем описание для предмета (из файла)
           itemCard->obj.SetMember(ItemCardFixer::descriptionVar, newDescrGfx);            // назначаем в поле "DF_description" карточки это описание
        }
    }
}

void mys::handle_keyPress (uint32_t keyCode, float hold_time, bool is_up) 
{

    // if (is_up) RE::DebugNotification(std::to_string(keyCode).c_str());        // see pressed keycode in-game

    //LOG("called handle_keyPress() with keycode - {}", keyCode);

    if (keyCode == my::rightHandKeyCode)             // pwatk double-hit fix
    {  
        if (is_up)                   attackKeyHolding = false;
        else if (hold_time > 0.2f) attackKeyHolding = true;
    }

    if (keyCode == 20 && !player->IsInCombat())  return;   // T out of combat

    if (keyCode == keyCodes.at(0))  // dodge
    {
        if (dodge_timer <= 0 && hold_time < 1.0f && !player->HasMagicEffect(dodge_KD_eff) && !hasHeavyArmor && player->HasPerk(dodgePerk)) {
              dodge_timer = 0.78f;
              gameplay::windRunnerCheck(mys::player);
              u_cast_on_self(dodge_KD_spl, player);
        }
    }
    if (keyCode == keyCodes.at(1))  // bats
    {
        if (hold_time > 0.7f && player->HasMagicEffect(my::bloodRivers_allow) && !player->HasMagicEffect(my::bloodRivers_kd)) {
              u_cast_on_self(my::bloodRiversAB, player);
              return;
        }

        if (is_up && dodge_timer <= 0 && player->HasSpell(my::vampirism) && player->HasPerk(my::vamp_1st_perk)
            && !player->HasMagicEffect(my::bats_kd) && !player->HasMagicEffect(my::batForm_eff)) {
              dodge_timer = 1.40f;
              u_cast_on_self(my::become_bats, player);
        }
    }
    if (keyCode == keyCodes.at(2))  // boil
    {
        if (player->HasPerk(my::vamp_left2_perk) && player->HasSpell(my::vampirism) && !player->HasMagicEffect(my::bloodThirst_kd))
        {
              if (is_up) vamp_C_state = 0;
              if (hold_time > 0.3f && vamp_C_state < 1)  // stage 1 hold
              {
                  vamp_C_state = 1;
                  u_playSound(player, my::vampireHeartBeat, 1.0f);
                  player->InstantiateHitShader(my::bloodBubbles_FX, 1.5f, player);  // apply shader on actor
                  return;
              }
              if (hold_time > 1.8f && vamp_C_state < 2 && player->HasSpell(my::bloodRiversAB))  // stage 2 hold
              {
                  vamp_C_state = 2;
                  player->InstantiateHitShader(my::bloodFire_FX, 1.5f, player);
                  return;
              }
              if (hold_time > 2.8f && vamp_C_state < 3 && player->HasSpell(my::bloodRiversAB))  // stage 3 hold
              {
                  vamp_C_state = 3;
                  my::vamp_state_glob->value = 1;
                  u_cast_on_self(my::bloodThirstHoldAB, player);
                  return;
              }
              if (is_up)  // instant tap
              {
                  if (my::vamp_state_glob->value == 0) {
                      float hp_ = player->GetActorValue(RE::ActorValue::kHealth);
                      u_damage_av(player, RE::ActorValue::kHealth, hp_ * 0.25f);
                  }
                  u_cast_on_self(my::bloodThirstAB, player);
              }
        }
    }
    if (keyCode == keyCodes.at(3))  // adrenaline
    {
        if (player->IsInCombat() && player->HasPerk(my::heavy_25_perk) && !player->HasMagicEffect(my::adrenalineKD)) {
              if (is_up)  //  instant tap
              {
                  u_cast_on_self(my::adrenaline_tap, player);
                  RE::DebugNotification(my::adrenaline_text.c_str());
              }
              if (hold_time > 1.8f)  //  hold
              {
                  u_cast_on_self(my::adrenaline_hold, player);
                  RE::DebugNotification(my::adrenalineMax_text.c_str());
              }
        }
    }
    if (keyCode == keyCodes.at(4))  // nb main invis (twilight)
    {
        if (player->HasPerk(my::nb_perk_1) && !player->HasMagicEffect(my::nb_mainEff))
        {
              if (is_up) nb_hold_state = 0;
              if (hold_time > 0.4f && nb_hold_state < 1 && player->HasPerk(my::nb_perk_2) &&
                  !player->HasMagicEffect(my::nb_main_kd)) {
                  nb_hold_state = 1;
                  u_cast_on_self(my::nb_main_holdFF, player);
                  return;
              }
              if (is_up && !player->HasMagicEffect(my::nb_main_kd)) {  // tap
                  my::nb_hitCounter->value = 0;
                  u_cast_on_self(my::nb_mainInvisAb, player);
              }
        }
    }
    if (keyCode == keyCodes.at(5))  // nb sting
    {
        if (player->HasMagicEffect(my::nb_openMode) && !player->HasMagicEffect(my::nb_sting_kd)) {
              if (is_up) {
                  //my::nb_hitCounter->value = 0;
                  u_cast_on_self(my::nb_sting, player);
                  float st = player->GetActorValue(RE::ActorValue::kStamina);
                  u_damage_av(player, RE::ActorValue::kStamina, st * 0.3f);
              }
              if (hold_time > 0.5f) {
                  //my::nb_hitCounter->value = 0;
                  u_cast_on_self(my::nb_sting_hold, player);
                  float st = player->GetActorValue(RE::ActorValue::kStamina);
                  u_damage_av(player, RE::ActorValue::kStamina, st * 0.3f);
              }
        }
    }
    if (keyCode == keyCodes.at(6))  // nb mirror
    {
        if (is_up && my::nb_hitCounter->value > 2.f && !player->HasMagicEffect(my::nb_mirror_kd)) {
              my::nb_hitCounter->value = 0;
              u_cast_on_self (my::nb_Reflect, player);
              float st = player->GetActorValue(RE::ActorValue::kStamina);
              u_damage_av(player, RE::ActorValue::kStamina, st * 0.3f);
        }
    }
    if (keyCode == keyCodes.at(7))  // nb twin
    {
        if (is_up && my::nb_hitCounter->value > 7.f && !player->HasMagicEffect(my::nb_twin_kd)) {
              my::nb_hitCounter->value = 0;
              u_cast_on_self (my::nb_2_Twin_FF, player);
              float st = player->GetActorValue(RE::ActorValue::kStamina);
              float mp = player->GetActorValue(RE::ActorValue::kMagicka);
              u_damage_av(player, RE::ActorValue::kStamina, st * 0.3f);
              u_damage_av(player, RE::ActorValue::kMagicka, mp * 0.3f);
        }
    }
    if (keyCode == keyCodes.at(8))  // cultist
    {
        if (is_up && player->HasPerk(my::cult_boethia_1)) {
              u_cast_on_self(my::boethiaFF, player);
        }
        else if (is_up && player->HasPerk(my::cult_meridia_1) && !player->HasMagicEffect(my::meridia_kd)) {
              u_cast_on_self(my::meridiaFF, player);
        }
        else if (is_up && player->HasPerk(my::cult_nocturn_1)) {
              u_cast_on_self(my::nocturnalFF, player);
        }
        else if (is_up && player->HasPerk(my::cult_vermina_1)) {
              u_cast_on_self(my::verminaFF, player);
        }
    }
    if (keyCode == keyCodes.at(9))  // race
    {
		if (!player->HasMagicEffect(my::race_ab_kd))
		{
            if (is_up && player->HasSpell(my::redguardFF))      u_cast_on_self(my::redguardFF, player); 
			else if (is_up && player->HasSpell(my::argonianFF)) u_cast_on_self(my::argonianFF, player); 
			else if (is_up && player->HasSpell(my::khajeetFF))  u_cast_on_self(my::khajeetFF,  player);
		}
    }
    if (keyCode == 87)   //  F11
    {
        if (xdescr_state == 0) {
              xdescr_on = !xdescr_on;  // switch item descriptions mode
              xdescr_state = 1;
              RE::DebugNotification("Item Description Switch...");
              u_SendInventoryUpdateMessage(player, nullptr);
        }
    }
 }


void apply_levelUp_bonuses()
{

    LOG("called apply_levelUp_bonuses()");

    float baseHP = mys::player->GetBaseActorValue(RE::ActorValue::kHealth);
    float baseST = mys::player->GetBaseActorValue(RE::ActorValue::kStamina);
    float baseMP = mys::player->GetBaseActorValue(RE::ActorValue::kMagicka);

    if (baseHP > 250) baseHP = 250;
    if (baseMP > 250) baseMP = 250;
    if (baseST > 250) {
        if (mys::player->HasSpell(my::doomSteed)) {
            if (baseST > 300) baseST = 300;
        }
		else baseST = 250;
    }
    // HP
    auto hp_bonuses = my::lvlup_hp_bonuses->effects;
    if (baseHP > 109)
    {
        float poisDisease = baseHP * 0.15f, regenWeight = baseHP * 0.6f;
        hp_bonuses[0]->effectItem.magnitude = poisDisease;
        hp_bonuses[1]->effectItem.magnitude = regenWeight;
    }
    else  {
        hp_bonuses[0]->effectItem.magnitude = 1;
        hp_bonuses[1]->effectItem.magnitude = 1;
    }

    // ST
    auto st_bonuses = my::lvlup_st_bonuses->effects;
    if (baseST > 109)
    {
        float armor = 0, regen_st = 0, damage = 0, atkspeed = 0, movespeed = 0, steed = 1.f;

        if (mys::player->HasSpell(my::doomSteed)) steed = 1.25f;

        armor = baseST - 100;
        regen_st = (baseST - 100) * 1.5f;
        atkspeed = (baseST - 100) / 1000;
        movespeed = (baseST - 100) / 10;
        damage = baseST / 10;

        st_bonuses[0]->effectItem.magnitude = armor * steed;
        st_bonuses[1]->effectItem.magnitude = regen_st * steed;
        st_bonuses[2]->effectItem.magnitude = damage * steed;
        st_bonuses[3]->effectItem.magnitude = damage * steed;
        st_bonuses[4]->effectItem.magnitude = atkspeed * steed;
        st_bonuses[5]->effectItem.magnitude = movespeed * steed;
    } 
    else {
        for (auto &eff : st_bonuses) eff->effectItem.magnitude = 0;
    }

    // MP
    auto mp_bonuses = my::lvlup_mp_bonuses->effects;
    if (baseMP > 109)
    {
        float costRedux = 0, magres = 0, regen_mp = 0;
        regen_mp = baseMP;

        if (baseMP < 150) {
            costRedux = (baseMP / 10 - 5 + ((baseMP / 10) - 10));
            magres = (baseMP - 100) / 10;
        } else {
            costRedux = baseMP / 10;
            magres = 5 + (baseMP - 150) / 5;
        }

        mp_bonuses[0]->effectItem.magnitude = costRedux;
        mp_bonuses[1]->effectItem.magnitude = magres;
        mp_bonuses[2]->effectItem.magnitude = regen_mp;
    }
    else {
        for (auto& eff : mp_bonuses) eff->effectItem.magnitude = 1;
    }

    mys::player->RemovePerk(my::gm_lvlup_bonuses);
    mys::player->AddPerk(my::gm_lvlup_bonuses);

}


void sf_dispel_all()
{
        LOG("called sf_dispel_all()");
        my::glob_destr_1->value = 0;
        my::glob_destr_2->value = 0;
        my::glob_destr_3->value = 0;
        my::glob_alter_1->value = 0;
        my::glob_alter_2->value = 0;
        my::glob_alter_3->value = 0;
 
        if (mys::player->HasSpell(my::sf_absrb_const))   mys::player->RemoveSpell(my::sf_absrb_const);
        if (mys::player->HasSpell(my::sf_armor_const))   mys::player->RemoveSpell(my::sf_armor_const);
        if (mys::player->HasSpell(my::sf_penet_const))   mys::player->RemoveSpell(my::sf_penet_const);
        if (mys::player->HasSpell(my::sf_rflct_const))   mys::player->RemoveSpell(my::sf_rflct_const);
        if (mys::player->HasSpell(my::sf_speed_const))   mys::player->RemoveSpell(my::sf_speed_const); 
        if (mys::player->HasSpell(my::sf_stamina_const)) mys::player->RemoveSpell(my::sf_stamina_const); 

        auto sf_cloak_effects = u_get_effects_by_keyword(mys::player, my::sf_cloakEff_KW);
        if (!sf_cloak_effects.empty()) {
            for (auto eff : sf_cloak_effects) eff->Dispel(true);
        }

        mys::reserved_MP = 0;
        my::sf_descr->magicItemDescription = my::sf_noEffects;
        RE::DebugNotification(my::sf_all_clear.c_str());
}

void sf_manaReserve_buff_cast(uint16_t buff_index)
{
        LOG("called sf_manaReserve_cast()");

        float currMana = mys::player->GetActorValue(RE::ActorValue::kMagicka);
        float baseMana = mys::player->GetBaseActorValue(RE::ActorValue::kMagicka);

        if (currMana < 100) return;

        if      (buff_index == 1 && baseMana < 170)  return;        // speed
        else if (buff_index == 2 && baseMana < 180)  return;        // penetr
        else if (buff_index == 3 && baseMana < 170)  return;        // armor
        else if (buff_index == 4 && baseMana < 180)  return;        // reflect
        else if (buff_index == 5 && baseMana < 170)  return;        // absorb
        else if (buff_index == 6 && baseMana < 170)  return;        // stamina
        
        RE::SpellItem* buff = nullptr;

        if      (buff_index == 1)   buff = my::sf_speed_const;            
        else if (buff_index == 2)   buff = my::sf_penet_const;
        else if (buff_index == 3)   buff = my::sf_armor_const;
        else if (buff_index == 4)   buff = my::sf_rflct_const;
        else if (buff_index == 5)   buff = my::sf_absrb_const;
        else if (buff_index == 6)   buff = my::sf_stamina_const;

        if (buff) {
            if (mys::player->HasSpell(buff)) {
                mys::player->RemoveSpell(buff);
            } else {
                mys::player->AddSpell(buff);
            }
        }
        my::sf_handle_reserved_MP();        //  check active manareserve buffs, apply new hooks
}

void my::sf_handle_reserved_MP()
{
        LOG("called my::sf_handle_reserved_MP()");

        mys::reserved_MP = 0;

        if (!mys::player) return;
        if (!mys::player->HasPerk(sf_perk_3)) return;
        
        if (mys::player->HasSpell(sf_speed_const))   mys::reserved_MP += 50.0;
        if (mys::player->HasSpell(sf_penet_const))   mys::reserved_MP += 60.0;
        if (mys::player->HasSpell(sf_armor_const))   mys::reserved_MP += 50.0;
        if (mys::player->HasSpell(sf_rflct_const))   mys::reserved_MP += 70.0;
        if (mys::player->HasSpell(sf_absrb_const))   mys::reserved_MP += 70.0;
        if (mys::player->HasSpell(sf_stamina_const)) mys::reserved_MP += 40.0;

        float maxMana = u_get_actor_value_max(mys::player, RE::ActorValue::kMagicka);
        float currMana = mys::player->GetActorValue(RE::ActorValue::kMagicka);
        float remainedMaxMana = maxMana - mys::reserved_MP;

        if (currMana > remainedMaxMana) {
            float diff = currMana - remainedMaxMana;
            mys::player->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kMagicka, -diff);
        }
}


bool inMap (uint16_t tier, uint16_t val)    // checks if map contains key
{
        if (tier == 1) {
            if (sf_tier1_names.count(val)) return true;
        }
        else if (tier == 2) {
            if (sf_tier2_names.count(val)) return true;
        }
        else if (tier == 3) {
            if (sf_tier3_names.count(val)) return true;
        }
        return false;
}

void sf_update_descr()        // active effects info
{
    std::string s = "";
    uint16_t val = my::glob_destr_1->value;
    if (val > 0 && inMap(1, val)) s += sf_tier1_names.at(val) + ", ";
    val = my::glob_alter_1->value;
    if (val > 0 && inMap(1, val)) s += sf_tier1_names.at(val) + ", ";
    val = my::glob_destr_2->value;
    if (val > 0 && inMap(2, val)) s += sf_tier2_names.at(val) + ", ";
    val = my::glob_alter_2->value;
    if (val > 0 && inMap(2, val)) s += sf_tier2_names.at(val) + ", ";
    val = my::glob_destr_3->value;
    if (val > 0 && inMap(3, val)) s += sf_tier3_names.at(val) + ", ";
    val = my::glob_alter_3->value;
    if (val > 0 && inMap(3, val)) s += sf_tier3_names.at(val) + ", ";

    if (mys::player->HasSpell(my::sf_absrb_const)) s += my::sf_absorb_text + ", ";
    if (mys::player->HasSpell(my::sf_armor_const)) s += my::sf_armor_text + ", ";
    if (mys::player->HasSpell(my::sf_penet_const)) s += my::sf_penetr_text + ", ";
    if (mys::player->HasSpell(my::sf_rflct_const)) s += my::sf_reflect_text + ", ";
    if (mys::player->HasSpell(my::sf_speed_const)) s += my::sf_speed_text + ", ";
    if (mys::player->HasSpell(my::sf_stamina_const)) s += my::sf_stamina_text + ", ";

    s.pop_back(); s.pop_back();
    my::sf_descr->magicItemDescription = s;
}


inline void sf_add (RE::TESGlobal *glob, uint16_t new_val, uint16_t tier)
{
        uint16_t    curr_val = glob->value;
        std::string add = my::sf_add_new;
        std::string rem = my::sf_rem_current;

        if (tier == 1) {
            if (inMap(1, curr_val)) rem += sf_tier1_names.at(curr_val);
            if (inMap(1, new_val))  add += sf_tier1_names.at(new_val);
        }
        else if (tier == 2) {
            if (inMap(2, curr_val)) rem += sf_tier2_names.at(curr_val);
            if (inMap(2, new_val))  add += sf_tier2_names.at(new_val);
        }
        else if (tier == 3) {
            if (inMap(3, curr_val)) rem += sf_tier3_names.at(curr_val);
            if (inMap(3, new_val))  add += sf_tier3_names.at(new_val);
        }

        if (curr_val > 0) RE::DebugNotification(rem.c_str());
        glob->value = new_val;
        RE::DebugNotification(add.c_str());
}

inline void sf_remove (RE::TESGlobal* glob)
{
        glob->value = 0;
        RE::DebugNotification(my::sf_removed.c_str());
}

inline void  sf_handle_glob (RE::TESGlobal* glob_dest, RE::TESGlobal* glob_alt, uint16_t index, uint16_t tier)
{

           if (index < 10) {
                if (glob_dest->value == index)  sf_remove(glob_dest);
                else    sf_add(glob_dest, index, tier);
           }
           else {
                if (glob_alt->value == index)  sf_remove(glob_alt);
                else    sf_add(glob_alt, index, tier);
           }
}

void my::sf_handle(RE::ActiveEffect* eff, RE::EffectSetting* baseEff) 
{

       //LOG("called sf_handle()");
           
       if (baseEff->HasKeyword(my::sf_dispel_KW)) {
           sf_dispel_all();
           return;
       }          

       uint32_t formID = eff->spell->GetLocalFormID();
       if(!sf_map.count(formID))  return;
       uint16_t spell_idx = sf_map.at(formID);    

       const char* spellName = eff->spell->GetName();
       if      (spellName[1] == '1')  sf_handle_glob(my::glob_destr_1, my::glob_alter_1, spell_idx, 1);     //  sf 1 rank 
       else if (spellName[1] == '2')  sf_handle_glob(my::glob_destr_2, my::glob_alter_2, spell_idx, 2);     //  sf 2 rank
       else if (spellName[1] == '3')  sf_handle_glob(my::glob_destr_3, my::glob_alter_3, spell_idx, 3);     //  sf 3 rank
       else if (spellName[1] == 'C')  sf_manaReserve_buff_cast(spell_idx);                                 //  sf reserve buffs
       sf_update_descr();

}


inline float sf_get_manaDrain()  // SF
{
        float drain = 0;
        if (my::glob_destr_1->value > 0) drain += 15;    //   drain mana with every hit
        if (my::glob_destr_2->value > 0) drain += 15;
        //if (my::glob_alter_1->value > 0) drain += 18;
        if (my::glob_alter_2->value > 0) drain += 15;
        return drain;
}


float reflectedArrow_dmg(float dealt_dmg)
{
        LOG("called reflectedArrow_dmg()");

        auto weap = u_get_weapon(mys::player, false);
        if (!my::reflectedArrow || !my::reflectedArrow->ammoSource || !weap) return 50.f;
        float weapDmg = weap->attackDamage;
        float avPowerMod = mys::player->GetActorValue(RE::ActorValue::kMarksmanPowerModifier);
        float penetr = mys::player->GetActorValue(RE::ActorValue::kMarksmanModifier);
        if (penetr > 80.f) penetr = 80.f;
        float armor = mys::player->GetActorValue(RE::ActorValue::kDamageResist);
        float penetratedArmor = armor * (1 - penetr / 100);  // for ex armor was 1000, with 30 penetr will be 700
        float armorFactorMult = (1 - penetratedArmor / 1000);  // for ex armor after penetration is 200, so mult will be 0.8
        if (armorFactorMult < 0.2f) armorFactorMult = 0.2f;
        if (armorFactorMult > 1.0f) armorFactorMult = 1.0f;
        float arrowRes = mys::player->GetActorValue(RE::ActorValue::kSpeechcraftSkillAdvance);
        LOG("dealt_dmg_param - {}, weapDmg - {}, powerMod - {}, penetr - {}, armorFactorMult - {}", dealt_dmg, weapDmg, avPowerMod, penetr, armorFactorMult);

        //LOG("finished reflectedArrow_dmg()");

        return ((dealt_dmg + weapDmg*1.6f) * (1 + avPowerMod / 100)) * armorFactorMult * (1 - arrowRes/200);
}


float manaShield (RE::Actor* target, float damage, RE::TESDataHandler* handler, bool sf)
{
        LOG("called manaShield()");

        float manaPerDmg = sf ? 3.f : 4.f;        // Magicka cost per 1 phys damage         
        float absorbPercent = sf ? 0.25f : 0.2f;    // absorb percent

        float currentMana = target->GetActorValue(RE::ActorValue::kMagicka);
        float dmgToHP = 0;
        float dmgAbsorbed = 0;
        float manaSpent = 0;

        if (currentMana < damage * absorbPercent * manaPerDmg  ||  currentMana < 35)        // break shield if  damage was > mana
        {   
            dmgToHP = damage - (currentMana / manaPerDmg);
            dmgAbsorbed = damage - dmgToHP;
            manaSpent = dmgAbsorbed * manaPerDmg;
            target->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kMagicka, -manaSpent);
            RE::SpellItem* breakSpell = handler->LookupForm<RE::SpellItem>(0x57C91C, "RfaD SSE - Awaken.esp");  // refrain explosion and dispel
            target->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)->CastSpellImmediate(breakSpell, false, nullptr, 1.f, false, 0.f, target);
            return dmgToHP;
        }

        dmgToHP = damage * (1 - absorbPercent);
        dmgAbsorbed = damage - dmgToHP;
        auto req_incoming_damage = RE::GameSettingCollection::GetSingleton()->GetSetting("fDiffMultHPToPCL")->GetFloat();   // fDiffMultHPByPCL for dealth dmg
        manaSpent = dmgAbsorbed * manaPerDmg * req_incoming_damage;
        target->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kMagicka, -manaSpent);

        float dmg_delta = damage - dmgToHP;
        return dmg_delta;
}


float convert_physDmg_toFrost (RE::Actor* target, RE::HitData* hit_data, float percent)
{
    LOG("convert_physDmg_toFrost()");

    float physDmgBeforeResist = hit_data->physicalDamage;
    float physDmgAfterResist  = hit_data->totalDamage;

    float dmgToBecomeFrost = physDmgBeforeResist * percent;
    float remained_physDmg = physDmgAfterResist * (1 - percent);

    float frostRes = target->GetActorValue(RE::ActorValue::kResistFrost);
    float magRes = target->GetActorValue(RE::ActorValue::kResistMagic);
    if (frostRes > 75) { frostRes = 75; }
    if (magRes   > 75) { magRes   = 75; }
        
    float finalFrostDmg = dmgToBecomeFrost * (1 - (frostRes/100)) * (1 - (magRes/100)); 

    auto damage_resist = 1.f;

    target->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kHealth,  -finalFrostDmg);            //  deal "frost" damage to hp/st
    target->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kStamina, -finalFrostDmg*0.5f);    //  

    float dmg_delta = physDmgAfterResist - remained_physDmg;

    // SKSE::log::info("physDmg was = {}, physDmg become = {}, frost res = {}, frost dmg become = {}", damage, resultPhysDmg, targetFrostRes, finalFrostDmg);

    return dmg_delta;    // amount of phys damage that became frost
}


void sword_injure (RE::Actor* agressor, RE::Actor* target, RE::HitData *hit_data, bool isPwatk)
{
    LOG("called sword_injure()");

    uint16_t chance = 14;

    if (agressor->HasPerk(my::swordFocus2)) chance += 5;
    if (agressor->HasPerk(my::swordFocus3)) chance += 5;

    uint16_t random = rand() % 100;

    if (random < chance) {
        //LOG("sword_injure() - applying injure");
        float dmg = hit_data->totalDamage * 0.7f;            //  phys dmg resisted  
        if (!isPwatk) dmg *= 2;
        if        (u_worn_has_keyword(target, my::slash_5)) dmg *= 0.60f;
        else if (u_worn_has_keyword(target, my::slash_4)) dmg *= 0.68f;
        else if (u_worn_has_keyword(target, my::slash_3)) dmg *= 0.76f;
        else if (u_worn_has_keyword(target, my::slash_2)) dmg *= 0.84f;
        else if (u_worn_has_keyword(target, my::slash_1)) dmg *= 0.92f;
        
        u_playSound(target, my::slashSound, 2.f);
        agressor->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)->CastSpellImmediate(my::injureSpell, false, target, 1.f, false, dmg, agressor);        // onHit
    }
}


void handle_stagger (RE::Actor* target, RE::TESObjectWEAP* weap, bool pwatk)
{
    RE::SpellItem* staggerSpell = nullptr;
    int random = rand() % 10;
    if (weap->GetWeaponType() < RE::WEAPON_TYPE(5))  // 1H
    {
       if (pwatk) {
            staggerSpell = my::handler->LookupForm<RE::SpellItem>(0x2E9A1D, "Requiem.esp");    // stagger power 1H
            random += 2;
       }
    }
    else   //  2H
    {
       if (pwatk) {
            staggerSpell = my::handler->LookupForm<RE::SpellItem>(0x2E9A1D, "Requiem.esp");    // stagger power 2H
            random += 3;
       }
       else {
            staggerSpell = my::handler->LookupForm<RE::SpellItem>(0x32C8B1, "Requiem.esp");     // stagger light 2H
       }
    }

    if (staggerSpell && random > 5) {
         u_cast_on_self(staggerSpell, target);
    }
}


 std::string trimmed_str (float val)
 {
    std::string s = std::to_string(val);
    while (s.length() > 4) s.pop_back();
    return s;
 }

void print_dmg_to_console (RE::Actor* agr, RE::Actor* target, RE::TESObjectWEAP* weap, float damage) 
{
    std::string s = "hit - " + std::string(agr->GetName()) + ", ";
    if (weap->GetWeaponType() < RE::WEAPON_TYPE(5))   // 1H
    {
         s += "1h_powerMod - " + std::to_string(int(agr->GetActorValue(RE::ActorValue::kOneHandedPowerModifier))) + ", ";
         s += "1h_penetr - " + std::to_string(int(agr->GetActorValue(RE::ActorValue::kOneHandedModifier))) + ", ";
    }
    else  // 2H
    {
         s += "2h_powerMod - " + std::to_string(int(agr->GetActorValue(RE::ActorValue::kTwoHandedPowerModifier))) + ", ";
         s += "2h_penetr - " + std::to_string(int(agr->GetActorValue(RE::ActorValue::kTwoHandedModifier))) + ", ";
    }
   
	float total_atkDmgMult = 1.f, total_pwAtkDmgMult = 1.f;      
	// прогоняем 1.f через ентри перков агрессора, что-бы узнать суммарные мульты. При этом проверяются кондишены, и мы получаем реальный мульт, там где кондишены не прошли игнорируется.
	RE::BGSEntryPoint::HandleEntryPoint (RE::BGSEntryPoint::ENTRY_POINT::kModAttackDamage, agr, weap, target, std::addressof(total_atkDmgMult));
    RE::BGSEntryPoint::HandleEntryPoint (RE::BGSEntryPoint::ENTRY_POINT::kModPowerAttackDamage, agr, weap, target, std::addressof(total_pwAtkDmgMult));

    s += "perks_dmgMult - " + trimmed_str(total_atkDmgMult) + ", ";
    s += "pwAtkDmgMult - " + trimmed_str(total_pwAtkDmgMult) + ",  ";
    s += "HP was - " + std::to_string(int(target->GetActorValue(RE::ActorValue::kHealth))) + ",  ";
    s += "damage - " + std::to_string(int(damage));

    RE::ConsoleLog::GetSingleton()->Print(s.c_str());  // print to console
}

float allOnHitEffects (RE::Actor* target, RE::HitData hit_data)   //  [fires at the beginning of attack process, but cannot interrupt attack]
{
    
    LOG("called allOnHitEffects()");

    float damage = hit_data.totalDamage;        //  damage after resist
    bool isPowerAtk = hit_data.flags.any(RE::HitData::Flag::kPowerAttack);  // any() checks bit flag
    bool isBlocked = hit_data.flags.any(RE::HitData::Flag::kBlocked);
    bool isBash = hit_data.flags.any(RE::HitData::Flag::kBash);
    bool isMelee = hit_data.flags.any(RE::HitData::Flag::kMeleeAttack);

    float manaShield_dmg_delta = 0;
    float convert_dmg_delta = 0;

    auto agressor = hit_data.aggressor.get().get();
    auto weap     = hit_data.weapon;                        // weap object
	auto weapRef  = agressor->GetEquippedEntryData(false);  // weap ref for extraData etc.

    if (!target || !agressor) return damage;
    LOG("allOnHitEffects___1");

    if (!isMelee && my::reflectedArrow && agressor->HasMagicEffect(my::arrow_reflect))    // when reflected arrow hits player
    {    
        float reflectedArrowDmg = reflectedArrow_dmg(damage);
        //target->SetGraphVariableFloat("StaggerMagnitude", 5.f);
        target->NotifyAnimationGraph("StaggerStart");
        u_kill_projectile(my::reflectedArrow);
        my::reflectedArrow = nullptr;
        return reflectedArrowDmg;
    }

    if (!weap)        // still can be bash
    {
        if (isBash) {                                                                                                
             u_cast(my::bash_kd_self, agressor, agressor);
             if (agressor->IsPlayerRef()) {
                 float stCost = 0.1f * u_get_actor_value_max(agressor, RE::ActorValue::kStamina);
                 agressor->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kStamina, -stCost);
             }
             return damage;
        }
        return damage;
    }

	if (!my::mercenaryQst->IsCompleted()) {
        target->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kStamina, -damage * 0.4f);
    }
	
    if (weap->HasKeyword(my::sword_1h)  &&  agressor->HasPerk(my::swordFocus1) && !isBlocked && !target->HasMagicEffect(my::injureEff) && !target->HasKeyword(my::actorDwarven)) {
        sword_injure(agressor, target, &hit_data, isPowerAtk);
    }

    if (target->IsPlayerRef())
    {
          LOG("allOnHitEffects___target_was_player");

          handle_stagger (target, weap, isPowerAtk);

          if (target->HasMagicEffect(my::nb_hold_slowTime)) {    // nb slowtime and invert damage to heal
                 target->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kHealth, damage);
                 return 1.0f;  // player will get 1 dmg
          }

          if (target->HasMagicEffect(my::nb_fullReflect)) {  // nb full reflect ability
                 agressor->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kHealth, -damage*2);
                 return 1.0f;
          }

          RE::TESForm* form = target->GetEquippedObject(false);                 //  object in right hand
          if (form) {
              if (u_form_has_keyword(form, my::breakableBowKW)) {             //  check form's keywords, don't need cast to weapon
                  u_cast_on_self(my::breakBowSpl, target);
              }
          }

          if (target->HasMagicEffect(my::manaShield_2)) {
              manaShield_dmg_delta = manaShield(target, damage, my::handler, false);
          } 
          else if (target->HasMagicEffect(my::manaShield_4_sf)) {
              manaShield_dmg_delta = manaShield(target, damage, my::handler, true);
          }
          if (target->HasPerk(my::snowElf_anabioz)) {
              convert_dmg_delta = convert_physDmg_toFrost(target, &hit_data, 0.3f);
          }

		  print_dmg_to_console(agressor, target, weap, damage-manaShield_dmg_delta-convert_dmg_delta);

    }// end targer->player only
    else if (agressor->IsPlayerRef())
    {
        LOG("allOnHitEffects___agressor_was_player");

        if (weapRef && weapRef->IsPoisoned())         // oil
		{  
              bool conv = gameplay::oil_proc (agressor, target, &hit_data, isPowerAtk);
              if (conv) convert_dmg_delta = convert_physDmg_toFrost (target, &hit_data, 0.3f);  // frost oil
        }
        float sf_manaDrain = sf_get_manaDrain();                    //  SF
        if (sf_manaDrain > 0)    {
            mys::player->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kMagicka, -sf_manaDrain);
        }
        if (mys::player->HasMagicEffect(my::nb_openMode)) {
            my::nb_hitCounter->value += 1;
        }
        if (my::nb_magicBlade->value == 1.f) {                        //  NB magicBlade
            float mp = u_get_actor_value_max(mys::player, RE::ActorValue::kMagicka) * 0.06f;
            mys::player->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kMagicka, -mp);
            if (mys::player->GetActorValue(RE::ActorValue::kMagicka) > mp*2) {
                  float magRes = target->GetActorValue(RE::ActorValue::kResistMagic);
                  float dmg = (mp * 8) * (1 - magRes / 100);
                  target->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kHealth, -dmg);
            } 
        }
    }

    LOG("finished allOnHitEffects()");

    return damage - manaShield_dmg_delta - convert_dmg_delta;
}


inline void handle_weaponBased_attack_speed()             //    compensation depents on weapon type and current atk speed
{

    LOG("called handle_weaponBased_attack_speed()");

    float atkSpeed  =  mys::player->GetActorValue(RE::ActorValue::kWeaponSpeedMult);    //  for example atkSpeed is 1.4
    const auto weap =  u_get_weapon(mys::player, false);

    if (atkSpeed < 1 || !weap || !weap->IsMelee()) return;

    float x = (atkSpeed - 1) / 10;            // then atkSpeed factor is 0.04                        
    float y = 0;

    if (weap->IsOneHandedDagger())        y = 1.5;
    else if (weap->IsOneHandedSword())  y = 2;        // 0.04 * 2  =  0.08  debuff for sword with atkspeed 1.4
    else if (weap->IsOneHandedAxe())    y = 3;      // 0.04 * 3  =  0.12  debuff for axe with atkspeed 1.4
    else if (weap->IsOneHandedMace())   y = 3.5;    // 0.04 * 3.5 = 0.14  debuff for mace with atkspeed 1.4
    else return;    
 
    float debuff_magn = x * y;

    if (u_worn_has_keyword(mys::player, mys::armorHeavyKw)) debuff_magn *= 1.35f;

    mys::player->RemoveSpell(my::atkSpeedDebuff);
    auto effects = my::atkSpeedDebuff->effects;
    effects[0]->effectItem.magnitude = debuff_magn;
    mys::player->AddSpell(my::atkSpeedDebuff);
}


inline void handle_moveSpeed_compensation()                   //  depents on armor weight and current movespeed                  
{
    LOG("called handle_moveSpeed_compensation()");

    if (!mys::hasHeavyArmor) {mys::ms_compensator = 0; return; }     //  only for heavy armor = on
    float speedMult  = mys::player->GetActorValue(RE::ActorValue::kSpeedMult);
    float smooth = 0.f;
    if (speedMult < 101.f) return;
    if (speedMult < 110.f) {
          smooth = (0.1f - (speedMult-100)/100);
    }
    float eq_weight = u_get_worn_equip_weight (mys::player);    // armor + weapon weight
    
    // auto total_inv_weight = mys::player->GetActorValue(RE::ActorValue::kInventoryWeight);        //  for total inventory weight

    mys::ms_compensator = ((eq_weight*1.6f)/1000 * (1+(speedMult-100)/150)) - smooth;

}


void player_anim_handle(RE::BSTEventSink<RE::BSAnimationGraphEvent>   *this_,
                        RE::BSAnimationGraphEvent                      *event,
                        RE::BSTEventSource<RE::BSAnimationGraphEvent> *dispatcher)

{
    //SKSE::log::info("called player_anim_handle()");

    const auto animation = Utils_anim_namespace::try_find_animation(event->tag.c_str());

    //LOG ("got animation name - {}", event->tag.c_str());    // see all animation names

    if (animation == Utils_anim_namespace::AnimationEvent::kWeaponSwing ||
        animation == Utils_anim_namespace::AnimationEvent::kWeaponLeftSwing) {
            u_cast_on_self(my::stressSpell, mys::player);
    }
    else if (animation == Utils_anim_namespace::AnimationEvent::kBowDraw ||
             animation == Utils_anim_namespace::AnimationEvent::kBowDrawStart) {
            u_cast_on_self(my::stressStart, mys::player);
    }
    else if (animation == Utils_anim_namespace::AnimationEvent::kBowZoomStart) {    
           if (mys::player->HasPerk(my::bowSlowTimePerk)) {
                float maxst = u_get_actor_value_max(mys::player, RE::ActorValue::kStamina);
                mys::player->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)
                    ->CastSpellImmediate(my::zoomStress, false, nullptr, 1.f, false, 22.f+maxst*0.11f, mys::player);
           } 
    } 
    else if (animation == Utils_anim_namespace::AnimationEvent::kBowRelease  ||
             animation == Utils_anim_namespace::AnimationEvent::kArrowDetach ||
             animation == Utils_anim_namespace::AnimationEvent::kInterruptCast ) {
             u_cast_on_self(my::stressDispel, mys::player);
    }
    else if (animation == Utils_anim_namespace::AnimationEvent::kWeapEquip_Out) {                        
            const auto weap = u_get_weapon(mys::player, false);
            if (weap && weap->IsCrossbow())  mys::player->NotifyAnimationGraph("reloadStart");        //  start reload anim after every equip crossbow
    }
    //else if (animation == Utils_anim_namespace::AnimationEvent::kAttackStop) {
    //       u_cast_on_self(my::stressDispel, mys::player);
    //}
}


bool is_const (RE::EffectSetting* baseEff) {
    return  baseEff->data.castingType == RE::MagicSystem::CastingType::kConstantEffect;
}

void handle_capValues_onAdjustEff (RE::Actor* actor, RE::ActorValue av, float min_cap, float max_cap, RE::SpellItem *regulator, RE::ActiveEffect *eff)
{
    LOG("called handle_capValues_onAdjustEff()"); 

    float av_val = actor->GetActorValue(av);
    float avValPlusMagn  = av_val + eff->magnitude;
    float avValMinusMagn = av_val - eff->magnitude;
    auto baseEff = eff->effect->baseEffect;

    if (av_val >= max_cap)                  // if av value is already > cap
    {
        if      (!is_const(baseEff)) eff->magnitude = 0;
        else if (is_const(baseEff) && !baseEff->IsDetrimental()) regulator->effects[0]->effectItem.magnitude += eff->magnitude;              
    }
    else if (avValPlusMagn > max_cap)      // if current av_val + adding effect magnitude will be > cap
    {
        float diff = avValPlusMagn - max_cap;
        if    (!is_const(baseEff)) eff->magnitude -= diff;
        else if(is_const(baseEff) && !baseEff->IsDetrimental())  regulator->effects[0]->effectItem.magnitude += diff;
    }
    else if (av_val <= min_cap && (baseEff->IsDetrimental() || av == RE::ActorValue::kShoutRecoveryMult))
    {
        eff->magnitude = 0;
    }
    else if (avValMinusMagn < min_cap && (baseEff->IsDetrimental() || av == RE::ActorValue::kShoutRecoveryMult))
    {
        float diff = min_cap - avValMinusMagn;
        if  (!is_const(baseEff)) eff->magnitude -= (min_cap - avValMinusMagn);
        else                     regulator->effects[0]->effectItem.magnitude -= diff;
        
    }
    actor->RemoveSpell(regulator);
    actor->AddSpell(regulator);

}


// адрес этой фнк мы передаем методу show(), который принимает std::function<void(unsigned int)>, т.е. функцию с такой сигнатурой.
// когда игрок выберет что-то в боксе, будет вызвана эта колбек-функция, ей в параметр прилетит результат выбора.
// это особенность отложенного вызова, мы не вызываем эту фнк сами, она будет вызвана позже, после выбора,
// нет смысла делать ей возвращаемое значение, т.к. мы не можем написать например int x = ourFunc(), т.к. это означало бы вызов в определенном месте кода, а функция вызывается игровым событием.


void msgBoxChoiceCallback (unsigned int result)        // [full automated]  [called only after player chose key in messageBox]
{        

    int32_t code = 0;
    if      (result == 0) code = 45;    // X             // msgBox choice to keyCode  [reached maximum of 10 choises]
    else if (result == 1) code = 46;    // C    
    else if (result == 2) code = 47;    // V
    else if (result == 3) code = 20;    // T
    else if (result == 4) code = 21;    // Y
    else if (result == 5) code = 2;        // 1
    else if (result == 6) code = 3;        // 2
    else if (result == 7) code = 4;        // 3

    if (my::instantAbIndex == 0) {                        
        if        (result == 8) code = 42;  // lShift
        else if (result == 9) code = 56;  // lAlt
    }
    else {
        if        (result == 8) code = 259;  // Mouse4
        else if (result == 9) code = 260;  // Mouse5
    }

    bindGlobs.at(my::instantAbIndex)->value = code;        // fill glob for game restarts
    keyCodes.at(my::instantAbIndex) = code;                // fill vector
}


bool on_drink_potion (RE::Actor* actor, RE::AlchemyItem* potion, RE::ExtraDataList* extra_data_list) 
{

    LOG("called on_drink_potion()");

    if (!potion->effects.empty())
    {
        for (auto& eff : potion->effects)        {
             // ...
        }
    }
    return true;                // just apply potion.  return false - decline applying
}


void on_apply_poison (RE::InventoryEntryData* data, RE::AlchemyItem* poison, int count) 
{
    LOG ("called on_apply_poison()");
	if (poison->HasKeyword(my::oil_keyword))
	{
        u_cast_on_self (my::oil_after_use, mys::player);   // papyrus does all logic 

        //if (const auto queue = RE::UIMessageQueue::GetSingleton())
		//{
        //  queue->AddMessage(RE::InventoryMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kHide, nullptr);  // close inventory
        //  queue->AddMessage(RE::TweenMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kHide, nullptr);      // close tween menu (necessary)
        //  auto camera = RE::PlayerCamera::GetSingleton();
        //  if (camera->IsInFirstPerson()) camera->ForceThirdPerson();  // force camera to 3rd person for oil anim
        //}
        //mys::player->NotifyAnimationGraph("IdleCombatStretchingStart");  // play anim event
	}
}

void snowElf_applyChant ()
{
    RE::EnchantmentItem* theEnch = nullptr;
    theEnch = my::snowElfEnch;
    uint16_t level = mys::player->GetLevel();
    float frostDmg = (level * 10) / 3.5f;
    if (frostDmg > 145.f) frostDmg = 145.f;
    float resDebuff = level * 0.2f;
    if (resDebuff > 10.f) resDebuff = 10.f;
    theEnch->effects[0]->effectItem.magnitude = frostDmg;                     //  ench magnitudes, only for description
    theEnch->effects[1]->effectItem.magnitude = resDebuff;
    my::snowElf_insteadEnch->effects[0]->effectItem.magnitude = resDebuff;   //  onHit spell magnitudes, real

    auto entryData = mys::player->GetEquippedEntryData(false);
    if (!entryData) return;
    // theEnch->data.chargeOverride = 0xFFFF;                            //  max charge if not infinite (c)
    auto datalist = entryData->extraLists->front();

    u_enchant_equipped_weapon(mys::player->GetInventoryChanges(), entryData->object, datalist, theEnch, 0);
}


void on_adjust_active_effect (RE::ActiveEffect *eff, float power, bool &unk)
{
       if (!eff || !eff->effect || !eff->effect->baseEffect) return;
       //LOG("called on_adjust_active_effect(), eff name - {}", eff->GetBaseObject()->fullName);

        auto baseEff = eff->GetBaseObject();

        RE::Actor* target = skyrim_cast<RE::Actor*>(eff->target);   // getTargetActor() doesn't work, we must skyrim_cast from RE::Magictarget
        if (!target) return;

        // av cap handles
        
        if (baseEff->HasKeyword(my::regulator_eff)) return;
        if (baseEff->data.primaryAV == RE::ActorValue::kSpeedMult || baseEff->data.secondaryAV == RE::ActorValue::kSpeedMult)
        {  
            u_update_speedMult(target);                // update pc / npc ms av instead req_script
            if (!target->IsPlayerRef()) return;        // dont handle ms cap for npc
            handle_capValues_onAdjustEff(target, RE::ActorValue::kSpeedMult, 30.0f, 150.0f, my::speedCap_regulator, eff);
            u_update_speedMult(target);  
        }
        else if (baseEff->data.primaryAV == RE::ActorValue::kAbsorbChance || baseEff->data.secondaryAV == RE::ActorValue::kAbsorbChance) {
            handle_capValues_onAdjustEff(target, RE::ActorValue::kAbsorbChance, -400.0f, 75.0f, my::absorbCap_regulator, eff);
        }
        else if (baseEff->data.primaryAV == RE::ActorValue::kShoutRecoveryMult || baseEff->data.secondaryAV == RE::ActorValue::kShoutRecoveryMult) {
            handle_capValues_onAdjustEff(target, RE::ActorValue::kShoutRecoveryMult, 0.2f, 1000.0f, my::shoutCap_regulator, eff);
        }

        //SKSE::log::info("on_adjust_active_effect - target is - {}, caster is - {}", eff->GetTargetActor()->GetName(), eff->GetCasterActor()->GetName());
        if (baseEff->HasKeyword(my::alch_heal_KW))
        {
              if (mys::player->HasPerk(my::paladinCenter))    {    
                if (mys::player->HasPerk(my::palCenterBoost))
                     eff->magnitude *= 1.2f;
                else
                     eff->magnitude *= 1.15f;
              }
              if (mys::player->HasPerk(my::chihMeridiaShield))
              {
                 float mult = 1 + (mys::player->GetActorValue(RE::ActorValue::kRestoration) * 0.0015f);
                 eff->magnitude *= mult;
              }
        }

        if (baseEff->GetArchetype() == RE::EffectSetting::Archetype::kSlowTime)    {
              if (target->HasPerk(my::bossFightDebuff) && !baseEff->HasKeyword(my::slowTimeExclude)) {
                 eff->magnitude  = 0.82f;
                 eff->duration  *= 0.5f;
              }
        }

        if (baseEff->HasKeyword(my::dll_check_KW))
        {
              if (baseEff->HasKeyword(my::bindKeyKw))
              {            
                 my::instantAbIndex = eff->magnitude;
                 if (my::instantAbIndex == 0) {
                     u_MessageBox::MyMessageBox::Show(my::msgBoxDodge, &msgBoxChoiceCallback);  //   show MessageBox to choose Dodge key
                 }
                 else {
                     u_MessageBox::MyMessageBox::Show(my::msgBoxAbils, &msgBoxChoiceCallback);  //   show MessageBox to choose Ability key
                 }
              }
              if (baseEff->HasKeyword(my::snowElfEnchKW)) snowElf_applyChant();                // snowElf chant        
              if (baseEff->HasKeyword(my::sf_effect_KW))  my::sf_handle(eff, baseEff);         // sf    
              //if (baseEff->HasKeyword(my::oil_keyword)) handle_oil(target, baseEff, true);   // oil 

        }// dll check effects

    return;
}


void on_valueMod_Effect_Finish (RE::ValueModifierEffect* modEff)
{
    if (!modEff || !modEff->effect || !modEff->effect->baseEffect) return;

    if (modEff->actorValue == RE::ActorValue::kSpeedMult)  // && modEff->effect->baseEffect->GetArchetype() == RE::EffectSetting::Archetype::kPeakValueModifier
    {
       RE::Actor* target = skyrim_cast<RE::Actor*>(modEff->target);
       if (target) u_update_speedMult(target);                                // instead of REQ_Speed_Change
    }
}


void _set_proj_collision_layer (RE::Projectile* proj, RE::Actor *reflector) 
{
    LOG("called  _set_proj_collision_layer()");
    auto shape = (RE::bhkShapePhantom*)proj->unk0E0;
    auto ref = (RE::hkpShapePhantom*)shape->referencedObject.get();
    auto& colFilterInfo = ref->collidable.broadPhaseHandle.collisionFilterInfo;

    uint32_t fInfo;
    reflector->GetCollisionFilterInfo(fInfo);
    colFilterInfo &= (0x0000FFFF);
    colFilterInfo |= (fInfo << 16);
}


bool on_arrow_collide (RE::ArrowProjectile* arrow, RE::hkpAllCdPointCollector* collidable)
{
    LOG("called on_arrow_collide()");
    if (collidable && arrow && arrow->shooter) {
          auto shooter = arrow->shooter.get().get()->As<RE::Actor>();
          if (shooter && (shooter->IsPlayerRef() || shooter->HasMagicEffect(my::arrow_reflect)))
          {
              for (auto& hit : collidable->hits) {
                     auto refrA = RE::TESHavokUtilities::FindCollidableRef(*hit.rootCollidableA);
                     auto refrB = RE::TESHavokUtilities::FindCollidableRef(*hit.rootCollidableB);
                     if (refrA && refrA->formType == RE::FormType::ActorCharacter) {
                        // seems refrA not using, projectile collide target is always refrB
                     }
                     if (refrB && refrB->formType == RE::FormType::ActorCharacter)
                     {
                         if (!refrB->IsPlayerRef())            // when collides target (non-player) 
                         {
                            auto target = refrB->As<RE::Actor>();
                            if (!target || !target->HasMagicEffect(my::arrow_reflect)) return true;    
                            _set_proj_collision_layer(arrow, target);  
                            arrow->SetActorCause(target->GetActorCause());
                            arrow->shooter = refrB->GetHandle();
                            arrow->linearVelocity *= -1;            // change arrow's owner and turn speed to -1
                            my::reflectedArrow = arrow;
                            LOG("arrow_collide 1");
                            auto vec = u_get_effects_by_keyword(target, my::arrowReflectKW);
                            if (!vec.empty()) {
                                for (auto eff : vec) eff->Dispel(true);        // dispel arrow reflect eff
                            }
                            LOG("arrow_collide 2");
                            return false;
                         }
                     }
              }
          }
    }
    return true;
}

bool on_meleeWeap_collide (RE::Actor* attacker, RE::Actor* victim)
{
     if (attacker->IsPlayerRef() && mys::attackKeyHolding && !u_is_power_attacking(attacker))    // [pwatk double-hit fix] 
     {
          //RE::DebugNotification("not allow this attack");
          return false;
     }
     //RE::DebugNotification("allow this attack");
     return true;
}

void handle_cap (RE::Actor* actor, RE::ActorValue av, float min_cap, float max_cap, RE::SpellItem *regulatorSpell)
{

    LOG("called handle_cap()");
    float regulator_magn = regulatorSpell->effects[0]->effectItem.magnitude;
    float av_value = actor->GetActorValue(av);

    if (av_value < max_cap && regulator_magn > 0)        // if became less than cap, and have reserved regulator from past, give it back to av
    {
          float diff = (max_cap - av_value);
          if (regulator_magn >= diff) regulator_magn -= diff;
          else regulator_magn = 0;
          regulatorSpell->effects[0]->effectItem.magnitude = regulator_magn;
    }
    else if (av_value > min_cap && regulator_magn < 0)
    {
          float diff = (av_value - min_cap);
          if (abs(regulator_magn) >= diff) regulator_magn += diff;
          else regulator_magn = 0;
          regulatorSpell->effects[0]->effectItem.magnitude = regulator_magn;
    }
    else if (av_value > max_cap)
    {
          float diff = (av_value - max_cap);
          regulatorSpell->effects[0]->effectItem.magnitude += diff;
    }
    else return;    // if similar case, don't re-give regulator spell (below)

    actor->RemoveSpell(regulatorSpell);
    actor->AddSpell(regulatorSpell);
}


void handle_cap_actorValues (RE::Actor *actor)
{

    handle_cap(actor, RE::ActorValue::kSpeedMult, 30.0f, 150.0f, my::speedCap_regulator);              // speedMult
    u_update_speedMult(actor);                                                             
    handle_cap(actor, RE::ActorValue::kShoutRecoveryMult, 0.2f, 1000.0f, my::shoutCap_regulator);     // shout recovery  (note than shout buff effects are not detrimental but makes minus magn)

    if (actor->HasSpell(my::doomAtronach))                                                              // atronach (alt) absorb
    {
          float absorbChance = (actor->GetActorValue(RE::ActorValue::kAbsorbChance) + 140);
          if (absorbChance > 75) absorbChance = 75;
          my::atronachAbsorbChanceGlob->value = absorbChance;    
    }
    else {
        handle_cap(actor, RE::ActorValue::kAbsorbChance, -400.0f, 75.0f, my::absorbCap_regulator);       // absorb
    }

}


void update_mass_effects (RE::Actor* actor, float total_eq_weight = 0, bool aboutToEquipHeavyArmor = false)
{
    LOG("called update_mass_effects()");
    if (total_eq_weight == 0) total_eq_weight = u_get_worn_equip_weight(actor);   // if called with default value
    float perks_speedFactor = 1.f, perks_InfamyFactor = 1.f, perks_noiseFactor = 1.f;

    if (mys::hasHeavyArmor || aboutToEquipHeavyArmor)  // heavy armor 
    {
        if (actor->HasPerk(my::heavy_1_perk))  {perks_speedFactor -= 0.4f;  perks_InfamyFactor *= 0.95f;}
		if (actor->HasPerk(my::heavy_1_perk2)) {perks_speedFactor -= 0.2f; }
        if (actor->HasPerk(my::heavy_25_perk)) {perks_speedFactor -= 0.09f; perks_InfamyFactor *= 0.85f;}
        if (actor->HasPerk(my::heavy_sprint_perk)) {perks_speedFactor -= 0.06f; perks_InfamyFactor *= 0.8f;}
        if (actor->HasPerk(my::heavy_50_perk)) {perks_speedFactor -= 0.05f; perks_InfamyFactor *= 0.7f;}
        if (actor->HasPerk(my::heavy_75_perk)) {perks_speedFactor -= 0.05f; perks_InfamyFactor *= 0.8f;}
        if (actor->HasPerk(my::heavy_100_perk)){/*speedFactor not affect*/  perks_InfamyFactor *= 0.7f; }    
    }
    else {
        perks_speedFactor = (actor->HasPerk(my::evasion_1_perk))  ? 0.25f : 0.5f;        // light
        perks_InfamyFactor = (actor->HasPerk(my::evasion_1_perk)) ? 0.5f  : 1.0f;
        perks_noiseFactor = (actor->HasPerk(my::evasion_1_perk))  ? 0.5f  : 1.0f;
    }
    my::mass_penalty_speed->effects[0]->effectItem.magnitude = total_eq_weight * perks_speedFactor;
    my::mass_penalty_noise->effects[0]->effectItem.magnitude = total_eq_weight * 0.01f * perks_noiseFactor;  
    my::mass_buff_mass->effects[0]->effectItem.magnitude = total_eq_weight * 0.01f;
    my::mass_buff_infamy->effects[0]->effectItem.magnitude = total_eq_weight * 0.01f * perks_InfamyFactor;
    actor->RemoveSpell(my::mass_buff_mass);
    actor->RemoveSpell(my::mass_buff_infamy);
    actor->RemoveSpell(my::mass_penalty_noise);
    actor->RemoveSpell(my::mass_penalty_speed);
    actor->AddSpell(my::mass_penalty_speed);
    actor->AddSpell(my::mass_buff_mass);
    actor->AddSpell(my::mass_buff_infamy);
    actor->AddSpell(my::mass_penalty_noise);
    u_update_speedMult(actor);
    // auto total_inv_weight = mys::player->GetActorValue(RE::ActorValue::kInventoryWeight);   //  for total inventory weight

}

inline void update_perkArmorBuffs (RE::Actor* actor)  //  to apply perk % armor bonuses for npc
{
    actor->ModActorValue(RE::ActorValue::kDamageResist, -1.0f);
    actor->ModActorValue(RE::ActorValue::kDamageResist, 1.0f);
}

void handle_bossAppearance(RE::Actor *boss)
{
    my::currentBoss = boss;
    if      (boss->HasSpell(my::olve_training))  my::bossFightID->value = 1;
	else if (boss->HasSpell(my::seph_training))  my::bossFightID->value = 2;
	else if (boss->HasSpell(my::seph_training2)) my::bossFightID->value = 2;

}

void on_equip (RE::Actor* actor, RE::TESBoundObject* object) 
{
    if (!actor || !object) return;
    LOG("called on_equip() for - {}", object->GetName());
    if (actor->HasMagicEffect(my::bossFightStarter)) handle_bossAppearance(actor);
    if (!actor->IsPlayerRef()) { update_perkArmorBuffs(actor); return; }  
    if (!object->IsWeapon() && !object->IsArmor()) return;
    float worn_weight = u_get_worn_equip_weight(mys::player);                //  at this momemt this item is not equipped yet, and its weight won't be taken into account
    float this_item_weight = object->GetWeight();
    float total_eq_weight = worn_weight + this_item_weight;                    //  add new item weight to current weight
    bool aboutToEqHeavyArmor = (object->IsArmor() && object->As<RE::TESObjectARMO>()->HasKeyword(mys::armorHeavyKw)) ? 1 : 0;
    update_mass_effects(actor, total_eq_weight, aboutToEqHeavyArmor);

    // equip_manager->EquipObject(actor, bound_object, nullptr, 1, left_hand_equip);           //  force to equip some item

}

void on_unequip (RE::Actor* actor, RE::TESBoundObject* object)
{
    LOG("called on_unequip()");
    if (!actor || !object) return;
    if (!actor->IsPlayerRef()) return;
    if (!object->IsWeapon() && !object->IsArmor()) return;

    float worn_weight = u_get_worn_equip_weight(mys::player);
    float this_item_weight = object->GetWeight();
    float total_eq_weight = worn_weight - this_item_weight;
    update_mass_effects(actor, total_eq_weight);
}



std::string simpleHash (const std::string& str)
{
    std::string hashed = str;
    for (char& c : hashed) c += 1;        // simple "shift" (adding one character)
    return hashed;
}


void log_game_info (RE::Actor* pl, bool death = false, RE::Actor* killer = nullptr) 
{
    LOG("called log_game_info()");
    using string = std::string;

    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);  // path to app folder, in our case RfaD SE/

    string dllPath = path;
    size_t lastSlashPos = dllPath.rfind('\\');
    string folderPath = dllPath.substr(0, lastSlashPos + 1);
    string filePath = folderPath + "\\Logs\\log.dat";  // RfaD SE / Logs

    string info = "";
    if (death) info = "Death: ";
    else       info = "Info: ";

    info += "Character - " + string(pl->GetName()) + "; ";

    if (death)    {
        if (killer) {
            info += "Killer name - " + string(killer->GetName()) + "; " + "Killer ID - " + u_int2hex(killer->formID) + "; ";
        }
    }
    if (pl->GetCurrentLocation()) {
        info += string("Location - ") + string(pl->GetCurrentLocation()->fullName) + "; ";
    }
    else info += "Location - unknown; ";
    info += string("Level - ") + std::to_string(pl->GetLevel()) + "; ";
    int req_incoming_damage = int(RE::GameSettingCollection::GetSingleton()->GetSetting("fDiffMultHPToPCL")->GetFloat()*100); 
    int req_dealt_damage    = int(RE::GameSettingCollection::GetSingleton()->GetSetting("fDiffMultHPByPCL")->GetFloat()*100);
    info += string("Requiem - ") + std::to_string(req_dealt_damage) + "/" + std::to_string(req_incoming_damage) + "; ";

    if (!death) // info
    {
        string baseMP = std::to_string(mys::player->GetBaseActorValue(RE::ActorValue::kMagicka));
        string baseHP = std::to_string(mys::player->GetBaseActorValue(RE::ActorValue::kHealth));
        string baseST = std::to_string(mys::player->GetBaseActorValue(RE::ActorValue::kStamina));
        info += string("Base MP/HP/ST - ") + baseMP + "/" + baseHP + "/" + baseST + "; ";
        info += string("MaxHP - ") + std::to_string(u_get_actor_value_max(pl, RE::ActorValue::kHealth)) + "; ";
        info += string("CarryWeight - ") + std::to_string(pl->GetActorValue(RE::ActorValue::kCarryWeight)) + "; ";
        info += string("Gold - ") + std::to_string(u_get_item_count(pl, 0xf)) + "; ";
        info += u_get_entered_console_commands();
    }
    
    char time_buf[21];
    time_t now;
    time(&now);
    strftime(time_buf, 21, "%Y-%m-%dT%H:%M:%S", gmtime(&now));  // current time string
    info += string("Log Time - ") + string(time_buf) + "(+3 hour for Moscow);#";

    std::ofstream ofs (filePath.c_str(), std::ios::app | std::ios::binary);  // Открываем файл в двоичном режиме для дозаписи (ios::app)
    if (ofs.is_open())
    {
        string hashedStr = simpleHash(info);

        size_t len = hashedStr.size();
        ofs.write((char*)&len, sizeof(len));
        ofs.write((char*)&hashedStr[0], len);    // binary write
        ofs.close();
    }
    else LOG ("log_player_info - ofstream file not open");
}

void log_player_death (RE::Actor* pl, RE::Actor* killer)
{
    log_game_info (pl, true, killer);
}


void on_death (RE::Actor* victim, RE::Actor* killer)  // event onDeath, called after death
{
    if (!victim) return;
    qst::check_onDeath_questGoals(victim);
    if (victim->IsPlayerRef()) {
        log_player_death(victim, killer);
    }
    if (victim->HasMagicEffect(my::bossFightStarter)) {
        my::bossFightID->value = 0;
    }
} 

void my::on_wait()
{
    LOG("called on_wait()");
    auto poisonData = u_get_pc_poison();
    if (poisonData && poisonData->count > 1) {
        poisonData->count = 0;
        u_remove_pc_poison();
    }
}

void on_wait_menu_open()       // event
{
    my::waitStartGameHour = RE::TESForm::LookupByID<RE::TESGlobal>(0x38)->value;
}

void on_wait_menu_close()      // event
{
    auto gameHour = RE::TESForm::LookupByID<RE::TESGlobal>(0x38)->value;
	if ((gameHour - my::waitStartGameHour) > 0.9f)
	{
        my::on_wait();
	}
}

void on_inventory_open()   // event
{
    //...
}

void on_inventory_close()  // event
{
	//...
}

inline void update_oil (RE::Actor *pl)
{
    auto poisonData = u_get_pc_poison();
    if (poisonData) {
         if (poisonData->count > 0) {
            if (pl->IsWeaponDrawn())
               poisonData->count -= 1;          // decrement oil every update 
         } 
	     else u_remove_pc_poison();  // oil ends
    }
}

inline void update_gamelog ()
{
	if (mys::gameProcessed->value > 0) {
          my::log_counter++;
          if (my::log_counter > 250) {    // +- 15min
                 my::log_counter = 0;
                 log_game_info(mys::player);
          }
    }
}

inline void update_keycodes ()
{
    for (int i = 0; i < bindGlobs.size(); i++) {
          keyCodes.at(i) = bindGlobs.at(i)->value;
    }
    my::rightHandKeyCode = RE::ControlMap::GetSingleton()->GetMappedKey("Right Attack/Block"sv, RE::INPUT_DEVICE::kMouse) + Utils::kMacro_NumKeyboardKeys;
    // if device = mouse it returns keyMask (as in InputWatcher), i have to add 256 (NumKeyboardKeys) to it, but still can bind RHand on keyBoard, so how to get device? (todo)
}



void on_my_update()
{
    float baseMana = mys::player->GetBaseActorValue(RE::ActorValue::kMagicka);
    mys::player->SetActorValue(RE::ActorValue::kMood, baseMana);

    if (u_worn_has_keyword(mys::player, mys::armorHeavyKw))  mys::hasHeavyArmor = true;
    else  mys::hasHeavyArmor = false;
    apply_levelUp_bonuses();
    handle_cap_actorValues(mys::player);
    update_mass_effects(mys::player);
    handle_moveSpeed_compensation();
    handle_weaponBased_attack_speed();
    mys::player->WornArmorChanged();  // for update armor % perk entries
    mys::attackKeyHolding = false;
    mys::xdescr_state = 0;
    
    if (mys::gameProcessed->value > 0)  // after player selected start
    {
        qst::check_class_achievements(mys::player);
        if (my::bossFightID->value > 0) {
             gameplay::bossFightHandle(my::currentBoss, my::bossFightID->value, mys::bossUpdMeter);
        }
    }

    if (my::twicedUpdate)  // every 2nd update
    {
        update_gamelog();
        update_keycodes();
        update_oil (mys::player);
        my::vamp_state_glob->value = 0;
    }
    my::twicedUpdate = !my::twicedUpdate;

    if (mys::player->HasSpell(my::snowElf_raceAb)) {
        auto ench = u_get_actors_weap_ench(mys::player, 0);
        if (ench && ench->formID == my::snowElfEnch->formID)
            my::snowElf_wears_EnchWeap->value = 1;
        else
            my::snowElf_wears_EnchWeap->value = 0;
    }
    // u_log_actor_perk_entries(mys::player, RE::BGSPerkEntry::EntryPoint::kModIncomingDamage, "ModIncomingDamage");
    // u_log_actor_perk_entries(mys::player, RE::BGSPerkEntry::EntryPoint::kModAttackDamage, "ModAttackDamage");
    // u_log_actor_perk_entries(mys::player, RE::BGSPerkEntry::EntryPoint::kModPowerAttackDamage,
    // "ModPowerAttackDamage"); u_log_actor_perk_entries(mys::player,
    // RE::BGSPerkEntry::EntryPoint::kModTargetDamageResistance, "ModTargetDamageResistance");
}


//void on_micro_update()   // every 1 sec
//{ 
//	 gameplay::gmpl_on_micro_update(); 
//}


//--------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------ fill most data on game loads (mainMenu) ---------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------
namespace my
{

    void fill_gamePointers()
    { 
     
        vamp_state_glob = handler->LookupForm<RE::TESGlobal>(0xB960D3, "RfaD SSE - Awaken.esp");
        snowElf_wears_EnchWeap = handler->LookupForm<RE::TESGlobal>(0xC2E0F0, "RfaD SSE - Awaken.esp");
        nb_hitCounter = handler->LookupForm<RE::TESGlobal>(0xC9D865, "RfaD SSE - Awaken.esp");
        nb_1_atkChoice = handler->LookupForm<RE::TESGlobal>(0xCDF64F, "RfaD SSE - Awaken.esp");
        nb_magicBlade = handler->LookupForm<RE::TESGlobal>(0xDF100A, "RfaD SSE - Awaken.esp");
        atronachAbsorbChanceGlob = handler->LookupForm<RE::TESGlobal>(0x87A67A, "RfaD SSE - Awaken.esp");
        bossFightID = handler->LookupForm<RE::TESGlobal>(0x1C7FFB, "RfaD SSE - Awaken.esp");
        redguardFF = handler->LookupForm<RE::SpellItem>(0x284194, "RfaD SSE - Awaken.esp");
        argonianFF = handler->LookupForm<RE::SpellItem>(0x3A04AC, "RfaD SSE - Awaken.esp");
        khajeetFF  = handler->LookupForm<RE::SpellItem>(0xEB1866, "RfaD SSE - Awaken.esp");
        bloodBrand = handler->LookupForm<RE::SpellItem>(0x238167, "RfaD SSE - Awaken.esp");
        bloodBoil_HP_Buff = handler->LookupForm<RE::SpellItem>(0xB72963, "RfaD SSE - Awaken.esp");
        bloodBubbles_FX = handler->LookupForm<RE::TESEffectShader>(0xBAE83, "Apocalypse - Magic of Skyrim.esp");
        bloodFire_FX    = handler->LookupForm<RE::TESEffectShader>(0xBFAC0, "Apocalypse - Magic of Skyrim.esp");
        bleedShader_FX  = handler->LookupForm<RE::TESEffectShader>(0x9FC16, "Apocalypse - Magic of Skyrim.esp");
        nb_2_Twin_FF = handler->LookupForm<RE::SpellItem>(0xD72531, "RfaD SSE - Awaken.esp");
        nb_main_holdFF = handler->LookupForm<RE::SpellItem>(0xDD298F, "RfaD SSE - Awaken.esp");
        snowElf_insteadEnch = handler->LookupForm<RE::SpellItem>(0xC2E0EE, "RfaD SSE - Awaken.esp");
        snowElf_anabioz = handler->LookupForm<RE::BGSPerk>(0x800C10, "RfaD SSE - Awaken.esp");
        snowElf_raceAb = handler->LookupForm<RE::SpellItem>(0x7D3169, "RfaD SSE - Awaken.esp");
        nb_sting = handler->LookupForm<RE::SpellItem>(0xDAF204, "RfaD SSE - Awaken.esp");
        nb_sting_hold = handler->LookupForm<RE::SpellItem>(0xDB942B, "RfaD SSE - Awaken.esp");
        nb_Reflect = handler->LookupForm<RE::SpellItem>(0xCA7A92, "RfaD SSE - Awaken.esp");
        nb_mainInvisAb = handler->LookupForm<RE::SpellItem>(0xC9D866, "RfaD SSE - Awaken.esp");
        nb_mainEff = handler->LookupForm<RE::EffectSetting>(0xCA2977, "RfaD SSE - Awaken.esp");
        nb_hold_slowTime = handler->LookupForm<RE::EffectSetting>(0xDD298E, "RfaD SSE - Awaken.esp");
        nb_openMode = handler->LookupForm<RE::EffectSetting>(0xCA297D, "RfaD SSE - Awaken.esp");
        nb_mirror_kd = handler->LookupForm<RE::EffectSetting>(0xDF100B, "RfaD SSE - Awaken.esp");
        nb_sting_kd  = handler->LookupForm<RE::EffectSetting>(0xEA2537, "RfaD SSE - Awaken.esp");
        nb_twin_kd   = handler->LookupForm<RE::EffectSetting>(0xEA2538, "RfaD SSE - Awaken.esp");
        nb_main_kd = handler->LookupForm<RE::EffectSetting>(0xDD2991, "RfaD SSE - Awaken.esp");
        bindsInfo_eff = handler->LookupForm<RE::EffectSetting>(0xE5143A, "RfaD SSE - Awaken.esp");
        snowElfEnch = handler->LookupForm<RE::EnchantmentItem>(0x7FBAE8, "RfaD SSE - Awaken.esp");
    
	    actorBoss      = handler->LookupForm<RE::BGSKeyword>(0x1C7FF6, "RfaD SSE - Awaken.esp");
        dodgeEffectKW  = handler->LookupForm<RE::BGSKeyword>(0x7B4A92, "RfaD SSE - Awaken.esp");
        arrowReflectKW = handler->LookupForm<RE::BGSKeyword>(0xECADB8, "RfaD SSE - Awaken.esp");
        bindKeyKw = handler->LookupForm<RE::BGSKeyword>(0xE2DCC6, "RfaD SSE - Awaken.esp");
        manaShield_2 = handler->LookupForm<RE::EffectSetting>(0x535A3E, "RfaD SSE - Awaken.esp");
        nb_fullReflect = handler->LookupForm<RE::EffectSetting>(0xCA7A98, "RfaD SSE - Awaken.esp");
        manaShield_4_sf = handler->LookupForm<RE::EffectSetting>(0xAE9C9C, "RfaD SSE - Awaken.esp");
        injureEff      = handler->LookupForm<RE::EffectSetting>(0x9F9FF2, "RfaD SSE - Awaken.esp");
        arrow_reflect = handler->LookupForm<RE::EffectSetting>(0xBB4732, "RfaD SSE - Awaken.esp");
        adrenalineKD    = handler->LookupForm<RE::EffectSetting>(0x897552, "RfaD SSE - Awaken.esp");
        adrenaline_tap  = handler->LookupForm<RE::SpellItem>(0xA5A416, "RfaD SSE - Awaken.esp");
        adrenaline_hold = handler->LookupForm<RE::SpellItem>(0xBB9846, "RfaD SSE - Awaken.esp");
        bloodRiversAB   = handler->LookupForm<RE::SpellItem>(0xA1A0A2, "RfaD SSE - Awaken.esp");
        bloodThirstAB = handler->LookupForm<RE::SpellItem>(0xB4A0CB, "RfaD SSE - Awaken.esp");
        bloodThirstHoldAB = handler->LookupForm<RE::SpellItem>(0xB4F1E0, "RfaD SSE - Awaken.esp");
        bloodThirst_kd = handler->LookupForm<RE::EffectSetting>(0xB4F1E3, "RfaD SSE - Awaken.esp");
        bloodRivers_kd = handler->LookupForm<RE::EffectSetting>(0xA38735, "RfaD SSE - Awaken.esp");
        bloodRivers_allow = handler->LookupForm<RE::EffectSetting>(0xA242D5, "RfaD SSE - Awaken.esp");
        injureSpell  = handler->LookupForm<RE::SpellItem>(0x3C3C2C, "RfaD SSE - Awaken.esp");            // injure_dmg_spell
        breakableBowKW = handler->LookupForm<RE::BGSKeyword>(0x2E232E, "Requiem.esp");
        slowTimeExclude = handler->LookupForm<RE::BGSKeyword>(0x250D70, "RfaD SSE - Awaken.esp");
        lvlup_bonuses_KW = handler->LookupForm<RE::BGSKeyword>(0x3833C5, "devFixes.esp");
        breakBowSpl = handler->LookupForm<RE::SpellItem>(0x7637F8, "RfaD SSE - Awaken.esp");
        stressSpell = handler->LookupForm<RE::SpellItem>(0xA5E9F, "devFixes.esp");      // attack stress FF
        stressStart = handler->LookupForm<RE::SpellItem>(0xBF603, "devFixes.esp");      // bow stress
        stressDispel = handler->LookupForm<RE::SpellItem>(0xBF607, "devFixes.esp");     // stress dispel
        zoomStress   = handler->LookupForm<RE::SpellItem>(0x64C2C9, "devFixes.esp");    // zoom stress
        atkSpeedDebuff = handler->LookupForm<RE::SpellItem>(0x2AD599, "devFixes.esp");  // player attack speed handle
        lvlup_hp_bonuses = handler->LookupForm<RE::SpellItem>(0xFB49,  "devFixes.esp"); //
        lvlup_st_bonuses = handler->LookupForm<RE::SpellItem>(0x388540,"devFixes.esp"); //    
        lvlup_mp_bonuses = handler->LookupForm<RE::SpellItem>(0xFB3F,  "devFixes.esp"); //    lvlup bonuses
        gm_lvlup_bonuses = handler->LookupForm<RE::BGSPerk>(0x4899DC,  "devFixes.esp"); //
        bash_kd_self = handler->LookupForm<RE::SpellItem>(0x3E3FDD, "devFixes.esp"); 
        block_anim_blocker = handler->LookupForm<RE::SpellItem>(0x3E3FDA, "devFixes.esp");
        become_bats = handler->LookupForm<RE::SpellItem>(0x19FD4, "Requiem for a Dream - Kelpie.esp");
        batForm_eff = handler->LookupForm<RE::EffectSetting>(0x5949, "Requiem for a Dream - Kelpie.esp");

		olve_training  = handler->LookupForm<RE::SpellItem>(0x3E17F4,"RfaD SSE - Awaken.esp");
        seph_training  = handler->LookupForm<RE::SpellItem>(0x98877, "RfaD SSE - Awaken.esp");
        seph_training2 = handler->LookupForm<RE::SpellItem>(0x4CECF, "refrain_DeadLands.esp");

		oil_after_use = handler->LookupForm<RE::SpellItem>(0x23C924, "RfaD SSE - Awaken.esp");
       
		race_ab_kd = handler->LookupForm<RE::EffectSetting>(0x284198, "RfaD SSE - Awaken.esp");
        bats_kd    = handler->LookupForm<RE::EffectSetting>(0x7DBDF8, "RfaD SSE - Awaken.esp");
        msgBoxDodge = handler->LookupForm<RE::BGSMessage>(0xB542F4, "RfaD SSE - Awaken.esp");
        msgBoxAbils = handler->LookupForm<RE::BGSMessage>(0xE83EDB, "RfaD SSE - Awaken.esp");
        vampirism     = RE::TESForm::LookupByID<RE::SpellItem>(0xED0A8);   //
        actorDwarven = RE::TESForm::LookupByID<RE::BGSKeyword>(0x1397A);  //
        alch_heal_KW = RE::TESForm::LookupByID<RE::BGSKeyword>(0x42503);  //  
        magicSlow_KW = RE::TESForm::LookupByID<RE::BGSKeyword>(0xB729E);  //  
        sword_1h   = RE::TESForm::LookupByID<RE::BGSKeyword>(0x1E711);  //  
        doomSteed    = RE::TESForm::LookupByID<RE::SpellItem>(0xE5F5E);   //        
        doomAtronach = RE::TESForm::LookupByID<RE::SpellItem>(0xE5F51);   //
        swordFocus1  = RE::TESForm::LookupByID<RE::BGSPerk>(0x5F56F);      //    
        swordFocus2  = RE::TESForm::LookupByID<RE::BGSPerk>(0xC1E90);      //            } (from skyrim.esm) 
        swordFocus3  = RE::TESForm::LookupByID<RE::BGSPerk>(0xC1E91);      //
        bowSlowTimePerk = RE::TESForm::LookupByID<RE::BGSPerk>(0x103ADA); //
        evasion_1_perk = RE::TESForm::LookupByID<RE::BGSPerk>(0xBE123);   //
        heavy_1_perk =  RE::TESForm::LookupByID<RE::BGSPerk>(0xBCD2A);    //
        heavy_25_perk = RE::TESForm::LookupByID<RE::BGSPerk>(0x58F6F);    //
        heavy_50_perk = RE::TESForm::LookupByID<RE::BGSPerk>(0x58F6C);    //
        heavy_75_perk = RE::TESForm::LookupByID<RE::BGSPerk>(0x107832);   //
        heavy_100_perk = RE::TESForm::LookupByID<RE::BGSPerk>(0x105F33);   //
        heavy_sprint_perk = RE::TESForm::LookupByID<RE::BGSPerk>(0x7935E); //
        nb_perk_1 = RE::TESForm::LookupByID<RE::BGSPerk>(0x153D0);         //
        nb_perk_2 = RE::TESForm::LookupByID<RE::BGSPerk>(0x59B78);         //
        heavy_1_perk2 = handler->LookupForm<RE::BGSPerk>(0x9A8D38, "devFixes.esp");
        
        slashSound   = RE::TESForm::LookupByID<RE::BGSSoundDescriptorForm>(0xDAB81);      //
        vampireHeartBeat = RE::TESForm::LookupByID<RE::BGSSoundDescriptorForm>(0xFF9E8);  //
        slash_1 = handler->LookupForm<RE::BGSKeyword>(0xAD3965, "Requiem.esp");
        slash_2 = handler->LookupForm<RE::BGSKeyword>(0xAD3964, "Requiem.esp");
        slash_3 = handler->LookupForm<RE::BGSKeyword>(0xAD3963, "Requiem.esp");
        slash_4 = handler->LookupForm<RE::BGSKeyword>(0xAD3962, "Requiem.esp");
        slash_5 = handler->LookupForm<RE::BGSKeyword>(0xAD3A24, "Requiem.esp");
        bossFightDebuff = handler->LookupForm<RE::BGSPerk>(0xF67E9F, "RfaD SSE - Awaken.esp");
        dll_check_KW = handler->LookupForm<RE::BGSKeyword>(0x9D823A,"RfaD SSE - Awaken.esp");
        sf_effect_KW = handler->LookupForm<RE::BGSKeyword>(0x13B7BE, magick_esp);
        sf_dispel_KW = handler->LookupForm<RE::BGSKeyword>(0x14093E, magick_esp);
        sf_perk_3 = handler->LookupForm<RE::BGSPerk>(0x2C8B53, magick_esp);
        palCenterBoost = handler->LookupForm<RE::BGSPerk>(0x80F, "devFixes.esp");
        paladinCenter = handler->LookupForm<RE::BGSPerk>(0xFB4F, "Requiem for a Dream - DivineBlessings.esp");

        chihMeridiaShield = handler->LookupForm<RE::BGSPerk>(0x131535, "ChihSkillTree.esp");
        vamp_left2_perk = handler->LookupForm<RE::BGSPerk>(0x54E96, "ChihSkillTree.esp");
        vamp_1st_perk   = handler->LookupForm<RE::BGSPerk>(0x59FDA, "ChihSkillTree.esp");
        
        cult_vermina_1 = handler->LookupForm<RE::BGSPerk>(0xCED17, "ChihSkillTree.esp");
        cult_boethia_1 = handler->LookupForm<RE::BGSPerk>(0x54EB3, "ChihSkillTree.esp");    
        cult_meridia_1 = handler->LookupForm<RE::BGSPerk>(0x54E81, "ChihSkillTree.esp");
        cult_nocturn_1 = handler->LookupForm<RE::BGSPerk>(0x54E7E, "ChihSkillTree.esp");
        cult_azura_1   = handler->LookupForm<RE::BGSPerk>(0x54E7F, "ChihSkillTree.esp");               //  cultist perks
       
        boethiaFF = handler->LookupForm<RE::SpellItem>(0xD18C2,  "Ch1h_MeridiaPatch.esp");                // meridia_patch.esp
        verminaFF = handler->LookupForm<RE::SpellItem>(0xCED1A,  "ChihSkillTree.esp");
        nocturnalFF = handler->LookupForm<RE::SpellItem>(0x91BAD,"ChihSkillTree.esp");                    // skillTree.esp
        meridiaFF = handler->LookupForm<RE::SpellItem>(0x54EB8,  "ChihSkillTree.esp");
        meridia_kd = handler->LookupForm<RE::EffectSetting>(0x14ED3, "ChihSkillTree.esp");

		mercenaryQst = handler->LookupForm<RE::TESQuest>(0xF1CF, "RfaD SSE - Awaken.esp");

		bossFightStarter = handler->LookupForm<RE::EffectSetting>(0x1C7FF9, "RfaD SSE - Awaken.esp");
        oil_keyword = handler->LookupForm<RE::BGSKeyword>(0xCFAED,  "RfaD SSE - Awaken.esp");
        snowElfEnchKW = handler->LookupForm<RE::BGSKeyword>(0x99B51C, "RfaD SSE - Awaken.esp");
        allow_fx_kw = handler->LookupForm<RE::BGSKeyword>(0x664F36, "RfaD SSE - Awaken.esp");
        speedCap_regulator = handler->LookupForm<RE::SpellItem>(0x5B3671, "devFixes.esp");
        absorbCap_regulator = handler->LookupForm<RE::SpellItem>(0x5BD967, "devFixes.esp");
        shoutCap_regulator = handler->LookupForm<RE::SpellItem>(0x5BD96A, "devFixes.esp");
        regulator_eff = handler->LookupForm<RE::BGSKeyword>(0x5C2AE8, "devFixes.esp");
        

        mass_penalty_speed = handler->LookupForm<RE::SpellItem>(0x5A41F7, "devFixes.esp");
        mass_penalty_noise = handler->LookupForm<RE::SpellItem>(0x5A41FB, "devFixes.esp");
        mass_buff_mass = handler->LookupForm<RE::SpellItem>(0x5A41FE, "devFixes.esp");
        mass_buff_infamy = handler->LookupForm<RE::SpellItem>(0x5A4200, "devFixes.esp");

        sf_cloakEff_KW  = handler->LookupForm<RE::BGSKeyword>(0x1B5B7D, magick_esp);

        glob_destr_1 = handler->LookupForm<RE::TESGlobal>(0x13B7B8, magick_esp);
        glob_destr_2 = handler->LookupForm<RE::TESGlobal>(0x13B7B9, magick_esp);
        glob_destr_3 = handler->LookupForm<RE::TESGlobal>(0x13B7BA, magick_esp);
        glob_alter_1 = handler->LookupForm<RE::TESGlobal>(0x145ABF, magick_esp);
        glob_alter_2 = handler->LookupForm<RE::TESGlobal>(0x145AD3, magick_esp);
        glob_alter_3 = handler->LookupForm<RE::TESGlobal>(0x154F5B, magick_esp);

        sf_speed_const = handler->LookupForm<RE::SpellItem>(0x154F51, magick_esp);
        sf_penet_const = handler->LookupForm<RE::SpellItem>(0x154F53, magick_esp);
        sf_armor_const = handler->LookupForm<RE::SpellItem>(0x154F55, magick_esp);
        sf_rflct_const = handler->LookupForm<RE::SpellItem>(0x154F57, magick_esp);
        sf_absrb_const = handler->LookupForm<RE::SpellItem>(0x154F59, magick_esp);
        sf_stamina_const = handler->LookupForm<RE::SpellItem>(0x272234, magick_esp);
        sf_descr = handler->LookupForm<RE::EffectSetting>(0x2773B0, magick_esp);
    }


    void fill_data()
    {
        bindGlobs.emplace_back(handler->LookupForm<RE::TESGlobal>(0xE74BA4, "RfaD SSE - Awaken.esp"));  // [0] - 56 - "Dodge"       // lAlt
        bindGlobs.emplace_back(handler->LookupForm<RE::TESGlobal>(0xE74BA5, "RfaD SSE - Awaken.esp"));  // [1] - 47 - "Bats"        // V
        bindGlobs.emplace_back(handler->LookupForm<RE::TESGlobal>(0xE74BA6, "RfaD SSE - Awaken.esp"));  // [2] - 46 - "Boil"        // C
        bindGlobs.emplace_back(handler->LookupForm<RE::TESGlobal>(0xE74BA7, "RfaD SSE - Awaken.esp"));  // [3] - 45 - "Adrenaline"  // X
        bindGlobs.emplace_back(handler->LookupForm<RE::TESGlobal>(0xE74BA8, "RfaD SSE - Awaken.esp"));  // [4] - 45 - "Twilight"    // X
        bindGlobs.emplace_back(handler->LookupForm<RE::TESGlobal>(0xE74BA9, "RfaD SSE - Awaken.esp"));  // [5] - 20 - "Sting"       // T
        bindGlobs.emplace_back(handler->LookupForm<RE::TESGlobal>(0xE74BAA, "RfaD SSE - Awaken.esp"));  // [6] - 21 - "Mirror"      // Y
        bindGlobs.emplace_back(handler->LookupForm<RE::TESGlobal>(0xE74BAB, "RfaD SSE - Awaken.esp"));  // [7] - 22 - "Twin"        // U
        bindGlobs.emplace_back(handler->LookupForm<RE::TESGlobal>(0xE74BAC, "RfaD SSE - Awaken.esp"));  // [8] - 47 - "Cultist"     // V
        bindGlobs.emplace_back(handler->LookupForm<RE::TESGlobal>(0xE74BAD, "RfaD SSE - Awaken.esp"));  // [9] - 20 - "Race"        // T
        
        keyCodes = { 56, 47, 46, 45, 45, 20, 21, 22, 47, 20 };   // default binds of instant abilities

        sf_map.emplace(1292213, 1);
        sf_map.emplace(1292223, 2);
        sf_map.emplace(1292225, 3);  // sf_1 spells
        sf_map.emplace(1333949, 10);
        sf_map.emplace(2168340, 11);

        sf_map.emplace(1333952, 1);
        sf_map.emplace(1333954, 2);
        sf_map.emplace(1333956, 3);  // sf_2 spells
        sf_map.emplace(1333969, 10);
        sf_map.emplace(1354833, 11);

        sf_map.emplace(1396574, 1);
        sf_map.emplace(1396572, 2);
        sf_map.emplace(1396576, 3);  // sf_3 spells
        sf_map.emplace(1333972, 10);

        sf_map.emplace(1375693, 1);  // speed
        sf_map.emplace(1375695, 2);  // penetr
        sf_map.emplace(1375697, 3);  // armor            // sf_3 reserved mana buffs
        sf_map.emplace(1375699, 4);  // reflect
        sf_map.emplace(1375701, 5);  // absorb
        sf_map.emplace(2564656, 6);  // stamina

		reflectedArrow = nullptr;
        currentBoss = nullptr;
        currentBossAutocast = nullptr;
        twicedUpdate = false;
        instantAbIndex = 0;
        rightHandKeyCode = 256;        // initial value = mouse left
        log_counter = 230;
        waitStartGameHour = 0;
        x_desc::parseFile();        // read and save data from xDescriptions file
    }
    
    void fill_translation()            //  TRANSLATION  RU / EN
    {
        auto lang  = RE::GetINISetting("sLanguage:General");
        if (!lang) return;
        auto langS = std::string(lang->GetString());
        if (langS == "RUSSIAN" || langS == "russian" || langS == "Russian" || langS == "ru")  // ru 
        {
            sf_tier1_names.emplace(1, "Чары Мороза");
            sf_tier1_names.emplace(2, "Чары Огня");
            sf_tier1_names.emplace(3, "Чары Молний");
            sf_tier1_names.emplace(10, "Чары Телекинеза");
            sf_tier1_names.emplace(11, "Гнев Арканы");

            sf_tier2_names.emplace(1, "Снижение сопротивления морозу");
            sf_tier2_names.emplace(2, "Снижение сопротивления огню");
            sf_tier2_names.emplace(3, "Снижение сопротивления молнии");
            sf_tier2_names.emplace(10, "Прилив сил");
            sf_tier2_names.emplace(11, "Снижение физического урона");

            sf_tier3_names.emplace(1, "Волна Мороза");
            sf_tier3_names.emplace(2, "Волна Огня");
            sf_tier3_names.emplace(3, "Электрическая Волна");
            sf_tier3_names.emplace(10, "Волна Телекинеза");

            sf_speed_text = "Резерв - Скорость";
            sf_penetr_text = "Резерв - Сила";
            sf_armor_text = "Резерв - Броня";
            sf_reflect_text = "Резерв - Отражение";
            sf_absorb_text = "Резерв - Поглощение";
            sf_stamina_text = "Резерв - Выносливость";

            sf_rem_current = "Чары удалены: ";
            sf_add_new = "Чары добавлены: ";
            sf_removed = "Чары удалены";
            sf_all_clear = "Все эффекты боевого мага сброшены";
            sf_noEffects = "Нет Эффектов";

            adrenaline_text = "Выброс адреналина";
            adrenalineMax_text = "Максимальный адреналин";

            oil_decline_text = "На оружие уже нанесено масло";
        }
        else    // en
        {
            sf_tier1_names.emplace(1, "Frost Chant");
            sf_tier1_names.emplace(2, "Fire Chant");
            sf_tier1_names.emplace(3, "Shock Chant");
            sf_tier1_names.emplace(10, "Telecinetic Chant");
            sf_tier1_names.emplace(11, "Arcane Rage");

            sf_tier2_names.emplace(1, "Decrease Frost Resist");
            sf_tier2_names.emplace(2, "Decrease Fire Resist");
            sf_tier2_names.emplace(3, "Decrease Shock Resist");
            sf_tier2_names.emplace(10, "Burst of Strength");
            sf_tier2_names.emplace(11, "Decrease Physical Damage");

            sf_tier3_names.emplace(1, "Frost Wave");
            sf_tier3_names.emplace(2, "Fire Wave");
            sf_tier3_names.emplace(3, "Shock Wave");
            sf_tier3_names.emplace(10, "Telecinetic Wave");

            sf_speed_text = "Reserve - Speed";
            sf_penetr_text = "Reserve - Penetration";
            sf_armor_text = "Reserve - Armor";
            sf_reflect_text = "Reserve - Reflect";
            sf_absorb_text = "Reserve - Absorb";
            sf_stamina_text = "Reserve - Stamina";

            sf_rem_current = "Chant removed: ";
            sf_add_new = "Chant added: ";
            sf_removed = "Chant removed";
            sf_all_clear = "All spellfury effects removed";
            sf_noEffects = "No Effects";

            adrenaline_text = "Adrenaline rush";
            adrenalineMax_text = "Maximum adrenaline rush";

            oil_decline_text = "Oil has already been applied";
        }
    }
}




//___________ newrite onResistApply function _______

/*
auto check_resistance(
    RE::MagicTarget& this_,
    RE::MagicItem& magic_item,
    const RE::Effect& effect,
    const RE::TESBoundObject* bound_object,
    const Config& config) -> float
{
  logger::debug("Check resist"sv);
  if (magic_item.hostileCount <= 0 || bound_object && bound_object->formType == RE::FormType::Armor)
    {
      logger::debug("Non hostile"sv);
      return 1.f;
    }

  const auto alchemy_item = magic_item.As<RE::AlchemyItem>();

  if (alchemy_item && ((!alchemy_item->IsPoison() && !effect.IsHostile()) ||
                       (alchemy_item->GetSpellType() == RE::MagicSystem::SpellType::kIngredient &&
                        alchemy_item->IsFood())))
    {
      logger::debug("alchemy item non poison"sv);
      return 1.f;
    }

  if (!effect.baseEffect)
    {
      logger::debug("Base effect null"sv);
      return 1.f;
    }

  // ReSharper disable once CppCStyleCast  // NOLINT(clang-diagnostic-cast-align)
  const auto actor = (RE::Actor*)((char*)&this_ - 0x98);
  if (!actor)
    {
      logger::debug("Actor is null");
      return 1.f;
    }

  const auto resist_av = effect.baseEffect->data.resistVariable;
  const auto second_resist_av = get_second_resist_av(magic_item);

  const auto max_resist = get_max_resist(actor, config);

  const auto high_cap = 1.f / config.resist_tweaks().resist_weight();
  const auto low_cap = config.resist_tweaks().low();

  auto resist_percent = get_resist_percent(actor, resist_av, low_cap, high_cap, config);
  auto second_resist_percent =
      get_resist_percent(actor, second_resist_av, low_cap, high_cap, config);

  if (resist_percent < max_resist) { resist_percent = max_resist; }
  if (second_resist_percent < max_resist) { second_resist_percent = max_resist; }

  if (resist_av != RE::ActorValue::kNone)
    {
      if (second_resist_av == RE::ActorValue::kPoisonResist &&
          config.resist_tweaks().no_double_resist_check_poison())
        {
          second_resist_percent = 1.f;
        }
      if (second_resist_av == RE::ActorValue::kResistMagic &&
          config.resist_tweaks().no_double_resist_check_magick())
        {
          second_resist_percent = 1.f;
        }
      if (resist_av == RE::ActorValue::kDamageResist &&
          config.resist_tweaks().enable_damage_resist_tweak() &&
          config.resist_tweaks().no_double_damage_resist_check())
        {
          second_resist_percent = 1.f;
        }
    }

  return resist_percent * second_resist_percent;
}


*/

//___________ onResistApply function - unused hook now _______

/*
float on_resist_apply (RE::MagicTarget* this_, RE::MagicItem* magic_item, const RE::Effect* effect,
                      const RE::TESBoundObject* bound_object) {
    if (magic_item->hostileCount <= 0 || bound_object && bound_object->formType == RE::FormType::Armor) {
        return 1.f;
    }

    const auto alchemy_item = magic_item->As<RE::AlchemyItem>();

    if (alchemy_item &&
        ((!alchemy_item->IsPoison() && !effect->IsHostile()) ||
         (alchemy_item->GetSpellType() == RE::MagicSystem::SpellType::kIngredient && alchemy_item->IsFood()))) {
        return 1.f;  //  если у нас алхимия или ингридиент - возвращаем 1
    }

    if (!effect->baseEffect) {
        return 1.f;  //  если нет базового эффекта - возвращаем 1
    }

    const auto actor = (RE::Actor*)((char*)&this_ - 0x98);
    if (!actor) {
        return 1.f;  //  если Actor = null  - возвращаем 1
    }

    // const auto  resist_av_name        = effect->baseEffect->data.resistVariable;        //   смотрим чем режется эффект. Второй
    // резист обычно магрез, кроме ядов. const auto second_resist_av_name  = u_get_secondary_resist_name(magic_item);

    // auto max_resist = RE::GameSettingCollection::GetSingleton()->GetSetting("fPlayerMaxResistance")->GetFloat();        //
    // вернется 75 при капах резистов 75

    // const auto high_cap = 1.f / config.resist_tweaks().resist_weight();
    // const auto low_cap = config.resist_tweaks().low();
    //
    // auto resist_percent = get_resist_percent(actor, resist_av, low_cap, high_cap, config);
    // auto second_resist_percent = get_resist_percent(actor, second_resist_av, low_cap, high_cap, config);
    //
    // if (resist_percent < max_resist) {
    //     resist_percent = max_resist;
    // }
    // if (second_resist_percent < max_resist) {
    //     second_resist_percent = max_resist;
    // }

    // if (resist_av_name != RE::ActorValue::kNone) {
    //     if (second_resist_av_name == RE::ActorValue::kPoisonResist  &&
    //     config.resist_tweaks().no_double_resist_check_poison()) {
    //         second_resist_percent = 1.f;
    //     }
    //     if (second_resist_av_name == RE::ActorValue::kResistMagic  &&
    //     config.resist_tweaks().no_double_resist_check_magick()) {
    //         second_resist_percent = 1.f;
    //     }
    //     if (resist_av_name == RE::ActorValue::kDamageResist && config.resist_tweaks().enable_damage_resist_tweak() &&
    //         config.resist_tweaks().no_double_damage_resist_check()) {
    //         second_resist_percent = 1.f;
    //     }
    // }

    return 1.f;
}
*/

/*

 void CopyTextDisplayData (RE::ExtraTextDisplayData* from, RE::ExtraTextDisplayData* to)
 {
    to->displayName = from->displayName;
    to->displayNameText = from->displayNameText;
    to->ownerQuest = from->ownerQuest;
    to->ownerInstance = from->ownerInstance;
    to->temperFactor = from->temperFactor;
    to->customNameLength = from->customNameLength;
}


 [[nodiscard]] const bool UpdateExtras (RE::ExtraDataList* copy_from, RE::ExtraDataList* copy_to)
 {
	 //....

    if (copy_from->HasType(RE::ExtraDataType::kTextDisplayData)) {
        auto textdisplaydata = static_cast<RE::ExtraTextDisplayData*>(copy_from->GetByType(RE::ExtraDataType::kTextDisplayData));
        if (textdisplaydata) {
            RE::ExtraTextDisplayData* textdisplaydata_fake = RE::BSExtraData::Create<RE::ExtraTextDisplayData>();
            CopyTextDisplayData(textdisplaydata, textdisplaydata_fake);
            copy_to->Add(textdisplaydata_fake);
        } 
		else return false;
    }
 }

 */


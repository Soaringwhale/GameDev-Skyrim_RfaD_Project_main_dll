
#pragma warning (disable : 4244)
#pragma warning (disable : 4189)

#include "core.h"
#include "utils.h"
#include "ItemCardFixer.h"
#include "gameplay.h"
#include <iostream>
#include <fstream>
#include <windows.h>
#include <shlobj.h>  // to get path to app folder
#include <unordered_set>

#ifdef _UNICODE
    #define SetConsoleTitle SetConsoleTitleW            // to avoid char / lpwstr trouble 
#else
    #define SetConsoleTitle SetConsoleTitleA        
#endif

#define KINSTANT  RE::MagicSystem::CastingSource::kInstant  // for castSpellImmediate

using std::ios;

static std::map <uint32_t, uint16_t>     sf_map;
static std::map <uint16_t, std::string>  sf_tier1_names;
static std::map <uint16_t, std::string>  sf_tier2_names;
static std::map <uint16_t, std::string>  sf_tier3_names;

static std::map<RE::FormID, std::string> x_descriptions;
//static std::map<RE::FormID, std::string> old_descriptions;

static std::vector <RE::TESGlobal*> bindGlobs;
static std::vector <float> keyCodes;


static bool IsPluginInstalled (std::string espName)
{
    if (auto index = my::handler->GetModIndex(espName); index > 0) {
        return true;
    }
    else return false;
}

namespace my {        //   namespace with static variables used only in this file, so no need for their extra declarations

    void initGameData()
    {
        LOG("called my::initGameData()");
        handler = RE::TESDataHandler::GetSingleton();
        fill_gamePointers();
        fill_data();
        fill_translation();
        qst::initQuestVars();
        gameplay::initGameplayData();
        LOG("finished my::initGameData()");
    }
}

//  struct with static variables that must be visible in any file, requiring extra declarations

float               mys::reserved_MP;        
float               mys::time_delta;
float               mys::micro_timer;
float               mys::ms_compensator;
float               mys::eq_weight;
float               mys::dodge_timer;
uint16_t            mys::nb_hold_state;
uint16_t            mys::xdescr_state;
uint16_t            mys::bossUpdMeter;
bool                mys::xdescr_on;
bool                mys::hasHeavyArmor;
bool                mys::attackKeyHolding;
RE::Actor*          mys::player;
RE::UI*             mys::ui;
RE::BGSKeyword*     mys::armorHeavyKw;
RE::EffectSetting*  mys::dodge_KD_eff;
RE::SpellItem*      mys::dodge_KD_spl;
RE::BGSPerk*        mys::dodgePerk;
RE::TESGlobal*      mys::speed_cast_glob;
RE::TESGlobal*      mys::gameProcessed;
RE::TESGlobal*      mys::widget_shown;


void mys::init_globs()
{ 
    player = RE::PlayerCharacter::GetSingleton();
    ui     = RE::UI::GetSingleton();

    armorHeavyKw    = RE::TESForm::LookupByID<RE::BGSKeyword>(0x6BBD2);
    dodgePerk       = RE::TESForm::LookupByID<RE::BGSPerk>(0x79376);
    dodge_KD_eff    = my::handler->LookupForm<RE::EffectSetting>(0x15D3F4, "devFixes.esp");
    dodge_KD_spl    = my::handler->LookupForm<RE::SpellItem>(0x6AA965, "Requiem.esp");
    speed_cast_glob = my::handler->LookupForm<RE::TESGlobal>(0xBA02F2, "RfaD SSE - Awaken.esp");
    gameProcessed   = my::handler->LookupForm<RE::TESGlobal>(0x7E992,  "RfaD SSE - Awaken.esp");
    widget_shown    = my::handler->LookupForm<RE::TESGlobal>(0x753BC6,  "RfaD SSE - Awaken.esp");
    
    reserved_MP = 0.f;
    ms_compensator = 0.f;
    eq_weight = 0.f;
    dodge_timer = 0.f;
    hasHeavyArmor = false;
    attackKeyHolding = false;
    xdescr_on = false;
    xdescr_state = 0;
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
        LOG("GetFormFromString() - can't get form {} ,{}", formIDstr, modname);
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
        if (line.empty() || line[0] != '0') return;   // read only "0x..." strings

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
        string filePath = folderPath + "\\MO2\\mods\\RFAD_PATCH\\xItemDescriptions.ini";   // Patch folder

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
        if (mys::xdescr_on || item->GetFormType() == RE::FormType::Misc)
        {
           auto newDescrGfx = RE::GFxValue(x_descriptions[item->formID].c_str());          // получаем описание для предмета (из файла)
           itemCard->obj.SetMember(ItemCardFixer::descriptionVar, newDescrGfx);            // назначаем в поле "DF_description" карточки это описание
        }
    }
}

void mys::handle_keyPress (uint32_t keyCode, float hold_time, bool is_up, bool is_held) 
{
    // if (is_up) RE::DebugNotification(std::to_string(keyCode).c_str());        // to see pressed keycode in-game

    if (ui->IsMenuOpen(RE::Console::MENU_NAME)) return;    // opened console

    if (keyCode == 20 && !player->IsInCombat()) return;    // T out of combat

    if (keyCode == my::rightHandKeyCode)                   // pwatk double-hit fix
    {  
        if (is_up)
        {
           attackKeyHolding = false;
           //my::blockBashBlocker.p->value = 0;
           my::pwatk_state = 0;

           if(player->HasMagicEffect(my::longStrideEff.p) || mys::dodge_timer > 1.f) mys::dodge_timer = 0;  
        }
        else if (hold_time > 0.2f) attackKeyHolding = true;
    }

    if (keyCode == keyCodes.at(0))  // dodge
    {
        if (!hasHeavyArmor && player->HasPerk(dodgePerk))
        {
           bool d = false;   // dodge
           int type = my::dodgeType.p->value;
           if      (type == 0 && hold_time < 1.f  && dodge_timer <= 0 && !player->HasMagicEffect(dodge_KD_eff)) {d = true; dodge_timer = 0.5f;}
           else if (type == 1 && hold_time < 1.f  && dodge_timer <= 0 && !player->HasMagicEffect(dodge_KD_eff)) {d = true; dodge_timer = 0.8f;}
           else if (type == 2 && dodge_timer <= 0 && !player->HasMagicEffect(dodge_KD_eff)) {d = true; dodge_timer = 0.82f;}
           else if (type == 2 && is_up) {
               dodge_timer = 0.2f;
           }

           if (d) {
               u_cast_on_self(dodge_KD_spl, player);
               if (!player->HasMagicEffect(my::adrenalineKD.p)) {
                   gameplay::windRunnerCheck(player);
               }  
           }
        }
    }
    else if (keyCode == keyCodes.at(3))  // adrenaline
    {
        if (player->IsInCombat() && player->HasPerk(my::heavy_25_perk.p) && !player->HasMagicEffect(my::adrenalineKD.p)) {
              float st_restore = 0.25f * u_get_actor_value_max (player, RE::ActorValue::kStamina);
              if (player->HasPerk(my::heavy_50_perk.p)) st_restore *= 1.25f;
              if (is_up)  //  instant tap
              {
                  u_cast_on_self (my::adrenaline_tap.p, player);
                  player->RestoreActorValue (RE::ACTOR_VALUE_MODIFIERS::kDamage, RE::ActorValue::kStamina, st_restore);
                  RE::DebugNotification (my::adrenaline_text.c_str());
              }
              if (hold_time > 0.8f)  //  hold
              {
                  u_cast_on_self(my::adrenaline_hold.p, player);
                  player->RestoreActorValue (RE::ACTOR_VALUE_MODIFIERS::kDamage, RE::ActorValue::kStamina, st_restore);
                  RE::DebugNotification(my::adrenalineMax_text.c_str());
              }
        }
    }

    if (keyCode == keyCodes.at(1))  // bats
    {
         if (is_up && dodge_timer <= 0 && player->HasSpell(my::vampirism.p) && player->HasPerk(my::vamp_1st_perk.p)
            && !player->HasMagicEffect(my::bats_kd.p) && !player->HasMagicEffect(my::batForm_eff.p)) {
              dodge_timer = 1.40f;
              u_cast_on_self(my::become_bats.p, player);
         }
    }
    else if (keyCode == keyCodes.at(2))  // boil
    {
        if (player->HasPerk(my::vamp_left2_perk.p) && player->HasSpell(my::vampirism.p) && !player->HasMagicEffect(my::bloodThirst_kd.p))
        {
              if (is_up) my::vamp_C_holdState = 0;
              if (hold_time > 0.3f && my::vamp_C_holdState < 1)  // stage 1 hold
              {
                  my::vamp_C_holdState = 1;
                  u_playSound(player, my::vampireHeartBeat.p, 1.0f);
                  player->ApplyEffectShader(my::bloodBubbles_FX.p, 1.5f, player);  // apply shader on actor
                  return;
              }
              if (hold_time > 1.6f && my::vamp_C_holdState < 2)  // stage 2 hold
              {
                  my::vamp_C_holdState = 2;
                  player->ApplyEffectShader(my::bloodFire_FX.p, 1.5f, player);
                  return;
              }
              if (hold_time > 2.6f && my::vamp_C_holdState < 3)  // stage 3 hold
              {
                  my::vamp_C_holdState = 3;
                  my::vamp_state_glob.p->value = 1;
                  u_cast_on_self(my::bloodThirstHoldAB.p, player);
                  return;
              }
              if (is_up)  // instant tap
              {
                  if (my::vamp_state_glob.p->value == 0 && my::vamp_C_tapState < 1) {
                      float hp_ = player->GetActorValue(RE::ActorValue::kHealth);
                      if (player->HasPerk(my::heliotrope.p)) u_damage_av(player, RE::ActorValue::kHealth, hp_ * 0.1f);
                      else u_damage_av (player, RE::ActorValue::kHealth, hp_ * 0.2f);
                      u_cast_on_self(my::bloodThirstAB.p, player);
                      my::vamp_C_tapState = 1;
                  }
                  
              }
        }
    }
    else if (keyCode == keyCodes.at(4))  // nb main invis (twilight)
    {
        if (player->HasPerk(my::nb_perk_1.p) && !player->HasMagicEffect(my::nb_mainEff.p))
        {
              if (is_up) nb_hold_state = 0;
              if (hold_time > 0.4f && nb_hold_state < 1 && player->HasPerk(my::nb_perk_2.p) &&
                  !player->HasMagicEffect(my::nb_main_kd.p)) {
                  nb_hold_state = 1;
                  u_cast_on_self(my::nb_main_holdFF.p, player);
                  return;
              }
              if (is_up && !player->HasMagicEffect(my::nb_main_kd.p)) {  // tap
                  my::nb_hitCounter.p->value = 0;
                  u_cast_on_self(my::nb_mainInvisAb.p, player);
              }
        }
    }

    if (keyCode == keyCodes.at(5))  // nb sting
    {
        if (player->HasMagicEffect(my::nb_openMode.p) && !player->HasMagicEffect(my::nb_sting_kd.p)) {

              if (is_up) {
                  //my::nb_hitCounter->value = 0;
                  u_cast_on_self(my::nb_sting.p, player);
                  float st = player->GetActorValue(RE::ActorValue::kStamina);
                  float mp = player->GetActorValue(RE::ActorValue::kMagicka);
                  u_damage_av(player, RE::ActorValue::kMagicka, mp * 0.15f);
                  u_damage_av(player, RE::ActorValue::kStamina, st * 0.25f);
              }
              if (hold_time > 0.6f) {
                  //my::nb_hitCounter->value = 0;
                  u_cast_on_self(my::nb_sting_hold.p, player);
                  float st = player->GetActorValue(RE::ActorValue::kStamina);
                  float mp = player->GetActorValue(RE::ActorValue::kMagicka);
                  u_damage_av(player, RE::ActorValue::kMagicka, mp * 0.15f);
                  u_damage_av(player, RE::ActorValue::kStamina, st * 0.25f);
              }
        }
    }
    if (keyCode == keyCodes.at(6))  // nb mirror
    {
        if (is_up && my::nb_hitCounter.p->value > 2.f && !player->HasMagicEffect(my::nb_mirror_kd.p)) {
              my::nb_hitCounter.p->value = 0;
              u_cast_on_self (my::nb_Reflect.p, player);
              float st = player->GetActorValue(RE::ActorValue::kStamina);
              u_damage_av(player, RE::ActorValue::kStamina, st * 0.3f);
        }
    }
    if (keyCode == keyCodes.at(7))  // nb twin
    {
        if (is_up && player->HasPerk(my::nb_perk_2.p) && my::nb_hitCounter.p->value > 7.f && !player->HasMagicEffect(my::nb_twin_kd.p)) {
              my::nb_hitCounter.p->value = 0;
              u_cast_on_self (my::nb_2_Twin_FF.p, player);
              float st = player->GetActorValue(RE::ActorValue::kStamina);
              float mp = player->GetActorValue(RE::ActorValue::kMagicka);
              u_damage_av(player, RE::ActorValue::kStamina, st * 0.3f);
              u_damage_av(player, RE::ActorValue::kMagicka, mp * 0.3f);
        }
    }
    if (keyCode == keyCodes.at(10))  // nb blynk
    {
        if (player->HasPerk(my::nb_perk_1.p))
        {
              if (hold_time > 0.18f && nb_hold_state < 1) {
                  player->ApplyArtObject(my::nb_blynk_fx.p, 1.4f, player, false, true);
                  nb_hold_state = 2;
              } else if (hold_time > 0.3f && nb_hold_state < 3) {
                  if(!player->HasMagicEffect(my::nb_blynk_kd.p)) u_cast_on_self(my::nb_blynk_hold.p, player);
                  nb_hold_state = 4;
              }

              if (is_up) {
                  if (nb_hold_state < 1) {
                      if (!player->HasMagicEffect(my::nb_blynk_kd.p)) u_cast_on_self(my::nb_blynk.p, player);
                  }
                  nb_hold_state = 0;
              }
        } 
    }

    if (keyCode == keyCodes.at(8))  // cultist
    {
        if (is_up && player->HasPerk(my::cult_meridia_1.p) && !player->HasMagicEffect(my::meridia_kd.p)) {
              u_cast_on_self(my::meridiaFF.p, player);
        }
        else if (is_up && player->HasPerk(my::cult_nocturn_1.p)) {
              u_cast_on_self(my::nocturnalFF.p, player);
        }
        else if (is_up && player->HasPerk(my::cult_vermina_1.p)) {
              u_cast_on_self(my::verminaFF.p, player);
        }
    }
    else if (keyCode == keyCodes.at(9))  // race
    {
        if (!player->HasMagicEffect(my::race_ab_kd.p))
        {
            if      (is_up && player->HasSpell(my::redguardFF.p)) u_cast_on_self(my::redguardFF.p, player); 
            else if (is_up && player->HasSpell(my::argonianFF.p)) u_cast_on_self(my::argonianFF.p, player); 
            else if (is_up && player->HasSpell(my::khajeetFF.p))  u_cast_on_self(my::khajeetFF.p,  player);
        }
    }
    else if (keyCode == 68) {                //  F10
        if (is_up) {
            toggle_numbers_widget();
        }
    }
    else if (keyCode == 87)                  //  F11
    { 
        if (xdescr_state == 0) {
              xdescr_on = !xdescr_on;  // switch item descriptions mode
              xdescr_state = 1;
              RE::DebugNotification("Item Description Switch...");
              u_SendInventoryUpdateMessage(player, nullptr);
        }
    }
    else if (keyCode == 88)                  //  F12
    { 
        if (is_up) {
            
            player->DrawWeaponMagicHands(false);
            if (!my::meditateState) {
                RE::DebugNotification("Вы погружаетесь в свои мысли..");
                my::meditateState = true;
                RE::PlayerCamera::GetSingleton()->ForceThirdPerson();
                u_cast_on_self(my::meditationFFself.p, player);
                player->AddSpell(my::meditationSpell.p);
            }
            else
            {
                //u_jump(player);
                player->NotifyAnimationGraph("IdleSitCrossLeggedExit");
                player->NotifyAnimationGraph("IdleChairExitStart");
                player->RemoveSpell(my::meditationSpell.p);
                my::meditateState = false;
            }
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
        if (mys::player->HasSpell(my::doomSteed.p)) {
            if (baseST > 300) baseST = 300;
        }
        else baseST = 250;
    }
    // HP
    auto hp_bonuses = my::lvlup_hp_bonuses.p->effects;
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
    auto st_bonuses = my::lvlup_st_bonuses.p->effects;
    if (baseST > 109)
    {
        float armor = 0, regen_st = 0, damage = 0, atkspeed = 0, movespeed = 0, steed = 1.f;

        if (mys::player->HasSpell(my::doomSteed.p)) steed = 1.25f;

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
    auto mp_bonuses = my::lvlup_mp_bonuses.p->effects;
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

    mys::player->RemovePerk(my::gm_lvlup_bonuses.p);
    mys::player->AddPerk(my::gm_lvlup_bonuses.p);

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

        if      (buff_index == 1 && baseMana < 180)  return;        // speed
        else if (buff_index == 2 && baseMana < 190)  return;        // penetr
        else if (buff_index == 3 && baseMana < 190)  return;        // armor
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
        if (!mys::player->HasPerk(sf_perk_3.p)) return;
        
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
           
       if (baseEff->HasKeyword(my::sf_dispel_KW.p)) {
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
       else if (spellName[1] == 'C')  sf_manaReserve_buff_cast(spell_idx);                                  //  sf reserve buffs
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
        if (!my::reflectedArrow || !my::reflectedArrow->ammoSource || !weap) return 80.f;
        float weapDmg = weap->attackDamage;
        float avPowerMod = mys::player->GetActorValue(RE::ActorValue::kMarksmanPowerModifier);
        float penetr = mys::player->GetActorValue(RE::ActorValue::kMarksmanModifier);
        if (penetr > 80.f) penetr = 80.f;
        float armor = mys::player->GetActorValue(RE::ActorValue::kDamageResist);
        float penetratedArmor = armor * (1 - penetr / 100);  // for ex armor was 1000, with 30 penetr will be 700
        float armorFactorMult = (1 - penetratedArmor / 1000);  // for ex armor after penetration is 200, so mult will be 0.8
        if (armorFactorMult < 0.2f) armorFactorMult = 0.2f;
        if (armorFactorMult > 1.0f) armorFactorMult = 1.0f;
        float arrowRes = mys::player->GetActorValue(RE::ActorValue::kSpeechcraftSkillAdvance); // dealt_dmg already includes arrowResFactor, so we compensate it's effect and scale dmg
        //LOG("dealt_dmg_param - {}, weapDmg - {}, powerMod - {}, penetr - {}, armorFactorMult - {}", dealt_dmg, weapDmg, avPowerMod, penetr, armorFactorMult);

        //LOG("finished reflectedArrow_dmg()");
        float grim = u_is_equipped(mys::player, my::grimoireAssassin.p) ? 0.8f : 1.f;
        return ((dealt_dmg + weapDmg*1.5f) * (1 + avPowerMod/100)) * armorFactorMult * (1 + arrowRes/200) * grim * u_req_inc_damage(); 
}


float manaShield (RE::Actor* target, float damage, RE::TESDataHandler* handler, bool sf)
{
        LOG("called manaShield()");

        float absorbPercent = sf ? 0.25f : 0.2f;  // absorb percent
        float manaPerDmg = sf ? 2.5f : 3.f;       // Magicka cost per 1 phys damage         

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
            RE::SpellItem* breakSpell = handler->LookupForm<RE::SpellItem>(0x57C91C, "RfaD SSE - Awaken.esp");  // explosion and dispel
            target->GetMagicCaster(KINSTANT)->CastSpellImmediate(breakSpell, false, nullptr, 1.f, false, 0.f, target);
            return dmgToHP;
        }

        dmgToHP = damage * (1 - absorbPercent);
        dmgAbsorbed = damage - dmgToHP;
        manaSpent = dmgAbsorbed * manaPerDmg * u_req_inc_damage();
        target->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kMagicka, -manaSpent);

        float dmg_delta = damage - dmgToHP;
        return dmg_delta;
}

void handle_quen_4phys (RE::Actor* target, RE::Actor *agressor, float damageBeforeRes)
{
     float currentMana = target->GetActorValue(RE::ActorValue::kMagicka);
     float convertRate = 0.1f * 4.7f;  // x4.7 rate for phys, i.e. pure mag dmg is higher than phys
     float dmg = damageBeforeRes * convertRate;  // final dmg: 1200 * 0.3 for ex

     if (currentMana < dmg)  // damage nulled mana
     {
         //float dmgToHp = dmg - currentMana;
         //u_damage_av(...)
         target->NotifyAnimationGraph("StaggerStart");
         u_cast_on_self(my::quen_expl.p, target);
         u_cast_on_self(my::kd_on_self.p, target);
         // cast explosion, stagger, cast kd
     }

     u_cast_on_self(my::quen_onHitSoundVisual.p, target);
     u_damage_av (target, RE::ActorValue::kMagicka, dmg);
     u_damage_av (target, RE::ActorValue::kStamina, -dmg * 0.33f);  // restore st

     //std::string s = "QUEN BARRIER: got physHit ";
     //if (agressor) {
     //    s += "from " + std::string(agressor->GetName()) + "; ";
     //}
     //s += "damage (without armor resist) - " + std::to_string(int(damageBeforeRes)) + "; ";
     //s += "convert rate - " + std::to_string(convertRate) + "; ";
     //s += "damage to mana - " + std::to_string(int(dmg));
     //RE::ConsoleLog::GetSingleton()->Print(s.c_str()); 
}


float convert_physDmg_toFrost (RE::Actor* target, RE::HitData* hit_data, float percent)
{
    LOG("convert_physDmg_toFrost()");

    float physDmgBeforeResist = hit_data->physicalDamage;
    float physDmgAfterResist  = hit_data->totalDamage;

    float dmgToBecomeFrost = physDmgBeforeResist * percent;
    float remained_physDmgAfterResist = physDmgAfterResist * (1 - percent);

    float frostRes = target->GetActorValue(RE::ActorValue::kResistFrost);
    float magRes = target->GetActorValue(RE::ActorValue::kResistMagic);
    if (frostRes > 75) { frostRes = 75; }
    if (magRes   > 75) { magRes   = 75; }
        
    float finalFrostDmg = dmgToBecomeFrost * (1 - (frostRes/100)) * (1 - (magRes/100)); 

    auto damage_resist = 1.f;

    target->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kHealth,  -finalFrostDmg);            //  deal "frost" damage to hp/st
    target->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kStamina, -finalFrostDmg*0.3f);       //  

    float dmg_delta = physDmgAfterResist - remained_physDmgAfterResist;

    // SKSE::log::info("physDmg was = {}, physDmg become = {}, frost res = {}, frost dmg become = {}", damage, resultPhysDmg, targetFrostRes, finalFrostDmg);

    return dmg_delta;    // amount of phys damage that became frost
}

float injure_dmg_mult (RE::Actor *target)
{
    float mult = 1.f;
    if      (u_worn_has_keyword(target, my::slash_5.p)) mult *= 0.60f;
    else if (u_worn_has_keyword(target, my::slash_4.p)) mult *= 0.68f;
    else if (u_worn_has_keyword(target, my::slash_3.p)) mult *= 0.76f;
    else if (u_worn_has_keyword(target, my::slash_2.p)) mult *= 0.84f;
    else if (u_worn_has_keyword(target, my::slash_1.p)) mult *= 0.92f;

    //if (target->IsPlayerRef()) mult *= u_req_inc_damage();  // requiem in/out x1.5 etc
    //else                       mult *= u_req_out_damage();

    if (target->HasSpell(my::doomLady.p)) mult *= 0.5f;  // lady reduces here, tortle reduces from perk, so HandleEntryPoint() will see it

    return mult;
}

void proc_injure (RE::Actor* agressor, RE::Actor* target, RE::HitData *hit_data, bool isPwatk, bool knuckles = false)
{
    LOG("called proc_injure()");

    uint16_t chance = 16;
    if (knuckles) chance += 9;
    else {
         if (agressor->HasPerk(my::swordFocus2.p)) chance += 4;
         if (agressor->HasPerk(my::swordFocus3.p)) chance += 4;
    }
    
    uint16_t random = rand() % 100;

    if (random < chance) {
        //LOG("sword_injure() - applying injure");
        float dmg = hit_data->totalDamage;            //  phys dmg resisted  
        //if (!isPwatk) dmg *= 2;
        
        dmg *= (injure_dmg_mult(target) * 0.7f);

        u_playSound(target, my::slashSound.p, 2.f);
        agressor->GetMagicCaster(KINSTANT)->CastSpellImmediate(my::injureSpell.p, false, target, 1.f, false, dmg, agressor);     // onHit
    }
}


RE::TESObjectARMO* eq_knuckles (RE::Actor* a)  // returns actors worn knuckles if has unarmed weap out
{
    if (auto gauntlets = a->GetWornArmor(RE::BGSBipedObjectForm::BipedObjectSlot::kHands)) {
        if (gauntlets && gauntlets->HasKeyword(my::knuckles_kw.p)) {
            if (auto weap = u_get_weapon(a, false)) {
                if (weap && weap->formID == my::myUnarmed.p->formID) {
                    return gauntlets;
                }
            }
        }
    }
    return nullptr;
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

void print_dmg_to_console (RE::Actor* agr, RE::Actor* target, RE::TESObjectWEAP* weap, float damage, bool isPwAtk, bool blocked) 
{

    if (target->IsPlayerRef()) damage *= u_req_inc_damage();
    else                       damage *= u_req_out_damage();

    float armor = target->GetActorValue(RE::ActorValue::kDamageResist);
    float penetr = 0;

    std::string s = "hit - " + std::string(agr->GetName()) + "; ";
    if (isPwAtk) s += "[Power] ";
    else         s += "[Ligth] ";

    if (blocked) s += "[Blocked] ";

    if (weap)
    {
        if (weap->formID == my::vanillaUnarmed.p->formID)
        {
            float unarmed = agr->GetActorValue(RE::ActorValue::kUnarmedDamage);
            s += "unarmed_hidden_weap; unarmed_av - " + std::to_string(int(unarmed)) + "; ";
        }
        else if (weap->IsBow() || weap->IsCrossbow())
        {
            penetr = agr->GetActorValue(RE::ActorValue::kMarksmanModifier);
            s += "range_penetr - " + std::to_string(int(penetr));
        }
        else if (weap->GetWeaponType() < RE::WEAPON_TYPE(5))   // 1H
        {
            penetr = agr->GetActorValue(RE::ActorValue::kOneHandedModifier);
            s += "1h_penetr - " + std::to_string(int(penetr));
        }
        else  // 2H
        {
            penetr = agr->GetActorValue(RE::ActorValue::kTwoHandedModifier);
            s += "2h_penetr - " + std::to_string(int(penetr));
        }

        float total_atkDmgMult = 1.f, total_pwAtkDmgMult = 1.f, total_player_overcap = 1.f;      
        // прогоняем 1.f через ентри перков агрессора, что-бы узнать суммарные мульты. При этом проверяются кондишены, и мы получаем реальный мульт, там где кондишены не прошли игнорируется.
        RE::BGSEntryPoint::HandleEntryPoint (RE::BGSEntryPoint::ENTRY_POINT::kModIncomingDamage, target, agr, weap, std::addressof(total_player_overcap));
        RE::BGSEntryPoint::HandleEntryPoint (RE::BGSEntryPoint::ENTRY_POINT::kModAttackDamage, agr, weap, target, std::addressof(total_atkDmgMult));
        //RE::BGSEntryPoint::HandleEntryPoint (RE::BGSEntryPoint::ENTRY_POINT::kModTargetDamageResistance, agr, weap, target, std::addressof(unArmor));
        if (isPwAtk) RE::BGSEntryPoint::HandleEntryPoint (RE::BGSEntryPoint::ENTRY_POINT::kModPowerAttackDamage, agr, weap, target, std::addressof(total_pwAtkDmgMult));

        if (weap->formID == my::vanillaUnarmed.p->formID)
        {
            //s += "armorAfterUnarmedPenetr - " + std::to_string(int(unArmor)) + "; ";
        }
        else
        {
           if (!isPwAtk) {
               penetr *= 0.5f;
               s += "(/2); ";
           }
           else s += "; ";

           float remainedArmor = armor * (100 - penetr) / 100;
           s += "armorAfterPenetr - " + std::to_string(int(remainedArmor)) + "; ";
        }
    
        s += "perks_dmgMult - " + u_trimmed_str(total_atkDmgMult) + ", ";
        if (isPwAtk) s += "pwAtkDmgMult - " + u_trimmed_str(total_pwAtkDmgMult) + ",  ";
        s += "player_modIncomingDamage - " + u_trimmed_str(total_player_overcap) + ",  ";
    }
    else   {
        float unarmed = agr->GetActorValue(RE::ActorValue::kUnarmedDamage);
        s += "no_unarmed_weap; unarmed_av - " + std::to_string(int(unarmed));
    }
    
    s += "HP was - " + std::to_string(int(target->GetActorValue(RE::ActorValue::kHealth))) + ",  ";
    s += "damage - " + std::to_string(int(damage));

    RE::ConsoleLog::GetSingleton()->Print(s.c_str());  // print to console
}

void print_target_physResists (RE::Actor* agr, RE::Actor* target, RE::TESObjectWEAP* weap, float damage)  // targer overcaps
{
    std::string s = "total target modIncomingDamage - ";
    float mob_modIncomingDamage = 1.f;
    RE::BGSEntryPoint::HandleEntryPoint(RE::BGSEntryPoint::ENTRY_POINT::kModIncomingDamage, target, agr, weap, std::addressof(mob_modIncomingDamage));
    s += u_trimmed_str (mob_modIncomingDamage) + ";";
    RE::ConsoleLog::GetSingleton()->Print(s.c_str());  // print to console
}


//  [fires at the beginning of attack process, but cannot interrupt attack]
//  [hit_data is a copy here, if need change result dmg, change a float damage var below]
float on_physical_hit (RE::Actor* target, RE::HitData hit_data)   
{
    LOG("called on_physical_hit()");

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
    auto leftweapRef = agressor->GetEquippedEntryData(true);

    // hit_data.attackData.get()->data.staminaMult;   STAMINA MULT

    if (!target || !agressor) return damage;
    //LOG("allOnHitEffects___1");

    if (target->IsPlayerRef())       my::lastHiterName = agressor->GetName();  // remember last agressor for log
    else if (agressor->IsPlayerRef()) my::lastTargetName = target->GetName();  // remember last target for log

    if (target->HasMagicEffect(my::quen_barrier.p))  {                     // quen spell
        handle_quen_4phys(target, agressor, hit_data.physicalDamage);
        return 1.f;
    }

    bool isBashing {false};
    agressor->GetGraphVariableBool ("IsBashing", isBashing);
    if (isBashing || agressor->GetAttackState() == RE::ATTACK_STATE_ENUM::kBash) {  // bash damage fix
        damage *= 0.2f;
    }

    if (my::reflectedArrow && agressor->HasMagicEffect(my::projReflectAutocast.p) && weap && weap->IsBow())    // when reflected arrow hits player
    {    
        float reflectedArrowDmg = reflectedArrow_dmg (damage);
        //target->SetGraphVariableFloat("StaggerMagnitude", 5.f);
        target->NotifyAnimationGraph("StaggerStart");
        u_kill_projectile (my::reflectedArrow);
        my::reflectedArrow = nullptr;
        return reflectedArrowDmg;
    }

    if (!weap)   // bash handle
    {
        if (isBash) {                                                                                                
             u_cast(my::bash_kd_self.p, agressor, agressor);
             if (agressor->IsPlayerRef()) {
                 float stCost = (0.08f * u_get_actor_value_max(agressor, RE::ActorValue::kStamina));
                 agressor->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kStamina, -stCost);
             }
             return damage;
        }
        return damage;
    }

    if (target->IsPlayerRef())    //  [target = player]
    {
          LOG("on_physical_hit___target_was_player");

          handle_stagger (target, weap, isPowerAtk);

          float dmgSTmult = 1.f * u_req_inc_damage();
          if (target->HasSpell(my::doomLady.p))  dmgSTmult *= 0.5f;   // lady 
          if (my::mercenaryQst.p->IsCompleted()) dmgSTmult *= 0.5f;   // merc
          target->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kStamina, -damage * 0.42f);    // dmg stamina

          if (isBlocked) {
              float blockMod = target->GetActorValue(RE::ActorValue::kBlockPowerModifier);
              blockMod      += target->GetActorValue(RE::ActorValue::kBlockModifier);
              target->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kStamina, damage * blockMod/250);   // block restore stamina

              if (eq_knuckles(target)) gameplay::knuckles_block(agressor, target, damage, blockMod);  // block with knuckles
          }

          if (target->HasMagicEffect(my::nb_hold_slowTime.p)) {    // nb slowtime and invert damage to heal
                 target->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kHealth, damage);
                 return 1.0f;  // player will get 1 dmg
          }

          if (target->HasMagicEffect(my::nb_fullReflect.p)) {  // nb full reflect ability
                 agressor->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kHealth, -damage*2);
                 return 1.0f;
          }

          RE::TESForm* form = target->GetEquippedObject(false);               //  player rhand weap ref
          if (form) {
              if (u_form_has_keyword(form, my::breakableBowKW.p)) {           //  check form's keywords, don't need cast to weapon
                  u_cast_on_self(my::breakBowSpl.p, target);
              }
          }
          if (target->HasPerk(my::temp_1.p)) damage *= 1.10f;  // || target->HasPerk(my::temp_2.p))  // temp
          if (target->HasMagicEffect(my::manaShield_2.p)) {
              manaShield_dmg_delta = manaShield(target, damage, my::handler, false);
          } 
          else if (target->HasMagicEffect(my::manaShield_4_sf.p)) {
              manaShield_dmg_delta = manaShield(target, damage, my::handler, true);
          }
          if (target->HasPerk(my::snowElf_anabioz.p)) {
              convert_dmg_delta += convert_physDmg_toFrost(target, &hit_data, 0.2f);  // anabioz
          }
          if (target->HasPerk(my::falmerSetConvertPerk.p)) {
              convert_dmg_delta += convert_physDmg_toFrost(target, &hit_data, 0.15f);  // falmer set convert
          }

          print_dmg_to_console (agressor, target, weap, damage-manaShield_dmg_delta-convert_dmg_delta, isPowerAtk, isBlocked);

    }
    else    // [targer = non player]
    {
        if (target->HasKeyword(my::playersAutomaton.p)) {
            auto hpPercent = u_get_av_percent(target, RE::ActorValue::kHealth);
            if (hpPercent < 0.75f && u_get_item_count(target, my::autoRepairKit.p->formID) > 0) {  // mech auto-heal kit
                u_cast_on_self (my::autoHealFromKit.p, target);
                RE::DebugNotification("Авто-ремонт...");
                target->RemoveItem(my::autoRepairKit.p, 1, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
            }
        }
        if (agressor->IsPlayerRef())   //  [player hits npc]
        {
            LOG("on_physical_hit___agressor_was_player");
            
            //print_target_physResists (agressor, target, weap, damage - manaShield_dmg_delta - convert_dmg_delta);
            
            target->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kStamina, -damage * 0.3f);   // dmg stamina
            
            if (agressor->HasPerk(my::snowElf_anabioz.p)) {  
                  convert_dmg_delta += convert_physDmg_toFrost(target, &hit_data, 0.2f);  // anabioz
            }
            if ((weapRef && weapRef->IsPoisoned()) || (leftweapRef && leftweapRef->IsPoisoned()))    // oil
            {  
                  gameplay::oil_proc (agressor, target, &hit_data, damage);
            }
            float sf_manaDrain = sf_get_manaDrain();                    //  SF
            if (sf_manaDrain > 0)    {
                mys::player->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kMagicka, -sf_manaDrain);
            }
            if (agressor->HasSpell(my::redguardFF.p)) {
                u_cast_on_self(my::redguardBuffDmg.p, agressor);          // redguard onHit dmg
            }
            if (agressor->HasSpell(my::vampirism.p)) {                    // vampire drain onHit
                if (target->HasMagicEffect(my::bloodBrandNovaEf.p) || target->HasMagicEffect(my::bloodBrandEf.p)) {
                    gameplay::vamp_drain_onHit(agressor, target);
                }
            }
            if (mys::player->HasMagicEffect(my::nb_openMode.p)) {         // NB   
                my::nb_hitCounter.p->value += 1;
            }
            if (my::nb_magicBlade.p->value == 1.f) {                      // NB magicBlade
                if (u_get_av_percent(agressor, RE::ActorValue::kMagicka) > 0.2f) {
                    float mp = u_get_actor_value_max(mys::player, RE::ActorValue::kMagicka) * 0.06f;
                    mys::player->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kMagicka, -mp);
                    float magRes = target->GetActorValue(RE::ActorValue::kResistMagic);
                    float mult = u_is_power_attacking(agressor) ? 7.f : 3.5f; 
                    float dmg = (mp * mult) * (1 - magRes / 100.f);
                    target->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kHealth, -dmg);
                }
            }
            if (auto knuckles = eq_knuckles(agressor)) {                      //  fister knuckles
                if (knuckles->HasKeyword(my::knuckles_injuring.p)) {
                      proc_injure (agressor, target, &hit_data, isPowerAtk, true);
                }
                if (hit_data.attackData && hit_data.attackData.get()) {
                    bool isLeft = hit_data.attackData.get()->IsLeftAttack();
                    gameplay::knuckles_hit(agressor, target, knuckles, damage, isPowerAtk, isLeft);
                }
            }
            if (u_is_equipped (agressor, my::grimoireMagDmg.p)) {    // grimoire of deal magDmg
                
                float magDmg = (damage*0.15f*agressor->GetActorValue(RE::ActorValue::kEnchantingSkillAdvance)*0.1f);
                agressor->GetMagicCaster(KINSTANT)->CastSpellImmediate(my::magDmgOnHit.p, false, target, 1.f, false, magDmg, agressor);
                float curseAmount = (weap->GetWeaponType() < RE::WEAPON_TYPE(5)) ? 2.f : 4.f;
                if (isPowerAtk) curseAmount *= 1.5f;
                gameplay::increase_arcane_curse(agressor, curseAmount);
                damage *= 0.85f;
            }
        }
    }

    if (weap->HasKeyword(my::sword_1h.p) && agressor->HasPerk(my::swordFocus1.p) && !isBlocked && !target->HasMagicEffect(my::injureEff.p) && !target->HasKeyword(my::actorDwarven.p)) {
        proc_injure (agressor, target, &hit_data, isPowerAtk);
    }

    //LOG("finished allOnHitEffects()");

    return damage - manaShield_dmg_delta - convert_dmg_delta;
}


inline void handle_weaponBased_attack_speed()             //    compensation depents on weapon type and current atk speed
{

    LOG("called handle_weaponBased_attack_speed()");

    float atkSpeed  =  mys::player->GetActorValue(RE::ActorValue::kWeaponSpeedMult);    //  for example atkSpeed is 1.5
    const auto weap =  u_get_weapon(mys::player, false);

    if (atkSpeed < 1 || !weap || !weap->IsMelee()) return;

    float x = (atkSpeed - 1) / 10;            // then atkSpeed factor is 0.05                        
    float y = 0;

    if      (weap->formID == my::myUnarmed.p->formID)  y = 4.f;    // 0.05 * 4 = 0.20  for unarmed with atkspeed 1.5
    else if (weap->IsOneHandedDagger())  y = 1.8f;
    else if (weap->IsOneHandedSword())   y = 2.f;      // 0.05 * 2  =  0.10  debuff for sword with atkspeed 1.5
    else if (weap->IsOneHandedAxe())     y = 3.f;      // 0.05 * 3  =  0.15  debuff for axe with atkspeed 1.5
    else if (weap->IsOneHandedMace())    y = 3.5f;     // 0.05 * 3.5 = 0,18  debuff for mace with atkspeed 1.5
    else if (weap->IsTwoHandedSword())   y = 4.f;      // 2h sword
    else if (weap->GetWeaponType() >= RE::WEAPON_TYPE(5)) y = 4.5f;    // 2h axe / warhammer
    
    else return;    
 
    float debuff_magn = x * y;

    if (u_worn_has_keyword(mys::player, mys::armorHeavyKw)) debuff_magn *= 1.35f;

    mys::player->RemoveSpell(my::atkSpeedDebuff.p);
    auto effects = my::atkSpeedDebuff.p->effects;
    effects[0]->effectItem.magnitude = debuff_magn;
    mys::player->AddSpell(my::atkSpeedDebuff.p);
}


inline void handle_moveSpeed_compensation()                   //  depents on armor weight and current movespeed                  
{
    LOG("called handle_moveSpeed_compensation()");

    if (!mys::hasHeavyArmor && !u_worn_has_keyword(mys::player, my::weapTypeBow.p)) {mys::ms_compensator = 0; return; }
    float speedMult  = mys::player->GetActorValue(RE::ActorValue::kSpeedMult);
    float smooth = 0.f;
    if (speedMult < 101.f) return;
    if (speedMult < 110.f) {
          smooth = (0.1f - (speedMult-100)/100);
    }
    float eq_weight = u_get_worn_equip_weight (mys::player);    // armor + weapon weight
    
    // auto total_inv_weight = mys::player->GetActorValue(RE::ActorValue::kInventoryWeight);        //  for total inventory weight

    mys::ms_compensator = ((eq_weight*2.f)/1000 * (1+(speedMult-100)/150)) - smooth;    // 70 weight and 140 ms will do  ->  (0,14 * 1,26) ≈ 0,18 

    if(u_worn_has_keyword(mys::player, my::weapTypeBow.p)) mys::ms_compensator *= 2.f;

}


void player_anim_handle (RE::BSTEventSink<RE::BSAnimationGraphEvent>   *this_,
                        RE::BSAnimationGraphEvent                      *event,
                        RE::BSTEventSource<RE::BSAnimationGraphEvent> *dispatcher)

{
    // even though anim_handle is useful, animation events aren't the most stable thing, you shouldn't implement complex systems via them (c)

    // auto actor_ = const_cast<RE::Actor*>(event->holder->As<RE::Actor>());  // if we need actor, we need const_cast, for some reason holder is const (c)

    const auto animation = Utils_anim_namespace::try_find_animation(event->tag.c_str());

    //LOG ("got animation name - {}", event->tag.c_str());    // see all animation names

    if (animation == Utils_anim_namespace::AnimationEvent::kWeaponSwing ||
        animation == Utils_anim_namespace::AnimationEvent::kWeaponLeftSwing) {
            u_cast_on_self(my::stressSpell.p, mys::player);
    }
    else if (animation == Utils_anim_namespace::AnimationEvent::kBowDraw ||
             animation == Utils_anim_namespace::AnimationEvent::kBowDrawStart) {
            u_cast_on_self(my::stressStart.p, mys::player);
    }
    else if (animation == Utils_anim_namespace::AnimationEvent::kBowZoomStart) {    
           if (mys::player->HasPerk(my::bowSlowTimePerk.p)) {
                float maxst = u_get_actor_value_max(mys::player, RE::ActorValue::kStamina);
                mys::player->GetMagicCaster(KINSTANT)->CastSpellImmediate(my::zoomStress.p, false, nullptr, 1.f, false, 22.f+maxst*0.11f, mys::player);
           } 
    } 
    else if (animation == Utils_anim_namespace::AnimationEvent::kBowRelease  ||
             animation == Utils_anim_namespace::AnimationEvent::kArrowDetach ||
             animation == Utils_anim_namespace::AnimationEvent::kInterruptCast ) {
             u_cast_on_self(my::stressDispel.p, mys::player);
    }
    else if (animation == Utils_anim_namespace::AnimationEvent::kWeapEquip_Out) {                        
            const auto weap = u_get_weapon(mys::player, false);
            if (weap && weap->IsCrossbow())  mys::player->NotifyAnimationGraph("reloadStart");        //  start reload anim after every equip crossbow
    }
    //else if (animation == Utils_anim_namespace::AnimationEvent::kPreHitFrame) {
    //   my::blockBashBlocker.p->value = 0;
    //}
    //else if (animation == Utils_anim_namespace::AnimationEvent::kAttackStop) {
    //       u_cast_on_self(my::stressDispel, mys::player);
    //}
    // 
    // "FootSprintLeft" , "FootSprintRight" - sprint animations
}


bool is_const (RE::EffectSetting* baseEff) {
    return  baseEff->data.castingType == RE::MagicSystem::CastingType::kConstantEffect;
}
bool shout (RE::ActorValue av) {return av == RE::ActorValue::kShoutRecoveryMult;}

void handle_capValues_onAdjustEff (RE::Actor* actor, RE::ActorValue av, float min_cap, float max_cap, RE::SpellItem *regulator, RE::ActiveEffect *eff)
{
    LOG("called handle_capValues_onAdjustEff() for {}", regulator->GetName()); 

    //shoutrecoverymult works inverted, 0.2 magn will be -0.2 real, and detrimental will increase val
    float magn = !shout(av) ? eff->magnitude : -eff->magnitude;
    float av_val = actor->GetActorValue(av);
    float avValPlusMagn  = av_val + magn;
    float avValMinusMagn = av_val - magn;
    auto baseEff = eff->effect->baseEffect;
    bool regChanged = false;

    if (baseEff->IsDetrimental())  // detrimental = check min cap.  detrimental shout (debuffs) won't handle at all
    {
        if (av_val <= min_cap && !shout(av)) eff->magnitude = 0;
        else if (avValMinusMagn < min_cap)
        {
            float diff = min_cap - avValMinusMagn;
            if   (!is_const(baseEff)) eff->magnitude -= diff;                            // FF detrimental
            else  {regulator->effects[0]->effectItem.magnitude -= diff; regChanged = 1;} // const detrimental
        }
    }
    else // beneficial = check max cap
    {
        if (av_val >= max_cap) {
            if (!is_const(baseEff)) eff->magnitude = 0;                                 // FF beneficial
            else {regulator->effects[0]->effectItem.magnitude += magn; regChanged = 1;} // const beneficial
        }
        else if (avValPlusMagn > max_cap)  // if current av_val + adding effect magnitude will be > cap
        {
            float diff = avValPlusMagn - max_cap;
            if    (!is_const(baseEff)) eff->magnitude -= diff;                            // FF beneficial
            else  {regulator->effects[0]->effectItem.magnitude += diff; regChanged = 1;}  // const beneficial
        }
    }
    if (regChanged) {
        actor->RemoveSpell(regulator);  // re-give if need
        actor->AddSpell(regulator);
    }
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
    else if (result == 4) code = 19;    // R
    else if (result == 5) code = 2;     // 1
    else if (result == 6) code = 3;     // 2
    else if (result == 7) code = 4;     // 3

    if (my::instantAbIndex == 0) {                        
        if      (result == 8) code = 42;    // lShift
        else if (result == 9) code = 56;    // lAlt
    }
    else {
        if      (result == 8) code = 259;  // Mouse4
        else if (result == 9) code = 260;  // Mouse5
    }

    bindGlobs.at(my::instantAbIndex)->value = code;        // fill glob for game restarts
    keyCodes.at(my::instantAbIndex) = code;                // fill vector
}

bool on_drink_potion (RE::Actor* pl, RE::AlchemyItem* potion, RE::ExtraDataList* extra_data_list) 
{

    LOG("called on_drink_potion()");

    if (IsPluginInstalled("Alchemy.esp"))
    {
        // player can not use same mutagens, for ex if we have big red, we can't use big red (ck conditions)

        if (potion->HasKeyword(my::mutagen_kw.p)) gameplay::handle_mutagen (pl, potion);
    }

    //if (!potion->effects.empty())
    //{
    //    for (auto& eff : potion->effects)        {
             // ...
    //    }
    //}

    return true;                // just apply potion.  return false - decline applying
}

bool isWeaponBound(RE::TESObjectWEAP *w)
{
    return (w->HasKeyword(my::bound1H.p) || w->HasKeyword(my::bound2H.p) || w->HasKeyword(my::boundBow.p));
}

void on_apply_poison (RE::InventoryEntryData* data, RE::AlchemyItem* poison, int &count) 
{
    LOG ("called on_apply_poison()");
    if (poison->HasKeyword(my::oil_keyword.p))
    {
        if (auto weap = u_get_weapon(mys::player, false)) {
            if (!isWeaponBound(weap)) {
                u_cast_on_self(my::oil_after_use.p, mys::player);  // papyrus does all logic, animation etc

                if (poison->effects[0]->baseEffect->formID == my::oil_silver_eff.p->formID) {
                    weap->AddKeyword(my::silverdust_weap_kw.p);    // add silver_oil keyword on use
                }
            }
            else 
            {
                RE::DebugNotification("Я не смогу нанести масло на это оружие..") ;
                count = 0;      // 0 charges for bound
            } 
        }
        //return true;
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
    auto entryData = mys::player->GetEquippedEntryData(false);
    if (!entryData) return;
    if (isWeaponBound(entryData->object->As<RE::TESObjectWEAP>())) return;

    RE::EnchantmentItem* theEnch = nullptr;
    theEnch = my::snowElfEnch.p;
    uint16_t level = mys::player->GetLevel();
    float frostDmg = (level * 10) / 3.5f;
    if (frostDmg > 145.f) frostDmg = 145.f;
    float resDebuff = level * 0.2f;
    if (resDebuff > 10.f) resDebuff = 10.f;
    theEnch->effects[0]->effectItem.magnitude = frostDmg;                     //  ench magnitudes, only for description
    theEnch->effects[1]->effectItem.magnitude = resDebuff;
    my::snowElf_insteadEnch.p->effects[0]->effectItem.magnitude = resDebuff;   //   onHit spell magnitudes, real
    // theEnch->data.chargeOverride = 0xFFFF;                            //  max charge if not infinite (c)
    auto datalist = entryData->extraLists->front();

    u_enchant_equipped_weapon(mys::player->GetInventoryChanges(), entryData->object, datalist, theEnch, 0);
}

void log_mag_damage (RE::ActiveEffect *eff, RE::Actor *target)
{

    LOG("called log_mag_damage()");

    auto base = eff->effect->baseEffect;
    float magRes = target->GetActorValue(RE::ActorValue::kResistMagic);

    std::string s = "magEff - " + std::string(base->GetName()) + "; from ";

    if (eff->GetCasterActor() && eff->GetCasterActor().get()) {
        my::lastHiterName = eff->GetCasterActor().get()->GetName(); // for log
        s += eff->GetCasterActor().get()->GetName() + std::string("; ");
    }
    else s += "unknown; ";


    float magn = eff->magnitude;
    float dur = eff->duration;
    float elementRes = 0;
    bool pure = false, isPois = false;;

    if (eff->spell)
    {
         s += "SPELL - " + std::string(eff->spell->GetName()) + "; ";
         if (eff->spell->GetDelivery() == RE::MagicSystem::Delivery::kTouch) {
            s += "(ON_TOUCH); ";
         }
    }
    
    //LOG("DEBUG - log_mag_damage_111");

    if (eff->spell && eff->spell->IgnoresResistance())
    {
         if (base->formID == my::injureEff.p->formID){
             s += "[injure]; ";
             magn *= injure_dmg_mult(target);     // injure
         }
         s += "PURE_DAMAGE/NO_RES; ";
         pure = true;
    }
    else if (base->data.resistVariable == RE::ActorValue::kResistFire)
    {
         elementRes = target->GetActorValue(RE::ActorValue::kResistFire);
         s += "[FIRE]; fireRes - " + std::to_string(int(elementRes)) + "; ";
    }
    else if (base->data.resistVariable == RE::ActorValue::kResistFrost)
    {
         elementRes = target->GetActorValue(RE::ActorValue::kResistFrost);
         s += "[FROST]; frostRes - " + std::to_string(int(elementRes)) + "; ";
    }
    else if (base->data.resistVariable == RE::ActorValue::kResistShock)
    {
         elementRes = target->GetActorValue(RE::ActorValue::kResistShock);
         s += "[SHOCK]; shockRes - " + std::to_string(int(elementRes)) + "; ";
    }
    else if (base->data.resistVariable == RE::ActorValue::kPoisonResist)
    {
         elementRes = target->GetActorValue(RE::ActorValue::kPoisonResist);
         s += "[POISON]; poisRes - " + std::to_string(int(elementRes)) + "; ";
         isPois = true;
    }
    else if (base->data.resistVariable == RE::ActorValue::kResistDisease)
    {
         elementRes = target->GetActorValue(RE::ActorValue::kResistDisease);
         s += "[DISEASE]; diseaseRes - " + std::to_string(int(elementRes)) + "; ";
         isPois = true;
    }
    else if (base->HasKeyword(my::Mirel_RBB_KW_AbsorbSpell.p) || base->HasKeyword(my::DLC1VampireDrainEffect.p) || base->HasKeyword(my::MagicVampireDrain.p))
    {
         float absorb = target->GetActorValue(RE::ActorValue::kAbsorbChance);
         s += "[ABSORB]; resistFromAbsorbChance - " + std::to_string(int(absorb * 0.7f)) + "%; ";
    }
    else {
         s += "[RESISTABLE_BUT_NO_ELEM]; ";
    }

    if (base->HasKeyword(my::magicShout.p)) {   
         float shoutRes = target->GetActorValue(RE::ActorValue::kSmithingSkillAdvance);    // shouts
         s += "(SHOUT); ShoutRes - " + std::to_string(int(shoutRes)) + "; ";
         magn *= (1 - shoutRes / 100.f);
    }

    if (!pure) s += "MagRes - " + std::to_string(int(magRes)) + "; ";
    
    if (!pure) {
         if (magRes > 75.f) magRes = 75.f;
         if (!isPois && elementRes > 75.f)  elementRes = 75.f;
         if (isPois  && elementRes > 100.f) elementRes = 100.f;   // cap resists

         magn *= (1 - elementRes / 100.f);                        // mult by res
         magn *= (1 - magRes / 100.f);
    }
    
    //LOG("DEBUG - log_mag_damage_222");

    float caster_ModSpellMagn = 1.f, target_ModIncomingSpellMagn = 1.f;
    float caster_ModSpellDur  = 1.f, target_ModIncomingSpellDur  = 1.f;

    if (eff->GetCasterActor() && eff->GetCasterActor().get()) {
         auto caster = eff->GetCasterActor().get();
         RE::BGSEntryPoint::HandleEntryPoint(RE::BGSEntryPoint::ENTRY_POINT::kModSpellDuration,  caster, eff->spell, target, std::addressof(caster_ModSpellDur));
         RE::BGSEntryPoint::HandleEntryPoint(RE::BGSEntryPoint::ENTRY_POINT::kModSpellMagnitude, caster, eff->spell, target, std::addressof(caster_ModSpellMagn));
         s += "caster modSpellMagnitude - " + u_trimmed_str(caster_ModSpellMagn) + "; ";
         
    }
    RE::BGSEntryPoint::HandleEntryPoint(RE::BGSEntryPoint::ENTRY_POINT::kModIncomingSpellDuration,  target, eff->spell, std::addressof(target_ModIncomingSpellDur));
    RE::BGSEntryPoint::HandleEntryPoint(RE::BGSEntryPoint::ENTRY_POINT::kModIncomingSpellMagnitude, target, eff->spell, std::addressof(target_ModIncomingSpellMagn));
    s += "player incomingSpellMagn - " + u_trimmed_str(target_ModIncomingSpellMagn) + "; ";

    magn *= caster_ModSpellMagn;
    magn *= target_ModIncomingSpellMagn;     //  mult by perk entries
    dur  *= caster_ModSpellDur;
    dur  *= target_ModIncomingSpellDur;

    if (target->IsPlayerRef()) magn *= u_req_inc_damage();   // mult by req in/out
    else                       magn *= u_req_out_damage();
  
    s += "HP was - " + std::to_string(int(target->GetActorValue(RE::ActorValue::kHealth))) + ";  ";

    s += "DAMAGE - " + std::to_string(int(magn)) + "/" + std::to_string(int(dur)) + "sec";   // handle dur by perks later..

    RE::ConsoleLog::GetSingleton()->Print(s.c_str());  // print to console
}

void handle_quen_4mag (RE::Actor *target, RE::ActiveEffect *eff)
{

    float mana = target->GetActorValue(RE::ActorValue::kMagicka);
    float caster_ModSpellMagn = 1.f; 
    if (eff->GetCasterActor() && eff->GetCasterActor().get()) {    // caster perks factor
         auto caster = eff->GetCasterActor().get();
         RE::BGSEntryPoint::HandleEntryPoint(RE::BGSEntryPoint::ENTRY_POINT::kModSpellMagnitude, caster, eff->spell, target, std::addressof(caster_ModSpellMagn));
    }

    float magn = eff->magnitude * caster_ModSpellMagn;
    float convertRate = 0.1f * 0.7f;  // ex glob * coeff

    if (mana < magn*convertRate)
    {
         target->NotifyAnimationGraph("StaggerStart");
         u_cast_on_self(my::quen_expl.p, target);
         u_cast_on_self(my::kd_on_self.p, target);
        // cast break, stagger, kd
    }
    u_cast_on_self(my::quen_onHitSoundVisual.p, target);
    u_damage_av(target, RE::ActorValue::kMagicka, magn * convertRate);
    eff->magnitude = 0;
    eff->duration = 0;
}

void mech_msgBox_callback (unsigned int result) {        
   // if (result == 0) ... // no logic now
}

void show_mech_status (RE::Actor *mob)
{
    LOG("called show_mech_status()");
    std::string text = "Здоровье: " + std::to_string(int(mob->GetActorValue(RE::ActorValue::kHealth))) + "/";
    text += std::to_string(int(u_get_actor_value_max(mob, RE::ActorValue::kHealth))) + "\n";
    text += "Броня: " + std::to_string(int(mob->GetActorValue(RE::ActorValue::kDamageResist))) + "\n";
    text += "Сопротивление магии: " + std::to_string(int(mob->GetActorValue(RE::ActorValue::kResistMagic))) + "\n";
    text += "Огню/холоду/молнии: " + std::to_string(int(mob->GetActorValue(RE::ActorValue::kResistFire))) + "/";
    text += std::to_string(int(mob->GetActorValue(RE::ActorValue::kResistFrost))) + "/";
    text += std::to_string(int(mob->GetActorValue(RE::ActorValue::kResistShock))) + "\n";
    auto weap = u_get_weapon (mob, false) ? u_get_weapon (mob, false) : u_get_weapon (mob, true);
    if (weap) {
        if (weap->IsBow() || weap->IsCrossbow()) {
            text += "Базовый урон: " + std::to_string(weap->attackDamage) + "\n";        // тут умножать на что-то, мб на мульт индиферента и тд
            //text += "Пробивание: " + std::to_string(int(mob->GetActorValue(RE::ActorValue::kMarksmanModifier)))+"\n";
        } 
        //else  // melee
        //{  
        //    text += "Базовый урон: " + std::to_string(int(mob->GetActorValue(RE::ActorValue::kMeleeDamage)))+"\n";
        //    if (weap->GetWeaponType() < RE::WEAPON_TYPE(5))   // 1H
        //    {
        //         text += "Пробивание: " + std::to_string(int(mob->GetActorValue(RE::ActorValue::kOneHandedModifier)))+"\n";  // 1H
        //    }
        //    else text += "Пробивание: " + std::to_string(int(mob->GetActorValue(RE::ActorValue::kTwoHandedModifier)))+"\n";  // 2H
        //}
    }
    text += "МОДИФИКАЦИИ:\n";
    auto upgrades = u_get_inv_items_by_keyword (mob, "refrain_mech_upgrade");
    for (const auto& up : upgrades) {
        text += std::string(up->GetName()) + "\n";
    }
    std::vector<std::string> buttons = { "OK" };
    u_show_message_box2 (text, buttons, &mech_msgBox_callback);
}

float get_physDmg_magEffect_mult (RE::ActiveEffect *eff, RE::Actor *agressor, RE::Actor *target)   // mag effects that imitates phys dmg
{
    LOG("called get_physDmg_magEffect_mult()");
    float mult = 1.f;
    float armor = target->GetActorValue (RE::ActorValue::kDamageResist);
    float penetr = agressor ? agressor->GetActorValue (RE::ActorValue::kBlockSkillAdvance) : 30.f;     //
    if (penetr > 80.f) penetr = 80.f;
    if (penetr < 0.f) penetr = 0.f;
    armor *= (1-(penetr*0.01f));
    if (armor > 800.f) armor = 800.f;
    mult *= ((1000 - armor)*0.001f);

    if (auto mItem = eff->spell; mItem->As<RE::EnchantmentItem>()) {
        float margres = target->GetActorValue (RE::ActorValue::kResistMagic);
        mult /= (1-(margres*0.01f));                                            // if ench ingore magres
    }
    return mult;
}

void on_adjust_active_effect (RE::ActiveEffect *eff, float power, bool &unk)
{
       if (!eff || !eff->effect || !eff->effect->baseEffect) return;
       //LOG("called on_adjust_active_effect(), eff name - {}", eff->GetBaseObject()->fullName);

        auto baseEff = eff->GetBaseObject();

        RE::Actor* target = skyrim_cast<RE::Actor*>(eff->target);   // getTargetActor() doesn't work, we must skyrim_cast from RE::Magictarget
        if (!target) return;

        // onHit custom self-chants handle
        if (auto mItem = eff->spell) {
            if (mItem->GetDelivery() == RE::MagicSystem::Delivery::kTouch) {
                if (auto sourceEnch = mItem->As<RE::EnchantmentItem>()) {
                    if (auto caster = eff->caster.get().get()) {
                        auto extraEnchR = u_get_actors_weap_extra_ench(caster, false);  // only extra enchs (self-chanted)
                        auto extraEnchL = u_get_actors_weap_extra_ench(caster, true);
                        if ((extraEnchR && extraEnchR->formID == sourceEnch->formID) ||
                            (extraEnchL && extraEnchL->formID == sourceEnch->formID)) {
                               if (!u_is_power_attacking(caster)) eff->magnitude *= 0.5f;
                        }
                    }
                }
            }
        }

        // av cap handles
        if (baseEff->HasKeyword(my::regulator_eff.p)) return;
        if (baseEff->data.primaryAV == RE::ActorValue::kSpeedMult || baseEff->data.secondaryAV == RE::ActorValue::kSpeedMult)
        {  
            u_update_speedMult(target);                // update pc / npc ms av instead req_script
            if (baseEff->HasKeyword(my::magicSlow.p)) {
                if (u_actor_has_active_mgef_with_keyword(target, my::magicSlow.p)) eff->magnitude *= 0.5f;  // half-stack for ms
            }
            if (!target->IsPlayerRef()) return;        // dont handle ms cap for npc
            handle_capValues_onAdjustEff(target, RE::ActorValue::kSpeedMult, 30.f, 150.f, my::speedCap_regulator.p, eff);
            u_update_speedMult(target);
        }
        else if (baseEff->data.primaryAV == RE::ActorValue::kAbsorbChance || baseEff->data.secondaryAV == RE::ActorValue::kAbsorbChance) {
            handle_capValues_onAdjustEff(target, RE::ActorValue::kAbsorbChance, -400.f, 75.f, my::absorbCap_regulator.p, eff);
        }
        else if (baseEff->data.primaryAV == RE::ActorValue::kShoutRecoveryMult || baseEff->data.secondaryAV == RE::ActorValue::kShoutRecoveryMult) {
            handle_capValues_onAdjustEff(target, RE::ActorValue::kShoutRecoveryMult, 0.2f, 1000.f, my::shoutCap_regulator.p, eff);
        }

        //SKSE::log::info("on_adjust_active_effect - target is - {}, caster is - {}", eff->GetTargetActor()->GetName(), eff->GetCasterActor()->GetName());
        if (baseEff->HasKeyword(my::alch_heal_KW.p))
        {
              if (mys::player->HasPerk(my::paladinCenter.p))    {    
                  if (mys::player->HasPerk(my::palCenterBoost.p))
                     eff->magnitude *= 1.2f;
                  else
                     eff->magnitude *= 1.15f;
              }
              if (mys::player->HasPerk(my::chihMeridiaShield.p))
              {
                 float mult = 1 + (mys::player->GetActorValue(RE::ActorValue::kRestoration) * 0.0015f);
                 eff->magnitude *= mult;
              }
        }

        //if (baseEff->HasKeyword(my::oil_keyword.p)) {
        //      LOG("OnAdjustEff - OIL eff, name - {}, magn - {}, dur - {}", baseEff->fullName, eff->magnitude, eff->duration);   // OIL DEBUG (before perks scaling)
        //}

        if (baseEff->GetArchetype() == RE::EffectSetting::Archetype::kSlowTime)    {
              if (target->HasPerk(my::bossFightDebuff.p) && !baseEff->HasKeyword(my::slowTimeExclude.p)) {
                 eff->magnitude  = 0.82f;
                 eff->duration  *= 0.5f;
              }
        }
        if (baseEff->IsDetrimental())
        {
              if (target->HasMagicEffect(my::quen_barrier.p)) {              // quen for all
                 if (baseEff->data.primaryAV == RE::ActorValue::kHealth) {
                     handle_quen_4mag (target, eff);
                 }
              }
              // if (baseEff->data.castingType != RE::MagicSystem::CastingType::kConcentration)
              if (target->IsPlayerRef())
              {
                  if (baseEff->data.primaryAV == RE::ActorValue::kHealth) {
                      log_mag_damage (eff, target); 
                      if (u_is_equipped(target, my::grimoireConvert.p)) {
                          gameplay::increase_arcane_curse(target, eff->magnitude*0.008);
                          eff->magnitude *= 0.85f;
                      }
                  }
              }
              //else   // TEMP - see all detrimental effects, w/o handle perks entries
              //{
              //    LOG("detrimental_eff(), eff name - {}, magn - {}, dur - {}", eff->GetBaseObject()->fullName, eff->magnitude, eff->duration);
              ///}
        }
        else{
            if (!target->IsPlayerRef() && target->GetActorBase() && !target->GetActorBase()->numKeywords) gameplay::check_bestiary(target);
        }
        
        if (baseEff->HasKeyword(my::dll_check_KW.p))
        {
              if (baseEff->HasKeyword(my::bindKeyKw.p))
              {            
                 my::instantAbIndex = eff->magnitude;
                 if (my::instantAbIndex == 0) {
                     u_show_message_box(my::msgBoxDodge.p, &msgBoxChoiceCallback);  //   show MessageBox to choose Dodge key
                 }
                 else {
                     u_show_message_box(my::msgBoxAbils.p, &msgBoxChoiceCallback);  //   show MessageBox to choose Ability key
                 }
              }
              if (baseEff->formID == my::showMechStatus.p->formID) show_mech_status(target);
              if (baseEff->HasKeyword (my::snowElfEnchKW.p))  snowElf_applyChant();          // snowElf chant        
              if (baseEff->HasKeyword (my::sf_effect_KW.p))   my::sf_handle(eff, baseEff);   // sf
              if (baseEff->HasKeyword (my::longstride_KW.p))  mys::dodge_timer = 40.f;       // longstride


              if (baseEff->HasKeyword(my::physDamageEffect.p)){
                  auto caster = eff->caster.get().get();
                  eff->magnitude *= get_physDmg_magEffect_mult(eff, caster, target);  // physical mag effects
              }
                 
              // half-stack debuff system

              if (baseEff->HasKeyword(my::decreaseDmg_kw.p)) {
                  if (u_actor_has_active_mgef_with_keyword(target, my::decreaseDmg_kw.p)) eff->magnitude *= 0.5f;
              }
              else if (baseEff->HasKeyword(my::decreasePenetr_kw.p)) {
                  if (u_actor_has_active_mgef_with_keyword(target, my::decreasePenetr_kw.p)) eff->magnitude *= 0.5f;
              }
              else if (baseEff->HasKeyword(my::decreaseAtkSpeed_kw.p)) {
                  if (u_actor_has_active_mgef_with_keyword(target, my::decreaseAtkSpeed_kw.p)) eff->magnitude *= 0.5f;
              }
              else if (baseEff->HasKeyword(my::decreaseMagRes_kw.p)) {
                  if (u_actor_has_active_mgef_with_keyword(target, my::decreaseMagRes_kw.p)) eff->magnitude *= 0.5f;
              }
              else if (baseEff->HasKeyword(my::oil_remover.p)) {       // oil remover
                  u_cast_on_self(my::oil_after_use.p, mys::player);    // anim
                  u_remove_pc_poison(true);
                  u_remove_pc_poison(false);
              }
              else if (baseEff->formID == my::logEnabler.p->formID) {
                  // new char started logging / refresh first notes
                  if (!target->HasSpell(my::logEnabled.p)) log_game_info (target, false, false, nullptr, true);
              } 
              else if (baseEff->formID == my::moveAliesToPlayer.p->formID) {
                  u_move_all_followers_to_player(true, true);
              }
              //else if (baseEff->formID == my::blockBlockerEFf.p->formID)  // pwAtk block-canceling dur
              //{
              //    eff->duration = getBlockerDur(target);
              //}

              //if (baseEff->formID == my::bm_cascade_conc.p->formID)
              //{
              //    cascade_targets.insert(target->GetHandle());
              //}
              //if (baseEff->HasKeyword(my::oil_keyword)) handle_oil(target, baseEff, true);   // oil 

        }// dll check effects

    return;
}


void on_valueMod_Effect_Finish (RE::ValueModifierEffect* modEff)
{
    if (!modEff || !modEff->effect || !modEff->effect->baseEffect) return;
    //LOG("called on_valueMod_Effect_Finish()");
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

//-----------------------------------------------------------------------------------------------


void fill_launchData (RE::Projectile::LaunchData &data, RE::BGSProjectile* projBase, RE::Projectile::LaunchData* srcData = nullptr, RE::TESObjectREFR *shooter = nullptr,
                      RE::SpellItem* spell = nullptr)
{
    LOG("called fill_launchData()");
    data.projectileBase = projBase;
    data.contactNormal = srcData ? srcData->contactNormal : RE::NiPoint3();      // в этой фнк хорошо бы сделать Actor* параметр и у него брать локу, комбат контролер и тд
    data.desiredTarget = srcData ? srcData->desiredTarget : nullptr;
    data.shooter = srcData ? srcData->shooter : shooter;
    data.spell = srcData ? srcData->spell : spell;
    data.combatController = srcData ? srcData->combatController : nullptr;
    data.origin = srcData ? srcData->origin : RE::NiPoint3();
    data.angleZ = srcData ? srcData->angleZ : 1.f;
    data.angleX = srcData ? srcData->angleX : 1.f;                //  надо логнуть какой угол дефолтный у прожектайлов вперед
    data.parentCell = srcData ? srcData->parentCell: nullptr;
    data.scale = srcData ? srcData->scale : 1.f;
    data.area = srcData ? srcData->area : 1.f;
    data.castingSource = srcData ? srcData->castingSource : RE::MagicSystem::CastingSource::kOther;
    data.autoAim = 0;
    data.enchantItem = nullptr;
    data.poison = nullptr;
    data.weaponSource = nullptr;
    data.ammoSource = nullptr;
}

void on_projectile_launch (RE::Projectile* projRef, RE::Projectile::LaunchData& ldata)
{
    LOG("called on_projectile_launch()");
    if (ldata.projectileBase->formID == my::elemTriumvirateProj.p->formID)   // triumvirate of elements
    {
        if (!ldata.shooter || !ldata.shooter->IsPlayerRef()) return;
        fill_launchData (my::newLaunchData1, my::fireWaveHorzProj.p,  &ldata, nullptr);
        fill_launchData (my::newLaunchData2, my::frostWaveHorzProj.p, &ldata, nullptr);
        //my::newLaunchData1.angleX += 0.012f;
        //my::newLaunchData2.angleX += 0.024f;   // fly upper/lower (Y)

        RE::ProjectileHandle newHandle1 {};
        RE::ProjectileHandle newHandle2 {};
        RE::Projectile::Launch (&newHandle1, my::newLaunchData1);
        RE::Projectile::Launch (&newHandle2, my::newLaunchData2);

        u_set_rotation_y (newHandle1.get().get()->AsReference(), newHandle1.get().get()->GetAngleY() + 12.f);   // X-wing rotation of waves
        u_set_rotation_y (newHandle2.get().get()->AsReference(), newHandle2.get().get()->GetAngleY() - 12.f);
    }
}


bool on_arrow_collide (RE::ArrowProjectile* arrow, RE::hkpAllCdPointCollector* collidable)
{
    LOG("called on_arrow_collide()");
    if (collidable && arrow && arrow->shooter && arrow->shooter.get()) {
          auto shooter = arrow->shooter.get().get()->As<RE::Actor>();
          if (shooter && (shooter->IsPlayerRef() || shooter->HasMagicEffect(my::arrow_reflect.p)))
          {
              for (auto& hit : collidable->hits) {
                     auto refrA = RE::TESHavokUtilities::FindCollidableRef(*hit.rootCollidableA);
                     auto refrB = RE::TESHavokUtilities::FindCollidableRef(*hit.rootCollidableB);
                     //if (refrA && refrA->formType == RE::FormType::ActorCharacter) {
                        // seems refrA not using, projectile collide target is always refrB
                     //}
                     if (refrB && refrB->formType == RE::FormType::ActorCharacter)
                     {
                         if (!refrB->IsPlayerRef())            // when collides target (non-player) 
                         {
                            auto target = refrB->As<RE::Actor>();
                            if (!target || !target->HasMagicEffect(my::arrow_reflect.p)) return true;    
                            _set_proj_collision_layer(arrow, target);  
                            arrow->SetActorCause(target->GetActorCause());
                            arrow->shooter = refrB->GetHandle();
                            arrow->linearVelocity *= -1;            // change arrow's owner and turn speed to -1
                            my::reflectedArrow = arrow;
                            LOG("arrow_collide 1");
                            auto vec = u_get_effects_by_keyword(target, my::arrowReflectKW.p);
                            if (!vec.empty()) {
                                for (auto eff : vec) eff->Dispel(true);           // dispel arrow reflect eff  
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
    else if (av_value < min_cap)
    {
          float diff = (min_cap - av_value);
          regulatorSpell->effects[0]->effectItem.magnitude -= diff;
    }
    else return;    // if similar case, don't re-give regulator spell (below)

    actor->RemoveSpell(regulatorSpell);
    actor->AddSpell(regulatorSpell);
}


void handle_cap_actorValues (RE::Actor *actor)
{

    handle_cap (actor, RE::ActorValue::kSpeedMult, 30.f, 150.f, my::speedCap_regulator.p);             // speedMult
    u_update_speedMult (actor);                                                             
    handle_cap (actor, RE::ActorValue::kShoutRecoveryMult, 0.2f, 1000.f, my::shoutCap_regulator.p);     // shout recovery  (note than shout buff effects are not detrimental but makes minus magn)

    if (actor->HasSpell(my::doomAtronach.p))                                                            // atronach (alt) absorb
    {
          float absorbChance = (actor->GetActorValue(RE::ActorValue::kAbsorbChance) + 150);
          if (absorbChance > 75.f) absorbChance = 75.f;
          my::atronachAbsorbChanceGlob.p->value = absorbChance;    
    }
    else {
        handle_cap(actor, RE::ActorValue::kAbsorbChance, -400.f, 75.f, my::absorbCap_regulator.p);       // absorb
    }
}

void update_mass_effects (RE::Actor* actor, float total_eq_weight = 0, bool aboutToEquipHeavyArmor = false)
{

    LOG("called update_mass_effects()");
    if (total_eq_weight == 0) total_eq_weight = u_get_worn_equip_weight(actor);   // if called with default value
    float perks_speedFactor = 1.f, perks_InfamyFactor = 1.f, perks_noiseFactor = 1.f;

    if (mys::hasHeavyArmor || aboutToEquipHeavyArmor)  // heavy armor 
    {
        if (actor->HasPerk(my::heavy_1_perk.p))  {perks_speedFactor -= 0.4f;  perks_InfamyFactor *= 0.95f;}
        if (actor->HasPerk(my::heavy_1_perk2.p)) {perks_speedFactor -= 0.2f; }
        if (actor->HasPerk(my::heavy_25_perk.p)) {perks_speedFactor -= 0.09f; perks_InfamyFactor *= 0.85f;}
        if (actor->HasPerk(my::heavy_sprint_perk.p)) {perks_speedFactor -= 0.06f; perks_InfamyFactor *= 0.8f;}
        if (actor->HasPerk(my::heavy_50_perk.p)) {perks_speedFactor -= 0.05f; perks_InfamyFactor *= 0.7f;}
        if (actor->HasPerk(my::heavy_75_perk.p)) {perks_speedFactor -= 0.05f; perks_InfamyFactor *= 0.8f;}
        if (actor->HasPerk(my::heavy_100_perk.p)){/*speedFactor not affect*/  perks_InfamyFactor *= 0.7f; }    
    }
    else {
        perks_speedFactor = (actor->HasPerk(my::evasion_1_perk.p)) ? 0.25f : 0.5f;  // light
        perks_InfamyFactor = (actor->HasPerk(my::evasion_1_perk.p)) ? 0.5f : 1.0f;
        perks_noiseFactor = (actor->HasPerk(my::evasion_1_perk.p)) ? 0.5f : 1.0f;
    }
    //LOG("111");
    my::mass_penalty_speed.p->effects[0]->effectItem.magnitude = total_eq_weight * perks_speedFactor;
    my::mass_penalty_noise.p->effects[0]->effectItem.magnitude = total_eq_weight * 0.01f * perks_noiseFactor;  
    my::mass_buff_mass.p->effects[0]->effectItem.magnitude = total_eq_weight * 0.01f;
    my::mass_buff_infamy.p->effects[0]->effectItem.magnitude = total_eq_weight * 0.01f * perks_InfamyFactor;
    actor->RemoveSpell(my::mass_buff_mass.p);
    actor->RemoveSpell(my::mass_buff_infamy.p);
    actor->RemoveSpell(my::mass_penalty_noise.p);
    actor->RemoveSpell(my::mass_penalty_speed.p);
    actor->AddSpell(my::mass_penalty_speed.p);
    actor->AddSpell(my::mass_buff_mass.p);
    actor->AddSpell(my::mass_buff_infamy.p);
    actor->AddSpell(my::mass_penalty_noise.p);
    //LOG("222");
    u_update_speedMult(actor);
    // auto total_inv_weight = mys::player->GetActorValue(RE::ActorValue::kInventoryWeight);   //  for total inventory weight
}


void on_spell_release (RE::ActorMagicCaster* this_, RE::MagicItem* magic_item)   // works 1 time after spell release, don't catch that spell was dual-casted, except conc
{
    LOG("on_spell_release()");
    if (auto caster = this_->actor) {
        //this_->GetIsDualCasting());   // only conc
    }
}

bool on_check_cast (RE::ActorMagicCaster* this_, RE::MagicItem* mitem, bool dualCast)   // for ff spells works 1 time on casting and 2 times after release, always knows if dual cast.
{                                                                                       // for conc spells works every second. Also works when eating food, potions etc
    LOG("on_check_cast()");                                                             // works for shout to check kd, if force true we can shout even in kd
    if (this_; auto caster = this_->actor) {
        if (mitem; auto spell = mitem->As<RE::SpellItem>()) {
            if (mitem->GetSpellType() != RE::MagicSystem::SpellType::kSpell) return true;   // not power, ability, voice, scroll etc
            if (!caster->IsCasting(spell)) return true;   // without isCasting check will work for spells when opening magic menu. With this check still works for release and casting.
            gameplay::handle_cast_arcane_curse (caster, spell, dualCast);
        }     
    }
    return true;
}

float on_resist_apply (RE::MagicTarget* this_, RE::MagicItem* magic_item, const RE::Effect* effect)
{
    LOG("called on_resist_apply()");
    // for conc works every ~2 sec
    return 1.f;
}

void on_jump (RE::Actor *actor)
{
    LOG("called on_jump()");
    actor->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kStamina, -12.f);
    if (actor->HasSpell(my::meditationSpell.p)) actor->RemoveSpell(my::meditationSpell.p);
    my::meditateState = false;
}

void on_3d_load (RE::TESObjectREFR *refr)   // on spawn
{
    //LOG("on_3d_load()");
    //auto actor = refr->As<RE::Actor>();
    //
}

bool can_be_passed_mech (RE::TESBoundObject* obj)  // only items can be given / taken to mech
{
    LOG("can_be_passed_mech()");
    if (obj->As<RE::TESObjectMISC>() && obj->As<RE::TESObjectMISC>()->HasKeyword(my::canBeGivenToMech.p)) return true;
    if (obj->As<RE::TESObjectWEAP>() && obj->As<RE::TESObjectWEAP>()->HasKeyword(my::canBeGivenToMech.p)) return true;
    return false;
}


bool on_remove_item_from_actor (RE::Character *actor, RE::TESBoundObject* item, std::int32_t count, RE::ITEM_REMOVE_REASON reason, RE::TESObjectREFR* targetCont)
{
    LOG("on_remove_item_from_actor()");
    if (targetCont) {
        auto toActor = targetCont->As<RE::Actor>();   // give to actor-mech
        if (toActor && toActor->GetActorBase() && toActor->GetActorBase()->HasKeyword(my::playersAutomaton.p)) return can_be_passed_mech (item);
    }
    if (actor && actor->GetActorBase()) {   // take from actor-mech
        if (actor->GetActorBase()->HasKeyword(my::playersAutomaton.p) && targetCont) return can_be_passed_mech (item);
    }
        
    //if (reason == RE::ITEM_REMOVE_REASON::kStoreInTeammate)    // 
    //if (reason == RE::ITEM_REMOVE_REASON::kStoreInContainer)
    return true;  // allow move from container
}

bool on_remove_item_from_cont (RE::TESObjectREFR* cont, RE::TESBoundObject* item, std::int32_t count, RE::ITEM_REMOVE_REASON reason, RE::TESObjectREFR* targetCont)
{
    LOG("on_remove_item_from_cont()");
    //...
    return true;
}

bool on_additem_to_actor (RE::Character *actor, RE::TESBoundObject* object, RE::ExtraDataList* extraList, std::int32_t count, RE::TESObjectREFR* fromRefr)
{
    if (!actor || !object) return true;
    LOG("on_additem_to_actor() for - {}", object->GetName());
    return true;
}

bool on_additem_to_cont (RE::TESObjectREFR *cont, RE::TESBoundObject* object, RE::ExtraDataList* extraList, std::int32_t count, RE::TESObjectREFR* fromRefr)
{
    if (!cont || !object) return true;
    LOG("on_additem_to_cont() for - {}", object->GetName());
    return true;
}

void on_pickup_object (RE::Character* actor, RE::TESObjectREFR* object, std::int32_t count, bool arg3, bool playSound)
{
    if (!actor || !object) return;
    LOG("on_pickup_object() for - {}", object->GetName());
}


inline void update_perkArmorBuffs (RE::Actor* actor)  //  to apply perk % armor bonuses for npc
{
    actor->ModActorValue(RE::ActorValue::kDamageResist, -1.0f);
    actor->ModActorValue(RE::ActorValue::kDamageResist, 1.0f);
}

void on_equip (RE::Actor* actor, RE::TESBoundObject* object, RE::ObjectEquipParams* params) 
{
    if (!actor || !object) return;
    LOG("called on_equip() for - {}", object->GetName());
    if (actor->HasMagicEffect(my::bossFightStarter.p)) gameplay::handle_bossAppearance(actor);   // boss   
    if (!actor->IsPlayerRef()) { update_perkArmorBuffs(actor); return; }
    if (!object->IsWeapon() && !object->IsArmor()) return;
    float worn_weight = u_get_worn_equip_weight(mys::player);                  //  at this momemt this item is not equipped yet, and its weight won't be taken into account
    float this_item_weight = object->GetWeight();
    float total_eq_weight = worn_weight + this_item_weight;                    //  add new item weight to current weight
    bool aboutToEqHeavyArmor = false;

    if (object->IsArmor())    {
        if (auto armor = object->As<RE::TESObjectARMO>())
        {
             if (armor->HasKeyword(mys::armorHeavyKw)) {
                 aboutToEqHeavyArmor = true;
             }
             if (armor->HasKeyword(my::knuckles_kw.p)) {
                 RE::ActorEquipManager::GetSingleton()->EquipObject (actor, my::myUnarmed.p);   // equip unarmed weap
             } 
        }
    }
    else if (object->IsWeapon()) {
        if (auto weap = object->As<RE::TESObjectWEAP>()) {
            //..
        }
    }
    update_mass_effects(actor, total_eq_weight, aboutToEqHeavyArmor);

    // if (params->equipSlot...)
}

void on_unequip (RE::Actor* actor, RE::TESBoundObject* object, RE::ObjectEquipParams* params)
{
    if (!actor || !object) return;
    LOG("called on_unequip() for - {}", object->GetName());
    if (!actor->IsPlayerRef()) return;
    if (!object->IsWeapon() && !object->IsArmor()) return;
    float worn_weight = u_get_worn_equip_weight(mys::player);
    float this_item_weight = object->GetWeight();
    float total_eq_weight = worn_weight - this_item_weight;
    update_mass_effects(actor, total_eq_weight);
}


void encrypt (std::string& input, int shift) {
    for (char& c : input) c += shift;
}

void decrypt (std::string& input, int shift) {
    for (char& c : input) c -= shift;
}

bool only_digits (const std::string& str) { return str.find_first_not_of("0123456789") == std::string::npos; }

void log_game_info (RE::Actor* pl, bool load, bool death, RE::Actor* killer, bool refresh) 
{
    LOG("called log_game_info()");
    using string = std::string;

    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);  // path to app folder, in our case RfaD SE/

    string dllPath = path;
    size_t lastSlashPos = dllPath.rfind('\\');
    string folderPath = dllPath.substr(0, lastSlashPos + 1);
    string filePath = folderPath + "\\Logs\\NewLog.dat";  // RfaD SE / Logs

    int cur_deaths = 0, cur_notes = 0;
    std::vector<std::string> logFileLines;  // all log file content

    std::ifstream ifs (filePath.c_str());   // read
    if (ifs.is_open()) {
        
        string line;
        while (std::getline(ifs, line)) {
             logFileLines.push_back(line);  // save string by string to vec
        }
        ifs.close();

        if (logFileLines.size() >= 2) {
             if (refresh) {
                 logFileLines[0] = "0";  // just turned on logs on new character
                 logFileLines[1] = "1";
             }
             else {
                 decrypt(logFileLines[0], my::c_key);
                 decrypt(logFileLines[1], my::c_key);
                 // check if first strings after decrypt are only digits, to avoid crash from stoi()
                 if (only_digits(logFileLines[0])) cur_deaths = std::stoi(logFileLines[0]);  // current char deaths
                 if (only_digits(logFileLines[1])) cur_notes = std::stoi(logFileLines[1]);   // current char notes
                 cur_notes++;                                                                //
                 if (death) cur_deaths++;  // increment deaths if death
                 logFileLines[0] = std::to_string(cur_deaths);
                 logFileLines[1] = std::to_string(cur_notes);  // save new values
             }
        }
    } 
    else {
        logFileLines.push_back("0");
        logFileLines.push_back("1");  // if no file, create first notes
    }

    string info = "[" + logFileLines[1] + "] ";
    
    if      (death)   info += "DEATH; ";
    else if (load)    info += "LOAD";
    else if (refresh) info += "NEW_CHAR_STARTED_LOGGING; ";
    else              info += "Info; ";

    if (load) {
        info += (pl->IsInCombat()) ? "(IС); " : "(NC); ";
    }

    info += string(pl->GetName()) + "; ";

    if (death)    {
        if (killer) {
            info += "Killer - " + string(killer->GetName()) + "; " + "Killer ID - " + u_int2hex(killer->formID) + "; ";
        }
    }
    info += string("Lvl ") + std::to_string(pl->GetLevel()) + "; ";
    if (pl->GetCurrentLocation()) {
        info += string(pl->GetCurrentLocation()->fullName) + "; ";
    }
    else info += "Loc - ?; ";

    //LOG("1111");
    bool same = false;
    if (my::lastTargetName.length() > 0) {
        if (my::lastTargetName == my::lastHiterName) {
             info += "Last Hit To/From - " + my::lastTargetName + "; ";
             same = true;
        }
        else info += "Last Hit To - " + my::lastTargetName + "/";
    }
    else info +="NO_HITS_TO/";

    if (my::lastHiterName.length() > 0) {
        if (!same) info += "From - " + my::lastHiterName + "; ";
    }
    else info += "FROM; ";

    int req_incoming_damage = int(RE::GameSettingCollection::GetSingleton()->GetSetting("fDiffMultHPToPCL")->GetFloat()*100); 
    int req_dealt_damage    = int(RE::GameSettingCollection::GetSingleton()->GetSetting("fDiffMultHPByPCL")->GetFloat()*100);
    info += string("Req ") + std::to_string(req_dealt_damage) + "/" + std::to_string(req_incoming_damage) + "; ";
    info += string("DMult ") + u_trimmed_str(pl->GetActorValue(RE::ActorValue(RE::ActorValue::kAttackDamageMult))) + "; ";

    if (!death) // info
    {
        string baseMP = std::to_string(int(mys::player->GetBaseActorValue(RE::ActorValue::kMagicka)));
        string baseHP = std::to_string(int(mys::player->GetBaseActorValue(RE::ActorValue::kHealth)));
        string baseST = std::to_string(int(mys::player->GetBaseActorValue(RE::ActorValue::kStamina)));
        info += string("Base M/H/S ") + baseMP + "/" + baseHP + "/" + baseST + "; ";
        info += string("MaxH ") + std::to_string(int(u_get_actor_value_max(pl, RE::ActorValue::kHealth))) + "; ";
        //info += string("CarryWeight - ") + std::to_string(int(pl->GetActorValue(RE::ActorValue::kCarryWeight))) + "; ";
        info += string("Gold ") + std::to_string(u_get_item_count(pl, 0xf)) + "; ";
        info += string("XP ") + std::to_string(int(my::xp_points.p->value));
        info += "(x" + u_trimmed_str(my::xp_mult.p->value) + "); ";

        if (auto weapR = u_get_weapon(pl, false)) {
            info += "Weap - " + string(weapR->GetName()) + "; ";
            if (auto weapL = u_get_weapon(pl, true)) {
                if (weapL->GetWeaponType() < RE::WEAPON_TYPE(5))     // 1H weap, no need log 2h weapon 2 times
                    info += ", " + string(weapL->GetName()) + "; ";
            }
            else info += "; ";
        }
        auto helm = pl->GetWornArmor(RE::BGSBipedObjectForm::BipedObjectSlot::kHair);   // helm / mask
        if (helm) info += "Head - " + string(helm->GetName()) + "; ";

        if (IsPluginInstalled("AddItemMenuSE.esp")) {
            info += "[Additem] ON!; ";
        }
        if (IsPluginInstalled("Stats Editor MCM Menu.esp")) {
            info += "[StatEditor] ON!; ";
        }
    }
    //char time_buf[21];
    //time_t now;
    //time(&now);
    //strftime (time_buf, 21, "%Y-%m-%d|%H:%M:%S", gmtime(&now));  // compil says gmtime() is unsafe
    //info += string(time_buf) + "(+3);";

    char time_buf[21];
    time_t now = time(nullptr);  // Получаем текущее время
    struct tm time_info;         // Структура для хранения времени
    if (gmtime_s(&time_info, &now) == 0) {  // Если успешно 
        strftime(time_buf, sizeof(time_buf), "%Y-%m-%d|%H:%M:%S", &time_info);
        info += string(time_buf) + "(+3);";
    } 
    else info += "ERROR_TIME;"; 

    if (load) info += "HP_WAS - " + std::to_string(int(pl->GetActorValue(RE::ActorValue::kHealth))) + "; ";

    info += u_get_entered_console_commands();  // console
    
    encrypt(logFileLines[0], my::c_key);
    encrypt(logFileLines[1], my::c_key);
    encrypt(info, my::c_key);
    info = "#" + info;
    logFileLines.push_back(info);  // add new crypted string to prev content. All vector looks like 2 digit strings + crypted content with '#' delim
    std::ofstream ofs (filePath.c_str(), std::ios::trunc); // open in full re-write mode
    if (ofs.is_open())
    {
        for (size_t i = 0; i < logFileLines.size(); i++) {
            ofs << logFileLines[i];
            if (i<2) ofs << '\n';
        }
        ofs.close();
    }
    else LOG ("log_player_info - ofstream file not open");
}

void log_pre_load_game (RE::Actor* pl)
{
    LOG ("called log_pre_load_game()");
    if (!pl->HasSpell(my::logEnabled.p)) return;
    log_game_info (pl, true);
}

void log_player_death (RE::Actor* pl, RE::Actor* killer)
{
    LOG("called log_player_death()");
    if (!pl->HasSpell(my::logEnabled.p)) return;
    log_game_info (pl, false, true, killer);
}


void delete_saves()    // delete all saves after death
{
    LOG("called delete_saves()");

    using string = std::string;
    char path [MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);  // path to app folder, in our case RfaD SE/
    string dllPath = path;
    size_t lastSlashPos = dllPath.rfind('\\');
    string folderPath = dllPath.substr(0, lastSlashPos + 1);
    string savesPath = folderPath + "\\MO2\\profiles\\RfaD SE 5.2\\saves";    // RfaD SE / ... / saves

    namespace fs = std::filesystem;
    for (const auto& entry : fs::directory_iterator(savesPath)) {
        if (fs::is_regular_file(entry)) {
            fs::remove(entry.path());
        }
    }
}

void on_death (RE::Actor* victim, RE::Actor* killer)  // event onDeath, called after death
{
    LOG("called on_death()");
    if (!victim) return;
    qst::check_onDeath_questGoals(victim);
    if (victim->IsPlayerRef()) {
        log_player_death(victim, killer);
        std::string s = "Вас убил: ";
        if (killer) s += killer->GetName();
        RE::ConsoleLog::GetSingleton()->Print(s.c_str());
        if (victim->HasSpell(my::deleteSavesEnabled.p)) delete_saves();   //  delete all saves permanently
    }
    if (victim->HasMagicEffect(my::bossFightStarter.p)) {
        my::bossFightID.p->value = 0;
    }
}

void my::on_wait()
{
    LOG("called on_wait()");

    auto poisonDataR = u_get_pc_poison(false);  // R hand
    auto poisonDataL = u_get_pc_poison(true);   // L hand

    if (poisonDataR && poisonDataR->count > 1) {
        poisonDataR->count = 0;
        u_remove_pc_poison(false);
    }
    if (poisonDataL && poisonDataL->count > 1) {
        poisonDataL->count = 0;
        u_remove_pc_poison(true);
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

void on_activate (RE::TESObjectREFRPtr whoActivated, RE::TESObjectREFRPtr objectActivated)  // event
{
    auto whoActivatedRef = whoActivated.get();
    auto activatedObjRef = objectActivated.get();
    // ...
}

void on_location_change (RE::TESObjectREFRPtr refHandle, RE::BGSLocation* oldLoc, RE::BGSLocation* newLoc)  // event
{
    //auto refr  = refHandle.get();
    //auto actor = refr->As<RE::Actor>();
    //...
}

RE::BSEventNotifyControl on_key_input (const InputEvents* evns)
{
    if (!*evns) return RE::BSEventNotifyControl::kContinue;
    for (RE::InputEvent *e = *evns;  e;  e = e->next)        //  начинаем идти по InputEvents пока евент (е) истинен
    {            
        switch (e->eventType.get()) {
		case RE::INPUT_EVENT_TYPE::kButton:
                RE::ButtonEvent* buttonEvent = e->AsButtonEvent();
                if (buttonEvent && buttonEvent->HasIDCode())        //   && (buttonEvent->IsDown() || buttonEvent->IsPressed())
                {
                    uint32_t keyMask = buttonEvent->idCode;
                    uint32_t keyCode;
                    if (buttonEvent->device.get() == RE::INPUT_DEVICE::kMouse) {   // Mouse
                        keyCode = Utils::kMacro_NumKeyboardKeys + keyMask;
                    }
                    else if (buttonEvent->device.get() == RE::INPUT_DEVICE::kGamepad) {   // Gamepad
                        keyCode = Utils::GamepadMaskToKeycode(keyMask);  // for gamepad we need convert
                    }
                    else   // Keyboard
                        keyCode = keyMask;  //  for keyboard, keymask == keycode

                    if (keyCode >= Utils::kMaxMacros) continue;   // Valid scancode?

                    float duration  = buttonEvent->heldDownSecs;
                    //bool isPressed  = buttonEvent->value != 0 && duration >= 0;
                    //bool isReleased = buttonEvent->value == 0 && duration != 0;
                 
                    if (keyCode != 17)        // for [W] 
                        mys::handle_keyPress (keyCode, duration, buttonEvent->IsUp(), buttonEvent->IsHeld());

                    //if (isPressed) {
                        // ActionManager::GetSingleton()->OnKeyPressed(keyCode, duration);
                        // if (Globals::ui->numPausesGame == 0 && Globals::p->GetActorRuntimeData().currentProcess)
                        // ActionManager::GetSingleton()->EvaluateAndPerformActions();
                    //} else if (isReleased) {
                        // ActionManager::GetSingleton()->OnKeyReleased(keyCode);
                    //}
                }  
        }
    }
    return RE::BSEventNotifyControl::kContinue;
}

void check_silver_oil (RE::Actor *pl, bool is_left, RE::AlchemyItem *oil)     // re-add silver_oil kw after game re-log 
{
    if (oil->effects[0]->baseEffect->formID == my::oil_silver_eff.p->formID) {
        if (auto weap = u_get_weapon(pl, is_left)) {
            if (!weap->HasKeyword(my::silverdust_weap_kw.p)) {
                weap->AddKeyword(my::silverdust_weap_kw.p);        
            }
        }
    }
}

void remove_oil (RE::Actor *pl, bool is_left)
{
    u_remove_pc_poison(is_left);   // remove poison

    if (auto weap = u_get_weapon(pl, is_left)) {
        if (weap->HasKeyword(my::silverdust_weap_kw.p)) {
            weap->RemoveKeyword(my::silverdust_weap_kw.p);   // remove silver oil kw
        } 
    }
}

void update_oil (RE::Actor *pl)
{
    LOG("called update_oil()");
    auto poisonDataR = u_get_pc_poison(false);    // R hand
    auto poisonDataL = u_get_pc_poison(true);     // L hand

    if (poisonDataR) {
        if (poisonDataR->count > 0) {
            if (pl->IsWeaponDrawn()) poisonDataR->count -= 1;  // decrement oil every update
            check_silver_oil (pl, false, poisonDataR->poison);
        }
        else remove_oil (pl,false);
    }
    if (poisonDataL) {
        if (poisonDataL->count > 0) {
            if (pl->IsWeaponDrawn()) poisonDataL->count -= 1;
            check_silver_oil (pl, true, poisonDataL->poison);
        }
        else remove_oil (pl, true);  
    } 
}

void update_exp (RE::Actor* pl)
{
    my::exp_upd_counter++; 
    if (my::exp_upd_counter > 3) {
        my::exp_upd_counter = 0;
        u_cast_on_self(my::exp_update.p, pl);
    } 
}

void update_gamelog (RE::Actor *pl)
{
    if (!pl->HasSpell(my::logEnabled.p)) return;

    my::log_counter++;
    if (my::log_counter > 90) {    // 90 = every 6 min 
         my::log_counter = 0;
         log_game_info(pl);
    }
}

inline void update_keycodes ()
{
    for (size_t i = 0; i < bindGlobs.size(); i++) {
          keyCodes.at(i) = bindGlobs.at(i)->value;
    }
    my::rightHandKeyCode = RE::ControlMap::GetSingleton()->GetMappedKey("Right Attack/Block"sv, RE::INPUT_DEVICE::kMouse) + Utils::kMacro_NumKeyboardKeys;
    // if device = mouse it returns keyMask (as in InputWatcher), i have to add 256 (NumKeyboardKeys) to it, but still can bind RHand on keyBoard, so how to get device? (todo)
}

void snowElf_RE_Ench (bool is_left)  // do once after load game to refresh ench
{
      auto entryData = mys::player->GetEquippedEntryData(is_left);
      if (entryData) u_remove_weap_ench(entryData);
      snowElf_applyChant();
      my::snowElf_re_chanted = true;
}

void snowElf_checkEnch (RE::Actor *pl)
{
    if (pl->HasSpell(my::snowElf_raceAb.p))
    {
        auto rightEnch = u_get_actors_weap_extra_ench (pl, false);  // checks only custom ExtraEnch i.e in-game chanted
        auto leftEnch  = u_get_actors_weap_extra_ench (pl, true);

        if     (rightEnch && rightEnch->formID == my::snowElfEnch.p->formID) {
                  if (!my::snowElf_re_chanted) snowElf_RE_Ench(false);
                  my::snowElf_wears_EnchWeap.p->value = 1;
        }
        else if (leftEnch && leftEnch->formID == my::snowElfEnch.p->formID) {
                  if (!my::snowElf_re_chanted) snowElf_RE_Ench(true);
                  my::snowElf_wears_EnchWeap.p->value = 1;
        }
        else    { my::snowElf_wears_EnchWeap.p->value = 0;}
              
    }
}

void check_knuckles (RE::Actor *pl)  // equip/unequip
{
    if (pl->HasSpell(my::meditationSpell.p)) return;
    if (u_get_item_count(pl, my::myUnarmed.p->formID) && !u_worn_has_keyword(pl, my::knuckles_kw.p)) {   // have [unarmed] but no gloves eq
        pl->RemoveItem(my::myUnarmed.p, 1, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);  // -> remove unarmed
    }
    else if (!u_worn_has_keyword(pl, my::vendorItemWeapon.p) && !u_worn_has_keyword(pl, my::armorShield.p)){  // no weap/shield
        if (!(pl->GetEquippedObject(0) && pl->GetEquippedObject(0)->IsMagicItem()) && 
            !(pl->GetEquippedObject(1) && pl->GetEquippedObject(1)->IsMagicItem()))     // no spells
        {
            if (u_worn_has_keyword(pl, my::knuckles_kw.p))  // equipped knuckles gloves
            {
                if (u_get_item_count(pl, my::myUnarmed.p->formID) == 0)  // no [unarmed] in inventory/dropped
                    u_cast_on_self(my::knuckles_reequip.p, pl);  // -> additem + equip [unarmed]
                else                                   // [unarmed] in inventory
                    RE::ActorEquipManager::GetSingleton()->EquipObject(pl, my::myUnarmed.p);  // -> only equip
            }
        }
    }
}

void check_nb (RE::Actor* pl)
{
    if (auto Lweap = u_get_weapon(pl, true)) {
        if (Lweap->IsOneHandedDagger()) {
            if (my::nb_magicBlade.p->value == 1.f) Lweap->weaponData.reach = 0.75f;
            else Lweap->weaponData.reach = 0.49f;
        }
    }
    if (auto Rweap = u_get_weapon(pl, false)) {
        if (Rweap->IsOneHandedDagger()) {
            if (my::nb_magicBlade.p->value == 1.f) Rweap->weaponData.reach = 0.75f;
            else Rweap->weaponData.reach = 0.49f;
        }
    }
}

void handle_fister (RE::Actor *pl)  // dmg type keywords, chances
{
    float st = 15.f;  // pure stagger chance
    float im = 30.f;  // bash immunne chance
    if (pl->HasSpell(my::handFocus1.p)) {st += 2;  im += 10;}
    if (pl->HasSpell(my::handFocus2.p)) {st += 2;  im += 10;} 
    if (pl->HasSpell(my::handFocus3.p)) {st += 1;  im += 10;}

    if (auto gauntlets = pl->GetWornArmor(RE::BGSBipedObjectForm::BipedObjectSlot::kHands)) {
        if (gauntlets->HasKeyword(my::knuckles_kw.p)) {
            if (gauntlets->HasKeyword(my::knuckles_bleed.p)) my::myUnarmed.p->AddKeyword(my::DmgType_Pierce.p);
            if (gauntlets->HasKeyword(my::knuckles_blunt.p)) my::myUnarmed.p->AddKeyword(my::DmgType_Blunt.p);
            if (gauntlets->HasKeyword(gameplay::armorDragonscale.p)) {st += 5; im += 10;}  // dragonscale chances
            if (gauntlets->HasKeyword(my::vendorItemHide.p)) {im += 15;}             // hide (horker) chances
        }
    }
    my::pureStaggerChance.p->value = st;
    my::bashImmunChance.p->value = im;
}

void handle_pl_arcane_curse (RE::Actor *pl)
{
    LOG("called handle_pl_arcane_curse()");
    gameplay::handle_player_arcane_curse(pl);
    float curse = pl->GetActorValue(RE::ActorValue::kEnchantingSkillAdvance);
    if (curse > 100.f) gameplay::increase_arcane_curse(pl, 1.f);
    if (curse < 0) pl->ModActorValue(RE::ActorValue::kEnchantingSkillAdvance, -curse); // clamp
}


void on_my_update()
{
    mys::player->SetActorValue  (RE::ActorValue::kMood, mys::player->GetBaseActorValue(RE::ActorValue::kMagicka));
    mys::player->SetBaseActorValue (RE::ActorValue::kConfidence, mys::player->GetBaseActorValue(RE::ActorValue::kStamina)); // base av

    if (u_worn_has_keyword(mys::player, mys::armorHeavyKw))  mys::hasHeavyArmor = true;
    else  mys::hasHeavyArmor = false;
    apply_levelUp_bonuses();
    handle_cap_actorValues (mys::player);
    update_mass_effects(mys::player);
    handle_moveSpeed_compensation();
    handle_weaponBased_attack_speed();
    handle_numbers_widget ();
    handle_pl_arcane_curse (mys::player);
    update_exp (mys::player);
    mys::player->WornArmorChanged();  // for update armor % perk entries
    mys::attackKeyHolding = false;
    mys::xdescr_state = 0;
    mys::nb_hold_state = 0;

    if (mys::gameProcessed->value > 0)  // after player selected start
    {
        qst::check_class_achievements(mys::player);
        if (my::bossFightID.p->value > 0) {
             gameplay::bossFightHandle(my::bossFightID.p->value, mys::bossUpdMeter);
        }
        snowElf_checkEnch (mys::player);
        check_knuckles (mys::player);
    }

    if (my::twicedUpdate)  // every 2nd update
    {
        // ------- test ---------

        //auto alvorObj = RE::TESForm::LookupByID<RE::TESBoundObject>(0x13475);
        //auto alvorRef = u_place_at_me(mys::player, alvorObj, 1, false, false);
        //auto alvor = alvorRef->As<RE::Actor>();

        update_gamelog (mys::player);
        update_keycodes();
        update_oil (mys::player);
        handle_fister (mys::player);
        if (mys::player->HasPerk(my::nb_perk_1.p)) check_nb (mys::player);
        //my::vamp_state_glob->value = 0;
        my::vamp_state_glob.p->value = 0;
        my::vamp_C_tapState = 0;
        my::sameMutagensGlob.p->value = 0;

        //my::blockBashBlocker.p->value = 0;
        
    }
    my::twicedUpdate = !my::twicedUpdate;

    
    // u_log_actor_perk_entries(mys::player, RE::BGSPerkEntry::EntryPoint::kModIncomingDamage, "ModIncomingDamage");
    // u_log_actor_perk_entries(mys::player, RE::BGSPerkEntry::EntryPoint::kModAttackDamage, "ModAttackDamage");
    // u_log_actor_perk_entries(mys::player, RE::BGSPerkEntry::EntryPoint::kModPowerAttackDamage,
    // "ModPowerAttackDamage"); u_log_actor_perk_entries(mys::player,
    // RE::BGSPerkEntry::EntryPoint::kModTargetDamageResistance, "ModTargetDamageResistance");
}


//void on_micro_update()   // every 1 sec
//{ 
//     gameplay::gmpl_on_micro_update(); 
//}


//--------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------ fill most data on game loads (mainMenu) ---------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------
namespace my
{

    void fill_gamePointers()
    { 
        SKSE::log::info("TEST - called fill_gamePointers(), PtrsVec Size - {}", PtrsVec::getVec().size());  // TEST
        for (const auto& ptr : PtrsVec::getVec()) {
            ptr->init_ptr();   // init all game objects ptrs, use like some_spell.p
        }
        // sf
        sf_cloakEff_KW  = handler->LookupForm<RE::BGSKeyword>(0x1B5B7D, magick_esp);
        sf_descr = handler->LookupForm<RE::EffectSetting>(0x2773B0, magick_esp);
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
    }

    void fill_data()
    {
        bindGlobs.emplace_back(handler->LookupForm<RE::TESGlobal>(0xE74BA4, "RfaD SSE - Awaken.esp"));  // [0] - 56 - "Dodge"       // lAlt
        bindGlobs.emplace_back(handler->LookupForm<RE::TESGlobal>(0xE74BA5, "RfaD SSE - Awaken.esp"));  // [1] - 47 - "Bats"        // V
        bindGlobs.emplace_back(handler->LookupForm<RE::TESGlobal>(0xE74BA6, "RfaD SSE - Awaken.esp"));  // [2] - 46 - "Boil"        // C
        bindGlobs.emplace_back(handler->LookupForm<RE::TESGlobal>(0xE74BA7, "RfaD SSE - Awaken.esp"));  // [3] - 45 - "Adrenaline"  // X
        bindGlobs.emplace_back(handler->LookupForm<RE::TESGlobal>(0xE74BA8, "RfaD SSE - Awaken.esp"));  // [4] - 45 - "Twilight"    // X
        bindGlobs.emplace_back(handler->LookupForm<RE::TESGlobal>(0xE74BA9, "RfaD SSE - Awaken.esp"));  // [5] - 20 - "Sting"       // T
        bindGlobs.emplace_back(handler->LookupForm<RE::TESGlobal>(0xE74BAA, "RfaD SSE - Awaken.esp"));  // [6] - 19 - "Mirror"      // R
        bindGlobs.emplace_back(handler->LookupForm<RE::TESGlobal>(0xE74BAB, "RfaD SSE - Awaken.esp"));  // [7] - 22 - "Twin"        // U
        bindGlobs.emplace_back(handler->LookupForm<RE::TESGlobal>(0xE74BAC, "RfaD SSE - Awaken.esp"));  // [8] - 47 - "Cultist"     // V
        bindGlobs.emplace_back(handler->LookupForm<RE::TESGlobal>(0xE74BAD, "RfaD SSE - Awaken.esp"));  // [9] - 20 - "Race"        // T
        bindGlobs.emplace_back(handler->LookupForm<RE::TESGlobal>(0x2FD70A, "RfaD SSE - Awaken.esp"));  // [10] - 46 - "NB_blynk"   // C
        
        keyCodes = { 56, 47, 46, 45, 45, 20, 19, 22, 47, 20, 46 };   // default binds of instant abilities

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
        lastHiterName = "";
        lastTargetName = "";
        currentBossAutocast = nullptr;
        twicedUpdate = false;
        snowElf_re_chanted = false;
        meditateState = false;
        instantAbIndex = 0;
        rightHandKeyCode = 256;        // initial value = mouse left
        log_counter = 200;
        c_key = 33;
        exp_upd_counter = 0;
        waitStartGameHour = 0;
        vamp_C_holdState = 0;
        vamp_C_tapState = 0;
        pwatk_state = 0;
        x_desc::parseFile();        // read and save data from xDescriptions file
    }
    
    void fill_translation()            //  this plugin objs lang  RU / EN
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

#undef KINSTANT



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


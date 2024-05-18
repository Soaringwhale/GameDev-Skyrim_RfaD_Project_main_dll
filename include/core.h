
#pragma once

#include "RE/Skyrim.h"

template <typename T>
void init_ptr (T*& ptr, int address, const char* esp)								// unused now
{
    ptr = RE::TESDataHandler::GetSingleton()->LookupForm<T>(address, esp);
}

template<typename T>
class GamePtr
{ 
 public:
    T* ptr;
    int address;
    const char* esp;

	void init_ptr()  
    {
        ptr = RE::TESDataHandler::GetSingleton()->LookupForm<T>(address, esp);
    }
};

namespace my		 
{				
    static RE::TESDataHandler* handler;

    static RE::TESGlobal* vamp_state_glob;
    static RE::TESGlobal* nb_hitCounter;
    static RE::TESGlobal* nb_1_atkChoice;
    static RE::TESGlobal* nb_magicBlade;
    static RE::TESGlobal* atronachAbsorbChanceGlob;
    static RE::TESGlobal* snowElf_wears_EnchWeap;
      
    static RE::BGSPerk* snowElf_anabioz;
    static RE::BGSPerk* bowSlowTimePerk;

    static RE::BGSPerk *cult_vermina_1, *cult_meridia_1, *cult_nocturn_1, *cult_boethia_1, *cult_azura_1;
    static RE::SpellItem *boethiaFF, *verminaFF, *nocturnalFF, *meridiaFF;

    static RE::SpellItem* redguardFF;
    static RE::SpellItem* argonianFF;
    static RE::SpellItem* khajeetFF;

    static RE::BGSKeyword* arrowReflectKW;
    static RE::BGSKeyword* oil_keyword;
    static RE::EffectSetting* bindsInfo_eff;
    static RE::EffectSetting* meridia_kd;
    static RE::TESEffectShader* bloodBubbles_FX;
    static RE::TESEffectShader* bloodFire_FX;
    static RE::TESEffectShader* bleedShader_FX;
    static RE::SpellItem* become_bats;
    static RE::SpellItem* nb_main_holdFF;
    static RE::EffectSetting* bats_kd;
    static RE::EffectSetting* nb_fullReflect;
    static RE::EffectSetting* nb_mainEff;
    static RE::EffectSetting* nb_openMode;
    static RE::EffectSetting* nb_hold_slowTime;
    static RE::EffectSetting* nb_main_kd;
    static RE::EffectSetting* nb_mirror_kd;
    static RE::EffectSetting* nb_sting_kd;
    static RE::EffectSetting* nb_twin_kd;
    static RE::SpellItem* nb_sting;
    static RE::SpellItem* nb_sting_hold;
    static RE::SpellItem* vampirism;
    static RE::SpellItem* adrenaline_tap;
    static RE::SpellItem* nb_2_Twin_FF;
    static RE::EnchantmentItem* snowElfEnch;
    static RE::SpellItem* snowElf_insteadEnch;
    static RE::SpellItem* snowElf_raceAb;
    static RE::SpellItem* nb_Reflect;
    static RE::SpellItem* nb_mainInvisAb;
    static RE::SpellItem* adrenaline_hold;
    static RE::SpellItem* bloodBrand;
    static RE::SpellItem* bloodRiversAB;
    static RE::SpellItem* bloodThirstAB;
    static RE::SpellItem* bloodThirstHoldAB;
    static RE::SpellItem* bloodBoil_HP_Buff;
    static RE::SpellItem* doomAtronach;
    static RE::EffectSetting* bloodThirst_kd;
    static RE::EffectSetting* bloodRivers_kd;
    static RE::EffectSetting* bloodRivers_allow;
    static RE::EffectSetting* batForm_eff;
    static RE::EffectSetting* arrow_reflect;
    static RE::BGSSoundDescriptorForm* slashSound;
    static RE::BGSSoundDescriptorForm* vampireHeartBeat;
    static RE::BGSKeyword* dodgeEffectKW;
    static RE::EffectSetting* manaShield_2;
    static RE::EffectSetting* manaShield_4_sf;
    static RE::EffectSetting* injureEff;
    static RE::EffectSetting* adrenalineKD;
    static RE::EffectSetting* sf_descr;
    static RE::SpellItem* injureSpell;
    static RE::BGSKeyword* bindKeyKw;
    static RE::BGSKeyword* breakableBowKW;
    static RE::BGSKeyword* snowElfEnchKW;
    static RE::BGSKeyword* actorDwarven;
    static RE::BGSKeyword* allow_fx_kw;
    static RE::SpellItem* breakBowSpl;
    static RE::SpellItem* stressSpell;
    static RE::SpellItem* stressStart;
    static RE::SpellItem* stressDispel;
    static RE::SpellItem* zoomStress;
    static RE::SpellItem* atkSpeedDebuff;
    static RE::SpellItem* doomSteed;
    static RE::SpellItem* lvlup_hp_bonuses;
    static RE::SpellItem* lvlup_st_bonuses;
    static RE::SpellItem* lvlup_mp_bonuses;
    static RE::SpellItem* bash_kd_self;
    static RE::SpellItem* block_anim_blocker;  
    static RE::BGSKeyword* magicSlow_KW;
    static RE::BGSKeyword* sword_1h;
    static RE::BGSKeyword* alch_heal_KW;
    static RE::BGSKeyword* dll_check_KW;
    static RE::BGSKeyword* lvlup_bonuses_KW;
    static RE::BGSKeyword* sf_effect_KW;
    static RE::BGSKeyword* sf_dispel_KW;
    static RE::BGSKeyword* slash_1;
    static RE::BGSKeyword* slash_2;
    static RE::BGSKeyword* slash_3;
    static RE::BGSKeyword* slash_4;
    static RE::BGSKeyword* slash_5;
    static RE::BGSPerk* gm_lvlup_bonuses;
    static RE::BGSPerk* paladinCenter;
    static RE::BGSPerk* palCenterBoost;
    static RE::BGSPerk* chihMeridiaShield;
    static RE::BGSPerk* swordFocus1;
    static RE::BGSPerk* swordFocus2;
    static RE::BGSPerk* swordFocus3;
    static RE::BGSPerk* sf_perk_3;
    static RE::BGSPerk* vamp_left2_perk;
    static RE::BGSPerk* vamp_1st_perk;
    static RE::BGSPerk* nb_perk_1;
    static RE::BGSPerk* nb_perk_2;
    static RE::BGSPerk* bossFightDebuff;
    static RE::SpellItem*  speedCap_regulator;
    static RE::SpellItem*  absorbCap_regulator;
    static RE::SpellItem*  shoutCap_regulator;
    static RE::BGSKeyword* regulator_eff;
    static RE::BGSMessage* msgBoxDodge;
    static RE::BGSMessage* msgBoxAbils;

    static RE::BGSPerk* evasion_1_perk;
    static RE::BGSPerk* heavy_1_perk;
    static RE::BGSPerk* heavy_25_perk;
    static RE::BGSPerk* heavy_75_perk;
    static RE::BGSPerk* heavy_50_perk;
    static RE::BGSPerk* heavy_100_perk;
    static RE::BGSPerk* heavy_sprint_perk;

    static RE::SpellItem* mass_penalty_speed;
    static RE::SpellItem* mass_penalty_noise;
    static RE::SpellItem* mass_buff_mass;
    static RE::SpellItem* mass_buff_infamy;

    static RE::TESGlobal *glob_destr_1, *glob_destr_2, *glob_destr_3, *glob_alter_1, *glob_alter_2, *glob_alter_3;
    static RE::SpellItem *sf_speed_const, *sf_penet_const, *sf_armor_const, *sf_rflct_const, *sf_absrb_const, *sf_stamina_const;

    static RE::BGSKeyword* sf_cloakEff_KW;

    static uint32_t  instantAbIndex;
    static uint32_t  rightHandKeyCode;
    static int log_counter;
    static float waitStartGameHour;
    static std::string  sf_noEffects, sf_rem_current, sf_add_new, sf_removed, sf_all_clear, sf_speed_text,
					    sf_penetr_text, sf_armor_text, sf_reflect_text, sf_absorb_text, sf_stamina_text,
						adrenaline_text, adrenalineMax_text, oil_decline_text;
        
    static bool new_hooks_installed;
    static bool twicedUpdate;
       
    void initObjects();
    void install_new_hooks();
    void sf_handle_reserved_MP();
    void sf_handle(RE::ActiveEffect *eff, RE::EffectSetting *baseEff);
    void fill_gamePointers();
    void fill_data();
    void fill_translation();
    void on_wait();

    static RE::ArrowProjectile* reflectedArrow;
};


namespace x_desc  // for description module
{
    void parseFileLine(std::string line);
    void parseFile();
};

struct mys						//  variables that can be used in all project files
{
    static float time_delta;
    static float reserved_MP;
    static float ms_compensator;
    static float eq_weight;
    static float dodge_timer;
    static uint16_t vamp_C_state;
    static uint16_t nb_hold_state;
    static uint16_t xdescr_state;
    static bool  xdescr_on;
    static bool  hasHeavyArmor;
    static bool  attackKeyHolding;
       
    static  RE::Actor  *player;

    static RE::BGSKeyword	 *armorHeavyKw;
    static RE::EffectSetting *dodge_KD_eff;
    static RE::SpellItem     *dodge_KD_spl;
    static RE::BGSPerk	     *dodgePerk;
    static RE::TESGlobal	 *speed_cast_glob;
    static RE::TESGlobal     *gameProcessed;

    static void init_globs();
    static void handle_keyPress(uint32_t keyCode, float hold_time, bool is_up);

};


static const char* magick_esp = "Requiem for a Dream - Magick.esp";

float allOnHitEffects(RE::Actor* target, RE::HitData hit_data);

void player_anim_handle(RE::BSTEventSink<RE::BSAnimationGraphEvent>*, RE::BSAnimationGraphEvent*, RE::BSTEventSource<RE::BSAnimationGraphEvent>*);

float on_resist_apply(RE::MagicTarget* this_, RE::MagicItem* magic_item, const RE::Effect* effect, const RE::TESBoundObject* bound_object);

bool on_drink_potion(RE::Actor* actor, RE::AlchemyItem* potion, RE::ExtraDataList* extra_data_list);

void on_adjust_active_effect(RE::ActiveEffect* this_, float power, bool &unk);

bool on_arrow_collide(RE::ArrowProjectile* arrow, RE::hkpAllCdPointCollector* collidable);

bool on_meleeWeap_collide (RE::Actor* attacker, RE::Actor* victim);

void on_equip(RE::Actor* actor, RE::TESBoundObject* object);

void on_unequip(RE::Actor* actor, RE::TESBoundObject* object);

void on_death (RE::Actor* victim, RE::Actor* killer);

void on_wait_menu_open();
void on_wait_menu_close();
void on_inventory_open();
void on_inventory_close();

void on_valueMod_Effect_Finish(RE::ValueModifierEffect* modEff);

void on_item_card_upd (RE::ItemCard* itemCard, RE::TESForm* item);

void on_apply_poison (RE::InventoryEntryData* data, RE::AlchemyItem* poison, int count);

//bool on_arrow_collide(RE::Projectile* proj, RE::hkpCollidable* coll); 

void on_my_update();

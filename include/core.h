
#pragma once

#include "RE/Skyrim.h"
#include "GamePtr.h"


namespace my         
{                
    static RE::TESDataHandler* handler;

    static const char* magick_esp = "Requiem for a Dream - Magick.esp";

    //static RE::TESGlobal* vamp_state_glob;
    //static RE::TESGlobal* nb_hitCounter;

    //static GamePtr<RE::TESGlobal>  silver_oil_active (0x5BDDE3, "RfaD SSE - Awaken.esp");

    //static GamePtr<RE::TESObjectMISC> additemMenuPack (0x690D, "AddItemMenuSE.esp");

    static GamePtr<RE::SpellItem> deleteSavesEnabled (0x97C5DD, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::SpellItem> logEnabled (0x874CA5, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::EffectSetting> logEnabler (0x874CA4, "RfaD SSE - Awaken.esp");

    static GamePtr<RE::TESGlobal> xp_mult (0x8C0CEA, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::TESGlobal> xp_points (0x800, "Requiem for a Dream - Experience.esp");

    static GamePtr<RE::EffectSetting> quen_barrier (0x74EA96, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::SpellItem> quen_expl (0x758CD6, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::TESGlobal> quen_convertRate (0x753BC6, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::SpellItem> kd_on_self (0x758CD9, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::SpellItem> quen_onHitSoundVisual (0x75DDFE, "RfaD SSE - Awaken.esp");

    static GamePtr<RE::TESObjectWEAP> myUnarmed (0x8B6ABC, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::SpellItem> handFocus1 (0x8421D0, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::SpellItem> handFocus2 (0x8421D2, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::SpellItem> handFocus3 (0x8421D4, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::TESGlobal> bashImmunChance   (0x8472EF, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::TESGlobal> pureStaggerChance (0x8472EE, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::SpellItem>  knuckles_reequip (0x8421D6, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::BGSKeyword> knuckles_injuring(0x86FB92, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::BGSKeyword> knuckles_bleed   (0x893326, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::BGSKeyword> knuckles_blunt   (0x893327, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::BGSKeyword> knuckles_kw (0x80A56B, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::BGSArtObject> fister_white_spray(0x879DB9, "RfaD SSE - Awaken.esp");

    static GamePtr<RE::BGSKeyword> decreaseDmg_kw      (0x274E5E, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::BGSKeyword> decreasePenetr_kw   (0x525DFB, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::BGSKeyword> decreaseAtkSpeed_kw (0x6CAAE3, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::BGSKeyword> decreaseMagRes_kw   (0xB20FEB, "RfaD SSE - Awaken.esp");

    static GamePtr<RE::BGSKeyword> weapMaterialSilver (0x10AA1A);  // Skyrim.esm
    static GamePtr<RE::BGSKeyword> silverdust_weap_kw (0x5BDDE4, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::EffectSetting> oil_silver_eff (0xCFAEC, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::BGSKeyword> oil_remover (0x83D0AF, "RfaD SSE - Awaken.esp");

    static GamePtr <RE::SpellItem> blockBlockerSpell (0x3E3FDA, "devFixes.esp");
    static GamePtr<RE::EffectSetting> blockBlockerEFf(0x3E3FDC, "devFixes.esp");
    static GamePtr <RE::TESGlobal> blockBashBlocker (0xA799C9, "devFixes.esp");
    static GamePtr <RE::TESGlobal> dodgeType (0x8AC88C, "RfaD SSE - Awaken.esp");
    static GamePtr <RE::TESGlobal> vamp_state_glob (0xB960D3, "RfaD SSE - Awaken.esp");
    static GamePtr <RE::TESGlobal> nb_hitCounter     (0xC9D865, "RfaD SSE - Awaken.esp");
    
    static GamePtr <RE::SpellItem> redguardBuffDmg   (0x27F080, "RfaD SSE - Awaken.esp");

    static GamePtr<RE::SpellItem>     nb_blynk (0x2F31D5, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::SpellItem>     nb_blynk_hold (0x2F85F6, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::BGSArtObject>  nb_blynk_fx (0x2EE0C4, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::EffectSetting> nb_blynk_kd (0x353B55, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::EffectSetting> beast_ring_eff(0xB2F0AD, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::EffectSetting> bm_cascade_conc (0x3772CE, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::SpellItem>     cascade_debuffs_1     (0x39AA67, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::SpellItem>     cascade_debuffs_2     (0x39AA69, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::EffectSetting> decr_magres_self_eff1 (0x39AA66, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::EffectSetting> decr_magres_self_eff2 (0x39AA6B, "RfaD SSE - Awaken.esp");

    static GamePtr<RE::EffectSetting> templarComboEff (0x40A217, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::BGSSoundDescriptorForm> templarBeamStand (0x3FFFF3, "RfaD SSE - Awaken.esp");

    static GamePtr<RE::BGSKeyword> pwAtkTypeStanding (0x914E5);    // Skyrim.esm

    // for tesak only, mb delete later
    static GamePtr<RE::SpellItem> templarSwordFX (0x40A215, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::BGSKeyword> chrysamereKW  (0x74D69, "RfaD SSE - Awaken.esp");

    static GamePtr<RE::TESGlobal> snowElf_wears_EnchWeap(0xC2E0F0, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::TESGlobal> nb_1_atkChoice(0xCDF64F, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::TESGlobal> nb_magicBlade(0xDF100A, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::TESGlobal> atronachAbsorbChanceGlob(0x87A67A, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::TESGlobal> bossFightID(0x1C7FFB, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::SpellItem> redguardFF(0x284194, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::SpellItem> argonianFF(0x3A04AC, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::SpellItem> khajeetFF(0xEB1866, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::SpellItem> bloodBrand(0x238167, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::SpellItem> bloodBoil_HP_Buff(0xB72963, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::EffectSetting> bloodBrandEf (0x238169, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::EffectSetting> bloodBrandNovaEf (0xB72961, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::TESEffectShader> bloodBubbles_FX(0xBAE83, "Apocalypse - Magic of Skyrim.esp");
    static GamePtr<RE::TESEffectShader> bloodFire_FX(0xBFAC0, "Apocalypse - Magic of Skyrim.esp");
    static GamePtr<RE::TESEffectShader> bleedShader_FX(0x9FC16, "Apocalypse - Magic of Skyrim.esp");
    static GamePtr<RE::BGSPerk> heliotrope (0xA14F7A, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::EffectSetting> longStrideEff (0x6499, "Apocalypse - Magic of Skyrim.esp");
    static GamePtr<RE::SpellItem> nb_2_Twin_FF(0xD72531, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::SpellItem> nb_main_holdFF(0xDD298F, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::SpellItem> snowElf_insteadEnch(0xC2E0EE, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::BGSPerk> snowElf_anabioz(0x800C10, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::SpellItem> snowElf_raceAb(0x7D3169, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::SpellItem> nb_sting(0xDAF204, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::SpellItem> nb_sting_hold (0xDB942B, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::SpellItem> nb_Reflect (0xCA7A92, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::SpellItem> nb_mainInvisAb (0xC9D866, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::EffectSetting> nb_mainEff(0xCA2977, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::EffectSetting> nb_hold_slowTime(0xDD298E, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::EffectSetting> nb_openMode(0xCA297D, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::EffectSetting> nb_mirror_kd(0xDF100B, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::EffectSetting> nb_sting_kd(0xEA2537, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::EffectSetting> nb_twin_kd(0xEA2538, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::EffectSetting> nb_main_kd(0xDD2991, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::EffectSetting> bindsInfo_eff(0xE5143A, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::EnchantmentItem> snowElfEnch(0x7FBAE8, "RfaD SSE - Awaken.esp");

    static GamePtr<RE::BGSKeyword> actorBoss(0x1C7FF6, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::BGSKeyword> dodgeEffectKW(0x7B4A92, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::BGSKeyword> arrowReflectKW(0xECADB8, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::BGSKeyword> bindKeyKw(0xE2DCC6, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::EffectSetting> manaShield_2(0x535A3E, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::EffectSetting> nb_fullReflect(0xCA7A98, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::EffectSetting> manaShield_4_sf(0xAE9C9C, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::EffectSetting> injureEff(0x9F9FF2, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::EffectSetting> projReflectAutocast (0xC00732, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::EffectSetting> arrow_reflect (0xBB4732, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::EffectSetting> adrenalineKD(0x897552, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::SpellItem> adrenaline_tap(0xA5A416, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::SpellItem> adrenaline_hold(0xBB9846, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::SpellItem> bloodThirstAB(0xB4A0CB, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::SpellItem> bloodThirstHoldAB(0xB4F1E0, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::EffectSetting> bloodThirst_kd(0xB4F1E3, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::SpellItem> injureSpell(0x3C3C2C, "RfaD SSE - Awaken.esp");  // injure_dmg_spell
    static GamePtr<RE::BGSKeyword> breakableBowKW(0x2E232E, "Requiem.esp");
    static GamePtr<RE::BGSKeyword> slowTimeExclude(0x250D70, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::BGSKeyword> lvlup_bonuses_KW(0x3833C5, "devFixes.esp");
    static GamePtr<RE::SpellItem> breakBowSpl(0x7637F8, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::SpellItem> stressSpell(0xA5E9F, "devFixes.esp");        // attack stress FF
    static GamePtr<RE::SpellItem> stressStart(0xBF603, "devFixes.esp");        // bow stress
    static GamePtr<RE::SpellItem> stressDispel(0xBF607, "devFixes.esp");       // stress dispel
    static GamePtr<RE::SpellItem> zoomStress(0x64C2C9, "devFixes.esp");        // zoom stress
    static GamePtr<RE::SpellItem> atkSpeedDebuff(0x2AD599, "devFixes.esp");    // player attack speed handle
    static GamePtr<RE::SpellItem> lvlup_hp_bonuses(0xFB49, "devFixes.esp");    //
    static GamePtr<RE::SpellItem> lvlup_st_bonuses(0x388540, "devFixes.esp");  //
    static GamePtr<RE::SpellItem> lvlup_mp_bonuses(0xFB3F, "devFixes.esp");    //    lvlup bonuses
    static GamePtr<RE::BGSPerk> gm_lvlup_bonuses(0x4899DC, "devFixes.esp");    //
    static GamePtr<RE::SpellItem> bash_kd_self(0x3E3FDD, "devFixes.esp");
    static GamePtr<RE::SpellItem> block_anim_blocker(0x3E3FDA, "devFixes.esp");
    static GamePtr<RE::SpellItem> exp_update (0xCE6E1D, "devFixes.esp");
    static GamePtr<RE::SpellItem> become_bats(0x19FD4, "Requiem for a Dream - Kelpie.esp");
    static GamePtr<RE::EffectSetting> batForm_eff(0x5949, "Requiem for a Dream - Kelpie.esp");

    static GamePtr<RE::SpellItem> olve_training(0x3E17F4, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::SpellItem> seph_training(0x98877, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::SpellItem> seph_training2(0x4CECF, "refrain_DeadLands.esp");

    static GamePtr<RE::SpellItem> oil_after_use(0x23C924, "RfaD SSE - Awaken.esp");

    static GamePtr<RE::EffectSetting> race_ab_kd(0x284198, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::EffectSetting> bats_kd(0x7DBDF8, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::BGSMessage> msgBoxDodge(0xB542F4, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::BGSMessage> msgBoxAbils(0xE83EDB, "RfaD SSE - Awaken.esp");

    static GamePtr<RE::BGSKeyword> slash_1(0xAD3965, "Requiem.esp");
    static GamePtr<RE::BGSKeyword> slash_2(0xAD3964, "Requiem.esp");
    static GamePtr<RE::BGSKeyword> slash_3(0xAD3963, "Requiem.esp");
    static GamePtr<RE::BGSKeyword> slash_4(0xAD3962, "Requiem.esp");
    static GamePtr<RE::BGSKeyword> slash_5(0xAD3A24, "Requiem.esp");
    static GamePtr<RE::BGSKeyword> DmgType_Pierce (0xAD3956, "Requiem.esp");
    static GamePtr<RE::BGSKeyword> DmgType_Blunt  (0xAD3957, "Requiem.esp");
    static GamePtr<RE::BGSPerk> bossFightDebuff(0xF67E9F, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::BGSKeyword> dll_check_KW(0x9D823A, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::BGSKeyword> longstride_KW (0x6798A2, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::BGSKeyword> sf_effect_KW  (0x13B7BE, magick_esp);
    static GamePtr<RE::BGSKeyword> sf_dispel_KW  (0x14093E, magick_esp);
    static GamePtr<RE::BGSPerk> sf_perk_3(0x2C8B53, magick_esp);
    static GamePtr<RE::BGSPerk> palCenterBoost(0x80F, "devFixes.esp");
    static GamePtr<RE::BGSPerk> paladinCenter(0xFB4F, "Requiem for a Dream - DivineBlessings.esp");

    static GamePtr<RE::BGSPerk> temp_1 (0x54E7F, "ChihSkillTree.esp");  // temp, remove when changes
    static GamePtr<RE::BGSPerk> temp_2 (0x2EC9AE,"ChihSkillTree.esp");  //
    static GamePtr<RE::BGSPerk> chihMeridiaShield(0x131535, "ChihSkillTree.esp");
    static GamePtr<RE::BGSPerk> vamp_left2_perk(0x54E96, "ChihSkillTree.esp");
    static GamePtr<RE::BGSPerk> vamp_1st_perk(0x59FDA, "ChihSkillTree.esp");

    static GamePtr<RE::BGSPerk> cult_vermina_1(0xCED17, "ChihSkillTree.esp");
    static GamePtr<RE::BGSPerk> cult_boethia_1(0x54EB3, "ChihSkillTree.esp");
    static GamePtr<RE::BGSPerk> cult_meridia_1(0x54E81, "ChihSkillTree.esp");
    static GamePtr<RE::BGSPerk> cult_nocturn_1(0x54E7E, "ChihSkillTree.esp");
    static GamePtr<RE::BGSPerk> cult_azura_1(0x54E7F, "ChihSkillTree.esp");  //  cultist perks

    static GamePtr<RE::SpellItem> boethiaFF(0xD18C2, "Ch1h_MeridiaPatch.esp");  // meridia_patch.esp
    static GamePtr<RE::SpellItem> verminaFF(0xCED1A, "ChihSkillTree.esp");
    static GamePtr<RE::SpellItem> nocturnalFF(0x91BAD, "ChihSkillTree.esp");  // skillTree.esp
    static GamePtr<RE::SpellItem> meridiaFF(0x54EB8, "ChihSkillTree.esp");
    static GamePtr<RE::EffectSetting> meridia_kd(0x14ED3, "ChihSkillTree.esp");

    static GamePtr<RE::TESQuest> mercenaryQst (0xF1CF, "RfaD SSE - Awaken.esp");

    static GamePtr<RE::EffectSetting> bossFightStarter(0x1C7FF9, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::BGSKeyword> oil_keyword(0xCFAED, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::BGSKeyword> snowElfEnchKW(0x99B51C, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::BGSKeyword> allow_fx_kw(0x664F36, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::SpellItem> speedCap_regulator(0x5B3671, "devFixes.esp");
    static GamePtr<RE::SpellItem> absorbCap_regulator(0x5BD967, "devFixes.esp");
    static GamePtr<RE::SpellItem> shoutCap_regulator(0x5BD96A, "devFixes.esp");
    static GamePtr<RE::BGSKeyword> regulator_eff(0x5C2AE8, "devFixes.esp");
    static GamePtr<RE::BGSPerk> falmerSetConvertPerk (0xD1EE77, "devFixes.esp");

    static GamePtr<RE::SpellItem> mass_penalty_speed(0x5A41F7, "devFixes.esp");
    static GamePtr<RE::SpellItem> mass_penalty_noise(0x5A41FB, "devFixes.esp");
    static GamePtr<RE::SpellItem> mass_buff_mass(0x5A41FE, "devFixes.esp");
    static GamePtr<RE::SpellItem> mass_buff_infamy(0x5A4200, "devFixes.esp");

    static GamePtr<RE::BGSPerk> heavy_1_perk2 (0x9A8D38, "devFixes.esp");

    static GamePtr<RE::BGSKeyword> DLC1VampireDrainEffect   (0xD840, "Dawnguard.esm");
    static GamePtr<RE::BGSKeyword> Mirel_RBB_KW_AbsorbSpell (0x2029B9, "Requiem - Breaking Bad.esp");

    static GamePtr<RE::TESGlobal>  sameMutagensGlob (0xC1949E, "RfaD SSE - Awaken.esp");
    static GamePtr<RE::BGSKeyword> mutagen_kw (0x81D, "Alchemy.esp");

    //static GamePtr<RE::SpellItem> rbb_univ_spell (0x2FAD46, "Requiem - Breaking Bad.esp");

    //---------------------------------------------------------------------------------------- skyrim.esm -----
    static GamePtr<RE::BGSKeyword> MagicVampireDrain (0x101BDE);
    static GamePtr<RE::BGSKeyword> actorDwarven(0x1397A);    //
    static GamePtr<RE::BGSKeyword> alch_heal_KW(0x42503);    //
    static GamePtr<RE::BGSKeyword> magicSlow (0xB729E);    //
    static GamePtr<RE::BGSKeyword> magicShout (0x46B99);     //
    static GamePtr<RE::BGSKeyword> bound1H (0x510BE);        //
    static GamePtr<RE::BGSKeyword> bound2H (0x84D1D);        // 
    static GamePtr<RE::BGSKeyword> boundBow(0x28A42);        //
    static GamePtr<RE::BGSKeyword> sword_1h(0x1E711);        //
    static GamePtr<RE::BGSKeyword> vendorItemHide (0x914EA);    //
    static GamePtr<RE::BGSKeyword> vendorItemWeapon (0x8F958);  //
    static GamePtr<RE::BGSKeyword> armorShield (0x965B2);       //
    static GamePtr<RE::SpellItem> vampirism(0xED0A8);           //
    static GamePtr<RE::SpellItem> doomSteed (0xE5F5E);          //
    static GamePtr<RE::SpellItem> doomLady (0xE5F54);           //
    static GamePtr<RE::SpellItem> doomAtronach (0xE5F51);       //
    static GamePtr<RE::BGSPerk> swordFocus1(0x5F56F);           //
    static GamePtr<RE::BGSPerk> swordFocus2(0xC1E90);        //   } skyrim.esm
    static GamePtr<RE::BGSPerk> swordFocus3(0xC1E91);        //
    static GamePtr<RE::BGSPerk> bowSlowTimePerk(0x103ADA);   //
    static GamePtr<RE::BGSPerk> evasion_1_perk(0xBE123);     //
    static GamePtr<RE::BGSPerk> heavy_1_perk  (0xBCD2A);     //
    static GamePtr<RE::BGSPerk> heavy_25_perk(0x58F6F);      //
    static GamePtr<RE::BGSPerk> heavy_50_perk(0x58F6C);      //
    static GamePtr<RE::BGSPerk> heavy_75_perk(0x107832);     //
    static GamePtr<RE::BGSPerk> heavy_100_perk(0x105F33);    //
    static GamePtr<RE::BGSPerk> heavy_sprint_perk(0x7935E);  //
    static GamePtr<RE::BGSPerk> nb_perk_1(0x153D0);          //
    static GamePtr<RE::BGSPerk> nb_perk_2(0x59B78);          //
    static GamePtr<RE::BGSSoundDescriptorForm> slashSound (0xDAB81);        //
    static GamePtr<RE::BGSSoundDescriptorForm> vampireHeartBeat (0xFF9E8);  //
    // bestiary checks
    static GamePtr<RE::TESRace> FalmerRace  (0x131F4);
    static GamePtr<RE::TESRace> SprigganRace(0x13204); 
    static GamePtr<RE::TESRace> TrollRace   (0x13205);
    static GamePtr<RE::TESRace> TrollFrostRace (0x13206);
    static GamePtr<RE::TESRace> ChaurusRace (0x131EB);
    static GamePtr<RE::TESRace> ChaurusReaperRace (0xA5601);
    static GamePtr<RE::TESRace> FrostbiteSpiderRace      (0x131F8);
    static GamePtr<RE::TESRace> FrostbiteSpiderRaceGiant (0x4E507);  // spider fact contain many creatures, so use races
    static GamePtr<RE::TESRace> FrostbiteSpiderRaceLarge (0x53477);
    static GamePtr<RE::TESRace> DraugrRace (0xD53);
    static GamePtr<RE::TESRace> WolfRace           (0x1320A);  // wolf fact contain nix-hound etc, so use race
    static GamePtr<RE::TESFaction> BearFaction     (0xFBBF3);  // bear races are x3, so use fact
    static GamePtr<RE::TESFaction> SabreCatFaction (0x435A2);
    static GamePtr<RE::BGSKeyword> isBeastRace (0xD61D1);
    // static GamePtr<RE::TESRace> WerewolfBeastRace (0xCDD84);
    
    //============================================================================================================= skyrim.esm ====

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
   
    static RE::EffectSetting* sf_descr;
    static RE::TESGlobal *glob_destr_1, *glob_destr_2, *glob_destr_3, *glob_alter_1, *glob_alter_2, *glob_alter_3;
    static RE::SpellItem *sf_speed_const, *sf_penet_const, *sf_armor_const, *sf_rflct_const, *sf_absrb_const, *sf_stamina_const;
    static RE::BGSKeyword* sf_cloakEff_KW;

    static uint16_t vamp_C_holdState, vamp_C_tapState, pwatk_state;
    static uint32_t instantAbIndex;
    static uint32_t rightHandKeyCode;
    static int log_counter, exp_upd_counter, c_key;
    static RE::SpellItem *currentBossAutocast;
    static float waitStartGameHour;
    static std::string  sf_noEffects, sf_rem_current, sf_add_new, sf_removed, sf_all_clear, sf_speed_text,
                        sf_penetr_text, sf_armor_text, sf_reflect_text, sf_absorb_text, sf_stamina_text,
                        adrenaline_text, adrenalineMax_text, oil_decline_text;

    static std::string lastHiterName, lastTargetName;
        
    static bool twicedUpdate;
    static bool snowElf_re_chanted;
       
    void initGameData();
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

struct mys                        //  variables that can be used in all project files
{
    static float time_delta;
    static float micro_timer;
    static float reserved_MP;
    static float ms_compensator;
    static float eq_weight;
    static float dodge_timer;
    static uint16_t nb_hold_state;
    static uint16_t xdescr_state;
    static uint16_t bossUpdMeter;
    static bool  xdescr_on;
    static bool  hasHeavyArmor;
    static bool  attackKeyHolding;
       
    static  RE::Actor  *player;
    static  RE::UI     *ui;

    static RE::BGSKeyword     *armorHeavyKw;
    static RE::EffectSetting *dodge_KD_eff;
    static RE::SpellItem     *dodge_KD_spl;
    static RE::BGSPerk         *dodgePerk;
    static RE::TESGlobal     *speed_cast_glob;
    static RE::TESGlobal     *gameProcessed;

    static void init_globs();
    static void handle_keyPress(uint32_t keyCode, float hold_time, bool is_up, bool is_held);

};

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

void log_game_info (RE::Actor* pl, bool load = false, bool death = false, RE::Actor* killer = nullptr, bool refresh = false);

void log_pre_load_game (RE::Actor* pl);

void on_valueMod_Effect_Finish(RE::ValueModifierEffect* modEff);

void on_item_card_upd (RE::ItemCard* itemCard, RE::TESForm* item);

void on_apply_poison (RE::InventoryEntryData* data, RE::AlchemyItem* poison, int &count);

//bool on_arrow_collide(RE::Projectile* proj, RE::hkpCollidable* coll); 

void on_my_update();
//void on_micro_update();

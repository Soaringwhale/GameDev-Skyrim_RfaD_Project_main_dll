
// SYNCED ++++

#pragma once

#include "utils.h"
#include "gameplay.h"
#include "core.h"

static std::vector<RE::SpellItem*> smalls;
static std::vector<RE::SpellItem*> meds;     // mutagens
static std::vector<RE::SpellItem*> bigs;

static std::map<RE::FormID, RE::BGSKeyword*> bestiary_races_map;

enum BossFight  // no need enum class, we want to compare with int, and don't want to use BossFight::
{
    OlvePre2stage = 101,
    Olve1stage = 1,
    Olve2stage = 2,
    Sephiroth = 3
};

namespace gameplay 
{

   // scale only [ef] magnitude here (charges remain and alchItem keywords - improved/pure) 
   // alchemy perks scales goes from ck, as well as duration scales
   void cast_oil (RE::SpellItem* spell, RE::Actor* target, RE::Actor* pl, RE::AlchemyItem* oil, float ef, float magnOverride)
   {
       float magn = spell->effects[0]->GetMagnitude() * ef;  // all magnitudes in spell must be the same, because we use override
       if (magnOverride > 0) magn = magnOverride;
       //LOG("eff name - {}, effect magn - {}, ef - {}, total magn - {}", spell->effects[0]->baseEffect->fullName, spell->effects[0]->GetMagnitude(), ef, magn);
       pl->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)->CastSpellImmediate(spell, false, target, 1.f, false, magn, pl);
   }

   void knuckles_block (RE::Actor* agr, RE::Actor* target, float& damage, float blockMod) {

       if (target->HasSpell(knuckles_combo_ON.p)) {
           u_cast(knuckles_blockedMarkerSelf.p, target, agr);
       }
       u_playSound (target, bluntVsOther.p, 1.f);
       damage *= (1 - blockMod / 200.f);
   }

   void set_univ_eff (int id, float mag, uint32_t dur)  // knuckles univ spell
   {
       knuckles_univ.p->effects[id]->effectItem.magnitude = mag;
       knuckles_univ.p->effects[id]->effectItem.duration = dur;
   }

   void knuckles_hit (RE::Actor* agr, RE::Actor* target, RE::TESObjectARMO *hands, float &damage, bool isPwAtk, bool isLeft)
   { 
       LOG("called knuckles_hit()");

       int32_t price = hands->value; // base price

	   auto st = agr->GetActorValue(RE::ActorValue::kStamina);
       if      (agr->HasPerk(unarmed_80.p)) damage *= (1 + (st * 0.004f));  // unarmed perks stamina-scaling
       else if (agr->HasPerk(unarmed_50.p)) damage *= (1 + (st * 0.003f));
       else if (agr->HasPerk(unarmed_20.p)) damage *= (1 + (st * 0.002f));

	   if (!isLeft && isPwAtk) damage *= 2.f;  // right hand pwAtk

       if (hands->HasKeyword(knuckles_bleeding.p)) {
           fish_bleed.p->effects[0]->effectItem.magnitude = price/100;  // bleed
           u_cast (fish_bleed.p, target, agr);
       }
       else if (hands->HasKeyword(knuckles_blunt.p))  set_univ_eff (1, price/200, 12);  // armor %

       if (hands->HasKeyword(knuckles_uniq.p))
       {
           if      (hands->HasKeyword(armorHide.p))     set_univ_eff (2, 15.f, 1); // st
           else if (hands->HasKeyword(armorScale.p))    set_univ_eff (7, 15.f, 5); // pois
           else if (hands->HasKeyword(armorGlass.p))    set_univ_eff (4, 25.f, 3); // shock
           else if (hands->HasKeyword(armorElven.p))    set_univ_eff (3, 30.f, 3); // fire
           else if (hands->HasKeyword(armorOrcit.p))    set_univ_eff (2, 20.f, 1); // st
           else if (hands->HasKeyword(sigvaldSet.p))    set_univ_eff (0, 25.f, 2); // white flame
           else if (hands->HasKeyword(armorLeather.p)) {set_univ_eff (5, 35.f, 2); set_univ_eff (6, 3.f, 10);}
       }
       u_cast (knuckles_univ.p, target, agr);
      //.......................combo.......................
       if (!isPwAtk) {
           u_cast(knuckles_jab_spell.p, target, agr);
           if (agr->HasMagicEffect(knuckles_afterBlockTimerSelf.p)) {  // hit just afer block
               RE::DebugNotification("Контрудар");
               u_playSound(target, bluntVsOther.p, 1.f);
               u_cast(knuckles_jab_spell.p, target, agr);
               if ((rand() % 10) > 5) {
                   target->SetGraphVariableFloat("StaggerMagnitude", 0.5f);
                   target->NotifyAnimationGraph("StaggerStart");
               }
           }
       }
       auto jabs = u_same_activeEffects_count(target, knuckles_jab.p);   // how many jabs on target
       if (jabs > 0) {
          if (isPwAtk) {  
              damage *= (1.f + (jabs * 0.15f));     //  pwatks after jabs
          }
       }

      //________DEBUG________________________________________________________________________
      //float atkDmgMult = 1.f, pwAtkDmgMult = 1.f, target_overcap = 1.f;    
      // прогоняем 1.f через ентри перков агрессора, что-бы узнать суммарные мульты. При этом проверяются кондишены, и мы получаем реальный мульт
      //RE::BGSEntryPoint::HandleEntryPoint (RE::BGSEntryPoint::ENTRY_POINT::kModIncomingDamage, target, agr, my::myUnarmed.p, std::addressof(target_overcap));
      //RE::BGSEntryPoint::HandleEntryPoint (RE::BGSEntryPoint::ENTRY_POINT::kModAttackDamage, agr, my::myUnarmed.p, target, std::addressof(atkDmgMult));
      //if (isPwAtk) RE::BGSEntryPoint::HandleEntryPoint (RE::BGSEntryPoint::ENTRY_POINT::kModPowerAttackDamage, agr, my::myUnarmed.p, target, std::addressof(pwAtkDmgMult));

       //std::string left  = isLeft ? "LEFT" : "RIGHT";
       //std::string pwatk = isPwAtk ? "POWER" : "LIGHT";
       //LOG("FIST HIT: {}, {}, DAMAGE - {}, perks_AtkDamageMult - {}, perks_PWAtkDamageMult - {}, target_modIncomingDamage - {}",
             // left, pwatk, damage, atkDmgMult, pwAtkDmgMult, target_overcap);
   }

   void regive_mutagen (RE::SpellItem *spell, RE::Actor* pl, int count)
   {
       sameMutagensGlob.p->value = count;  // change for perk. 1 won't change magn, 2 will do 0.75, 3 will do 0.7
       pl->RemoveSpell(spell);
       pl->AddSpell(spell);
   }

   void add_mutagen (RE::SpellItem *spell, RE::Actor* pl, int count)
   {
       sameMutagensGlob.p->value = count;
       pl->AddSpell(spell);
   }

   void remove_other (std::vector<RE::SpellItem*>& vec, RE::Actor *pl) // mutagens
   {
      for (auto& mut : vec) {
          if (pl->HasSpell(mut)) pl->RemoveSpell(mut);
      }
   }

   void handle_mutagen (RE::Actor *pl, RE::AlchemyItem* potion)
   {
      int red_count = 0, green_count = 0, blue_count = 0;
      
      if      (potion->HasKeyword(m_small_kw.p)) remove_other (smalls, pl);
      else if (potion->HasKeyword(m_med_kw.p))   remove_other (meds, pl);      // remove same size mutagens
      else if (potion->HasKeyword(m_big_kw.p))   remove_other (bigs, pl);
         
      if      (potion->HasKeyword(m_red_kw.p))    red_count++;
      else if (potion->HasKeyword(m_green_kw.p))  green_count++;               // check drinked mutagen color
      else if (potion->HasKeyword(m_blue_kw.p))   blue_count++;
      
      //re-give already active mutagens
      if (pl->HasSpell(m_red_small.p))   regive_mutagen (m_red_small.p, pl, ++red_count);
      if (pl->HasSpell(m_red_mid.p))     regive_mutagen (m_red_mid.p, pl, ++red_count);
      if (pl->HasSpell(m_red_big.p))     regive_mutagen (m_red_big.p, pl, ++red_count); // prefix ++, cause we want increase before func calls
      
      if (pl->HasSpell(m_green_small.p)) regive_mutagen (m_green_small.p, pl, ++green_count);
      if (pl->HasSpell(m_green_mid.p))   regive_mutagen (m_green_mid.p, pl, ++green_count);
      if (pl->HasSpell(m_green_big.p))   regive_mutagen (m_green_big.p, pl, ++green_count);
      
      if (pl->HasSpell(m_blue_small.p))  regive_mutagen(m_blue_small.p, pl, ++blue_count);
      if (pl->HasSpell(m_blue_mid.p))    regive_mutagen (m_blue_mid.p, pl, ++blue_count);
      if (pl->HasSpell(m_blue_big.p))    regive_mutagen (m_blue_big.p, pl, ++blue_count);
      
      if (potion->HasKeyword(m_red_kw.p)) {
          if      (potion->HasKeyword(m_small_kw.p)) add_mutagen (m_red_small.p, pl, red_count);  // add new mutagen
          else if (potion->HasKeyword(m_med_kw.p))   add_mutagen (m_red_mid.p,   pl, red_count);
          else if (potion->HasKeyword(m_big_kw.p))   add_mutagen (m_red_big.p,   pl, red_count);
      }
      else if (potion->HasKeyword(m_green_kw.p)) {
          if      (potion->HasKeyword(m_small_kw.p)) add_mutagen (m_green_small.p, pl, green_count);
          else if (potion->HasKeyword(m_med_kw.p))   add_mutagen (m_green_mid.p,   pl, green_count);
          else if (potion->HasKeyword(m_big_kw.p))   add_mutagen (m_green_big.p,   pl, green_count);
      }
      else if (potion->HasKeyword(m_blue_kw.p)) {
          if      (potion->HasKeyword(m_small_kw.p)) add_mutagen (m_blue_small.p, pl, blue_count);
          else if (potion->HasKeyword(m_med_kw.p))   add_mutagen (m_blue_mid.p,   pl, blue_count);
          else if (potion->HasKeyword(m_big_kw.p))   add_mutagen (m_blue_big.p,   pl, blue_count);
      }
   }

   void oil_proc (RE::Actor* pl, RE::Actor* target, RE::HitData *hit_data, float& damage)
   {
      LOG("called oil_proc");
      
      auto poisonData = u_get_pc_poison(false);  // check right hand
      if (!poisonData) {
           poisonData = u_get_pc_poison(true);   // check left hand
           if (!poisonData) return;
      }
      auto oil = poisonData->poison;                             // alchItem oil
      if (!oil || oil->effects.empty()) return;
      auto oil_id = oil->effects[0]->baseEffect->formID;         // id of effectSetting
      
      float charges = float(poisonData->count);
      float max_charge = 140.f;      // w/o oil grade

      float ef = 1.f;

      if (oil->HasKeyword(oil_improvedKW)) {       //  improved oils
          max_charge += 40.f;                                     
          ef *= 1.2f;    // improved/pure, cause those kw are only in potions, onHit spells has no improved kw
      }
      else if (oil->HasKeyword(oil_pureKW)) {      //  pure oils
          max_charge += 80.f;                                     
          ef *= 1.4f;
      }

      float part = charges/max_charge;       // for ex 90/120 charges remained, part = 0.75
      float debuff = (1-part)*0.8f;          // (1 - 0.75)/*0.8 = 0.2
      if (pl->HasPerk(alch_poison_50_perk)) debuff *= 0.7f;   // decrease debuff from perks
      if (pl->HasPerk(alch_100_perk))       debuff *= 0.7f;   //
      ef -= debuff;      // 1 - 0.2 = 0.8  (charge-dependent debuff)

      //LOG("oil_proc: charges remain - {}, part - {}, debuff - {}, charge-dependent efficiency - {}", charges, part, debuff, ef);

      if (oil_id == oil_silver->formID)    // silver oil
      {
          if (target->HasKeyword(actorTypeUndead) || target->HasKeyword(vampire.p))
          {
              float firstMagn = oil->effects[0]->GetMagnitude() * ef;   // 4.50  (~15 with all perks and ef 1.4)
              float secondMagn = oil->effects[1]->GetMagnitude() * ef;  // 3.12  (~10)
              float dotDmg = firstMagn + (damage * (secondMagn * 0.01f));  // 15 + 10% of dmg
              // firstMagn = 17 for flat damage and non-silver buff, secondMagn = 10 for dmg percent and debuff penetr
              if (!hit_data->weapon->HasKeyword(weapMaterialSilver))
              {
                  damage *= (1.f + firstMagn*0.01f);      // non silver weapon will do x1.17 dmg to undead
              }
              cast_oil (silverDust1.p, target, pl, oil, ef, dotDmg);     // damage spell
              cast_oil (silverDust2.p, target, pl, oil, ef);             // debuff spell
          }
      }
      else if (oil_id == oil_disease->formID)   // black oil
      {
          auto diseaseRes = target->GetActorValue(RE::ActorValue::kResistDisease);    // ignores 50% of disease res, 100% res will make 50% effnss
          float effnss = (1.f - (diseaseRes / 200.f));
          ef *= effnss;
          cast_oil (oil_diseaseOnHit, target, pl, oil, ef);
      }
      else if (oil_id == oil_ignite->formID)    // ignite oil
      {   
          cast_oil (oil_igniteOnHit1, target, pl, oil, ef);  // decrease fire res               
          int fireRes = int(target->GetActorValue(RE::ActorValue::kResistFire));
          if (fireRes < -20) fireRes = -20;
          int random = rand() % (fireRes+50);
          if (random < 20)
              cast_oil (oil_igniteOnHit2, target, pl, oil, ef);  // ignite
      }
      else if (oil_id == oil_frost->formID)     // frost oil
      {    
          cast_oil (oilFrost_onHit1.p, target, pl, oil, ef);  // frost dmg
          int frostRes = int(target->GetActorValue(RE::ActorValue::kResistFrost));
          if (frostRes < -10) frostRes = -10;
          int random = rand() % (frostRes+40);
          if (target->HasMagicEffect(oilFrostDmgDecrease.p)) damage *= 1.1f; // is frozen
          else if (random < 10) {
              cast_oil (oilFrost_onHit2.p, target, pl, oil, ef);  // freeze
              cast_oil (oilFrost_onHit3.p, target, pl, oil, ef);  // atkspeed
          }
      }
      else if (oil_id == oil_poison->formID)     // poison oil
      {
          float poisonRes = target->GetActorValue(RE::ActorValue::kPoisonResist);
          if (poisonRes < 0)    poisonRes = 0;
          if (poisonRes > 99.f) poisonRes = 99.f;
          float effnss = (1.f - (poisonRes / 100.f));
          ef *= effnss;
          int random = rand() % (10 + int(poisonRes/10.f));
          if (random < 7) {
              if (u_same_activeEffects_count(target, oilPoison_stackCounter.p) > 4) {  // if x5 stacks cast overdose
                  float hp = target->GetActorValue(RE::ActorValue::kHealth);
                  float magnOverdose = oil->effects[1]->GetMagnitude() * ef;
                  cast_oil (oilPoison_overdose.p, target, pl, oil, ef, hp*(magnOverdose/100));  // overdose
              }
              cast_oil (oilPoison_onHit1.p, target, pl, oil, ef);   // poison dmg
              cast_oil (oilPoison_onHit2.p, target, pl, oil, ef);   // debuffs
          }
      }
      else if (oil_id == oil_garlic->formID)    // garlic oil
      {    
          if (!target->HasMagicEffect(oilGarlicMarker) && (target->HasSpell(vampirism) || target->HasKeyword(vampire.p))) {
              cast_oil (oilGarlic_onHit1.p, target, pl, oil, ef);
              cast_oil (oilGarlic_onHit2.p, target, pl, oil, ef);
          }
      }
      else if (oil_id == oil_corrose->formID)    // corrose oil
      {    
          float magn = oil->effects[0]->GetMagnitude() * 0.01f * ef;  // default 0.03, scales will make ~0.10 (10% of armor n damage)
          float armorDmg = target->GetActorValue(RE::ActorValue::kDamageResist) * magn;
          cast_oil (oil_corroseOnHit, target, pl, oil, ef, armorDmg);
          damage *= (1.f - magn);  // reduce your dmg, for ex *= 0.9
      }
   }

   void vamp_drain_onHit (RE::Actor* agr, RE::Actor* target)
   {
       float amount = agr->GetLevel() * 0.7f;
       agr->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kHealth, amount);
       agr->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kStamina, amount);
       target->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kHealth, -amount);
       target->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kStamina, -amount);
   }

   void windRunnerCheck (RE::Actor* pl)
   {
      if (pl->HasPerk(windRunnerPerk.p))
      {
          float maxHP = u_get_actor_value_max(pl, RE::ActorValue::kHealth);
          float maxST = u_get_actor_value_max(pl, RE::ActorValue::kStamina);
          float maxMP = u_get_actor_value_max(pl, RE::ActorValue::kMagicka);

          if  (pl->GetActorValue(RE::ActorValue::kHealth) < maxHP / 2)
               pl->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kHealth, maxHP * 0.15f);
          else pl->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kHealth, maxHP * 0.05f);

          if  (pl->GetActorValue(RE::ActorValue::kStamina) < maxST / 2)
               pl->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kStamina, maxST * 0.15f);
          else pl->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kStamina, maxST * 0.05f);

          if  (pl->GetActorValue(RE::ActorValue::kMagicka) < maxMP / 2)
               pl->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kMagicka, maxMP * 0.15f);
          else pl->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kMagicka, maxMP * 0.05f);
      }
   }

   void handle_bossAppearance (RE::Actor *boss)
   {
        currBoss = boss;
        auto fightID = bossFightID.p->value;
        if (boss->HasSpell(my::olve_training.p) && fightID != Olve2stage && fightID != OlvePre2stage)
            bossFightID.p->value = Olve1stage;
        else if (boss->HasSpell(my::seph_training.p) || boss->HasSpell(my::seph_training2.p))
            bossFightID.p->value = Sephiroth;
   }

   void bossFightHandle (int fight, uint16_t& counter)        //  works onUpdate
   {
      LOG("called gameplay::bossFightHandle()");
      if (!currBoss || !currBoss->IsInCombat()) return;

      counter++;
      if (counter > 300) {
          counter = 0;
          return;
      }
      //RE::DebugNotification(std::to_string(counter).c_str());

      if (fight == Olve1stage || fight == Olve2stage)
      {
          if ((fight == Olve2stage) && (counter > 120))  // return to 1 stage after 4min
          {
               counter = 0;
               bossFightID.p->value = Olve1stage;
          }
          else if (olveState == 0 && (counter %7==0))  // pre-shield / every 14 sec
          {
              u_cast_on_self(olvePreShield, currBoss);
              olveState = 1;
          }
          else if (olveState == 1 && (counter %7==0))   // expl / 14 sec
          {
              u_cast_on_self(olveCast, currBoss);
              olveState = 0;
          }
      }
      else if (fight == OlvePre2stage)   // olve just entering 2 stage
      {
          counter = 0;                 // start check stage duration
          bossFightID.p->value = Olve2stage;
      }

      else if (fight == Sephiroth)
      {
          if (counter >= 15)  // 30 sec
          {
              u_cast_on_self(sephCast, currBoss);
              counter = 0;
          } 
      }
   }

   void check_bestiary (RE::Actor* actor)  // adds bestiary keywords to generic npc   (TO GAMEPLAY)
   {
          LOG("called check_bestiary()");
          if (actor->GetActorValue(RE::ActorValue::kMood) == 1.f) return;
	      
          auto race_id = actor->GetRace()->formID;
          auto base = actor->GetActorBase();
	      
          if (bestiary_races_map.count(race_id) > 0) {
              base->AddKeyword(bestiary_races_map.at(race_id));
          }
          else if (race_id == DraugrRace.p->formID) {
              if (actor->GetLevel() < 75) base->AddKeyword(BestiaryDraugr.p);        // draugr
              else                        base->AddKeyword(BestiaryHulkingDraugr.p); // strong draugr
          }
          else if (actor->HasSpell(AbForswornSkinchanges.p)) {                       // werewolf
              if (actor->GetRace()->HasKeyword(DLC2Werebear.p))     base->AddKeyword(BestiaryWerebear.p);
              else if (actor->GetRace()->HasKeyword(isBeastRace.p)) base->AddKeyword(BestiaryWerewolf.p);
              else return; // prevent set mood = 1 in human form
          }
          else if (actor->HasSpell(AbSkeletonChampion.p))   base->AddKeyword(BestiarySkeletonGang.p);
          else if (actor->IsInFaction(BearFaction.p))       base->AddKeyword(BestiaryBear.p);
          else if (actor->IsInFaction(SabreCatFaction.p))   base->AddKeyword(BestiarySabrecat.p);
          else if (actor->IsInFaction(LichFaction.p))       base->AddKeyword(BestiaryLich.p);
          else if (actor->IsInFaction(ArchLichFaction.p))   base->AddKeyword(BestiaryGrandLich.p);
          else if (actor->IsInFaction(BerserkFaction.p))    base->AddKeyword(BestiaryDraugrBerserk.p);
          else if (actor->IsInFaction(CairnBGfaction.p))    base->AddKeyword(BestiaryBlackGuard.p);
          else if (actor->IsInFaction(ValleyGhostFaction.p))base->AddKeyword(BestiaryValleyGhost.p);
          
          actor->SetActorValue(RE::ActorValue::kMood, 1.f);
   }

   void initGameplayData()
   {
      auto handler = RE::TESDataHandler::GetSingleton();

      oil_diseaseOnHit = handler->LookupForm<RE::SpellItem>(0xE9074, "RfaD SSE - Awaken.esp");
      oil_igniteOnHit1 = handler->LookupForm<RE::SpellItem>(0x12AE7E, "RfaD SSE - Awaken.esp");
      oil_igniteOnHit2 = handler->LookupForm<RE::SpellItem>(0x12AE80, "RfaD SSE - Awaken.esp");
      oil_corroseOnHit = handler->LookupForm<RE::SpellItem>(0x2233B9, "RfaD SSE - Awaken.esp");

      oil_silver  = handler->LookupForm<RE::EffectSetting>(0xCFAEC,  "RfaD SSE - Awaken.esp");
      oil_disease = handler->LookupForm<RE::EffectSetting>(0xE9071,  "RfaD SSE - Awaken.esp");
      oil_frost   = handler->LookupForm<RE::EffectSetting>(0x12FF97, "RfaD SSE - Awaken.esp");
      oil_garlic  = handler->LookupForm<RE::EffectSetting>(0x16CC81, "RfaD SSE - Awaken.esp");
      oil_ignite  = handler->LookupForm<RE::EffectSetting>(0x12AE7D, "RfaD SSE - Awaken.esp");
      oil_poison  = handler->LookupForm<RE::EffectSetting>(0x16CC7E, "RfaD SSE - Awaken.esp");
      oil_corrose = handler->LookupForm<RE::EffectSetting>(0x21913B, "RfaD SSE - Awaken.esp");

      oilGarlicMarker = handler->LookupForm<RE::EffectSetting>(0x2284D0, "RfaD SSE - Awaken.esp");
      oil_KWHolder    = handler->LookupForm<RE::EffectSetting>(0x1FFBCA, "RfaD SSE - Awaken.esp");
      oil_improvedKW  = handler->LookupForm<RE::BGSKeyword>(0x21912E, "RfaD SSE - Awaken.esp");
      oil_pureKW      = handler->LookupForm<RE::BGSKeyword>(0x21E298, "RfaD SSE - Awaken.esp");
  
      olveShield = handler->LookupForm<RE::SpellItem>(0xD01250, "RfaD SSE - Awaken.esp");

      olvePreShieldVoice = handler->LookupForm<RE::BGSSoundDescriptorForm>(0xAFE0DC, "RfaD SSE - Awaken.esp");
      
      vampirism = RE::TESForm::LookupByID<RE::SpellItem>(0xED0A8);
      alch_poison_25_perk = RE::TESForm::LookupByID<RE::BGSPerk>(0x105F2F);
      alch_poison_50_perk = RE::TESForm::LookupByID<RE::BGSPerk>(0x105F2C);
      alch_poison_75_perk = RE::TESForm::LookupByID<RE::BGSPerk>(0x58217);
      alch_100_perk       = RE::TESForm::LookupByID<RE::BGSPerk>(0x58218); 
      actorTypeUndead     = RE::TESForm::LookupByID<RE::BGSKeyword>(0x13796);
      weapMaterialSilver  = RE::TESForm::LookupByID<RE::BGSKeyword>(0x10AA1A);

      olvePreShield = handler->LookupForm<RE::SpellItem>(0x1F0890, "RfaD SSE - Awaken.esp");
      olveCast = handler->LookupForm<RE::SpellItem>(0x1C7FFC, "RfaD SSE - Awaken.esp");
      sephCast = handler->LookupForm<RE::SpellItem>(0x1EB778, "RfaD SSE - Awaken.esp");
      currBoss = nullptr;
      olveState = 0;

      smalls.emplace_back (m_red_small.p);
      smalls.emplace_back (m_green_small.p);
      smalls.emplace_back (m_blue_small.p);
      meds.emplace_back   (m_red_mid.p);
      meds.emplace_back   (m_green_mid.p);   // mutagens 
      meds.emplace_back   (m_blue_mid.p);
      bigs.emplace_back   (m_red_big.p);
      bigs.emplace_back   (m_green_big.p);
      bigs.emplace_back   (m_blue_big.p);

	  // bestiary [race;keyword] map
      bestiary_races_map.emplace(WolfRace.p->formID, BestiaryWolf.p);
      bestiary_races_map.emplace(FalmerRace.p->formID, BestiaryFalmer.p);
      bestiary_races_map.emplace(SprigganRace.p->formID, BestiarySpriggan.p);
      bestiary_races_map.emplace(TrollRace.p->formID, BestiarySimpleTroll.p);
      bestiary_races_map.emplace(TrollFrostRace.p->formID, BestiaryFrostTroll.p);
      bestiary_races_map.emplace(DLC1TrollRaceArmored.p->formID, BestiaryArmoredTroll.p);
      bestiary_races_map.emplace(DLC1TrollFrostRaceArmored.p->formID, BestiaryArmoredFrostTroll.p);
      bestiary_races_map.emplace(ChaurusRace.p->formID, BestiaryChaurus.p);
      bestiary_races_map.emplace(ChaurusReaperRace.p->formID, BestiaryChaurusReaper.p);
      bestiary_races_map.emplace(DLC1ChaurusHunterRace.p->formID, BestiaryChaurusHunter.p);
      bestiary_races_map.emplace(FrostbiteSpiderRace.p->formID, BestiaryFrostbiteSpider.p);
      bestiary_races_map.emplace(FrostbiteSpiderRaceGiant.p->formID, BestiaryFrostbiteSpider.p);
      bestiary_races_map.emplace(FrostbiteSpiderRaceLarge.p->formID, BestiaryFrostbiteSpider.p);
      bestiary_races_map.emplace(DLC1SoulCairnMistman.p->formID, BestiaryMistMan.p);
   }
}



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

        contractsDone = handler->LookupForm<RE::TESGlobal>(0x1B3BA9, "RfaD SSE - Awaken.esp");
        castleMagesKilled = handler->LookupForm<RE::TESGlobal>(0x19A64C, "RfaD SSE - Awaken.esp");
        ambassadorsKilled = handler->LookupForm<RE::TESGlobal>(0x288948, "RfaD SSE - Awaken.esp");
        stonesBuffStopDay = handler->LookupForm<RE::TESGlobal>(0x6D0994, "devFixes.esp");
        aedraToken = handler->LookupForm<RE::BGSPerk>(0xE460B, "Requiem for a Dream - DivineBlessings.esp");
        daedraToken = handler->LookupForm<RE::BGSPerk>(0xE460C, "Requiem for a Dream - DivineBlessings.esp");
        pal_ring = handler->LookupForm<RE::TESObjectARMO>(0x145AD3, "devFixes.esp");

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
        auto id = victim->formID;

        if (qst::startDB->currentStage == 40) {
                 if (victim->IsInFaction(RE::TESForm::LookupByID<RE::TESFaction>(0x86EEE)))  qst::currGoalVal->value += 1;  // guard faction
        }
        else if (qst::startDB->currentStage == 60) {
                 // ambassadors (elenwen, ancarion, ancano, ondolemar)
                 if (id == 0x133AF || id == 0x1E7D7 || id == 0x13269 || id == 0x0401CAF3) qst::currGoalVal->value += 1;               
        }
        else if (qst::startMerc->currentStage == 40) {
                 if (victim->HasSpell(qst::mercTraining) || victim->HasSpell(qst::bldHorkerTraining)) qst::currGoalVal->value += 1;  // mercenaries
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
    qst::currGoalVal->value = qst::contractsDone->value;
    return qst::currGoalVal->value;
}

bool vigharMovarthDead() { return (qst::isVigharDead->value > 0 && qst::MS14->currentStage > 100); }

bool sephirothDead() { return (qst::sephirothQuest->IsCompleted() || qst::sephirothQuest->currentStage > 50);  }

bool dbcontractsDone() { return (qst::DBc1->currentStage>20 && qst::DBc2->currentStage>20 && qst::DBc3->currentStage>20);}

bool vampArtifacts() { return (qst::DLC1RV08->currentStage >= 200 && qst::DLC1RV09->currentStage >= 200); }

bool learnedFountain() {return mys::player->HasSpell(qst::handler->LookupForm<RE::SpellItem>(0x4FA6, "RfaD SSE - Awaken.esp"));}

bool allArchLichesDead() { return (qst::isAnachoretDead->value && qst::isSiltineDead->value && qst::isReaperDead->value); }

bool conjRitualQuestDone() { return (RE::TESForm::LookupByID<RE::TESQuest>(0x99F27)->currentStage > 100); }

bool equippedConjurPhylactery() { return u_worn_has_keyword(mys::player, qst::conjPhylact); }

bool has_pal_ring() { return u_get_item_count(mys::player, qst::pal_ring->formID) > 0; }


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
    if (RE::TESForm::LookupByID<RE::TESQuest>(0x211D5)->currentStage > 95) x++;  // T02
    if (RE::TESForm::LookupByID<RE::TESQuest>(0x1C48E)->currentStage > 30) x++;  // T03
    if (RE::TESForm::LookupByID<RE::TESQuest>(0x53309)->currentStage > 25) x++;  // FreeformRiften05
    if (RE::TESForm::LookupByID<RE::TESQuest>(0x43E1A)->currentStage > 10) x++;  // FreeformRiften13
    if (RE::TESForm::LookupByID<RE::TESQuest>(0x242AF)->currentStage > 197)x++;  // DA16
    if (RE::TESForm::LookupByID<RE::TESQuest>(0x7D949)->currentStage > 10) x++;  // DA11Intro
    if (u_get_item_count(mys::player, 0x705C3) > 0) x++;    // Runil Journal

    qst::currGoalVal->value = float(x);
    return x;
}

int tombQuestsDone ()
{
    int x = 0;
    if (qst::deathBrandQuest->currentStage > 90)  x++;                           // DLC2dunHaknirTreasureQST (Haknir)
    if (qst::azidalQuest->currentStage     > 475) x++;                           // DLC2dunKolbjornQST (Azidal)
    if (qst::zakrisosQuest->currentStage   > 50)  x++;                           // DLC2RR03 (Zakrisos)
    if (qst::valokQuest->currentStage       > 50)  x++;                          // DLC2SV01 (Valok)
    if (RE::TESForm::LookupByID<RE::TESQuest>(0x7EDE5)->currentStage > 70) x++;  // dunAngarvundeQST
    if (RE::TESForm::LookupByID<RE::TESQuest>(0x35354)->currentStage > 70) x++;  // dunForelhostQST
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

    qst::currGoalVal->value = float(x);
    return x;
}

int solsteimMasksFound()
{
    int x = 0;
    if (u_get_item_count(mys::player, 0x04024037) > 0) x++;  // zahkriisos
    if (u_get_item_count(mys::player, 0x040240FE) > 0) x++;  // ahzidal
    if (u_get_item_count(mys::player, 0x040240FF) > 0) x++;  // dukaan
    qst::currGoalVal->value = float(x);
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
    qst::currGoalVal->value = float(x);
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
    qst::currGoalVal->value = float(x);
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
    qst::currGoalVal->value = float(x);
    return x;
}

int staffsFound()
{
    int x = 0;
    if (u_get_item_count(mys::player, 0x35369) > 0) x++;   // magnus
    if (u_get_item_count(mys::player, 0x1CB36) > 0) x++;   // rose
    if (u_get_item_count(mys::player, 0x35066) > 0) x++;   // skull
    if (u_get_item_count(mys::player, 0x2AC6F) > 0) x++;   // wabbajack
    qst::currGoalVal->value = float(x);
    return x;
}

int essence_found ()
{
    int x = 0;
    if (u_get_item_count(mys::player, 0xFB8F94) > 0) x++;
    if (u_get_item_count(mys::player, 0x1D7CB9) > 0) x++;
    if (u_get_item_count(mys::player, 0x1D7CBA) > 0) x++;
    qst::currGoalVal->value = float(x);
    return x;
}

int altars_touched()
{
    qst::currGoalVal->value = qst::altarsTouched.p->value;
    return int(qst::currGoalVal->value);
}

int vampire_dust_have() {
    
    qst::currGoalVal->value = u_get_item_count(mys::player, 0x3AD76);  
    return int(qst::currGoalVal->value);
}

int castle_mages_killed()
{
    qst::currGoalVal->value = qst::castleMagesKilled->value;
    return int(qst::currGoalVal->value);
}

int ambassadors_killed()
{
    qst::currGoalVal->value = qst::ambassadorsKilled->value;
    return int(qst::currGoalVal->value);
}

int daedra_guards_killed ()
{
    int x = 0;
    if (qst::isMeridiaBossDead->value > 0) x++;
    if (qst::isNamiraBossDead->value  > 0) x++;
    if (qst::isVerminaBossDead->value > 0) x++;
    if (qst::sephirothQuest->currentStage > 50) x++;
    if (qst::DA02->currentStage > 30) x++;  // boethia guard
    qst::currGoalVal->value = float(x);
    return x;
}

int arch_lich_killed ()
{
    int x = 0;
    if (qst::isAnachoretDead->value > 0) x++;
    if (qst::isSiltineDead->value > 0) x++;
    if (qst::isReaperDead->value > 0) x++;
    qst::currGoalVal->value = float(x);
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
    qst::currGoalVal->value = float(x);
    return x;
}

bool chosenDaedra_fullPerks_n_quest()
{
    bool ok_ = false;
    if (mys::player->HasPerk(qst::cult_boethia_2)  && qst::DA02->currentStage > 30)   ok_ = true;
    if (mys::player->HasPerk(qst::cult_mephala_2)  && qst::DA08->currentStage > 40)   ok_ = true;
    if (mys::player->HasPerk(qst::cult_vermina_2)  && qst::DA16->currentStage > 197)  ok_ = true;
    if (mys::player->HasPerk(qst::cult_nocturn_2)  && qst::TG09->currentStage > 50)   ok_ = true;
    if (mys::player->HasPerk(qst::cult_meridia_2)  && qst::DA09->currentStage > 350)  ok_ = true;
    if (mys::player->HasPerk(qst::cult_azura_2)    && qst::DA01->currentStage > 85)   ok_ = true;
    if (mys::player->HasPerk(qst::cult_namira_2.p) && qst::DA11->currentStage > 60)   ok_ = true;
    return ok_;
}

void check_horseCert()
{
    auto certificate = qst::handler->LookupForm<RE::TESObjectBOOK>(0x8C3A8A, "devFixes.esp");
    auto horsecost = RE::TESForm::LookupByID<RE::TESGlobal>(0xF49BD);
    auto stablesquest = RE::TESForm::LookupByID<RE::TESQuest>(0x68D73);
    if (u_get_item_count(mys::player, certificate->formID) > 0)  // if have cert
    {
        if (u_get_game_statistic("Horses Owned") > 0)        // and bought horse
        {
             horsecost->value = 2500.f;                                // price = 2500
             mys::player->RemoveItem(certificate, 1, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);     // remove cert
        }
        else horsecost->value = 0.f;  // have cert but haven't bought a horse yet = price 0
    }
    else horsecost->value = 2500.f;   // have no cert = price 2500
    u_updQuestTextGlob(stablesquest, horsecost);
}

namespace qst
{
  void check_class_achievements(RE::Actor* player)
  {

    if (qst::startTravel->currentStage > 0)            //  [TRAVELER]
    {
        auto this_ = qst::startTravel;
        auto stage = qst::startTravel->currentStage;
        if    (stage == 1  && qst::stonesBuffStopDay->value)            setStage(this_, 10);  // [1] touch stone
        else if (stage == 10 && u_is_in_city(player))                   setStage(this_, 20);  // [2] travel to city     
        else if (stage == 20 && qMisc("Misc Objectives Completed") > 9) setStage(this_, 30);  // [3] misc quests x10     
        else if (stage == 30 && qMisc("Ingredients Harvested") > 149)   setStage(this_, 40);  // [4] harvested  x150
        else if (stage == 40 && qMisc("Side Quests Completed") > 2)     setStage(this_, 50);  // [5] side quests x3
        else if (stage == 50 && qMisc("Nirnroots Found") > 29)          setStage(this_, 60);  // [6] nirnroots x30
        else if (stage == 60 && qMisc("Side Quests Completed") > 5)     setStage(this_, 70);  // [7] side quests x6 
        else if (stage == 70 && qMisc("Words Of Power Learned") > 14)   setStage(this_, 80);  // [8] words of power x15
        else if (stage == 80 && priestMasksFound() > 9)                 setStage(this_, 90);  // [9] priest masks    x10
        else if (stage == 90 && blackBooksDone() > 5)                   setStage(this_, 100); // [10] black books x6
        u_updQuestTextGlob(this_, qst::currGoalVal);
    }
    else if (qst::startMerc->currentStage > 0)            //  [MERCENARY]
    {
        auto this_ = qst::startMerc;
        auto stage = qst::startMerc->currentStage;
        //      (stage == 1  && witch reward_taken)                                          // [1] reward for witch
        if      (stage == 10 && getContractsDone() > 2)                setStage(this_, 20);  // [2] contrtacts x3
        else if (stage == 20 && qst::StalleoQuest->currentStage > 60)  setStage(this_, 30);  // [3] stalleo quest
        else if (stage == 30 && getContractsDone()   > 19)             setStage(this_, 40);  // [4] contracts x20
        else if (stage == 40 && qst::currGoalVal->value > 14)          setStage(this_, 50);  // [5] slay mercs x15
        else if (stage == 50 && qMisc("Dungeons Cleared") > 39)        setStage(this_, 60);  // [6] dungeons clear x40
        else if (stage == 60 && getContractsDone() > 44)               setStage(this_, 70);  // [7] contracts x45
        else if (stage == 70 && qst::DLC2RR02->currentStage > 110)     setStage(this_, 80);  // [8] raven_rock
        else if (stage == 80 && tombQuestsDone() > 11)                 setStage(this_, 90);  // [9] thomb quests x12
        u_updQuestTextGlob(this_, qst::currGoalVal);
    }
    else if (qst::startPal->currentStage > 0)        //  [PALADIN]
    {
        auto this_ = qst::startPal;
        auto stage = qst::startPal->currentStage;
        if      (stage == 1  && player->HasPerk(qst::aedraToken))   setStage(this_, 10);   // [1] take bless
        else if (stage == 10 && citizenHelped() > 9)                setStage(this_, 20);   // [2] help citizen x10
        else if (stage == 20 && qst::DBDestroy->currentStage > 30)  setStage(this_, 30);   // [3] destroy DB
        else if (stage == 30 && priestsHelped() > 2)                setStage(this_, 40);   // [4] help priests x3
        else if (stage == 40 && qst::DA10->currentStage > 65)       setStage(this_, 50);   // [5] help tyranus
        else if (stage == 50 && has_pal_ring())                     setStage(this_, 60);   // [6] ring of paladin
        else if (stage == 60 && altars_touched() > 8)               setStage(this_, 70);   // [7] altars touched x9
        else if (stage == 70 && essence_found() > 2)                setStage(this_, 80);   // [8] find 3 essence
        else if (stage == 80 && qst::isAnachoretDead->value > 0)    setStage(this_, 90);   // [9] anachoret
        u_updQuestTextGlob(this_, qst::currGoalVal);
    }
    else if (qst::startVigil->currentStage > 0)        //  [VIGILANT]
    {
        auto this_ = qst::startVigil;
        auto stage = qst::startVigil->currentStage;
        //      (stage == 1  && take_amulet_papyrus))                 setStage(this_, 10);   // [1] amulet
        //      (stage == 10 && deliver_note_papyrus)                 setStage(this_, 20);   // [2] note to carcetta
        if      (stage == 20 && qMisc("Undead Killed") > 9)           setStage(this_, 30);   // [3] undead x10
        else if (stage == 30 && qMisc("Daedra Killed") > 4)           setStage(this_, 40);   // [4] daedra x5
        else if (stage == 40 && qst::DA11->currentStage > 100)        setStage(this_, 50);   // [5] slay cannibals
        else if (stage == 50 && qst::DA10->currentStage > 65)         setStage(this_, 60);   // [6] tyranus
        else if (stage == 60 && qst::currGoalVal->value > 19)         setStage(this_, 70);   // [7] vampires x20
        else if (stage == 70 && qst::isNamiraBossDead->value)         setStage(this_, 80);   // [8] namira canal boss
        else if (stage == 80 && vigharMovarthDead())                  setStage(this_, 90);   // [9] vighar & movart
        else if (stage == 90 && sephirothDead())                      setStage(this_, 100);  // [10] sephiroth
        u_updQuestTextGlob(this_, qst::currGoalVal);
    }
    else if (qst::startDB->currentStage > 0)        //  [ASSASSIN]
    {
        auto this_ = qst::startDB;
        auto stage = qst::startDB->currentStage;
        if      (stage == 1  && dbcontractsDone())                setStage(this_, 10);  // [1] contracts
        else if (stage == 10 && qMisc("Poisons Mixed") > 4)       setStage(this_, 20);  // [2] poisons x5
        else if (stage == 20 && qMisc("Murders") > 9)             setStage(this_, 30);  // [3] peaceful x10
        else if (stage == 30 && qMisc("People Killed") > 59)      setStage(this_, 40);  // [4] enemies x60
        else if (stage == 40 && qst::currGoalVal->value > 29)     setStage(this_, 50);  // [5] guards x30
        else if (stage == 50 && castle_mages_killed() > 2)        setStage(this_, 60);  // [6] castle mage x3
        else if (stage == 60 && ambassadors_killed()  > 2)        setStage(this_, 70);  // [7] ambassador x3
        else if (stage == 70 && qst::DLC2RR02->currentStage >90)  setStage(this_, 80);  // [8] morag tong
        u_updQuestTextGlob(this_, qst::currGoalVal);
    }
    else if (qst::startVamp->currentStage > 0)        //  [VAMPIRE]
    {
        auto this_ = qst::startVamp;
        auto stage = qst::startVamp->currentStage;
      //if      (stage == 1  && phials papyrus)                   setStage(this_, 10);   // [1] phials
        if      (stage == 10 && qst::currGoalVal->value > 9)      setStage(this_, 20);   // [2] feed x10
        else if (stage == 20 && qst::isMarkDead->value > 0)       setStage(this_, 30);   // [3] slay Mark
        else if (stage == 30 && qst::isWilhelmDead->value > 0)    setStage(this_, 40);   // [4] slay Wilhelm
        else if (stage == 40 && qst::currGoalVal->value > 19)     setStage(this_, 50);   // [5] vampires x20
        else if (stage == 50 && qst::currGoalVal->value > 9)      setStage(this_, 55);   // [6] dawnguards x10
      //else if (stage == 55 && speak_to_vesper_papyrus)          setStage(this_, 60);   // 
      //else if (stage == 60 && bring_artifacts_papyrus)          setStage(this_, 70);   // [7] bring artifacts x4
        else if (stage == 70 && vigharMovarthDead())              setStage(this_, 80);   // [8] vighar & movart
        else if (stage == 80 && qst::DLC1VQ08->currentStage > 40) setStage(this_, 90);   // [9] harkon dead
        u_updQuestTextGlob(this_, qst::currGoalVal);
    }
    else if (qst::startMage->currentStage > 0)        //  [MAGE]
    {
        auto this_ = qst::startMage;
        auto stage = qst::startMage->currentStage;
        if      (stage == 1  && learnedFountain())                setStage(this_, 10);  // [1] learn fountain
        else if (stage == 10 && qMisc("Potions Mixed") > 29)      setStage(this_, 20);  // [2] potions x30
        else if (stage == 20 && hasMagicAdeptPerk())              setStage(this_, 30);  // [3] adept
        else if (stage == 30 && qMisc("Spells Learned") > 29)     setStage(this_, 40);  // [4] spells x30
        else if (stage == 40 && qst::DLC2TT2->currentStage >400)  setStage(this_, 50);  // [5] neloth - kill ildari
        else if (stage == 50 && staffsFound() > 3)                setStage(this_, 60);  // [6] staffs x4
        else if (stage == 60 && qst::MG08->currentStage > 50)     setStage(this_, 70);  // [7] arch-mage        
        else if (stage == 70 && ritualQuests() > 4)               setStage(this_, 80);  // [8] ritual quest x5
        else if (stage == 80 && solsteimMasksFound() > 2)         setStage(this_, 90);  // [9] solsteim masks
        u_updQuestTextGlob(this_, qst::currGoalVal);
    }
    else if (qst::startCult->currentStage > 0)        //  [CULTIST]
    {
        auto this_ = qst::startCult;
        auto stage = qst::startCult->currentStage;
        if      (stage == 1  && player->HasPerk(qst::daedraToken))   setStage(this_, 10);  // [1] daedra bless
      //else if (stage == 10 && heart_scroll_papyrus)                setStage(this_, 20);  // [2] heart scroll
        else if (stage == 20 && qMisc("People Killed") > 29)         setStage(this_, 30);  // [3] humanoid x30
        else if (stage == 30 && qst::currGoalVal->value > 14)        setStage(this_, 40);  // [4] cultist  x15
        else if (stage == 40 && chosenDaedra_fullPerks_n_quest())    setStage(this_, 50);  // [5] chosen daedra full
        else if (stage == 50 && blackBooksDone() > 2)                setStage(this_, 60);  // [6] black books x3
        else if (stage == 60 && daedra_guards_killed() > 2)          setStage(this_, 70);  // [7] daedra guards x3
        else if (stage == 70 && daedra_artifacts_have() > 14)        setStage(this_, 80);  // [8] daedra artifact x15
        else if (stage == 80 && haveMiraakSephirothMasks())          setStage(this_, 90);  // [9] miraak & seph masks
        u_updQuestTextGlob(this_, qst::currGoalVal);
    }
    else if (qst::startNecr->currentStage > 0)            //  [NECR]
    {
        auto this_ = qst::startNecr;
        auto stage = qst::startNecr->currentStage;
      //if      (stage == 1  && body_parts_papyrus)              setStage(this_, 10);  // [1] body parts
      //if      (stage == 10 && ritual_scroll_papyrus)           setStage(this_, 20);  // [2] scroll
        if      (stage == 20 && qMisc("Souls Trapped") > 19)     setStage(this_, 30);  // [3] souls x20
        else if (stage == 30 && qMisc("People Killed") > 79)     setStage(this_, 40);  // [4] humanoid x80
        else if (stage == 40 && vampire_dust_have() > 19)        setStage(this_, 50);  // [5] vampire dust x20
        else if (stage == 50 && qst::currGoalVal->value > 14)    setStage(this_, 60);  // [6] necromants x15
        else if (stage == 60 && conjRitualQuestDone())           setStage(this_, 70);  // [7] ritual quest conj
        else if (stage == 70 && equippedConjurPhylactery())      setStage(this_, 80);  // [8] conj phylactery
        else if (stage == 80 && arch_lich_killed() > 2)          setStage(this_, 90);  // [9] arch liches x3
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

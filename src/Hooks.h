#pragma once

#include "SKSE/SKSE.h"
#include "core.h"
#include "ItemCardFixer.h"

#ifdef SKYRIM_SUPPORT_AE
    #define RELOCATION_OFFSET(SE, AE) (AE)
#else
    #define RELOCATION_OFFSET(SE, AE) (SE)
#endif


namespace hooks
{
    //------ hooks --------

    static float upd_timer = 2.0f;  // 2 sec

    class OnPlayerUpdate_Hook
    {
    public:
        static void install_hook()    {
            old_func = REL::Relocation<uintptr_t>(RE::VTABLE_PlayerCharacter[0]).write_vfunc(0xAD, new_func);
        }
    private:
        static void new_func(RE::PlayerCharacter* player_, float delta)  // первый аргумент RE::Actor *this (по-хорошему)
        {
            mys::time_delta = delta;  // for other functions

            upd_timer -= delta;

            if (upd_timer <= 0.f) {
                upd_timer = 2.0f;  // every 2 sec
                on_my_update();
            }
            return old_func(player_, delta);
        }
        static inline REL::Relocation<decltype(new_func)> old_func;
    };

    class OnCharacterUpdate_Hook  // not used
    {
    public:
        static void install_hook() { old_func = REL::Relocation<uintptr_t>(RE::VTABLE_Character[0]).write_vfunc(0xAD, new_func); }
    private:
        static void new_func(RE::Character* character, float delta)
        {
            if (character) {
                if (character->currentProcess && character->currentProcess->middleHigh) {
                    //...
                }
            }
            return old_func(character, delta);
        }
        static inline REL::Relocation<decltype(new_func)> old_func;
    };

    class HiddenSpeedMult_updater_Hook  // returns actor hidden speed mult, default 1.0
    {
    public:
        static void install_hook() {
            old_func = SKSE::GetTrampoline().write_call<5>(REL::ID(37013).address() + 0x1A,    new_func);
        }
    private:
        static float new_func(RE::Actor* a_actor)
        {
            float hiddenMult = old_func(a_actor);  // default func is called
            if (a_actor->IsPlayerRef() && mys::dodge_timer > 0) {
                mys::dodge_timer -= mys::time_delta;
                hiddenMult += 2.2f;
            }
            return hiddenMult - mys::ms_compensator;
        }
        static inline REL::Relocation<decltype(new_func)> old_func;
    };

    // НАДО ХУКНУТЬ CALC_ATTACK_STAMINA_DRAIN , но не function call хуками т.к. их там 4 штуки (4 места вызова), а function start хуком из 8 ролика. Сделать траты стамины через это.

    // ---------------------------------------  AI HOOKS -----------------------------------------------

    class EvadeProjectileChanceNPC_Hook  // chance that npc move aside when projectile flies (AI). it doesn't consider "reaction time" parameter, so npc might not have time to dodge at long range
    {
    public:
        static void install_hook() { old_func = SKSE::GetTrampoline().write_call<5>(REL::ID(46589).address() + 0xef, new_func); }
    private:
        static float new_func(RE::Actor* a_actor)
        {
            old_func(
                a_actor);  // even if we don't need default func calculations, it's always better to call it for mod compatibility. Another mod can hook this after our mod
            //return 0.25f;    // 1 will make 100% chance. Later maybe add a AI-simulator function, that high-level agile NPC evade more often
        }
        static inline REL::Relocation<decltype(new_func)> old_func;
    };

    class AttacksChanceNPC_Hook  // works when npc calcs chances for different attack types (attack choose), return 0 will make them not attack at all (will stand in block)
    {
    public:
        static void install_hook() { old_func = SKSE::GetTrampoline().write_call<5>(REL::ID(48139).address() + 0x2ae, new_func); }
    private:
        static float new_func(RE::Actor* a_actor)
        {
            old_func(a_actor);
            // return 0.f;    // unable to attack, for example w/o stamina
        }
        static inline REL::Relocation<decltype(new_func)> old_func;
    };
    //-------------------------------------------------------------------------------------------------

    class OnRefrInitHavok_Hook  // virt. InitHavok() / not used
    {
    public:
        static void install_hook()
        {
            old_func = REL::Relocation<uintptr_t>(RE::VTABLE_TESObjectREFR[0]).write_vfunc(0x66, new_func);
        }
    private:
        static void new_func(RE::TESObjectREFR* this_)
        {
            if (this_->GetBaseObject())
            {
                if (this_->GetBaseObject()->GetFormType() == RE::FormType::Misc) {
                    this_->DetachHavok(this_->GetCurrent3D());  // disable havok and activation (object will become full static)
                    return;  // don't init havok
                }
            }
            old_func(this_);
        }
        static inline REL::Relocation<decltype(new_func)> old_func;
    };
    
    class OnLoad3D_Hook  // virt. Load3D()
    {
      public:
        static void install_hook()
        {
            old_func_ref = REL::Relocation<uintptr_t>(RE::VTABLE_TESObjectREFR[0]).write_vfunc(0x6A, new_func_ref);  // objects
            old_func_npc = REL::Relocation<uintptr_t>(RE::VTABLE_Character[0]).write_vfunc(0x6A, new_func_npc);      // actors
        }
      private:
        static RE::NiAVObject* new_func_ref(RE::TESObjectREFR* this_, bool a_backgroundLoading)
        {
            if (this_) {
                on_3d_load(this_);
            }
            return old_func_ref(this_, a_backgroundLoading);
        }
        static RE::NiAVObject* new_func_npc(RE::Character* this_, bool a_backgroundLoading)
        {
            if (this_) {
                on_3d_load(this_);
            }
            return old_func_npc(this_, a_backgroundLoading);
        }
        static inline REL::Relocation<decltype(new_func_ref)> old_func_ref;
        static inline REL::Relocation<decltype(new_func_npc)> old_func_npc;
    };

    class OnRemoveItem_Hook  // virt. RemoveItem()  ЭТО ПРИМЕР ИЗ КАТЕГОРИИ "ВОЗВРАЩЕНИЕ СЛОЖНЫХ СТРУКТУР ПО ЗНАЧЕНИЮ" (имеет недефолтный конструктор/деструктор либо ее размер не степерь 2)
    {                                               // поэтому оно преобразуется компилятором по правилу:  handle foo (this, a2...) -> handle* foo (this, handle* ans, a2...)  и хукать нужно так.
      public:                                       // в реалиях коммонлибы это работает в основном если по значению возвращается сложная структура либо handlе, тогда преобразуем хук.
        static void install_hook()    {
            old_func_pc = REL::Relocation<uintptr_t>(RE::VTABLE_PlayerCharacter[0]).write_vfunc(0x56, new_func_pc);  // player
            old_func_npc = REL::Relocation<uintptr_t>(RE::VTABLE_Character[0]).write_vfunc(0x56, new_func_npc);      // actor
            old_func_ref = REL::Relocation<uintptr_t>(RE::VTABLE_TESObjectREFR[0]).write_vfunc(0x56, new_func_ref);  // container
        }
      private:
        static RE::ObjectRefHandle* new_func_pc(RE::PlayerCharacter* this_, RE::ObjectRefHandle* ans, RE::TESBoundObject* item, std::int32_t count, RE::ITEM_REMOVE_REASON reason, 
               RE::ExtraDataList* extraList, RE::TESObjectREFR* targetCont, const RE::NiPoint3* dropLoc = 0, const RE::NiPoint3* rotate = 0)
        {
            bool ok = on_remove_item_from_actor(this_, item, count, reason, targetCont);
            if (ok)    return old_func_pc(this_, ans, item, count, reason, extraList, targetCont, dropLoc, rotate);
            else return old_func_pc(this_, ans, item, 0, reason, extraList, targetCont, dropLoc, rotate);  // decline remove item
        }
        static RE::ObjectRefHandle* new_func_npc (RE::Character* this_, RE::ObjectRefHandle* ans, RE::TESBoundObject* item, std::int32_t count,    RE::ITEM_REMOVE_REASON reason, 
               RE::ExtraDataList* extraList, RE::TESObjectREFR* targetCont, const RE::NiPoint3* dropLoc = 0, const RE::NiPoint3* rotate = 0)
        {
            bool ok = on_remove_item_from_actor(this_, item, count, reason, targetCont);
            if (ok)    return old_func_npc(this_, ans, item, count, reason, extraList, targetCont, dropLoc, rotate);
            else return old_func_npc(this_, ans, item, 0, reason, extraList, targetCont, dropLoc, rotate);
        }
        static RE::ObjectRefHandle* new_func_ref(RE::TESObjectREFR* this_, RE::ObjectRefHandle* ans, RE::TESBoundObject* item, std::int32_t count,    RE::ITEM_REMOVE_REASON reason,
               RE::ExtraDataList* extraList, RE::TESObjectREFR* targetCont, const RE::NiPoint3* dropLoc = 0, const RE::NiPoint3* rotate = 0)
        {
            bool ok = on_remove_item_from_cont(this_, item, count, reason, targetCont);
            if (ok)    return old_func_ref(this_, ans, item, count, reason, extraList, targetCont, dropLoc, rotate);
            else return old_func_ref(this_, ans, item, 0, reason, extraList, targetCont, dropLoc, rotate);
        }
        static inline REL::Relocation<decltype(new_func_pc)> old_func_pc;
        static inline REL::Relocation<decltype(new_func_npc)> old_func_npc;
        static inline REL::Relocation<decltype(new_func_ref)> old_func_ref;
    };

    class OnAddItemToContainer_Hook  // virt. AddObjectToContainer()
    {
      public:
        static void install_hook()    {
            old_func_pc = REL::Relocation<uintptr_t>(RE::VTABLE_PlayerCharacter[0]).write_vfunc(0x5A, new_func_pc);  // player
            old_func_npc = REL::Relocation<uintptr_t>(RE::VTABLE_Character[0]).write_vfunc(0x5A, new_func_npc);      // actor
            old_func_ref = REL::Relocation<uintptr_t>(RE::VTABLE_TESObjectREFR[0]).write_vfunc(0x5A, new_func_ref);  // container
        }
      private:
        static void new_func_pc(RE::PlayerCharacter* this_, RE::TESBoundObject* obj, RE::ExtraDataList* extraList,
            std::int32_t count, RE::TESObjectREFR* fromRefr)
        {
            bool ok = on_additem_to_actor(this_, obj, extraList, count, fromRefr);
            if (ok)    old_func_pc(this_, obj, extraList, count, fromRefr);
        }
        static void new_func_npc(RE::Character* this_, RE::TESBoundObject* obj, RE::ExtraDataList* extraList, std::int32_t count, RE::TESObjectREFR* fromRefr)
        {
            bool ok = on_additem_to_actor(this_, obj, extraList, count, fromRefr);
            if (ok)    old_func_npc(this_, obj, extraList, count, fromRefr);
        }
        static void new_func_ref(RE::TESObjectREFR* this_, RE::TESBoundObject* obj, RE::ExtraDataList* extraList, std::int32_t count, RE::TESObjectREFR* fromRefr)
        {
            bool ok = on_additem_to_cont(this_, obj, extraList, count, fromRefr);
            if (ok)    old_func_ref(this_, obj, extraList, count, fromRefr);
        }
        static inline REL::Relocation<decltype(new_func_pc)> old_func_pc;
        static inline REL::Relocation<decltype(new_func_npc)> old_func_npc;
        static inline REL::Relocation<decltype(new_func_ref)> old_func_ref;
    };

    class OnPickUpObject_Hook  // virt. PickUpObject()
    {
      public:
        static void install_hook()    {
            old_func_pc = REL::Relocation<uintptr_t>(RE::VTABLE_PlayerCharacter[0]).write_vfunc(0xCC, new_func_pc);
            old_func_npc = REL::Relocation<uintptr_t>(RE::VTABLE_Character[0]).write_vfunc(0xCC, new_func_npc);
        }
      private:
        static void new_func_pc(RE::PlayerCharacter* this_, RE::TESObjectREFR* obj, std::int32_t count, bool arg3 = false, bool playSound = true)
        {
            on_pickup_object(this_, obj, count, arg3, playSound);
            old_func_pc(this_, obj, count, arg3, playSound);
        }
        static void new_func_npc(RE::Character* this_, RE::TESObjectREFR* obj, std::int32_t count, bool arg3 = false, bool playSound = true)
        {
            on_pickup_object (this_, obj, count, arg3, playSound);
            old_func_npc (this_, obj, count, arg3, playSound);
        }
        static inline REL::Relocation<decltype(new_func_pc)> old_func_pc;
        static inline REL::Relocation<decltype(new_func_npc)> old_func_npc;
    };

    class OnEquip_Hook
    {
      public:
        static void install_hook()    {
            old_func = SKSE::GetTrampoline().write_call<5>(REL::ID(37938).address() + RELOCATION_OFFSET(0xE5, 0x170), new_func);
        }
      private:
        static void new_func(RE::ActorEquipManager* equip_manager, RE::Actor* actor, RE::TESBoundObject* bound_object,
            RE::ObjectEquipParams* params)
        {
            on_equip(actor, bound_object, params);
            return old_func(equip_manager, actor, bound_object, params);
        }
        static inline REL::Relocation<decltype(new_func)> old_func;
    };

    class OnUnEquip_Hook
    {
     public:
        static void install_hook() {
            old_func = SKSE::GetTrampoline().write_call<5>(REL::ID(37945).address() + 0x138, new_func);
        }
     private:
        static void new_func(RE::ActorEquipManager* equip_manager, RE::Actor* actor, RE::TESBoundObject* bound_object, RE::ObjectEquipParams* params)
        {
            on_unequip(actor, bound_object, params);
            return old_func(equip_manager, actor, bound_object, params);
        }
        static inline REL::Relocation<decltype(new_func)> old_func;
    };

    class OnDispel_Hook
    {
      public:
        static void install_hook() { old_func = SKSE::GetTrampoline().write_call<5>(REL::ID(33777).address() + 0x32, new_func); }
      private:
        static void new_func(RE::ActiveEffect* eff, char force)
        {
            if (eff && eff->effect && eff->effect->baseEffect) {
                if (eff->effect->baseEffect->HasKeyword(my::no_dispel.p))  return;
                if (eff->spell && eff->spell->HasKeyword(my::no_dispel.p)) return;  // food, potions
            }
            return old_func(eff, force);
        }
        static inline REL::Relocation<decltype(new_func)> old_func;
    };

    class On_AnimEvent_Hook
    {
      public:
        static void install_hook()    {
            old_func_pc = REL::Relocation<uintptr_t>(RE::VTABLE_PlayerCharacter[2]).write_vfunc(0x1, new_func);
            //old_func_npc = REL::Relocation<uintptr_t>(RE::VTABLE_Character[2]).write_vfunc(0x1, new_func);
        }
      private:
        static auto new_func(RE::BSTEventSink<RE::BSAnimationGraphEvent>* this_, RE::BSAnimationGraphEvent* event,    RE::BSTEventSource<RE::BSAnimationGraphEvent>* dispatcher) -> void
        {
            if (!event || !event->holder) {
                return old_func_pc(this_, event, dispatcher);
            }
            player_anim_handle(this_, event, dispatcher);
            return old_func_pc(this_, event, dispatcher);
        }
        static inline REL::Relocation<decltype(new_func)> old_func_pc;
        //static inline REL::Relocation<decltype(new_func)> old_func_npc;
    };

    class OnJump_Hook  // it hooks one of functions calls within the JumpHandler::ProcessButton function (in section where successful jump occured), so we can't decline the jump
    {
     public:
        static void install_hook() { old_func = SKSE::GetTrampoline().write_call<5>(REL::ID(36271).address() + 0x190, new_func); }
     private:
        static void new_func(RE::Actor* a)
        {
            if (a) on_jump(a);
            return old_func(a);
        }
        static inline REL::Relocation<decltype(new_func)> old_func;
    };

    class OnPhysicalHit_Hook  // works just before hit, can't decline hit event itself
    {
      public:
        static void install_hook() { old_func = SKSE::GetTrampoline().write_call<5>(REL::ID(37673).address() + 0x3c0, new_func); }
      private:
        static void new_func(RE::Actor* target, RE::HitData& hit_data)
        {
            hit_data.totalDamage = on_physical_hit (target, hit_data);  // calc new total damage of hit
            return old_func (target, hit_data);
        }
        static inline REL::Relocation<decltype(new_func)> old_func;
    };

    class On_MeleeWeapCollide_Hook  //  melee weapon collides target, works before hit process, can decline hit event
    {
      public:
        static void install_hook() { old_func = SKSE::GetTrampoline().write_call<5>(REL::ID(37650).address() + 0x38B, new_func); }
      private:
        static void new_func(RE::Actor* attacker, RE::Actor* victim, RE::Projectile* proj, char aleft)
        {
            if (!attacker || !victim) {
                return old_func(attacker, victim, proj, aleft);
            }
            bool allowAttack = on_meleeWeap_collide (attacker, victim);
            if (allowAttack) return old_func (attacker, victim, proj, aleft);
            else mys::attackKeyHolding = false;  // if not allow this attack, turn off the blocker immediate
        }
        static inline REL::Relocation<decltype(new_func)> old_func;
    };


    class OnSpellRelease_Hook   //  virt. ActorMagicCaster::SpellCast()    //  works on spell release
    {
      public:
        static void install_hook()    {
            old_func = REL::Relocation<uintptr_t>(RE::VTABLE_ActorMagicCaster[0]).write_vfunc(0x9, new_func);
        }
      private:
        static void new_func (RE::ActorMagicCaster* this_, bool doCast, std::uint32_t arg2, RE::MagicItem* spell)
        {
            if (this_ && spell) on_spell_release (this_, spell);
            old_func (this_, doCast, arg2, spell);
        }
        static inline REL::Relocation<decltype(new_func)> old_func;
    };

    class OnCheckCast_Hook   //  virt. ActorMagicCaster::CheckCast()    //  for ff spells works 1 time on casting and 2 times after release, always knows if dual cast.
    {                                                                   //  for conc spells works every second.  Also works when eating food, potions etc
      public:
        static void install_hook()    {
            old_func = REL::Relocation<uintptr_t>(RE::VTABLE_ActorMagicCaster[0]).write_vfunc(0xA, new_func);
        }
      private:
        static bool new_func (RE::ActorMagicCaster* this_, RE::MagicItem* spell, bool dualCast, float* alchStrength, RE::MagicSystem::CannotCastReason* reason, bool useBaseValueForCost)
        {
            if (this_ && spell) 
                return on_check_cast (this_, spell, dualCast);
            return old_func (this_, spell, dualCast, alchStrength, reason, useBaseValueForCost);
        }
        static inline REL::Relocation<decltype(new_func)> old_func;
    };

    class OnApplyResist_Hook  //  срабатывает при порезке об резисты, возвращает мульт остатка урона, например 0.3 (если резист был 70%)
    {
    public:
        static void install_hook()    {
            REL::Relocation<uintptr_t> for_npc_hook{ RELOCATION_ID(261401, 0) };
            REL::Relocation<uintptr_t> for_pc_hook{ RELOCATION_ID(261920, 0) };
            old_func_npc = for_npc_hook.write_vfunc(0x0A, new_func_npc);
            old_func_pc = for_pc_hook.write_vfunc(0x0A, new_func_pc);
        }
    private:
        static float new_func_pc (RE::MagicTarget* this_, RE::MagicItem* magic_item, RE::Effect* effect, RE::TESBoundObject* bound_object)
        {
            if (!this_ || !magic_item || !effect) {
                return old_func_pc(this_, magic_item, effect, bound_object);
            } else {
                return on_resist_apply (this_, magic_item, effect);
            }
        }
        static float new_func_npc(RE::MagicTarget* this_, RE::MagicItem* magic_item, RE::Effect* effect, RE::TESBoundObject* bound_object)
        {
            if (!this_ || !magic_item || !effect) {
                return old_func_npc(this_, magic_item, effect, bound_object);
            } else {
                return on_resist_apply (this_, magic_item, effect);
            }
        }
        static inline REL::Relocation<decltype(new_func_pc)> old_func_pc;
        static inline REL::Relocation<decltype(new_func_npc)> old_func_npc;
    };

    class OnPlayerDrinkPotion_Hook  // before drinking, сan decline potion. Doesn't check poisons
    {
    public:
        static void install_hook()
        {
            REL::Relocation<uintptr_t> hook{ RELOCATION_ID(261916, 0) };
            old_func = hook.write_vfunc(0x10F, new_func);
        }
    private:
        static bool new_func(RE::Actor* this_, RE::AlchemyItem* potion, RE::ExtraDataList* extra_data_list)
        {
            if (!this_ || !potion) {
                return old_func(this_, potion, extra_data_list);
            }
            if (!on_drink_potion(this_, potion, extra_data_list))
                return true;                                  // predict from applying
            return old_func(this_, potion, extra_data_list);  // simple applying
        }

        static inline REL::Relocation<decltype(new_func)> old_func;
    };

    class OnApplyPoison_Hook  // apply poison to weapon
    {
    public:
        static void install_hook() { old_func = SKSE::GetTrampoline().write_call<5>(REL::ID(39407).address() + 0x106, new_func); }
    private:
        static void new_func(RE::InventoryEntryData* data, RE::AlchemyItem* poison, int count)
        {
            if (data && poison && count > 0) {
                on_apply_poison(data, poison, count);
            }
            return old_func(data, poison, count);
        }

        static inline REL::Relocation<decltype(new_func)> old_func;
    };

    class OnAdjustActiveEffect_Hook  //    before adjust
    {
    public:
        static void install_hook() { old_func = SKSE::GetTrampoline().write_call<5>(REL::ID(33763).address() + 0x4A3, new_func); }
    private:
        static void new_func(RE::ActiveEffect* this_, float power, bool unk)
        {
            on_adjust_active_effect(this_, power, unk);
            old_func(this_, power, unk);
        }
        static bool new_hooks_installed;
        static inline REL::Relocation<decltype(new_func)> old_func;
    };

    class OnEffectFinish_Hook  // хуки для каждого архетипа эффекта по отдельности. Пока используется ValueModifier через соответствующий класс.
    {
    public:
        static void install_hook()
        {
            REL::Relocation<std::uintptr_t> hook{ RE::ValueModifierEffect::VTABLE[0] };
            old_func = hook.write_vfunc(0x15, new_func_4_valueMod);  //   хук OnEffectFinish для ValueModifier эффектов
        }

    private:
        static void new_func_4_valueMod(RE::ValueModifierEffect* this_)  // works for modav and mod peak av
        {
            if (this_)
                on_valueMod_Effect_Finish(this_);
            old_func(this_);
        }
        static inline REL::Relocation<decltype(new_func_4_valueMod)> old_func;
    };
    inline bool is_playable_spell(RE::SpellItem* spel)
    {
        using ST = RE::MagicSystem::SpellType;
        using AV = RE::ActorValue;
        auto type = spel->GetSpellType();
        if (type == ST::kStaffEnchantment || type == ST::kScroll || type == ST::kSpell || type == ST::kLeveledSpell) {
            RE::ActorValue av = spel->GetAssociatedSkill();
            return av == AV::kAlteration || av == AV::kConjuration || av == AV::kDestruction || av == AV::kIllusion || av == AV::kRestoration;
        }
        return false;
    }

    class CastSpeed_Hook
    {
    public:
        static void install_hook()
        {
            old_func = REL::Relocation<uintptr_t>(RE::VTABLE_ActorMagicCaster[0]).write_vfunc(0x1d, new_func);
        }
    private:
        static void new_func(RE::MagicCaster* mcaster, float dtime)  // mcaster = this
        {
            using State_ = RE::MagicCaster::State;
            auto state = mcaster->state;
            // if (state == static_cast<uint32_t>(S::kUnk01) || state == static_cast<uint32_t>(S::kUnk02))    // old
            if (state == State_::kUnk01 || state == State_::kUnk02) {
                if (RE::Actor* a_ = mcaster->GetCasterAsActor();
                    a_ && a_->IsPlayerRef() && mcaster->currentSpell && mcaster->currentSpell->As<RE::SpellItem>()) {
                    auto spel = mcaster->currentSpell->As<RE::SpellItem>();
                    float time_origin = mcaster->currentSpell->GetChargeTime();
                    float time_new = time_origin * mys::speed_cast_glob->value;
                    float k = time_new > 0.00001f ? time_origin / time_new : 1000000.0f;
                    return old_func(mcaster, dtime * k);
                }
            }
            return old_func(mcaster, dtime);
        }

        static inline REL::Relocation<decltype(new_func)> old_func;
    };

    class OnProjectileLaunch_Hook  //  on projectile laucnhes
    {
    public:
        static void install_hook() { old_func = SKSE::GetTrampoline().write_call<5>(REL::ID(33672).address() + 0x377, new_func); }
    private:
        static void new_func(RE::ProjectileHandle* handle, RE::Projectile::LaunchData& ldata)
        {
            if (handle) {
                on_projectile_launch(handle->get().get(), ldata);
            }
            old_func(handle, ldata);
        }
        static inline REL::Relocation<decltype(new_func)> old_func;
    };

    class OnMagicProjHit_Hook  //  on magic projectile hit target
    {
    public:
        static void install_hook()    {
            old_func = REL::Relocation<uintptr_t>(RE::VTABLE_MissileProjectile[0]).write_vfunc(0xBE, new_func);
        }
    private:
        static bool new_func(RE::Projectile* this_, RE::hkpCollidable*)
        {
            //this_->linearVelocity *= -1;        //  turn proj back
            return false;
        }
        static inline REL::Relocation<decltype(new_func)> old_func;
    };

    class OnArrowMeetsCollision_Hook
    {
    public:
        static void install_hook()    {
            old_func = REL::Relocation<uintptr_t>(RE::VTABLE_ArrowProjectile[0]).write_vfunc(0xBE, new_func);
        }
    private:
        static void new_func(RE::ArrowProjectile* arrow, RE::hkpAllCdPointCollector* collidable)
        {
            bool needDefaultLogic = on_arrow_collide(arrow, collidable);
            if (needDefaultLogic)
                old_func(arrow, collidable);
        }

        static inline REL::Relocation<decltype(new_func)> old_func;
    };

    class OnArrowHit_Hook final  //  arrow hits target (hit event) - not used now
    {
      public:
        static void install_hook() { old_func = SKSE::GetTrampoline().write_call<5>(REL::ID(43027).address() + 0x90, new_func); }
      private:
        static void new_func(RE::Character* attacker, RE::Actor* target, RE::Projectile* proj, bool is_left)
        {
            // not used now
        }
        static inline REL::Relocation<decltype(new_func)> old_func;
    };

    inline float check_MP(float magn)
    {
        float max_MP = (mys::player->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kTemporary, RE::ActorValue::kMagicka) +
                        mys::player->GetActorValueMax(RE::ActorValue::kMagicka));

        if (mys::player->GetActorValue(RE::ActorValue::kMagicka) > (max_MP - (mys::reserved_MP + 2.f))) {
            return 0;
        } 
        else return magn;
    }


    class OnHealthPercentRegen_Hook
    {
      public:
        static void install_hook() { old_func = SKSE::GetTrampoline().write_call<5>(REL::ID(37509).address() + 0x68, new_func); }
      private:
        static void new_func(RE::Actor* actor, RE::ActorValue av, float value)
        {
            // value = 0;  // disable regen
            old_func(actor, av, value);
        }
        static inline REL::Relocation<decltype(new_func)> old_func;
    };

    class OnManaPercentRegen_Hook
    {
      public:
        static void install_hook() { old_func = SKSE::GetTrampoline().write_call<5>(REL::ID(37511).address() + 0x68, new_func); }
      private:
        static void new_func(RE::Actor* actor, RE::ActorValue av, float value)
        {
            if (mys::reserved_MP > 0.f && actor->IsPlayerRef()) {
                value = check_MP(value);
            }
            old_func(actor, av, value);
        }
        static inline REL::Relocation<decltype(new_func)> old_func;
    };

    class OnStaminaPercentRegen_Hook
    {
      public:
        static void install_hook() { old_func = SKSE::GetTrampoline().write_call<5>(REL::ID(37510).address() + 0x176, new_func); }
      private:
        static void new_func(RE::Actor* actor, RE::ActorValue av, float value)
        {
            // value = 0;  // disable regen
            old_func(actor, av, value);
        }
        static inline REL::Relocation<decltype(new_func)> old_func;
    };

    // RE::VTABLE_PlayerCharacter[5] - по этому индексу лежит VFTable ActorValueOwner игрока, если нужно что-то хукать оттуда, например modAV

    class OnModActorValue_Hook
    {
    public:
        static void install_hook()
        {
            REL::Relocation<uintptr_t> hook{ RELOCATION_ID(258043, 0) };
            old_func = hook.write_vfunc(0x20, new_func);
        }
    private:
        static void new_func(RE::ValueModifierEffect* this_, RE::Actor* actor, float value, RE::ActorValue av)
        {
            if (mys::reserved_MP > 0.f && actor->IsPlayerRef() && this_->actorValue == RE::ActorValue::kMagicka) {  // без бафов на резерв маны не уйдем дальше проверки mys::reserved_MP > 0
                if (this_->effect->baseEffect->IsDetrimental()) {
                    old_func(this_, actor, value, av);  //  если эффект detrimental, вообще не трогаем
                    return;
                } 
                else if (this_->effect->baseEffect->data.flags.any(RE::EffectSetting::EffectSettingData::Flag::kRecover)) {
                    // если recover, то пропускаем как обычно т.к. это не реген, а например шмотка на макс. ману
                    old_func(this_, actor, value, av);
                    my::sf_handle_reserved_MP();  // возможно придется пересчитать текущую ману относительно максимальной (можно сократить этот метод)
                    return;
                }
                this_->magnitude = check_MP(this_->magnitude);
            }
            old_func(this_, actor, value, av);
        }
        static inline REL::Relocation<decltype(new_func)> old_func;
    };

    class OnModPeakActorValue_Hook
    {
    public:
        static void install_hook() {
            REL::Relocation<uintptr_t> hook{ RELOCATION_ID(257874, 0) };
            old_func = hook.write_vfunc(0x20, new_func);
        }
    private:
        static void new_func(RE::ValueModifierEffect* this_, RE::Actor* actor, float value, RE::ActorValue av)
        {
            if (mys::reserved_MP > 0.f && actor->IsPlayerRef() && this_->actorValue == RE::ActorValue::kMagicka) {
                if (this_->effect->baseEffect->IsDetrimental() ||
                    this_->effect->baseEffect->data.flags.any(RE::EffectSetting::EffectSettingData::Flag::kRecover)) {
                    old_func(this_, actor, value, av);
                    my::sf_handle_reserved_MP();
                    return;
                }
                this_->magnitude = check_MP(this_->magnitude);
            }
            old_func(this_, actor, value, av);
        }
        static inline REL::Relocation<decltype(new_func)> old_func;
    };

    class OnAdvanceSkill_Hook    // vanilla skill advance / not used
    {
      public:
        static void install_hook()  {
            old_func = REL::Relocation<uintptr_t>(RE::VTABLE_PlayerCharacter[0]).write_vfunc(0xF7, new_func);
        }
      private:
        static void new_func (RE::PlayerCharacter *player, RE::ActorValue a_av, float a_points, RE::TESForm* a_arg3)
        {
            // a_points = 0;   // disable skill advance
            old_func (player, a_av, a_points, a_arg3);
        }
        static inline REL::Relocation<decltype (new_func)> old_func;
    };

    // ------------------------------------------------------ disable shadows ----------------------------------------------------------------

    void stop_shadow_cast(RE::NiAVObject* a_object)  //   NiAVObject -> NiNode -> BSFaceGenNiNode  (inherit)
    {
        RE::BSVisit::TraverseScenegraphGeometries(a_object, [&](RE::BSGeometry* a_geometry) -> RE::BSVisit::BSVisitControl {
            const auto& effect = a_geometry->properties[RE::BSGeometry::States::kEffect];
            const auto lightingShader = netimmerse_cast<RE::BSLightingShaderProperty*>(effect.get());
            if (lightingShader) {
                lightingShader->SetFlags(RE::BSShaderProperty::EShaderPropertyFlag8::kCastShadows, false);  // set CastShadows flag = false
            }
            return RE::BSVisit::BSVisitControl::kContinue;
        });
    }

    class DisableShadowsHook_OnStoreHeadNodes
    {
     public:
        static void install_hook()    {
            old_func = SKSE::GetTrampoline().write_call<5>(RELOCATION_ID(24228, 24732).address() + RELOCATION_OFFSET(0x1CD, 0x15B), new_func);
        }
     private:
        static void new_func(RE::Actor* a_actor, RE::NiAVObject* a_root, RE::BSFaceGenNiNode* a_faceNode)
        {
            old_func(a_actor, a_root, a_faceNode);
            if (a_actor && a_faceNode && !a_actor->IsPlayerRef()) {
                stop_shadow_cast(a_faceNode);
            }
        }
        static inline REL::Relocation<decltype(new_func)> old_func;
    };
    class DisableShadowsHook_OnAttachArmor
    {
     public:
        static void install_hook()    {
            old_func = SKSE::GetTrampoline().write_call<5>(RELOCATION_ID(15501, 15678).address() + RELOCATION_OFFSET(0xA13, 0xB60), new_func);
        }

     private:
        static void new_func(RE::NiAVObject* NiAvObject_, RE::BSFadeNode* a_root3D)
        {
            old_func(NiAvObject_, a_root3D);
            const auto user = (a_root3D && NiAvObject_) ? a_root3D->GetUserData() : nullptr;
            const auto actor = user ? user->As<RE::Actor>() : nullptr;  // check item's actor
            if (actor && !actor->IsPlayerRef()) {
                stop_shadow_cast(NiAvObject_);
            }
        }
        static inline REL::Relocation<decltype(new_func)> old_func;
    };
    class DisableShadowsHook_OnAttachWeapon  // torches/weapons/anything with TESMODEL
    {
     public:
        static void install_hook()  {
            old_func = SKSE::GetTrampoline().write_call<5>(RELOCATION_ID(15569, 15746).address() + RELOCATION_OFFSET(0x2DD, 0x2EC), new_func);
        }
     private:
        static void new_func(RE::NiAVObject* NiAvObject_, RE::BSFadeNode* a_root3D)
        {
            old_func(NiAvObject_, a_root3D);
            const auto user = (a_root3D && NiAvObject_) ? a_root3D->GetUserData() : nullptr;
            const auto actor = user ? user->As<RE::Actor>() : nullptr;  // check item's actor
            if (actor && !actor->IsPlayerRef()) {
                stop_shadow_cast(NiAvObject_);
            }
        }
        static inline REL::Relocation<decltype(new_func)> old_func;
    };

    //----------------------------------------------------------------------------------------------------------------------------------------
    // ------------------------------------------------------ camera -------------------------------------------------------------------------
    //----------------------------------------------------------------------------------------------------------------------------------------

    class On3rdPersonCameraUpdateRot_Hook  // ThirdPersonState::UpdateRotation -> NiQuaternion::FromEulerAnglesXYZ ,  хукаем именно инструмент задающий кварт. внутри функции
    {
     public:
        static void install_hook() { old_func = SKSE::GetTrampoline().write_call<5>(REL::ID(49976).address() + 0xc5, new_func); }
     private:
        static void new_func(RE::NiQuaternion& quat, float x, float y, float z)
        {
            // auto camera = RE::PlayerCamera::GetSingleton();
            // if (!camera->IsInThirdPerson())    camera->ForceThirdPerson();                        // force 3rd person
            // RE::PlayerCamera::GetSingleton()->ToggleFreeCameraMode(false);                      // tfc without freezeTime
            // if (camera->cameraTarget != RE::PlayerCharacter::GetSingleton()->GetHandle()) {}
            //
            // x = x + 0.1f;   // повернуть камеру вверх/вниз
            // y = y - 0.1f;   // повернуть камеру как если наклонять голову влево/вправо. положительные влево, отрицательные вправо
            // z = z + 0.1f;
            //...
            // your logic
            //...
            // log_cam();
            old_func(quat, x, y, z);
            SKSE::log::info("On3rdPersonUpdateRotation");
        }
        static inline REL::Relocation<decltype(new_func)> old_func;
    };

    //------------------ test ----------------------

    class Test_Function_Hook1  // from enable  , test bool ret
    {
     public:
        static void install_hook() {
            old_func = SKSE::GetTrampoline().write_call<5>(REL::ID(18637).address() + 0x156, new_func);  //   from
        }
     private:
        static void new_func(RE::TESObjectCELL* cell)
        {
            SKSE::log::info("CALLED SMTH FROM TESObjectCELL");
            //auto ret = old_func (actorProcess, actor, delta);
            //return ret;
            old_func(cell);
        }
        static inline REL::Relocation<decltype(new_func)> old_func;
    };

    class Test_Function_Hook2  //
    {
     public:
        static void install_hook()    {
            old_func = SKSE::GetTrampoline().write_call<5>(REL::ID(18159).address() + 0x13, new_func);  //   cell
        }
     private:
        static bool new_func(RE::TESForm* someForm)
        {
            SKSE::log::info("CALLED SMTH FROM TESObjectCELL 2");
            //auto ret = old_func (actorProcess, actor, delta);
            //return ret;
            return old_func(someForm);
        }
        static inline REL::Relocation<decltype(new_func)> old_func;
    };

    class Test_VFunc_Hook  //
    {
     public:
        static void install_hook()    {
            old_func = REL::Relocation<uintptr_t>(RE::VTABLE_TESObjectREFR[0]).write_vfunc(0x6A, new_func);
        }
     private:
        static RE::NiAVObject* new_func(RE::TESObjectREFR* this_, bool a_backgroundLoading)
        {
            if (this_) {
                SKSE::log::info("HOOKED 3D LOAD FOR REFR - {}", this_->GetName());
            }
            // code before 3d load
            auto ret = old_func(this_, a_backgroundLoading);
            // code after 3d load
            return ret;
        }
        static inline REL::Relocation<decltype(new_func)> old_func;
    };

    void install_hooks();

};

namespace description_hooks
{
    namespace pch_
    {
        template <class T>
        void write_thunk_call (std::uintptr_t a_src)
        {
            T::old_func = SKSE::GetTrampoline().write_call<5>(a_src, T::new_func);
        }
    }

    static inline std::string debugDescription = "<font color='#AAFF33'> My Custom <font color=\"#FFFFFF\">White</font> Debug Text";

    namespace ItemCardHooks  // хуки заполнения (или размещения) карточек предметов
    {
        struct ItemCardPopulateHook   // хукает заполнение или размещение карточки, мы вставляем свое описание (если нужно) в один из мемберов, а потом в хуках меню fixer его применяет
        {
            static void new_func (RE::ItemCard* itemCard, RE::TESBoundObject** a_item, char a3)
            {
                old_func (itemCard, a_item, a3);                                                        //  вначале выполняем old_func 
                if (itemCard && a_item && *a_item) {
                    on_item_card_upd (itemCard, *a_item);       //  вставляем описание из мапа в карточку, если это нужно данному предмету
                }
            };
            static inline REL::Relocation<decltype(new_func)> old_func;

            static inline void install_hook()
            {
                REL::Relocation<std::uintptr_t> target { RELOCATION_ID(50005, 50949), RELOCATION_OFFSET(0x80, 0x80) };            
                pch_::write_thunk_call<ItemCardPopulateHook>(target.address());

                REL::Relocation<std::uintptr_t> target2 { RELOCATION_ID(50201, 51130), RELOCATION_OFFSET(0xB2, 0xB2) };
                pch_::write_thunk_call<ItemCardPopulateHook>(target2.address());

                REL::Relocation<std::uintptr_t> target3 { RELOCATION_ID(50297, 51218), RELOCATION_OFFSET(0x35, 0x35) };
                pch_::write_thunk_call<ItemCardPopulateHook>(target3.address());

                REL::Relocation<std::uintptr_t> target4 { RELOCATION_ID(50674, 51569), RELOCATION_OFFSET(0x80, 0x7A) };
                pch_::write_thunk_call<ItemCardPopulateHook>(target4.address());

                REL::Relocation<std::uintptr_t> target5 { RELOCATION_ID(50973, 51852), RELOCATION_OFFSET(0x80, 0x7A) };
                pch_::write_thunk_call<ItemCardPopulateHook>(target5.address());

                //if (REL::Module::IsAE()) {
                //    REL::Relocation<std::uintptr_t> target6 { RELOCATION_ID(0, 51458), RELOCATION_OFFSET(0x0, 0x87) };
                //    pch_::write_thunk_call<ItemCardPopulateHook>(target6.address());
                //}
            }
        };

        struct ItemCardPopulateHook2
        {
            static void new_func (RE::ItemCard* itemCard, RE::TESForm* a_item)
            {
                old_func (itemCard, a_item);
                if (itemCard && a_item) {
                    on_item_card_upd(itemCard, a_item);
                }                
            };

            static inline REL::Relocation<decltype(new_func)> old_func;

            static inline void install_hook()
            {
                REL::Relocation<std::uintptr_t> target { RELOCATION_ID(50298, 51219), RELOCATION_OFFSET(0x32, 0x32) };
                pch_::write_thunk_call<ItemCardPopulateHook2>(target.address());

                REL::Relocation<std::uintptr_t> target2 { RELOCATION_ID(51152, 52032), RELOCATION_OFFSET(0x8F, 0x8F) };
                pch_::write_thunk_call<ItemCardPopulateHook2>(target2.address());
            }
        };
    }

    namespace MenuHooks    // menu hooks for xdescriptions
    {

        // функция применения описания. По сути само описание вставляется (если нужно) раньше в хуках карточки, а здесь происходит его apply к предмету
        static inline void applyDescription (RE::IMenu* a_menu, const char* itemCard = "_root.Menu_mc.itemCard")
        {
            RE::GFxValue itemCard_val;
            bool hasItemCard = a_menu->uiMovie->GetVariable(&itemCard_val, itemCard);    // проверяем есть ли карточка у предмета, и сразу получаем ее значение в itemCard_val
            if (!hasItemCard) return;
            ItemCardFixer::applyDescription(itemCard_val);  // отправляем полученное в ItemCardFixer::applyDescription()
        }


        struct InventoryMenuHook   // по-видимому это хук при наведении на предмет в инвентаре. функция должна вернуть RE::UI_MESSAGE_RESULTS , видимо инфа о предмете
        {
            static RE::UI_MESSAGE_RESULTS new_func (RE::IMenu* a_menu, RE::UIMessage& a_message)  
            {
                auto result = old_func (a_menu, a_message);    // результат old_func кладем в переменную
                ItemCardFixer::uiMovie = a_menu->uiMovie;
                applyDescription(a_menu);                      // применяем описание
                return result;
            };
            static inline std::uint32_t idx = 0x4;
            static inline REL::Relocation<decltype(new_func)> old_func;

            static inline void install_hook()
            {
                REL::Relocation<uintptr_t> hook{ RELOCATION_ID(269049, 215494)};
                old_func = hook.write_vfunc(idx, new_func);
            }
        };
        struct InventoryMenuHookTwo
        {
            static RE::UI_MESSAGE_RESULTS new_func (RE::IMenu* a_menu, float a_interval, std::uint32_t a_currentTime)
            {
                auto result = old_func (a_menu, a_interval, a_currentTime);
                applyDescription(a_menu);
                return result;
            };
            static inline std::uint32_t idx = 0x5;
            static inline REL::Relocation<decltype(new_func)> old_func;

            static inline void install_hook()
            {
                REL::Relocation<uintptr_t> hook{RELOCATION_ID(269049, 215494)};
                old_func = hook.write_vfunc(idx, new_func);
            }
        };
        struct BarterMenuHook
        {
            static RE::UI_MESSAGE_RESULTS new_func (RE::IMenu* a_menu, RE::UIMessage& a_message)
            {
                auto result = old_func(a_menu, a_message);
                applyDescription(a_menu);
                return result;
            };
            static inline std::uint32_t idx = 0x4;
            static inline REL::Relocation<decltype(new_func)> old_func;

            static inline void install_hook()
            {
                REL::Relocation<uintptr_t> hook{RELOCATION_ID(267991, 214926)};
                old_func = hook.write_vfunc(idx, new_func);
            }
        };
        struct BarterMenuHookTwo
        {
            static RE::UI_MESSAGE_RESULTS new_func (RE::IMenu* a_menu, float a_interval, std::uint32_t a_currentTime)
            {
                auto result = old_func(a_menu, a_interval, a_currentTime);
                applyDescription(a_menu);
                return result;
            };
            static inline std::uint32_t idx = 0x5;
            static inline REL::Relocation<decltype(new_func)> old_func;

            static inline void install_hook()
            {
                REL::Relocation<uintptr_t> hook{RELOCATION_ID(267991, 214926)};
                old_func = hook.write_vfunc(idx, new_func);
            }
        };
        struct ContainerMenuHook
        {
            static RE::UI_MESSAGE_RESULTS new_func(RE::IMenu* a_menu, RE::UIMessage& a_message)
            {
                auto result = old_func(a_menu, a_message);
                applyDescription(a_menu);
                return result;
            };
            static inline std::uint32_t idx = 0x4;
            static inline REL::Relocation<decltype(new_func)> old_func;

            static inline void install_hook()
            {
                REL::Relocation<uintptr_t> hook{RELOCATION_ID(268222, 215061)};
                old_func = hook.write_vfunc(idx, new_func);
            }
        };
        struct ContainerMenuHookTwo
        {
            static RE::UI_MESSAGE_RESULTS new_func(RE::IMenu* a_menu, float a_interval, std::uint32_t a_currentTime)
            {
                auto result = old_func(a_menu, a_interval, a_currentTime);
                applyDescription(a_menu);
                return result;
            };
            static inline std::uint32_t idx = 0x5;
            static inline REL::Relocation<decltype(new_func)> old_func;

            static inline void install_hook()
            {
                REL::Relocation<uintptr_t> hook{RELOCATION_ID(268222, 215061)};
                old_func = hook.write_vfunc(idx, new_func);
            }
        };
        struct GiftMenuHook
        {
            static RE::UI_MESSAGE_RESULTS new_func(RE::IMenu* a_menu, RE::UIMessage& a_message)
            {
                auto result = old_func(a_menu, a_message);
                applyDescription(a_menu);
                return result;
            };
            static inline std::uint32_t idx = 0x4;
            static inline REL::Relocation<decltype(new_func)> old_func;

            static inline void install_hook()
            {
                REL::Relocation<uintptr_t> hook{RELOCATION_ID(268697, 215323)};
                old_func = hook.write_vfunc(idx, new_func);
            }
        };
        struct GiftMenuHookTwo
        {
            static RE::UI_MESSAGE_RESULTS new_func(RE::IMenu* a_menu, float a_interval, std::uint32_t a_currentTime)
            {
                auto result = old_func(a_menu, a_interval, a_currentTime);
                applyDescription(a_menu);
                return result;
            };
            static inline std::uint32_t idx = 0x5;
            static inline REL::Relocation<decltype(new_func)> old_func;

            static inline void install_hook()
            {
                REL::Relocation<uintptr_t> hook{RELOCATION_ID(268697, 215323)};
                old_func = hook.write_vfunc(idx, new_func);
            }
        };
        struct MagicMenuHook
        {
            static RE::UI_MESSAGE_RESULTS new_func(RE::IMenu* a_menu, RE::UIMessage& a_message)
            {
                auto result = old_func(a_menu, a_message);
                applyDescription(a_menu);
                return result;
            };
            static inline std::uint32_t idx = 0x4;
            static inline REL::Relocation<decltype(new_func)> old_func;

            static inline void install_hook()
            {
                REL::Relocation<uintptr_t> hook{RELOCATION_ID(269321, 215652)};
                old_func = hook.write_vfunc(idx, new_func);
            }
        };
        struct MagicMenuHookTwo
        {
            static RE::UI_MESSAGE_RESULTS new_func(RE::IMenu* a_menu, float a_interval, std::uint32_t a_currentTime)
            {
                auto result = old_func(a_menu, a_interval, a_currentTime);
                applyDescription(a_menu);
                return result;
            };
            static inline std::uint32_t idx = 0x5;
            static inline REL::Relocation<decltype(new_func)> old_func;

            static inline void install_hook()
            {
                REL::Relocation<uintptr_t> hook{ RELOCATION_ID(269321, 215652) };
                old_func = hook.write_vfunc(idx, new_func);
            }
        };
        struct CraftingMenuHook
        {
            static RE::UI_MESSAGE_RESULTS new_func(RE::CraftingMenu* a_menu, RE::UIMessage& a_message)
            {
                auto result = old_func(a_menu, a_message);
                applyDescription(a_menu, "_root.Menu.ItemInfo");
                return result;
            };
            static inline std::uint32_t idx = 0x4;
            static inline REL::Relocation<decltype(new_func)> old_func;

            static inline void install_hook()
            {
                REL::Relocation<uintptr_t> hook{ RELOCATION_ID(268432, 215111) };
                old_func = hook.write_vfunc(idx, new_func);
            }
        };
        struct CraftingMenuHookTwo
        {
            static RE::UI_MESSAGE_RESULTS new_func(RE::IMenu* a_menu, float a_interval, std::uint32_t a_currentTime)
            {
                auto result = old_func(a_menu, a_interval, a_currentTime);
                applyDescription(a_menu, "_root.Menu.ItemInfo");
                return result;
            };
            static inline std::uint32_t idx = 0x5;
            static inline REL::Relocation<decltype(new_func)> old_func;

            static inline void install_hook()
            {
                REL::Relocation<uintptr_t> hook{ RELOCATION_ID(268432, 215111) };
                old_func = hook.write_vfunc(idx, new_func);
            }
        };

        static void Menu_Hooks_Install()
        {
            InventoryMenuHook::install_hook();
            InventoryMenuHookTwo::install_hook();
            BarterMenuHook::install_hook();
            BarterMenuHookTwo::install_hook();
            ContainerMenuHookTwo::install_hook();
            ContainerMenuHook::install_hook();
            GiftMenuHook::install_hook();
            GiftMenuHookTwo::install_hook();
            //MagicMenuHook::install_hook();
            //MagicMenuHookTwo::install_hook();
            //CraftingMenuHook::install_hook();
            //CraftingMenuHookTwo::install_hook();
        }
    }

    static inline void Install_Hooks()
    {
        ItemCardHooks::ItemCardPopulateHook::install_hook();
        ItemCardHooks::ItemCardPopulateHook2::install_hook();
        MenuHooks::Menu_Hooks_Install();
    }
};

#undef RELOCATION_OFFSET

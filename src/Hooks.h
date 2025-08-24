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
                on_my_update(player_);
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
        static void install_hook() {old_func = SKSE::GetTrampoline().write_call<5>(REL::ID(37013).address() + 0x1A, new_func);}
      private:
        static float new_func(RE::Actor* actor)
        {
            float hiddenMult = old_func(actor);       // default func is called
            on_hidden_speed_mult (actor, hiddenMult);
            return hiddenMult - mys::ms_compensator;
        }
        static inline REL::Relocation<decltype(new_func)> old_func;
    };

    class OnMovementDelta_Hook    // мб расстояние движения за 1 кадр. потестить. Если ничего не возвращать персонаж не будет двигаться.
    {
      public:
        static void install_hook() {old_func = SKSE::GetTrampoline().write_call<5>(REL::ID(36359).address() + 0xF0, new_func);}
      private:
        static void new_func (RE::Actor* actor, float delta)
        {
            // return;  // disable movement
            return old_func(actor, delta);
        }
        static inline REL::Relocation<decltype(new_func)> old_func;
    };

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
        static void install_hook() {
            old_func = REL::Relocation<uintptr_t>(RE::VTABLE_TESObjectREFR[0]).write_vfunc(0x66, new_func);
        }
    private:
        static void new_func(RE::TESObjectREFR* this_)
        {
            if (this_->GetBaseObject()){
                if (this_->GetBaseObject()->GetFormType() == RE::FormType::Misc) {
                    this_->DetachHavok(this_->GetCurrent3D());  // disable havok and activation (object will become full static)
                    return;  // don't init havok
                }
            }
            old_func(this_);
        }
        static inline REL::Relocation<decltype(new_func)> old_func;
    };
    
    class OnLoad3D_Hook  // virt. Load3D()    // early spawn  // not fully loaded yet
    {
      public:
        static void install_hook() {
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

    class OnActorSpawnFinish_Hook   // Actor::ShouldBeRagdolled() from Actor::InitHavokHelper()  // at the end of appearance
    {
      public:
        static void install_hook() { old_func = SKSE::GetTrampoline().write_call<5>(REL::ID(36192).address() + 0x5de, new_func); }
      private:
        static bool new_func (RE::Actor* actor)  // actor already has HighProcess, 3D, CharController(havok), MovementController
        {
            if (actor) on_actor_spawn_finish (actor);
            return old_func(actor);
        }
        static inline REL::Relocation<decltype(new_func)> old_func;
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
            old_func_pc(this_, obj, count, arg3, playSound);        // count = 0 to disable pickUp
        }
        static void new_func_npc(RE::Character* this_, RE::TESObjectREFR* obj, std::int32_t count, bool arg3 = false, bool playSound = true)
        {
            on_pickup_object (this_, obj, count, arg3, playSound);
            old_func_npc (this_, obj, count, arg3, playSound);
        }
        static inline REL::Relocation<decltype(new_func_pc)> old_func_pc;
        static inline REL::Relocation<decltype(new_func_npc)> old_func_npc;
    };

    class OnEquip_Hook    // ActorEquipManager::Equip()
    {
      public:
        static void install_hook()    {
            old_func = SKSE::GetTrampoline().write_call<5>(REL::ID(37938).address() + RELOCATION_OFFSET(0xE5, 0x170), new_func);
        }
      private:
        static void new_func(RE::ActorEquipManager* equip_manager, RE::Actor* actor, RE::TESBoundObject* bound_object, RE::ObjectEquipParams* params)
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

    class OnDispel_Hook  // dispel effect
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
  
    class OnDamageFromWater_Hook  //  sinking w/o oxygen and dangerous water  // every frame // Actor::DoDamage()  from  Actor::Update()
    {
      public:
        static void install_hook() { 
            old_func_drown  = SKSE::GetTrampoline().write_call<5>(REL::ID(36357).address() + 0x8a3, new_func_drown);
            old_func_danger = SKSE::GetTrampoline().write_call<5>(REL::ID(36357).address() + 0xfb4, new_func_danger); 
        }
      private:
        static bool new_func_drown (RE::Actor* actor, float damage, RE::Actor* attacker, bool a4) // only player can drown
        {
            return old_func_drown (actor, damage, attacker, a4);
        }
        static bool new_func_danger (RE::Actor* actor, float damage, RE::Actor* attacker, bool a4)  // dangerous water affects any npc
        {
            return old_func_danger (actor, damage, attacker, a4);
        }
        static inline REL::Relocation<decltype(new_func_drown)> old_func_drown;
        static inline REL::Relocation<decltype(new_func_danger)> old_func_danger;
    };

    class OnTrapDamage_Hook  // all physical traps including spike-elevator platform, don't handle mgeffect dmg like poison spike
    {
      public:
        static void install_hook() { 
            old_func1  = SKSE::GetTrampoline().write_call<5>(REL::ID(36508).address() + 0x1ae, new_func1);
            old_func2 = SKSE::GetTrampoline().write_call<5>(REL::ID(36509).address() + 0x43, new_func2); 
        }
      private:
        static bool new_func1 (RE::Actor* actor, float damage, RE::Actor* attacker, bool a4) // Actor::DoDamage() from Actor::DoTrap2()
        {
            //return false; // disable dmg
            return old_func1 (actor, damage, attacker, a4);
        }
        static void new_func2 (RE::Actor* actor, RE::HitData &hitData)  // Actor::DoHitMe() from Actor::DoTrap1()
        {
            //return; // disable dmg
            return old_func2 (actor, hitData);
        }
        static inline REL::Relocation<decltype(new_func1)> old_func1;
        static inline REL::Relocation<decltype(new_func2)> old_func2;
    };

    class OnCalcFallDamage_Hook  //  from height and ragdoll (after land surfing)
    {
      public:
        static void install_hook() { 
            old_func_height  = SKSE::GetTrampoline().write_call<5>(REL::ID(36973).address() + 0xae, new_func_height); 
            old_func_ragdoll = SKSE::GetTrampoline().write_call<5>(REL::ID(36346).address() + 0x35, new_func_ragdoll);
        }
      private:
        static float new_func_height(RE::Actor* actor, float height, float a3)
        {
            // сделать тут эпик приземление в тяже если пролетел больше 1-2м
            //return 0;   // disable dmg
            return old_func_height (actor, height, a3);   // default
        }
		static float new_func_ragdoll(RE::Actor* actor, float height, float a3)
        {
            return 0;  // ragdoll dmg disabled
            // return old_func_ragdoll (actor, height, a3);   // default
        }
        static inline REL::Relocation<decltype(new_func_height)> old_func_height;
        static inline REL::Relocation<decltype(new_func_ragdoll)> old_func_ragdoll;
    };

    class OnOpenConsole_Hook  
    {
      public:
        static void install_hook() { old_func = SKSE::GetTrampoline().write_call<5>(REL::ID(51395).address() + 0x13, new_func); }
      private:
        static void new_func () // Console::ToggleOpenConsole_14085AA30()
        {
            //return;          // disable open console 
            old_func();    // default open console
        }
        static inline REL::Relocation<decltype(new_func)> old_func;
    };
	 
    class OnConsoleInputHook    // Script::CompileAndRun_1402E75F0
    {
      public:
        static void install_hook() { old_func = SKSE::GetTrampoline().write_call<5>(REL::ID(52065).address() + 0xE2, new_func);}
      private:
        static void new_func (RE::Script* a_script, RE::ScriptCompiler* a_compiler, RE::COMPILER_NAME a_name, RE::TESObjectREFR* a_targetRef) 
        {   //                                                                      kSystemWindowCompiler = 1
            std::string enteredCommand = a_script->GetCommand();
            old_func (a_script, a_compiler, a_name, a_targetRef);
        }
        static inline REL::Relocation<decltype(new_func)> old_func;
    };

    class OnConsoleKill_Hook  // Actor::Kill_1405D4700 from kill / killall commands
    {
      public:
        static void install_hook() { 
            old_func1 = SKSE::GetTrampoline().write_call<5>(REL::ID(21553).address() + 0xf0, new_func1); 
            old_func2 = SKSE::GetTrampoline().write_call<5>(REL::ID(21554).address() + 0xa7, new_func2);
        }
      private:
        static void new_func1 (RE::Actor *victim, RE::Actor *killer, float a3, bool a4, bool a5)
        {
            if (victim && victim->HasKeyword(mys::actorBoss)) {
                old_func1 (mys::player, killer, a3, a4, a5);  // kill command -> punish
                RE::ConsoleLog::GetSingleton()->Print("Магия консоли была сильна, но босс оказался сильнее..");
            }
            else old_func1 (victim, killer, a3, a4, a5);
        }
		static void new_func2 (RE::Actor *victim, RE::Actor *killer, float a3, bool a4, bool a5)
        {
            if (victim && victim->HasKeyword(mys::actorBoss)) {
                old_func2(mys::player, killer, a3, a4, a5);  // killall command -> punish
                RE::ConsoleLog::GetSingleton()->Print("Магия консоли была сильна, но босс оказался сильнее..");
            }
            else old_func2 (victim, killer, a3, a4, a5);
        }
        static inline REL::Relocation<decltype(new_func1)> old_func1;
        static inline REL::Relocation<decltype(new_func2)> old_func2;
    };

    class OnQuickSaveLoadProcessButton_Hook
    {
      public:
        static void install_hook() {old_func = REL::Relocation<uintptr_t>(RE::VTABLE_QuickSaveLoadHandler[0]).write_vfunc(0x5, new_func);}
      private:
        static void new_func(void* this_, void* a2)
        {
            return;  // disable quick save/load button processing
        }
        static inline REL::Relocation<decltype(new_func)> old_func;
    };

    class OnCalcAttackStamina_Hook  // power attack, power bash etc.
    {
      public:
        static void install_hook() { 
            old_func1 = SKSE::GetTrampoline().write_call<5>(REL::ID(28629).address() + 0x1561, new_func1); 
            old_func2 = SKSE::GetTrampoline().write_call<5>(REL::ID(37650).address() + 0x16e,  new_func2);
            old_func3 = SKSE::GetTrampoline().write_call<5>(REL::ID(38047).address() + 0xbb,   new_func3);
            old_func4 = SKSE::GetTrampoline().write_call<5>(REL::ID(48139).address() + 0x29b,  new_func4);
        }
      private:
        static float new_func1 (RE::ActorValueOwner *avOwner, RE::BGSAttackData *atkData)  {
            return old_func1 (avOwner, atkData);
        }
        static float new_func2 (RE::ActorValueOwner *avOwner, RE::BGSAttackData *atkData)  {;
            return old_func2 (avOwner, atkData);
        }
        static float new_func3 (RE::ActorValueOwner *avOwner, RE::BGSAttackData *atkData)  {
            return old_func3 (avOwner, atkData);
        }
        static float new_func4 (RE::ActorValueOwner *avOwner, RE::BGSAttackData *atkData)  {
            return old_func4 (avOwner, atkData);
        }
        static inline REL::Relocation<decltype(new_func1)> old_func1;
        static inline REL::Relocation<decltype(new_func2)> old_func2;
        static inline REL::Relocation<decltype(new_func3)> old_func3;
        static inline REL::Relocation<decltype(new_func4)> old_func4;
    };

    class OnCalcSprintStamina_Hook  // sprint st drain per frame 
    {
      public:
        static void install_hook() { old_func = SKSE::GetTrampoline().write_call<5>(REL::ID(36994).address() + 0xc9, new_func); }
      private:
        static float new_func (float currStamina, float a_delta)
        {
            return old_func(currStamina, a_delta);
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

    class OnMeleeWeapCollide_Hook  //  melee weapon collides target.  Actor::CombatHit_140628C20  from  Actor::DoMeleeAttack_140627930   // has target
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

    class OnDoMeleeAttack_Hook   // Actor::DoMeleeAttack_140627930 from HitFrameHandler::Handle()    // before all, has no target yet
    {
      public:
        static void install_hook() { old_func = SKSE::GetTrampoline().write_call<5>(REL::ID(41747).address() + 0x3a, new_func); }

      private:
        static void new_func(RE::Actor* actor, bool mbLeftHand, bool a3)  // Actor::DoMeleeAttack_140627930
        {
            SKSE::log::info("called OnDoMeleeAttack");
            //old_func(actor, mbLeftHand, a3);
        }
        static inline REL::Relocation<decltype(new_func)> old_func;
    };

    class OnSetLifeState_Hook  // kBleedout, kEssentialDown, kDying, kDead, kDontMove etc.
    {
      public:
        static void install_hook() { old_func = SKSE::GetTrampoline().write_call<5>(REL::ID(36872).address() + 0x6c4, new_func); } 
      private:
        static void new_func (RE::Actor* actor, RE::ACTOR_LIFE_STATE state)  // SetLifeState from KillImpl (sets essentialDown there)
        {
            //if (actor->HasKeyword(ourPlayersNpcKeyword)) actor->StopCombat();  // end combat + not fall
            //else old_func (actor, state);
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
      public:                                                           //  works for shout to check kd, if force true we can shout even in kd
        static void install_hook()    {
            old_func = REL::Relocation<uintptr_t>(RE::VTABLE_ActorMagicCaster[0]).write_vfunc(0xA, new_func);
        }
      private:
        static bool new_func (RE::ActorMagicCaster* this_, RE::MagicItem* spell, bool dualCast, float* alchStrength, RE::MagicSystem::CannotCastReason* reason, bool useBaseValueForCost)
        {
            if (on_check_cast (this_, spell, dualCast)) // our checks and logic
                return old_func (this_, spell, dualCast, alchStrength, reason, useBaseValueForCost); // game checks
            else return false;  // now allow cast from our side
        }
        static inline REL::Relocation<decltype(new_func)> old_func;
    };

    class OnCheckAbsorb_Hook   //  virt.
    {
      public:
        static void install_hook()    {
            old_func_npc = REL::Relocation<uintptr_t>(RE::VTABLE_Character[4]).write_vfunc(0x0B, new_func_npc);
            old_func_pc  = REL::Relocation<uintptr_t>(RE::VTABLE_PlayerCharacter[4]).write_vfunc(0x0B, new_func_pc);
        }
      private:
        static bool new_func_npc (RE::MagicTarget* mtarget, RE::Actor* caster, RE::MagicItem* mItem, const RE::Effect* effect)
        {
            int8_t retcode = on_check_absorb (mtarget, caster, mItem, effect);
            if      (retcode == 1) return true;    // force true but w/o default processings (mana, fx) (won't absorb ignore-absorb spells anyway)
            else if (retcode == 0) return false;   // don't process absorb
            else return old_func_npc(mtarget, caster, mItem, effect); // == -1   // default process absorb 
        }
        static bool new_func_pc (RE::MagicTarget* mtarget, RE::Actor* caster, RE::MagicItem* mItem, const RE::Effect* effect)
        {
            int8_t retcode = on_check_absorb (mtarget, caster, mItem, effect);
            if      (retcode == 1) return true;    // 
            else if (retcode == 0) return false;   // 
            else return old_func_pc (mtarget, caster, mItem, effect); // == -1
        }
        static inline REL::Relocation<decltype(new_func_npc)> old_func_npc;
        static inline REL::Relocation<decltype(new_func_pc)>  old_func_pc;
    };

    class OnAddMagicTargetHook   //  virt. MagicTarget::AddTarget()  - works before add effect, before absorb check, before adjustActiveEffect, processes inactive effects
    {
      public:
        static void install_hook()    {
            old_func_npc = REL::Relocation<uintptr_t>(RE::VTABLE_Character[4]).write_vfunc(0x1, new_func_npc);
            old_func_pc  = REL::Relocation<uintptr_t>(RE::VTABLE_PlayerCharacter[4]).write_vfunc(0x1, new_func_pc);
        }
      private:
        static bool new_func_npc (RE::MagicTarget* mtarget, RE::MagicTarget::AddTargetData &a_targetData)
        {
            if (on_add_effect (mtarget, a_targetData)) return old_func_npc(mtarget, a_targetData);  // default spell apply
            else return false;   // decline
        }
        static bool new_func_pc (RE::MagicTarget* mtarget, RE::MagicTarget::AddTargetData &a_targetData)
        {
            if (on_add_effect (mtarget, a_targetData)) return old_func_pc (mtarget, a_targetData);
            else return false;   // 
        }
        static inline REL::Relocation<decltype(new_func_npc)> old_func_npc;
        static inline REL::Relocation<decltype(new_func_pc)>  old_func_pc;

    };

    class OnAdjustActiveEffect_Hook  //   ActiveEffect::Adjust()  from  MagicTarget::CheckAddEffect()
    {
      public:
        static void install_hook() { old_func = SKSE::GetTrampoline().write_call<5>(REL::ID(33763).address() + 0x4A3, new_func); }
      private:
        static void new_func(RE::ActiveEffect* eff, float power, bool unk)
        {
            on_adjust_active_effect(eff, power, unk);
            old_func(eff, power, unk);
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
            if (!this_ || !magic_item || !effect) return old_func_pc (this_, magic_item, effect, bound_object);
            else return on_resist_apply (this_, magic_item, effect);
        }
        static float new_func_npc (RE::MagicTarget* this_, RE::MagicItem* magic_item, RE::Effect* effect, RE::TESBoundObject* bound_object)
        {
            if (!this_ || !magic_item || !effect) return old_func_npc (this_, magic_item, effect, bound_object);
			else return on_resist_apply (this_, magic_item, effect);
        }
        static inline REL::Relocation<decltype(new_func_pc)> old_func_pc;
        static inline REL::Relocation<decltype(new_func_npc)> old_func_npc;
    };

    class OnPlayerDrinkPotion_Hook  // before drinking. Doesn't check poison
    {
      public:
        static void install_hook() {
            old_func = REL::Relocation<uintptr_t>(RE::VTABLE_PlayerCharacter[0]).write_vfunc(0x10F, new_func);
        }
      private:
        static bool new_func(RE::Actor* this_, RE::AlchemyItem* potion, RE::ExtraDataList* extra_data_list)
        {
            if (this_ && potion) {
                if (!on_drink_potion(this_, potion, extra_data_list))
                    return true;   // cancel applying potion
            }                              
            return old_func(this_, potion, extra_data_list);
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
        static void install_hook() {
            old_func = REL::Relocation<uintptr_t>(RE::VTABLE_ActorMagicCaster[0]).write_vfunc(0x1d, new_func);
        }
      private: 
        static void new_func (RE::MagicCaster* mcaster, float dtime)  // mcaster = this
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

    struct OnGetBowPower_Hook   // players current bow drawning power
    {
      public:
        static void install_hook() {old_func = SKSE::GetTrampoline().write_call<5>(REL::ID(42928).address() + 0x604, new_func);}
      private:
        static float new_func (float draw_power, float weapon_speed)
        {
           float amount = old_func (draw_power, weapon_speed);
           return amount;
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

    class OnProjectileHandleSpellCollision_Hook  //  Projectile::HandleSpellCollision
    {
      public:
        static void install_hook() { old_func = SKSE::GetTrampoline().write_call<5>(REL::ID(43013).address() + 0x251, new_func); }
      private:
        static bool new_func(RE::Projectile* proj, RE::TESObjectREFR* refr, RE::NiPoint3* location, 
			                 RE::hkVector4* unk, RE::COL_LAYER collisionLayer, RE::MATERIAL_ID materialID, bool* handled)
        {
            if (collisionLayer == RE::COL_LAYER::kTrigger) {    // stairs expl multiple damage fix (trouble in locations triggers collision layer)
                *handled = true; 
                return false;
            }
            return old_func (proj, refr, location, unk, collisionLayer, materialID, handled);
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

    class OnEffectFinish_Hook   // virt Finish() ; for modav and peak av
    {
      public:
        static void install_hook() { 
            old_func_valMod  = REL::Relocation<uintptr_t>(RE::ValueModifierEffect::VTABLE[0]).write_vfunc(0x15, new_func_valMod);
            old_func_pvalMod = REL::Relocation<uintptr_t>(RE::PeakValueModifierEffect::VTABLE[0]).write_vfunc(0x15, new_func_pvalMod);
        }
      private:
        static void new_func_valMod(RE::ValueModifierEffect* this_)
        {
            if (this_) on_valueMod_Effect_Finish(this_);
            old_func_valMod(this_);
        }
        static void new_func_pvalMod(RE::PeakValueModifierEffect* this_)
        {
            if (this_) on_valueMod_Effect_Finish(this_);
            old_func_pvalMod(this_);
        }
        static inline REL::Relocation<decltype(new_func_valMod)>  old_func_valMod;
        static inline REL::Relocation<decltype(new_func_pvalMod)> old_func_pvalMod;
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

    //----------------------------------------------------------------------------------------------------------------------------------------
    // ------------------------------------------------------ HUD ----------------------------------------------------------------------------
    //----------------------------------------------------------------------------------------------------------------------------------------

    class OnMenusProcessMsg_Hook  // disable all game menus
    {
      public:
        static void install_hook() {
            old_func1  = REL::Relocation<uintptr_t>(RE::VTABLE_JournalMenu[0]).write_vfunc(0x4, new_func1);
            old_func2  = REL::Relocation<uintptr_t>(RE::VTABLE_MagicMenu[0]).write_vfunc(0x4, new_func2);
            //...
        }
      private:
        using MSG = RE::UI_MESSAGE_RESULTS; 
        static MSG new_func1 (RE::JournalMenu* menu, RE::UIMessage& msg) {return MSG::kIgnore;}  // disable menu
        static MSG new_func2 (RE::MagicMenu* menu, RE::UIMessage& msg)   {return MSG::kIgnore;}
        //...
        static inline REL::Relocation<decltype (new_func1)> old_func1;
        static inline REL::Relocation<decltype (new_func2)> old_func2;
        //..
    };

    class OnEnemyHealthHUD_Hook  // enemy health bar
    {
      public:
        static void install_hook() {old_func = REL::Relocation<uintptr_t>(RE::VTABLE_EnemyHealth[0]).write_vfunc(0x2, new_func);}
      private:
        static bool new_func (void* this_, RE::UIMessage* a_uiMessage)
        {
            return old_func (this_, a_uiMessage);
           //return false;  // disable enemy health bar
        }
        static inline REL::Relocation<decltype(new_func)> old_func;
    };


    //------------------ test ----------------------

    // 

	//------------------++
    void install_hooks();
	//------------------++
};

class DisableAIProcess_Hook
{
  public:
    static void install_hook() {
        old_func = SKSE::GetTrampoline().write_call<5>(REL::ID(40432).address() + 0x29, new_func);
    }
  private:
    static void new_func (uint32_t &handle, float deltaZero, bool a3)
    {
       RE::ActorPtr outActor;
       RE::LookupReferenceByHandle (handle, outActor);
       if (auto actor = outActor.get()) {
           //if (actor->HasKeyword(PlayerNpcKeyword)) {
           //    SKSE::log::info("disable processing for actor - {}", actor->GetName());
           //    return;
           //}
       }
       old_func (handle, deltaZero, a3);   // for other npc default processing
    }
    static inline REL::Relocation<decltype(new_func)> old_func;
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
                   on_item_card_upd (itemCard, *a_item, ItemCardFixer::uiMovie);       //  вставляем описание из мапа в карточку, если это нужно данному предмету
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
                    on_item_card_upd (itemCard, a_item, ItemCardFixer::uiMovie);
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
            ItemCardFixer::applyDescription(itemCard_val, a_menu);  // отправляем полученное в ItemCardFixer::applyDescription()
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
            static inline REL::Relocation<decltype(new_func)> old_func;

            static inline void install_hook() {
                REL::Relocation<uintptr_t> hook{ RELOCATION_ID(269049, 215494)};
                old_func = hook.write_vfunc(0x4, new_func);
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
            static inline REL::Relocation<decltype(new_func)> old_func;

            static inline void install_hook() {
                REL::Relocation<uintptr_t> hook{RELOCATION_ID(269049, 215494)};
                old_func = hook.write_vfunc(0x5, new_func);
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
            static inline REL::Relocation<decltype(new_func)> old_func;

            static inline void install_hook() {
                REL::Relocation<uintptr_t> hook{RELOCATION_ID(267991, 214926)};
                old_func = hook.write_vfunc(0x4, new_func);
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
            static inline REL::Relocation<decltype(new_func)> old_func;

            static inline void install_hook() {
                REL::Relocation<uintptr_t> hook{RELOCATION_ID(267991, 214926)};
                old_func = hook.write_vfunc(0x5, new_func);
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
            static inline REL::Relocation<decltype(new_func)> old_func;

            static inline void install_hook() {
                REL::Relocation<uintptr_t> hook{RELOCATION_ID(268222, 215061)};
                old_func = hook.write_vfunc(0x4, new_func);
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
            static inline REL::Relocation<decltype(new_func)> old_func;

            static inline void install_hook() {
                REL::Relocation<uintptr_t> hook{RELOCATION_ID(268222, 215061)};
                old_func = hook.write_vfunc(0x5, new_func);
            }
        };
        struct GiftMenuHook
        {
            static RE::UI_MESSAGE_RESULTS new_func (RE::IMenu* a_menu, RE::UIMessage& a_message)
            {
                auto result = old_func(a_menu, a_message);
                applyDescription(a_menu);
                return result;
            };
            static inline REL::Relocation<decltype(new_func)> old_func;

            static inline void install_hook()  {
                REL::Relocation<uintptr_t> hook{RELOCATION_ID(268697, 215323)};
                old_func = hook.write_vfunc(0x4, new_func);
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
            static inline REL::Relocation<decltype(new_func)> old_func;

            static inline void install_hook()  {
                REL::Relocation<uintptr_t> hook{RELOCATION_ID(268697, 215323)};
                old_func = hook.write_vfunc(0x5, new_func);
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
            static inline REL::Relocation<decltype(new_func)> old_func;

            static inline void install_hook()  {
                REL::Relocation<uintptr_t> hook{RELOCATION_ID(269321, 215652)};
                old_func = hook.write_vfunc(0x4, new_func);
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
            static inline REL::Relocation<decltype(new_func)> old_func;

            static inline void install_hook() {
                REL::Relocation<uintptr_t> hook{ RELOCATION_ID(269321, 215652) };
                old_func = hook.write_vfunc(0x5, new_func);
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
            static inline REL::Relocation<decltype(new_func)> old_func;

            static inline void install_hook() {
                REL::Relocation<uintptr_t> hook{ RELOCATION_ID(268432, 215111) };
                old_func = hook.write_vfunc(0x4, new_func);
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
            static inline REL::Relocation<decltype(new_func)> old_func;

            static inline void install_hook() {
                REL::Relocation<uintptr_t> hook{ RELOCATION_ID(268432, 215111) };
                old_func = hook.write_vfunc(0x5, new_func);
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

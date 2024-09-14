
#pragma once

#include "core.h"
#include "ItemCardFixer.h"
#include "InputWatcher.h"
#include "logger.h"
//#include "PCH.h"


static float upd_timer = 2.0f;   // 2 sec

struct OnPlayerUpdate_Hook final
{
public:
    static void install_hook()                                                     
    {
        REL::Relocation<uintptr_t> update_pl_address_{RELOCATION_ID(261916, 0)};
        old_func = update_pl_address_.write_vfunc(0xAD, update_pc);                     //  vfunc
    }
 private:
    static void update_pc(RE::PlayerCharacter *player_, float delta)
    {
      
        mys::time_delta = delta;        // for other functions

		mys::micro_timer -= delta;
        upd_timer -= delta;

		//if (mys::micro_timer <= 0.f) {
        //    mys::micro_timer = 1.f;
        //    on_micro_update();           // every 1 sec
        //}

        if (upd_timer <= 0.f) {
            upd_timer = 2.0f;            // every 2 sec
            on_my_update();
        }
        return old_func(player_, delta);
    }

    static inline  REL::Relocation<decltype(update_pc)>  old_func;
}; 


class HiddenSpeedMult_updater_Hook        // returns actor hidden speed mult, default 1.0
{                              
public:
    static void install_hook() {

        old_func = SKSE::GetTrampoline().write_call<5>(REL::RelocationID(37013, 37943).address() + REL::Relocate(0x1A, 0x51), new_func);
    }
private:
    static float new_func(RE::Actor* a_actor)
    {
        float hiddenMult = old_func(a_actor);

        if (a_actor->IsPlayerRef() && mys::dodge_timer > 0)
        {
            mys::dodge_timer -= mys::time_delta;
            hiddenMult += 2;
        }
        return hiddenMult - mys::ms_compensator;    
    }
    
    static inline REL::Relocation<decltype(new_func)> old_func;
};



class OnEquip_Hook
{
 public:
    static void install_hook ()  {
     
        old_func = SKSE::GetTrampoline().write_call<5>(REL::ID(37938).address() + 0xE5, new_func);
        // old_func_defaultSlot = SKSE::GetTrampoline().write_call<5>(REL::ID(37938).address() + 0x7F, new_func);    // 
    }
 private:
    static void new_func(RE::ActorEquipManager* equip_manager, RE::Actor* actor, RE::TESBoundObject* bound_object, void* extra_data_list)
    {
        if (!equip_manager || !actor || !bound_object || !extra_data_list) {
            return old_func (equip_manager, actor, bound_object, extra_data_list);
        }
        on_equip(actor, bound_object);
        return old_func(equip_manager, actor, bound_object, extra_data_list);
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
    static void new_func(RE::ActorEquipManager* equip_manager, RE::Actor* actor, RE::TESBoundObject* bound_object, void* extra_data_list)
    {
        if (!equip_manager || !actor || !bound_object || !extra_data_list) {
            return old_func(equip_manager, actor, bound_object, extra_data_list);
        }
        on_unequip(actor, bound_object);
        return old_func(equip_manager, actor, bound_object, extra_data_list);
    }

    static inline REL::Relocation<decltype(new_func)> old_func;
};



class On_PlayerAnimEvent_Hook {
public:
    static void install_hook() {

        REL::Relocation<uintptr_t>  hook {RELOCATION_ID(261918, 0)};

        old_func = hook.write_vfunc(0x1, new_func);
    }

private:
    static auto new_func (RE::BSTEventSink<RE::BSAnimationGraphEvent>   *this_,
                          RE::BSAnimationGraphEvent *event,
                          RE::BSTEventSource<RE::BSAnimationGraphEvent> *dispatcher) -> void
    {
       //SKSE::log::info("catched on_animation call");
       if (!event || !event->holder) {
           return old_func(this_, event, dispatcher);
       }
       player_anim_handle(this_, event, dispatcher);
       return old_func(this_, event, dispatcher);
    }

    static inline REL::Relocation<decltype(new_func)> old_func;        
};



class PhysDamageToActor_Hook     // works just before hit, but can't decline hit event itself
{
  public:
    static void install_hook()
     {
        old_func = SKSE::GetTrampoline().write_call<5>(REL::ID(37673).address() + 0x3c0, new_func);
     }

  private:
     static void new_func(RE::Actor* target, RE::HitData &hit_data)
     {
        hit_data.totalDamage = allOnHitEffects(target, hit_data);
        return old_func(target, hit_data);
     }

     static inline REL::Relocation<decltype(new_func)> old_func;
};


class On_MeleeWeapCollide_Hook        //  melee weapon collides target, works before hit process, can decline hit event
{                    
  public:
     static void install_hook() {
          old_func = SKSE::GetTrampoline().write_call<5>(REL::ID(37650).address() + 0x38B, new_func);
     }
  private:
     static void new_func(RE::Actor* attacker, RE::Actor* victim, RE::Projectile* proj, char aleft)
     {
        if (!attacker || !victim) { return old_func (attacker, victim, proj, aleft); }
        bool allowAttack = on_meleeWeap_collide (attacker, victim);
        if (allowAttack) return old_func(attacker, victim, proj, aleft); 
        else mys::attackKeyHolding = false;        // if not allow this attack, turn off the blocker immediate
        
     }

    static inline REL::Relocation<decltype(new_func)> old_func;
};


class OnApplyResist_Hook        //  срабатывает при порезке об резисты, возвращает мульт остатка урона, например 0.3 (если резист был 70%)
{
  public:
    static void install_hook() {

        REL::Relocation<uintptr_t> for_npc_hook {RELOCATION_ID(261401, 0)};
        REL::Relocation<uintptr_t> for_pc_hook  {RELOCATION_ID(261920, 0)};

        old_func_npc = for_npc_hook.write_vfunc(0x0A, new_func_npc);
        old_func_pc  =  for_pc_hook.write_vfunc(0x0A, new_func_pc); 
    }

  private:
    static float new_func_pc(RE::MagicTarget *this_, RE::MagicItem *magic_item, RE::Effect *effect, RE::TESBoundObject *bound_object)
    {
        if (!this_ || !magic_item || !effect) {
           return old_func_pc(this_, magic_item, effect, bound_object);
        }
        else {
           return on_resist_apply(this_, magic_item, effect, bound_object);        
        }
    }
    static float new_func_npc(RE::MagicTarget* this_, RE::MagicItem* magic_item, RE::Effect* effect, RE::TESBoundObject* bound_object)
    {
        if (!this_ || !magic_item || !effect) {
           return old_func_npc(this_, magic_item, effect, bound_object);
        }
        else {
           return on_resist_apply(this_, magic_item, effect, bound_object);
        }

    }

    static inline REL::Relocation<decltype(new_func_pc)> old_func_pc;
    static inline REL::Relocation<decltype(new_func_npc)> old_func_npc;
};



class OnPlayerDrinkPotion_Hook            // before drinking, сan decline potion. Doesn't check poisons
{
  public:
    static void install_hook()    {
        REL::Relocation<uintptr_t> hook {RELOCATION_ID(261916, 0)};
        old_func = hook.write_vfunc(0x10F, new_func);
    }

  private:
    static bool new_func(RE::Actor* this_, RE::AlchemyItem* potion, RE::ExtraDataList* extra_data_list)
    {
       if (!this_ || !potion) {
           return old_func(this_, potion, extra_data_list);
       }
       if (!on_drink_potion(this_, potion, extra_data_list)) return true;  // predict from applying
       return old_func(this_, potion, extra_data_list);                    // simple applying
    }

    static inline REL::Relocation<decltype(new_func)> old_func;
};


class OnApplyPoison_Hook        // apply poison to weapon
{
public:
    static void install_hook() {
       old_func = SKSE::GetTrampoline().write_call<5>(REL::ID(39407).address() + 0x106, new_func);
    }
private:
    static void new_func(RE::InventoryEntryData* data, RE::AlchemyItem* poison, int count) {

	   if (data && poison && count > 0) {
           on_apply_poison(data, poison, count);
       }
       return old_func (data, poison, count);
    }

    static inline REL::Relocation<decltype(new_func)> old_func;
};

class OnAdjustActiveEffect_Hook        //    before adjust
{
  public:
    static void install_hook() {
       old_func = SKSE::GetTrampoline().write_call<5>(REL::ID(33763).address() + 0x4A3, new_func);
    }

  private:
    static void new_func(RE::ActiveEffect *this_, float power, bool unk)  
    {
       on_adjust_active_effect(this_, power, unk);
       old_func(this_, power, unk);
    }

    static bool new_hooks_installed;
    static inline REL::Relocation<decltype(new_func)> old_func;
};



class OnEffectFinish_Hook           // хуки для каждого архетипа эффекта по отдельности. Пока используется только ValueModifier           
{
 public:
    static void install_hook()
    {
       REL::Relocation<std::uintptr_t> hook {RE::ValueModifierEffect::VTABLE[0]};
       old_func = hook.write_vfunc(0x15,  new_func_4_valueMod);                         //   0x15  - finish.  Это должно хукать OnEffectFinish для ValueModifier эффектов
    }

 private:
    static void new_func_4_valueMod (RE::ValueModifierEffect* this_)    // works for modav and mod peak av
    {
       if (this_) on_valueMod_Effect_Finish(this_);
       old_func(this_);
    }

    static inline REL::Relocation<decltype(new_func_4_valueMod)> old_func;
};


inline bool is_playable_spell (RE::SpellItem* spel)
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
    static void new_func(RE::MagicCaster* mcaster, float dtime)
    {
       using S = RE::MagicCaster::State;
       uint32_t state = mcaster->state.underlying();
       // if (state == static_cast<uint32_t>(S::kUnk01) || state == static_cast<uint32_t>(S::kUnk02))    // old
       if (state == 1 || state == 2) {
           if (RE::Actor *a_ = mcaster->GetCasterAsActor(); a_ && a_->IsPlayerRef() && mcaster->currentSpell && mcaster->currentSpell->As<RE::SpellItem>())
           {
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


class OnMagicProjHit_Hook        //  on magic projectile hit target
{
  public:
    static void install_hook() {
       old_func = REL::Relocation<uintptr_t>(RE::VTABLE_MissileProjectile[0]).write_vfunc(0xBE, new_func);
    }
  private:
    static bool new_func (RE::Projectile* proj,  RE::hkpCollidable*)
    {        
        proj->linearVelocity *= -1;        //  turn proj back
        return false;
    }
    static inline REL::Relocation<decltype(new_func)> old_func;
};

class OnArrowMeetsCollision_Hook   
{
public:
    static void install_hook() {
        old_func = REL::Relocation<uintptr_t>(RE::VTABLE_ArrowProjectile[0]).write_vfunc(0xBE, new_func);
    }
private:
    static void new_func(RE::ArrowProjectile* arrow, RE::hkpAllCdPointCollector* collidable)
    {
        bool needDefaultLogic = on_arrow_collide(arrow, collidable);
        if (needDefaultLogic)  old_func(arrow, collidable);
    }

    static inline REL::Relocation<decltype(new_func)> old_func;
};

/*
class OnArrowHit_Hook final            //  arrow hits target (hit event) - not used now
{
 public:
  static void install_hook()  {
        old_func = SKSE::GetTrampoline().write_call<5>(REL::ID(43027).address() + 0x90, new_func);
  }
 private:
  static void new_func(RE::Character *attacker, RE::Actor *target, RE::Projectile *proj, bool is_left)
  {
        // not used now
  }
   static inline REL::Relocation<decltype(new_func)> old_func;
};
*/

inline float check_MP (float magn)
{
    float max_MP = (mys::player->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kTemporary, RE::ActorValue::kMagicka) +
                    mys::player->GetPermanentActorValue(RE::ActorValue::kMagicka));

    if (mys::player->GetActorValue(RE::ActorValue::kMagicka) > (max_MP - (mys::reserved_MP + 2.f))) {
       return 0;
    }
    else
       return magn;
}

class OnManaPercentRegen_Hook 
{
  public:
    static void install_hook() {
       old_func = SKSE::GetTrampoline().write_call<5>(REL::ID(37511).address() + 0x68, new_func);
    }

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


class OnModActorValue_Hook
{
public:
    static void install_hook() {
       REL::Relocation<uintptr_t> hook{RELOCATION_ID(258043, 0)};
       old_func = hook.write_vfunc(0x20, new_func);
    }

private:
    static void new_func(RE::ValueModifierEffect* this_, RE::Actor* actor, float value, RE::ActorValue av)
    {
       if (mys::reserved_MP > 0.f && actor->IsPlayerRef() && this_->actorValue == RE::ActorValue::kMagicka) {      // без бафов на резерв маны не уйдем дальше проверки mys::reserved_MP > 0
           if (this_->effect->baseEffect->IsDetrimental()) {
               old_func(this_, actor, value, av);  //  если эффект detrimental, вообще не трогаем
               return;
           }
		   else if (this_->effect->baseEffect->data.flags.any(RE::EffectSetting::EffectSettingData::Flag::kRecover)) {
                // если recover, то пропускаем как обычно т.к. это не реген, а например шмотка на макс. ману
               old_func(this_, actor, value, av);
               my::sf_handle_reserved_MP();          // возможно придется пересчитать текущую ману относительно максимальной (можно сократить этот метод)
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
       REL::Relocation<uintptr_t> hook{RELOCATION_ID(257874, 0)};
       old_func = hook.write_vfunc(0x20, new_func);
    }
 private:
    static void new_func(RE::ValueModifierEffect* this_, RE::Actor* actor, float value, RE::ActorValue av)
    {
       if (mys::reserved_MP > 0.f && actor->IsPlayerRef() && this_->actorValue == RE::ActorValue::kMagicka)
       {
           if (this_->effect->baseEffect->IsDetrimental() || this_->effect->baseEffect->data.flags.any(RE::EffectSetting::EffectSettingData::Flag::kRecover)) {                              
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

using ActorKillev = RE::ActorKill::Event;

class OnDeathEvent : public RE::BSTEventSink<ActorKillev>      // override ProcessEvent() and write our logic on death                                                          
{
public:
    virtual RE::BSEventNotifyControl ProcessEvent (const ActorKillev* evn, RE::BSTEventSource<ActorKillev>*) override
    {
       on_death(evn->victim, evn->killer);
       return RE::BSEventNotifyControl::kContinue;    // must be at the end of ProcessEvent()
    }
    static OnDeathEvent* GetSingleton()    {
       static OnDeathEvent singleton;
       return std::addressof(singleton);
    }
    void enable()  // this we call to register event listening
    {
       RE::ActorKill::GetEventSource()->AddEventSink(this);
    }
};


class MenuOpenCloseEventSink : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
{
public:
	virtual RE::BSEventNotifyControl ProcessEvent (const RE::MenuOpenCloseEvent* ev, RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override
	{
        if (ev) {
           if (ev->menuName == "Sleep/Wait Menu")
		   {
                if (ev->opening) on_wait_menu_open();
                else             on_wait_menu_close();
           }
		   else if (ev->menuName == "InventoryMenu")
		   {
			    if (ev->opening) on_inventory_open();
                else             on_inventory_close();
		   }
        }
		return RE::BSEventNotifyControl::kContinue;
	}

    static MenuOpenCloseEventSink* GetSingleton() {
        static MenuOpenCloseEventSink singleton;
        return std::addressof(singleton);
    }
    void enable()
    { 
		RE::UI::GetSingleton()->AddEventSink<RE::MenuOpenCloseEvent>(this);
    }
};


//------------------------------------------------------ for descriptions --------------------------

namespace pch_
{
    template <class T>
    void write_thunk_call (std::uintptr_t a_src)
    {
        auto& trampoline = SKSE::GetTrampoline();
        SKSE::AllocTrampoline(14);
        T::old_func = trampoline.write_call<5>(a_src, T::new_func);
    }
}


namespace description_hooks
{
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
                REL::Relocation<std::uintptr_t> target { RELOCATION_ID(50005, 50949), REL::VariantOffset(0x80, 0x80, 0x80) };            
                pch_::write_thunk_call<ItemCardPopulateHook>(target.address());

                REL::Relocation<std::uintptr_t> target2 { RELOCATION_ID(50201, 51130), REL::VariantOffset(0xB2, 0xB2, 0xB2) };
                pch_::write_thunk_call<ItemCardPopulateHook>(target2.address());

                REL::Relocation<std::uintptr_t> target3 { RELOCATION_ID(50297, 51218), REL::VariantOffset(0x35, 0x35, 0x35) };
                pch_::write_thunk_call<ItemCardPopulateHook>(target3.address());

                REL::Relocation<std::uintptr_t> target4 { RELOCATION_ID(50674, 51569), REL::VariantOffset(0x80, 0x7A, 0x80) };
                pch_::write_thunk_call<ItemCardPopulateHook>(target4.address());

                REL::Relocation<std::uintptr_t> target5 { RELOCATION_ID(50973, 51852), REL::VariantOffset(0x80, 0x7A, 0x80) };
                pch_::write_thunk_call<ItemCardPopulateHook>(target5.address());

                if (REL::Module::IsAE()) {
                    REL::Relocation<std::uintptr_t> target6 { RELOCATION_ID(0, 51458), REL::VariantOffset(0x0, 0x87, 0x0) };
                    pch_::write_thunk_call<ItemCardPopulateHook>(target6.address());
                }
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
                REL::Relocation<std::uintptr_t> target { RELOCATION_ID(50298, 51219), REL::VariantOffset(0x32, 0x32, 0x32) };
                pch_::write_thunk_call<ItemCardPopulateHook2>(target.address());

                REL::Relocation<std::uintptr_t> target2 { RELOCATION_ID(51152, 52032), REL::VariantOffset(0x8F, 0x8F, 0x8B) };
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
                REL::Relocation<uintptr_t> hook{RELOCATION_ID(269321, 215652)};
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
                REL::Relocation<uintptr_t> hook{RELOCATION_ID(268432, 215111)};
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
                REL::Relocation<uintptr_t> hook{RELOCATION_ID(268432, 215111)};
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
}

//-----------------------------------------------------------------------------------------------------


static void SKSEMessageHandler(SKSE::MessagingInterface::Message* message)
{
    switch (message->type) {
        case SKSE::MessagingInterface::kDataLoaded:            // All ESM/ESL/ESP plugins have loaded, main menu is now active. It is now safe to access form data
           my::initGameData();
           mys::init_globs();
           OnPlayerUpdate_Hook::install_hook();
           PhysDamageToActor_Hook::install_hook();
           On_PlayerAnimEvent_Hook::install_hook();
           HiddenSpeedMult_updater_Hook::install_hook();
           OnAdjustActiveEffect_Hook::install_hook();
           CastSpeed_Hook::install_hook();
           OnEquip_Hook::install_hook();
           OnUnEquip_Hook::install_hook();
           //OnArrowHit_Hook::install_hook();
           OnArrowMeetsCollision_Hook::install_hook();
           On_MeleeWeapCollide_Hook::install_hook();
           //OnMagicProjHit_Hook::install_hook();
           OnEffectFinish_Hook::install_hook();
           //OnApplyResist_Hook::install_hook();
           OnPlayerDrinkPotion_Hook::install_hook();
           OnApplyPoison_Hook::install_hook();
           //OnAttackAction_Hook::install_hook();
           InputWatcher::GetSingleton()->enable();            // key press event
           OnDeathEvent::GetSingleton()->enable();            // on  death event
           MenuOpenCloseEventSink::GetSingleton()->enable();  // open/close menu event

		   OnManaPercentRegen_Hook::install_hook();
           OnModActorValue_Hook::install_hook();
           OnModPeakActorValue_Hook::install_hook();
           description_hooks::Install_Hooks();   // must be last
           break;
        case SKSE::MessagingInterface::kPostLoadGame:        // Player's selected save game has finished loading
           my::sf_handle_reserved_MP();
           break;
        //case SKSE::MessagingInterface::kNewGame:            // new game
        //   OnPlayerUpdate_Hook::install_hook();
        //   break;
    }
}


SKSEPluginLoad(const SKSE::LoadInterface* a_skse)
{
    auto g_messaging = reinterpret_cast<SKSE::MessagingInterface*>(a_skse->QueryInterface(SKSE::LoadInterface::kMessaging));
    if (!g_messaging) {
        SKSE::log::critical("Failed to load messaging interface! This error is fatal, plugin will not load.");
        return false;
    }

    // SKSE::log::info("loaded");

    SKSE::Init(a_skse);
    SetupLog();                             // Comment this line if you want to switch off the logger
    SKSE::AllocTrampoline(1 << 10);

    g_messaging->RegisterListener("SKSE", SKSEMessageHandler);  // Message Handler implemented in SKSE plugin loader

    return true;

}




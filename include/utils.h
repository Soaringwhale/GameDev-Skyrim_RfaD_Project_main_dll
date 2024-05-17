#pragma once

#include <functional>
#include <future>
#include <vector>


#define DEBUG_LOGS 1							//  DEBUG ON/OFF

#if	    DEBUG_LOGS
 #define LOG(...)  SKSE::log::info(__VA_ARGS__)		//  if DEBUG_LOGS, __VA_ARGS__ for any arguments
#else
 #define LOG(...)				// if !DEBUG_LOGS,  then LOG() = nothing
#endif


auto u_form_has_keyword(const RE::TESForm* form, const RE::BGSKeyword* keyword) -> bool;

auto u_worn_has_keyword(RE::Actor* actor, RE::BGSKeyword* keyword) -> bool;

auto u_get_weapon(const RE::Actor* actor, const bool is_left_hand) -> RE::TESObjectWEAP*;

auto u_get_eff_secondary_av_name(const RE::ActiveEffect& active_effect) -> RE::ActorValue;

auto u_get_second_AV_weight(const RE::ValueModifierEffect& active_effect) -> float;

auto u_actor_has_active_mgef_with_keyword(RE::Actor* actor, const RE::BGSKeyword* keyword) -> bool;

auto u_cast (RE::SpellItem *spell, RE::Actor *target, RE::Actor  *caster) -> void;

auto u_cast_on_self (RE::SpellItem* spell, RE::Actor* actor) -> void;

auto u_is_power_attacking(RE::Actor *actor) -> bool;

auto u_is_in_city (RE::Actor *actor) -> bool;

auto u_place_at_me(RE::TESObjectREFR* target, RE::TESForm* form, std::uint32_t count, bool force_persist, bool initially_disabled) -> RE::TESObjectREFR*;

auto u_get_effects_by_keyword(RE::Actor *actor, const RE::BGSKeyword *keyword) -> std::vector<RE::ActiveEffect*>;

auto u_dispel_effect_from_actor(RE::ActiveEffect* effect, RE::Actor* actor) -> bool;

auto u_get_secondary_resist_name(const RE::MagicItem *magic_item) -> RE::ActorValue;

auto u_enchant_equipped_weapon(RE::InventoryChanges*, RE::TESBoundObject*, RE::ExtraDataList*, RE::EnchantmentItem*, int16_t) -> RE::ExtraDataList*;

auto u_get_actor_value_max(RE::Actor* actor, const RE::ActorValue av) -> float;

auto u_playSound(RE::Actor* a, RE::BGSSoundDescriptorForm* a_descriptor, float a_volumeOverride) -> void;

void u_log_actor_perk_entries(RE::Actor* actor, RE::BGSPerkEntry::EntryPoint theEntry, std::string entryNameForLog = "");

void u_kill_projectile (RE::Projectile* proj);

void u_addItem (RE::Actor* a, RE::TESBoundObject* item, RE::ExtraDataList* extraList = nullptr, int count = 1, RE::TESObjectREFR* fromRefr = nullptr);

void u_player_addItem (RE::TESBoundObject* item, int count);

auto u_get_item_count(RE::Actor* actor, uint32_t formid_) -> int32_t;

bool u_setStage(RE::TESQuest* quest, uint16_t stage);

void u_updQuestTextGlob (RE::TESQuest* quest, RE::TESGlobal *glob);

auto u_get_actors_weap_ench (RE::Actor* a, bool left) -> RE::EnchantmentItem*;

auto u_get_worn_equip_weight(RE::Actor* actor) -> float;

auto u_damage_av (RE::Actor* actor, RE::ActorValue av, float magn) -> void;

void u_update_speedMult(RE::Actor* actor);

int  u_get_game_statistic(std::string statName);

auto u_get_entered_console_commands() -> std::string;

auto u_int2hex (int decimal) -> std::string;

void u_SendInventoryUpdateMessage (RE::TESObjectREFR* a_inventoryRef, const RE::TESBoundObject* a_updateObj);


namespace Utils_anim_namespace
{

    enum class AnimationEvent {
        kWeaponSwing = 0,
        kWeaponLeftSwing = 1,
        kJumpUp = 2,
        kBowDrawStart = 3,
        kBashExit = 4,
        kTkDodgeStart = 5,
        kTkDodgeIFrameEnd = 6,
        kPreHitFrame = 7,
        kHitFrame = 8,
        kBowRelease = 9,
        kArrowDetach = 10,
        kInterruptCast = 11,
        kBowDraw = 12,
        kWeapEquip_Out = 13,
        kBowZoomStop = 14,
        kAttackWinStart = 15,
        kAttackWinEnd = 16,
        kBowZoomStart = 17,
        kAttackStop = 18,
        kNone = 99
    };

    constexpr inline std::string_view  WEAPON_SWING  = "weaponSwing"sv;					
    constexpr inline std::string_view  WEAPON_SWING_LEFT = "weaponLeftSwing"sv;
    constexpr inline std::string_view  JUMP_UP = "JumpUp"sv;
    constexpr inline std::string_view  BOW_DRAW_START = "bowDrawStart"sv;
    constexpr inline std::string_view  BASH_EXIT  = "bashExit"sv;
    constexpr inline std::string_view  TKDR_DODGE_START = "TKDR_DodgeStart"sv;
    constexpr inline std::string_view  TKDR_I_FRAME_END = "TKDR_IFrameEnd"sv;
    constexpr inline std::string_view  PRE_HIT_FRAME = "preHitFrame"sv;
    constexpr inline std::string_view  HIT_FRAME = "HitFrame"sv;
    constexpr inline std::string_view  BOW_RELEASE = "BowRelease"sv;
    constexpr inline std::string_view  ARROW_DETACH = "arrowDetach"sv;
    constexpr inline std::string_view  INTERRUPT_CAST = "InterruptCast"sv;
    constexpr inline std::string_view  BOW_DRAW = "BowDraw"sv;
    constexpr inline std::string_view  WEAP_EQUIP_OUT = "WeapEquip_Out"sv;
    constexpr inline std::string_view  BOW_ZOOM_STOP = "BowZoomStop"sv;
    constexpr inline std::string_view  BOW_ZOOM_START = "BowZoomStart"sv;
	constexpr inline std::string_view  ATK_WIN_START = "AttackWinStart"sv;
    constexpr inline std::string_view  ATK_WIN_END = "AttackWinEnd"sv;
    constexpr inline std::string_view  ATK_STOP = "attackStop"sv;
    

    static std::map <std::string_view, AnimationEvent>  animation_map
	{
        {WEAPON_SWING,      AnimationEvent::kWeaponSwing},
        {WEAPON_SWING_LEFT, AnimationEvent::kWeaponLeftSwing},
        {JUMP_UP,           AnimationEvent::kJumpUp},
        {BOW_DRAW_START,    AnimationEvent::kBowDrawStart},
        {BASH_EXIT,         AnimationEvent::kBashExit},
        {TKDR_DODGE_START,  AnimationEvent::kTkDodgeStart},
        {TKDR_I_FRAME_END,  AnimationEvent::kTkDodgeIFrameEnd},
        {PRE_HIT_FRAME,     AnimationEvent::kPreHitFrame},
        {HIT_FRAME,         AnimationEvent::kHitFrame},
        {BOW_RELEASE,       AnimationEvent::kBowRelease},
        {ARROW_DETACH,		AnimationEvent::kArrowDetach},
        {INTERRUPT_CAST,	AnimationEvent::kInterruptCast},
        {BOW_DRAW,          AnimationEvent::kBowDraw},
        {WEAP_EQUIP_OUT,	AnimationEvent::kWeapEquip_Out},
        {BOW_ZOOM_STOP,		AnimationEvent::kBowZoomStop},
        {BOW_ZOOM_START,    AnimationEvent::kBowZoomStart},
        {ATK_WIN_START,		AnimationEvent::kAttackWinStart},
		{ATK_WIN_END,		AnimationEvent::kAttackWinEnd},
        {ATK_STOP,		    AnimationEvent::kAttackStop}
	};

    auto try_find_animation(const std::string& key) -> AnimationEvent;

}  // namespace Utils_anim_namespace


class BGSEntryPointFunctionDataTwoValue : public RE::BGSEntryPointFunctionData {			//   исп. для хранения данных функции ентри с 2 слотами, к примеру выбор AV и выбор множителя.
public:
    inline static constexpr auto VTABLE = RE::VTABLE_BGSEntryPointFunctionDataTwoValue;
    inline static constexpr auto RTTI = RE::RTTI_BGSEntryPointFunctionDataTwoValue;

    float data1;					// 08 - DATA
    float data2;					// 0C
};
static_assert(sizeof(BGSEntryPointFunctionDataTwoValue) == 0x10);


 namespace u_MessageBox {

        template <typename T>
        inline RE::GPtr<T> GetMenu()
		{
            auto ui = RE::UI::GetSingleton();
            return ui->GetMenu<T>(T::MENU_NAME);
        }

        class MyMessageBox
		{
            class MessageBoxResultCallback : public RE::IMessageBoxCallback		// inherit from IMessageBoxCallback
			{
                std::function<void(unsigned int)> _callback;

              public:
                ~MessageBoxResultCallback() override 
				{ /**/ }
                MessageBoxResultCallback(std::function<void(unsigned int)> callback) : _callback(callback)		// recieves func pointer and fills field _callback
				{ /**/ }

                void Run (RE::IMessageBoxCallback::Message message) override {			// run() recieves message and calls _callback
                    _callback(static_cast<unsigned int>(message));
                }
            };

         public:
            static void Show (const std::string& bodyText, std::vector<std::string> buttonTextValues, std::function<void(unsigned int)> callback)
			{
                auto* factoryManager = RE::MessageDataFactoryManager::GetSingleton();
                auto* uiStringHolder = RE::InterfaceStrings::GetSingleton();
                auto* factory = factoryManager->GetCreator<RE::MessageBoxData>(uiStringHolder->messageBoxData);
                auto* messagebox = factory->Create();

                RE::BSTSmartPointer<RE::IMessageBoxCallback> messageCallback = RE::make_smart<MessageBoxResultCallback>(callback);
                messagebox->callback = messageCallback;
                messagebox->bodyText = bodyText;
                for (auto text : buttonTextValues) messagebox->buttonText.push_back(text.c_str());
                messagebox->QueueMessage();
            }

            static void Show (RE::BGSMessage* msg, std::function<void(unsigned int)> callback)
			{
                RE::BSString descr;
                msg->GetDescription(descr, msg->ownerQuest);

                auto playerRef = RE::PlayerCharacter::GetSingleton();

                std::vector<std::string> buttonTextValues;
                for (const auto& btn : msg->menuButtons) {
                    if (btn->conditions.IsTrue(playerRef, playerRef)) {
                        buttonTextValues.push_back(std::string(btn->text.c_str()));
                    }
                }
                Show(std::string(descr.c_str()), buttonTextValues, callback);
            }
        };
 }

    // вручную прогнать значение через ентри перков актера, к примеру прогнать урон через все mod incoming damage, либо спел через все mod incoming spell magnitude

    // float damage_resist = 1.f;	//  сюда по ссылке вернется множитель после прогона через ентри 
	// RE::BGSEntryPoint::HandleEntryPoint(RE::BGSEntryPoint::ENTRY_POINT::kModIncomingDamage, target, hit_data->aggressor,  hit_data->weapon,  std::addressof(damage_resist));
    // для спела они другие

    // RE::TESObjectWEAP *playerWeap = form->As<RE::TESObjectWEAP>();
	// .As<>() - СКСЕшный способ приведения, шаблонный метод, внутри там cast RE::TESObjectWEAP *playerWeap = static_cast<RE::TESObjectWEAP*>(form); 
    // c++ способ приведения через cast напрямую, должен тоже работать.

    // RE::Effect*
    // RE::EffectItem*
    // RE::EffectSetting*
    // RE::ActiveEffect*

    /*enum class WEAPON_TYPE {
        kHandToHandMelee = 0,
        kOneHandSword = 1,
        kOneHandDagger = 2,
        kOneHandAxe = 3,
        kOneHandMace = 4,
        kTwoHandSword = 5,
        kTwoHandAxe = 6,
        kBow = 7,
        kStaff = 8,
        kCrossbow = 9
    };*/

    //---------------------  for input events  etc  from jarari ----------------------

    namespace Utils {
        enum {
            // first 256 for keyboard, then 8 mouse buttons, then mouse wheel up, wheel down, then 16 gamepad buttons
            kMacro_KeyboardOffset = 0,  // not actually used, just for self-documentation
            kMacro_NumKeyboardKeys = 256,

            kMacro_MouseButtonOffset = kMacro_NumKeyboardKeys,  // 256
            kMacro_NumMouseButtons = 8,

            kMacro_MouseWheelOffset = kMacro_MouseButtonOffset + kMacro_NumMouseButtons,  // 264
            kMacro_MouseWheelDirections = 2,

            kMacro_GamepadOffset = kMacro_MouseWheelOffset + kMacro_MouseWheelDirections,  // 266
            kMacro_NumGamepadButtons = 16,

            kMaxMacros = kMacro_GamepadOffset + kMacro_NumGamepadButtons  // 282
        };

        enum {
            kGamepadButtonOffset_DPAD_UP = kMacro_GamepadOffset,  // 266
            kGamepadButtonOffset_DPAD_DOWN,
            kGamepadButtonOffset_DPAD_LEFT,
            kGamepadButtonOffset_DPAD_RIGHT,
            kGamepadButtonOffset_START,
            kGamepadButtonOffset_BACK,
            kGamepadButtonOffset_LEFT_THUMB,
            kGamepadButtonOffset_RIGHT_THUMB,
            kGamepadButtonOffset_LEFT_SHOULDER,
            kGamepadButtonOffset_RIGHT_SHOULDER,
            kGamepadButtonOffset_A,
            kGamepadButtonOffset_B,
            kGamepadButtonOffset_X,
            kGamepadButtonOffset_Y,
            kGamepadButtonOffset_LT,
            kGamepadButtonOffset_RT  // 281
        };

        uint32_t GamepadMaskToKeycode(uint32_t keyMask);
        // std::string SplitString(const std::string str, const std::string delimiter, std::string& remainder);
        RE::TESForm* GetFormFromMod(std::string modname, uint32_t formid);
        // RE::TESForm* GetFormFromConfigString(const std::string str);
        bool PerformAction(RE::BGSAction* action, RE::Actor* actor);
        bool PlayIdle(RE::TESIdleForm* idle, RE::Actor* actor, RE::DEFAULT_OBJECT action, RE::Actor* target);
        RE::Actor* PickActor(RE::bhkPickData& pick, RE::TESObjectCELL* cell, RE::NiPoint3 origin, RE::NiPoint3 dir,
                             float dist, RE::Actor* excludedActor = nullptr);
        bool ActorHasKeywords(RE::Actor* actor, std::vector<RE::BGSKeyword*>& keywords);
        RE::NiPoint3 ToDirectionVector(RE::NiMatrix3 mat);
        RE::NiPoint3 ToUpVector(RE::NiMatrix3 mat);
        RE::NiPoint3 ToRightVector(RE::NiMatrix3 mat);
        void Normalize(RE::NiPoint3& p);
    }


namespace Globals {
    extern RE::PlayerCharacter* p;
    extern RE::PlayerControls* pc;
    extern RE::PlayerCamera* pcam;
    extern RE::UI* ui;
    extern RE::ControlMap* controlMap;
    extern RE::UserEvents* userEvents;
    extern RE::BSInputDeviceManager* inputDeviceManager;
    void InitializeGlobalVaribles();
}


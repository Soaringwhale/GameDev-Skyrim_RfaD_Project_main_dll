#pragma once

#pragma warning (disable : 4100)  // unused parameters

#include <functional>
#include <future>
#include <vector>

#define DEBUG_LOGS 1                            //  DEBUG ON/OFF

#if        DEBUG_LOGS
 #define LOG(...)  SKSE::log::info(__VA_ARGS__)        //  if DEBUG_LOGS, __VA_ARGS__ for any arguments
#else
 #define LOG(...)                // if !DEBUG_LOGS,  then LOG() = nothing
#endif


// преобразовать табы в пробелы:
// 1) CTRL + H
// 2) нажать иконку .* для поиска с исп. регулярных выражений
// 3) найти [\t]
// 4) заменить на *4 пробела*

enum class SkyrimEquipSlot   // EQUP game objects
{
    kRight,
    kLeft,
    kEitherHand,
    kBothHands,
    kPower,       // 'NONE' slot, this name for convenience
    kVoice,
    kShield,
    kPotion
};

auto u_form_has_keyword (const RE::TESForm* form, const RE::BGSKeyword* keyword) -> bool;
auto u_worn_has_keyword (RE::Actor* actor, RE::BGSKeyword* keyword) -> bool;
auto u_get_effects_by_keyword (RE::Actor* actor, const RE::BGSKeyword* keyword) -> std::vector<RE::ActiveEffect*>;
auto u_is_bow_drawning (RE::Actor* actor) -> bool;
auto u_is_attacking (RE::Actor* actor) -> bool;
auto u_is_power_attacking (RE::Actor* actor) -> bool;

auto u_same_activeEffects_count (RE::Actor* actor, const RE::EffectSetting* base_eff) -> uint32_t;
auto u_get_eff_secondary_av_name (const RE::ActiveEffect& active_effect) -> RE::ActorValue;
auto u_get_secondary_resist_name (const RE::MagicItem* magic_item) -> RE::ActorValue;
auto u_get_second_AV_weight(const RE::ValueModifierEffect& active_effect) -> float;
auto u_actor_has_active_mgef_with_keyword (RE::Actor* actor, const RE::BGSKeyword* keyword) -> bool;

auto u_damage_av (RE::Actor* actor, RE::ActorValue av, float magn) -> void;
auto u_get_av_percent (RE::Actor* actor, RE::ActorValue av) -> float;
auto u_get_actor_value_max (RE::Actor* actor, const RE::ActorValue av) -> float;

auto u_cast (RE::SpellItem *spell, RE::Actor *target, RE::Actor  *caster) -> void;
auto u_cast_on_self (RE::SpellItem* spell, RE::Actor* actor) -> void;

auto u_is_in_city (RE::Actor *actor) -> bool;
bool u_center_on_cell (RE::PlayerCharacter* player, const char editorID, RE::TESObjectCELL *cell);
auto u_place_at_me (RE::TESObjectREFR* target, RE::TESForm* form, std::uint32_t count, bool force_persist, bool initially_disabled) -> RE::TESObjectREFR*;
void u_set_rotation_x (RE::TESObjectREFR *refr, float angle);
void u_set_rotation_y (RE::TESObjectREFR *refr, float angle);  // rotate reference
void u_set_rotation_z (RE::TESObjectREFR *refr, float angle);
auto u_get_distance (RE::TESObjectREFR* a, RE::TESObjectREFR* b) -> float;
bool u_mark_for_delete (void*, void*, RE::TESObjectREFR*);

void u_jump (RE::Actor* actor);
void u_set_ghost (RE::Actor *actor, bool set);
void u_update_speedMult (RE::Actor* actor);
auto u_get_player_followers() -> std::vector<RE::Actor*>;
auto u_get_commanded_actors (RE::Actor* actor) -> RE::BSTArray<RE::CommandedActorData>*;
void u_move_all_followers_to_player (const bool followers, const bool summons);
void u_toggle_controls (bool status_);
auto u_disable_controls_selective() -> RE::stl::enumeration<RE::UserEvents::USER_EVENT_FLAG, std::uint32_t>;
bool u_findClosestReferenceOfTypeInRadius (RE::TES* tes, const RE::TESBoundObject &obj, const RE::NiPoint3 &centerOfSearch, float afRadius, RE::NiPointer<RE::TESObjectREFR> &outRef);

auto u_dispel_effect_from_actor (RE::ActiveEffect* effect, RE::Actor* actor) -> bool;

auto u_get_weapon (const RE::Actor* actor, const bool is_left_hand) -> RE::TESObjectWEAP*;
auto u_get_equipped_spell_by_slot (RE::Actor*, RE::MagicSystem::CastingSource) -> RE::SpellItem*;
auto u_get_equipped_shout (RE::Actor* actor) -> RE::TESShout*;
auto u_get_current_casting_spell (RE::Actor* actor, uint32_t mcasterSlot) -> RE::MagicItem*;
auto u_get_worn_equip_weight (RE::Actor* actor) -> float;
auto u_get_all_worn_objects (RE::Actor* actor) -> RE::TESObjectREFR::InventoryItemMap;
auto u_get_inv_items_by_keyword (RE::TESObjectREFR* ref, const std::string &keywordEditorID) -> std::vector<RE::TESBoundObject*>;
void u_remove_all_items (RE::Actor* actor);
auto u_get_item_count (RE::Actor* actor, uint32_t formid_) -> int32_t;
auto u_papyrusGetItemCount (RE::TESObjectREFR* cont, RE::TESForm* item) -> int32_t;
void u_addItem (RE::Actor* a, RE::TESBoundObject* item, RE::ExtraDataList* extraList = nullptr, int count = 1, RE::TESObjectREFR* fromRefr = nullptr);
void u_equip_item (RE::Actor*, RE::TESBoundObject*, std::optional<SkyrimEquipSlot> slot = std::nullopt);
void u_equip_spell (RE::Actor*, RE::SpellItem*, SkyrimEquipSlot);
void u_unequip_all_items (RE::Actor* actor);
bool u_is_equipped (RE::Actor* actor, RE::TESBoundObject* obj);

bool u_setStage (RE::TESQuest* quest, uint16_t stage);
void u_updQuestTextGlob (RE::TESQuest* quest, RE::TESGlobal *glob);
int  u_get_game_statistic (std::string statName);
auto u_get_entered_console_commands() -> std::string;

auto u_get_actors_weap_extra_ench (RE::Actor* a, bool left) -> RE::EnchantmentItem*;
auto u_enchant_equipped_weapon (RE::InventoryChanges*, RE::TESBoundObject*, RE::ExtraDataList*, RE::EnchantmentItem*, int16_t) -> RE::ExtraDataList*;
void u_remove_weap_ench (RE::InventoryEntryData *entry);

auto u_get_pc_poison (bool is_left_hand) -> RE::ExtraPoison*;
void u_remove_pc_poison (bool is_left_hand);

auto u_int2hex (int decimal) -> std::string;
auto u_trimmed_str (float number) -> std::string;

auto u_req_inc_damage() -> float;  // requiem damage taken like x1.5
auto u_req_out_damage() -> float;  // requiem damage dealt like x1.5

void u_exit_game();
void u_set_god_mode (bool set);

auto u_get_open_state (RE::TESObjectREFR *refr) -> uint32_t;
void u_set_open_state (RE::TESObjectREFR* refr, bool a_open, bool a_snap);

bool u_is_food (RE::TESBoundObject* obj);

void u_SendInventoryUpdateMessage (RE::TESObjectREFR* a_inventoryRef, const RE::TESBoundObject* a_updateObj);

void u_log_actor_perk_entries (RE::Actor* actor, RE::BGSPerkEntry::EntryPoint theEntry, std::string entryNameForLog = "");

void u_playSound (RE::Actor* a, RE::BGSSoundDescriptorForm* a_descriptor, float a_volumeOverride);

void u_apply_imod (RE::TESImageSpaceModifier *imod, float power);
void u_stop_imod  (RE::TESImageSpaceModifier *imod);

auto u_get_current_weatherType_region () -> RE::TESRegion*;
void u_set_weather (RE::Sky *sky, RE::TESWeather *weather, bool force, bool accelerate);

void u_kill_projectile (RE::Projectile* proj);

void u_set_collision_layer (RE::Actor* actor, RE::COL_LAYER layer);
void u_remove_collision (RE::Actor* actor);
void u_return_collision (RE::Actor* actor);

void u_show_message_box  (RE::BGSMessage* msgBox, std::function<void(unsigned int)> callback);
void u_show_message_box2 (std::string& bodyText, std::vector<std::string> buttonTextValues, std::function<void(unsigned int)> callback);

char* u_get_extra_editorID (RE::ExtraDataList* lists);
void  u_set_extra_editorID (RE::ExtraDataList* lists, const char* edid);
auto  u_get_extra_textDisplayData (RE::ExtraDataList *exlist) -> RE::ExtraTextDisplayData*;
void  u_set_extra_item_name (void* menu, RE::InventoryEntryData* item, RE::ExtraDataList* extralist, const char* name);

void u_toggle_vanilla_attribute_bars (bool a_bEnable);

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
        kAttackPowerLeft_FXstart = 19,
        kAttackPowerRight_FXstart = 20,
        kAttackPowerStanding_FXstart = 21,
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

    constexpr inline std::string_view PWATK_LEFT_START  = "AttackPowerLeft_FXstart"sv;
    constexpr inline std::string_view PWATK_RIGHT_START = "AttackPowerRight_FXstart"sv;
    constexpr inline std::string_view PWATK_STAND_START = "AttackPowerStanding_FXstart"sv;
    

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
        {ARROW_DETACH,        AnimationEvent::kArrowDetach},
        {INTERRUPT_CAST,    AnimationEvent::kInterruptCast},
        {BOW_DRAW,          AnimationEvent::kBowDraw},
        {WEAP_EQUIP_OUT,    AnimationEvent::kWeapEquip_Out},
        {BOW_ZOOM_STOP,        AnimationEvent::kBowZoomStop},
        {BOW_ZOOM_START,    AnimationEvent::kBowZoomStart},
        {ATK_WIN_START,        AnimationEvent::kAttackWinStart},
        {ATK_WIN_END,        AnimationEvent::kAttackWinEnd},
        {ATK_STOP,            AnimationEvent::kAttackStop},
        {PWATK_LEFT_START,  AnimationEvent::kAttackPowerLeft_FXstart},
        {PWATK_RIGHT_START, AnimationEvent::kAttackPowerRight_FXstart},
        {PWATK_STAND_START, AnimationEvent::kAttackPowerStanding_FXstart}

    };

    auto try_find_animation(const std::string& key) -> AnimationEvent;

}  // namespace Utils_anim_namespace


class BGSEntryPointFunctionDataTwoValue : public RE::BGSEntryPointFunctionData {            //   исп. для хранения данных функции ентри с 2 слотами, к примеру выбор AV и выбор множителя.
public:
    inline static constexpr auto VTABLE = RE::VTABLE_BGSEntryPointFunctionDataTwoValue;
    inline static constexpr auto RTTI = RE::RTTI_BGSEntryPointFunctionDataTwoValue;

    float data1;                    // 08 - DATA
    float data2;                    // 0C
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
            class MessageBoxResultCallback : public RE::IMessageBoxCallback        // inherit from IMessageBoxCallback
            {
                std::function<void(unsigned int)> _callback;

              public:
                ~MessageBoxResultCallback() override 
                { /**/ }
                MessageBoxResultCallback(std::function<void(unsigned int)> callback) : _callback(callback)        // recieves func pointer and fills field _callback
                { /**/ }

                void Run (RE::IMessageBoxCallback::Message message) override {            // run() recieves message and calls _callback
                    _callback(static_cast<unsigned int>(message));
                }
            };

         public:
            static void Show (const std::string& bodyText, std::vector<std::string> buttonTextValues, std::function<void(unsigned int)> callback)
            {
                RE::MessageDataFactoryManager* factoryManager = RE::MessageDataFactoryManager::GetSingleton();
                RE::InterfaceStrings* uiStringHolder = RE::InterfaceStrings::GetSingleton();
                const RE::BSTDerivedCreator<RE::MessageBoxData, RE::IUIMessageData>* factory = factoryManager->GetCreator<RE::MessageBoxData>(uiStringHolder->messageBoxData);
                RE::MessageBoxData* messagebox = factory->CreateDerived();

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
                Show (std::string(descr.c_str()), buttonTextValues, callback);
            }
        };
 }


 namespace NiPointExt   // extended ? 
 {
     static inline RE::NiPoint3 Normalize(RE::NiPoint3 v)
     {
         float length = std::sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
         if (length > 1e-8f) {
             v.x /= length;
             v.y /= length;
             v.z /= length;
         }
		 else {
             v.x = 0.f;
             v.y = 0.f;
             v.z = 1.f;
         }
         return v;
     }

     static inline float GetMatrixDeterminant(const RE::NiMatrix3 mat)
     {
         return mat.entry[0][0] * (mat.entry[1][1] * mat.entry[2][2] - mat.entry[2][1] * mat.entry[1][2]) -
                mat.entry[0][1] * (mat.entry[1][0] * mat.entry[2][2] - mat.entry[1][2] * mat.entry[2][0]) +
                mat.entry[0][2] * (mat.entry[1][0] * mat.entry[2][1] - mat.entry[1][1] * mat.entry[2][0]);
     }

     static RE::NiMatrix3 LookAt (const RE::NiPoint3& cameraPos, const RE::NiPoint3& targetPos, const RE::NiPoint3& upHint = { 0.f, 0.f, 1.f })
     {
         RE::NiPoint3 forward = targetPos - cameraPos;
         float length = forward.Length();

         if (length <= 0.f) {
             return RE::NiMatrix3();  // Единичная матрица, если камера уже на цели
         }

         forward = forward / length;  // Нормализация

         // Проверяем, не коллинеарны ли forward и upHint
         RE::NiPoint3 right;
         if (std::abs(forward.Dot(upHint)) > 0.999f)  // Если угол почти 0° или 180°
         {
             // Используем альтернативный вектор "вверх" (например, ось X)
             RE::NiPoint3 altUp = { 1.f, 0.f, 0.f };
             right = NiPointExt::Normalize(altUp.Cross(forward));
         }
         else {
             right = NiPointExt::Normalize(upHint.Cross(forward));
         }

         right = -right;

         RE::NiPoint3 up = NiPointExt::Normalize(forward.Cross(right));

         // Заполняем матрицу (вторая колонка = forward, как в вашем случае)
         RE::NiMatrix3 rotation;
         rotation.entry[0][0] = right.x;  // Right X
         rotation.entry[1][0] = right.y;  // Right Y
         rotation.entry[2][0] = right.z;  // Right Z

         rotation.entry[0][1] = forward.x;  // Forward X
         rotation.entry[1][1] = forward.y;  // Forward Y
         rotation.entry[2][1] = forward.z;  // Forward Z

         rotation.entry[0][2] = up.x;  // Up X
         rotation.entry[1][2] = up.y;  // Up Y
         rotation.entry[2][2] = up.z;  // Up Z

         float det = NiPointExt::GetMatrixDeterminant(rotation);
         if (det < 0.0f) {
             rotation.entry[0][2] *= -1;  // Инвертируем up X
             rotation.entry[1][2] *= -1;  // Инвертируем up Y
             rotation.entry[2][2] *= -1;  // Инвертируем up Z
         }
         return rotation;
     }

     static inline RE::NiPoint3 Cross (const RE::NiPoint3& a, const RE::NiPoint3& b)
     {
         return RE::NiPoint3{ a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x };
     }
 } // namespace NiPointExt

 //--------------------------
 /*
 namespace NiNodeExt
{
    static inline RE::NiPoint3 Normalize(RE::NiPoint3 v)
    {
        float length = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
        if (length > 1e-8f) {
            v.x /= length;
            v.y /= length;
            v.z /= length;
        } else {
            v.x = 0.f;
            v.y = 0.f;
            v.z = 1.f;
        }

        return v;
    }

    static inline float GetMatrixDeterminant(const RE::NiMatrix3 mat)
    {
        return mat.entry[0][0] * (mat.entry[1][1] * mat.entry[2][2] - mat.entry[2][1] * mat.entry[1][2]) -
               mat.entry[0][1] * (mat.entry[1][0] * mat.entry[2][2] - mat.entry[1][2] * mat.entry[2][0]) +
               mat.entry[0][2] * (mat.entry[1][0] * mat.entry[2][1] - mat.entry[1][1] * mat.entry[2][0]);
    }

    static RE::NiMatrix3 LookAt (const RE::NiPoint3& cameraPos, const RE::NiPoint3& targetPos, const RE::NiPoint3& upHint = { 0.f, 0.f, 1.f })
    {
        RE::NiPoint3 forward = targetPos - cameraPos;
        float length = forward.Length();

        if (length <= 0.0f) {
            return RE::NiMatrix3();  // Единичная матрица, если камера уже на цели
        }

        forward = forward / length;  // Нормализация

        // Проверяем, не коллинеарны ли forward и upHint
        RE::NiPoint3 right;
        if (std::abs(forward.Dot(upHint)) > 0.999f)  // Если угол почти 0° или 180°
        {
            // Используем альтернативный вектор "вверх" (например, ось X)
            RE::NiPoint3 altUp = { 1.0f, 0.0f, 0.0f };
            right = NiPointExt::Normalize(altUp.Cross(forward));
        } else {
            right = NiPointExt::Normalize(upHint.Cross(forward));
        }

        right = -right;

        RE::NiPoint3 up = NiPointExt::Normalize(forward.Cross(right));

        // Заполняем матрицу (вторая колонка = forward, как в вашем случае)
        RE::NiMatrix3 rotation;
        rotation.entry[0][0] = right.x;  // Right X
        rotation.entry[1][0] = right.y;  // Right Y
        rotation.entry[2][0] = right.z;  // Right Z

        rotation.entry[0][1] = forward.x;  // Forward X
        rotation.entry[1][1] = forward.y;  // Forward Y
        rotation.entry[2][1] = forward.z;  // Forward Z

        rotation.entry[0][2] = up.x;  // Up X
        rotation.entry[1][2] = up.y;  // Up Y
        rotation.entry[2][2] = up.z;  // Up Z

        float det = NiPointExt::GetMatrixDeterminant(rotation);
        if (det < 0.0f) {
            rotation.entry[0][2] *= -1;  // Инвертируем up X
            rotation.entry[1][2] *= -1;  // Инвертируем up Y
            rotation.entry[2][2] *= -1;  // Инвертируем up Z
        }

        return rotation;
    }

    static inline RE::NiPoint3 Cross(const RE::NiPoint3& a, const RE::NiPoint3& b)
    {
        return RE::NiPoint3{ a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
    }

} // namespace NiNodeExt
*/

namespace SPMath
{
    static const float MetricToUnit = 69.9912510936133f;  // соотношение метров и юнитов
    static const float UnitToMetric = 0.0142875f;
    static const float Deg2Rad = 0.017453292f;
}
namespace u_cam    //  это из хедера, реализацию функций ниже брать из .cpp файла i3sn-a
{ 
     class MyCameraState : public RE::TESCameraState
     {
       public:
         using TESCameraState::TESCameraState;
         MyCameraState (RE::TESCamera* a_camera, RE::CameraState a_id) : RE::TESCameraState(a_camera, a_id) {}
         MyCameraState (const MyCameraState&) = delete;                // no copy c-tor
         MyCameraState& operator= (const MyCameraState&) = delete;     // no operator =
         ~MyCameraState() override = default;

         void Begin() override
         {
             //old_allowAutoVanityMode = cam->allowAutoVanityMode;
             // PlayerCamera::SetAllowAutoVanityMode
             //cube = GraphicsMenu::GetInstance()->CreateCube({0, 0, 0}, {1, 1, 1, 1}, 1000, 15);
         }

         void End() override
         {
             //auto cam = (RE::PlayerCamera*)camera;
             //CameraAPI::GetInstance()->BindToPlayer(true);
         }

         void Update (RE::BSTSmartPointer<RE::TESCameraState>& a_nextState) override
         {
             if (isLerpPos) {
                 float speed = 10 * SPMath::MetricToUnit;  // units per second
                 float deltaTime = lerpPosTime01;          // время кадра

                 // Вычисляем направление и нормализуем его
                 RE::NiPoint3 direction = cameraPos - camera->cameraRoot->world.translate;
                 float distance = direction.Unitize();  // длина вектора и нормализация

                 // Линейное перемещение
                 if (distance > 0.001f) {
                     float step = speed * deltaTime;
                     camera->cameraRoot->world.translate += direction * std::min(step, distance);
                 }
             }
			 else {
                 camera->cameraRoot->world.translate = cameraPos;
             }

             if (isAutoLook) {
                 camera->cameraRoot->world.rotate = NiPointExt::LookAt(camera->cameraRoot->world.translate, lookTarget);
             }
         };

         void Init (bool isAutoLook_ = false, bool isLerpPos_ = false, float lerpPosTime01_ = 1)
         {
             this->isAutoLook = isAutoLook_;
             this->isLerpPos = isLerpPos_;
             this->lerpPosTime01 = std::clamp(lerpPosTime01_, 0.f, 1.f);
         }

         void SetPos(const RE::NiPoint3& target) { cameraPos = target; }
         void SetLookTarget(const RE::NiPoint3& target) { lookTarget = target; }

         void GetRotation(RE::NiQuaternion& a_rotation) override {}
         void GetTranslation(RE::NiPoint3& a_translation) {}

       private:
         bool isAutoLook;
         bool isLerpPos;
         float lerpPosTime01;
         RE::NiPoint3 cameraPos = { 0, 0, 0 };
         RE::NiPoint3 lookTarget = { 0, 0, 0 };
     };

     class MyCameraAPI
     {
       public:
         MyCameraAPI();
         ~MyCameraAPI() = default;

         static MyCameraAPI* GetSingleton() {
             static MyCameraAPI* instance = new MyCameraAPI();
             return instance;
         }

         static RE::NiPoint2 WorldPtToScreen(const RE::NiPoint3& a_worldPos);
         // static DirectX::XMFLOAT2 WorldPtToScreen(const DirectX::XMFLOAT3& a_worldPos);   // no DirectX include now
         static RE::NiPoint2 WorldPtToScreenBySize(const RE::NiPoint3& a_worldPos);

         void BindToPlayer();
         void UnbindFromPlayer(bool isAutoLook, bool isLerpPos, float lerpPosTime01);
         void SetCameraPos(const RE::NiPoint3& pos);
         void SetCameraRot(const RE::NiMatrix3& rot);
         void SetCameraLookTarget(const RE::NiPoint3& point);
         void SetInventoryState();
         void SetCreateCharState();

       private:
         bool isBindedToPlayer = true;
         RE::CameraState prevState = RE::CameraState::kFirstPerson;
         RE::BSTSmartPointer<MyCameraState> freeState = RE::make_smart<MyCameraState>(RE::PlayerCamera::GetSingleton(), RE::CameraState::kPCTransition);
     };
};


namespace u_Collision    // ------ разные мувы с коллизией -------------
{
    
    RE::hkRefPtr<RE::hkpWorldObject> _collisionObj;  // указатель в который можно присвоить controller->bumpedCharCollisionObject

    const auto colRef = RE::TESHavokUtilities::FindCollidableRef(_collisionObj->collidable);  // TESObjectREFR

    void ClearCollisionObject() { _collisionObj.reset(); }  // reset

    void SetCollisionObject (RE::Actor* a_actor)  // присваиваем в этот указатель rigitBody Ptr актера ("пикаем" актера)
    {
       _collisionObj = a_actor->GetCharController()->bumpedCharCollisionObject;
    }

    static constexpr auto COLLISION_FLAG = static_cast<std::uint32_t>(RE::CFilter::Flag::kNoCollision);  // kNoCollision flag  aka  1 << 14 (можно юзать как 1 из вариантов откл. коллизии)

    void set_collision_layer (RE::Actor* actor, RE::COL_LAYER layer);   // сетает принятый RE::COL_LAYER в соответствующие этому енаму биты (выставит текущий COL_LAYER)
    void remove_collision (RE::Actor* actor);
    void return_collision (RE::Actor* actor);   // вернуть коллизию, если отключили через RE::COL_LAYER

    // uint16_t collisionGroup = colFilterInfo >> 16;  // по этому смещению в colFilterInfo лежит collisionGroup (выцепляем)
}

    // -------------------------------------------------------------- tutor ------------------------------------------------------------------------------------------

    // RE::TESObjectWEAP *playerWeap = form->As<RE::TESObjectWEAP>();
    // .As<>() - — —≈шный способ приведени¤, шаблонный метод, внутри там cast RE::TESObjectWEAP *playerWeap = static_cast<RE::TESObjectWEAP*>(form); 
    // c++ способ приведени¤ через cast напр¤мую, должен тоже работать.

    //  refr->IsNot(RE::FormType::ActorCharacter)     //  check formtype of TESObjectREFR

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

    // auto state = pl->actorState1;                                        // [actor state - movement dir]
    // if      (state.movingForward > 0)
    // else if (state.movingBack    > 0) //...

    // if (state.meleeAttackState == RE::ATTACK_STATE_ENUM::kBowDrawn);     // [actor state - bow drawn]

    // auto proc = pl->currentProcess;                                      // [actor process - pw atk direction]
    //    if (proc && proc->high) {
    //        if (proc->high->attackData.get()) {
    //            if (proc->high->attackData.get()->data.attackType != my::pwAtkTypeStanding.p)
 
    // if ((actor->formFlags & 0x800) != 0 )                                 // check form flags  IDA ver
    // if ((actor->formFlags & RE::TESForm::RecordFlags::kDisabled) != 0 )   // +++
	// 
	// wcheck form flags  clib ver

    // auto iniCollection = RE::INISettingCollection::GetSingleton();        //  [ini settings change]
    // iniCollection->GetSetting("bAlwaysActive:General")->data.b = true;                                // disable pause in alt+tab
    // iniCollection->GetSetting("bAllowScriptedAutosave:SaveGame")->data.b = false;
    // iniCollection->GetSetting("bAllowScriptedForceSave:SaveGame")->data.b = false;
    // iniCollection->GetSetting("bDisableAutoSave:SaveGame")->data.b = true;
    // --------------------------------------------------------------------------------------------------------------------------------------------------------------------

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
        RE::Actor* PickActor(RE::bhkPickData& pick, RE::TESObjectCELL* cell, RE::NiPoint3 origin, RE::NiPoint3 dir, float dist, RE::Actor* excludedActor = nullptr);
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




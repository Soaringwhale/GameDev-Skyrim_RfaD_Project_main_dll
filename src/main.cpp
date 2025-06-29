
#pragma warning (disable : 4189) 
#pragma warning (disable : 4100)

#include "Hooks.h"
#include "Events.h"
#include "InputWatcher.h"
#include "ui.h"

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query (const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
{
#ifndef DEBUG
    auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
#else
    auto path = logger::log_directory();
    if (!path) {
        return false;
    }
    *path /= Version::PROJECT;
    *path += ".log"sv;
    auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
#endif
    auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));
#ifndef DEBUG
    log->set_level(spdlog::level::trace);
#else
    log->set_level(spdlog::level::info);
    log->flush_on(spdlog::level::info);
#endif
    spdlog::set_default_logger(std::move(log));
    spdlog::set_pattern("%g(%#): [%^%l%$] %v"s);
    logger::info(FMT_STRING("{} v{}"), Version::PROJECT, Version::NAME);
    a_info->infoVersion = SKSE::PluginInfo::kVersion;
    a_info->name = Version::PROJECT.data();
    a_info->version = Version::MAJOR;
    if (a_skse->IsEditor()) {
        logger::critical("Loaded in editor, marking as incompatible"sv);
        return false;
    }
    const auto ver = a_skse->RuntimeVersion();
    if (ver < SKSE::RUNTIME_1_5_39) {
        logger::critical(FMT_STRING("Unsupported runtime version {}"), ver.string());
        return false;
    }
    return true;
}


void hooks::install_hooks()
{
    SKSE::log::info("called install_hooks()");
    OnPlayerUpdate_Hook::install_hook();
    //OnCharacterUpdate_Hook::install_hook();  // any actor
    //OnRefrInitHavok_Hook::install_hook();    // disable havok / interactions
    //OnLoad3D_Hook::install_hook();  
    OnPhysicalHit_Hook::install_hook();
    On_AnimEvent_Hook::install_hook();
    HiddenSpeedMult_updater_Hook::install_hook();
    OnAdjustActiveEffect_Hook::install_hook();
    CastSpeed_Hook::install_hook();
    OnSpellRelease_Hook::install_hook();    // 
    OnCheckCast_Hook::install_hook();
    OnPickUpObject_Hook::install_hook();
    OnAddItemToContainer_Hook::install_hook();
    OnRemoveItem_Hook::install_hook();
    OnEquip_Hook::install_hook();
    OnUnEquip_Hook::install_hook();
    OnDispel_Hook::install_hook();
    OnJump_Hook::install_hook();
    //OnArrowHit_Hook::install_hook();
    OnArrowMeetsCollision_Hook::install_hook();
    On_MeleeWeapCollide_Hook::install_hook();
    OnProjectileLaunch_Hook::install_hook();
    //OnMagicProjHit_Hook::install_hook();
    OnEffectFinish_Hook::install_hook();
    //OnApplyResist_Hook::install_hook();
    OnPlayerDrinkPotion_Hook::install_hook();
    OnApplyPoison_Hook::install_hook();
    //OnAttackAction_Hook::install_hook();
    //OnAdvanceSkill_Hook::install_hook();
    OnHealthPercentRegen_Hook::install_hook();
    OnManaPercentRegen_Hook::install_hook();
    OnStaminaPercentRegen_Hook::install_hook();
    OnModActorValue_Hook::install_hook();
    OnModPeakActorValue_Hook::install_hook();
    // 
    //DisableShadowsHook_OnAttachArmor::install_hook();
    //DisableShadowsHook_OnAttachWeapon::install_hook();
    //DisableShadowsHook_OnStoreHeadNodes::install_hook();
    //On3rdPersonCameraUpdateRot_Hook::install_hook();

    // AI
    //EvadeProjectileChanceNPC_Hook::install_hook();
    //AttacksChanceNPC_Hook::install_hook();

    // Test_Function_Hook1::install_hook();
    // Test_Function_Hook2::install_hook();
    // Test_VFunc_Hook::install_hook();

}

void events::register_for_events()
{
    SKSE::log::info("register_for_events()");
    OnDeathEvent::GetSingleton()->register_();              // death event
    OnActivateEvent::GetSingleton()->register_();           // activate event
    MenuOpenCloseEventSink::GetSingleton()->register_();    // open/close menu event
    // LocationChangeEvent::GetSingleton()->register_();    // location change event
    // ActorCellEvent::GetSingleton()->register_();         // cell change event (player only)
    // CellAttachDetachEvent::GetSingleton()->register_();  // cell refs init when enter new cell
    // CellFullyLoadedEvent::GetSingleton()->register_();   // 
    // ObjectLoadedEvent::GetSingleton()->register_();      // object loaded event (better use 3DLoad hook)
}


static void SKSEMessageHandler (SKSE::MessagingInterface::Message* message)
{
    switch (message->type) {
    case SKSE::MessagingInterface::kDataLoaded:
        my::initGameData();
        mys::init_globs();
        hooks::install_hooks();
        description_hooks::Install_Hooks();
        events::register_for_events();
        InputWatcher::GetSingleton()->register_();   // key press event
        UISettings::get_singleton().load_From_INI();
        RfadWidget::register_();
        break;
    case SKSE::MessagingInterface::kPostLoadGame:        // Player's selected savegame has finished loading
        //my::sf_handle_reserved_MP();
        //UISettings::get_singleton().load_From_INI();   // reload ui settings
        //handle_numbers_widget ();
        break;
    case SKSE::MessagingInterface::kPreLoadGame:         
        log_pre_load_game(mys::player);  // Log just before savegame loads
        break;
    case SKSE::MessagingInterface::kNewGame:             // ng
        //UISettings::get_singleton().load_From_INI();
        //RfadWidget::show();
        break;
    }
    // kPostLoad runs after the plugin loaded, kPostPost is after all plugins
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load (const SKSE::LoadInterface* a_skse)
{
    auto g_messaging = reinterpret_cast<SKSE::MessagingInterface*>(a_skse->QueryInterface(SKSE::LoadInterface::kMessaging));
    if (!g_messaging) {
        logger::critical("Failed to load messaging interface! This error is fatal, plugin will not load.");
        return false;
    }

    logger::info("loaded");

    SKSE::Init(a_skse);
    SKSE::AllocTrampoline(1 << 10);

    g_messaging->RegisterListener("SKSE", SKSEMessageHandler);

    return true;
}

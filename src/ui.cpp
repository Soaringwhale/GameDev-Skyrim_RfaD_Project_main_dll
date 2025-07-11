
#pragma warning (disable : 4189) 

#include "ui.h"
#include "utils.h"
#include "core.h"
#include "Events.h"
#include "SimpleIni.h"



RE::BSEventNotifyControl events::MenuOpenCloseEventSink::ProcessEvent (const RE::MenuOpenCloseEvent* ev, RE::BSTEventSource<RE::MenuOpenCloseEvent>*)  // catch menues open/close
{

    if (ev) {
        if (ev->menuName == RE::SleepWaitMenu::MENU_NAME)
        {
            if (ev->opening) on_wait_menu_open();
            else             on_wait_menu_close();
        }
        else if (ev->menuName == RE::InventoryMenu::MENU_NAME)
        {
            if (ev->opening) on_inventory_open();
            else             on_inventory_close();
            // event->opening ? RfadWidget::toggle_visibility(false) : RfadWidget::toggle_visibility(true);
        }
        else if (ev->menuName == RE::HUDMenu::MENU_NAME)      {
            ev->opening ? RfadWidget::show() : RfadWidget::hide();
        }
        else if (ev->menuName == RfadWidget::TRUE_HUD)        {
            ev->opening ? RfadWidget::show() : RfadWidget::hide();
        }
        else if (ev->menuName == RE::LoadingMenu::MENU_NAME)  {
            ev->opening ? RfadWidget::hide() : RfadWidget::show();
        }
        //else if (event->menuName == RE::JournalMenu::MENU_NAME) {
        //     Settings::get_singleton().load();
        //     ev->opening ? RfadWidget::toggle_visibility(false) : RfadWidget::toggle_visibility (true);
        //}
        //else if (event->menuName == RE::MapMenu::MENU_NAME) {
        //   event->opening ? RfadWidget::toggle_visibility(false) : RfadWidget::toggle_visibility(true);
        //}
        //else if (event->menuName == RE::TweenMenu::MENU_NAME) {
        //   event->opening ? RfadWidget::toggle_visibility(false) : RfadWidget::toggle_visibility(true);
        //}
        //else if (event->menuName == RE::MagicMenu::MENU_NAME) {
        //   event->opening ? RfadWidget::toggle_visibility(false) : RfadWidget::toggle_visibility(true);
        //}
        //else if (event->menuName == RE::FaderMenu::MENU_NAME) {
        //   event->opening ? RfadWidget::hide() : RfadWidget::show();
        //}

    }

    // copypast from true hud
    using ContextID = RE::UserEvents::INPUT_CONTEXT_ID;
    // Hide the widgets when a menu is open
    auto control_map = RE::ControlMap::GetSingleton();
    if (control_map) {
        auto& priorityStack = control_map->contextPriorityStack;
        if (priorityStack.empty())
        {
            RfadWidget::toggle_visibility(false);
        }
        else if (priorityStack.back() == ContextID::kGameplay || priorityStack.back() == ContextID::kFavorites || priorityStack.back() == ContextID::kConsole)
        {
            RfadWidget::toggle_visibility(true);
        }
        else if ((priorityStack.back() == ContextID::kCursor ||
                    priorityStack.back() == ContextID::kItemMenu ||
                    priorityStack.back() == ContextID::kMenuMode ||
                    priorityStack.back() == ContextID::kInventory) &&
                   (RE::UI::GetSingleton()->IsMenuOpen(RE::DialogueMenu::MENU_NAME) ||
                    RE::UI::GetSingleton()->IsMenuOpen(RE::CraftingMenu::MENU_NAME) ||
                    (RE::UI::GetSingleton()->IsMenuOpen(RE::BarterMenu::MENU_NAME) ||
                     RE::UI::GetSingleton()->IsMenuOpen(RE::ContainerMenu::MENU_NAME) ||
                     RE::UI::GetSingleton()->IsMenuOpen(RE::GiftMenu::MENU_NAME) ||
                     RE::UI::GetSingleton()->IsMenuOpen(RE::InventoryMenu::MENU_NAME))))
        {
            RfadWidget::toggle_visibility(false);
        }
        else
        {
            RfadWidget::toggle_visibility(false);
        }
    }

    return RE::BSEventNotifyControl::kContinue;
}
//--------------------------------------------------------------- end process event


RfadWidget::RfadWidget()    // ctor
{
    // auto scale_form_manager = RE::BSScaleformManager::GetSingleton();

    LOG ("RfadWidget() called c-tor");

    inputContext = Context::kNone;
    depthPriority = 0;

    menuFlags.set(RE::UI_MENU_FLAGS::kAlwaysOpen);            // IMenu::menuFlags
    menuFlags.set(RE::UI_MENU_FLAGS::kRequiresUpdate);
    menuFlags.set(RE::UI_MENU_FLAGS::kAllowSaving);
    // menuFlags.set(RE::UI_MENU_FLAGS::kCustomRendering);
    // menuFlags.set(RE::UI_MENU_FLAGS::kAssignCursorToRenderer);

    if (uiMovie) {
        uiMovie->SetMouseCursorCount(0);
        uiMovie->SetVisible(true);
    }

    // scale_form_manager->LoadMovieEx(this, MENU_PATH, [](RE::GFxMovieDef* def) -> void {
    //   def->SetState(RE::GFxState::StateType::kLog, RE::make_gptr<RfadLogger>().get());
    // });

    if (auto scaleform = RE::BSScaleformManager::GetSingleton()) {
        scaleform->LoadMovie(this, this->uiMovie, MENU_PATH);
    }

    LOG("RfadWidget() finished c-tor");
}

void RfadWidget::register_()
{
    if (mys::widget_shown->value == 0) return;
    if (auto ui = RE::UI::GetSingleton()) {
        ui->Register (MENU_NAME, creator);
        LOG ("Register menu");
        //show();
    }
}

void RfadWidget::show()
{
    LOG("called RfadWidget::show()");
    if (mys::widget_shown->value == 0) return;
    if (auto message_queue = RE::UIMessageQueue::GetSingleton()) {
            message_queue->AddMessage(MENU_NAME, RE::UI_MESSAGE_TYPE::kShow, nullptr);
            
    }
}

void RfadWidget::hide()
{
    LOG("called RfadWidget::hide()");
    //if(mys::widget_shown->value > 0.f) return;
    if (auto message_queue = RE::UIMessageQueue::GetSingleton()) {
            message_queue->AddMessage(MENU_NAME, RE::UI_MESSAGE_TYPE::kHide, nullptr);
    }
}

void RfadWidget::toggle_visibility (bool mode)
{
    LOG("called RfadWidget::toggle_visibility({})", mode);
    if (mode && mys::widget_shown->value == 0) return;
    auto ui = RE::UI::GetSingleton();
    if (!ui)  return;
    auto overlay_menu = ui->GetMenu(MENU_NAME);
    if (!overlay_menu || !overlay_menu->uiMovie) return;
    overlay_menu->uiMovie->SetVisible(mode);
}


void toggle_numbers_widget ()   // core.h  handle helper func (F10)
{
    LOG("called toggle_numbers_widget()");
    if (mys::widget_shown->value > 0) {
        mys::widget_shown->value = 0;
        RfadWidget::hide();
        RfadWidget::toggle_visibility(false);
    }
    else {
        mys::widget_shown->value = 1.f;
        RfadWidget::show();
        RfadWidget::toggle_visibility(true);
    }
}

void handle_numbers_widget ()   // core.h  handle helper func (update state)
{  
    LOG("called handle_numbers_widget()");
    if(mys::ui->IsMenuOpen (RE::DialogueMenu::MENU_NAME) ||
       mys::ui->IsMenuOpen(RE::InventoryMenu::MENU_NAME)) return;    // don't handle in dialogues or inventory
    if (mys::widget_shown->value == 1.f) {
        RfadWidget::show();
        RfadWidget::toggle_visibility(true);
    }
    //else {
    //    RfadWidget::hide();
    //    RfadWidget::toggle_visibility(false);
    //}
}

float RfadWidget::av_total_regen_value (const RE::ActorValue av, const RE::ActorValue av_rate, const RE::ActorValue av_rate_mult)
{

    auto player = RE::PlayerCharacter::GetSingleton();
    if (!player) return 0.f;

    auto active_effects = player->GetActiveEffectList();
    if (!active_effects) return 0.f;

    auto valid_effect = [](const RE::ActiveEffect* active_effect) -> bool
    {
        return active_effect && !active_effect->flags.any(RE::ActiveEffect::Flag::kInactive) && active_effect->effect &&
               active_effect->effect->baseEffect &&
               !active_effect->effect->baseEffect->data.flags.any(RE::EffectSetting::EffectSettingData::Flag::kRecover) &&
               !active_effect->effect->baseEffect->IsDetrimental();
    };

    auto value_or_peakvalue_mod = [av](const RE::ActiveEffect* active_effect) -> bool
    {
        return (active_effect->effect->baseEffect->HasArchetype(RE::EffectSetting::Archetype::kPeakValueModifier) ||
                active_effect->effect->baseEffect->HasArchetype(RE::EffectSetting::Archetype::kValueModifier)) &&
                active_effect->effect->baseEffect->data.primaryAV == av;
    };

    auto restore_value_counter = 0.f;

    for (auto active_effect : *active_effects)
    {
        if (!valid_effect(active_effect)) continue;

        if (value_or_peakvalue_mod(active_effect)) {
            restore_value_counter += active_effect->magnitude;
            continue;
        }

        if (active_effect->effect->baseEffect->HasArchetype(RE::EffectSetting::Archetype::kDualValueModifier)) {
            auto base_effect = active_effect->effect->baseEffect;

            if (base_effect->data.primaryAV == av && base_effect->data.secondaryAV != av) {
                restore_value_counter += active_effect->magnitude;
            } else if (base_effect->data.primaryAV == av && base_effect->data.secondaryAV == av) {
                restore_value_counter += (active_effect->magnitude * base_effect->data.secondAVWeight);
                restore_value_counter += active_effect->magnitude;
            } else if (base_effect->data.secondaryAV == av) {
                restore_value_counter += (active_effect->magnitude * base_effect->data.secondAVWeight);
            }
        }
    }

    auto regeneration = 0.f;
    auto rate = player->GetActorValue(av_rate);
    auto mult_rate = player->GetActorValue(av_rate_mult);

    if (rate <= 0.f || mult_rate <= 0.f) {
        regeneration = 0.f;
    }
    else {
        auto max_value = u_get_actor_value_max (player, av);
        regeneration = (max_value * (rate * 0.01f)) * (mult_rate * 0.01f);
    }

    return restore_value_counter + regeneration;
}

static int updater = 0;

void RfadWidget::update()
{
    if (mys::widget_shown->value == 0) return;
    updater++;
    if (updater < 10) return;    // every 10 update   // ~  6 times / sec
    else updater = 0;

    auto ui = RE::UI::GetSingleton();
    if (!ui || ui->GameIsPaused())  return;

    RE::GPtr<RE::IMenu> Rfad_widget = ui->GetMenu (MENU_NAME);   //
    if (!Rfad_widget || !Rfad_widget->uiMovie)  return;                                                                                                                                                                                                                                                                                                                            

    auto pl = RE::PlayerCharacter::GetSingleton();

    using string = std::string;

    string hp_regen_str = std::to_string(int(av_total_regen_value(RE::ActorValue::kHealth, RE::ActorValue::kHealRate, RE::ActorValue::kHealRateMult))) + string("/s");
    string st_regen_str = std::to_string(int(av_total_regen_value(RE::ActorValue::kStamina, RE::ActorValue::kStaminaRate, RE::ActorValue::kStaminaRateMult)))+ string("/s");
    string mp_regen_str = std::to_string(int(av_total_regen_value(RE::ActorValue::kMagicka, RE::ActorValue::kMagickaRate, RE::ActorValue::kMagickaRateMult)))+ string("/s");

    string hp_view_str = std::to_string(int(pl->GetActorValue(RE::ActorValue::kHealth))) + string("/") 
                       + std::to_string(int(u_get_actor_value_max(pl, RE::ActorValue::kHealth)));         
    string st_view_str = std::to_string(int(pl->GetActorValue(RE::ActorValue::kStamina))) + string("/") +
                         std::to_string(int(u_get_actor_value_max(pl, RE::ActorValue::kStamina)));               // like "450/600"
    string mp_view_str = std::to_string(int(pl->GetActorValue(RE::ActorValue::kMagicka))) + string("/") +
                         std::to_string(int(u_get_actor_value_max(pl, RE::ActorValue::kMagicka)));

    string arcaneCurseAmountStr = std::to_string(int(pl->GetActorValue(RE::ActorValue::kEnchantingSkillAdvance)));   // arcane curse

    const RE::GFxValue health_regen  {hp_regen_str};
    const RE::GFxValue stamina_regen {st_regen_str};
    const RE::GFxValue magicka_regen {mp_regen_str};
    const RE::GFxValue value_health  {hp_view_str};
    const RE::GFxValue value_stamina {st_view_str};
    const RE::GFxValue value_magicka {mp_view_str};
    const RE::GFxValue arcaneCurseAmount {arcaneCurseAmountStr};

    Rfad_widget->uiMovie->Invoke ("rfadwidgetframe1.setHealthText", nullptr, &health_regen, 1);      // invoke - прокидываем значение в swf и вызываем функцию из ActionScript
    Rfad_widget->uiMovie->Invoke ("rfadwidgetframe1.setStaminaText", nullptr, &stamina_regen, 1);
    Rfad_widget->uiMovie->Invoke ("rfadwidgetframe1.setMagickaText", nullptr, &magicka_regen, 1);               //  методы из ActionScript, вызываемые у frame1
    Rfad_widget->uiMovie->Invoke ("rfadwidgetframe1.setValueHealthText", nullptr, &value_health, 1);            //  (т.к. у нас нет анимаций, текст это всего 1 кадр который обновляется)
    Rfad_widget->uiMovie->Invoke ("rfadwidgetframe1.setValueStaminaText", nullptr, &value_stamina, 1);
    Rfad_widget->uiMovie->Invoke ("rfadwidgetframe1.setValueMagickaText", nullptr, &value_magicka, 1);

    Rfad_widget->uiMovie->Invoke ("rfadwidgetframe1.updateArcaneFatigueValue", nullptr, &arcaneCurseAmount, 1);

    apply_layout (Rfad_widget);
}


void RfadWidget::apply_layout (const RE::GPtr<RE::IMenu>& Rfad_widget)      // применение перерисовки
{

    if (!Rfad_widget || !Rfad_widget->uiMovie) return;

    auto settings = UISettings::get_singleton();

    const RE::GFxValue scale = settings.scale_;
    const RE::GFxValue pos_regen_hp_x = settings.hp_regen_pos_x;
    const RE::GFxValue pos_regen_hp_y = settings.hp_regen_pos_y;
    const RE::GFxValue pos_regen_st_x = settings.st_regen_pos_x;
    const RE::GFxValue pos_regen_st_y = settings.st_regen_pos_y;
    const RE::GFxValue pos_regen_mp_x = settings.mp_regen_pos_x;
    const RE::GFxValue pos_regen_mp_y = settings.mp_regen_pos_y;
    const RE::GFxValue pos_hp_x = settings.hp_pos_x;
    const RE::GFxValue pos_hp_y = settings.hp_pos_y;
    const RE::GFxValue pos_st_x = settings.st_pos_x;
    const RE::GFxValue pos_st_y = settings.st_pos_y;
    const RE::GFxValue pos_mp_x = settings.mp_pos_x;
    const RE::GFxValue pos_mp_y = settings.mp_pos_y;   // numbers on bars

    const RE::GFxValue pos_ac_x = settings.ac_pos_x;   // arcane curse art pos
    const RE::GFxValue pos_ac_y = settings.ac_pos_y;
    const RE::GFxValue ac_scale = settings.ac_scale;

    Rfad_widget->uiMovie->Invoke("rfadwidgetframe1.setHealthY", nullptr, &pos_regen_hp_y, 1);
    Rfad_widget->uiMovie->Invoke("rfadwidgetframe1.setHealthX", nullptr, &pos_regen_hp_x, 1);
    Rfad_widget->uiMovie->Invoke("rfadwidgetframe1.setStaminaY", nullptr, &pos_regen_st_y, 1);
    Rfad_widget->uiMovie->Invoke("rfadwidgetframe1.setStaminaX", nullptr, &pos_regen_st_x, 1);
    Rfad_widget->uiMovie->Invoke("rfadwidgetframe1.setMagickaY", nullptr, &pos_regen_mp_y, 1);
    Rfad_widget->uiMovie->Invoke("rfadwidgetframe1.setMagickaX", nullptr, &pos_regen_mp_x, 1);
    Rfad_widget->uiMovie->Invoke("rfadwidgetframe1.setValueHealthY", nullptr, &pos_hp_y, 1);   
    Rfad_widget->uiMovie->Invoke("rfadwidgetframe1.setValueHealthX", nullptr, &pos_hp_x, 1);
    Rfad_widget->uiMovie->Invoke("rfadwidgetframe1.setValueStaminaY", nullptr, &pos_st_y, 1);
    Rfad_widget->uiMovie->Invoke("rfadwidgetframe1.setValueStaminaX", nullptr, &pos_st_x, 1);
    Rfad_widget->uiMovie->Invoke("rfadwidgetframe1.setValueMagickaY", nullptr, &pos_mp_y, 1);
    Rfad_widget->uiMovie->Invoke("rfadwidgetframe1.setValueMagickaX", nullptr, &pos_mp_x, 1);
    Rfad_widget->uiMovie->SetVariable("rfadwidgetframe1._xscale", scale);
    Rfad_widget->uiMovie->SetVariable("rfadwidgetframe1._yscale", scale);

    Rfad_widget->uiMovie->Invoke("rfadwidgetframe1.setValueArcaneCurseX", nullptr, &pos_ac_x, 1);
    Rfad_widget->uiMovie->Invoke("rfadwidgetframe1.setValueArcaneCurseY", nullptr, &pos_ac_y, 1);
    Rfad_widget->uiMovie->Invoke("rfadwidgetframe1.setArcaneCurseScale",  nullptr, &ac_scale, 1);
}

void RfadWidget::AdvanceMovie (const float interval, const uint32_t current_time) 
{
    //LOG("AdvanceMovie");
    update();
    IMenu::AdvanceMovie(interval, current_time);
}

void UISettings::load_From_INI()  // using SimpleIni.h
{
        LOG("called load_From_INI()");
        constexpr auto path_to_ini = L"Data/MCM/Settings/Reflyem - Widget.ini";
        constexpr auto path_to_ini_default = L"Data/interface/RfadWidgetSettings/settings.ini";   // my

        constexpr auto section = L"Main";
        constexpr auto f_pos_regen_health_x = L"fPosRegenHealthX";
        constexpr auto f_pos_regen_health_y = L"fPosRegenHealthY";
        constexpr auto f_pos_regen_stamina_x = L"fPosRegenStaminaX";
        constexpr auto f_pos_regen_stamina_y = L"fPosRegenStaminaY";
        constexpr auto f_pos_regen_magicka_x = L"fPosRegenMagickaX";
        constexpr auto f_pos_regen_magicka_y = L"fPosRegenMagickaY";
        constexpr auto f_pos_value_health_x = L"fPosValueHealthX";
        constexpr auto f_pos_value_health_y = L"fPosValueHealthY";
        constexpr auto f_pos_value_stamina_x = L"fPosValueStaminaX";
        constexpr auto f_pos_value_stamina_y = L"fPosValueStaminaY";
        constexpr auto f_pos_value_magicka_x = L"fPosValueMagickaX";
        constexpr auto f_pos_value_magicka_y = L"fPosValueMagickaY";
        constexpr auto f_scale_widget = L"fScaleWidget";

        constexpr auto f_pos_arcane_curse_x = L"fPosArcaneCurseX";
        constexpr auto f_pos_arcane_curse_y = L"fPosArcaneCurseY";
        constexpr auto f_scale_arcane_curse = L"fScaleArcaneCurseWidget";

        auto read_double = [this](const CSimpleIni& ini, const wchar_t* key, double& value) -> void
        {
            if (ini.GetValue(section, key)) {
                value = ini.GetDoubleValue(section, key);
            }
        };

        auto read_settings = [this, read_double](CSimpleIni& ini, const wchar_t* path) -> void
        {
            ini.LoadFile(path);
            if (ini.IsEmpty()) return;

            read_double (ini, f_pos_regen_health_x,  hp_regen_pos_x);          // read from ini to class fields
            read_double (ini, f_pos_regen_health_y,  hp_regen_pos_y);
            read_double (ini, f_pos_regen_stamina_x, st_regen_pos_x);
            read_double (ini, f_pos_regen_stamina_y, st_regen_pos_y);
            read_double (ini, f_pos_regen_magicka_x, mp_regen_pos_x);
            read_double (ini, f_pos_regen_magicka_y, mp_regen_pos_y);
            read_double (ini, f_pos_value_health_x, hp_pos_x);
            read_double (ini, f_pos_value_health_y, hp_pos_y);
            read_double (ini, f_pos_value_stamina_x, st_pos_x);
            read_double (ini, f_pos_value_stamina_y, st_pos_y);
            read_double (ini, f_pos_value_magicka_x, mp_pos_x);
            read_double (ini, f_pos_value_magicka_y, mp_pos_y);
            read_double (ini, f_scale_widget, scale_);

            read_double (ini, f_pos_arcane_curse_x, ac_pos_x);
            read_double (ini, f_pos_arcane_curse_y, ac_pos_y);
            read_double (ini, f_scale_arcane_curse, ac_scale);
        };

        CSimpleIni ini;
        ini.SetUnicode();
        read_settings (ini, path_to_ini_default);
        // read_settings (ini, path_to_ini);
}

#pragma once



//-------------------------------------------------------  The Widget  -------------------------------------
class RfadWidget : public RE::IMenu
{
  public:
    static constexpr const char* MENU_PATH = "RfadWidget";
    static constexpr const char* MENU_NAME = "RfadWidget";
    static constexpr std::string_view TRUE_HUD = "TrueHUD";       // райт почти ничего не использует из трухуда, только одну желтую полоску, а так есть api трухуда на гите

    RfadWidget();

    static void register_();
    static void show();
    static void hide();
    static void update();
    static void apply_layout  (const RE::GPtr<RE::IMenu>& rfad_widget);
    static void toggle_visibility (bool mode);

    static float av_total_regen_value (const RE::ActorValue av, const RE::ActorValue av_rate, const RE::ActorValue av_rate_mult);   // for numbers on bars

    // static auto creator() -> RE::stl::owner<RE::IMenu*> { return new RfadWidget(); }
    static RE::IMenu* creator() { return new RfadWidget(); }

    auto AdvanceMovie (float interval, uint32_t current_time) -> void override;

};

// -------------------------------------------------------------------- settings  ----------------------------------------------------------

struct UISettings
{
  public:
     auto scale() const -> double { return scale_; }

     auto get_hp_regen_pos_x() const -> double { return hp_regen_pos_x; }
     auto get_hp_regen_pos_y() const -> double { return hp_regen_pos_y; }
     auto get_st_regen_pos_x() const -> double { return st_regen_pos_x; }
     auto get_st_regen_pos_y() const -> double { return st_regen_pos_y; }
     auto get_mp_regen_pos_x() const -> double { return mp_regen_pos_x; }
     auto get_mp_regen_pos_y() const -> double { return mp_regen_pos_y; }  // getters

     auto get_hp_pos_x() const -> double { return hp_pos_x; }
     auto get_hp_pos_y() const -> double { return hp_pos_y; }
     auto get_st_pos_x() const -> double { return st_pos_x; }
     auto get_st_pos_y() const -> double { return st_pos_y; }
     auto get_mp_pos_x() const -> double { return mp_pos_x; }
     auto get_mp_pos_y() const -> double { return mp_pos_y; }

    [[nodiscard]] static auto get_singleton() noexcept -> UISettings& {
         static UISettings instance;
        return instance;
    }

    void load_From_INI();   // read from ini to fields

  private:
    UISettings() = default;

    double scale_{100.};

    double hp_regen_pos_x {210.};
    double hp_regen_pos_y {600.};
    double st_regen_pos_x {210.};
    double st_regen_pos_y {660.};
    double mp_regen_pos_x {210.};
    double mp_regen_pos_y {630.};  // fields

    double hp_pos_x {110.};
    double hp_pos_y {600.};
    double st_pos_x {110.};
    double st_pos_y {660.};
    double mp_pos_x {110.};
    double mp_pos_y {630.};
};

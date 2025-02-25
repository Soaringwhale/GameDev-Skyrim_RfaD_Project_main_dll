
// base from DescriptionFramework by NightfallStorm

#pragma once

enum ItemCardType : int {
    ICT_NONE,
    ICT_ARMOR,
    ICT_WEAPON,
    ICT_MISC,
    ICT_BOOK,
    ICT_FOOD,
    ICT_POTION,
    ICT_SPELL,
    ICT_INGREDIENT,
    ICT_KEY,
    ICT_SHOUT,
    ICT_ACTIVE_EFFECT,
    ICT_SOUL_GEMS,
    ICT_SPELL_DEFAULT,
    ICT_LIST,
    ICT_CRAFT_ENCHANTING,
    ICT_HOUSE_PART,
};

class ItemCardFixer
{
  public:
    static inline constexpr auto descriptionVar = "DF_description";
    static inline constexpr auto appliedDescriptionVar = "DF_fixed";
    static inline constexpr auto itemInfoVar = "itemInfo";
    static inline constexpr auto typeVar = "type";

    static inline int debugBorder = 0xFC0303;
    static inline int descriptionDebugBorder = 0x02f70f;
    // static inline RE::GFxValue wideBackground;

    static inline RE::GPtr<RE::GFxMovieView> uiMovie;

    struct CollectDFVariables : RE::GFxValue::ObjectVisitor {
    public:
        // Collect DF variables so we can erase them when reverting description
        virtual void Visit (const char* a_name, [[maybe_unused]] const RE::GFxValue& a_val) override {
            if (std::string(a_name).find("DF_") != std::string::npos) {
                collectedVariableNames.emplace_back(a_name);
            }
        }

        std::list<const char*> collectedVariableNames;
    };

    struct AddDisplayBorder : RE::GFxValue::ObjectVisitor {
    public:
        // Collect DF variables so we can erase them when reverting description
        virtual void Visit(const char* a_name, const RE::GFxValue& a_val) override {
            RE::GFxValue borderColor;
            RE::GFxValue borderNumber;

            if (!a_val.IsDisplayObject()) {
                return;
            }

            a_val.GetMember("borderColor", &borderColor);
            if (borderColor.IsNumber() && borderColor.GetNumber() == descriptionDebugBorder) {
                return;
            }

            //logger::debug("Applying debug border to {}", a_name);
            RE::GFxValue displayVal = a_val;
            displayVal.SetMember("border", true);
            displayVal.SetMember("borderColor", debugBorder);
        }
    };

    static void applyDescription (RE::GFxValue a_itemCard)       // a_itemCard это карточка предмета на которое мы условно навелись
    {
        //SKSE::log::info("called applyDescription() from fixer.h");
        // 
        // ниже проверки, смотрим различные поля из ui карточек (GetMember)

        if (!a_itemCard.IsObject()) return;

        RE::GFxValue a_itemInfo;
        a_itemCard.GetMember(itemInfoVar, &a_itemInfo);
        if (!a_itemInfo.IsObject()) {
            //logger::debug("No iteminfo object, returning");     // проверка что у карточки есть iteminfo
            return;
        }

        RE::GFxValue a_description;
        a_itemInfo.GetMember(descriptionVar, &a_description);    // проверка что в iteminfo есть member нового описания (почти всегда выход будет здесь, кроме предметов с файла)
        if (!a_description.IsString()) {
            //logger::debug("No description present, returning");
            return;
        }

        RE::GFxValue a_type;
        a_itemInfo.GetMember(typeVar, &a_type);
        if (!a_type.IsNumber()) {
            //logger::debug("Not type number, returning");
            return;
        }

        RE::GFxValue appliedDescription;
        a_itemInfo.GetMember(appliedDescriptionVar, &appliedDescription);     // получаем булевый параметр проверяющий есть ли уже (наше новое?) описание в iteminfo

        if (appliedDescription.IsBool() && appliedDescription.GetBool()) {    // если (наше новое?) описание уже есть в iteminfo то выходим, во избежание дублированных описаний
            return;
        }

        //RE::GFxValue itemName;                               // получаем параметр - имя самого предмета
        //a_itemInfo.GetMember("name", &itemName);
        //logger::debug("Fixing up item {} with item type {} using description {}", itemName.GetString(), a_type.GetUInt(), a_description.GetString());
        // дебаг - например - Обновляем предмет Меч Храмовника с типом 3 и описанием "..."


        // к-тор фиксера принимает ПО ССЫЛКЕ itemCard, itemInfo, description. Далее он будет редактировать эти данные конкретного предмета.
        ItemCardFixer* fixer = new ItemCardFixer(a_itemCard, a_itemInfo, a_description.GetString(), (ItemCardType)a_type.GetUInt());


        // 1 - исправим iteminfo, чтобы применить описание там, где это необходимо
        fixer->applyDescription();

        // 2 - re-apply item info, чтобы itemcard отображалась в right frame for an object
        fixer->itemCard.SetMember(itemInfoVar, a_itemInfo);

        // 3 - исправляем саму itemcard по мере необходимости  [возможно это стиль, мб мне это не нужно]
        fixer->fixItemCard();

        delete fixer;

        appliedDescription.SetBoolean(true);    
        a_itemInfo.SetMember(appliedDescriptionVar, appliedDescription);    // записываем в это iteminfo что для данного предмета мы уже вставили свое описание
    }

    static void revertDescription (RE::GFxValue a_itemCard)
    {
        if (!a_itemCard.IsObject()) return;
        RE::GFxValue a_itemInfo;
        a_itemCard.GetMember(itemInfoVar, &a_itemInfo);
        if (!a_itemInfo.IsObject()) return;

        // First, remove all DF variables used except the one holding the current description
        
        a_itemCard.DeleteMember(appliedDescriptionVar);
        a_itemInfo.DeleteMember(appliedDescriptionVar);

        // Second, apply a blank itemInfo to reset SWF elements
        RE::GFxValue origType;
        a_itemInfo.GetMember("type", &origType);
        a_itemInfo.SetMember("type", ICT_NONE);

        a_itemCard.SetMember("itemInfo", a_itemInfo);  // This will skip to the empty itemCard frame, clearing out all previous SWF elements we touched

        // Third, reapply original itemInfo to get the item card where it should be
        a_itemInfo.SetMember("type", origType);
        a_itemCard.SetMember("itemInfo", a_itemInfo);
    }

private:
    ItemCardFixer (RE::GFxValue& a_itemCard, RE::GFxValue& a_itemInfo, const char* a_description, ItemCardType a_type) {
        assert(!a_itemCard.IsUndefined());
        assert(!a_itemInfo.IsUndefined());
        assert(strcmp(a_description, "") != 0);

        itemCard = a_itemCard;
        itemInfo = a_itemInfo;
        type = a_type;
        description = a_description;
    }
    void handleEffects();
    void handleSoulLVL();
    void fixTest(float x, float y, float width, float height);
    void fixHTML(const char* a_displayVariable, const char* a_descriptionVariable);
    void fixBackground(const char* a_displayLabel);

    void handleArmor();
    void handleWeapon();
    void handleMisc();
    void handleBook();
    void handleFood();
    void handlePotion();
    void handleSpell();
    void handleIngredient();
    void handleKey();
    void handleShout();
    void handleActiveEffect();
    void handleSoulGems();
    void handleSpellDefault();
    void handleList();
    void handleCraftEnchanting();
    void handleHousePart();

    void fixArmor();
    void fixWeapon();
    void fixMisc();
    void fixBook();
    void fixFood();
    void fixPotion();
    void fixSpell();
    void fixIngredient();
    void fixKey();
    void fixShout();
    void fixActiveEffect();
    void fixSoulGems();
    void fixSpellDefault();
    void fixList();
    void fixCraftEnchanting();
    void fixHousePart();

    void applyDescription();
    void fixItemCard();

    RE::GFxValue itemInfo;
    RE::GFxValue itemCard;
    ItemCardType type;
    const char* description;
};

#pragma warning (disable : 4244)
#pragma warning (disable : 4702)

#include "ItemCardFixer.h"

void ItemCardFixer::handleEffects()       // именно здесь происходит замена описания, подменяя поле "effects". Иммерсивное описание в самой шмотке тоже хранится в них
{
    //SKSE::log::info("called handleEffects() from fixer.cpp");

    auto effectsStr = std::string("");
    RE::GFxValue effects;
    itemInfo.GetMember("effects", &effects);
    if (effects.IsString() && std::strcmp(effects.GetString(), "undefined") && std::strcmp(effects.GetString(), "")) {
        effectsStr = std::string(effects.GetString());
        effectsStr.erase(remove(effectsStr.begin(), effectsStr.end(), ' '), effectsStr.end());
        if (!effectsStr.empty()) {
            effectsStr = std::string(effects.GetString()) + "\n";
        }
    }
    if (!effectsStr.empty()) {
        // SKSE::log::info("effects str - {}", effectsStr.c_str());
        auto len = effectsStr.length();
        for (size_t i = 0; i < len - 2; i++) effectsStr.pop_back();
        effectsStr = "(" + effectsStr;
        effectsStr.pop_back();
        effectsStr.pop_back();
        auto effectsDesc = effectsStr + this->description + ")";
        itemInfo.SetMember("effects", effectsDesc.c_str());
    }
}

// --------------------------------------------------------- ВСЕ ЧТО НИЖЕ МНЕ ВЕРОЯТНО НЕ НУЖНО ---------------------------------------------------

void ItemCardFixer::handleSoulLVL()
{
    // Append description to soulLVL, while faking as a book
    auto soulStr = std::string("");
    RE::GFxValue soulLVL;
    itemInfo.GetMember("soulLVL", &soulLVL);
    if (soulLVL.IsString() && std::strcmp(soulLVL.GetString(), "undefined") && std::strcmp(soulLVL.GetString(), "")) {
        soulStr = std::string(soulLVL.GetString());
        soulStr.erase(remove(soulStr.begin(), soulStr.end(), ' '), soulStr.end());
        if (!soulStr.empty()) {
            soulStr = std::string(soulLVL.GetString()) + "\n";
        }
    }

    auto soulsDesc = soulStr + this->description;
    itemInfo.SetMember("description", soulsDesc.c_str());
    itemInfo.SetMember(ItemCardFixer::typeVar, ItemCardType::ICT_BOOK);
}

void ItemCardFixer::fixHTML (const char* a_displayVariable, const char* a_descriptionVariable)
{
    RE::GFxValue displayLabel;
    RE::GFxValue descriptionLabel;
    RE::GFxValue heightLabel;
    itemCard.GetMember(a_displayVariable, &displayLabel);
    itemInfo.GetMember(a_descriptionVariable, &descriptionLabel);
    if (displayLabel.IsDisplayObject() && descriptionLabel.IsString()) {
        displayLabel.SetMember("textAutoSize", "shrink");
        displayLabel.SetTextHTML(descriptionLabel.GetString());

        // Make text clearer for easier reading
        displayLabel.SetMember("antiAliasType", "advanced");
        displayLabel.SetMember("gridFitType", "pixel");
    }    
}

void ItemCardFixer::fixBackground(const char* a_displayLabel)
{
    return; // TODO: Re-enable background scaling when properly fixed
    // Increase the background size, and the display label to match
    float heightScale {1.f};
    float widthScale  {1.f};
    float heightOffset{0.f};
    float widthOffset {0.f};

    RE::GFxValue background;
    RE::GFxValue backgroundFixed;
    RE::GFxValue displayLabel;
    RE::GFxValue backgroundX;
    RE::GFxValue backgroundY;
    RE::GFxValue displayX;
    RE::GFxValue displayY;
    RE::GFxValue backgroundHeight;
    RE::GFxValue backgroundWidth;

    itemCard.GetMember("background", &background);
    itemCard.GetMember(a_displayLabel, &displayLabel);

    background.GetMember("DF_backgroundFixed", &backgroundFixed);
    background.GetMember("_height", &backgroundHeight);
    background.GetMember("_width", &backgroundWidth);
    background.GetMember("_y", &backgroundY);
    background.GetMember("_x", &backgroundX);
    displayLabel.GetMember("_x", &displayX);
    displayLabel.GetMember("_y", &displayY);

    // If background is already scaled, don't scale it again
    if (!backgroundFixed.IsBool() || !backgroundFixed.GetBool()) {
        backgroundHeight.SetNumber(backgroundHeight.GetNumber() * heightScale);
        background.SetMember("_height", backgroundHeight);
        backgroundY.SetNumber(backgroundY.GetNumber() + heightOffset);
        background.SetMember("_y", backgroundY);

        backgroundWidth.SetNumber(backgroundWidth.GetNumber() * widthScale);
        background.SetMember("_width", backgroundWidth);
        backgroundX.SetNumber(backgroundX.GetNumber() + widthOffset);
        background.SetMember("_x", backgroundX);

        background.SetMember("DF_backgroundFixed", true);
    }
    
    auto bottomY = backgroundY.GetNumber() + backgroundHeight.GetNumber();
    auto newDisplayHeight = bottomY - displayY.GetNumber();
    newDisplayHeight -= 15; // Padding
    displayLabel.SetMember("_height", newDisplayHeight);

    displayX.SetNumber(backgroundX.GetNumber() + 15);
    displayLabel.SetMember("_x", displayX);
    auto rightX = backgroundX.GetNumber() + backgroundWidth.GetNumber();
    auto newDisplayWidth = rightX - displayX.GetNumber();
    newDisplayWidth -= 15;  // Padding
    displayLabel.SetMember("_width", newDisplayWidth);

    fixTest(backgroundX.GetNumber(), backgroundY.GetNumber(), backgroundWidth.GetNumber(), backgroundHeight.GetNumber());
}

void ItemCardFixer::fixTest (float x, float y, float width, float height)
{
    return; // TODO: Use for scale9Grid fixing, also consider this code for making text fields for ingredient support
    RE::GFxValue args[4];
    RE::GFxValue rectangle;
    RE::GFxValue itemcard;
    RE::GFxValue grid;
    RE::GFxValue background;
    RE::GFxValue scale9Grid;

    // Magic numbers courtesy of Nem
    args[0].SetNumber(30.0f);
    args[1].SetNumber(79.0f);
    args[2].SetNumber(370.0f);
    args[3].SetNumber(44.0f);


    uiMovie->CreateObject(&rectangle, "flash.geom.Rectangle", args, 4);
    //logger::info("Rectangle type {}", rectangle.GetType());
    uiMovie->GetVariable(&itemcard, "_root.Menu_mc.itemCardFadeHolder.ItemCard_mc");
    //logger::info("itemCard type {}", itemcard.GetType());
    itemcard.GetMember("scale9Grid", &grid);
    //logger::info("grid type before {}", grid.GetType());
    itemcard.SetMember("scale9Grid", rectangle);
    itemcard.GetMember("scale9Grid", &grid);
    //logger::info("grid type after {}", grid.GetType());

    itemcard.GetMember("background", &background);
    background.GetMember("scale9Grid", &scale9Grid);
    //logger::info("scale9Grid type before {}", scale9Grid.GetType());
    uiMovie->Invoke("_root.Menu_mc.itemCardFadeHolder.ItemCard_mc.background.scale9Grid", nullptr, &rectangle, 4);
    background.GetMember("scale9Grid", &scale9Grid);
    //logger::info("scale9Grid type before manual set {}", scale9Grid.GetType());
    background.SetMember("scale9Grid", rectangle);
    background.GetMember("scale9Grid", &scale9Grid);
    //logger::info("scale9Grid type after {}", scale9Grid.GetType());
}

// --------------------------------------------------------- ВСЕ ЧТО ВЫШЕ МНЕ ВЕРОЯТНО НЕ НУЖНО ---------------------------------------------------


void ItemCardFixer::handleArmor()
{
    handleEffects();
}
void ItemCardFixer::handleWeapon()
{
    handleEffects();
}
void ItemCardFixer::handleMisc()
{
    // Fake MISC items as books
    itemInfo.SetMember("description", this->description);
    itemInfo.SetMember(typeVar, ItemCardType::ICT_BOOK);
}
void ItemCardFixer::handleBook()
{
    // Append DF's description to the books
    auto origDescStr = std::string("");
    RE::GFxValue desc;
    itemInfo.GetMember("description", &desc);
    if (desc.IsString() && std::strcmp(desc.GetString(), "undefined") && std::strcmp(desc.GetString(), "")) {
        origDescStr = std::string(desc.GetString());
        origDescStr.erase(remove(origDescStr.begin(), origDescStr.end(), ' '), origDescStr.end());
        if (!origDescStr.empty()) {
            origDescStr = std::string(desc.GetString()) + "\n";
        }
    }

    auto newDesc = origDescStr + this->description;
    itemInfo.SetMember("description", newDesc.c_str());
}
void ItemCardFixer::handleFood()
{
    handleEffects();
}
void ItemCardFixer::handlePotion()
{
    handleEffects();
}
void ItemCardFixer::handleSpell()
{
    handleEffects();
}
void ItemCardFixer::handleIngredient()
{
    /* unsupported*/
}
void ItemCardFixer::handleKey()
{
    // Fake KEY items as books
    itemInfo.SetMember("description", this->description);
    itemInfo.SetMember(typeVar, ItemCardType::ICT_BOOK);
}
void ItemCardFixer::handleShout()
{
    handleEffects();
}
void ItemCardFixer::handleActiveEffect()
{
    handleEffects();
}
void ItemCardFixer::handleSoulGems()
{
    handleSoulLVL();
}
void ItemCardFixer::handleSpellDefault()
{
    handleEffects();
}
void ItemCardFixer::handleList()
{
    /* unsupported*/
}
void ItemCardFixer::handleCraftEnchanting()
{
    handleEffects();
    handleSoulLVL();
}
void ItemCardFixer::handleHousePart()
{
    handleEffects();
}

void ItemCardFixer::fixArmor()
{
    fixHTML("ApparelEnchantedLabel", "effects");
    fixBackground("ApparelEnchantedLabel");
}
void ItemCardFixer::fixWeapon()
{
    // Remove weapon charge if there's no actual effects
    RE::GFxValue weaponChargeLabel;
    RE::GFxValue DFdescription;
    RE::GFxValue effects;
    itemCard.GetMember("WeaponChargeMeter", &weaponChargeLabel);
    itemInfo.GetMember(descriptionVar, &DFdescription);
    itemInfo.GetMember("effects", &effects);
    //logger::debug("types {} {} {}", weaponChargeLabel.GetType(), DFdescription.GetType(), effects.GetType());
    if (weaponChargeLabel.IsDisplayObject() && DFdescription.IsString() && effects.IsString()) {
        if (!strcmp(DFdescription.GetString(), effects.GetString())) {
            // effects is same as description, turn off weapon charge label
            //logger::debug("Turning off charge label");
            RE::GFxValue::DisplayInfo info;
            weaponChargeLabel.GetDisplayInfo(&info);
            info.SetVisible(false);
            weaponChargeLabel.SetDisplayInfo(info);
            //logger::debug("Weapon Charge Label disabled");
        }
    }
    fixHTML("WeaponEnchantedLabel", "effects");
    fixBackground("WeaponEnchantedLabel");
}
void ItemCardFixer::fixMisc()
{
    // MISC items are now books, fix the same way
    fixBook();
}
void ItemCardFixer::fixBook()
{
    // As a book, make it HTML
    fixHTML("BookDescriptionLabel", "description");
    fixBackground("BookDescriptionLabel");
}
void ItemCardFixer::fixFood()
{
    fixHTML("PotionsLabel", "effects");
    fixBackground("PotionsLabel");
}
void ItemCardFixer::fixPotion()
{
    fixHTML("PotionsLabel", "effects");
    fixBackground("PotionsLabel");
}
void ItemCardFixer::fixSpell()
{
    fixHTML("MagicEffectsLabel", "effects");
    //fixBackground("MagicEffectsLabel");
}
void ItemCardFixer::fixIngredient()
{
    /* unsupported */
}
void ItemCardFixer::fixKey()
{
    fixBook();
}
void ItemCardFixer::fixShout()
{
    // As a shout, use text auto shrink
    fixHTML("ShoutEffectsLabel", "effects");
    // TODO: Can't fix shouts since it will clobber over other elements
}
void ItemCardFixer::fixActiveEffect()
{
    fixSpell();
    fixBackground("MagicEffectsLabel");
}
void ItemCardFixer::fixSoulGems()
{
    // Soul gems are faked as books now, fix up as a book
    fixBook();
}
void ItemCardFixer::fixSpellDefault()
{
    fixSpell();
    fixBackground("MagicEffectsLabel");
}
void ItemCardFixer::fixList()
{
    /* unsupported */
}
void ItemCardFixer::fixCraftEnchanting()
{
    fixHTML("SoulLevel", "soulLVL");
    fixHTML("EnchantmentLabel", "effects");
    fixBackground("SoulLevel");
    fixBackground("EnchantmentLabel");
}
void ItemCardFixer::fixHousePart()
{
    fixSpell();
}

void ItemCardFixer::applyDescription()
{
    //logger::info("called applyDescription() for description - {} and item with type - {}", description, type);
    switch (type) {
    case ICT_ARMOR:         { handleArmor();        break; }
    case ICT_WEAPON:        { handleWeapon();       break; }
    case ICT_MISC:          { handleMisc();         break; }
    case ICT_BOOK:          { handleBook();         break; }
    case ICT_FOOD:          { handleFood();         break; }
    case ICT_POTION:        { handlePotion();       break; }
    case ICT_SPELL:         { handleSpell();        break; }
    case ICT_INGREDIENT:    { handleIngredient();   break; }
    case ICT_KEY:           { handleKey();          break; }
    case ICT_SHOUT:         { handleShout();        break; }
    case ICT_ACTIVE_EFFECT: { handleActiveEffect(); break; }
    case ICT_SOUL_GEMS:     { handleSoulGems();     break; }
    case ICT_SPELL_DEFAULT: { handleSpellDefault(); break; }
    case ICT_LIST:          { handleList();         break; }
    case ICT_CRAFT_ENCHANTING:  { handleCraftEnchanting(); break; }
    case ICT_HOUSE_PART:        { handleHousePart();       break; }
    }
}

void ItemCardFixer::fixItemCard()
{
    //logger::info("called fixItemCard() for item with type - {}", type);
    switch (type) {
    case ICT_ARMOR:      { fixArmor();   break; }
    case ICT_WEAPON:     { fixWeapon();  break; }
    case ICT_MISC:       { fixMisc();    break; }
    case ICT_BOOK:       { fixBook();    break; }
    case ICT_FOOD:       { fixFood();    break; }
    case ICT_POTION:     { fixPotion();  break; }
    case ICT_SPELL:      { fixSpell();   break; }
    case ICT_INGREDIENT: { fixIngredient();    break; }
    case ICT_KEY:        { fixKey();        break; }
    case ICT_SHOUT:      { fixShout();        break; }
    case ICT_ACTIVE_EFFECT:    { fixActiveEffect(); break; }
    case ICT_SOUL_GEMS:        { fixSoulGems();     break; }
    case ICT_SPELL_DEFAULT:    { fixSpellDefault(); break; }
    case ICT_LIST:             { fixList();         break;    }
    case ICT_CRAFT_ENCHANTING: { fixCraftEnchanting(); break;}
    case ICT_HOUSE_PART:       { fixHousePart();    break;    }
    }
}

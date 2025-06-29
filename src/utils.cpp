
#pragma warning (disable : 4189)

#include "utils.h"
#include "REL/Relocation.h"
//#include "core.h"
#include <windows.h>
#include <Xinput.h>

auto u_form_has_keyword (const RE::TESForm* form, const RE::BGSKeyword* keyword) -> bool
{
    if (!form || !keyword) {
        //SKSE::log::info("null form or kw");
        return false;
    }

    const auto keyword_form = form->As<RE::BGSKeywordForm>();

    if (!keyword_form) {
        return false;
    }

    return keyword_form->HasKeyword(keyword);
}

auto u_worn_has_keyword (RE::Actor* actor, RE::BGSKeyword* keyword) -> bool
{
    if (actor && keyword) {
        auto inv = actor->GetInventoryChanges();
        if (!inv) return false;

        using FuncT = bool (*)(RE::InventoryChanges*, RE::BGSKeyword*);
        const REL::Relocation<FuncT> func {RELOCATION_ID(15808, 0)};
        return func(inv, keyword);
    }
    return false;
}


auto u_get_weapon (const RE::Actor *actor, const bool is_left_hand) -> RE::TESObjectWEAP*
{
    if (!actor) return nullptr;
    const auto form = actor->GetEquippedObject(is_left_hand);
    if (!form) return nullptr;
    const auto weapon = form->As<RE::TESObjectWEAP>();
    if (!weapon) return nullptr;
    return weapon;
}

auto u_get_eff_second_av_name (const RE::ActiveEffect& active_effect) -> RE::ActorValue
{
    if (!active_effect.effect || !active_effect.effect->baseEffect) {
        return RE::ActorValue::kNone;
    }

    if (!active_effect.effect->baseEffect->HasArchetype(RE::EffectSetting::Archetype::kDualValueModifier)) {
        return RE::ActorValue::kNone;
    }

    return active_effect.effect->baseEffect->data.secondaryAV;
}

auto u_get_second_AV_weight(const RE::ValueModifierEffect& active_effect) -> float
{
    if (!active_effect.effect || !active_effect.effect->baseEffect) {
        return 0.f;
    }
    return active_effect.effect->baseEffect->data.secondAVWeight;
}


auto u_actor_has_active_mgef_with_keyword (RE::Actor *actor, const RE::BGSKeyword *keyword) -> bool 
{
    LOG ("called u_actor_has_active_mgef_with_keyword()");
    auto active_effects = actor->GetActiveEffectList();      // list
    if (!active_effects) {
        return false;
    }
    for (const auto ef : *active_effects)
    {
        if (!ef || ef->flags.any(RE::ActiveEffect::Flag::kInactive) || !ef->effect || !ef->effect->baseEffect) {
            continue;
        }
        if (ef->effect->baseEffect->HasKeyword(keyword)) {
            return true;
        }
    }
    return false;
}

auto u_same_activeEffects_count (RE::Actor *actor, const RE::EffectSetting *base_eff) -> uint32_t
{
    auto active_effects = actor->GetActiveEffectList();   // list
    if (!active_effects) return 0;

    uint32_t count = 0;
    for (const auto ef : *active_effects)
    {
        //SKSE::log::info("name - {}", ef->effect->baseEffect->GetName());
        if (!ef || ef->flags.any(RE::ActiveEffect::Flag::kInactive) || !ef->effect || !ef->effect->baseEffect) {
            continue;
        }
        if (ef->effect->baseEffect->formID == base_eff->formID) {
            count ++;
        }
    }
    return count;
}

void u_dispel_all_except (RE::BGSKeyword *excl_kw, RE::Actor *actor)  // now works via hook
{
    LOG("called u_dispel_all_except()");
    auto active_effects = actor->GetActiveEffectList();   // list
    if (!active_effects) return;

    for (const auto ef : *active_effects)
    {
        if (ef && ef->effect && ef->effect->baseEffect) {
            auto base  = ef->effect->baseEffect;
            //auto spell = ef->spell;
            //RE::DebugNotification(spell->GetName());
            if (base->data.castingType == RE::MagicSystem::CastingType::kFireAndForget) {
                if (!base->HasKeyword(excl_kw)) ef->Dispel(true);
            }
        }
    }
}

auto u_cast (RE::SpellItem *spell, RE::Actor *target, RE::Actor *caster) -> void
{
    auto& caster_ = false ? spell->HasKeyword(nullptr) ? target : caster : caster;
    auto& target_ = false ? spell->HasKeyword(nullptr) ? caster : target : target;
    if (spell->data.delivery == RE::MagicSystem::Delivery::kSelf) {
        caster->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)->CastSpellImmediate(spell, true, caster_, 1.f, false, 0.f, caster_);
    } else {
        caster->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)->CastSpellImmediate(spell, true, target_, 1.f, false, 0.f, caster_);
    }
}

void u_cast_on_self (RE::SpellItem* spell, RE::Actor* actor)
{
    if (!spell || !actor) return;
    actor->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)->CastSpellImmediate(spell, false, nullptr, 1.0f, false, 0.0f, actor);
}

/*
void u_equip_spell (RE::Actor* actor, RE::SpellItem* spell, SkyrimEquipSlot slot)
{
    if (!actor || !spell) return;
    auto eq = RE::ActorEquipManager::GetSingleton();
    if      (slot == SkyrimEquipSlot::kRight) eq->EquipSpell (actor, spell, my::slotRightHand.p);
    else if (slot == SkyrimEquipSlot::kLeft)  eq->EquipSpell (actor, spell, my::slotLeftHand.p);
    else if (slot == SkyrimEquipSlot::kPower) eq->EquipSpell (actor, spell);
}

void u_equip_item (RE::Actor* actor, RE::TESBoundObject* obj, std::optional<SkyrimEquipSlot> slot)   // additem (if need) and equip
{
    if (!actor || !obj) return;
    if (!actor->GetInventory().contains(obj)) actor->AddObjectToContainer(obj, nullptr, 1, nullptr);
    auto eq = RE::ActorEquipManager::GetSingleton();
    if      (slot == SkyrimEquipSlot::kRight)  eq->EquipObject (actor, obj, nullptr, 1, my::slotRightHand.p);
    else if (slot == SkyrimEquipSlot::kLeft)   eq->EquipObject (actor, obj, nullptr, 1, my::slotLeftHand.p);
    else if (slot == SkyrimEquipSlot::kShield) eq->EquipObject (actor, obj, nullptr, 1, my::slotShield.p);
    else if (slot == SkyrimEquipSlot::kPotion) eq->EquipObject (actor, obj, nullptr, 1, my::slotPotion.p);
    else eq->EquipObject (actor, obj);  // default equip
}
*/

RE::SpellItem* u_get_equipped_spell_by_slot (RE::Actor* actor, RE::MagicSystem::CastingSource source) 
{
    using Source = RE::MagicSystem::CastingSource;
    uint32_t slot = static_cast<uint32_t>(source);    // 0 - left,  1 - right, 2 - other (ability/shout) (in our case only ability), 3 - instant (not used here, is used for castSpellImmediate)
    
    if (source == Source::kLeftHand || source == Source::kRightHand) {
        auto mgItem = actor->selectedSpells[slot];               // selectedSpells[] contain only l/r hand spells
        if (mgItem && mgItem->formType == RE::FormType::Spell) return mgItem->As<RE::SpellItem>();
        else return 0;
    }
    else if (auto powerForm = actor->selectedPower) {
        if (powerForm->formType != RE::FormType::Shout) return powerForm->As<RE::SpellItem>();  //  ability from actor->selectedPower
    }  
    return 0;
}

RE::TESShout* u_get_equipped_shout (RE::Actor* actor)
{
    if (auto powerForm = actor->selectedPower) {
        if (powerForm->formType == RE::FormType::Shout) return powerForm->As<RE::TESShout>();
    }
    return nullptr;
} 

RE::MagicItem* u_get_current_casting_spell (RE::Actor* actor, uint32_t mcasterSlot)  //  0 - left  1 - right, 2 - shout/iability , 3 - ? (mb instant)
{                                           
    RE::ActorMagicCaster *actorsMagicSlot = actor->magicCasters[mcasterSlot];   // magicCaster is used only while is_casting, for example ritual casting or shout hold
    if (actorsMagicSlot && actorsMagicSlot->currentSpell)
        return actorsMagicSlot->currentSpell; // current casting magicItem (can be spell or shout)
    else return nullptr;
} 

auto u_is_power_attacking (RE::Actor* actor) -> bool
{
    const auto current_process = actor->currentProcess;
    if (!current_process) return false;

    const auto high_process = current_process->high;
    if (!high_process) return false;

    const auto& attack_data = high_process->attackData;
    if (!attack_data) return false;

    const auto flags = attack_data->data.flags;
    return flags.any(RE::AttackData::AttackFlag::kPowerAttack);
}

auto u_place_at_me (RE::TESObjectREFR* target, RE::TESForm* form, std::uint32_t count, bool force_persist, bool initially_disabled) -> RE::TESObjectREFR* 
{
    using FuncT = RE::TESObjectREFR*(RE::BSScript::Internal::VirtualMachine*, RE::VMStackID, RE::TESObjectREFR*,
                                     RE::TESForm*, std::uint32_t, bool, bool);

    RE::VMStackID frame = 0;
    const REL::Relocation<FuncT> func{RELOCATION_ID (55672, 56203)};
    auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
    return func(vm, frame, target, form, count, force_persist, initially_disabled);
}

void u_set_rotation_x (RE::TESObjectREFR *refr, float angle)
{
    REL::Relocation<decltype(u_set_rotation_x)> func (REL::ID(19360));
    func (refr, angle);
}

void u_set_rotation_y (RE::TESObjectREFR *refr, float angle)
{
    REL::Relocation<decltype(u_set_rotation_y)> func (REL::ID(19361));
    func (refr, angle);
}

void u_set_rotation_z (RE::TESObjectREFR *refr, float angle)
{
    REL::Relocation<decltype(u_set_rotation_z)> func (REL::ID(19362));
    func (refr, angle);
}

void u_jump (RE::Actor* actor)   // makes an actor jump if he can (check state)
{
    REL::Relocation<decltype(u_jump)> func(REL::ID(36271));
    func(actor);
}


auto u_get_effects_by_keyword (RE::Actor *actor, const RE::BGSKeyword *keyword) -> std::vector<RE::ActiveEffect*>
{
    auto active_effects = actor->GetActiveEffectList();
    std::vector<RE::ActiveEffect*> effects = {};
    if (!active_effects) {
        return effects;
    }

    for (auto active_effect : *active_effects) {
        if (!active_effect || active_effect->flags.any(RE::ActiveEffect::Flag::kInactive) || 
            !active_effect->effect || !active_effect->effect->baseEffect) {
            continue;
        }

        const auto effect = active_effect->effect;

        if (const auto base_effect = effect->baseEffect; !base_effect->HasKeywordID(keyword->formID)) {
            continue;
        }

        effects.push_back(active_effect);
    }
    return effects;        //  vector of  RE::ActiveEffect*
}


auto u_dispel_effect_from_actor (RE::ActiveEffect* effect, RE::Actor* actor) -> bool
{
    if (!effect || !actor) {
        return false;
    }
    RE::ActorHandle& actor_handle = effect->caster;
    RE::MagicItem* spell = effect->spell;

    if (!spell || !actor_handle) {
        return false;
    }
    return actor->DispelEffect(spell, actor_handle, effect);
}

auto u_enchant_equipped_weapon (RE::InventoryChanges* changes, RE::TESBoundObject* boundOb, RE::ExtraDataList* extraList, RE::EnchantmentItem* ench, int16_t amount) -> RE::ExtraDataList*
{
    RE::Actor* player = RE::PlayerCharacter::GetSingleton();
    if (!u_get_weapon(player, 0))  return nullptr;

    REL::Relocation<decltype(u_enchant_equipped_weapon)> func (REL::ID(15906));

    return func(changes, boundOb, extraList, ench, amount);
}

RE::EnchantmentItem* u_get_actors_weap_extra_ench (RE::Actor* actor, bool left)    // checks only ExtraEnch, i.e in-game added custom ench
{
    auto inv = actor->GetInventory ([](RE::TESBoundObject& obj) { if(obj.IsWeapon()) return true; else return false;} );     // lambda (boolean function-filter)  
    //   inv - map of inventory items {item, data}
    bool checkleft = false;
    for (auto& [item, data] : inv) {         // range-based cycle for map
        const auto& [count, entry] = data;
        if (entry->extraLists) {
            for (const auto& xList : *entry->extraLists) {
                const auto xEnchLeft = xList->GetByType<RE::ExtraWornLeft>();
                const auto xEnchRight = xList->GetByType<RE::ExtraWorn>();
                if ((xEnchLeft || xEnchRight) && xList->GetByType<RE::ExtraEnchantment>()) {
                    if      (xEnchLeft)  checkleft = true;
                    else if (xEnchRight) checkleft = false;
                    if (checkleft == left) return xList->GetByType<RE::ExtraEnchantment>()->enchantment;
                }
            }
        }
    }
    return nullptr;
}


void u_remove_weap_ench (RE::InventoryEntryData* entry)
{
    if (entry->extraLists) {
        for (const auto& xList : *entry->extraLists) {
            const auto xEnchLeft = xList->GetByType<RE::ExtraWornLeft>();
            const auto xEnchRight = xList->GetByType<RE::ExtraWorn>();
            if ((xEnchLeft || xEnchRight) && xList->GetByType<RE::ExtraEnchantment>()) {
                xList->RemoveByType(RE::ExtraDataType::kEnchantment);
            }
        }
    }
}

RE::ExtraPoison* u_get_pc_poison (bool is_left_hand)
{
    LOG("called u_get_pc_poison()");
    if (auto weapEntryData = RE::PlayerCharacter::GetSingleton()->GetEquippedEntryData(is_left_hand)) {
        if (weapEntryData->IsPoisoned() && weapEntryData->extraLists) {
            if (auto extraData = weapEntryData->extraLists->front()) {
                if (auto poisonData = extraData->GetByType<RE::ExtraPoison>()) {
                    return poisonData;
                }
            }
        }
    }
    return nullptr;
}

void u_remove_pc_poison (bool is_left_hand)
{
    LOG("called u_remove_pc_poison()");
    if (auto weapEntryData = RE::PlayerCharacter::GetSingleton()->GetEquippedEntryData(is_left_hand)) {
        if (weapEntryData->IsPoisoned() && weapEntryData->extraLists) {
            if (auto extraData = weapEntryData->extraLists->front()) {
                if (auto poisonData = extraData->GetByType<RE::ExtraPoison>()) {
                    extraData->Remove(RE::ExtraPoison::EXTRADATATYPE, poisonData);
                }
            }
        }
    }
}

void u_kill_projectile(RE::Projectile* proj)
{
    REL::Relocation<decltype(u_kill_projectile)> func (REL::ID(42930));
    func(proj);
}

bool u_setStage (RE::TESQuest* quest, uint16_t stage)
{
    if (!quest) return 0;
    REL::Relocation<decltype(u_setStage)> func (REL::ID(24482));
    return func (quest, stage);
}

void u_updQuestTextGlob (RE::TESQuest* quest, RE::TESGlobal* glob)
{
    if (!quest) return;
    REL::Relocation<decltype(u_updQuestTextGlob)> func (REL::ID(24509));
    func(quest, glob);
}

void u_addItem (RE::Actor* a, RE::TESBoundObject* item, RE::ExtraDataList* extraList, int count, RE::TESObjectREFR* fromRefr)
{
    REL::Relocation<decltype(u_addItem)> func (REL::ID(36525));
    func(a, item, extraList, count, fromRefr);
}

int32_t u_get_item_count (RE::Actor *actor, uint32_t formid_)
{
    const auto inv = actor->GetInventory();

    for (const auto& [item, invData] : inv) {
         const auto& [count, entry] = invData;
         if (item->formID == formid_) return count;
    }
    return 0;
}

void u_remove_all_items (RE::Actor *actor)
{
    auto inventory = actor->GetInventory();
    for (const auto &[item, data] : inventory) {
        const auto &[count, entry] = data;
        if (entry) { // && entry->IsWorn()) {
           actor->RemoveItem(item, count, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
        }
    }
    return;
}

int32_t u_papyrusGetItemCount (RE::TESObjectREFR* cont, RE::TESForm* item) // can check references (if the object ref is persistent, it exists in a container as a reference)
{
    std::int32_t result = 0;
    if (cont)
         if (auto boundObject = cont->GetBaseObject(); boundObject) // check container ref's base obj, is it valid TESBoundObject
            if (auto type = boundObject->formType; type == RE::FormType::Container || type == RE::FormType::NPC) {  //  only these forms are valid
                bool isBound = skyrim_cast<RE::TESBoundObject*>(item) != nullptr;
                bool isReference = skyrim_cast<RE::TESObjectREFR*>(item) != nullptr; // now check item, cast TESForm to -> TESBoundObject and TESObjectREFR
                if (isBound || isReference) {
                    using func_t = std::int32_t (*)(RE::TESObjectREFR*, RE::TESForm*, std::int64_t, std::int32_t);
                    REL::Relocation<func_t> func {REL::ID(56062)};
                    result = func (cont, item, NULL, NULL);
                }
            }
    return result;
}

bool u_is_in_city (RE::Actor* actor)
{
    auto cityKW = RE::TESForm::LookupByID<RE::BGSKeyword>(0x13168);        // big cities
    if (actor->GetCurrentLocation() && actor->GetCurrentLocation()->HasKeyword(cityKW)) return true;
    else return false;
}

bool u_center_on_cell (RE::PlayerCharacter* player, const char editorID, RE::TESObjectCELL *cell)   // перемещает игрока в центр ячейки, в editorId передавать 0 
{
    REL::Relocation<decltype(u_center_on_cell)> func (REL::ID(39365));   
    return func (player, editorID, cell);
}

bool get_miscStat (RE::BSFixedString& statName, int& o_value)
{
    using func_t = decltype (&get_miscStat);
    REL::Relocation<func_t> func {REL::ID(16120)};  // 1405E1510
    return func (statName, o_value);
}

int u_get_game_statistic (std::string statName) 
{
    int val = 0;
    RE::BSFixedString f_statName = statName;
    get_miscStat(f_statName, val);
    return val;
}


RE::GFxMovie* getConsoleMovie() {
    const auto UI = RE::UI::GetSingleton();
    const auto consoleMenu = UI ? UI->GetMenu<RE::Console>() : nullptr;
    return consoleMenu ? consoleMenu->uiMovie.get() : nullptr;
}

std::string u_get_entered_console_commands()
{
    std::string result = "Console - ";
    if (const auto consoleMovie = getConsoleMovie()) 
    {
        RE::GFxValue commandsVal;
        consoleMovie->GetVariable (&commandsVal, "_global.Console.ConsoleInstance.Commands");  // stores this session console entries
       
        if (commandsVal.IsArray()) {
            const auto size = commandsVal.GetArraySize();
            if   (size == 0) result += "N/A; ";
            else
            {
                for (uint32_t i = 0; i < size; i++) {
                    RE::GFxValue val;
                    commandsVal.GetElement(i, &val);
                    if (val.IsString()) {
                        result += std::string(val.GetString()) + "; ";
                    }
                }  
            }
        }
    }
    return result;
}


float u_get_worn_equip_weight (RE::Actor* actor)
{ 
    if (!actor) return 0;
    float eq_weight = 0;

    auto armor = actor->GetWornArmor(RE::BGSBipedObjectForm::BipedObjectSlot::kBody);
    auto arms = actor->GetWornArmor(RE::BGSBipedObjectForm::BipedObjectSlot::kHands);
    auto boots = actor->GetWornArmor(RE::BGSBipedObjectForm::BipedObjectSlot::kFeet);
    auto helm = actor->GetWornArmor(RE::BGSBipedObjectForm::BipedObjectSlot::kHair);
    auto shield = actor->GetWornArmor(RE::BGSBipedObjectForm::BipedObjectSlot::kShield);
    auto weapon_l = u_get_weapon(actor, true);
    auto weapon_r = u_get_weapon(actor, false);

    if (armor) eq_weight += armor->weight;
    if (arms) eq_weight += arms->weight;
    if (boots) eq_weight += boots->weight;
    if (helm) eq_weight += helm->weight;
    if (shield) eq_weight += shield->weight;

    if (weapon_r) {
        if (weapon_r->GetWeaponType() >= RE::WEAPON_TYPE(5)) {   // two handed
            eq_weight += weapon_r->weight;
            return eq_weight;
        }
        else eq_weight += weapon_r->weight;
    }

    if (weapon_l) eq_weight += weapon_l->weight;  

    return eq_weight;
}

RE::TESObjectREFR::InventoryItemMap u_get_all_worn_objects (RE::Actor* actor)  // returns all worn objects
{
    LOG("called u_get_all_worn_objects()");
    auto inventory = actor->GetInventory();
    RE::TESObjectREFR::InventoryItemMap filtered;
    for (const auto &[item, data] : inventory) {
        const auto &[count, entry] = data;
        if (entry && entry->IsWorn()) {
            std::unique_ptr <RE::InventoryEntryData> newEntry = std::make_unique<RE::InventoryEntryData>(*entry);  // исп. make_unique для создания unique_ptr на копию entry
            std::pair <std::int32_t, std::unique_ptr<RE::InventoryEntryData>> newData (count, std::move(newEntry));  // создаем новую пару
            filtered.insert (std::make_pair(item, std::move(newData)));  // вставляем новую пару в filtered
        }
    }
    return filtered;
}

bool u_is_equipped (RE::Actor* actor, RE::TESBoundObject* obj)
{
    LOG("called u_is_equipped()");
    for (const auto &[item, data] : actor->GetInventory()) {
        const auto &[count, entry] = data;
        if (obj->formID == item->formID && entry && entry->IsWorn()) {
            return true;
        }
    }
    return false;
}

void u_unequip_all_items (RE::Actor* actor)
{
    LOG("called u_unequip_all()");
    auto inventory = actor->GetInventory();
    RE::TESObjectREFR::InventoryItemMap filtered;
    for (const auto &[item, data] : inventory) {
        const auto &[count, entry] = data;
        if (entry && entry->IsWorn()) {
            RE::ActorEquipManager::GetSingleton()->UnequipObject(actor, item);
        }
    }
}

std::vector<RE::TESBoundObject*> u_get_inv_items_by_keyword (RE::TESObjectREFR* ref, const std::string &keywordEditorID)
{
    LOG("called u_get_inv_items_by_keyword()");
    auto inventory = ref->GetInventory();
    std::vector<RE::TESBoundObject*> filtered;
    std::vector<std::string> kw = {keywordEditorID};
    for (const auto &[item, data] : inventory) {
        if(item->HasAnyKeywordByEditorID(kw))
            filtered.push_back (item);
    }
    return filtered;
}


void u_SendInventoryUpdateMessage (RE::TESObjectREFR* a_inventoryRef, const RE::TESBoundObject* a_updateObj)
{
    using func_t = decltype(&u_SendInventoryUpdateMessage);
    static REL::Relocation<func_t> func {RELOCATION_ID(51911, 52849)};
    return func (a_inventoryRef, a_updateObj);
}

void u_update_speedMult (RE::Actor* actor)
{
    if (!actor) return;
    actor->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kCarryWeight, 0.01f);
    actor->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kCarryWeight, -0.01f);
}

void u_damage_av (RE::Actor* actor, RE::ActorValue av, float magn)
{
    if (!actor) return;
    actor->RestoreActorValue (RE::ACTOR_VALUE_MODIFIER::kDamage, av, -magn);
}


RE::ActorValue u_get_secondary_resist_name (const RE::MagicItem *magic_item)    //  second resist is usually mag res, except poisons
{
    if (magic_item->IsPoison()) {
        return RE::ActorValue::kPoisonResist;
    }
    return RE::ActorValue::kResistMagic;
}

 float u_get_actor_value_max (RE::Actor* actor, const RE::ActorValue av)
 {
    if (!actor) return 0.f;
    return actor->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kTemporary, av) + actor->GetActorValueMax(av);
 }


float u_get_av_percent (RE::Actor* actor, RE::ActorValue av) 
{
    if (!actor)  return 0.f;

    float current = actor->GetActorValue(av);
    float max = u_get_actor_value_max (actor, av);
    if (max <= 0.f)  return 0.f; // divide 0 protect

    float percent = current / max;
    return std::clamp (percent, 0.0f, 1.0f);
}


 inline int soundHelper_a(void* manager, RE::BSSoundHandle* a2, int a3, int a4)    // sub_140BEEE70
 {
    using func_t = decltype(&soundHelper_a);
    REL::Relocation<func_t> func{RELOCATION_ID(66401, 67663)};
    return func(manager, a2, a3, a4);
 }

 inline void soundHelper_b(RE::BSSoundHandle* a1, RE::NiAVObject* source_node)    // sub_140BEDB10
 {
    using func_t = decltype(&soundHelper_b);
    REL::Relocation<func_t> func{RELOCATION_ID(66375, 67636)};
    return func(a1, source_node);
 }

 inline char __fastcall soundHelper_c(RE::BSSoundHandle* a1)                       // sub_140BED530
 {
    using func_t = decltype(&soundHelper_c);
    REL::Relocation<func_t> func{RELOCATION_ID(66355, 67616)};
    return func(a1);
 }

 inline char set_sound_position (RE::BSSoundHandle* a1, float x, float y, float z) {
    using func_t = decltype(&set_sound_position);
    REL::Relocation<func_t> func {RELOCATION_ID(66370, 67631)};
    return func(a1, x, y, z);
 }

 void u_playSound(RE::Actor* a, RE::BGSSoundDescriptorForm* a_descriptor, float a_volumeOverride)
 {
    RE::BSSoundHandle handle;
    handle.soundID = static_cast<uint32_t>(-1);
    handle.assumeSuccess = false;
    *(uint32_t*)&handle.state = 0;

    soundHelper_a (RE::BSAudioManager::GetSingleton(), &handle, a_descriptor->GetFormID(), 16);

    if (set_sound_position(&handle, a->data.location.x, a->data.location.y, a->data.location.z)) {
        handle.SetVolume(a_volumeOverride);
        soundHelper_b (&handle, a->Get3D());
        soundHelper_c (&handle);              // play
    }
 }

 std::string  u_int2hex (int decimal)
 {
    std::stringstream stream;
    stream << std::hex << decimal;
    return stream.str();
 }

 std::string u_trimmed_str (float number)  // returns string(float) with 2 symbols afer .
 {
     std::ostringstream oss;
     oss << std::fixed << std::setprecision(2) << number;
     return oss.str();
 }

 float u_req_inc_damage ()  // requiem damage taken like x1.5
 {
    return RE::GameSettingCollection::GetSingleton()->GetSetting("fDiffMultHPToPCL")->GetFloat(); 
 }

 float u_req_out_damage ()
 {
    return RE::GameSettingCollection::GetSingleton()->GetSetting("fDiffMultHPByPCL")->GetFloat();
 }

void u_exit_game ()
{
    RE::Main::GetSingleton()->quitGame = true;
}

void u_set_god_mode (bool set)      // tgm on/off
{
    REL::Relocation<decltype(u_set_god_mode)> func (REL::ID(39424));
    func(set);
}

 std::vector<RE::Actor*> u_get_player_followers() 
 {
    std::vector<RE::Actor*> followers;
    
    if (const auto processLists = RE::ProcessLists::GetSingleton())
    {
      for (auto& actorHandle : processLists->highActorHandles) {
        if (auto actor = actorHandle.get(); actor && actor->IsPlayerTeammate()) {
           followers.push_back(actor.get());
        }
      }
      for (auto& actorHandle : processLists->middleHighActorHandles) {
        if (auto actor = actorHandle.get(); actor && actor->IsPlayerTeammate()) {
           followers.push_back(actor.get());
        }
      }
      for (auto& actorHandle : processLists->middleLowActorHandles) {
        if (auto actor = actorHandle.get(); actor && actor->IsPlayerTeammate()) {
           followers.push_back(actor.get());
        }
      }
      for (auto& actorHandle : processLists->lowActorHandles) {
        if (auto actor = actorHandle.get(); actor && actor->IsPlayerTeammate()) {
           followers.push_back(actor.get());
        }
      }
    }
    return followers;
 }

RE::BSTArray<RE::CommandedActorData>* u_get_commanded_actors (RE::Actor* actor)
{
    if (auto aiprocess = actor->currentProcess) {
        if (auto mid_high = aiprocess->middleHigh) {
            if (!mid_high->commandedActors.empty()) {
                return std::addressof(mid_high->commandedActors);  // commanded actors are in middleHigh group
            }
        }
    }
    return nullptr;
}

void u_move_all_followers_to_player (const bool followers, const bool summons)
{
    auto player = RE::PlayerCharacter::GetSingleton();
    
    if (followers) {
        auto teammates = u_get_player_followers();
        for (auto teammate : teammates) {
          if (teammate) {
            teammate->MoveTo(player);
          }
        }
    }
    if (summons) {
        if (auto array_summons = u_get_commanded_actors(player))
        {
           for (const auto& summon_data : *array_summons) {
              if (summon_data.commandedActor && summon_data.commandedActor.get() && summon_data.commandedActor.get().get()) {
                  summon_data.commandedActor.get()->MoveTo(player);
              }
           }
        } 
    }
}

bool u_mark_for_delete (void* a1, void* a2, RE::TESObjectREFR* refr)  //  includes virt. refr->SetDelete(),  call as (nullptr, nullptr, ref)
{
    using func_type = decltype(u_mark_for_delete);
    REL::Relocation<func_type> func (REL::ID(22027));
    return func (a1, a2, refr);
}


void u_clone_npc()  //  temp // temporary // test // use if works         // creates new actor / object   // simple ver
{
    auto boundObj = RE::TESForm::LookupByID<RE::TESBoundObject>(0x13475);  // ?
    auto emptyRef = RE::ObjectRefHandle();
    // auto newRefHandle = RE::TESDataHandler::GetSingleton()->CreateReferenceAtLocation(boundObj, position, rotation,
    // cell, world, nullptr, nullptr, emptyRef, false, true); RE::TESObjectREFR* newRef = newRefHandle.get().get();
}

void u_toggle_controls (bool status_)
{
    auto control = RE::ControlMap::GetSingleton();
    if (!control) return;
    control->ToggleControls(RE::ControlMap::UEFlag::kAll, status_);
}

auto u_disable_controls_selective() -> RE::stl::enumeration<RE::UserEvents::USER_EVENT_FLAG, std::uint32_t>
{
    auto controls = RE::ControlMap::GetSingleton();
    auto wasEnabledControls = controls->enabledControls;  // controls, enabled before this func
    using Con = RE::ControlMap::UEFlag;
    controls->ToggleControls(Con::kFighting, false);
    controls->ToggleControls(Con::kJumping, false);       // disable some controls
    controls->ToggleControls(Con::kMenu, false);
    controls->ToggleControls(Con::kMovement, false);
    controls->ToggleControls(Con::kMainFour, false);
    return wasEnabledControls;                            // return what was enabled

    // RE::ControlMap::GetSingleton()->enabledControls = result_of_this_function;     // restore controls
}


bool u_findClosestReferenceOfTypeInRadius (RE::TES* tes, const RE::TESBoundObject &obj, const RE::NiPoint3 &centerOfSearch, float afRadius, RE::NiPointer<RE::TESObjectREFR> &outRef)
{
    REL::Relocation<decltype(u_findClosestReferenceOfTypeInRadius)> func (REL::ID(13269));
    return func (tes, obj, centerOfSearch, afRadius, outRef);

    // ---- как вызывать ----:
    //if (RE::AlchemyItem* mead = RE::TESForm::LookupByID<RE::AlchemyItem>(0x34C5D)){
    //    if (auto meadObj = mead->As<RE::TESBoundObject>()) {
    //        RE::TESObjectREFRPtr outRefPtr;
    //        RE::NiPoint3 playerPos = mys::player->GetPosition();
    //        FindClosestReferenceOfTypeInRadius(RE::TES::GetSingleton(), *meadObj, playerPos, 500.f, outRefPtr); // radius in unit
    //        if(outRefPtr.get()) RE::DebugNotification(outRefPtr.get()->GetName());
    //    }
    //} 
}

uint32_t u_get_open_state (RE::TESObjectREFR* refr)     // get door or container open state  1: Open, 2: Opening, 3: Closed, 4: Closing, 0: None (object can't be opened or closed)
{
    if (!refr) return 0;
    REL::Relocation<decltype(u_get_open_state)> func (RELOCATION_ID(14180, 14288).address());    // в момент активации закрытой двери будет 1, в момент активации открытой 3
    return func (refr);
}

bool u_is_food (RE::TESBoundObject* obj)     //
{
    RE::BGSKeyword *vendorItemFood = RE::TESForm::LookupByID<RE::BGSKeyword>(0x8CDEA);
    if (RE::AlchemyItem* food = obj->As<RE::AlchemyItem>()) {
        return food->HasKeyword(vendorItemFood);  // alchItem && hasKeyword(food)
    }
    return false;
}

void u_apply_imod (RE::TESImageSpaceModifier* imod, float power)
{
    RE::ImageSpaceModifierInstanceForm::Trigger (imod, power, RE::PlayerCharacter::GetSingleton()->Get3D()); // power 0 - 1.0               
}

void u_stop_imod (RE::TESImageSpaceModifier* imod)
{
    RE::ImageSpaceModifierInstanceForm::Stop (imod);
}

RE::TESRegion* u_get_current_weatherType_region ()  // получает текущий погодный регион ячейки игрока, работает и из интерьера. Список id всех условных регионов можно посмотреть в рп
{
    using func_type = decltype(u_get_current_weatherType_region);
    REL::Relocation<func_type> func (REL::ID(25713));
    return func();
}

void u_set_weather (RE::Sky *sky, RE::TESWeather *weather, bool force, bool accelerate)   // 1 par = RE::Sky::GetSingleton() , accelerate = soft transition
{
    using func_type = decltype(u_set_weather);
    REL::Relocation<func_type> func (REL::ID(25694));
    func (sky, weather, force, accelerate);
}

void u_show_message_box2 (std::string& bodyText, std::vector<std::string> buttonTextValues, std::function<void(unsigned int)> callback)
{
   LOG("called u_show_message_box2()");
   u_MessageBox::MyMessageBox::Show (bodyText, buttonTextValues, callback);    // from zero
}

void u_show_message_box (RE::BGSMessage* msgBox, std::function<void(unsigned int)> callback)
{
    LOG("called u_show_message_box()");
    u_MessageBox::MyMessageBox::Show (msgBox, callback);    // from ready messageBox object
}

float u_get_distance (RE::TESObjectREFR* a, RE::TESObjectREFR* b) 
{
    return a->GetPosition().GetDistance(b->GetPosition());  // also there is  .GetSquaredDistance()
}


void craftingMenusTest()       //  later learn how to deal with craft menus
{
    auto ui = RE::UI::GetSingleton();
    if (ui->IsMenuOpen("Crafting Menu"))
    {
        auto Menu = ui->GetMenu<RE::CraftingMenu>().get();
        auto menu1 = skyrim_cast<RE::CraftingSubMenus::ConstructibleObjectMenu*>(Menu->subMenu);   //  кузница, создание предметов
        auto menu2 = skyrim_cast<RE::CraftingSubMenus::EnchantConstructMenu*>(Menu->subMenu);      //  зачарование
        auto menu3 = skyrim_cast<RE::CraftingSubMenus::SmithingMenu*>(Menu->subMenu);              //  улучшение предметов, проточка

        if (menu3 && menu3->furniture && menu3->furniture->HasKeywordString("CraftingSmithingSharpeningWheel")) {    // доп проверка что это именно меню проточки (с)
            menu3->unk100[menu3->unk150].constructibleObject->createdItem;    // доступ к createdItem 
        }
        
    }
}


 void u_log_actor_perk_entries (RE::Actor* actor, RE::BGSPerkEntry::EntryPoint theEntry, std::string entryNameForLog)
 {

    using PerkEntryFunction = RE::BGSEntryPointPerkEntry::EntryData::Function;
    int numPerks = actor->GetActorBase()->perkCount;

    for (int i = 0; i < numPerks; i++)
    {    
         RE::BGSPerk* perk = actor->GetActorBase()->perks[i].perk;
         if (!perk) continue;
         for (auto &entry : perk->perkEntries) 
         {        
            if (entry->GetType() != RE::PERK_ENTRY_TYPE::kEntryPoint) continue;                                        //   тут отсеиваем архетипы  ability и quest
            
            // тут приводим ентри общего архетипа BGSPerkEntry к типу конкретно BGSEntryPointPerkEntry (не ability и quest)
            RE::BGSEntryPointPerkEntry *entry_point = skyrim_cast<RE::BGSEntryPointPerkEntry*>(entry);    
            if (entry_point->entryData.entryPoint != theEntry) continue;        //   тут отсеиваем все ентри кроме нужного, например  ModSpellMagnitude
            if (entry_point->entryData.function == PerkEntryFunction::kMultiplyValue)                        //   проверяем мат. функцию нашего PerkEntry
            {
                    // приводим поле ентри ->function_data типа EntryPointFunctionData к конкретному типу EntryPointFunctionDataOneValue (данные функции с 1 слотом)
                     RE::BGSEntryPointFunctionDataOneValue* function_data = skyrim_cast<RE::BGSEntryPointFunctionDataOneValue*>(entry_point->functionData);
                     if (!function_data) continue;
                     LOG("found {} in perk - {}, perkId - {}, func: multiply_value by - {}", entryNameForLog, perk->fullName, u_int2hex(perk->formID), function_data->data);
            }
            else if (entry_point->entryData.function == PerkEntryFunction::kAddValue)        
            {
                     RE::BGSEntryPointFunctionDataOneValue* function_data = skyrim_cast<RE::BGSEntryPointFunctionDataOneValue*>(entry_point->functionData);
                     if (!function_data) continue;
                     LOG("found {} in perk - {}, perkId - {}, func: add_value - {}", entryNameForLog, perk->fullName, perk->formID, function_data->data);
            }
            else if (entry_point->entryData.function == PerkEntryFunction::kMultiplyActorValueMult)            //  тут в функции уже 2 слота, AV и мульт, значит приводим к типу для 2 слотов
            {
                     BGSEntryPointFunctionDataTwoValue* function_data = skyrim_cast<BGSEntryPointFunctionDataTwoValue*>(entry_point->functionData);
                     if (!function_data) continue;
                     LOG("found {} in perk - {}, perkId - {}, func: multiply AV*mult, mult is - {}", entryNameForLog, perk->fullName, u_int2hex(perk->formID), function_data->data2);
            }
            else if (entry_point->entryData.function == PerkEntryFunction::kMultiply1PlusActorValueMult) {
                     BGSEntryPointFunctionDataTwoValue* function_data = skyrim_cast<BGSEntryPointFunctionDataTwoValue*>(entry_point->functionData);
                     if (!function_data) continue;
                     LOG("found {} in perk - {}, perkId - {}, func: multiply 1 + AV*mult, mult is - {}", entryNameForLog, perk->fullName, u_int2hex(perk->formID), function_data->data2);
            } 
         }
    }
    SKSE::log::info("\n----------------------------------------------------------------------------------\n");
 }
 

namespace Utils_anim_namespace
{
    auto try_find_animation (const std::string &key) -> AnimationEvent
    {
        const auto it = animation_map.find(key);
        if (it == animation_map.end())
                return AnimationEvent::kNone;
       
        return it->second;
    }
} 


 //----------------------------- fenix template class _generic_foo_ , Calls native function by the signature and id (с), check later -----------------------


// template <int id, typename x_Function>
// class _generic_foo_;
//
// template <int id, typename x_Result, typename... x_Args>
// class _generic_foo_<id, x_Result(x_Args...)> {
//public:
//    static x_Result eval(x_Args... args) {
//        using func_t = x_Result(x_Args...);
//        REL::Relocation<func_t> func{REL::ID(id)};
//        return func(std::forward<x_Args>(args)...);
//    }
// };
//
// void play_sound(RE::TESObjectREFR* a, int formid) {
//    RE::BSSoundHandle handle;
//    handle.soundID = static_cast<uint32_t>(-1);
//    handle.assumeSuccess = false;
//    *(uint32_t*)&handle.state = 0;
//
//    auto manager = _generic_foo_<66391, void*()>::eval();
//    _generic_foo_<66401, int(void*, RE::BSSoundHandle*, int, int)>::eval(manager, &handle, formid, 16);
//    if (_generic_foo_<66370, bool(RE::BSSoundHandle*, float, float, float)>::eval(
//            &handle, a->data.location.x, a->data.location.y, a->data.location.z)) {
//        _generic_foo_<66375, void(RE::BSSoundHandle*, RE::NiAVObject*)>::eval(&handle, a->Get3D());
//        _generic_foo_<66355, bool(RE::BSSoundHandle*)>::eval(&handle);
//    }
// }


using namespace RE;

uint32_t Utils::GamepadMaskToKeycode(uint32_t keyMask)
{
    switch (keyMask) {
         case XINPUT_GAMEPAD_DPAD_UP:
            return kGamepadButtonOffset_DPAD_UP;
         case XINPUT_GAMEPAD_DPAD_DOWN:
            return kGamepadButtonOffset_DPAD_DOWN;
         case XINPUT_GAMEPAD_DPAD_LEFT:
            return kGamepadButtonOffset_DPAD_LEFT;
         case XINPUT_GAMEPAD_DPAD_RIGHT:
            return kGamepadButtonOffset_DPAD_RIGHT;
         case XINPUT_GAMEPAD_START:
            return kGamepadButtonOffset_START;
         case XINPUT_GAMEPAD_BACK:
            return kGamepadButtonOffset_BACK;
         case XINPUT_GAMEPAD_LEFT_THUMB:
            return kGamepadButtonOffset_LEFT_THUMB;
         case XINPUT_GAMEPAD_RIGHT_THUMB:
            return kGamepadButtonOffset_RIGHT_THUMB;
         case XINPUT_GAMEPAD_LEFT_SHOULDER:
            return kGamepadButtonOffset_LEFT_SHOULDER;
         case XINPUT_GAMEPAD_RIGHT_SHOULDER:
            return kGamepadButtonOffset_RIGHT_SHOULDER;
         case XINPUT_GAMEPAD_A:
            return kGamepadButtonOffset_A;
         case XINPUT_GAMEPAD_B:
            return kGamepadButtonOffset_B;
         case XINPUT_GAMEPAD_X:
            return kGamepadButtonOffset_X;
         case XINPUT_GAMEPAD_Y:
            return kGamepadButtonOffset_Y;
         case 0x9:
            return kGamepadButtonOffset_LT;
         case 0xA:
            return kGamepadButtonOffset_RT;
         default:
            return kMaxMacros;  // Invalid
    }
}

TESForm*  Utils::GetFormFromMod (std::string modname, uint32_t formid)            //  обертка над LookupByID()  , возвращает форму по имени мода и id
{
    if (!modname.length() || !formid) return nullptr;
    TESDataHandler* dh = TESDataHandler::GetSingleton();
    TESFile* modFile = nullptr;
    for (auto it = dh->files.begin(); it != dh->files.end(); ++it) {
         TESFile* f = *it;
         if (strcmp(f->fileName, modname.c_str()) == 0) {
            modFile = f;
            break;
         }
    }
    if (!modFile) return nullptr;
    uint8_t modIndex = modFile->compileIndex;
    uint32_t id = formid;
    if (modIndex < 0xFE) {
         id |= ((uint32_t)modIndex) << 24;
    } else {
         uint16_t lightModIndex = modFile->smallFileCompileIndex;
         if (lightModIndex != 0xFFFF) {
            id |= 0xFE000000 | (uint32_t(lightModIndex) << 12);
         }
    }
    return TESForm::LookupByID(id);
}


bool  Utils::PerformAction (BGSAction* action, Actor* actor)
{
    LOG("called PerformAction");
    if (action && actor) {
         //std::unique_ptr<TESActionData> data (TESActionData::Create());   // в старом варианте тут было через unique_ptr    // was  
         //data->source = NiPointer<TESObjectREFR>(actor);                  // 
         //data->action = action;
         //typedef bool func_t(TESActionData*);
         //REL::Relocation<func_t> func{RELOCATION_ID(40551, 41557)};  // DoIt ()
         //return func(data.get()); 
         TESActionData adata (RE::ACTIONPRIORITY::Priority_1, actor, action);   // fenix clib variant
         adata.DoIt();
    }
    return false;
}

bool  Utils::PlayIdle (TESIdleForm* idle, Actor* actor, DEFAULT_OBJECT action, Actor* target)
{
    if (actor && actor->currentProcess) {
         typedef bool (*func_t)(AIProcess*, Actor*, DEFAULT_OBJECT, TESIdleForm*, bool, bool, Actor*);
         REL::Relocation<func_t> func{RELOCATION_ID(38290, 39256)};
         return func (actor->currentProcess, actor, action, idle, true, true, target);
    }
    return false;
}

Actor*  Utils::PickActor (bhkPickData& pick, TESObjectCELL* cell, NiPoint3 origin, NiPoint3 dir, float dist, Actor* excludedActor)
{
    if (!cell) return nullptr;
    bhkWorld* world = cell->GetbhkWorld();
    if (!world) return nullptr;

    float fBS2HavokScale = bhkWorld::GetWorldScale();
    pick.rayInput.from = origin * fBS2HavokScale;
    pick.rayInput.to = (origin + dir * dist) * fBS2HavokScale;

    uint32_t collisionGroup = (uint32_t)COL_LAYER::kProjectile;
    if (excludedActor) {
         excludedActor->GetCollisionFilterInfo(collisionGroup);
    }
    collisionGroup = collisionGroup >> 16;
    pick.rayInput.filterInfo = (collisionGroup << 16) | (uint32_t)COL_LAYER::kCharController;

    world->PickObject(pick);

    if (pick.rayOutput.HasHit()) {
         auto collidable = pick.rayOutput.rootCollidable;
         if (collidable) {
            TESObjectREFR* ref = TESHavokUtilities::FindCollidableRef(*collidable);
            if (ref && ref->formType == FormType::ActorCharacter) {
                     return reinterpret_cast<Actor*>(ref);
            }
         }
    }
    return nullptr;
}

bool Utils::ActorHasKeywords(RE::Actor* actor, std::vector<RE::BGSKeyword*>& keywords)
{
    bool hasKeyword = true;
    for (const auto& keyword : keywords) {
         // logger::debug("Checking keyword {}", keyword->GetFormEditorID());
         hasKeyword = actor->HasKeyword(keyword);
         if (!hasKeyword) break;
    }
    // logger::debug("HasKeyword {}", hasKeyword);
    return hasKeyword;
}

NiPoint3 Utils::ToDirectionVector(NiMatrix3 mat) { return NiPoint3(mat.entry[2][0], mat.entry[2][1], mat.entry[2][2]); }

NiPoint3 Utils::ToUpVector(NiMatrix3 mat) { return NiPoint3(mat.entry[1][0], mat.entry[1][1], mat.entry[1][2]); }

NiPoint3 Utils::ToRightVector(NiMatrix3 mat) { return NiPoint3(mat.entry[0][0], mat.entry[0][1], mat.entry[0][2]); }

void Utils::Normalize(NiPoint3& p) {
    float l = p.Length();
    if (l == 0) {
         p.x = 1;
         p.y = 0;
         p.z = 0;
    } else {
         p.x /= l;
         p.y /= l;
         p.z /= l;
    }
}

// globals.cpp for actionManager ----------------------
PlayerCharacter* Globals::p = nullptr;
PlayerControls* Globals::pc = nullptr;
PlayerCamera* Globals::pcam = nullptr;
UI* Globals::ui = nullptr;
ControlMap* Globals::controlMap = nullptr;
UserEvents* Globals::userEvents = nullptr;
BSInputDeviceManager* Globals::inputDeviceManager = nullptr;

void Globals::InitializeGlobalVaribles() {
    p = PlayerCharacter::GetSingleton();
    pc = PlayerControls::GetSingleton();
    pcam = PlayerCamera::GetSingleton();
    ui = UI::GetSingleton();
    controlMap = ControlMap::GetSingleton();
    userEvents = UserEvents::GetSingleton();
    inputDeviceManager = BSInputDeviceManager::GetSingleton();
}




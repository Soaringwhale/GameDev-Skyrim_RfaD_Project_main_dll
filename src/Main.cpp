#include "logger.h"
#include "RE/Skyrim.h"
#pragma once

float TotalDmgCalc(RE::Actor* target, float& damage, float absorbCoeff, float cost) 
{
    float currentMana = target->GetActorValue(RE::ActorValue::kMagicka);
    float absCost = abs(cost);
    float dmgDealt;
    float dmgAbsorbed;
    float manaSpent;
    if (currentMana < (absorbCoeff) * damage * absCost)
    {
        dmgDealt = damage - (currentMana / absCost);
        dmgAbsorbed = damage - dmgDealt;
        manaSpent = abs(cost) * dmgAbsorbed;
        target->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kMagicka, -manaSpent);
        return dmgDealt;
    }
    dmgDealt = damage * (1 - absorbCoeff);
    dmgAbsorbed = damage - dmgDealt;
    manaSpent = abs(cost) * dmgAbsorbed;
    target->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kMagicka, -manaSpent);
    return dmgDealt;
}

float ManaShield(RE::Actor* target, float& damage) 
{
    float manaCostPerDmg = -4;      // Magicka cost per 1 physical damage (mod)
    auto handler = RE::TESDataHandler::GetSingleton();
    const int absorbPerkID = 0x63819A;      // Perk giving mana shield effect (mod)
    const int noArmorPerkID = 0x63819B;     // Perk improving mana shield efficiency with no armor on a character (mod)
    const int alterationBoostPerkID = 0x0D7999;     // Perk improving mana shield efficiency (original game)
    const char* espName = "RfaD SSE - Awaken.esp";
    auto absorbPerk = handler->LookupForm<RE::BGSPerk>(absorbPerkID, espName);
    auto noArmorPerk = handler->LookupForm<RE::BGSPerk>(noArmorPerkID, espName);
    auto alterationBoostPerk = RE::TESForm::LookupByID<RE::BGSPerk>(alterationBoostPerkID);
    float absorbCoeff;      // Absorbed/non-absorbed phys damage ratio
    bool hasAbsorb = target->HasPerk(absorbPerk);
    bool hasNoArmor = target->HasPerk(noArmorPerk);
    bool hasAlteration = target->HasPerk(alterationBoostPerk);
    float resultDmg;

    if (hasAbsorb) 
    {
        if (target->GetActorValue(RE::ActorValue::kMagicka) < 500) 
        {
            auto handler = RE::TESDataHandler::GetSingleton();
            //RE::SpellItem* spellToCast = handler->LookupForm<RE::SpellItem>(0x3CDE57, espName);  //Slow time test spell
            RE::SpellItem* spellToCast = handler->LookupForm<RE::SpellItem>(0x57C91C, espName);
            target->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)
               ->CastSpellImmediate(spellToCast, false, nullptr, 1.0f, false, 0.0f, target);
            return damage;
        }
        if (hasNoArmor)
        {
            if (hasAlteration) 
            {
                absorbCoeff = 0.3;
                resultDmg = TotalDmgCalc(target, damage, absorbCoeff, manaCostPerDmg);
                SKSE::log::info("{} total, {} absorbed, {}% absorption", damage, damage - resultDmg, absorbCoeff * 100);
                SKSE::log::info("No Armor perk and Alteration perk is on you");
                return resultDmg;
            }
            absorbCoeff = 0.25;
            resultDmg = TotalDmgCalc(target, damage, absorbCoeff, manaCostPerDmg);
            SKSE::log::info("{} total, {} absorbed, {}% absorption", damage, damage - resultDmg, absorbCoeff * 100);
            SKSE::log::info("No Armor perk is on you");
            return resultDmg;
        }
        if (hasAlteration)
        {
            absorbCoeff = 0.25;
            resultDmg = TotalDmgCalc(target, damage, absorbCoeff, manaCostPerDmg);
            SKSE::log::info("{} total, {} absorbed, {}% absorption", damage, damage - resultDmg, absorbCoeff * 100);
            SKSE::log::info("Alteration perk is on you");
            return resultDmg;
        }
        absorbCoeff = 0.2;
        resultDmg = TotalDmgCalc(target, damage, absorbCoeff, manaCostPerDmg);
        SKSE::log::info("{} total, {} absorbed, {}% absorption", damage, damage - resultDmg, absorbCoeff * 100);
        SKSE::log::info("no perks are on you!");
        return resultDmg;
    }
    SKSE::log::info("no Mana Shield");
    return damage;
}

float manaShield(RE::Actor* target, RE::HitData hit_data) 
{ 
    return ManaShield(target, hit_data.totalDamage); 
}


    class manaShieldHook
    {
    public:
        static void Hook() 
        {
            old_func = SKSE::GetTrampoline().write_call<5>(REL::ID(37673).address() + 0x3c0,        // Original function copied and transfered to another address
                                                           new_func);  // SkyrimSE.exe+(0x628c20+0x3ñ0)
        }

    private:
        static void new_func(RE::Actor* target, RE::HitData& hit_data)      // Function replacing original call
        {
            hit_data.totalDamage = manaShield(target, hit_data);
            SKSE::log::info("{} real dmg dealt", hit_data.totalDamage);
            return old_func(target, hit_data);
        }
        static inline REL::Relocation<decltype(new_func)> old_func;
    };

static void SKSEMessageHandler(SKSE::MessagingInterface::Message* message) 
{
    switch (message->type) {
        case SKSE::MessagingInterface::kDataLoaded:
            manaShieldHook::Hook();

            break;
    }
}

    SKSEPluginLoad(const SKSE::LoadInterface* a_skse) 
    {
        auto g_messaging =
            reinterpret_cast<SKSE::MessagingInterface*>(a_skse->QueryInterface(SKSE::LoadInterface::kMessaging));
        if (!g_messaging) 
        {
            SKSE::log::critical("Failed to load messaging interface! This error is fatal, plugin will not load.");
            return false;
        }

        SKSE::log::info("loaded");

        SKSE::Init(a_skse);
        SetupLog();       // Comment this line if you want to switch off the logger
        SKSE::AllocTrampoline(1 << 10);

        g_messaging->RegisterListener("SKSE", SKSEMessageHandler);      // Message Handler implemented in SKSE plugin loader

        return true;
    }

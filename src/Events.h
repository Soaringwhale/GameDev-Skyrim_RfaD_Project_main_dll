#pragma once
#pragma warning (disable : 4100)

#include "core.h"

namespace events
{
    void register_for_events();  // register all needed events in main

    using ActorKillev = RE::ActorKill::Event;
    class OnDeathEvent : public RE::BSTEventSink<ActorKillev>  // death event
    {
    public:
        virtual RE::BSEventNotifyControl ProcessEvent(const ActorKillev* evn, RE::BSTEventSource<ActorKillev>*) override
        {
            on_death(evn->victim, evn->killer);
            return RE::BSEventNotifyControl::kContinue;  // must be at the end of ProcessEvent()
        }
        static OnDeathEvent* GetSingleton()
        {
            static OnDeathEvent singleton;
            return std::addressof(singleton);
        }
        void register_()  //  register event listening
        {
            RE::ActorKill::GetEventSource()->AddEventSink(this);
        }
    };

    class OnActivateEvent : public RE::BSTEventSink<RE::TESActivateEvent>  // activate event
    {
    public:
        virtual RE::BSEventNotifyControl ProcessEvent(const RE::TESActivateEvent* evn,    RE::BSTEventSource<RE::TESActivateEvent>*) override    {
            on_activate(evn->actionRef, evn->objectActivated);
            return RE::BSEventNotifyControl::kContinue;
        }
        static OnActivateEvent* GetSingleton()
        {
            static OnActivateEvent singleton;
            return std::addressof(singleton);
        }
        void register_()
        {
            RE::ScriptEventSourceHolder::GetSingleton()->GetEventSource<RE::TESActivateEvent>()->AddEventSink(this);
        }
    };

    class LocationChangeEvent :    public RE::BSTEventSink<RE::TESActorLocationChangeEvent>  // change location event  (won't be called when changing cells of same location)
    {
    public:
        virtual RE::BSEventNotifyControl ProcessEvent(const RE::TESActorLocationChangeEvent* evn,    RE::BSTEventSource<RE::TESActorLocationChangeEvent>*) override
        {
            on_location_change(evn->actor, evn->oldLoc, evn->newLoc);
            return RE::BSEventNotifyControl::kContinue;
        }
        static LocationChangeEvent* GetSingleton()
        {
            static LocationChangeEvent singleton;
            return std::addressof(singleton);
        }
        void register_()
        {
            RE::ScriptEventSourceHolder::GetSingleton()->GetEventSource<RE::TESActorLocationChangeEvent>()->AddEventSink(this);
        }
    };

    class ActorCellEvent : public RE::BSTEventSink<RE::BGSActorCellEvent>  // player enters another cell
    {
    public:
        virtual RE::BSEventNotifyControl ProcessEvent(const RE::BGSActorCellEvent* evn,    RE::BSTEventSource<RE::BGSActorCellEvent>*) override
        {
            bool leave = evn->flags.any(RE::BGSActorCellEvent::CellFlag::kLeave);
            if (leave) {
                //
            }
            return RE::BSEventNotifyControl::kContinue;
        }
        static ActorCellEvent* GetSingleton()
        {
            static ActorCellEvent singleton;
            return std::addressof(singleton);
        }
        void register_() { RE::PlayerCharacter::GetSingleton()->GetEventSource<RE::BGSActorCellEvent>()->AddEventSink(this); }
    };

    class CellAttachDetachEvent : public RE::BSTEventSink<RE::TESCellAttachDetachEvent>  // вызывается для референсов из новой cell когда она загружается, не работает для игрока
    {                                      //
      public:
        virtual RE::BSEventNotifyControl ProcessEvent(const RE::TESCellAttachDetachEvent* evn,    RE::BSTEventSource<RE::TESCellAttachDetachEvent>*) override
        {
            //...
            return RE::BSEventNotifyControl::kContinue;
        }
        static CellAttachDetachEvent* GetSingleton() 
        {
            static CellAttachDetachEvent singleton;
            return std::addressof(singleton);
        }
        void register_()
        {
            RE::ScriptEventSourceHolder::GetSingleton()->GetEventSource<RE::TESCellAttachDetachEvent>()->AddEventSink(this);
        }
    };

    class CellFullyLoadedEvent : public RE::BSTEventSink<RE::TESCellFullyLoadedEvent>  // вероятно окружающие ячейки из GridsToLoad, т.е. 5 ячеек за раз
    {                                                         //
      public:
        virtual RE::BSEventNotifyControl ProcessEvent(const RE::TESCellFullyLoadedEvent* evn, RE::BSTEventSource<RE::TESCellFullyLoadedEvent>*) override
        {
            // ...
            return RE::BSEventNotifyControl::kContinue;
        }
        static CellFullyLoadedEvent* GetSingleton()
        {
            static CellFullyLoadedEvent singleton;
            return std::addressof(singleton);
        }
        void register_()
        {
            RE::ScriptEventSourceHolder::GetSingleton()->GetEventSource<RE::TESCellFullyLoadedEvent>()->AddEventSink(this);
        }
    };

    class ObjectLoadedEvent : public RE::BSTEventSink<RE::TESObjectLoadedEvent>  // object loaded event  / better use 3DLoad Hook
    {
      public:
        virtual RE::BSEventNotifyControl ProcessEvent(const RE::TESObjectLoadedEvent* evn,    RE::BSTEventSource<RE::TESObjectLoadedEvent>*) override
        {
            // on_object_loaded (evn->formID, evn->loaded);
            return RE::BSEventNotifyControl::kContinue;
        }
        static ObjectLoadedEvent* GetSingleton()
        {
            static ObjectLoadedEvent singleton;
            return std::addressof(singleton);
        }
        void register_()
        {
            RE::ScriptEventSourceHolder::GetSingleton()->GetEventSource<RE::TESObjectLoadedEvent>()->AddEventSink(this);
        }
    };

    //---------------------------------------------------------------------------- open/close menues event listen ------------------------------------------------------
    class MenuOpenCloseEventSink : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
    {
      public:
        virtual RE::BSEventNotifyControl ProcessEvent (const RE::MenuOpenCloseEvent* ev,    RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override;  // on menues open/сlose

        static MenuOpenCloseEventSink* GetSingleton()
        {
            static MenuOpenCloseEventSink singleton;
            return std::addressof(singleton);
        }
        void register_()  // register for catching menues open/close
        {
            RE::UI::GetSingleton()->AddEventSink<RE::MenuOpenCloseEvent>(this);
        }
    };
    //------------------------------------------------------------ input event listen ----------------------------------------------------------------------
    class  InputEvent : public RE::BSTEventSink<InputEvents>
    {
      public:
        virtual RE::BSEventNotifyControl ProcessEvent (const InputEvents* evns,	RE::BSTEventSource<InputEvents>* dispatcher) override {
            return on_key_input(evns); 
        }
        //TES_HEAP_REDEFINE_NEW();

        static InputEvent* GetSingleton() { 
            static InputEvent singleton;
            return std::addressof(singleton);
        }
        void register_()  {                          // register for event
            RE::BSInputDeviceManager::GetSingleton()->AddEventSink(this);
        }
    };
};





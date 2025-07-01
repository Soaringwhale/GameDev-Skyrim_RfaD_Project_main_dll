#pragma once

using namespace SKSE;

using InputEvents = RE::InputEvent*;
class  InputWatcher  :  public  RE::BSTEventSink<InputEvents>        // наследуемся от BSTEventSink чтобы переопределить одну функцию  (ProcessEvent)
{
  public:
    virtual RE::BSEventNotifyControl ProcessEvent (const InputEvents* evns, RE::BSTEventSource<InputEvents>* dispatcher)  override;
    TES_HEAP_REDEFINE_NEW();

    static InputWatcher* GetSingleton() { 
        static InputWatcher singleton;
        return std::addressof(singleton);
    }
    void register_()  {                          // register to catch buttons
        RE::BSInputDeviceManager::GetSingleton()->AddEventSink(this);
    }
};

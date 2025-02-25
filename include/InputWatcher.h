#pragma once

using namespace SKSE;

using InputEvents = RE::InputEvent*;


class  InputWatcher  :  public  RE::BSTEventSink<InputEvents>        // наследуемся от BSTEventSink чтобы переопределить одну функцию  (ProcessEvent)
{
  public:
    virtual RE::BSEventNotifyControl ProcessEvent (const InputEvents* evns, RE::BSTEventSource<InputEvents>* dispatcher)  override;
    TES_HEAP_REDEFINE_NEW();


    static InputWatcher* GetSingleton()        // my.
    {
        static InputWatcher singleton;
        return std::addressof(singleton);
    }

    void enable()                            // my.  this method we call in main to register our class for work.
    {
        if (auto input = RE::BSInputDeviceManager::GetSingleton()) {
            input->AddEventSink(this);
        }
    }
};


#include "InputWatcher.h"
#include "utils.h"
#include "core.h"

using namespace RE;

BSEventNotifyControl  InputWatcher::ProcessEvent (const InputEvents* evns,  BSTEventSource<InputEvents>* dispatcher)		//  ProcessEvent() это непосредственно обработка нажатий.
{
    if (!*evns) return RE::BSEventNotifyControl::kContinue;

    for (InputEvent *e = *evns;  e;  e = e->next)		//  начинаем идти по InputEvents пока евент (е) истинен
	{			
        switch (e->eventType.get()) {
            case INPUT_EVENT_TYPE::kButton:

                ButtonEvent* buttonEvent = e->AsButtonEvent();

				if (buttonEvent && buttonEvent->HasIDCode())		//   && (buttonEvent->IsDown() || buttonEvent->IsPressed())
				{
                    uint32_t keyMask = buttonEvent->idCode;
                    uint32_t keyCode;

                    // Mouse
                    if (buttonEvent->device.get() == INPUT_DEVICE::kMouse) {
                        keyCode = Utils::kMacro_NumKeyboardKeys + keyMask;
                    }
                    // Gamepad
                    else if (buttonEvent->device.get() == INPUT_DEVICE::kGamepad) {
                        keyCode = Utils::GamepadMaskToKeycode(keyMask);  // for gamepad we need convert
                    }
                    // Keyboard
                    else
                        keyCode = keyMask;  //  for keyboard, keymask == keycode

                    // Valid scancode?
                    if (keyCode >= Utils::kMaxMacros) continue;

                    float duration  = buttonEvent->heldDownSecs;
                    //bool isPressed  = buttonEvent->value != 0 && duration >= 0;
                    //bool isReleased = buttonEvent->value == 0 && duration != 0;

					if (keyCode != 17)																// for [W]
						mys::handle_keyPress(keyCode, duration, buttonEvent->IsUp());
					

                    //if (isPressed) {
                        // ActionManager::GetSingleton()->OnKeyPressed(keyCode, duration);
                        // if (Globals::ui->numPausesGame == 0 && Globals::p->GetActorRuntimeData().currentProcess)
                        // ActionManager::GetSingleton()->EvaluateAndPerformActions();
                    //} else if (isReleased) {
                        // ActionManager::GetSingleton()->OnKeyReleased(keyCode);
                    //}
				}

                
        }
    }

    return BSEventNotifyControl::kContinue;
}
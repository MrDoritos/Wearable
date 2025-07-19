#include "ui_func.h"

namespace wbl {
namespace UI {

void dispatch_input_events(IElement &element, Dpad &dpad, const bool &update_dpad) {
    for (int i = 0; i < dpad.button_count; i++) {
        const Dpad::Button &button = dpad.buttons[i];

        const UI::EventValues values[] = {
            UI::EventValues::DPAD_ENTER,
            UI::EventValues::DPAD_UP,
            UI::EventValues::DPAD_RIGHT,
            UI::EventValues::DPAD_DOWN,
            UI::EventValues::DPAD_LEFT,
        };

        UI::Event input_event(UI::EventTypes::USER_INPUT);

        if (button.is_pressed())
            input_event.value = (UI::EventValues)(input_event.value | UI::EventValues::PRESSED);
        if (button.is_held())
            input_event.value = (UI::EventValues)(input_event.value | UI::EventValues::HELD);
        if (button.is_released())
            input_event.value = (UI::EventValues)(input_event.value | UI::EventValues::RELEASED);

        if (!input_event.value)
            continue;
        input_event.direction = UI::EventDirection::CHILDREN;

        input_event.value = (UI::EventValues)(input_event.value | values[i]);

        element.dispatch_event(input_event);
    }

    #ifdef INPUT_DEBUG
    dpad.print_states();
    #endif

    if (update_dpad)
        dpad.update();
}

}
}
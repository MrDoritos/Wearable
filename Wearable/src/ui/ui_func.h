#pragma once

#include "config.h"
#include "ui.h"
#include "user_inputs.h"

namespace wbl {
namespace UI {

int dispatch_input_events(IElement &element, Dpad &dpad, const bool &update_dpad = true);

}
}
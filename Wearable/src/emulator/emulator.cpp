#include <thread>
#include <signal.h>
#include <stdlib.h>
#include "console.h"
#include "user_inputs.h"

extern "C" void app_main();

void handle_signal(int signal) {
    console::cons.~constructor();
    //wbl::dpad.~Dpad();
    exit(0);
}

int main() {
    signal(SIGINT, handle_signal);
    signal(SIGQUIT, handle_signal);
    app_main();
    return 0;
}
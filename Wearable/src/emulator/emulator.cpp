#include <thread>
#include "console.h"

extern "C" void app_main();

int main() {
    std::thread app(app_main);

    int key;

    while (true) {
        key = console::readKey();

        if (key == VK_ESCAPE || key == 'q')
            break;
    }

    app.~thread();

    return 0;
}
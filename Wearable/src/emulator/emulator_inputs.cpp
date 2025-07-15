#include "user_inputs.h"
#include "console.h"

#include <thread>
#include <mutex>
#include <condition_variable>
#include <signal.h>

std::thread input_thread;
wbl::Dpad wbl::dpad = wbl::Dpad();

std::mutex mux;
std::condition_variable cv;
bool running = false;

void input_loop() {
    running = true;

    {
    std::unique_lock<std::mutex> lock(mux);
    while (running) {
        int key = console::readKey();

        if (key < 1)
            break;

        wbl::Dpad::Button *action = nullptr;

        switch (key) {
            case 402: action = &wbl::dpad.down; break;
            case 403: action = &wbl::dpad.up; break;
            case 404: action = &wbl::dpad.left; break;
            case 405: action = &wbl::dpad.right; break;
            case '\n': action = &wbl::dpad.enter; break;
            case 'q':
            case 0x1b:
                kill(0, SIGINT);
                break;
            default: continue;
        }

        if (!action)
            continue;

        action->rising_edge();
        action->falling_edge();
    }
    }

    running = false;
    return 0;
}

wbl::Dpad::~Dpad() {
    if (running) {
        fclose(stdin);
        input_thread.join();
        freopen("/dev/stdin", "r", stdin);
        fflush(stdin);
    }
}

esp_err_t wbl::Dpad::init() {
    input_thread = std::thread(input_loop);
    return ESP_OK;
}
#include <zephyr/kernel.h>
#include <zephyr/input/input.h>
#include <zephyr/dt-bindings/input/input-event-codes.h>
#include <zephyr/logging/log.h>
#include "display_direct.h"
#include "font.h"
#include "colors.h"

LOG_MODULE_REGISTER(task2, LOG_LEVEL_INF);
K_MSGQ_DEFINE(button_queue, sizeof(uint16_t), 8, 4);

static void button_callback(struct input_event *evt, void *user_data)
{
    ARG_UNUSED(user_data);
    if (evt->type == INPUT_EV_KEY && evt->value == 1) {
        uint16_t code = evt->code;
        (void)k_msgq_put(&button_queue, &code, K_NO_WAIT);
    }
}
INPUT_CALLBACK_DEFINE(NULL, button_callback, NULL);

static void show_message(uint16_t code)
{
    const char *message;
    uint16_t color;
    switch (code) {
    case INPUT_KEY_0: message = "BUTTON A\nLEFT"; color = COLOR_RED; break;
    case INPUT_KEY_1: message = "BUTTON B\nMIDDLE"; color = COLOR_LIME; break;
    case INPUT_KEY_2: message = "BUTTON C\nRIGHT"; color = COLOR_BLUE; break;
    default: return;
    }
    screen_clear();
    draw_string_scaled(30, 60, message, color, 2);
    LOG_INF("%s", message);
}

int main(void)
{
    display_direct_init();
    screen_clear();
    draw_string(10, 20, "Press A, B or C", COLOR_WHITE);
    while (1) {
        uint16_t code;
        k_msgq_get(&button_queue, &code, K_FOREVER);
        show_message(code);
    }
    return 0;
}

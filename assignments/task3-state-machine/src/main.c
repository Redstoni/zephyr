#include <zephyr/kernel.h>
#include <zephyr/input/input.h>
#include <zephyr/dt-bindings/input/input-event-codes.h>
#include <zephyr/smf.h>
#include <zephyr/device.h>
#include <zephyr/drivers/led_strip.h>
#include <zephyr/logging/log.h>
#include "display_direct.h"
#include "font.h"
#include "colors.h"

LOG_MODULE_REGISTER(task3, LOG_LEVEL_INF);
K_MSGQ_DEFINE(button_queue, sizeof(uint16_t), 8, 4);

enum state_id { S1, S2, S3 };
struct app_data { struct smf_ctx ctx; };
static struct app_data app;
static const struct smf_state states[];

static void enter_s1(void *o) { ARG_UNUSED(o); screen_clear(); draw_string_scaled(50, 60, "STATE S1", COLOR_RED, 2); LOG_INF("S1"); }
static void enter_s2(void *o) { ARG_UNUSED(o); screen_clear(); draw_string_scaled(50, 60, "STATE S2", COLOR_LIME, 2); LOG_INF("S2"); }
static void enter_s3(void *o) { ARG_UNUSED(o); screen_clear(); draw_string_scaled(50, 60, "STATE S3", COLOR_BLUE, 2); LOG_INF("S3"); }

static enum smf_state_result run_common(void *o)
{
    uint16_t code;
    if (k_msgq_get(&button_queue, &code, K_MSEC(50)) == 0) {
        switch (code) {
        case INPUT_KEY_0: smf_set_state(SMF_CTX(o), &states[S1]); break;
        case INPUT_KEY_1: smf_set_state(SMF_CTX(o), &states[S2]); break;
        case INPUT_KEY_2: smf_set_state(SMF_CTX(o), &states[S3]); break;
        default: break;
        }
    }
    return SMF_EVENT_HANDLED;
}
static const struct smf_state states[] = {
    [S1] = SMF_CREATE_STATE(enter_s1, run_common, NULL, NULL, NULL),
    [S2] = SMF_CREATE_STATE(enter_s2, run_common, NULL, NULL, NULL),
    [S3] = SMF_CREATE_STATE(enter_s3, run_common, NULL, NULL, NULL),
};

static void button_callback(struct input_event *evt, void *user_data)
{
    ARG_UNUSED(user_data);
    if (evt->type == INPUT_EV_KEY && evt->value == 1) {
        uint16_t code = evt->code;
        (void)k_msgq_put(&button_queue, &code, K_NO_WAIT);
    }
}
INPUT_CALLBACK_DEFINE(NULL, button_callback, NULL);

#define STRIP_NODE DT_ALIAS(led_strip)
#define PIXELS DT_PROP(STRIP_NODE, chain_length)
static const struct device *const strip = DEVICE_DT_GET(STRIP_NODE);
static struct led_rgb leds[PIXELS];

static void blink_thread(void *a, void *b, void *c)
{
    ARG_UNUSED(a); ARG_UNUSED(b); ARG_UNUSED(c);
    if (!device_is_ready(strip)) {
        LOG_ERR("LED strip not ready");
        return;
    }
    bool on = false;
    while (1) {
        on = !on;
        leds[0].r = on ? 32 : 0;
        leds[0].g = 0;
        leds[0].b = 0;
        (void)led_strip_update_rgb(strip, leds, PIXELS);
        k_sleep(K_MSEC(500));
    }
}
K_THREAD_DEFINE(blink_tid, 2048, blink_thread, NULL, NULL, NULL, 5, 0, 0);

int main(void)
{
    display_direct_init();
    smf_set_initial(SMF_CTX(&app), &states[S1]);
    while (1) {
        int rc = smf_run_state(SMF_CTX(&app));
        if (rc != 0) { LOG_ERR("SMF error: %d", rc); break; }
    }
    return 0;
}

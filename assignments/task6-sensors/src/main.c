/* BIE-IOT Sensors 1: onboard gyroscope, zbus and display */
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/zbus/zbus.h>
#include <zephyr/input/input.h>
#include <zephyr/dt-bindings/input/input-event-codes.h>
#include <zephyr/sys/atomic.h>
#include <zephyr/logging/log.h>
#include <stdio.h>
#include "display_direct.h"
#include "font.h"
#include "colors.h"

LOG_MODULE_REGISTER(sensor_app);

struct gyro_msg {
    struct sensor_value x, y, z;
};

ZBUS_SUBSCRIBER_DEFINE(display_sub, 4);
ZBUS_CHAN_DEFINE(gyro_chan, struct gyro_msg, NULL, NULL,
                 ZBUS_OBSERVERS(display_sub), ZBUS_MSG_INIT(0));

static const uint32_t intervals_ms[] = {1000, 500, 200};
static atomic_t frequency_index = ATOMIC_INIT(1);
static atomic_t sampling_active = ATOMIC_INIT(1);
static K_SEM_DEFINE(display_ready, 0, 1);

#define SAMPLE_STACK_SIZE 1536
#define DISPLAY_STACK_SIZE 2048
#define THREAD_PRIORITY 5

static void sampling_thread(void *a, void *b, void *c)
{
    ARG_UNUSED(a); ARG_UNUSED(b); ARG_UNUSED(c);
    const struct device *mpu = DEVICE_DT_GET_ONE(invensense_mpu6050);
    if (!device_is_ready(mpu)) {
        LOG_ERR("MPU gyroscope not ready");
        return;
    }
    while (true) {
        struct sensor_value xyz[3];
        if (sensor_sample_fetch(mpu) == 0 &&
            sensor_channel_get(mpu, SENSOR_CHAN_GYRO_XYZ, xyz) == 0) {
            struct gyro_msg msg = {.x = xyz[0], .y = xyz[1], .z = xyz[2]};
            (void)zbus_chan_pub(&gyro_chan, &msg, K_MSEC(100));
        } else {
            LOG_WRN("Gyroscope read failed");
        }
        k_msleep(intervals_ms[atomic_get(&frequency_index)]);
    }
}
K_THREAD_DEFINE(gyro_tid, SAMPLE_STACK_SIZE, sampling_thread,
                NULL, NULL, NULL, THREAD_PRIORITY, 0, 0);

static void display_thread(void *a, void *b, void *c)
{
    ARG_UNUSED(a); ARG_UNUSED(b); ARG_UNUSED(c);
    k_sem_take(&display_ready, K_FOREVER);
    const struct zbus_channel *chan;
    while (true) {
        if (zbus_sub_wait(&display_sub, &chan, K_FOREVER) != 0 ||
            chan != &gyro_chan) continue;
        struct gyro_msg msg;
        if (zbus_chan_read(&gyro_chan, &msg, K_MSEC(100)) != 0) continue;
        char line[80];
        screen_clear();
        draw_string_scaled(10, 10, "GYROSCOPE", COLOR_AQUA, 2);
        draw_string(10, 55, "Angular speed (rad/s)", COLOR_WHITE);
        snprintf(line, sizeof(line), "X: %+.3f", sensor_value_to_double(&msg.x));
        draw_string_scaled(10, 90, line, COLOR_WHITE, 2);
        snprintf(line, sizeof(line), "Y: %+.3f", sensor_value_to_double(&msg.y));
        draw_string_scaled(10, 125, line, COLOR_WHITE, 2);
        snprintf(line, sizeof(line), "Z: %+.3f", sensor_value_to_double(&msg.z));
        draw_string_scaled(10, 160, line, COLOR_WHITE, 2);
        snprintf(line, sizeof(line), "%s  %u ms   A:go B:stop C:rate",
                 atomic_get(&sampling_active) ? "RUN" : "STOP",
                 intervals_ms[atomic_get(&frequency_index)]);
        draw_string(10, 215, line, COLOR_YELLOW);
    }
}
K_THREAD_DEFINE(display_tid, DISPLAY_STACK_SIZE, display_thread,
                NULL, NULL, NULL, THREAD_PRIORITY, 0, 0);

static void button_cb(struct input_event *evt, void *user_data)
{
    ARG_UNUSED(user_data);
    if (evt->type != INPUT_EV_KEY || evt->value == 0) return;
    switch (evt->code) {
    case INPUT_KEY_0: /* A = left */
        atomic_set(&sampling_active, 1);
        k_thread_resume(gyro_tid);
        LOG_INF("Sampling started");
        break;
    case INPUT_KEY_1: /* B = middle */
        atomic_set(&sampling_active, 0);
        k_thread_suspend(gyro_tid);
        LOG_INF("Sampling stopped");
        break;
    case INPUT_KEY_2: { /* C = right */
        atomic_val_t old_index = atomic_get(&frequency_index);
        atomic_set(&frequency_index, (old_index + 1) % 3);
        LOG_INF("Sample interval: %u ms", intervals_ms[atomic_get(&frequency_index)]);
        break;
    }
    default: break;
    }
}
INPUT_CALLBACK_DEFINE(NULL, button_cb, NULL);

int main(void)
{
    display_direct_init();
    draw_string(10, 10, "Waiting for gyroscope...", COLOR_WHITE);
    k_sem_give(&display_ready);
    LOG_INF("A=start B=stop C=sampling interval");
    return 0;
}

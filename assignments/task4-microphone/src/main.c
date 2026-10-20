#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/led_strip.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

LOG_MODULE_REGISTER(task4, LOG_LEVEL_INF);
#define STRIP_NODE DT_ALIAS(led_strip)
#define PIXELS DT_PROP(STRIP_NODE, chain_length)
#define SAMPLES 80
#define NOISE_FLOOR 40   /* tune in lab, using logged peak-to-peak values */
#define FULL_SCALE 900   /* tune in lab, using logged peak-to-peak values */

static const struct adc_dt_spec mic = ADC_DT_SPEC_GET(DT_PATH(zephyr_user));
static const struct device *const strip = DEVICE_DT_GET(STRIP_NODE);
static struct led_rgb pixels[PIXELS];

int main(void)
{
    if (!adc_is_ready_dt(&mic) || !device_is_ready(strip)) {
        LOG_ERR("ADC or LED strip not ready");
        return 0;
    }
    int rc = adc_channel_setup_dt(&mic);
    if (rc) { LOG_ERR("ADC setup failed: %d", rc); return 0; }

    int16_t sample;
    struct adc_sequence seq = { .buffer = &sample, .buffer_size = sizeof(sample) };
    while (1) {
        int min_value = INT16_MAX, max_value = INT16_MIN;
        for (int i = 0; i < SAMPLES; ++i) {
            rc = adc_sequence_init_dt(&mic, &seq);
            if (rc == 0) { rc = adc_read_dt(&mic, &seq); }
            if (rc) { LOG_ERR("ADC read failed: %d", rc); break; }
            min_value = MIN(min_value, (int)sample);
            max_value = MAX(max_value, (int)sample);
            k_sleep(K_MSEC(1));
        }
        if (rc) { k_sleep(K_MSEC(250)); continue; }

        /* Microphone signal is AC around a DC bias: measure peak-to-peak. */
        int peak_to_peak = max_value - min_value;
        int level = CLAMP(peak_to_peak - NOISE_FLOOR, 0, FULL_SCALE - NOISE_FLOOR);
        int lit = (level * PIXELS + (FULL_SCALE - NOISE_FLOOR - 1)) /
                  (FULL_SCALE - NOISE_FLOOR);
        for (int i = 0; i < PIXELS; ++i) {
            pixels[i].r = (i < lit) ? 40 : 0;
            pixels[i].g = 0;
            pixels[i].b = 0;
        }
        rc = led_strip_update_rgb(strip, pixels, PIXELS);
        if (rc) { LOG_ERR("LED update failed: %d", rc); }
        LOG_INF("Microphone peak-to-peak: %d, LEDs: %d/%d", peak_to_peak, lit, PIXELS);
        k_sleep(K_MSEC(60));
    }
    return 0;
}

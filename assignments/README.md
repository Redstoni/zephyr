# BIE-IOT graded assignments — prepared for lab testing

These are **untested source drafts**, not confirmed working submissions. They are based on the supplied zephyr-m5-fit examples. The instructor may use a different Zephyr revision; fix build errors before claiming completion.

## Build each app in the existing Zephyr workspace

```sh
source ~/zephyrproject/.venv/bin/activate
export ZEPHYR_BASE=~/zephyrproject/zephyr
cd /path/to/zephyr-m5-fit-main
west build -p always -b m5stack_fire/esp32/procpu -d build assignments/task1-shell
west flash -d build
```

To build another app, replace `assignments/task1-shell` with its folder. Flash each independently (only one application runs on the board at a time). Serial console is typically `picocom -b 115200 /dev/ttyACM0`, or `/dev/ttyUSB0`.

## Test plan

1. **task1-shell**: open Zephyr shell, type `hello` and see the greeting. If using the QEMU-based `zephyr4squirrels` environment, the shell command can be ported there as well.
2. **task2-buttons-display**: press left, middle, right, verify screen changes to A, B, C.
3. **task3-state-machine**: press A/B/C, verify S1/S2/S3; verify the first onboard LED-strip pixel blinks twice per second independently of input. In this implementation, three states select each other on any button press, exactly as the provided transition table describes.
4. **task4-microphone**: monitor serial log; clap and compare peak-to-peak value with quiet room. Verify louder sounds light more red LEDs. Tune `NOISE_FLOOR` and `FULL_SCALE` in `main.c` according to observed values; do not represent them as calibrated dB values.

## Hardware wiring reference for task 4

Official M5Stack Fire documentation lists microphone at GPIO34, RGB LED signal at GPIO15, and speaker at GPIO25. For the classic ESP32, GPIO34 corresponds to ADC1 channel 6. Sources: https://docs.m5stack.com/en/core/fire and the ESP32 datasheet. Zephyr's ADC uses `&adc0` / `channel@6` in the overlay. The microphone shares analog resources with speaker; speaker is disabled per assignment instructions.

## Notes

- No physical M5Stack was available to verify these applications.
- Run a clean build for each application and check the resolved devicetree if Task 4's ADC or speaker node differs in your Zephyr revision.
- Task 3 LED blinking refers to pixel 0 of the RGB LED strip, not the LCD backlight.
- All assignments are separate, so flashing one replaces the previous program.

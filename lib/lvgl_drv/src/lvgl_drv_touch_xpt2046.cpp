#ifdef LVGL_TOUCH_XPT2046

#include <Arduino.h>
#include <lvgl_drv.h>

#define CMD_START_Z1_CONVERSION 0xB1
#define CMD_START_Z2_CONVERSION 0xC1
#define CMD_START_X_CONVERSION 0xD1
#define CMD_START_Y_CONVERSION 0x91
#define Z_THRESHOLD 600

bool lvgl_touch_xpt2046_read_xy(int16_t *x, int16_t *y)
{
    lvgl_bus_spi.beginTransaction(SPISettings(LVGL_XPT2046_SPI_FREQ, MSBFIRST, SPI_MODE0));
    digitalWrite(PIN_XPT2046_CS, LOW);
    lvgl_bus_spi.transfer16(CMD_START_Z1_CONVERSION);
    auto z1 = lvgl_bus_spi.transfer16(CMD_START_Z2_CONVERSION) >> 3;
    auto z2 = lvgl_bus_spi.transfer16(CMD_START_X_CONVERSION) >> 3;
    auto raw_x = lvgl_bus_spi.transfer16(CMD_START_Y_CONVERSION) >> 3; // Normalize to 12 bits
    auto raw_y = lvgl_bus_spi.transfer16(0) >> 3;                      // Normalize to 12 bits
    digitalWrite(PIN_XPT2046_CS, HIGH);
    lvgl_bus_spi.endTransaction();
    int16_t z = z1 + 4095 - z2;
    if (z < Z_THRESHOLD)
        return false;

#if 0 // For calibration
    static auto min_x = INT_MAX, max_x = -INT_MAX, min_y = INT_MAX, max_y = -INT_MAX;
    if (raw_x < min_x)
        min_x = raw_x;
    if (raw_x > max_x)
        max_x = raw_x;
    if (raw_y < min_y)
        min_y = raw_y;
    if (raw_y > max_y)
        max_y = raw_y;
    log_i("min_x=%d,max_x=%d,min_y=%d,max_y=%d", min_x, max_x, min_y, max_y);
#endif
#ifdef TFT_ORIENTATION_PORTRAIT
    *x = ((raw_x - LVGL_XPT2046_MIN_X) * TFT_WIDTH) / (LVGL_XPT2046_MAX_X - LVGL_XPT2046_MIN_X);
    *y = TFT_HEIGHT - ((raw_y - LVGL_XPT2046_MIN_Y) * TFT_HEIGHT) / (LVGL_XPT2046_MAX_Y - LVGL_XPT2046_MIN_Y);
#else
#ifdef TFT_ORIENTATION_LANDSCAPE
#else
#ifdef TFT_ORIENTATION_PORTRAIT_INV
#else
#ifdef TFT_ORIENTATION_LANDSCAPE_INV
#else
#error TFT_ORIENTATION not defined!
#endif
#endif
#endif
#endif
    log_i("x=%d,y=%d,raw_z=%d", *x, *y, z);
    return true;
}

void lvgl_touch_init()
{
    pinMode(PIN_XPT2046_CS, OUTPUT);
    digitalWrite(PIN_XPT2046_CS, true);
}

void lvgl_touch_read(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    static int16_t last_x = 0, last_y = 0;
    // log_d("Touch: (%d,%d)", last_x, last_y);
    data->state = lvgl_touch_xpt2046_read_xy(&last_x, &last_y) ? LV_INDEV_STATE_PR : LV_INDEV_STATE_RELEASED;
    data->point.x = last_x;
    data->point.y = last_y;
    // log_d("Touch: (%d,%d)", last_x, last_y);
}

#endif
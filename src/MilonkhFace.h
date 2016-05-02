#ifndef MILONKH_FACE_H
#define MILONKH_FACE_H

#include <pebble.h>

static void init();
static void finish();
static void update_time();
static void update_battery(Layer *layer, GContext *ctx);
static void update_colors();
static void update_fonts();
static GColor random_color(bool light);
static int random_hex_val(int min, int max);
static void load_main_window();
static void unload_main_window();
static void tick_handler(struct tm *tick_time, TimeUnits units_changed);
static void battery_callback(BatteryChargeState state);

#endif

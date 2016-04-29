#ifndef MILONKH_FACE_H
#define MILONKH_FACE_H

static void init();
static void finish();
static void update_time();
static void update_battery(Layer *layer, GContext *ctx);
static void update_colors();
static GColor inverted_color(unsigned int r, unsigned int g, unsigned int b);
static void load_main_window();
static void unload_main_window();
static void tick_handler(struct tm *tick_time, TimeUnits units_changed);
static void battery_callback(BatteryChargeState state);

#endif

#include <pebble.h>
#include "MilonkhFace.h"
#include "Settings.h"

static Window *main_window;
static TextLayer *time_layer, *date_layer, *header_layer;
static Layer *battery_layer;
static GFont time_font, date_font;
static BitmapLayer *background_layer;
static GBitmap *background_bitmap;

static int battery_level = 0;

static GColor default_background_color, default_color;

static void init() {
	main_window = window_create();

	window_set_window_handlers(main_window, (WindowHandlers) {
		.load = load_main_window,
		.unload = unload_main_window
	});

	// show window, animated = true, black background
	window_stack_push(main_window, true);
	window_set_background_color(main_window, GColorBlack);

	// make sure time is right at the start
	update_time();

	// register for time updates
	tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
	
	// init random number generator
	srand(time(NULL));
}

static void finish() {
	window_destroy(main_window);
}

static void update_colors() {
	// generate some random backgrounds and contrasting text colors
	unsigned int r = rand()%256, g = rand()%256, b = rand()%256;
	GColor time_background_color = GColorFromRGB(r, g, b);
	//GColor time_color = inverted_color(r, g, b);
	GColor time_color = gcolor_legible_over(time_background_color);
	
	r = rand()%256, g = rand()%256, b = rand()%256;
	default_background_color = GColorFromRGB(r, g, b);
	//default_color = inverted_color(r, g, b);
	default_color = gcolor_legible_over(default_background_color);
	
	// update main and time layer background color
	window_set_background_color(main_window, default_background_color);
	text_layer_set_background_color(time_layer, time_background_color);
	
	// update all text layer's colors
	text_layer_set_text_color(time_layer, time_color);
	text_layer_set_text_color(header_layer, default_color);
	text_layer_set_text_color(date_layer, default_color);
	
	// redraw battery to update its colors
	battery_callback(battery_state_service_peek());
}

static GColor inverted_color(unsigned int r, unsigned int g, unsigned int b) {
	return GColorFromRGB(255-r, 255-g, 255-b);
}

static void update_time() {
	update_colors();
	
	// get the time
	time_t temp = time(NULL);
	struct tm *tick_time = localtime(&temp);

	// write time into string
	static char time_buffer[8];
	strftime(time_buffer, sizeof(time_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
	
	// write date into string
	static char date_buffer[16];
	strftime(date_buffer, sizeof(date_buffer), "%d %b (%a)", tick_time);

	// display it
	text_layer_set_text(time_layer, time_buffer);
	text_layer_set_text(date_layer, date_buffer);
}

static void update_battery(Layer *layer, GContext *ctx) {
	GRect bounds = layer_get_bounds(layer);

	// determine bar width
	int width = (int)(float)(((float)battery_level / 100.0f) * 114.0f);

	// draw background
	graphics_context_set_fill_color(ctx, default_background_color);
	graphics_fill_rect(ctx, bounds, 0, BATTERY_CORNERS);

	// draw bar
	graphics_context_set_fill_color(ctx, default_color);
	graphics_fill_rect(ctx, GRect(0, 0, width, bounds.size.h), 0, BATTERY_CORNERS);
}

static void load_main_window(Window *window) {
	// window information
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);

	// create and draw background
	/*background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
	background_layer = bitmap_layer_create(bounds);
	bitmap_layer_set_bitmap(background_layer, background_bitmap);
	layer_add_child(window_layer, bitmap_layer_get_layer(background_layer));*/

	// get fonts
	time_font = fonts_load_custom_font(resource_get_handle(TIME_FONT));
	date_font = fonts_load_custom_font(resource_get_handle(DATE_FONT));
	
	// create and layout header layer
	header_layer = text_layer_create(HEADER_BOUNDS);
	text_layer_set_text_color(header_layer, TEXT_COLOR);
	text_layer_set_background_color(header_layer, GColorClear);
	text_layer_set_text(header_layer, HEADER_TEXT);
	text_layer_set_font(header_layer, date_font); // TODO different font?
	text_layer_set_text_alignment(header_layer, GTextAlignmentCenter); 
	
	// create and layout time layer
	time_layer = text_layer_create(
		GRect(TIME_X, PBL_IF_ROUND_ELSE(TIME_Y_ROUND, TIME_Y_NORMAL), bounds.size.w, TIME_HEIGHT));
	text_layer_set_background_color(time_layer, GColorClear);
	text_layer_set_text_color(time_layer, TEXT_COLOR);
	text_layer_set_text(time_layer, "13:37");
	text_layer_set_font(time_layer, time_font);
	text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
	
	// create and layout date layer
	date_layer = text_layer_create(DATE_BOUNDS);
	text_layer_set_text_color(date_layer, TEXT_COLOR);
	text_layer_set_background_color(date_layer, GColorClear);
	text_layer_set_font(date_layer, date_font);
	text_layer_set_text_alignment(date_layer, GTextAlignmentCenter);
	
	// create and layout battery layer
	battery_layer = layer_create(BATTERY_BOUNDS);
	layer_set_update_proc(battery_layer, update_battery);
	//battery_callback(battery_state_service_peek());
	
	// random colors for everybody
	update_colors();
	
	// add date, time and battery layers to window's root layer
	layer_add_child(window_layer, text_layer_get_layer(header_layer));
	layer_add_child(window_layer, text_layer_get_layer(time_layer));
	layer_add_child(window_layer, text_layer_get_layer(date_layer));
	layer_add_child(window_layer, battery_layer);
}

static void unload_main_window(Window* window) {
	gbitmap_destroy(background_bitmap);
	bitmap_layer_destroy(background_layer);
	text_layer_destroy(time_layer);
	text_layer_destroy(date_layer);
	layer_destroy(battery_layer);
	fonts_unload_custom_font(time_font);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
	update_time();
}

static void battery_callback(BatteryChargeState state) {
	// update battery meter
	battery_level = state.charge_percent;
	layer_mark_dirty(battery_layer);
}

int main(void) {
	init();
	app_event_loop();
	finish();
	return 0;
}

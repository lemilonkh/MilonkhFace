#include <pebble.h>
#include "MilonkhFace.h"
#include "Settings.h"

static Window *main_window;
static TextLayer *time_layer, *date_layer, *header_layer;
static Layer *battery_layer;
static BitmapLayer *background_layer;
static GBitmap *background_bitmap;

static int battery_level = 0;
static GColor battery_background_color, battery_color;

static int current_font = -1;
static GFont big_fonts[FONT_COUNT];
static GFont small_fonts[FONT_COUNT];

static char symbols[] = {',','_',':','#','*','/','$','~',';','+','^','?','=','!'};
#define RANDOM_SYMBOL() (symbols[rand() % ARRAY_LENGTH(symbols)])

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
	bool lightOnDark = rand()%2 == 0;
	
	color bg = random_color(!lightOnDark);
	color text = random_color(lightOnDark);
	
	GColor background_color, text_color;
	
	// make sure text is readable
	if(lightOnDark) {
		background_color = make_GColor(mute_max_color(bg, text));
		text_color = make_GColor(text);
	} else {
		text_color = make_GColor(mute_max_color(text, bg));
		background_color = make_GColor(bg);
	}
	
	// make sure the both foreground colors aren't the same
	GColor detail_color;
	do {
		detail_color = make_GColor(random_color(lightOnDark));
	} while(gcolor_equal(text_color, detail_color) || gcolor_equal(background_color, detail_color));
								   
	// update main and time layer background color
	window_set_background_color(main_window, background_color);
	//text_layer_set_background_color(time_layer, time_background_color);
	
	// update all text layer's colors
	text_layer_set_text_color(time_layer, detail_color);
	text_layer_set_text_color(header_layer, detail_color);
	text_layer_set_text_color(date_layer, text_color);
	
	// redraw battery to update its colors
	battery_background_color = detail_color;
	battery_color = text_color;
	battery_callback(battery_state_service_peek());
}

static GColor make_GColor(color col) {
	return GColorFromRGB(col.r, col.g, col.b);
}

static color mute_max_color(color x, color y) {
	color result = x;
	
	// subtract maximal color component of y from x
	if(y.r >= y.g && y.r >= y.g) {
		result.r = 0;
	} else if(y.g >= y.r && y.g >= y.b) {
		result.g = 0;
	} else if(y.b >= y.r && y.b >= y.g) {
		result.b = 0;
	}
	
	return result;
}

static color random_color(bool light) {
	int color_vals[3] = {0,0,0};
	int min = 0, max = 3;
	
	if(light) {
		min = 2;
		max = 3;
	} else {
		min = 0;
		max = 1;
	}
	
	int index1 = rand()%3;
	int index2 = rand()%3;
	color_vals[index1] = random_hex_val(min, max);
	color_vals[index2] = random_hex_val(min, max);
	
	color rand_col = {color_vals[0], color_vals[1], color_vals[2]};
	return rand_col;
}

// generates one of the following values: 0x00, 0x55, 0xAA, 0xFF
static int random_hex_val(int min, int max) {
	int val = ((rand() % (max-1))+min) * 5;
	return 16*val + val;
}

static void update_time() {
	update_colors();
	update_fonts();
	
	// get the time
	time_t temp = time(NULL);
	struct tm *tick_time = localtime(&temp);

	// write time into string
	static char time_buffer[8];
	static char* time_placeholder;
	if(clock_is_24h_style())
		time_placeholder = "%H:%M";
	else
		time_placeholder = "%I:%M";
	
	// get random symbol
	char symbol = RANDOM_SYMBOL();
	time_placeholder[2] = symbol;
	
	strftime(time_buffer, sizeof(time_buffer), time_placeholder, tick_time);
	
	// write date into string
	static char date_buffer[8];
	static char date_placeholder[9] = "/%d/%m/";
	date_placeholder[0] = RANDOM_SYMBOL();
	date_placeholder[3] = RANDOM_SYMBOL();
	date_placeholder[6] = RANDOM_SYMBOL();
	
	strftime(date_buffer, sizeof(date_buffer), date_placeholder, tick_time);

	// display it
	text_layer_set_text(time_layer, time_buffer);
	text_layer_set_text(date_layer, date_buffer);
}

static void update_battery(Layer *layer, GContext *ctx) {
	GRect bounds = layer_get_bounds(layer);

	// determine bar width
	int width = (int)(float)(((float)battery_level / 100.0f) * 114.0f);

	// draw background
	graphics_context_set_fill_color(ctx, battery_background_color);
	graphics_fill_rect(ctx, bounds, 0, BATTERY_CORNERS);

	// draw bar
	graphics_context_set_fill_color(ctx, battery_color);
	graphics_fill_rect(ctx, GRect(0, 0, width, bounds.size.h), 0, BATTERY_CORNERS);
}

static void update_fonts() {
	if(++current_font >= FONT_COUNT)
		current_font = 0;
	
	text_layer_set_font(header_layer, small_fonts[current_font]);
	text_layer_set_font(time_layer, big_fonts[current_font]);
	text_layer_set_font(date_layer, small_fonts[current_font]);
}

static void load_main_window(Window *window) {
	// window information
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);

	// get fonts
	for(int f = 0; f < FONT_COUNT; f++) {
		big_fonts[f] = fonts_load_custom_font(resource_get_handle(BIG_FONTS[f]));
		small_fonts[f] = fonts_load_custom_font(resource_get_handle(SMALL_FONTS[f]));
	}
	
	// create and layout header layer
	header_layer = text_layer_create(HEADER_BOUNDS);
	text_layer_set_background_color(header_layer, GColorClear);
	text_layer_set_text(header_layer, HEADER_TEXT);
	text_layer_set_text_alignment(header_layer, GTextAlignmentCenter); 
	
	// create and layout time layer
	time_layer = text_layer_create(
		GRect(TIME_X, PBL_IF_ROUND_ELSE(TIME_Y_ROUND, TIME_Y_NORMAL), bounds.size.w, TIME_HEIGHT));
	text_layer_set_background_color(time_layer, GColorClear);
	text_layer_set_text(time_layer, "13/37");
	text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
	
	// create and layout date layer
	date_layer = text_layer_create(DATE_BOUNDS);
	text_layer_set_background_color(date_layer, GColorClear);
	text_layer_set_text_alignment(date_layer, GTextAlignmentCenter);
	
	// create and layout battery layer
	battery_layer = layer_create(BATTERY_BOUNDS);
	layer_set_update_proc(battery_layer, update_battery);
	//battery_callback(battery_state_service_peek());
	
	// random colors and fonts for everybody
	update_colors();
	update_fonts();
	
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
	
	for(int f = 0; f < FONT_COUNT; f++) {
		fonts_unload_custom_font(big_fonts[f]);
		fonts_unload_custom_font(small_fonts[f]);
	}
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
	update_time();
}

static void battery_callback(BatteryChargeState state) {
	battery_level = state.charge_percent;
	layer_mark_dirty(battery_layer);
}

int main(void) {
	init();
	app_event_loop();
	finish();
	return 0;
}

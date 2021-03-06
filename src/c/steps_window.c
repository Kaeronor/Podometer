#include <pebble.h>
#include "steps_window.h"
#include "distance_window.h"
#include "menu_window.h"

static Window *s_window;
static GFont s_res_gothic_28_bold;
static GFont s_res_gothic_24_bold;
static Layer *s_window_layer;
static Layer *s_progress_layer;
static GBitmap *s_res_image;
static BitmapLayer *s_bitmaplayer_1;
static TextLayer *s_textlayer_2;
static TextLayer *s_textlayer_3;
StatusBarLayer *status_bar_layer;

// Global variables for the person's infos, declared in main.c
extern int steps;
extern int goal;

// Local variables for text displaying
static char buffer1[16];
static char buffer2[16];
static char finalbuffer[25];

// Action to do when Select Button is clicked
static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
	show_menu_window();
}

// Action to do when Down Button is clicked
static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
	hide_steps_window();
	show_distance_window();
}

// Enable button's interactions
static void click_config_provider(void *context) {
	window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
	window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

// Display numbers in a more pleasant form (eg: 10'000)
static void display_value(uint16_t value, char buffer[]) {
	int thousands = value / 1000;
	int hundreds = value % 1000;

	if (thousands > 0) {
		snprintf(buffer, sizeof(buffer1), "%d'%03d", thousands, hundreds);
	}
	else {
		snprintf(buffer, sizeof(buffer1), "%d", hundreds);
	}
}

// Update routine for the progress bar
static void progress_layer_update_proc(Layer *layer, GContext *ctx) {
	GRect inset;

	// If the current goal is overpassed, the progress bar continues with another color
	if (steps > goal) {
		// progress bar Gray (first turn)
		inset = grect_inset(layer_get_bounds(layer), GEdgeInsets(16));
		graphics_context_set_fill_color(ctx, GColorDarkGray);
		graphics_fill_radial(ctx, inset, GOvalScaleModeFitCircle, 8, DEG_TO_TRIGANGLE(0), DEG_TO_TRIGANGLE(360));

		// progress bar White (second turn)
		inset = grect_inset(layer_get_bounds(layer), GEdgeInsets(16));
		graphics_context_set_fill_color(ctx, GColorWhite);
		graphics_fill_radial(ctx, inset, GOvalScaleModeFitCircle, 8, DEG_TO_TRIGANGLE(0), DEG_TO_TRIGANGLE(360 * (steps - goal) / goal));

		// external circle
		inset = grect_inset(layer_get_bounds(layer), GEdgeInsets(16));
		graphics_context_set_fill_color(ctx, GColorWhite);
		graphics_fill_radial(ctx, inset, GOvalScaleModeFitCircle, 1, DEG_TO_TRIGANGLE(0), DEG_TO_TRIGANGLE(360));

		// internal circle
		inset = grect_inset(layer_get_bounds(layer), GEdgeInsets(24));
		graphics_context_set_fill_color(ctx, GColorWhite);
		graphics_fill_radial(ctx, inset, GOvalScaleModeFitCircle, 1, DEG_TO_TRIGANGLE(0), DEG_TO_TRIGANGLE(360));

		// start marker
		inset = grect_inset(layer_get_bounds(layer), GEdgeInsets(14));
		graphics_context_set_fill_color(ctx, GColorWhite);
		int trigangle = DEG_TO_TRIGANGLE(0);
		int line_width_trigangle = 560;
		graphics_fill_radial(ctx, inset, GOvalScaleModeFitCircle, 14, trigangle - line_width_trigangle, trigangle);

		// end marker
		inset = grect_inset(layer_get_bounds(layer), GEdgeInsets(17));
		graphics_context_set_fill_color(ctx, GColorBlack);
		trigangle = DEG_TO_TRIGANGLE(360 * (steps - goal) / goal);
		line_width_trigangle = 400;
		graphics_fill_radial(ctx, inset, GOvalScaleModeFitCircle, 7, trigangle - line_width_trigangle, trigangle);

	}
	else { // Or display the simple progress bar toward goal
		// progress bar
		inset = grect_inset(layer_get_bounds(layer), GEdgeInsets(16));
		graphics_context_set_fill_color(ctx, GColorDarkGray);
		graphics_fill_radial(ctx, inset, GOvalScaleModeFitCircle, 8, DEG_TO_TRIGANGLE(0), DEG_TO_TRIGANGLE(360 * steps / goal));

		// external circle
		inset = grect_inset(layer_get_bounds(layer), GEdgeInsets(16));
		graphics_context_set_fill_color(ctx, GColorWhite);
		graphics_fill_radial(ctx, inset, GOvalScaleModeFitCircle, 1, DEG_TO_TRIGANGLE(0), DEG_TO_TRIGANGLE(360));

		// internal circle
		inset = grect_inset(layer_get_bounds(layer), GEdgeInsets(24));
		graphics_context_set_fill_color(ctx, GColorWhite);
		graphics_fill_radial(ctx, inset, GOvalScaleModeFitCircle, 1, DEG_TO_TRIGANGLE(0), DEG_TO_TRIGANGLE(360));

		// start marker
		inset = grect_inset(layer_get_bounds(layer), GEdgeInsets(14));
		graphics_context_set_fill_color(ctx, GColorWhite);
		int trigangle = DEG_TO_TRIGANGLE(0);
		int line_width_trigangle = 560;
		graphics_fill_radial(ctx, inset, GOvalScaleModeFitCircle, 14, trigangle - line_width_trigangle, trigangle);

		// end marker
		inset = grect_inset(layer_get_bounds(layer), GEdgeInsets(16));
		graphics_context_set_fill_color(ctx, GColorWhite);
		trigangle = DEG_TO_TRIGANGLE(360 * steps / goal);
		line_width_trigangle = 400;
		graphics_fill_radial(ctx, inset, GOvalScaleModeFitCircle, 8, trigangle - line_width_trigangle, trigangle);
	}

	// Update the text layers (steps and goal)
	display_value(steps, buffer1);
	text_layer_set_text(s_textlayer_2, buffer1);

	display_value(goal, buffer2);
	snprintf(finalbuffer, sizeof(finalbuffer), "%s %s", "Current goal:", buffer2);
	text_layer_set_text(s_textlayer_3, finalbuffer);
}

// Init function called when the window is created
static void initialise_ui(void) {
	s_window = window_create();
	s_window_layer = window_get_root_layer(s_window);
	window_set_click_config_provider(s_window, click_config_provider);    // buttons interactions
	window_set_background_color(s_window, GColorBlack);
#ifndef PBL_SDK_3
	window_set_fullscreen(s_window, false);
#endif

	s_res_gothic_28_bold = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD);

	// status bar
	status_bar_layer = status_bar_layer_create();
	status_bar_layer_set_colors(status_bar_layer, GColorBlack, GColorWhite);
	status_bar_layer_set_separator_mode(status_bar_layer, StatusBarLayerSeparatorModeDotted);
	layer_add_child(s_window_layer, (Layer *)status_bar_layer);

  // footprint image
  s_res_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_FOOTPRINT_36);
  s_bitmaplayer_1 = bitmap_layer_create(GRect(53, 84, 36, 36));
  bitmap_layer_set_bitmap(s_bitmaplayer_1, s_res_image);
  layer_add_child(s_window_layer, (Layer *)s_bitmaplayer_1);
  
	// progress bar
	s_progress_layer = layer_create(GRect(0, 7, 144, 150));
	layer_set_update_proc(s_progress_layer, progress_layer_update_proc);
	layer_add_child(s_window_layer, (Layer *)s_progress_layer);

	// steps counter
	s_textlayer_2 = text_layer_create(GRect(41, 48, 62, 30));
	text_layer_set_background_color(s_textlayer_2, GColorBlack);
	text_layer_set_text_color(s_textlayer_2, GColorWhite);
	text_layer_set_font(s_textlayer_2, s_res_gothic_28_bold);
	text_layer_set_text_alignment(s_textlayer_2, GTextAlignmentCenter);
	layer_add_child(s_window_layer, (Layer *)s_textlayer_2);

	// current goal
	s_textlayer_3 = text_layer_create(GRect(0, 143, 144, 20));
	text_layer_set_background_color(s_textlayer_3, GColorBlack);
	text_layer_set_text_color(s_textlayer_3, GColorWhite);
	text_layer_set_text_alignment(s_textlayer_3, GTextAlignmentCenter);
	text_layer_set_font(s_textlayer_3, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_3);
}

// Deinit function called when the window is destroyed
static void destroy_ui(void) {
	window_destroy(s_window);
	status_bar_layer_destroy(status_bar_layer);
	layer_destroy(s_progress_layer);
	text_layer_destroy(s_textlayer_2);
	text_layer_destroy(s_textlayer_3);
}

static void handle_window_unload(Window* window) {
	destroy_ui();
}

// Display the steps window on the Pebble
void show_steps_window(void) {
	initialise_ui();
	window_set_window_handlers(s_window, (WindowHandlers) {
		.unload = handle_window_unload,
	});
	window_stack_push(s_window, true);
}

// Ask for an update of the steps window to display current values
void update_steps_window(void) {
	if (s_window)
		layer_mark_dirty(s_progress_layer);
}

// Hide the steps window, so that another window can be displayed
void hide_steps_window(void) {
	window_stack_remove(s_window, true);
}

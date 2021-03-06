#include <pebble.h>
#include "goal_window.h"
#include "layers_selection.h"

/* THIS FILE IS GREATLY INSPIRED OF THE CODE IN THE FOLLOWING LINK
http://www.mediafire.com/file/btramcjbtnq1a9w/pinentrytestmodification.zip  */

// Global variables for the person's infos, declared in main.c
extern int goal;

static char* selection_handle_get_text(int index, void *context) {
	GoalWindow *goal_window = (GoalWindow*)context;
	snprintf(
		goal_window->field_buffs[index],
		sizeof(goal_window->field_buffs[0]), "%d", (int)goal_window->goals.digits[index]);
	return goal_window->field_buffs[index];
}

static void selection_handle_complete(void *context) {
	GoalWindow *goal_window = (GoalWindow*)context;
	goal_window->callbacks.goal_complete(goal_window->goals, goal_window);
}

// Goal incementing
static void selection_handle_inc(int index, uint8_t clicks, void *context) {
	GoalWindow *goal_window = (GoalWindow*)context;
	goal_window->goals.digits[index] = goal_window->goals.digits[index] + 100;
	if (goal_window->goals.digits[index] > MAX_GOAL_VALUE)
	{
		goal_window->goals.digits[index] = 100;
	}
}

// Goal decrementing
static void selection_handle_dec(int index, uint8_t clicks, void *context) {
	GoalWindow *goal_window = (GoalWindow*)context;
	goal_window->goals.digits[index] = goal_window->goals.digits[index] - 100;
	if (goal_window->goals.digits[index] < 100)
	{
		goal_window->goals.digits[index] = MAX_GOAL_VALUE;
	}
}

// Goal window creation
GoalWindow* goal_window_create(GoalWindowCallbacks callbacks) {
	GoalWindow *goal_window = (GoalWindow*)malloc(sizeof(GoalWindow));
	if (goal_window) {
		goal_window->window = window_create();
		goal_window->callbacks = callbacks;
		if (goal_window->window) {
			goal_window->field_selection = 0;
			goal_window->goals.digits[0] = goal;


			// Get window parameters
			Layer *window_layer = window_get_root_layer(goal_window->window);
			GRect bounds = layer_get_bounds(window_layer);

			// Main TextLayer
			goal_window->main_text = text_layer_create(GRect(0, 30, bounds.size.w, 40));
			text_layer_set_text(goal_window->main_text, "Daily goal");
			text_layer_set_font(goal_window->main_text, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
			text_layer_set_text_alignment(goal_window->main_text, GTextAlignmentCenter);
			layer_add_child(window_layer, text_layer_get_layer(goal_window->main_text));

			// Sub TextLayer
			goal_window->sub_text = text_layer_create(GRect(1, 125, bounds.size.w, 40));
			text_layer_set_text(goal_window->sub_text, "steps");
			text_layer_set_text_alignment(goal_window->sub_text, GTextAlignmentCenter);
			text_layer_set_font(goal_window->sub_text, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
			layer_add_child(window_layer, text_layer_get_layer(goal_window->sub_text));

			// Create selection layer
			goal_window->selection = selection_layer_create(GRect(10, 75, 80, 30), NUM_CELLS);
			selection_layer_set_cell_width(goal_window->selection, 0, 120);
			selection_layer_set_cell_padding(goal_window->selection, 10);
			selection_layer_set_active_bg_color(goal_window->selection, GColorDarkGray);
			selection_layer_set_click_config_onto_window(goal_window->selection, goal_window->window);

      // Set callbacks
			selection_layer_set_callbacks(goal_window->selection, goal_window, (SelectionLayerCallbacks)
			{
				.get_cell_text = selection_handle_get_text,
				.complete = selection_handle_complete,
				.increment = selection_handle_inc,
				.decrement = selection_handle_dec,
			});
			
      // Add to window
      layer_add_child(window_get_root_layer(goal_window->window), goal_window->selection);
			return goal_window;
		}
	}

	return NULL;
}

// Goal window destruction
void goal_window_destroy(GoalWindow *goal_window) {
	if (goal_window)
	{
		selection_layer_destroy(goal_window->selection);
		text_layer_destroy(goal_window->sub_text);
		text_layer_destroy(goal_window->main_text);
		free(goal_window);
		goal_window = NULL;
		return;
	}
}

// Display the goal window
void goal_window_push(GoalWindow *goal_window, bool animated)
{  window_stack_push(goal_window->window, animated);  }

// Remove the goal window
void goal_window_pop(GoalWindow *goal_window, bool animated)
{  window_stack_remove(goal_window->window, animated);  }

// Gets whether it is the topmost window or not
bool goal_window_get_topmost_window(GoalWindow *goal_window)
{  return window_stack_get_top_window() == goal_window->window;  }

// Sets the over-all color scheme of the window
void goal_window_set_highlight_color(GoalWindow *goal_window, GColor color) {
	goal_window->highlight_color = color;
	selection_layer_set_active_bg_color(goal_window->selection, color);
}

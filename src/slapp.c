#include <pebble.h>

#define SLAPS 0
#define TIME 0

static Window *s_main_window;
static TextLayer *s_output_layer;
int previousSlapCount = 0;
time_t lastSlapTime = 0;

static void worker_message_handler(uint16_t type, AppWorkerMessage *data) {
  if (type == SLAPS) {
    int slaps = data->data0;
    lastSlapTime = data->data1;
    DictionaryIterator *iterator;
    app_message_outbox_begin(&iterator);
    int key = 0;
    int value = (long int) lastSlapTime;
    dict_write_int(iterator, key, &value, sizeof(int), true );
    if(slaps > previousSlapCount) {
      previousSlapCount = slaps;
      app_message_outbox_send();
      vibes_short_pulse();
    }
  }
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  // Check to see if the worker is currently active
  bool running = app_worker_is_running();

  // Toggle running state
  AppWorkerResult result;
  if (running) {
    result = app_worker_kill();

    if (result == APP_WORKER_RESULT_SUCCESS) {
      text_layer_set_text(s_output_layer, "\n\nSlapp stopped!");
    } else {
      text_layer_set_text(s_output_layer, "\n\nError killing worker!");
    }
  } else {
    result = app_worker_launch();

    if (result == APP_WORKER_RESULT_SUCCESS) {
      text_layer_set_text(s_output_layer, "\n\nSlapp started!\nPress select to stop");
    } else {
      text_layer_set_text(s_output_layer, "\n\nError starting Slapp!");
    }
  }

  APP_LOG(APP_LOG_LEVEL_INFO, "Result: %d", result);
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);

  // Create output TextLayer
  s_output_layer = text_layer_create(GRect(5, 0, window_bounds.size.w - 10, window_bounds.size.h));
  text_layer_set_font(s_output_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_text(s_output_layer, "\n\nPress Select To Start");
  text_layer_set_overflow_mode(s_output_layer, GTextOverflowModeWordWrap);
  text_layer_set_text_alignment(s_output_layer, GTextAlignmentCenter);
  
  layer_add_child(window_layer, text_layer_get_layer(s_output_layer));
}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_output_layer);
}

static void init() {
  // Create main Window
  s_main_window = window_create();  
  window_set_click_config_provider(s_main_window, click_config_provider);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  window_stack_push(s_main_window, true);
  
  // Subscribe to Worker messages
  app_worker_message_subscribe(worker_message_handler);
}

static void deinit() {
  window_destroy(s_main_window);
  app_worker_message_unsubscribe();
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
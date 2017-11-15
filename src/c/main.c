#include <pebble.h>

static Window *s_main_window;
static Layer *s_main_layer;
static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;

static GPath *s_sec_hand_path_ptr = NULL;
static GPath *s_min_hand_path_ptr = NULL;
static GPath *s_hour_hand_path_ptr = NULL;

#ifdef PBL_COLOR
  #define SEC_HAND_COLOR         GColorRed
  #define MINHOUR_HAND_COLOR     GColorBlack
#else
  #define SEC_HAND_COLOR         GColorBlack
  #define MINHOUR_HAND_COLOR     GColorBlack
#endif

static const GPathInfo SEC_HAND_PATH = {
  2,
  (GPoint []) {
    {0, -4}, {0, -79},
  }
};

static const GPathInfo MIN_HAND_PATH = {
  7,
  (GPoint []) {
    {-1,   0},
    {-1, -13},
    {-5, -50},
    {-0, -70},
    { 5, -50},
    { 1, -13},
    { 1,   0},
  }
};

static const GPathInfo HOUR_HAND_PATH = {
  7,
  (GPoint []) {
    {-3,   0},
    {-3, -13},
    {-7, -42},
    {-0, -50},
    { 7, -42},
    { 3, -13},
    { 3,   0},
  }
};

static void update_display(Layer *s_main_layer, GContext* ctx) {
  // Fill the path second hand:
  graphics_context_set_fill_color(ctx, MINHOUR_HAND_COLOR);
  gpath_draw_filled(ctx, s_hour_hand_path_ptr);
  gpath_draw_filled(ctx, s_min_hand_path_ptr);
  
  // Fill the path second hand:
  graphics_context_set_stroke_color(ctx, SEC_HAND_COLOR);
  gpath_draw_outline(ctx, s_sec_hand_path_ptr);
}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // update seconds
  gpath_rotate_to(s_sec_hand_path_ptr,  (tick_time->tm_sec * TRIG_MAX_ANGLE / 60));
  // update minutes
  gpath_rotate_to(s_min_hand_path_ptr,  ((tick_time->tm_min + tick_time->tm_sec/60.0) * TRIG_MAX_ANGLE / 60));
  // update hours
  gpath_rotate_to(s_hour_hand_path_ptr, ((tick_time->tm_hour + tick_time->tm_min/60.0) * TRIG_MAX_ANGLE / 12));

  // update display
  layer_mark_dirty(s_main_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "update time %d:%02d:%02d %d", tick_time->tm_hour, tick_time->tm_min, tick_time->tm_sec, tick_time->tm_mday);
  // update_time();

  // update seconds
  gpath_rotate_to(s_sec_hand_path_ptr,  (tick_time->tm_sec * TRIG_MAX_ANGLE / 60));
  // update minutes
  gpath_rotate_to(s_min_hand_path_ptr,  ((tick_time->tm_min + tick_time->tm_sec/60.0) * TRIG_MAX_ANGLE / 60));
  // update hours
  gpath_rotate_to(s_hour_hand_path_ptr, ((tick_time->tm_hour + tick_time->tm_min/60.0) * TRIG_MAX_ANGLE / 12));

  // update display
  layer_mark_dirty(s_main_layer);
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(s_main_window);
  GRect bounds = layer_get_bounds(window_layer);

  APP_LOG(APP_LOG_LEVEL_DEBUG, "window load");

  // Create GBitmap, then set to created BitmapLayer
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ZIFFERBLATT);
  s_background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_background_layer));

  // create display canvas
  s_main_layer = layer_create(bounds);
  
  // register callback to execute when layer ist marked dirty
  layer_set_update_proc(s_main_layer, update_display);
  layer_add_child(window_get_root_layer(window), s_main_layer);
  
  // setup watch hands
  // create watch hands with paths
  s_sec_hand_path_ptr = gpath_create(&SEC_HAND_PATH);
  s_min_hand_path_ptr = gpath_create(&MIN_HAND_PATH);
  s_hour_hand_path_ptr = gpath_create(&HOUR_HAND_PATH);
  
  // translate watch hands to middle of the screen
  gpath_move_to(s_sec_hand_path_ptr, GPoint(71,83));
  gpath_move_to(s_min_hand_path_ptr, GPoint(71,83));
  gpath_move_to(s_hour_hand_path_ptr, GPoint(71,83));
}

static void main_window_unload(Window *window) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "window unload");

  // destroy background
  gbitmap_destroy(s_background_bitmap);
  bitmap_layer_destroy(s_background_layer);
}

static void init() {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "app init");
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);

  // Make sure the time is displayed from the start
  update_time();

  // Register with TickTimerService
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
}

static void deinit() {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "app deinit");
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
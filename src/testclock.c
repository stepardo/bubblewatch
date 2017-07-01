#include <pebble.h>
#include <sys/time.h>
#include <math.h>

#define SETTINGS_KEY 1

typedef struct ClayConfig
{
  bool inverted;
  bool show_bubbles;
} __attribute__((__packed__)) ClayConfig;

ClayConfig settings;

static Window *s_window = NULL;
static Layer *digital_layer = NULL;
static Layer *clock_layer = NULL;
static TextLayer *inner_clock_text_layer = NULL;
static TextLayer *digital_layer_text_layer = NULL;
static unsigned hour = 0, minute = 0, second = 0;
static char digital_layer_text_buffer[22];
static short initialized = 0;
static int battery_level = 0;
static HealthValue heart_rate = -1;


static void battery_callback(BatteryChargeState state) {
  // Record the new battery level
  battery_level = state.charge_percent;
}

#define FOREGROUND \
  settings.inverted ? GColorBlack : GColorWhite

#define BACKGROUND \
  settings.inverted ? GColorWhite : GColorBlack

static void tick_handler(struct tm *tick_time, TimeUnits units_changed)
{
  // this only updates internal time
  // we will then mark all relevant layers dirty

  hour = tick_time->tm_hour;
  minute = tick_time->tm_min;
  second = tick_time->tm_sec;

  layer_mark_dirty(text_layer_get_layer(inner_clock_text_layer));
  if (!initialized || second == 0)
    {
      strftime(digital_layer_text_buffer, sizeof(digital_layer_text_buffer), "%H:%M  %d.%m.%Y", tick_time);
      text_layer_set_text(digital_layer_text_layer, digital_layer_text_buffer);
      initialized = 1;
    }
}

void digital_layer_update_callback(Layer *me, GContext *ctx)
{
  enum
  {
    WIDTH = 2,
  };

  GRect b = layer_get_bounds(me);
  GRect b1 = GRect(WIDTH, WIDTH, b.size.w - WIDTH * 2, b.size.h - WIDTH * 2);
  GRect b2 = GRect(WIDTH * 2, WIDTH * 2, b1.size.w - WIDTH * 2, b1.size.h - WIDTH * 2);

  graphics_context_set_fill_color(ctx, BACKGROUND);
  graphics_fill_rect(ctx, b, 3, 0);
  graphics_context_set_fill_color(ctx, FOREGROUND);
  graphics_fill_rect(ctx, b1, 3, GCornersAll);
  graphics_context_set_fill_color(ctx, BACKGROUND);
  graphics_fill_rect(ctx, b2, 3, GCornersAll);

  text_layer_set_text_color(digital_layer_text_layer, FOREGROUND);
}

void inner_clock_text_layer_update_callback(Layer *me, GContext *ctx)
{
  GRect bounds = layer_get_bounds(me);

  int32_t hour_angle   = (TRIG_MAX_ANGLE * (((hour % 12) * 6) + (minute / 10))) / (12 * 6);
  int32_t minute_angle = TRIG_MAX_ANGLE * minute / 60;
  int32_t second_angle = TRIG_MAX_ANGLE * second / 60;

  GPoint center = grect_center_point(&bounds);

  graphics_context_set_text_color(ctx, FOREGROUND);
  graphics_context_set_antialiased(ctx, true);

#define min(a, b) ( a < b ? a : b )

  int32_t second_hand_length = 140;
  int32_t minute_hand_length = min(bounds.size.h, bounds.size.w)/2;
  int32_t hour_hand_length   = min(bounds.size.h, bounds.size.w)/4;

  int32_t second_y = (-cos_lookup(second_angle) * second_hand_length / TRIG_MAX_RATIO) + center.y;
  int32_t second_x = (sin_lookup(second_angle) * second_hand_length / TRIG_MAX_RATIO) + center.x;

  int32_t minute_y = (-cos_lookup(minute_angle) * minute_hand_length / TRIG_MAX_RATIO) + center.y;
  int32_t minute_x = (sin_lookup(minute_angle) * minute_hand_length / TRIG_MAX_RATIO) + center.x;

  int32_t hour_y = (-cos_lookup(hour_angle) * hour_hand_length / TRIG_MAX_RATIO) + center.y;
  int32_t hour_x = (sin_lookup(hour_angle) * hour_hand_length / TRIG_MAX_RATIO) + center.x;

  graphics_context_set_stroke_color(ctx, FOREGROUND);
  graphics_context_set_stroke_width(ctx, 5);
  graphics_draw_line(ctx, center, GPoint(minute_x, minute_y));
  graphics_context_set_stroke_width(ctx, 6);
  graphics_draw_line(ctx, center, GPoint(hour_x, hour_y));
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_line(ctx, center, GPoint(second_x, second_y));

  graphics_context_set_antialiased(ctx, false);


  if (settings.show_bubbles)
    {
      graphics_context_set_stroke_width(ctx, 1);
      graphics_draw_circle(ctx, center, 8);
      graphics_context_set_fill_color(ctx, BACKGROUND);
      graphics_fill_circle(ctx, center, 7);

      char buf[3];
      snprintf(buf, 3, "%0d", hour);
      buf[2] = '\0';
      graphics_draw_text(ctx,
                         buf,
                         fonts_get_system_font(FONT_KEY_GOTHIC_14),
                         GRect(center.x - 9, center.y - 9, 18, 18),
                         GTextOverflowModeFill, GTextAlignmentCenter, 0);

      int32_t length = 22;
      int32_t minute_a_y = (-cos_lookup(minute_angle) * length / TRIG_MAX_RATIO) + center.y;
      int32_t minute_a_x = (sin_lookup(minute_angle) * length / TRIG_MAX_RATIO) + center.x;
      graphics_context_set_stroke_width(ctx, 1);
      graphics_draw_circle(ctx, GPoint(minute_a_x, minute_a_y), 8);
      graphics_context_set_fill_color(ctx, BACKGROUND);
      graphics_fill_circle(ctx, GPoint(minute_a_x, minute_a_y), 7);

      snprintf(buf, 3, "%0d", minute);
      buf[2] = '\0';
      graphics_draw_text(ctx,
                         buf,
                         fonts_get_system_font(FONT_KEY_GOTHIC_14),
                         GRect(minute_a_x - 9, minute_a_y - 9, 18, 18),
                         GTextOverflowModeFill, GTextAlignmentCenter, 0);

      length = 44;//min(bounds.size.w, bounds.size.h)/2 - 14;
      int32_t second_a_y = (-cos_lookup(second_angle) * length / TRIG_MAX_RATIO) + center.y;
      int32_t second_a_x = (sin_lookup(second_angle) * length / TRIG_MAX_RATIO) + center.x;

      graphics_context_set_stroke_width(ctx, 1);
      graphics_draw_circle(ctx, GPoint(second_a_x, second_a_y), 8);
      graphics_context_set_fill_color(ctx, BACKGROUND);
      graphics_fill_circle(ctx, GPoint(second_a_x, second_a_y), 7);

      snprintf(buf, 3, "%0d", second);
      buf[2] = '\0';
      graphics_draw_text(ctx,
                         buf,
                         fonts_get_system_font(FONT_KEY_GOTHIC_14),
                         GRect(second_a_x - 9, second_a_y - 9, 18, 18),
                         GTextOverflowModeFill, GTextAlignmentCenter, 0);
    }
#if defined(PBL_HEALTH)
  if (heart_rate != -1)
    {
      graphics_context_set_text_color(ctx, BACKGROUND);
      char buf[10];
      snprintf(buf, 10, "BPM: %d", (int)heart_rate);
#define X 2
#define Y bounds.size.h - 16
#define W bounds.size.w
#define H bounds.size.h
      graphics_draw_text(ctx, buf,
                         fonts_get_system_font(FONT_KEY_GOTHIC_14),
                         GRect(X - 1, Y - 1, W, H),
                         GTextOverflowModeFill, GTextAlignmentLeft, 0);
      graphics_draw_text(ctx, buf,
                         fonts_get_system_font(FONT_KEY_GOTHIC_14),
                         GRect(X + 1, Y - 1, W, H),
                         GTextOverflowModeFill, GTextAlignmentLeft, 0);
      graphics_draw_text(ctx, buf,
                         fonts_get_system_font(FONT_KEY_GOTHIC_14),
                         GRect(X - 1, Y + 1, W, H),
                         GTextOverflowModeFill, GTextAlignmentLeft, 0);
      graphics_draw_text(ctx, buf,
                         fonts_get_system_font(FONT_KEY_GOTHIC_14),
                         GRect(X + 1, Y + 1, W, H),
                         GTextOverflowModeFill, GTextAlignmentLeft, 0);
      graphics_context_set_text_color(ctx, FOREGROUND);
      graphics_draw_text(ctx, buf,
                         fonts_get_system_font(FONT_KEY_GOTHIC_14),
                         GRect(X, Y, W, H),
                         GTextOverflowModeFill, GTextAlignmentLeft, 0);
#undef X
#undef Y
#undef W
#undef H
    }
#endif
}

void clock_layer_update_callback(Layer *me, GContext *ctx)
{
  GRect bounds = layer_get_bounds(me);

  graphics_context_set_fill_color(ctx, BACKGROUND);
  graphics_fill_rect(ctx, bounds, 5, 0);
  graphics_context_set_fill_color(ctx, FOREGROUND);
  graphics_fill_rect(ctx, GRect(2, 2, bounds.size.w - 4, bounds.size.h - 4), 5, GCornersAll);
  graphics_context_set_fill_color(ctx, BACKGROUND);
  graphics_fill_rect(ctx, GRect(4, 4, bounds.size.w - 8, bounds.size.h - 8), 5, GCornersAll);

  graphics_context_set_stroke_color(ctx, FOREGROUND);
  graphics_context_set_stroke_width(ctx, 3);
  graphics_context_set_fill_color(ctx, FOREGROUND);

  // draw quarter markers
  enum
  {
    LENGTH = 6,
  };
  graphics_draw_line(ctx, GPoint(bounds.size.w/2, 0), GPoint(bounds.size.w/2, LENGTH));
  graphics_draw_line(ctx, GPoint(bounds.size.w/2, bounds.size.h - LENGTH), GPoint(bounds.size.w/2, bounds.size.h));
  graphics_draw_line(ctx, GPoint(0, bounds.size.h/2), GPoint(LENGTH, bounds.size.h/2));
  graphics_draw_line(ctx, GPoint(bounds.size.w - LENGTH, bounds.size.h/2), GPoint(bounds.size.w, bounds.size.h/2));
}

#if defined(PBL_HEALTH)
static void health_handler(HealthEventType type, void *context)
{
  APP_LOG(APP_LOG_LEVEL_ERROR, "health_handler");
  if (type == HealthEventHeartRateUpdate || type == HealthEventSignificantUpdate)
    heart_rate = health_service_peek_current_value(HealthMetricHeartRateBPM);

  APP_LOG(APP_LOG_LEVEL_ERROR, "heart_rate = %d, type = %x", (int)heart_rate, type);
  // update display when next timer event hits
}
#endif

static void prv_window_load(Window *window)
{
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  enum
  {
    DIGITAL_HEIGHT = 25,
  };

  digital_layer = layer_create(GRect(0, 0, bounds.size.w, DIGITAL_HEIGHT));
  layer_add_child(window_layer, digital_layer);
  layer_set_update_proc(digital_layer, digital_layer_update_callback);

  GRect digital_layer_bounds = layer_get_bounds(digital_layer);
  digital_layer_text_layer = text_layer_create(GRect(0, 0, digital_layer_bounds.size.w, digital_layer_bounds.size.h));
  text_layer_set_text_alignment(digital_layer_text_layer, GTextAlignmentCenter);
  text_layer_set_font(digital_layer_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_background_color(digital_layer_text_layer, GColorClear);
  layer_add_child(digital_layer, text_layer_get_layer(digital_layer_text_layer));

  clock_layer = layer_create(GRect(0, DIGITAL_HEIGHT, bounds.size.w, bounds.size.h - DIGITAL_HEIGHT));
  layer_add_child(window_layer, clock_layer);
  layer_set_update_proc(clock_layer, clock_layer_update_callback);

  inner_clock_text_layer = text_layer_create(GRect(8, 8, bounds.size.w - 16, bounds.size.h - DIGITAL_HEIGHT - 16));
  text_layer_set_background_color(inner_clock_text_layer, GColorClear);
  layer_add_child(clock_layer, text_layer_get_layer(inner_clock_text_layer));
  layer_set_update_proc(text_layer_get_layer(inner_clock_text_layer), inner_clock_text_layer_update_callback);

  time_t timeNow = time(NULL);
  struct tm *pLocalTime = localtime(&timeNow); // returns a pointer to static
                                               // data. cannot free

  // init time once on startup, don't wait for event
  tick_handler(pLocalTime, MINUTE_UNIT);

  // register for timer events
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);

  // also bet battery events
  battery_state_service_subscribe(battery_callback);

  // heart rate is only available if pebble health is available
  // don't include this code for other platforms
#if defined(PBL_HEALTH)
  if (!health_service_events_subscribe(health_handler, NULL))
    APP_LOG(APP_LOG_LEVEL_ERROR, "subscribing health handler did not work");
#endif
}

static void prv_window_unload(Window *window) {
  text_layer_destroy(digital_layer_text_layer);
  layer_destroy(text_layer_get_layer(inner_clock_text_layer));
  layer_destroy(clock_layer);
  layer_destroy(digital_layer);
}

static void load_settings()
{
  settings.inverted = false;
  settings.show_bubbles = true;
  persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
}

static void save_settings()
{
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
  layer_mark_dirty(clock_layer);
  layer_mark_dirty(text_layer_get_layer(inner_clock_text_layer));
}

static void app_message_received(DictionaryIterator *it, void *context)
{
  Tuple *t = dict_find(it, MESSAGE_KEY_ConfigInverted);
  if (t)
    settings.inverted = t->value->int32 == 1;

  t = dict_find(it, MESSAGE_KEY_ConfigShowBubbles);
  if (t)
    settings.show_bubbles = t->value->int32 == 1;

  save_settings();
}

static void prv_init(void)
{
  load_settings();

  app_message_register_inbox_received(app_message_received);
  app_message_open(48, 48);

  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = prv_window_load,
    .unload = prv_window_unload,
  });
  const bool animated = false;
  window_stack_push(s_window, animated);
}

static void prv_deinit(void) {
  window_destroy(s_window);
}

int main(void) {
  prv_init();
  app_event_loop();
  prv_deinit();
}

#include "fireworks_app.h"

#include <assert.h>
#include <string.h>

#include "audio_proc.h"
#include "chart.h"
#include "display.h"
#include "prng.h"
#include "util.h"

static uint16_t FireworksAppOnStart(App * instance) {
  assert(instance);

  return APP_EV_MASK_KNOB | APP_EV_MASK_AUDIO;
}

static void FireworksAppOnResume(App * instance) {
  assert(instance);

  FireworksApp * app = (FireworksApp *) instance;
  app->prev_tilt = 0x7fff;
}

static void (FireworksAppOnTick) (App * instance,
                                  GfxRect const * region,
                                  int16_t * audio_samples,
                                  int16_t tilt,
                                  int8_t knob_turn_delta,
                                  int8_t knob_press_delta,
                                  bool force_redraw) {
  assert(instance);

  FireworksApp * app = (FireworksApp *) instance;

  uint8_t gain = app->gain_widget.val;
  uint8_t brightness = app->brightness_widget.val * 255 / 100;
  uint8_t value_fade_rate = app->value_fade_widget.val * 255 / 100;
  uint8_t sat_fade_rate = app->sat_fade_widget.val * 255 / 100;
  int16_t delta_tilt = app->prev_tilt == 0x7FFF ? 0 : tilt - app->prev_tilt;


  int16_t * raw_buckets;
  int16_t * full_buckets;
  int16_t * octave_buckets;

  // Fade channels.
  for (size_t i = 0; i < FIREWORKS_APP_NUM_CHANNELS; ++i) {
    if (value_fade_rate >= app->channels[i].v) {
      app->channels[i].v = 0;
    } else {
      app->channels[i].v -= value_fade_rate;
    }
    if (sat_fade_rate >= app->channels[i].s) {
      app->channels[i].s = 0;
    } else {
      app->channels[i].s -= sat_fade_rate;
    }
  }

  // Pitch analysis.
  AudioProcAnalyzePitch(audio_samples,
                        gain,
                        &raw_buckets,
                        &full_buckets,
                        &octave_buckets);

  int16_t * buckets = app->mode_widget.val == 0 ? octave_buckets : full_buckets;

  // Find max bucket.
  size_t max_bucket_index = 0;
  int16_t max_bucket_value = 0;
  for (size_t i = 0; i < BUCKET_COUNT; ++i) {
    if (buckets[i] >= max_bucket_value) {
      max_bucket_index = i;
      max_bucket_value = buckets[i];
    }
  }
  if (max_bucket_value > 0xFF) max_bucket_value = 0xFF;

  // Find min channel.
  size_t min_channel_index = 0;
  int8_t min_channel_value = 0x7F;
  for (size_t i = 0; i < FIREWORKS_APP_NUM_CHANNELS; ++i) {
    if (app->channels[i].v <= min_channel_value) {
      min_channel_index = i;
      min_channel_value = app->channels[i].v;
    }
  }

  // Possibly replace weakest channel with new value.
  if (max_bucket_value > min_channel_value) {
    FireworksAppChannel * channel = &app->channels[min_channel_index];
    channel->bucket = max_bucket_index;
    channel->s = 0xFF;
    channel->v = max_bucket_value;
  }

  // Random assignment of channels to LEDs. Based on a (partial) random
  // permutation of an array containing all LED indices.
  uint8_t leds[10];
  for (uint8_t i = 0; i < 10; ++i) leds[i] = i;
  for (size_t i = 0; i < FIREWORKS_APP_NUM_CHANNELS; ++i) {
    uint8_t index = i + (PrngGenerate16() % (10 - i));
    uint8_t temp = leds[index];
    leds[index] = leds[i];
    leds[i] = temp;
  }

  if (delta_tilt > 10) {
    // Lightning strike!
    for (unsigned i = 0; i < 10; ++i) {
      AppSetPixel(i, RGB888(brightness, brightness, brightness));
    }
  } else {
    for (size_t i = 0; i < FIREWORKS_APP_NUM_CHANNELS; ++i) {
      FireworksAppChannel * channel = &app->channels[i];
      AppSetPixel(leds[i],
                  Rgb888Scale(Hsv2Rgb888(channel->bucket * (1536 / BUCKET_COUNT),
                                         channel->s,
                                         channel->v),
                              brightness));
    }
  }

  //////////////////////////////////////////////////////////////////////////////
  // GUI

  GfxRect graph_region, text_region, widget_region;
  GfxSubRegion(region, 0, 0, BUCKET_COUNT, 48, &graph_region);
  GfxSubRegion(region, 32, 0, region->w - 32, 48, &text_region);
  GfxSubRegion(region, 0, 48, DISPLAY_WIDTH, region->h - 48, &widget_region);

  if (force_redraw) {
    GfxFillRect(region, 0, 0, region->w, 48, RGB565_BLACK);
  }

  for (size_t i = 0; i < FIREWORKS_APP_NUM_CHANNELS; ++i) {
    char const * const * strings = app->mode_widget.val ?
                                   full_bucket_names :
                                   octave_bucket_names;
    FireworksAppChannel const * channel = &app->channels[i];
    char const * string = strings[channel->bucket];
    char buf[8] = "       ";
    memcpy(buf, string, strlen(string));
    int x = (i >> 2) * 48 + 2;
    int y = (i & 3) * 10 + 4;
    uint8_t v = channel->v < 32 ? channel->v * 8 : 255;
    GfxDrawString(&text_region, x, y, buf,
                  Hsv2Rgb565(channel->bucket * (1536 / BUCKET_COUNT),
                             channel->s,
                             v),
                  RGB565_BLACK);
  }

  DrawBarGraph(&graph_region,
               (uint16_t const *) buckets,
               BUCKET_COUNT,
               0,
               RGB565(0x80, 0x80, 0xC0),
               RGB565_BLACK);

  app->prev_tilt = tilt;

  app->widget->OnTick(app->widget,
                      &widget_region,
                      knob_turn_delta,
                      knob_press_delta,
                      force_redraw,
                      WIDGET_ACTIVE);
}

static char const * const bucket_mode_strings[] = {
  "Octave",
  "Full"
};

App * FireworksAppInit(FireworksApp * instance) {
  assert(instance);

  memset(instance, 0, sizeof(FireworksApp));

  App * app = &instance->app;
  app->title = "Fireworks";
  app->OnStart = FireworksAppOnStart;
  app->OnResume = FireworksAppOnResume;
  app->OnTick = FireworksAppOnTick;

  instance->widgets[0] = NumberWidgetInit(&instance->gain_widget,
                                          "Gain",
                                          DISPLAY_WIDTH,
                                          16,
                                          4,
                                          1,
                                          0,
                                          20,
                                          RGB565_BLACK,
                                          RGB565_DARK_GRAY,
                                          RGB565_LIGHT_GRAY,
                                          RGB565_WHITE,
                                          0);

  instance->widgets[1] = NumberWidgetInit(&instance->brightness_widget,
                                          "Brightness",
                                          DISPLAY_WIDTH,
                                          16,
                                          20,
                                          5,
                                          0,
                                          100,
                                          RGB565_BLACK,
                                          RGB565_DARK_GRAY,
                                          RGB565_LIGHT_GRAY,
                                          RGB565_WHITE,
                                          0);

  instance->widgets[2] = NumberWidgetInit(&instance->value_fade_widget,
                                          "Fade rate",
                                          DISPLAY_WIDTH,
                                          16,
                                          2,
                                          1,
                                          1,
                                          10,
                                          RGB565_BLACK,
                                          RGB565_DARK_GRAY,
                                          RGB565_LIGHT_GRAY,
                                          RGB565_WHITE,
                                          0);

  instance->widgets[3] = NumberWidgetInit(&instance->sat_fade_widget,
                                          "Desaturation rate",
                                          DISPLAY_WIDTH,
                                          16,
                                          3,
                                          1,
                                          0,
                                          10,
                                          RGB565_BLACK,
                                          RGB565_DARK_GRAY,
                                          RGB565_LIGHT_GRAY,
                                          RGB565_WHITE,
                                          0);

  instance->widgets[4] = EnumWidgetInit(&instance->mode_widget,
                                        "Mode",
                                        DISPLAY_WIDTH,
                                        16,
                                        ENUM_WIDGET_CLICK,
                                        bucket_mode_strings,
                                        ARRAY_LEN(bucket_mode_strings),
                                        0,
                                        RGB565_BLACK,
                                        RGB565_DARK_GRAY,
                                        RGB565_LIGHT_GRAY,
                                        RGB565_WHITE,
                                        0);

  instance->widgets[5] = TextWidgetInit(&instance->exit_widget,
                                        "Exit",
                                        DISPLAY_WIDTH,
                                        16,
                                        RGB565_BLACK,
                                        RGB565_DARK_GRAY,
                                        RGB565_LIGHT_GRAY,
                                        RGB565_WHITE,
                                        AppCommandPop());

  instance->widget = VerticalWidgetListInit(&instance->widget_list,
                                            instance->widgets,
                                            ARRAY_LEN(instance->widgets));

  return app;
}



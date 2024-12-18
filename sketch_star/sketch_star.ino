// 1. Install ESP32 support for Arduino IDE
// 2. Install fastled (sketch -> include library -> fastled)
// 3. Choose ESP32 DEV module

// -------------------- INCLUDES ------------------------------
#include "FastLED.h"

// -------------------- DEFINE CONSTANTS ----------------------
// LED AND PIN_BUTTON ARE SWITCHED FOR ONE MODEL (23 and 18)
#define PIN_LED_STRIP 23 
// #define PIN_BUTTON 18
// Number of LEDs on the LED strip
#define N_LEDS 250
// ANIMATION CONSTANTS
#define N_STAR 10


// -------------------- SETUP VARIABLES -----------------------
// CRGB is a class from the FastLED package, containing a value for R, G and B.
// pixel_buffer contains the data that will be written regularly to the LED strip.
CRGB pixel_buffer[N_LEDS];
uint16_t animation_speed = 150;

// -------------------- DEFINE FUNCTIONS -----------------------

// -------------------- FUNCTION WHEEL (GET RAINBOW COLORS BY NUMBER 0 - 255)
CRGB wheel(byte wheel_pos)
{
  uint8_t rgb[3];
  wheel_pos = 255 - wheel_pos;
  if (wheel_pos < 85)
  {
    rgb[0] = 255 - wheel_pos * 3;
    rgb[1] = 0;
    rgb[2] = wheel_pos * 3;
  }
  else if (wheel_pos < 170)
  {
    wheel_pos -= 85;
    rgb[0] = 0;
    rgb[1] = wheel_pos * 3;
    rgb[2] = 255 - wheel_pos * 3;
  }
  else
  {
    wheel_pos -= 170;
    rgb[0] = wheel_pos * 3;
    rgb[1] = 255 - wheel_pos * 3;
    rgb[2] = 0;
  }
  return CRGB(rgb[0], rgb[1], rgb[2]);
};


// -------------------- CLASSES ---------------------------
// -------------------- CLASS STAR 
class Star
{
public:
  Star();
  void random_init();
  void update();
  uint16_t pos;
  int32_t lifetime;
  int32_t lifetime_tot;
  uint32_t brightness;
  int32_t delay_frame;
  CRGB color;
};

Star::Star(){}

void Star::random_init()
{
  lifetime_tot = random(20000, 50000);
  lifetime = lifetime_tot;
  brightness = 0;
  pos = random(N_LEDS);
  delay_frame = random(50000);
  // Get a random rainbow color.
  color = wheel(random(256));

}

void Star::update()
{
  // If there is still delay of star to start, decrease it and skip this loop run.
  if (delay_frame > 0)
  {
    delay_frame -= animation_speed;
    return;
  }

  brightness = lifetime_tot - abs(lifetime_tot - lifetime * 2);
  lifetime -= animation_speed;

  if (lifetime <= 0)
  {
    random_init();
  }
}

// -------------------- SETUP -----------------------------

// ANIMATION SETUP 
Star stars[N_STAR];

void setup() {
  // put your setup code here, to run once:

  // Define LED type, pin and array to use for the data 
  FastLED.addLeds<WS2812B, PIN_LED_STRIP, GRB>(pixel_buffer, N_LEDS);

  // ANIMATION SETUP
  for (int i_star = 0; i_star < N_STAR; i_star++) {
    stars[i_star].random_init();
  }
}


// -------------------- LOOP ------------------------------

void loop() {
  // put your main code here, to run repeatedly:

  for (uint8_t i_star = 0; i_star < N_STAR; i_star++)
  {
    stars[i_star].update();
    pixel_buffer[stars[i_star].pos].r += map(stars[i_star].brightness, 0, stars[i_star].lifetime_tot, 0, stars[i_star].color.r);
    pixel_buffer[stars[i_star].pos].g += map(stars[i_star].brightness, 0, stars[i_star].lifetime_tot, 0, stars[i_star].color.g);
    pixel_buffer[stars[i_star].pos].b += map(stars[i_star].brightness, 0, stars[i_star].lifetime_tot, 0, stars[i_star].color.b);
  }
  
  // Write pixel buffer to the LED strip.
  FastLED.show();

  // RESET PIXEL BUFFER
  for (uint16_t i_led = 0; i_led < N_LEDS; i_led++)
  {
    pixel_buffer[i_led] = CRGB::Black;
  }
}


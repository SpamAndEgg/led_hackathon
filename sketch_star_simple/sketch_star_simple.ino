// 1. Install ESP32 support for Arduino IDE
// 2. Install fastled (sketch -> include library -> fastled)
// 3. Choose ESP32 DEV module

// -------------------- INCLUDES ------------------------------
#include "FastLED.h"

// -------------------- DEFINE CONSTANTS ----------------------
// LED AND PIN_BUTTON ARE SWITCHED FOR ONE MODEL
#define PIN_LED_STRIP 18  
// #define PIN_BUTTON 23  
// Number of LEDs on the LED strip
#define N_LEDS 250
// ANIMATION CONSTANTS
#define N_STAR 10


// -------------------- SETUP VARIABLES -----------------------
// CRGB is a class from the FastLED package, containing a value for R, G and B.
// pixel_buffer contains the data that will be written regularly to the LED strip.
CRGB pixel_buffer[N_LEDS];


// -------------------- CLASSES ---------------------------
// -------------------- CLASS STAR 
CRGB COLORS_TO_CHOSE_FROM[5] = {CRGB::Yellow, CRGB::Red, CRGB::Pink, CRGB::White, CRGB::Green};
class Star
{
public:
  Star();
  void random_init();
  void update();
  uint16_t pos;
  int32_t lifetime;
  CRGB color;
};

Star::Star(){}

void Star::random_init()
{
  lifetime = random(20, 300);
  pos = random(N_LEDS);
  // Get a random rainbow color.
  color = COLORS_TO_CHOSE_FROM[random(5)];

}

void Star::update()
{

  lifetime = lifetime - 1;
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
    pixel_buffer[stars[i_star].pos] = stars[i_star].color;
  }
  
  // Write pixel buffer to the LED strip.
  FastLED.show();

  // RESET PIXEL BUFFER
  for (uint16_t i_led = 0; i_led < N_LEDS; i_led++)
  {
    pixel_buffer[i_led] = CRGB::Black;
  }
}


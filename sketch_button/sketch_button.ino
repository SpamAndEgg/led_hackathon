// 1. Install ESP32 support for Arduino IDE
// 2. Install fastled (sketch -> include library -> fastled)
// 3. Choose ESP32 DEV module

// -------------------- INCLUDES ------------------------------
#include "FastLED.h"

// -------------------- DEFINE CONSTANTS ----------------------
// LED AND PIN_BUTTON ARE SWITCHED FOR ONE MODEL (23 and 18)
#define PIN_LED_STRIP 23 
#define PIN_BUTTON 18  
// Number of LEDs on the LED strip
#define N_LEDS 250


// -------------------- SETUP VARIABLES -----------------------
// Counter for the number of main loop iterations there has already been. 
uint32_t counter = 0;
// CRGB is a class from the FastLED package, containing a value for R, G and B.
// pixel_buffer contains the data that will be written regularly to the LED strip.
CRGB pixel_buffer[N_LEDS];


// -------------------- DEFINE FUNCTIONS -----------------------

// -------------------- FUNCTION BUTTON


void update_button()
{
  bool button_state = digitalRead(PIN_BUTTON);
  static bool last_state = true;
  
  if (button_state == LOW)
  {
    // Button is pressed. Write your code here
    if (button_state != last_state)
    {
      // Button is now pressed, but wasn't pressed last loop
      counter = counter + 1;
    }
  }
  last_state = button_state;
}

// -------------------- SETUP -----------------------------

void setup() {
  // put your setup code here, to run once:

  FastLED.addLeds<WS2812B, PIN_LED_STRIP, GRB>(pixel_buffer, N_LEDS);

  // Setup pullup resistors 
  pinMode(PIN_BUTTON, INPUT_PULLUP);

}


// -------------------- LOOP ------------------------------

void loop() {
  // put your main code here, to run repeatedly:

  update_button();

  pixel_buffer[counter % N_LEDS] = CRGB::Blue;

  // Write pixel buffer to the LED strip.
  FastLED.show();

  // RESET PIXEL BUFFER
  for (uint16_t i_led = 0; i_led < N_LEDS; i_led++)
  {
    pixel_buffer[i_led] = CRGB::Black;
  }

}


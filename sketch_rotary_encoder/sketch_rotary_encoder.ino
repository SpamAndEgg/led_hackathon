// 1. Install ESP32 support for Arduino IDE
// 2. Install fastled (sketch -> include library -> fastled)
// 3. Choose ESP32 DEV module

// -------------------- INCLUDES ------------------------------
#include "FastLED.h"

// -------------------- DEFINE CONSTANTS ----------------------
// LED AND PIN_BUTTON ARE SWITCHED FOR ONE MODEL (23 and 18)
#define PIN_LED_STRIP 23  
#define PIN_ENCODER_1 22
#define PIN_ENCODER_2 17
// Number of LEDs on the LED strip
#define N_LEDS 20


// -------------------- SETUP VARIABLES -----------------------
// Counter for the number of main loop iterations there has already been. 
int16_t counter = 0;
int last_val_encoder_1;
int val_encoder_1;
// CRGB is a class from the FastLED package, containing a value for R, G and B.
// pixel_buffer contains the data that will be written regularly to the LED strip.
CRGB pixel_buffer[N_LEDS];


// -------------------- DEFINE FUNCTIONS -----------------------

// -------------------- FUNCTION BUTTON

unsigned long _lastIncReadTime = micros(); 
unsigned long _lastDecReadTime = micros(); 
int _pauseLength = 10000;
int _fastIncrement = 3;

// const int ENCODER_MAX = 350;
const int ENCODER_MIN = 0;
volatile int encoder_value;

void read_encoder() {
  // Encoder interrupt routine for both pins. Updates counter
  // if they are valid and have rotated a full indent
 
  static uint8_t old_AB = 3;  // Lookup table index
  static int8_t encval = 0;   // Encoder value  
  static const int8_t enc_states[]  = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0}; // Lookup table

  old_AB <<=2;  // Remember previous state

  if (digitalRead(PIN_ENCODER_1)) old_AB |= 0x02; // Add current state of pin A
  if (digitalRead(PIN_ENCODER_2)) old_AB |= 0x01; // Add current state of pin B
  
  encval += enc_states[( old_AB & 0x0f )];
  int8_t changevalue = 0;
  // Update counter if encoder has rotated a full indent, that is at least 4 steps
  if( encval < -3 )  {        // Four steps forward
    changevalue = 1;
    if((micros() - _lastIncReadTime) < _pauseLength) {
      changevalue = _fastIncrement * changevalue; 
    }
    _lastIncReadTime = micros();             // Update counter
    encval = 0;
  }
  else if( encval > 3 ) {        // Four steps backward
    changevalue = -1;
    if((micros() - _lastDecReadTime) < _pauseLength) {
      changevalue = _fastIncrement * changevalue; 
    }
    _lastDecReadTime = micros();
    
    encval = 0;
  }
  counter = counter + changevalue;
  if (counter < 0) 
  {
    counter = N_LEDS - 1;
  }
} 
 
// -------------------- SETUP -----------------------------

void setup() {
  // put your setup code here, to run once:

  FastLED.addLeds<WS2812B, PIN_LED_STRIP, GRB>(pixel_buffer, N_LEDS);

  // Setup pullup resistors 
  pinMode(PIN_ENCODER_1, INPUT_PULLUP);
  pinMode(PIN_ENCODER_2, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(PIN_ENCODER_1), read_encoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_ENCODER_2), read_encoder, CHANGE); 

  Serial.begin(115200);

}


// -------------------- LOOP ------------------------------

void loop() {
  // put your main code here, to run repeatedly:

  pixel_buffer[counter % N_LEDS] = CRGB::Blue;

  // Write pixel buffer to the LED strip.
  FastLED.show();

  // RESET PIXEL BUFFER
  for (uint16_t i_led = 0; i_led < N_LEDS; i_led++)
  {
    pixel_buffer[i_led] = CRGB::Black;
  }

}


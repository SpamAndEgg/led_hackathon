// 1. Install ESP32 support for Arduino IDE
// 2. Install fastled (sketch -> include library -> fastled)
// 3. Choose ESP32 DEV module

// -------------------- INCLUDES ------------------------------
#include "FastLED.h"

// -------------------- DEFINE CONSTANTS ----------------------
#define PIN_ENCODER_1 22
#define PIN_ENCODER_2 17
// LED AND PIN_BUTTON ARE SWITCHED FOR ONE MODEL (23 and 18)
#define PIN_LED_STRIP 23  
#define PIN_BUTTON 18  
// Number of LEDs on the LED strip
#define N_LEDS 250
#define N_MODE 1
// ANIMATION CONSTANTS
#define N_STAR 20
#define N_DROP 5
#define DROP_DELAY_COUNTER 500


// -------------------- SETUP VARIABLES -----------------------
// Counter for the number of main loop iterations there has already been. 
uint32_t counter = 0;
// Track which mode currently is active. The mode can be switched with the button.
uint8_t mode = 0;
// Brightness can have values from 0 - 255
int16_t led_brightness = 150;
uint16_t animation_speed = 20;
// CRGB is a class from the FastLED package, containing a value for R, G and B.
// pixel_buffer contains the data that will be written regularly to the LED strip.
CRGB pixel_buffer[N_LEDS];


// -------------------- DEFINE FUNCTIONS -----------------------

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
      mode = (mode + 1) % N_MODE;
    }
  }
  last_state = button_state;
}

// -------------------- FUNCTION ENCODER

unsigned long _lastIncReadTime = micros(); 
unsigned long _lastDecReadTime = micros(); 
int _pauseLength = 25000;
int _fastIncrement = 3;

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
  int16_t changevalue = 0;
  // Update counter if encoder has rotated a full indent, that is at least 4 steps
  if( encval < -3 )
  {        // Four steps forward
    changevalue = 5;
    if((micros() - _lastIncReadTime) < _pauseLength) {
      changevalue = _fastIncrement * changevalue; 
    }
    _lastIncReadTime = micros();             // Update counter
    encval = 0;
  }
  else if ( encval > 3 ) 
  {        // Four steps backward
    changevalue = -5;
    if((micros() - _lastDecReadTime) < _pauseLength) {
      changevalue = _fastIncrement * changevalue; 
    }
    _lastDecReadTime = micros();
    encval = 0;
  }
  
  led_brightness = std::min(std::max(led_brightness + changevalue, 0), 255); 
  Serial.println(led_brightness);
  Serial.println(changevalue);
} 
 

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

Star::Star(){};

void Star::random_init()
{
  lifetime_tot = random(1500, 4000);
  lifetime = lifetime_tot;
  brightness = 0;
  pos = random(N_LEDS);
  delay_frame = random(4000);
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

Star stars[N_STAR];

void star_animation() {
    for (uint8_t i_star = 0; i_star < N_STAR; i_star++)
  {
    stars[i_star].update();
    pixel_buffer[stars[i_star].pos].r += map(stars[i_star].brightness, 0, stars[i_star].lifetime_tot, 0, stars[i_star].color.r);
    pixel_buffer[stars[i_star].pos].g += map(stars[i_star].brightness, 0, stars[i_star].lifetime_tot, 0, stars[i_star].color.g);
    pixel_buffer[stars[i_star].pos].b += map(stars[i_star].brightness, 0, stars[i_star].lifetime_tot, 0, stars[i_star].color.b);
  }
}

// -------------------- CLASS DROP 


class Drop
{
public:
    Drop();
    uint16_t pos_head;
    uint16_t pos_between_head_jump;
    uint8_t length;
    CRGB color;
    uint8_t color_rainbow;
    uint32_t speed;
    int32_t delay_counter;
    void random_init();
    void update();
};

Drop::Drop(){};

void Drop::random_init()
{
  delay_counter = random(300000, 50000);
  speed = random(5, 8);
  //speed = 2;
  length = random(3, 5);
  pos_head = 0;
  pos_between_head_jump = 0;

  // If color_int is not zero, use rainbow color instead of color scheme
  color_rainbow = random(1, 256);
}

void Drop::update()
{
  pos_between_head_jump += speed * animation_speed;
  // If the delay counter is on, reduce it by one, otherwise update the drop pos_head.
  if (delay_counter > 0)
  {
    delay_counter -= speed * animation_speed;
    return;
  }
  pos_between_head_jump = 0;
  pos_head++;
  delay_counter = DROP_DELAY_COUNTER;
  // Initiate the drop again if it not on the LED stripe anymore
  if (pos_head > N_LEDS + length)
  {
    random_init();
  }
}

Drop drops[N_DROP];

void drop_animation()
{
  double temp_res;
  float brightness_multiplier;
  for (byte i_drop = 0; i_drop < N_DROP; i_drop++)
  {
    drops[i_drop].update();
    temp_res = (double)(drops[i_drop].pos_between_head_jump) / (double)DROP_DELAY_COUNTER;
    temp_res = std::min(1.0, temp_res);
    // Here a drop is set. The drop effect is realized by reducing the brightness from the lowest pixel on to the top.
    // Special cases for when the drop exceeds the LED stripe to the top or bottom have to be consindered.

    // First, the visible part on the stripe is defined.
    // The top pixel of the drop that can be seen (If all are seen, this is 0)
    uint8_t dropindex_top;
    // The bottom pixel of the drop that can be seen (If all are seen, this is the drop length)
    uint8_t dropindex_bot;
    if (drops[i_drop].pos_head < drops[i_drop].length)
    {
      // Case: The drop exceeds the LED stripe at the top, thus only the visible pixels at the drop bottom have to be shown.
      dropindex_top = 0;
      dropindex_bot = drops[i_drop].pos_head;
    }
    else if (drops[i_drop].pos_head > N_LEDS)
    {
      // Case: The drop exceeds the LED stripe at the bottom, thus only the upper drop pixels are visible.
      dropindex_top = drops[i_drop].pos_head - N_LEDS;
      dropindex_bot = drops[i_drop].length;
    }
    else
    {
      // Case: The whole drop can be displayed.
      dropindex_top = 0;
      dropindex_bot = drops[i_drop].length;
    }
    // Go through every drop pixel and adjust the brightness. The bottom pixel will have full brightness.
    for (uint16_t i_droppixel = dropindex_top; i_droppixel < dropindex_bot; i_droppixel++)
    {
      brightness_multiplier = 0;
      // If bottom pixel set to full brightness.
      if (i_droppixel == 0)
      {
        brightness_multiplier = temp_res;
      }
      // If not the bottom pixel lower brightness, the more up the pixel the lower the brightness.
      else
      {
        brightness_multiplier = ((double)(drops[i_drop].length) - (double)(i_droppixel) - temp_res) / ((double)(drops[i_drop].length) - 1.0);
      }

      uint16_t color_length = 16;
      uint16_t wheel_selector = (drops[i_drop].color_rainbow + i_droppixel * color_length + (uint16_t)(temp_res * (double)color_length)) % 256;

      CRGB color = wheel(wheel_selector);

      // Update the pixel buffer.
      pixel_buffer[drops[i_drop].pos_head - i_droppixel - 1].r += color.r;
      pixel_buffer[drops[i_drop].pos_head - i_droppixel - 1].g += color.g;
      pixel_buffer[drops[i_drop].pos_head - i_droppixel - 1].b += color.b;
    }
  }
}


// -------------------- SETUP -----------------------------


void setup() {
  // put your setup code here, to run once:
  randomSeed(analogRead(0));

  // Define LED type, pin and array to use for the data 
  FastLED.addLeds<WS2812B, PIN_LED_STRIP, GRB>(pixel_buffer, N_LEDS);

  // Setup pullup resistors 
  pinMode(PIN_ENCODER_1, INPUT_PULLUP);
  pinMode(PIN_ENCODER_2, INPUT_PULLUP);
  pinMode(PIN_BUTTON, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(PIN_ENCODER_1), read_encoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_ENCODER_2), read_encoder, CHANGE);

  Serial.begin(115200);

  // ANIMATION SETUP
  for (int i_star = 0; i_star < N_STAR; i_star++) {
    stars[i_star].random_init();
  }

  for (int i_drop = 0; i_drop < N_DROP; i_drop++) {
    drops[i_drop].random_init();
  }
}


// -------------------- LOOP ------------------------------

void loop() {
  // put your main code here, to run repeatedly:

  update_button();

  // ANIMATIONS
  if (mode == 0)
  {


  }
  star_animation();
  drop_animation();

  // Set the brightness.
  FastLED.setBrightness(led_brightness);
  // Write pixel buffer to the LED strip.
  FastLED.show();

  // RESET PIXEL BUFFER
  for (uint16_t i_led = 0; i_led < N_LEDS; i_led++)
  {
    pixel_buffer[i_led] = CRGB::Black;
  }

}


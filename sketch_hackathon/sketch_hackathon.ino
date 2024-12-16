// 1. Install ESP32 support for Arduino IDE
// 2. Install fastled (sketch -> include library -> fastled)
// 3. Choose ESP32 DEV module

// -------------------- INCLUDES ------------------------------
#include "FastLED.h"

// -------------------- DEFINE CONSTANTS ----------------------
#define PIN_ENCODER_1 22
#define PIN_ENCODER_2 27
// LED AND PIN_BUTTON ARE SWITCHED FOR ONE MODEL
#define PIN_LED_STRIP 18  
#define PIN_BUTTON 23  
// Number of LEDs on the LED strip
#define N_LEDS 250
#define N_MODE 1
// ANIMATION CONSTANTS
#define N_STAR 20
#define N_DROP 5


// -------------------- SETUP VARIABLES -----------------------
// Counter for the number of main loop iterations there has already been. 
uint32_t counter = 0;
// Track which mode currently is active. The mode can be switched with the button.
uint8_t mode = 0;
// Brightness can have values from 0 - 255
int brightness = 150;
uint8_t animation_speed = 150;
// CRGB is a class from the FastLED package, containing a value for R, G and B.
// pixel_buffer contains the data that will be written regularly to the LED strip.
CRGB pixel_buffer[N_LEDS];


// -------------------- DEFINE FUNCTIONS -----------------------

// -------------------- FUNCTION BUTTON
unsigned long time_button_pressed = 0;
uint8_t last_button_state = false;
const int BUTTON_NO_PRESS = 0;
const int BUTTON_NO_EFFECT = 1;
const int BUTTON_CHANGE_MODE = 2;
const int PRESS_TIME_SAFTY = 20;       // 20 milliseconds
void update_button() 
{

  bool button_state = digitalRead(PIN_BUTTON);

  if (button_state == HIGH)
  // HIGH state means button is not pressed
  {
    if (last_button_state == BUTTON_CHANGE_MODE)
    { // short press
      last_button_state = BUTTON_NO_PRESS;
      mode = (mode + 1) % N_MODE;
      Serial.println("Button was pressed");
    }
  }
  else
  // button_state == LOW --> button is currently pressed.
  {
    if (last_button_state == BUTTON_NO_PRESS)
    {
      time_button_pressed = millis();
      last_button_state = BUTTON_NO_EFFECT;
    }
    else if (millis() - time_button_pressed > PRESS_TIME_SAFTY)
    { // pressed long enough to change mode
      last_button_state = BUTTON_CHANGE_MODE;
    }
  }
}


// -------------------- FUNCTION ENCODER


unsigned long previousMillis = 0;
const long debounce = 80;

int previousStateA;
int previousStateB;
int currentStateA;
int currentStateB;

volatile int flagStates;
volatile int flag;








void read_encoder() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= debounce) {
    previousMillis = currentMillis;
    currentStateA = digitalRead(PIN_ENCODER_1); // Simple read of current HIGH or LOW state
    currentStateB = digitalRead(PIN_ENCODER_2);
    flagStates = 1;
    if (previousStateA == currentStateA && previousStateB == currentStateB) { // Triggerd only if current state of both pins are equal to previous
      flag = 1;
    }
    previousStateA = currentStateA;
    previousStateB = currentStateB;
  }
}



/*
int state_a;
int state_a_last;

void read_encoder() 
{
  state_a = digitalRead(PIN_ENCODER_1);
  // If the previous and the current state of the encoder A are different, that means a Pulse has occured
  if (state_a != state_a_last)
  {
    // If the outputB state is different to the outputA state, that means the encoder is rotating clockwise
    if (digitalRead(PIN_ENCODER_2) != state_a)
    {
      Serial.println("Clockwise");
    }
    else
    {
      Serial.println("Counterclockwise");
    }
  }
  state_a_last = state_a;
}
*/
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
  lifetime_tot = random(5000, 10000);
  lifetime = lifetime_tot;
  brightness = 0;
  pos = random(N_LEDS);
  delay_frame = random(5000);
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



// -------------------- CLASS DROP 

class Drop
{
public:
    Drop();
    uint16_t n_pixel;
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


// -------------------- SETUP -----------------------------

// ANIMATION SETUP 
Star stars[N_STAR];
Drop drops[N_DROP];

void setup() {
  // put your setup code here, to run once:
  randomSeed(analogRead(0));

  // Define LED type, pin and array to use for the data 
  FastLED.addLeds<WS2812B, PIN_LED_STRIP, GRB>(pixel_buffer, N_LEDS);

  // Setup pullup resistors 
  pinMode(PIN_ENCODER_1, INPUT_PULLUP);
  pinMode(PIN_ENCODER_2, INPUT_PULLUP);
  pinMode(PIN_BUTTON, INPUT_PULLUP);

  // Setup rotary encoder
  attachInterrupt(digitalPinToInterrupt(PIN_ENCODER_1), read_encoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_ENCODER_2), read_encoder, CHANGE);
  previousStateA = digitalRead(PIN_ENCODER_1); // Storage of state before the interrupt
  previousStateB = digitalRead(PIN_ENCODER_2);

  Serial.begin(115200);

  // ANIMATION SETUP
  for (int i_star = 0; i_star < N_STAR; i_star++) {
    stars[i_star].random_init();
  }
}


// -------------------- LOOP ------------------------------

void loop() {
  // put your main code here, to run repeatedly:

  update_button();
  //read_encoder();
  if (flagStates == 1) {
    noInterrupts();
    Serial.print("A: "); Serial.print(currentStateA);
    Serial.print(" "); Serial.print(currentStateB); Serial.println(" :B");
    flagStates = 0;
    interrupts();
  }
  if (flag == 1) {
    noInterrupts();
    Serial.println("Same values as previous!");
    flag = 0;
    interrupts();
  }

  // ANIMATIONS
  if (mode == 0)
  {
    for (uint8_t i_star = 0; i_star < N_STAR; i_star++)
    {
      stars[i_star].update();
      pixel_buffer[stars[i_star].pos].r += map(stars[i_star].brightness, 0, stars[i_star].lifetime_tot, 0, stars[i_star].color.r);
      pixel_buffer[stars[i_star].pos].g += map(stars[i_star].brightness, 0, stars[i_star].lifetime_tot, 0, stars[i_star].color.g);
      pixel_buffer[stars[i_star].pos].b += map(stars[i_star].brightness, 0, stars[i_star].lifetime_tot, 0, stars[i_star].color.b);
    }
  }

  // Set the brightness.
  FastLED.setBrightness(brightness);
  // Write pixel buffer to the LED strip.
  FastLED.show();

  // RESET PIXEL BUFFER
  for (uint16_t i_led = 0; i_led < N_LEDS; i_led++)
  {
    pixel_buffer[i_led] = CRGB::Black;
  }
  delay(20);
}


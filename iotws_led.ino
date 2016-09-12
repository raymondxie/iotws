/*
 * Sample code #1 - IoT Workshop
 * Blink a LED on/off every second
 * NodeMCU has two onboard LED lights that you can blink them, or you can connect your own LED to test blink
 * 
 * Author: Raymond Xie
 * Date: 9/5/2016
 */

/*
// NodeMCU pin: D0-10 mapping to GPIO
static const uint8_t D0   = 16;  // onboard red LED
static const uint8_t D1   = 5;
static const uint8_t D2   = 4;
static const uint8_t D3   = 0;
static const uint8_t D4   = 2;  // onboard blue LED
static const uint8_t D5   = 14;
static const uint8_t D6   = 12;
static const uint8_t D7   = 13;
static const uint8_t D8   = 15;
static const uint8_t D9   = 3;
static const uint8_t D10  = 1;
static const uint8_t LED_BUILTIN = 16;
static const uint8_t BUILTIN_LED = 16;
*/

// HINT: you may change ledPin to other PIN and connecting your own LED for blinking
const int ledPin = D0;

// runs initial one-time when board is turned on
void setup() {
  // initialize digital pin (ledPin) as an output.
  pinMode(ledPin, OUTPUT);
}

// the loop function runs in loop forever, until you shut it down
void loop() {
  digitalWrite(ledPin, HIGH);   // turn the LED on - set voltage HIGH
  delay(1000);              // wait for a second
  digitalWrite(ledPin, LOW);    // turn the LED off - set voltage LOW
  delay(1000);              // wait for a second
}


/*
 * Controller for PCB Etching Agitator (PEA)
 * 
 * Push Button Control
 *  - Push to start a one hour cycle of agitation
 *  - Push again to halt agitation and return to horizontal
 *  
 *  Should have a beeper to notify end of cycle
 *  
*/

#include <Adafruit_SoftServo.h>  // SoftwareServo (works on non PWM pins)

#define ServoPin 1    // The pin used to control the servo
#define ButtonPin 2   // The pin used to read the push button
#define LEDPin 3      // HID

#define SPEED 10      // Speed of Servo movement (seconds/cycle)
#define LOWER 0       // The minimum servo angle
#define UPPER 179     // The maximum servo angle
#define LEVEL 90      // Angle at which the agitator table is horizontal

#define ETCH_TIME 3600000/2    // number of millisecond pairs 

Adafruit_SoftServo PEAServo;  // create servo object to control the agitator 

int servoDirection = 1;               // Direction servo movement
boolean refresh = false;              // toggle refresh on/off
boolean agitate;                      // On or off
int target;                           // 
int debounce;                         // added to debounce the button
int pushed;
int servoPos = LEVEL;  // configure initial servo position in shaddow reg
int tick;
void setup() {
  // Set up the interrupt that will refresh the servo for us automagically
  OCR0A = 0xAF;            // any number is OK
  TIMSK |= _BV(OCIE0A);    // Turn on the compare interrupt (below!)
  
  PEAServo.attach(ServoPin);    // Attach the servo to pin 0 on Trinket
  PEAServo.write(LEVEL);        // init servo to horizontal position (rest position) 
  delay(15);                    // Wait 15ms for the servo to reach the position

  pinMode(ButtonPin, INPUT);
  debounce = 0;
  pushed = false;
  pinMode(LEDPin, OUTPUT);
  pinMode(0, OUTPUT);
  pinMode(4, OUTPUT);

  agitate = false; // wait for button before agitating
  target = LEVEL;  // point to rest position

  digitalWrite(LEDPin, HIGH);
}

void loop() {
  if (tick-- <= 0) {
    agitate = false;
    target = LEVEL;      // point to rest position
  }
  
  if (!digitalRead(ButtonPin)){ // button has been pushed
    if (debounce <= 5) {
      debounce++;
    } else {
      pushed = true;
    }
  } else {
    debounce = 0;
  }

  if (pushed) {
    digitalWrite(0, HIGH);
    if (agitate) {  // agitating now, so stop
      digitalWrite(LEDPin, HIGH);
      agitate = false;
      target = LEVEL;      // point to rest position
    } else {
      digitalWrite(LEDPin, LOW);
      agitate = true;
      target = UPPER;  // point to top position
      servoDirection = 1;  // move towards top 
      tick = ETCH_TIME;
    }
    pushed = false;
    delay(500);  // make sure you dont see another push too soon.
    digitalWrite(0, LOW);
    }
  
  if (agitate) {    // Check for out of limits condition and direction to target
    if (servoPos >= 179) {
      target = 0;
      servoDirection = -1;  // move towards bottom
      digitalWrite(4, LOW);
    }
    if (servoPos <= 0) {
      target = 179;
      servoDirection = 1;  // move towards top
      digitalWrite(4, HIGH);
    } 
  } else {
    if (servoPos > LEVEL) {
      servoDirection = -1;  // move down towards rest state
      digitalWrite(4, LOW);
    }
    if (servoPos < LEVEL) {
      servoDirection = 1;  // move up towards rest state
      digitalWrite(4, HIGH);
    }
  }

  // Move towards target
  if (servoPos != target) { // still need to get to target
    servoPos = servoPos + servoDirection;
    PEAServo.write(servoPos); 
//    digitalWrite(0, HIGH);
    delay(16);
//    digitalWrite(0, LOW);
  } 
}

// We'll take advantage of the built in millis() timer that goes off
// to keep track of time, and refresh the servo every 20 milliseconds
volatile uint8_t counter = 0;
SIGNAL(TIMER0_COMPA_vect) {
  // this gets called every 2 milliseconds
  counter += 2;
  // every 20 milliseconds, refresh the servos!
  if (counter >= 20) {
    counter = 0;
    PEAServo.refresh();
  }
}


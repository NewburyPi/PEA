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

#define LOWER 0       // The minimum servo angle
#define UPPER 170     // The maximum servo angle
#define LEVEL 90      // Angle at which the agitator table is horizontal

#define ETCH_TIME 3600000/2    // number of millisecond pairs 

Adafruit_SoftServo PEAServo;  // create servo object to control the agitator 

int servoDirection = 1;               // Direction servo movement
boolean refresh = false;              // toggle refresh on/off
boolean agitate;                      // On or off
int target;                           // 
int debounce;                         // added to debounce the button
int pushed;
int servoPos = LEVEL; // configure initial servo position in shaddow reg
int tick;             //

void setup() {
  // Set up the interrupt that will refresh the servo for us automagically
  OCR0A = 0xAF;            // any number is OK
  TIMSK |= _BV(OCIE0A);    // Turn on the compare interrupt (below!)
  
  PEAServo.attach(ServoPin);    // Attach the servo to pin 0 on Trinket
  PEAServo.write(LEVEL);        // init servo to horizontal position (rest position) 
  delay(15);                    // Wait 15ms for the servo to reach the position

  pinMode(ButtonPin, INPUT);
  pinMode(LEDPin, OUTPUT);

  agitate = false;  // initialize to load state
  digitalWrite(LEDPin, HIGH);
  debounce = 0;
  pushed = false;   // wait for button before agitating
  target = LEVEL;   // point to rest position

  tick = ETCH_TIME; // Reset etch timer on startup

}

void loop() {
//  if (tick-- <= 0) {
//    agitate = false;
//    target = LEVEL;      // point to rest position
//  }
  
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
    if (agitate) {  // agitating now, so stop
      digitalWrite(LEDPin, HIGH);
      agitate = false;
      target = LEVEL;      // point to rest position
    } else {
      digitalWrite(LEDPin, LOW);
      agitate = true;
      target = UPPER;     // point to top position
      servoDirection = 1; // move towards top 
      tick = ETCH_TIME;   // reset timer on agitate
    }
    pushed = false;
    delay(500);  // make sure you dont see another push too soon.
    }
  
  if (agitate) {    // Check for out of limits condition and direction to target
    if (servoPos >= UPPER) {    // reverse direction
      target = LOWER;
      servoDirection = -1;  // move towards bottom
    }
    if (servoPos <= LOWER) {    // reverse direction
      target = UPPER;
      servoDirection = 1;  // move towards top
    } 
    // need to put the blinker here. 
  } else {          // Here if not agitating (in rest mode)
    if (servoPos > LEVEL) {    // reverse direction
      servoDirection = -1;  // move down towards rest state
    }
    if (servoPos < LEVEL) {    // reverse direction
      servoDirection = 1;  // move up towards rest state
    }
  }

  // Move towards target
  if (servoPos != target) { // still need to get to target
    servoPos = servoPos + servoDirection;
    PEAServo.write(servoPos); 
    delay(50);    // wait for servo to complete movement
                  // 50mS is neccessary to keep the movement smooth
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
  tick += 2;    // decrement the agitate timer
  // 
}


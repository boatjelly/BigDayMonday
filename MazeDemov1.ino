/* Jeremy Maze Demo - Left Wall Hugging v1
   Written: May 7, 2025
   Description: Jeremy follows the left wall to escape a maze using whiskers >=<
   I/O Pins:
     A0: LED Left Barrier
     A1: LED Right Barrier
     D3: [OCR2B] Right Motor Forward (H-Bridge Pin 15)
     D5: [OCR0B] Left Motor Forward (H-Bridge Pin 7)
     D6: [OCR0A] Left Motor Reverse (H-Bridge Pin 2)
     D11: [OCR2A] Right Motor Reverse (H-Bridge Pin 10)
     D12: Right Whisker (front sensor)
     D13: Left Whisker (left wall sensor)

  TO-DO:
  - get rid of LEDs?
  - change 8s?
  - actually connect shit to Jeremy
  - adjust PWM values
  - add PWM constants
  - CONSIDER: getting rid of functions and just hard-coding in placeswhere its only used once
  - i'm tired :(
*/

volatile unsigned char whiskerLeft = 1;   // Active-LOW
volatile unsigned char whiskerRight = 1;  // Active-LOW
const unsigned char DELAY = 50; // sick of writing 50

void setup() {
  cli();  // Disable global interrupts

  // PWM Setup
  TCCR0A = 0xA1;
  TCCR0B = 0x01;
  TCCR2A = 0xA1;
  TCCR2B = 0x01;

  // Pin direction setup
  DDRC = 0x03;   // A0, A1: LEDs
  DDRD = 0x68;   // D3, D5, D6: motor control
  DDRB = 0x08;   // D11: motor control

  // Pull-ups for whiskers
  PORTB |= 0x30; // D12, D13

  sei();  // Re-enable global interrupts
}

// ***** MOTOR CONTROL *****
void moveForward() {
  OCR0A = 8; OCR0B = 237;
  OCR2A = 8; OCR2B = 255;
}

void moveBackward() {
  OCR0A = 237; OCR0B = 8;
  OCR2A = 255; OCR2B = 8;
}

void turnLeft() {
  OCR0A = 0; OCR0B = 200;
  OCR2A = 200; OCR2B = 0;
}

void turnRight() {
  OCR0A = 200; OCR0B = 0;
  OCR2A = 0; OCR2B = 200;
}

// ***** MAIN LOOP: Hug the Left Wall Logic *****
void loop() {
  // Read whiskers
  whiskerLeft  = (PINB & 0x20) >> 5;  // D13
  whiskerRight = (PINB & 0x10) >> 4;  // D12

  if (whiskerLeft == 1 && whiskerRight == 1) {
    // Nothing in front -> explore left 
    turnLeft();
    _delay_ms(DELAY);
    moveForward();
    _delay_ms(DELAY);
  } 
  else if (whiskerLeft == 0 && whiskerRight == 1) {
    // Wall on left, front clear -> move forward hugging wall
    moveForward();
  } 
  else if (whiskerRight == 0 && whiskerLeft == 1) {
    // Wall on front-right -> turn slightly left
    turnLeft();
    _delay_ms(DELAY);
  } 
  else if (whiskerLeft == 0 && whiskerRight == 0) {
    // Fully blocked — back up, then turn right
    moveBackward();
    _delay_ms(DELAY);
    turnRight();
    _delay_ms(DELAY);
  } 
  else {
    moveForward();
  }
}

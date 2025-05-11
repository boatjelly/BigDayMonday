/*
  Smart Car Permanent Header
  Left
  By: Sasha Dauz, Robert J. Guziec, Jacob JM Horstman
  Written: May 8th, 2025
  Edited: --
  I/O Pins
  A0: Left Sensor            [ADMUX 00]
  A1: Center Sensor          [ADMUX 01]
  A2: Right Sensor           [ADMUX 02]
  A3:
  A4: Right Encoder          [PCI PORTC 0x10]
  A5: Left Encoder           [PCI PORTC 0x20]
  D0:
  D1:
  D2:
  D3: 
  D4: Left Motor Control     [x]          [Pin 1 H-Bridge]
  D5: Left Motor Reverse     [TCNT0 OC0B] [Pin 7 H-Bridge]
  D6: Left Motor Forward     [TCNT0 OC0A] [Pin 2 H-Bridge]
  D7: Right Motor Control    [x]          [Pin 9 H-Bridge]
  D8:
  D9:  Right Motor Forward   [TCNT1 OC1A] [Pin 15 H-Bridge]
  D10: Right Motor Reverse    [TCNT1 OC1B] [Pin 10 H-Bridge]
  D11: 
  D12: Right Whisker         [PCI PORTB 0x10]
  D13: Left Whisker          [PCI PORTB 0x20]

  Ideas:
  - ultrasonic sensor hug the left wall
  - whiskers hug the left wall
  - whiskers detection

  NOTE: *AIM JEREMY TO HIT LEFT WALL*
*/

volatile unsigned char whiskerLeft = 1;
volatile unsigned char whiskerRight = 1;
const unsigned char DELAY = 50; // sick of writing 50

void setup() {
  Serial.begin(9600);
  cli();

  // PWM Setup
  TCCR0A = 0xA1;
  TCCR0B = 0x01;
  TCCR1A = 0xA1;
  TCCR1B = 0x01;

  // Pin direction setup
  DDRD = 0x60;   // D5, D6: motor control
  DDRB = 0x06;   // D9, D10: motor control

  // Configure pin change interrupts for whiskers
  PCICR = 0x01;  // Enable PORT B
  PCMSK0 = 0x30; // PORT B, D12 and D13

  // Pull-ups for whiskers
  PORTB |= 0x30; // D12, D13

  sei();
}

// ***** MOTOR CONTROL *****
// Format for motors:
// OCR0A = L motor forward   OCR0B = L motor reverse
// OCR1A = R motor forward   OCR1B = R motor reverse

// Correct, tested
void moveForward() {
  OCR0A = 237; OCR0B = 0;
  OCR1A = 255; OCR1B = 0;
  //  Serial.println("moveForward");
}

// Correct, not tested
void moveBackward() {
  OCR0A = 0; OCR0B = 237;
  OCR1A = 0; OCR1B = 255;
  //  Serial.println("moveBackward");
}

// Correct, not tested
void turnLeft() {
  OCR0A = 0;   OCR0B = 255;
  OCR1A = 255; OCR1B = 0;
  //  Serial.println("turnLeft");
}

// Correct, not tested
void turnRight() {
  OCR0A = 255; OCR0B = 0;
  OCR1A = 0; OCR1B = 255;
  //  Serial.println("turnRight");
}

// ***** MAIN LOOP: Hug the Left Wall Logic *****

// If whisker left detects wall:
// 1. Back up
// 2. Turn slightly right
// 3. Go forward

void loop() {
  moveForward();
  Serial.println("loop");
}

// Consider moving function logic into logic here to avoid spaghetti code
ISR(PCINT0_vect) {
  Serial.println("ISR");
  // Read whiskers
  whiskerLeft  = (PINB & 0x20) >> 5;  // D13
  whiskerRight = (PINB & 0x10) >> 4;  // D12

  // Adjust time to move as needed
  if (!whiskerLeft) {
    // Back up a bit
    for(unsigned char i = 0; i < 8; i++){
      moveBackward();
      _delay_ms(50);
    }
    // Turn right slightly
    for(unsigned char i = 0; i < 8; i++){
      turnRight();
      _delay_ms(50);
    }
  }
  else if (!whiskerRight) {
    // Turn left if Jer hits a wall on the right
    for(unsigned char i = 0; i < 8; i++){
      moveBackward();
      _delay_ms(50);
    }
        // Turn left slightly
    for(unsigned char i = 0; i < 8; i++){
      turnLeft();
      _delay_ms(50);
    }
  }
}

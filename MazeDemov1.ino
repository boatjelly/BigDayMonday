/*
  Smart Car Permanent Header
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
  D3: Right Motor Reverse    [TCNT2 OC2B] [Pin 10 H-Bridge]
  D4: Left Motor Control     [x]          [Pin 1 H-Bridge]
  D5: Left Motor Reverse     [TCNT0 OC0B] [Pin 7 H-Bridge]
  D6: Left Motor Forward     [TCNT0 OC0A] [Pin 2 H-Bridge]
  D7: Right Motor Control    [x]          [Pin 9 H-Bridge]
  D8:
  D9:
  D10:
  D11: Right Motor Forward   [TCNT2 OC2A] [Pin 15 H-Bridge]
  D12: Right Whisker         [PCI PORTB 0x10]
  D13: Left Whisker          [PCI PORTB 0x20]
*/

volatile unsigned char whiskerLeft = 1;   // Active-LOW
volatile unsigned char whiskerRight = 1;  // Active-LOW
const unsigned char DELAY = 50; // sick of writing 50

void setup() {
  Serial.begin(9600);
  cli();

  // PWM Setup
  TCCR0A = 0xA1;
  TCCR0B = 0x01;
  TCCR2A = 0xA1;
  TCCR2B = 0x01;

  // Pin direction setup
  DDRD = 0x68;   // D3, D5, D6: motor control
  DDRB = 0x08;   // D11: motor control

  // Configure pin change interrupts for whiskers
  PCICR = 0x01;  // Enable PORT B
  PCMSK0 = 0x30; // PORT B, D12 and D13

  // Pull-ups for whiskers
  PORTB |= 0x30; // D12, D13

  sei();
}

// ***** MOTOR CONTROL *****
void moveForward() {
  OCR0A = 237; OCR0B = 0;
  OCR2A = 255; OCR2B = 0; // OCR2B NOT WORKING
  //  Serial.println("moveForward");
}

void moveBackward() {
  OCR0A = 0; OCR0B = 237;
  OCR2A = 0; OCR2B = 255;
  //  Serial.println("moveBackward");
}

void turnLeft() { // Check this
  OCR0A = 0; OCR0B = 200;
  OCR2A = 200; OCR2B = 0;
  //  Serial.println("turnLeft");

}

void turnRight() { // Check this
  OCR0A = 200; OCR0B = 0;
  OCR2A = 0; OCR2B = 200;
  //  Serial.println("turnRight");

}

// ***** MAIN LOOP: Hug the Left Wall Logic *****
void loop() {
  moveForward();
  Serial.println("loop");
}

ISR(PCINT0_vect) {
  Serial.println("ISR");
  // Read whiskers
  whiskerLeft  = (PINB & 0x20) >> 5;  // D13
  whiskerRight = (PINB & 0x10) >> 4;  // D12
  if (whiskerLeft == 0 && whiskerRight == 1) {
    // Wall on left, front clear -> move forward hugging wall
    moveForward();
    Serial.println("Left press");
  }
  else if (whiskerRight == 0 && whiskerLeft == 1) {
    // Wall on front-right -> turn slightly left
    turnLeft();
    _delay_ms(DELAY);
    Serial.println("Right press");
  }
}

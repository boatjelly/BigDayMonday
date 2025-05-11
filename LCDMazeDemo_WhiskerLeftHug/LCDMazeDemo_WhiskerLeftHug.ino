/*
  Smart Car Permanent Header
  Left
  By: Sasha Dauz, Robert J. Guziec, Jacob JM Horstman
  Written: May 8th, 2025
  Edited: 05/11/2025
  I/O Pins
  A0: LCD DB4
  A1: LCD DB3
  A2: LCD DB2
  A3: LCD DB1
  A4: Right Encoder          [PCI PORTC 0x10]
  A5: Left Encoder           [PCI PORTC 0x20]
  D0:
  D1:
  D2:
  D3: 
  D4: 
  D5: Left Motor Reverse     [TCNT0 OC0B] [Pin 7 H-Bridge]
  D6: Left Motor Forward     [TCNT0 OC0A] [Pin 2 H-Bridge]
  D7: LCD RS
  D8: 
  D9:  Right Motor Forward   [TCNT1 OC1A] [Pin 15 H-Bridge]
  D10: Right Motor Reverse    [TCNT1 OC1B] [Pin 10 H-Bridge]
  D11: LCD E
  D12: Right Whisker         [PCI PORTB 0x10]
  D13: Left Whisker          [PCI PORTB 0x20]

  Ideas:
  - ultrasonic sensor hug the left wall
  - whiskers hug the left wall
  - whiskers detection

  NOTE: *AIM JEREMY TO HIT LEFT WALL*
*/

// Global setups
#include "hd44780.h"
unsigned int avg = 0;  // ONE AND ONLY GROSS GLOBAL VARIABLE :D

void setup() {
  cli();

  // * * * * PWM AND MOTOR SETUP * * * *
  TCCR0A = 0xA1;
  TCCR0B = 0x01;
  TCCR1A = 0xA1;
  TCCR1B = 0x01;

  // Pin direction setup
  DDRD = 0x60;   // D5, D6: motor control
  DDRB = 0x06;   // D9, D10: motor control

  // * * * * WHISKER SETUP * * * *
  // Configure pin change interrupts for whiskers
  PCICR = 0x01;  // Enable PORT B
  PCMSK0 = 0x30; // PORT B, D12 and D13

  // Pull-ups for whiskers
  PORTB |= 0x30; // D12, D13
  
  // * * * * ENCODER SETUP * * * *
  // Configure Pin Change Interrupts
  PCICR = 0x02;   // PortC
  PCMSK1 = 0x30;  // A4, A5

  // Pull-ups for encoders
  PORTC |= 0x30;

  // * * * * LCD SETUP * * * *
  lcd_init();  // Initialize LCD

  // TESTING PURPOSES
  Serial.begin(9600);

  sei();
}

// ***** MOTOR CONTROL *****
// Format for motors:
// OCR0A = L motor forward   OCR0B = L motor reverse
// OCR1A = R motor forward   OCR1B = R motor reverse

// Move farward values
void moveForward() {
  OCR0A = 237; OCR0B = 0;
  OCR1A = 255; OCR1B = 0;
  //  Serial.println("moveForward");
}

// Move backwards
void moveBackward() {
  OCR0A = 0; OCR0B = 237;
  OCR1A = 0; OCR1B = 255;
  //  Serial.println("moveBackward");
}

// Turn left values
void turnLeft() {
  OCR0A = 0;   OCR0B = 255;
  OCR1A = 255; OCR1B = 0;
  //  Serial.println("turnLeft");
}

// Turn right values
void turnRight() {
  OCR0A = 255; OCR0B = 0;
  OCR1A = 0; OCR1B = 255;
  //  Serial.println("turnRight");
}

// ***** MAIN LOOP: Hug the Left Wall Logic *****
// LCD Odometer setup using the average value from encoders 
void loop() {

  lcd_clrscr(); // Clear screen

  moveForward(); // Move forward unless our ISR picks up something

  lcd_puts("Jeremy has run: ");
  lcd_goto(0x40);

  unsigned int distanceMM = ((avg * 106) / 100) % 10; // Equation to convert toggles to mm
  unsigned int distanceCM = ((avg * 106L) / 1000) ; // Equation to convert mm to cm

  // Display centimeters
  const char charBufferValue = 6;
  unsigned char charBuffer[charBufferValue];

  Serial.println(distanceCM);
  
  itoa(distanceCM, charBuffer, 10);
  lcd_puts(charBuffer);
  Serial.print(distanceCM); 

  lcd_puts(".");  // Decimal point!

  // Display millimeters
  itoa(distanceMM, charBuffer, 10);
  lcd_puts(charBuffer);
  Serial.print(distanceMM); 

  lcd_puts(" cm");

  _delay_ms(50);  // For refresh rate purposes!!!
}

// * * * * PIN CHANGE ISR FOR WHISKERS * * * *
// Consider moving function logic into logic here to avoid spaghetti code
ISR(PCINT0_vect) {
  Serial.println("ISR");

  unsigned char whiskerLeft = 0;
  unsigned char whiskerRight = 0;

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

// * * * * PIN CHANGE ISR FOR ENCODERS * * * *
ISR(PCINT1_vect) {
  static unsigned int rightA = 1;
  static unsigned int rightB = 1;
  static unsigned int leftA = 1;
  static unsigned int leftB = 1;
  static unsigned int counterRight = 0;
  static unsigned int counterLeft = 0;
  // Calculate encoder values
  rightB = (PINC & 0x10) >> 4;
  leftB = (PINC & 0x20) >> 5;
  if ((rightB != rightA) && !rightB) {
    counterRight++;
  }
  if ((leftB != leftA) && !leftB) {
    counterLeft++;
  }
  leftA = leftB;
  rightA = rightB;
  avg = (counterLeft + counterRight) / 2;
}

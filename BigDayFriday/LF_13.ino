/*
  Demonstration of +13 second track time
  By: Sasha Dauz, Jacob Horstman, Robert Guziec
  Written: April 19, 2025
  Edited: May 1, 2025
  I/O:
  A0:  [INPUT] Left Line Sensor
  A1:  [INPUT] Center Line Sensor
  A2:  [INPUT] Right Line Sensor
  A4:  [INPUT] Right Wheel Encoder
  A5:  [INPUT] Left Wheel Encoder
  ...
  D5:  [OCR0A] Left Motor Forward Speed; Connects to H-Bridge Pin 7
  D6:  [OCR0A] Left Motor Reverse Speed; Connects to H-Bridge Pin 2
  D9:  [OCR1A] Right Motor Forward Speed; Connects to Pin 10 on the H-Bridge
  D10: [OCR1B] Right Motor Reverse; Connects to Pin 15 on the H-Bridge

  Proud of you Jer!
*/

// * * * GROSS ENCODER GLOBALS * * *
volatile unsigned int avg = 0;

// * * * * GROSS GLOBAL VARIABLES * * * *
volatile unsigned char rightyTighty = 0;
volatile unsigned char leftyLoosey = 0;
volatile unsigned char center = 0;

// * * * SPEED CONSTANTS * * *
const unsigned char FWD = 230;
const unsigned char BLK = 215;
const unsigned int HLT = 1817;

void setup() {
  // Disable global interrupts
  cli();
  // Enable internal pull-up
  PORTC |= 0x30;  // A4, A5
  // Left Motor Setup
  TCCR0A = 0xA1;
  TCCR0B = 0x01;
  // Right Motor Setup
  TCCR1A = 0xA1;
  TCCR1B = 0x01;
  // Configure output pins
  DDRD = 0x60; // Output on D5, D6
  DDRB = 0x06; // Output on D9, D10
  // ADC in 8-bit mode, enable interrupts, auto trigger DISABLED
  ADCSRA = 0xCF;
  ADCSRB = 0x00;
  ADMUX = 0x60;
  // Configure Pin Change Interrupts
  PCICR = 0x02;   // PortC
  PCMSK1 = 0x30;  // A4, A5
  // Re-enable global interrupts
  sei();
}

void loop() {
  if (( leftyLoosey < BLK) && (rightyTighty < BLK) && (center >= BLK)) {
      OCR0A = (FWD);    // Left FWD
      OCR1A = (FWD);    // Right FWD
      OCR0A = 0;      // Left BCK
      OCR1B = 0;      // Right BCK
    } // Send it
    if (leftyLoosey < BLK) {
      OCR0A = (FWD);    // Left FWD
      OCR1A = 0;      // Right FWD
      OCR0A = 0;      // Left BCK
      OCR1B = (RVS);      // Right BCK
    } // This^ should turn right
    if (rightyTighty < BLK) {
      OCR0A = 0;      // Left FWD
      OCR1A = (FWD);    // Right FWD
      OCR0A = (RVS);      // Left BCK
      OCR1B = 0;      // Right BCK
    } // This^ should turn left
}

ISR(ADC_vect) {
  static unsigned char sensor = 0;
  switch (sensor) {
    case 0: // Collect LEFT sensor data A0, then change to A2
      leftyLoosey = ADCH;
      ADMUX &= 0xF8;
      ADMUX |= 0x02;
      sensor++;
      break;
    case 1: // Collect RIGHT sensor data A2, then change to A1
      rightyTighty = ADCH;
      ADMUX &= 0xF8;
      ADMUX |= 0x01;
      sensor++;
      break;
    case 2: // Collect Center sensor data A1, then change to A0
      center = ADCH;
      ADMUX &= 0xF8;
      sensor = 0;
      break;
  }
  ADCSRA |= 0x40;
}
  if (avg >= HLT) {
        OCR0A = 0;
        OCR1A = 0;
      }
  ADCSRA |= 0x40; // Start new conversion
}

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

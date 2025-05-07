/*
  Line Following with PI Control
  Written: May 7, 2025
  Edited: -
  I/O:
  A0:  [INPUT] Left Line Sensor
  A1:  [INPUT] Center Line Sensor
  A2:  [INPUT] Right Line Sensor
  A4:  [INPUT] Right Wheel Encoder
  A5:  [INPUT] Left Wheel Encoder
  D3:  [OCR2B] Right Motor Reverse; Connects to Pin 15 on the H-Bridge
  D5:  [OCR0B] Left Motor Forward Speed; Connects to H-Bridge Pin 7
  D6:  [OCR0A] Left Motor Reverse Speed; Connects to H-Bridge Pin 2
  D11: [OCR2A] Right Motor Forward Speed; Connects to Pin 10 on the H-Bridge

  Changes made:
  - PI Control added
  - greg exists in the ISRs
  - moved logic to loop; ISR only collects data and some PI stuff now

  Now with feedback control! Good job Jer.
*/

// * * * ENCODER GLOBALS * * *
volatile unsigned int avg = 0;

// * * * SENSOR DATA * * *
volatile unsigned char rightyTighty = 0;
volatile unsigned char leftyLoosey = 0;
volatile unsigned char center = 0;

// * * * PI CONTROL CONSTANTS * * *
const unsigned char FWD = 230;
const unsigned char BLK = 215;
const unsigned int HLT = 1817;

volatile unsigned char pV = 0;
const unsigned char sP = 123;
volatile unsigned int err = 0;
const unsigned char tau = 255;
volatile unsigned int errHist[255] = {};

void setup() {
  cli();
  PORTC |= 0x30;
  // Motor setup
  TCCR0A = 0xA1;
  TCCR0B = 0x01;
  TCCR2A = 0xA1;
  TCCR2B = 0x01;
  // Outputs
  DDRD = 0x68;
  DDRB = 0x08;
  // ADC
  ADCSRA = 0xCF;
  ADCSRB = 0x00;
  ADMUX = 0x60;
  // Encoder interrupts
  PCICR = 0x02;
  PCMSK1 = 0x30;
  sei();
}

void loop() {
  // Compute sum of error history for integral term
  int sum = 0;
  for (unsigned char j = 0; j < tau; j++) {
    sum += errHist[j];
  }

  // Adjustment using PI formula
  int adjustment = (2 * err) / 10 + (1 * sum) / 100;

  // Modify left and right motor speeds with PI adjustment
  int leftPWM = FWD + adjustment;
  int rightPWM = FWD - adjustment;

  // Kill the motors if the sensors see black
  if (leftyLoosey >= BLK) {
    leftPWM = 0;
  }
  if (rightyTighty >= BLK) {
    rightPWM = 0;
  }

  // You shall not EXCEED the limits
  if (leftPWM > 255) {
    leftPWM = 255;
  }
  if (leftPWM < 0) {
    leftPWM = 0;
  }
  if (rightPWM > 255) {
    rightPWM = 255;
  }
  if (rightPWM < 0) {
    rightPWM = 0;
  }

  // Stop the course if Jer completes track
  if (avg >= HLT) {
    OCR0B = 0;
    OCR2A = 0;
  } else {
    OCR0B = (unsigned char) leftPWM;
    OCR2A = (unsigned char) rightPWM;
  }
}

volatile unsigned char storeREG = 0;
ISR(ADC_vect) {
  storeREG = SREG;

  static unsigned int x = 0;
  static unsigned char sensor = 0;

  switch (sensor) {
    case 0:
      leftyLoosey = ADCH;
      ADMUX &= 0xF8;
      ADMUX |= 0x01;
      sensor++;
      break;
    case 1:
      center = ADCH;
      pV = center;
      err = sP - pV;
      errHist[x] = err;
      x++;
      if (x == tau) {
        x = 0;
      }
      ADMUX &= 0xF8;
      ADMUX |= 0x02;
      sensor++;
      break;
    case 2:
      rightyTighty = ADCH;
      ADMUX &= 0xF8;
      sensor = 0;
      break;
  }

  ADCSRA |= 0x40; // Start next conversion
  SREG = storeREG;
}

volatile unsigned char greg = 0;
ISR(PCINT1_vect) {
  greg = SREG;
  static unsigned int rightA = 1;
  static unsigned int rightB = 1;
  static unsigned int leftA = 1;
  static unsigned int leftB = 1;
  static unsigned int counterRight = 0;
  static unsigned int counterLeft = 0;

  rightB = (PINC & 0x10) >> 4;
  leftB = (PINC & 0x20) >> 5;

  if ((rightB != rightA) && !rightB) counterRight++;
  if ((leftB != leftA) && !leftB) counterLeft++;

  leftA = leftB;
  rightA = rightB;
  avg = (counterLeft + counterRight) / 2;
  SREG = greg;
}

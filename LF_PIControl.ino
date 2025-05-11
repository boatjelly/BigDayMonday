/*
  Line Following with PI Control
  Written: May 7, 2025
  Edited: May 10, 2025
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
  - added runningSum for live integral term
  - added detection buffer for line threshold
  - switched to weighted error based on all sensors (s/o Diego)

  TO DO:
  - see if it works
  - add 3 point turn

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

// PI Constants
// KP and KI control how strong the correction is
// KIDIV is a DIVisor to scale the integral term
// tau is how many error values we remember for integral control
const unsigned char KP = 2;
const unsigned char KI = 1;
const unsigned char KIDIV = 100;
const unsigned char tau = 50;

// error history and running sum
volatile unsigned int err = 0; // current error
volatile unsigned int errHist[tau] = {}; // history of past errors
volatile unsigned int runningSum = 0; // total of recent errors for int term
volatile unsigned char x = 0; // index to keep track of element positiin in errorHist

// weighted error constants
const signed char WL = 2; // left sensor weight
const signed char WC = 0; // center sensor weight
const signed char WR = 2; // right sensor weight

// detection buffer for black line
const unsigned char BLK_BUF = 5; // how much darker than threshold  
const unsigned char BLK_HOLD = 5; // how many times in a row seeing black to AHHHHHHH
volatile unsigned char lHold = 0; // left sensor counter
volatile unsigned char rHold = 0; // right sensor counter

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
  // calculate adjustment
  int adjustment = (KP * err) / 10 + (KI * runningSum) / KIDIV;

  // adjust motor speed based on error
  int leftPWM = FWD + adjustment;
  int rightPWM = FWD - adjustment;

  // if left sensor sees black multiple times, stop left motor
  if (leftyLoosey >= BLK + BLK_BUF) {
    lHold++;
    if (lHold >= BLK_HOLD) leftPWM = 0;
  } else {
    lHold = 0;
  }
  // if right sensor sees black multiple times, stop right motor
  if (rightyTighty >= BLK + BLK_BUF) {
    rHold++;
    if (rHold >= BLK_HOLD) rightPWM = 0;
  } else {
    rHold = 0;
  }

  // make sure that PWM isn't too high or low
  if (leftPWM > 255) leftPWM = 255;
  if (leftPWM < 0) leftPWM = 0;
  if (rightPWM > 255) rightPWM = 255;
  if (rightPWM < 0) rightPWM = 0;

  // stop when at end of track
  if (avg >= HLT) {
    OCR0B = 0;
    OCR2A = 0;
  } else {
    OCR0B = leftPWM;
    OCR2A = rightPWM;
  }
}

volatile unsigned char storeREG = 0;
ISR(ADC_vect) {
  storeREG = SREG;

  static unsigned char sensor = 0;

  switch (sensor) {
    case 0:
      leftyLoosey = ADCH;
      ADMUX = (ADMUX & 0xF8) | 0x01;
      sensor++;
      break;
    case 1:
      center = ADCH;
      ADMUX = (ADMUX & 0xF8) | 0x02;
      sensor++;
      break;
    case 2:
      rightyTighty = ADCH;

      // calculate weighted error with sensor values
      err = (-WL * leftyLoosey) + (WC * center) + (WR * rightyTighty);

      // update runningSum
      // remove old value that's gonna be overwritten
      runningSum -= errHist[x];
      // store newest error at x
      errHist[x] = err;
      // add new error to the sum
      runningSum += err;
      // move index
      x++;
      // back to the beginning if you're at the end of the buffer
      if (x == tau) x = 0;

      ADMUX = (ADMUX & 0xF8);
      sensor = 0;
      break;
  }

  ADCSRA |= 0x40;
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

/*
  Three Point Turn
  Written: May 15, 2025
  I/O:
  A0:
  A1:
  A2:
  A4:  [INPUT] Right Wheel Encoder
  A5:  [INPUT] Left Wheel Encoder

  D5:  [OCR0A] Left Motor Forward Speed; Connects to H-Bridge Pin 7
  D6:  [OCR0B] Left Motor Reverse Speed; Connects to H-Bridge Pin 2
  D9:  [OCR1A] Right Motor Forward Speed; Connects to Pin 10 on the H-Bridge
  D10: [OCR1B] Right Motor Reverse; Connects to Pin 15 on the H-Bridge
*/

// * * * ENCODER GLOBALS * * *
volatile unsigned int avg = 0;
const unsigned int HLTforward = 212;
const unsigned int HLT1 = 212 + HLTforward;
const unsigned int HLT2 = 212 + HLT1;
const unsigned int HLT3 = 70 + HLT2;
const unsigned int HLT4 = 300 + HLT2;

void setup() {
  cli();
  // Wheel Encoder Pull-Up Resistors
  PORTC |= 0x30; // Internal Resistors on A4, A5
  // Motor setup
  TCCR0A = 0xA1;
  TCCR0B = 0x01;
  TCCR1A = 0xA1;
  TCCR1B = 0x01;
  // Outputs
  DDRD = 0x60; // Output on D5, D6
  DDRB = 0x06; // Output on D9, D10
  // Encoder interrupts
  PCICR = 0x02;
  PCMSK1 = 0x30;

  Serial.begin(9600);
  sei();
}

void fwd() {
  OCR0A = 237; OCR0B = 0;
  OCR1A = 255; OCR1B = 0;
}
void one() {
  OCR0A = 140; OCR0B = 0;
  OCR1A = 255; OCR1B = 0;
}

void two() {
  OCR0A = 0; OCR0B = 255;
  OCR1A = 0; OCR1B = 140;
}

void three() {
  OCR0A = 200;   OCR0B = 0;
  OCR1A = 255; OCR1B = 0;
}

void loop() {
  while(avg < HLTforward){
    fwd();
  }
  while(avg < HLT1){
    one();
  }
  while(avg < HLT2){
    two();
  }
  while(avg < HLT3){
    three();
  }
  while(avg < HLT4){
    fwd();
  }
  OCR0A = 0;
  OCR1A = 0;
  OCR0B = 0;
  OCR1B = 0;

  Serial.println(avg);
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

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

  D5:  [OCR0A] Left Motor Forward Speed; Connects to H-Bridge Pin 7
  D6:  [OCR0B] Left Motor Reverse Speed; Connects to H-Bridge Pin 2
  D9:  [OCR1A] Right Motor Forward Speed; Connects to Pin 10 on the H-Bridge
  D10: [OCR1B] Right Motor Reverse; Connects to Pin 15 on the H-Bridge

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

// * * * P CONTROL CONSTANTS * * *
const unsigned char FWD = 120;
const unsigned int HLT = 65535; // haha big number
const unsigned char sP = 14;

// error history and running sum
volatile int lerr = 0;
volatile int rerr = 0;

void setup() {
  cli();
  PORTC |= 0x30;
  // Motor setup
  TCCR0A = 0xA1;
  TCCR0B = 0x01;
  TCCR1A = 0xA1;
  TCCR1B = 0x01;
  OCR0A = FWD;
  OCR1A = FWD;
  // Outputs
  DDRD = 0x60; // Output on D5, D6
  DDRB = 0x06; // Output on D9, D10
  // ADC
  ADCSRA = 0xCF;
  ADCSRB = 0x00;
  ADMUX = 0x60;
  // Encoder interrupts
  PCICR = 0x02;
  PCMSK1 = 0x30;
  Serial.begin(9600);
  sei();
}

void loop() {
  // P Control
  unsigned char kP = (10L * center) / 255;
  OCR0A += (kP * lerr) / 10;
  OCR1A += (kP * rerr) / 10;

  // stop when at end of track
  if (avg >= HLT) {
    OCR0A = 0;
    OCR1A = 0;
  }
  Serial.print(OCR0A); // Left Blue
  Serial.print('\t');
  Serial.print(OCR1A); // Right Red
  Serial.print('\t');
  Serial.println(sP);  // sP Green
}

volatile unsigned char greg = 0;
ISR(ADC_vect) {
  greg = SREG;

  static unsigned char sensor = 0;  // kill this

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
      ADMUX = (ADMUX & 0xF8);
      sensor = 0;
      break;
  }

  lerr =  sP - leftyLoosey;  //[-75, 180]
  rerr =  sP - rightyTighty; //[-75, 180]

  ADCSRA |= 0x40;
  SREG = greg;
}


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

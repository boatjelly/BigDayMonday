/* Maze Day Monday
  By: Sasha Dauz, Robert J. Guziec, Jacob JM Horstman
  Written: May 8th, 2025
  Edited: May 11, 2025
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
  D9:  Right Motor Forward   [TCNT2 OC1A] [Pin 15 H-Bridge]
  D10: Right Motor Reverse   [TCNT2 OC1B] [Pin 10 H-Bridge]
  D11: 
  D12: Right Whisker         [PCI PORTB 0x10]
  D13: Left Whisker          [PCI PORTB 0x20]
*/
void setup() {
  cli();

  // PWM Setup
  TCCR0A = 0xA1;
  TCCR0B = 0x01;
  TCCR1A = 0xA1;
  TCCR1B = 0x01;

  // Pin direction setup
  DDRD = 0x68;   // D5, D6: motor control
  DDRB = 0x06;   // D9, D10: motor control

  sei();
}

// ***** MOTOR CONTROL *****
// Format for motors:
// OCR0A = L motor forward   OCR0B = L motor reverse
// OCR2A = R motor forward   OCR2B = R motor reverse

void go() {
  OCR0A = 200; OCR0B = 0; // Left FWD, RVS
  
  OCR1A = 200; OCR1B = 0; // Right FWD, RVS
}

void loop() {
  go();
}

ISR(PCINT0_vect) {
}

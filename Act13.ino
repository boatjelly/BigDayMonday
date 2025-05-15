/*
Title: Activity 13 (Independent)
Description: Use SPI to write ADC value to 4 7-segment displays via 74HC595s and separate SS pins.
Written: May 15, 2025
I/O Pins:
A0–A3: SS1–SS4 (active low)
A5: Potentiometer input
D11: MOSI
D12: MISO
D13: SCK
*/

void setup() {
  cli();
  
  DDRB = 0x2C; // MOSI (PB3), SCK (PB5), SSx not used
  DDRC |= 0x0F; // Set A0–A3 as output for SS pins
  PORTC |= 0x0F; // Deactivate all SS pins (active low)
  
  // ADC Setup
  ADCSRA = 0xE7; // Enable, Start conversion, Auto-trigger, Prescaler 128
  ADCSRB = 0x00;
  ADMUX = 0x45; // Use A5, AVcc ref, right adjust
  
  // SPI Setup (MSB first, Primary, fosc/4)
  SPCR = 0x50;
  
  sei();
}

void loop() {
  unsigned int potVal = ADC;
  
  // 7-segment digit map (common cathode, active high)
  unsigned char segMap[10] = {0xFC, 0x60, 0xDA, 0xF2, 0x66, 0xB6, 0xBE, 0xE0, 0xFE, 0xF6};
  
  // Extract digits (from least to most significant)
  unsigned char digits[4];
  digits[0] = segMap[potVal % 10];
  digits[1] = segMap[(potVal / 10) % 10];
  digits[2] = segMap[(potVal / 100) % 10];
  digits[3] = segMap[(potVal / 1000) % 10];
  
  // Write each digit to its corresponding SS pin
  for (unsigned char i = 0; i < 4; i++) {
    PORTC &= ~(1 << i); // Activate SS[i]
    writeSPI(digits[i]); // Send data
    PORTC |= (1 << i); // Deactivate SS[i]
  }
  _delay_ms(50);
}

void writeToSPI(char dataToWrite){
    SPDR = dataToWrite;
    while (!(SPSR & (1 << SPIF)));  // Wait until transfer is complete
}

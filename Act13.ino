/*
Title: Activity 13 (Independent)
Description: Use SPI to write ADC value to 4 7-segment displays via 74HC595s and separate SS pins.
Authors: Sasha Dauz, Jacob JM Horstman
Written: May 15, 2025
I/O Pins:
A0–A3: SS1–SS4 (active low) (10s place to 1000s place)
A5: Potentiometer input
D11: MOSI - SER
D12: MISO - QH'
D13: SCK - SRCLK
*/

void setup() {
  cli();

  // Configure outputs
  DDRB = 0x2C;
  DDRC |= 0x0F;
  PORTC |= 0x0F;
  
  // ADC Setup
  ADCSRA = 0xE7;
  ADCSRB = 0x00;
  ADMUX = 0x45;
  
  // SPI Setup (MSB first, Primary, fosc/4)
  SPCR = 0x70;
  
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
    PORTC &= ~(1 << i);
    writeSPI(digits[i]);
    PORTC |= (1 << i);
  }
  _delay_ms(50);
}

void writeSPI(char dataToWrite){
    SPDR = dataToWrite;
    while (!(SPSR & (1 << SPIF)));  // Wait until transfer is complete
}

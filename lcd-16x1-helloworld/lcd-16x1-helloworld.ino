/*
LCD 16x1
  The circuit:
 * LCD RS pin to digital pin 2
 * LCD Enable E pin to digital pin 3
 * Contrast 3 - Resistencia (1k-10k) - GND
 * LCD D4 pin to digital pin 4
 * LCD D5 pin to digital pin 5
 * LCD D6 pin to digital pin 6
 * LCD D7 pin to digital pin 7
 * LCD R/W pin to ground
 * LCD VSS pin to ground
 * LCD VCC (GND) pin to 5V
*/

#include <LiquidCrystal.h>
const int rs = 2, en = 3, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void setup() {
  lcd.begin(16, 1);
}
void loop() {
  lcd.setCursor(0, 1);
  lcd.print("***  LU2FTI  ***");
}


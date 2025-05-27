#include <Adafruit_SH1106.h>
#include <Wire.h>
#include <Rotary.h>
#include <si5351.h>
#include <SPI.h>
#include <Adafruit_GFX.h>

#define OLED_RESET 4
#define IF         2000  // IF en kHz (2 MHz)
#define BAND_INIT  4
#define XT_CAL_F   25000  // CRISTAL CALIBRACIÓN POR DEFECTO
#define S_GAIN     303

#define tunestep   A0    // Encoder SW - PULSACION CORTA CAMBIA STEP, PULSACIÓN LARGA MENÚ CALIBRACIÓN CRISTAL
#define band       A1    // CAMBIA BANDA
#define rx_tx      A2
#define mode_pin   A3    // CAMBIA EL MODO LSB USB
#define adc        A6    // SMETER

Rotary r = Rotary(2, 3);
Adafruit_SH1106 display(OLED_RESET);
Si5351 si5351(0x60);

unsigned long freq, freqold, fstep;
long interfreq = IF, interfreqold = 0;
long cal = XT_CAL_F;
unsigned int smval;
byte encoder = 1;
byte stp, n = 1;
byte count, x, xo;
bool sts = 0;
unsigned int period = 100;
unsigned long time_now = 0;
const int ledPin = 9;
bool usb_mode = true;
bool menu_mode = false;
unsigned long pressStart = 0;
bool sw_pressed = false;

ISR(PCINT2_vect) {
  char result = r.process();
  if (!menu_mode) {
    if (result == DIR_CW) set_frequency(1);
    else if (result == DIR_CCW) set_frequency(-1);
  } else {
    if (result == DIR_CW) {
      cal += 10;
      si5351.set_correction(cal, SI5351_PLL_INPUT_XO);
    } else if (result == DIR_CCW) {
      cal -= 10;
      si5351.set_correction(cal, SI5351_PLL_INPUT_XO);
    }
  }
}

void set_frequency(short dir) {
  if (encoder == 1) {
    if (dir == 1) freq += fstep;
    if (freq >= 225000000) freq = 225000000;
    if (dir == -1) freq -= fstep;
    if (fstep == 1000000 && freq <= 1000000) freq = 1000000;
    else if (freq < 10000) freq = 10000;
  }
  if (encoder == 1) {
    if (dir == 1) n++;
    if (n > 42) n = 1;
    if (dir == -1) n--;
    if (n < 1) n = 42;
  }
}

void setup() {
  Wire.begin();
  display.begin(SH1106_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.display();

  pinMode(ledPin, OUTPUT);
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(tunestep, INPUT_PULLUP);
  pinMode(band, INPUT_PULLUP);
  pinMode(rx_tx, INPUT_PULLUP);
  pinMode(mode_pin, INPUT_PULLUP);

  statup_text();

  si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0);
  si5351.set_correction(cal, SI5351_PLL_INPUT_XO);
  si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA);
  si5351.output_enable(SI5351_CLK0, 1);

  //si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0);
  //si5351.set_correction(cal, SI5351_PLL_INPUT_XO);
  //si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA);   //Power oscillador 0
  //si5351.drive_strength(SI5351_CLK1, SI5351_DRIVE_2MA);   //Power oscillador 1
  //si5351.drive_strength(SI5351_CLK2, SI5351_DRIVE_2MA);   //Power oscillador 2
  //si5351.output_enable(SI5351_CLK0, 1);                   //1 - Enable / 0 - Disable CLK
  //si5351.output_enable(SI5351_CLK1, cwk);
  //si5351.output_enable(SI5351_CLK2, 1);

  PCICR |= (1 << PCIE2);
  PCMSK2 |= (1 << PCINT18) | (1 << PCINT19);
  sei();

  count = BAND_INIT;
  bandpresets();
  stp = 4;
  setstep();
}

void loop() {
  if (interfreq == 0) digitalWrite(ledPin, HIGH);
  else digitalWrite(ledPin, LOW);

  if (freqold != freq || interfreqold != interfreq || xo != x) {
    time_now = millis();
    tunegen();
    freqold = freq;
    interfreqold = interfreq;
    xo = x;
  }

  if (digitalRead(tunestep) == LOW) {
    if (!sw_pressed) {
      pressStart = millis();
      sw_pressed = true;
    } else if (millis() - pressStart > 700) {
      menu_mode = !menu_mode;
      delay(300);
    }
  } else {
    if (sw_pressed && millis() - pressStart < 700 && !menu_mode) {
      setstep();
      delay(300);
    }
    sw_pressed = false;
  }

  if (digitalRead(band) == LOW && !menu_mode) {
    inc_preset();
    delay(300);
  }

  sts = digitalRead(rx_tx) == LOW;

  if (digitalRead(mode_pin) == LOW && !menu_mode) {
    usb_mode = !usb_mode;
    delay(300);
  }

  if ((time_now + period) > millis()) {
    displayfreq();
    layout();
  }

  sgnalread();
}

void tunegen() {
  long if_offset = usb_mode ? IF : -IF;
  si5351.set_freq((freq + (if_offset * 1000ULL)) * 100ULL, SI5351_CLK0);
}

/**********************************************************************************************************
BATIDO O MEZCLA: ej. LSB
if_offset = -2000
(freq + if_offset * 1000) = (7100 - 2000000) = -1992900 kHz
***********************************************************************************************************/

void displayfreq() {
  unsigned int m = freq / 1000000;
  unsigned int k = (freq % 1000000) / 1000;
  unsigned int h = (freq % 1000);

  display.clearDisplay();
  display.setTextSize(2);
  char buffer[15] = "";
  if (m < 1) display.setCursor(41, 1), sprintf(buffer, "%03d.%03d", k, h);
  else if (m < 100) display.setCursor(5, 1), sprintf(buffer, "%2d.%03d.%03d", m, k, h);
  else display.setCursor(5, 1), sprintf(buffer, "%2d.%03d.%02d", m, k, h / 10);
  display.print(buffer);
}

void setstep() {
  switch (stp) {
    case 1: stp = 2; fstep = 10; break;
    case 2: stp = 3; fstep = 100; break;
    case 3: stp = 4; fstep = 1000; break;
    case 4: stp = 5; fstep = 10000; break;
    case 5: stp = 6; fstep = 100000; break;
    case 6: stp = 1; fstep = 1000000; break;
  }
}

void inc_preset() {
  count++;
  if (count > 3) count = 1;
  bandpresets();
  delay(50);
}

void bandpresets() {
  switch (count) {
    #case 1: freq = 145000000; break;
    #case 2: freq = 800000; break;
    #case 3: freq = 3650000; break;
    case 1: freq = 7000000; usb_mode = false; break;
    case 2: freq = 10000000; usb_mode = true; break;
    case 3: freq = 14000000; usb_mode = true; break;
    //para FT8
    #case 4: freq = 7074000; usb_mode = true; break;
    #case 5: freq = 10136000; usb_mode = true; break;
    #case 6: freq = 14074000; usb_mode = true; break;
    //
  }
  si5351.pll_reset(SI5351_PLLA);
  stp = 1;
  setstep();
}

void bandlist() {
  display.setTextSize(2);
  display.setCursor(0, 25);

  ##if (count == 1) display.print("GEN");
  ##if (count == 2) display.print("AM");
  ##if (count == 3) display.print("80m");
  if (count == 1) display.print("40m");
  if (count == 2) display.print("30m");
  if (count == 3) display.print("20m");
  // para FT8
  #if (count == 4) display.print("40m FT8");
  #if (count == 5) display.print("30m FT8");
  #if (count == 6) display.print("20m FT8");
  //
  if (count == 1) interfreq = 0;
  else if (!sts) interfreq = IF;
}

void layout() {
  display.setTextColor(WHITE);
  display.drawLine(0, 20, 127, 20, WHITE);
  display.setTextSize(1);
  display.setCursor(0, 25);

  if (!menu_mode) {
    display.print(usb_mode ? "USB" : "LSB");
    display.setCursor(92, 28);
    display.print(sts ? "TX" : "RX");
    bandlist();  // Muestra la banda actual
  } else {
    display.print(">> CALIBRACION <<");
    display.setCursor(0, 36);
    display.print("CAL: ");
    display.print(cal);
  }

  display.display();
}

void sgnalread() {
  smval = analogRead(adc);
  x = map(smval, 0, S_GAIN, 1, 14);
  if (x > 14) x = 14;
}

void statup_text() {
  display.setTextSize(1);
  display.setCursor(13, 10);
  display.print("Si5351 VFO V3.3");
  display.setCursor(16, 20);
  display.print("Nico Tarquini");
  display.setCursor(14, 36);
  display.setTextSize(3);
  display.print("LU2FTI");
  display.display();
  delay(2000);
}

// 
// 04.08.2024 
// Nicolás Tarquini para LU5FB Aspirante N°12 - https://github.com/nicosistemas
// CW Beacon usando Si5351 y Arduino NANO, adaptado de Z33T - https://www.qsl.net/z33t/
// Baliza CW configurada para VHF a 144.050 mHz. Funciona para todas las bandas hasta 160.000 mHz
//
//
// PINEs:
// Arduino --  Si5351
// A4          SDA         (Resistencia 470 ohm entre contactos)
// A5          SCL         (Resistencia 470 ohm entre contactos)
//
//---------------------------------------------------------------------------------------------------------
#include "si5351.h"

#define PIN_SP 10
#define PIN_TX 13

Si5351 si5351(0x60);

uint16_t duration = 40;    // Velocidad de pitido en milisegundos. A mayor duración pididos más lentos.
uint16_t hz = 500;         // Tono del pitido.
String cw_message = "CQ CQ CQ LU5FB OP NICOLAS";   // Mensaje entre " ".
uint32_t tx = 144050000;   // Frecuencia de transmisión.
//---------------------------------------------------------------------------------------------------------
void setup() {
  pinMode(PIN_TX, OUTPUT);

  si5351.init(SI5351_CRYSTAL_LOAD_8PF, 25002152, 0);  // ajuste del cristal
  si5351.set_freq(tx * SI5351_FREQ_MULT, SI5351_CLK0);
  si5351.output_enable(SI5351_CLK0, 0);
}
//---------------------------------------------------------------------------------------------------------
void loop() {
  cw_string_proc(cw_message);
  delay(500);           // Duración de la pausa al final antes de la señal larga: en ms

  cw(true);
  delay(1000);          // Duración de la señal larga al final: en ms

  cw(false);
  delay(1000);          // Duración de la pausa al final de la señal larga: en ms
}
//---------------------------------------------------------------------------------------------------------
void cw_string_proc(String str) {                      // processing string to characters
  for (uint8_t j = 0; j < str.length(); j++)
    cw_char_proc(str[j]);
}
//---------------------------------------------------------------------------------------------------------
void cw_char_proc(char m) {                            // Procesamiento de caracteres en CW
  String s;

  if (m == ' ') {                                      // Pausa entre palabras
    word_space();
    return;
  }

  if (m > 96)
    if (m < 123)
      m -= 32;

  switch (m) {
    case 'A': s = ".-#";     break;
    case 'B': s = "-...#";   break;
    case 'C': s = "-.-.#";   break;
    case 'D': s = "-..#";    break;
    case 'E': s = ".#";      break;
    case 'F': s = "..-.#";   break;
    case 'G': s = "--.#";    break;
    case 'H': s = "....#";   break;
    case 'I': s = "..#";     break;
    case 'J': s = ".---#";   break;
    case 'K': s = "-.-#";    break;
    case 'L': s = ".-..#";   break;
    case 'M': s = "--#";     break;
    case 'N': s = "-.#";     break;
    case 'O': s = "---#";    break;
    case 'P': s = ".--.#";   break;
    case 'Q': s = "--.-#";   break;
    case 'R': s = ".-.#";    break;
    case 'S': s = "...#";    break;
    case 'T': s = "-#";      break;
    case 'U': s = "..-#";    break;
    case 'V': s = "...-#";   break;
    case 'W': s = ".--#";    break;
    case 'X': s = "-..-#";   break;
    case 'Y': s = "-.--#";   break;
    case 'Z': s = "--..#";   break;

    case '1': s = ".----#";  break;
    case '2': s = "..---#";  break;
    case '3': s = "...--#";  break;
    case '4': s = "....-#";  break;
    case '5': s = ".....#";  break;
    case '6': s = "-....#";  break;
    case '7': s = "--...#";  break;
    case '8': s = "---..#";  break;
    case '9': s = "----.#";  break;
    case '0': s = "-----#";  break;

    case '?': s = "..--..#"; break;
    case '=': s = "-...-#"; break;
    case ',': s = "--..--#"; break;
    case '/': s = "-..-.#";  break;
  }

  for (uint8_t i = 0; i < 7; i++) {
    switch (s[i]) {
      case '.': ti();  break;                          // TI
      case '-': ta();  break;                          // TA
      case '#': char_space(); return;                  // end of Morse code symbol
    }
  }
}
//---------------------------------------------------------------------------------------------------------
void ti() {
  cw(true);                                            // TX TI
  delay(duration);

  cw(false);                                           // stop TX TI
  delay(duration);
}
//---------------------------------------------------------------------------------------------------------
void ta() {
  cw(true);                                            // TX TA
  delay(3 * duration);

  cw(false);                                           // stop TX TA
  delay(duration);
}
//---------------------------------------------------------------------------------------------------------
void char_space() {                                    // 3x, pausa entre palabras
  delay(2 * duration);                                 // 1 from element-end + 2 new
}
//---------------------------------------------------------------------------------------------------------
void word_space() {                                    // 7x, pausa entre palabras
  delay(6 * duration);                                 // 1 from element-end + 6 new
}
//---------------------------------------------------------------------------------------------------------
void cw(bool state) {                                  // TX-CW, TX-LED board
  if (state) {
    si5351.output_enable(SI5351_CLK0, 1);
    digitalWrite(PIN_TX, HIGH);
    tone(PIN_SP, hz);
  }
  else {
    si5351.output_enable(SI5351_CLK0, 0);
    digitalWrite(PIN_TX, LOW);
    noTone(PIN_SP);
  }
}

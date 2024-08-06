### ARDUINO PINES: 
(All to GND)
- __A0__        SW Encoder - Dial
- __A1__        Pulsador 1 - Selector de banda
- __A2__        Pulsador 2 - RX TX
- __A3__        Signal Meter

- __D2__        ENCODER
- __D3__        ENCODER


### Si5351

Resistencias 470 ohms en conectores SDA y SCL entre Arduino y Si5351

- __A4__ - R470 - SDA 
- __A5__ -R470 - SCL
- __VCC__ 5v

### OLED 1.3 SH1106

- __A4__ SDA 
- __A5__ SCL
- __VCC__ 3.3v


Librería: Adafruit_SH1106 https://www.electroniclinic.com/wp-content/uploads/2020/02/Adafruit_SH1106.zip

```console
#define OLED_RESET 4         //Reset required for SH1106
```

```console
Adafruit_SH1106 display(OLED_RESET); // SH1106 config required
```

```console
void setup() {

...
  display.begin(SH1106_SWITCHCAPVCC, 0x3C); // SH1106 config required
  
...
```


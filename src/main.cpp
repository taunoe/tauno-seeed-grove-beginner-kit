/*********************************************************************************
 *  Tauno Erik
 *  Alustatud: 2026.10.16
 *
 *
 *********************************************************************************/

#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <MsTimer2.h>
#include "DHT.h"
#include "LIS3DHTR.h"
#include "Seeed_BMP280.h"

// Sensor status or error codes
typedef enum {
    SENSOR_OK = 0,
    SENSOR_ERR_TIMEOUT,
    SENSOR_ERR_DISCONNECTED,
    SENSOR_ERR_OUT_OF_BOUNDS
} SensorStatus_t;

/***************************************************************************
 * Light Sensor data structure
 * Pin A6
 ***************************************************************************/
typedef struct {
  const uint8_t pin;        
  int value;
  SensorStatus_t status;
} LightSensor_t;

LightSensor_t valgus_andur = {A6, 0, SENSOR_OK};


/***************************************************************************
 * Sound Sensor data structure
 * Pin A2
 ***************************************************************************/
// Sound sensor moving average filter settings
#define SOUND_QUEUE_LEN  10

typedef struct {
  const uint8_t pin;
  int value;
  int queue[SOUND_QUEUE_LEN];
  SensorStatus_t status;
} SoundSensor_t;

SoundSensor_t heli_andur = {A2, 0, {0}, SENSOR_OK};


/***************************************************************************
 * Temperature and Humidity Sensor data structure
 * DHT sensor on pin 3
 ***************************************************************************/
typedef struct {
  const uint8_t pin;        
  float humid;
  float temp;
  SensorStatus_t status;
} TempHumSensor_t;

TempHumSensor_t dht_andur = {3, 0.0, 0.0, SENSOR_OK}; 

DHT dht(dht_andur.pin, DHT11);

/**************************************************************************
 * Pressure Sensor data structure
 * BMP280 sensor on pin A1
 **************************************************************************/
typedef struct {       
  float pressure;
  SensorStatus_t status;
} PressureSensor_t;

PressureSensor_t bmp280_andur = {0.0, SENSOR_OK}; 

BMP280 bmp280;

/**************************************************************************
 * Accelerometer Sensor data structure
 * LIS3DHTR accelerometer
 **************************************************************************/
typedef struct {       
  float x;
  float y;
  float z;
  SensorStatus_t status;
} AccelerometerSensor_t;

AccelerometerSensor_t lis3dht_andur = {0.0, 0.0, 0.0, SENSOR_OK};

LIS3DHTR<TwoWire> accelerometer;




/**************************************************************************
 * LED data structure
 * LED on pin 4
 **************************************************************************/
typedef struct {       
  const uint8_t pin;
  volatile bool state;
} LED_t;

LED_t LED = {4, false};


/**************************************************************************
 * Buzzer data structure
 * Buzzer on pin 5
 **************************************************************************/
typedef struct {       
  const uint8_t pin;
} Buzzer_t;

Buzzer_t BUZZER = {5};


/**************************************************************************
 * Button data structure
 * Button on pin 6
 **************************************************************************/
typedef struct {       
  const uint8_t pin;
  const uint8_t long_press;
  volatile int counter;
} Button_t;

Button_t BTN = {6, 100, 0};


/**************************************************************************
 * Rotary data structure
 * Rotary potentiometer on pin A0
 **************************************************************************/
typedef struct {       
  const uint8_t pin;
} Rotary_t;

Rotary_t ROTARY = {A0};


/**************************************************************************
 * Menu data structure
 **************************************************************************/

enum ProgramMode {
  MODE_NONE = 0,
  MODE_LIGHT = 1,
  MODE_SOUND = 2,
  MODE_TEMP_HUMID = 3,
  MODE_PRESSURE = 4,
  MODE_ACCEL = 5
};

typedef struct {       
  bool isActive;
  ProgramMode mode;
} Menu_t;

Menu_t MENU = {false, MODE_LIGHT};


// --- BITMAPS (Stored in PROGMEM to save RAM) ---
// (Bitmap data is kept the same as the original code)
const unsigned char sound_bmp[] U8X8_PROGMEM = {0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x3e, 0x00, 0x80, 0x3f, 0x0c, 0xc0, 0x3b, 0x0c, 0xfe, 0xb8, 0x0d, 0x7f, 0xb8, 0x0d, 0x03, 0xb8, 0x0d, 0x03, 0xb8, 0x0d, 0x03, 0xb8, 0x0d, 0x03, 0xb8, 0x0d, 0x03, 0xb8, 0x0d, 0xff, 0xb8, 0x0d, 0xc0, 0x39, 0x0c, 0x80, 0x3f, 0x0c, 0x00, 0x3e, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00};
const unsigned char temp_bmp[] U8X8_PROGMEM = {0x00, 0x06, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x09, 0x00, 0x00, 0x09, 0x00, 0x00, 0x09, 0x00, 0x00, 0x09, 0x00, 0x00, 0x09, 0x00, 0x00, 0x09, 0x00, 0x00, 0x09, 0x00, 0x00, 0x09, 0x00, 0x00, 0x09, 0x00, 0x00, 0x09, 0x00, 0x00, 0x09, 0x00, 0x00, 0x09, 0x00, 0x00, 0x09, 0x00, 0x00, 0x09, 0x00, 0x80, 0x19, 0x00, 0x80, 0x19, 0x00, 0xc0, 0x39, 0x00, 0xc0, 0x39, 0x00, 0xc0, 0x39, 0x00, 0xc0, 0x39, 0x00, 0xc0, 0x3f, 0x00, 0x80, 0x1f, 0x00, 0x80, 0x1f, 0x00, 0x00, 0x06, 0x00};
const unsigned char hum_bmp[] U8X8_PROGMEM= {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x09,0x00,0x80,0x10,0x00,0x00,0x00,0x00,0x40,0x20,0x00,0x20,0x40,0x00,0x20,0x40,0x00,0x10,0x80,0x00,0x10,0x80,0x00,0x08,0x00,0x01,0x08,0x00,0x01,0x08,0x00,0x01,0x08,0x00,0x01,0x08,0x00,0x01,0x08,0x00,0x01,0x10,0x80,0x00,0x00,0x00,0x00,0x40,0x20,0x00,0x80,0x10,0x00};
const unsigned char pressure_bmp1[] U8X8_PROGMEM= {0x00,0x00,0x00,0x00,0x06,0x00,0x00,0x06,0x00,0x00,0x06,0x00,0x00,0x06,0x00,0x80,0x96,0x01,0x80,0x1f,0x03,0x00,0x0f,0x06,0x18,0x06,0x04,0x1c,0x04,0x04,0x06,0x00,0x06,0xc6,0xff,0x03,0xc6,0xff,0x01,0x04,0x00,0x00,0xfc,0xff,0x03,0xf8,0xff,0x03,0x00,0x00,0x00,0xfe,0xff,0x07,0xfe,0xff,0x07,0x00,0x00,0x00};
const unsigned char pressure_bmp[] U8X8_PROGMEM= {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xc0,0x00,0x00,0x00,0xc0,0x00,0x00,0x00,0xc0,0x00,0x00,0x00,0xc0,0x00,0x00,0x00,0xc0,0x00,0x00,0x00,0xc0,0x00,0x00,0x00,0xc8,0xc4,0x01,0x00,0xf8,0x87,0x07,0x00,0xf8,0x87,0x07,0x60,0xf0,0x03,0x04,0x60,0xf0,0x03,0x04,0xf0,0xc0,0x00,0x04,0xf0,0xc0,0x00,0x04,0x18,0x00,0x00,0x06,0x18,0x00,0x00,0x06,0x08,0xfc,0xff,0x03,0x08,0xfc,0xff,0x03,0x18,0x00,0x00,0x00,0x18,0x00,0x00,0x00,0xf0,0xff,0xff,0x03,0xf0,0xff,0xff,0x03,0xe0,0xff,0xff,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xfe,0xff,0xff,0x1f,0xfe,0xff,0xff,0x1f,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
const unsigned char light_tmp[] U8X8_PROGMEM = {0x00, 0xc0, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x80, 0xc1, 0x60, 0x00, 0x80, 0xc3, 0x70, 0x00, 0x00, 0xc3, 0x30, 0x00, 0x00, 0xc7, 0x38, 0x00, 0x00, 0x06, 0x18, 0x00, 0x0c, 0xf0, 0x03, 0x0c, 0x3c, 0xfc, 0x0f, 0x0f, 0x78, 0x1e, 0x9e, 0x07, 0x60, 0x07, 0xb8, 0x01, 0x00, 0x03, 0x30, 0x00, 0x80, 0x03, 0x70, 0x00, 0x80, 0x01, 0x60, 0x00, 0xbf, 0x01, 0x60, 0x3f, 0xbf, 0x01, 0x60, 0x3f, 0x80, 0x01, 0x60, 0x00, 0x80, 0x03, 0x70, 0x00, 0x00, 0x03, 0x30, 0x00, 0x60, 0x07, 0xb8, 0x01, 0x78, 0x1e, 0x9e, 0x07, 0x3c, 0xfc, 0x0f, 0x0f, 0x0c, 0xf0, 0x03, 0x0c, 0x00, 0x06, 0x18, 0x00, 0x00, 0xc7, 0x38, 0x00, 0x00, 0xc3, 0x30, 0x00, 0x80, 0xc3, 0x70, 0x00, 0x80, 0xc1, 0x60, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00};
const unsigned char sound_bmp1[] U8X8_PROGMEM = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x60, 0x00, 0x00, 0x70, 0xc0, 0x00, 0x00, 0x48, 0x80, 0x01, 0x00, 0x4c, 0x18, 0x03, 0x00, 0x46, 0x30, 0x02, 0x00, 0x43, 0x60, 0x04, 0x80, 0x41, 0x43, 0x04, 0xfc, 0x40, 0x84, 0x08, 0x46, 0x40, 0x8c, 0x08, 0x46, 0x40, 0x88, 0x08, 0x46, 0x40, 0x10, 0x09, 0x06, 0x40, 0x10, 0x09, 0x06, 0x40, 0x10, 0x19, 0x06, 0x40, 0x10, 0x09, 0x06, 0x40, 0x10, 0x09, 0x46, 0x40, 0x18, 0x09, 0x46, 0x40, 0x88, 0x08, 0x46, 0x40, 0x8c, 0x08, 0xfc, 0x40, 0x86, 0x0c, 0x80, 0x41, 0x43, 0x04, 0x00, 0x43, 0x20, 0x04, 0x00, 0x42, 0x30, 0x02, 0x00, 0x44, 0x0c, 0x03, 0x00, 0x48, 0x80, 0x01, 0x00, 0x50, 0xc0, 0x00, 0x00, 0x60, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
const unsigned char acel_bmp[] U8X8_PROGMEM = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x07, 0x00, 0x00, 0x07, 0x1c, 0x00, 0x80, 0x01, 0x60, 0x00, 0x60, 0x00, 0x80, 0x00, 0x30, 0x00, 0x00, 0x01, 0x10, 0x00, 0x00, 0x02, 0x08, 0x00, 0x00, 0x04, 0x0c, 0x00, 0x00, 0x04, 0x04, 0x20, 0x01, 0x08, 0x04, 0x10, 0x02, 0x08, 0x02, 0xf8, 0x03, 0x08, 0x02, 0x0c, 0x0e, 0x10, 0x02, 0x0a, 0x10, 0x10, 0x02, 0x0a, 0x10, 0x10, 0x02, 0x02, 0x0a, 0x10, 0x02, 0x18, 0x06, 0x18, 0x06, 0x10, 0x02, 0x08, 0x04, 0x20, 0x01, 0x08, 0x04, 0xc0, 0x00, 0x0c, 0x08, 0x00, 0x00, 0x04, 0x18, 0x00, 0x00, 0x02, 0x30, 0x00, 0x00, 0x03, 0x60, 0x00, 0x80, 0x01, 0xc0, 0x00, 0xc0, 0x00, 0x00, 0x03, 0x30, 0x00, 0x00, 0xfc, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
const unsigned char temp_bmp1[] U8X8_PROGMEM = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0xfc, 0x01, 0x00, 0x00, 0xfe, 0xff, 0xff, 0x07, 0xfe, 0xff, 0xff, 0x1f, 0x8f, 0xff, 0xff, 0x3f, 0x0f, 0x00, 0x00, 0x3e, 0x0f, 0x00, 0x00, 0x3e, 0x8f, 0xff, 0xff, 0x3f, 0xfe, 0xff, 0xff, 0x1f, 0xfe, 0xff, 0xff, 0x07, 0xfc, 0x01, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};


U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R2, /* reset=*/U8X8_PIN_NONE);


// FUNCTION PROTOTYPES
void checker_ISR();

void read_light();
void print_light();
void display_light();

void read_sound();
void print_sound();
void display_sound();

void read_temp_humid();
void print_temp_humid();
void display_temp_humid();

void read_pressure();
void print_pressure();
void display_pressure();

void read_accel();
void print_accel();
void display_accel();

void display_menu();
void handle_button();

int filter_sound(int newData, int queue[], int len);


/***********************************************************************/
void setup() {
  Serial.begin(115200);
  Wire.begin();

  // Initialize Pins
  pinMode(LED.pin, OUTPUT);
  pinMode(BUZZER.pin, OUTPUT);
  pinMode(BTN.pin, INPUT);
  pinMode(ROTARY.pin, INPUT);
  pinMode(valgus_andur.pin, INPUT);
  pinMode(heli_andur.pin, INPUT);
  digitalWrite(BUZZER.pin, LOW);

  // Initialize Display
  u8g2.begin();

  // Temperature & Humidity Sensor
  dht.begin();

  // Pressure Sensor
  if (!bmp280.init()) {
    bmp280_andur.status = SENSOR_ERR_DISCONNECTED;
  } else {
    bmp280_andur.status = SENSOR_OK;
  }

  // Accelerometer
  accelerometer.begin(Wire, LIS3DHTR_ADDRESS_UPDATED);
  delay(100);
  if (!accelerometer) {
    lis3dht_andur.status = SENSOR_ERR_DISCONNECTED;
  } else {
    accelerometer.setOutputDataRate(LIS3DHTR_DATARATE_50HZ);
    lis3dht_andur.status = SENSOR_OK;
  }
  
  // Pre-fill the sound queue for the filter
  for (int i = 0; i < SOUND_QUEUE_LEN; i++) {
    heli_andur.queue[i] = analogRead(heli_andur.pin);
    delay(2);
  }

  // Start Timer Interrupt
  MsTimer2::set(10, checker_ISR);
  MsTimer2::start();
}


void loop() {
  // Nuppust anduri valimine ja menüü kuvamine
  handle_button();

  // Seriali CMD käsitlemine
  if (Serial.available() > 0) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd.equalsIgnoreCase("m1")) {
      MENU.mode = MODE_LIGHT;
      tone(BUZZER.pin, 800, 150);
    } else if (cmd.equalsIgnoreCase("m2")) {
      MENU.mode = MODE_SOUND;
      tone(BUZZER.pin, 800, 150);
    } else if (cmd.equalsIgnoreCase("m3")) {
      MENU.mode = MODE_TEMP_HUMID;
      tone(BUZZER.pin, 800, 150);
    } else if (cmd.equalsIgnoreCase("m4")) {
      MENU.mode = MODE_PRESSURE;
      tone(BUZZER.pin, 800, 150);
    } else if (cmd.equalsIgnoreCase("m5")) {
      MENU.mode = MODE_ACCEL;
      tone(BUZZER.pin, 800, 150);
    } else if (cmd.equalsIgnoreCase("m0")) {
      MENU.mode = MODE_NONE;
      tone(BUZZER.pin, 800, 150);
    } else {
      Serial.println("Unknown command. Use m0, m1, m2, m3, m4, or m5.");
    }
  }

  if (MENU.isActive) {
    display_menu();
  } else {
    switch (MENU.mode) {
      case MODE_LIGHT:
        read_light();
        print_light();
        display_light();
        break;
      case MODE_SOUND:
        read_sound();
        print_sound();
        display_sound();
        break;
      case MODE_TEMP_HUMID:
        read_temp_humid();
        print_temp_humid();
        display_temp_humid();
        break;
      case MODE_PRESSURE:
        read_pressure();
        print_pressure();
        display_pressure();
        break;
      case MODE_ACCEL:
        read_accel();
        display_accel();
        print_accel();
        break;
      case MODE_NONE:
      default:
        break;
    }
  }
}

/***************************************************************************/

// INTERRUPT SERVICE ROUTINE (ISR)
void checker_ISR() {
  static int led_blink_counter = 0;

  if (LED.state) {
    digitalWrite(LED.pin, HIGH);
    led_blink_counter++;

    if (led_blink_counter > 10) {
      led_blink_counter = 0;
      LED.state = false;
      digitalWrite(LED.pin, LOW);
    }
  }
  if (digitalRead(BTN.pin) == HIGH) {
    BTN.counter++;
  } else {
    BTN.counter = 0;
  }
}

// BUTTON HANDLING LOGIC
void handle_button() {
  static bool buttonWasPressed = false;

  if (BTN.counter > 0) {
    buttonWasPressed = true;
    LED.state = true;

    if (BTN.counter > BTN.long_press) {
      MENU.isActive = !MENU.isActive;
      tone(BUZZER.pin, 800, 150);
      while (digitalRead(BTN.pin) == HIGH) {
        delay(10);
      }
      BTN.counter = 0;
      buttonWasPressed = false;
    }
  } 
  else if (buttonWasPressed) {
    buttonWasPressed = false;

    if (MENU.isActive) {
      int selected = analogRead(ROTARY.pin) / 204.8 + 1;
      Serial.println( selected);
      if (selected >= MODE_LIGHT && selected <= MODE_ACCEL) {
        MENU.mode = (ProgramMode)selected;
      }
      MENU.isActive = false;
      tone(BUZZER.pin, 500, 50);
    }
  }
}


// DISPLAY FUNCTIONS
void display_menu() {
  int n = analogRead(ROTARY.pin) / 204.8 + 1;
  u8g2.firstPage();

  do {
    u8g2.setFont(u8g2_font_ncenB10_tr);
    u8g2.drawStr(18, 12, "SELECT MODE");
    u8g2.drawHLine(0, 16, 128);
    switch (n) {
      case 1: u8g2.drawXBMP(49, 24, 30, 30, light_tmp); u8g2.drawStr(44, 62, "Light"); break;
      case 2: u8g2.drawXBMP(49, 24, 30, 30, sound_bmp1); u8g2.drawStr(44, 62, "Sound"); break;
      case 3: u8g2.drawXBMP(49, 24, 30, 30, temp_bmp1); u8g2.drawStr(20, 62, "Temp/Humid"); break;
      case 4: u8g2.drawXBMP(49, 24, 30, 30, pressure_bmp); u8g2.drawStr(28, 62, "Pressure"); break;
      case 5: u8g2.drawXBMP(49, 24, 30, 30, acel_bmp); u8g2.drawStr(20, 62, "Accelerometer"); break;
    }
  } while (u8g2.nextPage());
}


void read_light() {
  valgus_andur.value = analogRead(valgus_andur.pin);
}

void print_light() {
  Serial.print("Light: ");
  Serial.println(valgus_andur.value);
}


void display_light() {
  u8g2.firstPage();

  do {
    u8g2.setFont(u8g2_font_t0_16b_mr);
    u8g2.drawCircle(8, 8, 8, U8G2_DRAW_ALL);
    if (valgus_andur.value >= 50 && valgus_andur.value < 100) {
      u8g2.drawDisc(8, 8, 8, U8G2_DRAW_UPPER_LEFT);
    }
    if (valgus_andur.value >= 100 && valgus_andur.value < 200) {
      u8g2.drawDisc(8, 8, 8, U8G2_DRAW_UPPER_RIGHT | U8G2_DRAW_UPPER_LEFT);
    }
    if (valgus_andur.value >= 200 && valgus_andur.value < 350) {
      u8g2.drawDisc(8, 8, 8, U8G2_DRAW_LOWER_LEFT | U8G2_DRAW_UPPER_RIGHT | U8G2_DRAW_UPPER_LEFT);
    }
    if (valgus_andur.value >= 350) {
      u8g2.drawDisc(8, 8, 8, U8G2_DRAW_ALL);
    }
    u8g2.setCursor(26, 32); u8g2.print(F("Light:"));
    u8g2.setCursor(80, 32); u8g2.print(valgus_andur.value);
    u8g2.drawHLine(0, 48, 128);
  } while (u8g2.nextPage());
}


void read_sound() {
  heli_andur.value = analogRead(heli_andur.pin);
}

void print_sound() {
  Serial.print("Sound: ");
  Serial.println(heli_andur.value);
}


void display_sound() {
  int s = filter_sound(heli_andur.value, heli_andur.queue, SOUND_QUEUE_LEN);

  u8g2.firstPage();

  do {
    u8g2.setFont(u8g2_font_t0_16b_mr);
    u8g2.drawXBMP(0, 0, 20, 20, sound_bmp);
    u8g2.setCursor(28, 32); u8g2.print(F("Sound:"));
    u8g2.setCursor(80, 32); u8g2.print(s);
  } while (u8g2.nextPage());
}


void read_temp_humid() {
  dht_andur.humid = dht.readHumidity();
  dht_andur.temp = dht.readTemperature();
}

void print_temp_humid() {
  Serial.print("Temp: ");
  Serial.print(dht_andur.temp, 1);
  Serial.print(" Humid: ");
  Serial.print(dht_andur.humid, 0);
  Serial.println();
}

void display_temp_humid() {
  u8g2.firstPage();

  do {
    u8g2.setFont(u8g2_font_t0_16b_mr);
    u8g2.drawXBMP(0, 0, 20, 30, temp_bmp);
    u8g2.drawXBMP(0, 35, 20, 20, hum_bmp);
    u8g2.drawLine(25, 0, 25, 64);
    u8g2.setCursor(32, 16); u8g2.print(F("Temp:"));
    u8g2.setCursor(78, 16); u8g2.print(dht_andur.temp, 1);
    u8g2.drawStr(110, 16, "C");
    u8g2.setCursor(32, 48); u8g2.print(F("Humid:"));
    u8g2.setCursor(84, 48); u8g2.print(dht_andur.humid, 0);
    u8g2.drawStr(110, 48, "%");
  } while (u8g2.nextPage());
}


void read_pressure() {
  bmp280_andur.pressure = bmp280.getPressure(); // Pa
}

void print_pressure() {
  Serial.print("Pressure: ");
  Serial.println(bmp280_andur.pressure / 100.0F, 2);
}
  
void display_pressure() {
  u8g2.firstPage();

  do {
    u8g2.setFont(u8g2_font_t0_16b_mr);
    u8g2.drawXBMP(0, 0, 20, 20, pressure_bmp1);
    u8g2.setCursor(20, 37); u8g2.print("Pressure:");
    u8g2.setCursor(25, 51); u8g2.print(bmp280_andur.pressure / 100.0F, 2);
    u8g2.print(" hPa");
  } while (u8g2.nextPage());
}


void read_accel() {
  lis3dht_andur.x = accelerometer.getAccelerationX();
  lis3dht_andur.y = - accelerometer.getAccelerationY();
  lis3dht_andur.z = accelerometer.getAccelerationZ();
}

void print_accel() {
  Serial.print("X: "); Serial.print(lis3dht_andur.x, 2);
  Serial.print(" Y: "); Serial.print(lis3dht_andur.y, 2);
  Serial.print(" Z: "); Serial.println(lis3dht_andur.z, 2);
}

void display_accel() {
  // Variables for accelerometer ball position
  static int ball_x = 80;
  static int ball_y = 35;

  if (lis3dht_andur.y > 0.2) {
    ball_y -= (lis3dht_andur.y * 2);
  }
  if (lis3dht_andur.y < -0.2) {
    ball_y -= (lis3dht_andur.y * 2);
  }
  if (lis3dht_andur.x > 0.2) {
    ball_x += (lis3dht_andur.x * 2);
  }
  if (lis3dht_andur.x < -0.2) {
    ball_x += (lis3dht_andur.x * 2);
  }

  ball_x = constrain(ball_x, 54, 106);
  ball_y = constrain(ball_y, 14, 56);

  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_t0_12b_mr);
    u8g2.setCursor(0, 16); u8g2.print(F("X:")); u8g2.print(lis3dht_andur.x, 2);
    u8g2.setCursor(0, 32); u8g2.print(F("Y:")); u8g2.print(lis3dht_andur.y, 2);
    u8g2.setCursor(0, 48); u8g2.print(F("Z:")); u8g2.print(lis3dht_andur.z, 2);
    u8g2.drawFrame(50, 10, 60, 50);
    u8g2.drawDisc(ball_x, ball_y, 3, U8G2_DRAW_ALL);
  } while (u8g2.nextPage());
}


int filter_sound(int newData, int queue[], int len) {
  int max_val = newData;
  int min_val = newData;
  long sum = newData;

  for (int i = len - 1; i > 0; i--) {
    queue[i] = queue[i - 1];

    if (queue[i] > max_val) {
      max_val = queue[i];
    }
    if (queue[i] < min_val) {
      min_val = queue[i];
    }
    sum += queue[i];
  }

  queue[0] = newData;

  if (len > 2) {
    sum = sum - max_val - min_val;
    return sum / (len - 2);
  } else {
    return sum / len;
  }
}

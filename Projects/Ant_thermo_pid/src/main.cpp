#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LCD5110_Graph.h>

#define PIN_ZERO 2
#define PIN_LCD_LIGHT 3
#define PIN_DIMMER 4
#define PIN_TEMP_ENV 7
#define PIN_TEMP_HEATER 6

LCD5110 myGLCD(8,9,10,11,12);
extern uint8_t SmallFont[];
extern uint8_t MediumNumbers[];
extern uint8_t BigNumbers[];
extern uint8_t TinyFont[];

OneWire tempEnvWire(PIN_TEMP_ENV);
DallasTemperature tempEnvSensor(&tempEnvWire);
DeviceAddress tempEnvAddress;
bool tempEnvSensorIsValid = false;

OneWire tempHeaterWire(PIN_TEMP_HEATER);
DallasTemperature tempHeaterSensor(&tempHeaterWire);
DeviceAddress tempHeaterAddress;
bool tempHeaterSensorIsValid = false;

size_t uptimeSec = (size_t)-1;

void PrintUptime() {
  char buf[9];
  byte tm[3];
  size_t seconds = uptimeSec % 86400l;
  tm[0] = seconds / 3600;
  seconds -= tm[0] * 3600l;
  if (tm[0] == 24) {
    tm[0] = 0;
  }
  tm[1] = seconds / 60;
  seconds -= tm[1] * 60;
  tm[2] = seconds;
  for (int i=0; i < 3; ++i) {
    char* pos = buf + i*3;
    if (tm[i] < 10) {
      pos[0] = '0';
      ++pos;
    }
    itoa(tm[i], pos, 10);
    if (i < 2) {
      buf[i*3+2] = ':';
    }
  }
  myGLCD.setFont(SmallFont);
  myGLCD.print(buf, CENTER, 0);
  myGLCD.setFont(TinyFont);
  myGLCD.print(buf, CENTER, 42);
}

void PrintEnvTemp(float value) {
  static float prevTemp = -200.0f;
  if (value != prevTemp) {
    prevTemp = value;
    myGLCD.setFont(MediumNumbers);
    myGLCD.printNumF(value, 2, LEFT, 8, '.', 5, '0');
  }
}

void PrintHeaterTemp(float value) {
  static float prevTemp = -200.0f;
  if (value != prevTemp) {
    prevTemp = value;
    myGLCD.setFont(MediumNumbers);
    myGLCD.printNumF(value, 2, LEFT, 26, '.', 5, '0');
  }
}

void setup() {
  delay(100);
  Serial.begin(9600);
  myGLCD.InitLCD();
  tempEnvSensor.begin();
  if (tempEnvSensorIsValid = tempEnvSensor.getAddress(tempEnvAddress, 0)) {
    tempEnvSensor.setResolution(tempEnvAddress, 12);
  }
  else {
    Serial.println("Env temp sensor not found!");
  }

  tempHeaterSensor.begin();
  if (tempHeaterSensorIsValid = tempHeaterSensor.getAddress(tempHeaterAddress, 0)) {
    tempHeaterSensor.setResolution(tempHeaterAddress, 12);
  }
  else {
    Serial.println("Heater temp sensor not found!");
  }
}

void loop() {
  size_t uptime = millis() / 1000;
  if (uptimeSec == uptime) {
    return;
  }
  uptimeSec = uptime;
  PrintUptime();

  if (!(uptimeSec % 5)) {
    if (tempEnvSensorIsValid) {
      float tempEnv = 0;
      if (tempEnvSensor.requestTemperaturesByAddress(tempEnvAddress)) {
        tempEnv = tempEnvSensor.getTempC(tempEnvAddress);
      }
      PrintEnvTemp(tempEnv);
    }
  }

  if (!((uptimeSec+2) % 5)) {
    if (tempHeaterSensorIsValid) {
      float tempHeater = 0;
      if (tempHeaterSensor.requestTemperaturesByAddress(tempHeaterAddress)) {
        tempHeater = tempHeaterSensor.getTempC(tempHeaterAddress);
      }
      PrintHeaterTemp(tempHeater);
    }
  }
  myGLCD.update();
}
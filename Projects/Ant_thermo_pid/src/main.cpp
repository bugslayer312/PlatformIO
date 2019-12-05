#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LCD5110_Graph.h>
#include <LCD5110Plotter.h>

#define PIN_ZERO 2
#define PIN_LCD_LIGHT 3
#define PIN_DIMMER 4
#define PIN_TEMP_ENV 7
#define PIN_TEMP_HEATER 6

LCD5110 myGLCD(8,9,10,11,12);
//extern uint8_t SmallFont[];
//extern uint8_t MediumNumbers[];
//extern uint8_t BigNumbers[];
extern uint8_t TinyFont[];
extern uint8_t heatBmp[];

LCD5110Plotter plotter(&myGLCD, 22.0f, 30.0f);

OneWire tempEnvWire(PIN_TEMP_ENV);
DallasTemperature tempEnvSensor(&tempEnvWire);
DeviceAddress tempEnvAddress;
bool tempEnvSensorIsValid = false;

OneWire tempHeaterWire(PIN_TEMP_HEATER);
DallasTemperature tempHeaterSensor(&tempHeaterWire);
DeviceAddress tempHeaterAddress;
bool tempHeaterSensorIsValid = false;

size_t uptimeSec = (size_t)-1;
bool needUpdate = false;

/*void PrintUptime() {
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

void AnimateHeat(uint8_t x0) {
  myGLCD.clrRectArea(x0, 0, x0+9, 9);
  static byte y0 = 2;
  myGLCD.drawBitmap(x0 + (y0 % 2), y0, heatBmp, 8, 8);
  if (y0) --y0;
  else y0 = 2;
  needUpdate = true;
} */

void ShowActivity() {
  static const char* symb[] = {"/", "-", "\\", "|"};
  static byte idx = 0;
  myGLCD.setFont(TinyFont);
  myGLCD.print(symb[idx++], RIGHT, 0);
  if (idx >= 4) idx = 0;
  needUpdate = true;
}

void DrawPowerWidget(uint16_t value) {
  static uint16_t prevValue = (uint16_t)-5;
  if (abs(value - prevValue) <= 2) return;
  if (prevValue < 1024 && prevValue > value) {
    myGLCD.clrRectArea(0, 7, 5, 47 - map(prevValue, 0, 1023, 0, 40));
  }
  prevValue = value;
  byte v = map(value, 0, 1023, 0, 40);
  byte stepVal = min(v, 5);
  myGLCD.fillRect(0, 47 - stepVal, 2, 47);
  if (v > 6) {
    stepVal = min(v, 12);
    myGLCD.fillRect(0, 47 - stepVal, 3, 40);
    if (v > 13) {
      stepVal = min(v, 19);
      myGLCD.fillRect(0, 47 - stepVal, 3, 33);
      if (v > 20) {
        stepVal = min(v, 26);
        myGLCD.fillRect(0, 47 - stepVal, 4, 26);
        if (v > 27) {
          stepVal = min(v, 33);
          myGLCD.fillRect(0, 47 - stepVal, 4, 19);
          if (v > 34) {
            myGLCD.fillRect(0, 47 - v, 5, 12);
          }
        }
      }
    }
  }
  myGLCD.setFont(TinyFont);
  myGLCD.print("   ", 0, 0);
  myGLCD.printNumI(map(value, 0, 1023, 0, 100), 0, 0);
  needUpdate = true;
}

/*void PrintEnvTemp(float value) {
  static float prevTemp = -200.0f;
  if (value != prevTemp) {
    prevTemp = value;
    myGLCD.setFont(MediumNumbers);
    myGLCD.printNumF(value, 2, CENTER, 10, '.', 5, '0');
    needUpdate = true;
  }
} */

void PrintHeaterTemp(float value) {
  static float prevTemp = -200.0f;
  if (value != prevTemp) {
    prevTemp = value;
    myGLCD.setFont(TinyFont);
    myGLCD.printNumF(value, 2, 50, 0, '.', 5, '0');
    myGLCD.drawCircle(71, 1, 1);
    myGLCD.print("C", 74, 0);
    needUpdate = true;
  }
}

void setup() {
  delay(100);
  Serial.begin(9600);
  myGLCD.InitLCD();
  plotter.InitSize(7, 8, 83, 47);
  myGLCD.drawRect(6, 7, 83, 47);
  myGLCD.update();
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
  DrawPowerWidget(analogRead(0));

  size_t uptime = millis() / 1000;
  if (uptimeSec != uptime) {
    uptimeSec = uptime;
    ShowActivity();

    /*if (!(uptimeSec % 10)) {
      if (tempEnvSensorIsValid) {
        float tempEnv = 0;
        if (tempEnvSensor.requestTemperaturesByAddress(tempEnvAddress)) {
          tempEnv = tempEnvSensor.getTempC(tempEnvAddress);
        }
        PrintEnvTemp(tempEnv);
      }
    } */

    if (!((uptimeSec+1) % 5)) {
      if (tempHeaterSensorIsValid) {
        float tempHeater = 0;
        if (tempHeaterSensor.requestTemperaturesByAddress(tempHeaterAddress)) {
          tempHeater = tempHeaterSensor.getTempC(tempHeaterAddress);
        }
        PrintHeaterTemp(tempHeater);
        plotter.Push(tempHeater);
      }
    }
  }
  if (needUpdate) {
    myGLCD.update();
    needUpdate = false;
  }
}
// #include <EEPROM.h>
#include <LiquidCrystal.h>
#include <OneWire.h>
#include <DallasTemperature.h>
// #include <avr/wdt.h>

#define KEY_NONE 1023
#define KEY_SELECT 640
#define KEY_LEFT 408
#define KEY_DOWN 255
#define KEY_UP 100
#define KEY_RIGHT 0

byte arrowRight[8] = {
  0b00000,
  0b00100,
  0b00010,
  0b11111,
  0b00010,
  0b00100,
  0b00000,
  0b00000
};

byte arrowUp[8] = {
  0b11111,
  0b00100,
  0b01110,
  0b10101,
  0b00100,
  0b00100,
  0b00100,
  0b00000
};

byte arrowDown[8] = {
  0b00100,
  0b00100,
  0b00100,
  0b10101,
  0b01110,
  0b00100,
  0b11111,
  0b00000
};

enum SystemState
{
  ssNormal = 1,
  ssTopEdit = 2,
  ssBottomEdit = 3
};

//const int pinTemp = 43;
const int pinTemp = 9;
//const int pinHeater = 46;
const int pinHeater = 12;

// LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
LiquidCrystal lcd(7, 8, 2, 3, 5, 6);
OneWire oneWire(pinTemp);
DallasTemperature tempSensors(&oneWire);
DeviceAddress address;

const unsigned long tempPeriodicityMs = 10000;
const unsigned long blinkPeriodicityMs = 500;
const unsigned long initDelayMs = 5000;
const unsigned long maxUptimeSec = 86400l; // 60*60*24

const float editTempDelta = 0.10f;

const int borderTempNumPos = 10;
const int upDownArrowsPos = 15;
const int selectArrowPos = 8;

const byte topSaveFlag = 0x07;
const byte bottomSaveFlag = topSaveFlag << 4;

bool g_isValidSensor = false;

SystemState g_sysState = ssNormal;
int g_keyPressed = KEY_NONE;

unsigned long g_tempEventCounter = 0;
unsigned long g_blinkCounter = 0;
unsigned long g_uptimeSec = initDelayMs / 1000;

bool g_blinkOn = true;

byte g_brightness = 64;                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           ;

float g_topTemp = 27.0f;
float g_bottomTemp = 25.0f;
float g_displayedTopTemp = g_topTemp;
float g_displayedBottomTemp = g_bottomTemp;

void SetHeating(bool value)
{
  Serial.print("Set heating: ");
  Serial.println(value);
  digitalWrite(pinHeater, value ? LOW : HIGH);
}

void SetBrightness(byte value)
{
  analogWrite(10, value);
}

void IncreaseBrightness()
{
  g_brightness += 32;
  SetBrightness(g_brightness);
}

void DisplayTemperature(float temp)
{
  lcd.setCursor(0, 1);
  lcd.print(temp);
}

void DisplayBorderTemperature(bool top)
{
  int pos = borderTempNumPos;
  const int row = top ? 0 : 1;
  const float temp = top ? g_displayedTopTemp : g_displayedBottomTemp;
  if (temp <= -10.0f)
  {
    --pos;
  }
  else if (temp < 10.0f)
  {
    lcd.setCursor(pos, row);
    lcd.write(' ');
    ++pos;
  }
  lcd.setCursor(pos, row);
  lcd.print(temp);
}

void DisplayGeneral()
{
  lcd.setCursor(2, 0);
  lcd.write(':');
  lcd.setCursor(5, 0);
  lcd.write(':');
  lcd.setCursor(upDownArrowsPos, 0);
  lcd.write(byte(1));
  lcd.setCursor(upDownArrowsPos, 1);
  lcd.write(byte(2));
}

void ClearArrowPlaces()
{
  lcd.setCursor(selectArrowPos, 0);
  lcd.write(' ');
  lcd.setCursor(selectArrowPos, 1);
  lcd.write(' ');
  g_blinkOn = true;
  g_blinkCounter = millis() - blinkPeriodicityMs;
}

void DisplayBlinkingArrow(bool topRow, bool blinkOn)
{
  lcd.setCursor(selectArrowPos, topRow ? 0 : 1);
  lcd.write(blinkOn ? byte(0) : ' ');
}

void DisplayTwoDigits(byte value, int pos)
{
  lcd.setCursor(pos, 0);
  if (value < 10)
  {
    lcd.print('0');
    // lcd.setCursor(pos + 1, 0);
  }
  lcd.print(value);
}

void DisplayUptime()
{
  Serial.println("Display uptime");
  unsigned long s = g_uptimeSec % 360000l;
  byte h = s / 3600;
  s -= h * 3600l;
  byte m = s / 60;
  s -= m * 60;
  DisplayTwoDigits(h, 0);
  DisplayTwoDigits(m, 3);
  DisplayTwoDigits(s, 6);
}

void DisplayLiveString(char* str, int count, int del)
{
  for (int i=0; i < count; ++i)
  {
    lcd.write(str[i]);
    delay(del);
  }
}

void UpdateState(int newState)
{
  if (newState == g_sysState)
    return;
  if (g_sysState != ssNormal)
  {
    if (g_topTemp != g_displayedTopTemp)
    {
      g_topTemp = g_displayedTopTemp;
      SaveBorderTemp(true);
    }
    if (g_bottomTemp != g_displayedBottomTemp)
    {
      g_bottomTemp = g_displayedBottomTemp;
      SaveBorderTemp(false);
    }
  }
  g_sysState = (SystemState)newState;
}

void LoadBorderTemp(bool top)
{
  /*byte data = EEPROM.read(0);
  float* temp = 0;
  int offset = 1;
  if (top)
  {
    if ((data & 0x0f) == topSaveFlag)
    {
      temp = &g_topTemp;
    }
  }
  else
  {
    if ((data & 0xf0) == bottomSaveFlag)
    {
      temp = &g_bottomTemp;
      offset += 4;
    }
  }
  if (!temp)
    return;
  int intValue = 0;
  for (int i = 0; i < 4; ++i)
  {
    data = EEPROM.read(offset + i);
    intValue |= (int)data << i*8;
  }
  //Serial.print("Loaded int temp: ");
  //Serial.println(intValue);
  *temp = intValue * 0.01f; */
}

void SaveBorderTemp(bool top)
{
  /*const int offset = top ? 1 : 5;
  byte value = EEPROM.read(0);
  if (top)
  {
    value &= 0xf0;
    value |= topSaveFlag;
  }
  else
  {
    value &= 0x0f;
    value |= bottomSaveFlag;
  }
  EEPROM.write(0, value);
  //Serial.print("Save temp: ");
  //Serial.print(top ? g_topTemp : g_bottomTemp);
  //Serial.print(", ");
  int intVal = (int)(round(100.0f * (top ? g_topTemp : g_bottomTemp)));
  //Serial.println(intVal);
  for (int i = 0; i < 4; ++i)
  {
    value = (byte)((intVal & (0xff << i*8)) >> i*8);
    EEPROM.write(offset + i, value);
  }*/
}

void OnKeyPressed(int key)
{
  switch(key)
  {
    case KEY_UP:
      UpdateState((g_sysState == ssNormal) ? ssBottomEdit : (SystemState)(g_sysState - 1));
      break;
    case KEY_DOWN:
      UpdateState((g_sysState == ssBottomEdit) ? ssNormal : (SystemState)(g_sysState + 1));
      break;
    case KEY_LEFT:
    case KEY_RIGHT:
      {
        if (g_sysState == ssNormal)
          break;
        float delta = (key == KEY_LEFT) ? -editTempDelta : editTempDelta;
        if (g_sysState == ssTopEdit)
        {
          g_displayedTopTemp += delta;
          DisplayBorderTemperature(true);
        }
        else
        {
          g_displayedBottomTemp += delta;
          DisplayBorderTemperature(false);
        }
      }
      break;
    case KEY_SELECT:
      IncreaseBrightness();
      break;
  }
  ClearArrowPlaces();
}

void CheckKeyPressed()
{
  int key = analogRead(0);
  if (key < 90)
  {
    key = KEY_RIGHT;
  }
  else if (key < 200)
  {
    key = KEY_UP;
  }
  else if (key < 400)
  {
    key = KEY_DOWN;
  }
  else if (key < 600)
  {
    key = KEY_LEFT;
  }
  else if (key < 800)
  {
    key = KEY_SELECT;
  }
  else
  {
    key = KEY_NONE;
  }
  if (key != g_keyPressed)
  {
    g_keyPressed = key;
    if (g_keyPressed != KEY_NONE)
      OnKeyPressed(key);
  }
}

void CheckTemperature()
{
  Serial.println("Requesting temperature...");
  if (!tempSensors.requestTemperaturesByAddress(address))
  {
    Serial.println("Temperature request failed!");
    lcd.setCursor(0, 1);
    lcd.print("Err ");
    SetHeating(false);
    return;
  }
  
  const float temp = tempSensors.getTempC(address);
  Serial.print("Temperature: ");
  Serial.println(temp);
  DisplayTemperature(temp);
  if (temp <= g_bottomTemp)
  {
    SetHeating(true);
  }
  else if (temp >= g_topTemp)
  {
    SetHeating(false);
  }
}

void setup() {
  //wdt_disable();
  Serial.begin(9600);
  lcd.begin(16, 2);
  SetBrightness(g_brightness);
  DisplayLiveString("Starting...", 11, 100);
  lcd.blink();
  delay(initDelayMs - 11*100);
  lcd.noBlink();
  // wdt_enable(WDTO_8S);
  Serial.println("Watchdog enabled");
  lcd.clear();
  
  LoadBorderTemp(true);
  LoadBorderTemp(false);
  g_displayedTopTemp = g_topTemp;
  g_displayedBottomTemp = g_bottomTemp;
  
  pinMode(pinHeater, OUTPUT);
  digitalWrite(pinHeater, HIGH);
  
  lcd.createChar(0, arrowRight);
  lcd.createChar(1, arrowUp);
  lcd.createChar(2, arrowDown);
  
  tempSensors.begin();
  g_isValidSensor = tempSensors.getAddress(address, 0);
  if (g_isValidSensor)
  {
    DisplayGeneral();
    DisplayUptime();
    DisplayBorderTemperature(true);
    DisplayBorderTemperature(false);
    tempSensors.setResolution(address, 12);
    CheckTemperature();
  }
  else
  {
    Serial.println("Temperature sensor access failed!");
    lcd.setCursor(0, 0);
    lcd.print("Error sensor");
  }
}

void loop() {
  if (g_isValidSensor)
  {
    CheckKeyPressed();
    delay(50);
    unsigned long ms = millis();
    unsigned long uptime = ms / 1000;
    if (uptime != g_uptimeSec)
    {
      if (uptime >= maxUptimeSec)
      {
        lcd.clear();
        DisplayLiveString("Restarting...", 13, 100);
        lcd.blink();
        delay(10000);
      }
      g_uptimeSec = uptime;
      DisplayUptime();
    }
    
    if (g_sysState != ssNormal && abs(ms - g_blinkCounter) >= blinkPeriodicityMs)
    {
      g_blinkCounter = ms;
      DisplayBlinkingArrow(g_sysState == ssTopEdit, g_blinkOn);
      g_blinkOn = !g_blinkOn;
    }
    if (abs(ms - g_tempEventCounter) >= tempPeriodicityMs)
    {
      g_tempEventCounter = ms;
      CheckTemperature();
    }
  }
  // wdt_reset();
}

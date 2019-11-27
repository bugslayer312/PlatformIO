#include <timer-api.h>
#include <timer_setup.h>

//timer_init_ISR_500KHz(TIMER_DEFAULT);
    //timer_init_ISR_200KHz(TIMER_DEFAULT);
    //timer_init_ISR_100KHz(TIMER_DEFAULT);
    //timer_init_ISR_50KHz(TIMER_DEFAULT);
    //timer_init_ISR_20KHz(TIMER_DEFAULT);
    //timer_init_ISR_10KHz(TIMER_DEFAULT);
    //timer_init_ISR_5KHz(TIMER_DEFAULT);
    //timer_init_ISR_2KHz(TIMER_DEFAULT);
    //timer_init_ISR_1KHz(TIMER_DEFAULT);
    //timer_init_ISR_500Hz(TIMER_DEFAULT);
    //timer_init_ISR_200Hz(TIMER_DEFAULT);
    //timer_init_ISR_100Hz(TIMER_DEFAULT);
    //timer_init_ISR_50Hz(TIMER_DEFAULT);
    //timer_init_ISR_20Hz(TIMER_DEFAULT);
    //timer_init_ISR_10Hz(TIMER_DEFAULT);
    //timer_init_ISR_5Hz(TIMER_DEFAULT);
    //timer_init_ISR_2Hz(TIMER_DEFAULT);
    //timer_init_ISR_1Hz(TIMER_DEFAULT);

#define __TIMER__ TIMER_DEFAULT

#if F_CPU > 8000000L
  #define TIMER_START() timer_init_ISR_2KHz(__TIMER__)
  #define MAX_TICK 20
#else
  #define TIMER_START() timer_init_ISR_5KHz(__TIMER__)
  #define MAX_TICK 25
#endif

#define TIMER_STOP() timer_stop_ISR(__TIMER__)

int zeroPin = 2;
int dimPin = 4;
volatile int tick, dimmer;

void setup() {
  //Serial.begin(9600);
  //Serial.println(F_CPU);
  pinMode(dimPin, OUTPUT);
  digitalWrite(dimPin, 0);
  pinMode(zeroPin, INPUT);
  dimmer = map(analogRead(0), 0, 1023, 0, MAX_TICK);
  attachInterrupt(0, detect_up, FALLING);
  TIMER_START();
}

void loop() {
  dimmer = map(analogRead(0), 0, 1023, 0, MAX_TICK);
  delay(5);
}

void timer_handle_interrupts(int timer) {
  //Serial.println(tick);
  tick++;
  if (tick >= MAX_TICK) {
    tick = 0;
  }
  digitalWrite(dimPin, tick >= dimmer ? 1 : 0);
}

void detect_up() {
  //Serial.print("UP:");
  //Serial.println(dimmer);
  //TIMER_STOP();
  //TIMER_START();
  tick = 0;
  attachInterrupt(0, detect_down, RISING);
}

void detect_down() {
  //Serial.println("DOWN");
  tick = 0;
  attachInterrupt(0, detect_up, FALLING);
}

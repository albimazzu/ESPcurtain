#include <Arduino.h>
#include "debounceInterrupt.h"
#include "FireTimer.h"

#define PIN_DOWN_CMD  6
#define PIN_UP_CMD  7
#define PIN_ACIN_1  1
#define PIN_ACIN_2  2
#define PIN_ACIN_3  5
#define PIN_ACIN_4  4
#define PIN_SENSE   3
#define PIN_485DIR   10
#define PIN_USRBTN   8
#define PIN_STATUSLED   9
#define PIN_IN1   0
#define PIN_IN2   19

// Istanza della classe DebounceInterrupt
DebounceInterrupt debounceInterruptUp(0, 80, PIN_ACIN_1, 50000);
DebounceInterrupt debounceInterruptDown(1, 80, PIN_ACIN_2, 50000);

FireTimer TIMER_CurtainMove;
FireTimer TIMER_Heartbeat;

bool OUT_CurtainUp = false;
bool OUT_CurtainDown = false;

int STEP_curtainMove = 0;
int oldSTEP_curtainMove = 1;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("ESPcurtain startup");
  pinMode(PIN_UP_CMD, OUTPUT);
  pinMode(PIN_DOWN_CMD, OUTPUT);
  pinMode(PIN_STATUSLED, OUTPUT);
  pinMode(PIN_485DIR, OUTPUT);

  pinMode(PIN_ACIN_1, INPUT);
  pinMode(PIN_ACIN_2, INPUT);
  pinMode(PIN_ACIN_3, INPUT);
  pinMode(PIN_ACIN_4, INPUT);
  pinMode(PIN_SENSE, INPUT);
  pinMode(PIN_USRBTN, INPUT);

  TIMER_CurtainMove.begin(23000);
  TIMER_Heartbeat.begin(500);
  
}


void loop() {

  TIMER_Heartbeat.start();
  if(TIMER_Heartbeat.fire())
  {
    TIMER_Heartbeat.stop();
    digitalWrite(PIN_STATUSLED,!digitalRead(PIN_STATUSLED));
  }
  
  digitalWrite(PIN_UP_CMD, OUT_CurtainUp);
  digitalWrite(PIN_DOWN_CMD, OUT_CurtainDown);

  if(STEP_curtainMove != oldSTEP_curtainMove)
  {
    oldSTEP_curtainMove = STEP_curtainMove;
    Serial.println("STEP_curtainMove="+String(STEP_curtainMove));
  }

  switch (STEP_curtainMove)
  {
    //IDLE
    case 0:    
      OUT_CurtainDown = false;
      OUT_CurtainUp = false;
      TIMER_CurtainMove.stop();
      if(debounceInterruptDown.isPressed())
        STEP_curtainMove = 2;
      else if(debounceInterruptUp.isPressed())
        STEP_curtainMove = 1;
    break;

    //Curtain UP
    case 1:
      OUT_CurtainUp = true;
      TIMER_CurtainMove.start();
      STEP_curtainMove = 3;
    break;

    //Curtain DOWN
    case 2:
      OUT_CurtainDown = true;
      TIMER_CurtainMove.start();
      STEP_curtainMove = 3;
    break;

    //wait button release
    case 3:
      if(!debounceInterruptDown.isPressed() && !debounceInterruptUp.isPressed())
        STEP_curtainMove = 4;
    break;

    //Check stop action
    case 4:
      if(TIMER_CurtainMove.fire() || debounceInterruptDown.isPressed() || debounceInterruptUp.isPressed())
      {
        TIMER_CurtainMove.stop();
        OUT_CurtainUp = false;
        OUT_CurtainDown = false;
        STEP_curtainMove = 5;
      }
    break;    

    //wait button release
    case 5:
      if(!debounceInterruptDown.isPressed() && !debounceInterruptUp.isPressed())
      STEP_curtainMove = 0;
    break;
  
  default:
    break;
  }


  
}


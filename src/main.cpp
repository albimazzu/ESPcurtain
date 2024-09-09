#include <Arduino.h>
#include "SPIFFS.h"
#include <FS.h>
#include <ArduinoJson.h> // JSON parsing

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

#define DEFAULT_TIME_fullShutterMove 10000
#define CALIBRATION_PRESSED_TIME 5000 //msec, when up/down buttons are pressed more than this time start calibration

// Istanza della classe DebounceInterrupt
DebounceInterrupt debounceInterruptUp(0, 80, PIN_ACIN_1, 100000); //time in microseconds
DebounceInterrupt debounceInterruptDown(1, 80, PIN_ACIN_2, 100000);

FireTimer TIMER_ShutterMove;
FireTimer TIMER_RelaySpikeFilter;
FireTimer TIMER_Heartbeat;

bool IN_CommandUp = false;
bool IN_CommandDown = false;
bool IN_ACIN_3 = false;
bool IN_ACIN_4 = false;

bool OUT_ShutterUp = false;
bool OUT_ShutterDown = false;

int STEP_shutterMove = 0;
int oldSTEP_shutterMove = 1;

unsigned long upDownPressedTime = 0;    //This variable count how much time up or down are kept pressed
unsigned long TIME_fullShutterMove = 0; //Time in millis needed for complete shutter movement (open or close)

void IO_sync();
void heartbeat();
void stateMachine();

//SPIFFS functions
void setupSPIFFS();
void loadConfig();
void saveConfig();


void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("ESPshutter startup");
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

  TIMER_Heartbeat.begin(500);  
  TIMER_RelaySpikeFilter.begin(200);


  setupSPIFFS();
  loadConfig();  // Load config.json var values
}


void loop() {

  heartbeat();
  IO_sync();
  stateMachine();

}

void stateMachine()
{

  if(STEP_shutterMove != oldSTEP_shutterMove)
  {
    oldSTEP_shutterMove = STEP_shutterMove;
    Serial.println("STEP_shutterMove="+String(STEP_shutterMove));
  }

  switch (STEP_shutterMove)
  {
    //IDLE
    case 0:    
      OUT_ShutterDown = false;
      OUT_ShutterUp = false;
      TIMER_ShutterMove.stop();
      if(IN_CommandDown != IN_CommandUp)
      {
        OUT_ShutterUp = IN_CommandUp;
        OUT_ShutterDown = IN_CommandDown;
        upDownPressedTime = millis();
        TIMER_ShutterMove.begin(TIME_fullShutterMove);
        TIMER_ShutterMove.start();
        STEP_shutterMove = 1;
      }
    break;

    //wait button release
    case 1:
      //If button is pressed more than CALIBRATION_PRESSED_TIME, start calibration process.
      if(millis()- upDownPressedTime > CALIBRATION_PRESSED_TIME)
      {
        Serial.println("Starting time calibration");
        STEP_shutterMove = 100;
      }
      if(!IN_CommandDown && !IN_CommandUp)
        STEP_shutterMove = 2;
    break;

    //Check stop action
    case 2:
      if(TIMER_ShutterMove.fire() || IN_CommandDown || IN_CommandUp)
      {
        TIMER_ShutterMove.stop();
        OUT_ShutterUp = false;
        OUT_ShutterDown = false;
        STEP_shutterMove = 3;
      }
    break;        

    //Relay spike filtering
    case 3:
      TIMER_RelaySpikeFilter.start();
      if(!IN_CommandDown && !IN_CommandUp && TIMER_RelaySpikeFilter.fire())
      {
        TIMER_RelaySpikeFilter.stop();        
        STEP_shutterMove = 0;
      }
    break;    

    //calibration phase
    case 100:
      if(!IN_CommandDown && !IN_CommandUp)
      {
        TIME_fullShutterMove  = millis()-upDownPressedTime;
        Serial.println("New TIME_fullShutterMove value:"+String(TIME_fullShutterMove));
        saveConfig();
        STEP_shutterMove = 3;
      }
      break;
  
    default:
      Serial.println("bad STEP_shutterMove!");
      STEP_shutterMove = 0;
      break;
  }

}

void heartbeat()
{
  TIMER_Heartbeat.start();
  if(TIMER_Heartbeat.fire())
  {
    TIMER_Heartbeat.stop();
    digitalWrite(PIN_STATUSLED,!digitalRead(PIN_STATUSLED));
  }  
}

void IO_sync()
{
  digitalWrite(PIN_UP_CMD, OUT_ShutterUp);
  digitalWrite(PIN_DOWN_CMD, OUT_ShutterDown);

  IN_CommandUp = !digitalRead(PIN_USRBTN);
  //IN_CommandUp = debounceInterruptUp.isPressed();
  IN_CommandDown = debounceInterruptDown.isPressed();
}

// Init SPIFFS
void setupSPIFFS() {
  if (!SPIFFS.begin(true)) {
    Serial.println("Failed to mount SPIFFS");
  } else {
    Serial.println("SPIFFS mounted successfully");
  }
}

//Loading config.json
void loadConfig() {
  File file = SPIFFS.open("/config.json", "r");
  if (!file) {
    Serial.println("Failed to open config file");
    return;
  }

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, file);
  if (error) {
    Serial.println("Failed to read config file, using default TIME_fullShutterMove");
    TIME_fullShutterMove = DEFAULT_TIME_fullShutterMove;
  } else {
    TIME_fullShutterMove = doc["TIME_fullShutterMove"] | DEFAULT_TIME_fullShutterMove;
    Serial.println("Loaded TIME_fullShutterMove: " + String(TIME_fullShutterMove));
  }
  file.close();
}

// Save data into config.json
void saveConfig() {
  File file = SPIFFS.open("/config.json", "w");
  if (!file) {
    Serial.println("Failed to open config file for writing");
    return;
  }

  JsonDocument doc;
  doc["TIME_fullShutterMove"] = TIME_fullShutterMove;
  
  if (serializeJson(doc, file) == 0) {
    Serial.println("Failed to write to config file");
  }
  
  file.close();
}
#define DEBUG_ENABLE 0

typedef enum
{
  FLOAT_UP =0,
  FLOAT_DOWN
}_float_state;

typedef enum
{
  FFD_MODE =0,
  FTD_MODE
}_Sysmode;

typedef enum
{
  RAIN_NONE = 0,
  RAINNING     //1
  
}_Rainstate;

typedef enum
{
    FTD_STATE_CHECK_FLOAT=0,
    FTD_STATE_OPEN_VALVE,
    FTD_STATE_CLOSE_VALVE,
    FTD_STATE_ALARM,  
    FTD_STATE_DONE
}_FTD_state;

#define POWER_OK_PIN              0
#define FLOAT_SWITCH_PIN          1
#define MUTE_PIN                  2   //need to modify 
#define OK_TO_RUN_PIN             3

#define VALVE_PIN_MN              4
#define VALVE_PIN_MP              7

#define VALVE_CLOSED_FEEDBACK_PIN 5
#define VALVE_OPEN_FEEDBACK_PIN   6

#define STROBE_PIN                8
#define BUZZER_PIN                9



const int VALVE_TIMEOUT_MS = 10000;


bool okToRun = false;
bool powerOk = false;
bool floatSwitch = false;
bool valveOn = false;
bool valveOpenFeedback = false;
bool valveClosedFeedback = false;
bool IsbuzzerOn = false;
bool IsstrobeOn = false;
bool turnOnbuzzer = false;
unsigned long valveOnOffTime = 0;
unsigned long buzzerOnTime = 0;
unsigned long strobeOnTime = 0;

unsigned long RainIndexTime = 0;

int ModeOld =-1, Modecur = 0;
_Sysmode system_mode = FTD_MODE;
_FTD_state FTD_state =FTD_STATE_DONE;
void openValve();
void closeValve();
void checkValveClosedFeedback();
void valveOff();
void buzzerOn();
void buzzerOff();
void strobeOn();
void strobeOff();
int mutedigitalreadLow(int pin);

void setup() {
  pinMode(OK_TO_RUN_PIN, INPUT_PULLUP);
  pinMode(MUTE_PIN, INPUT_PULLUP);
  pinMode(POWER_OK_PIN, INPUT_PULLUP);
  pinMode(FLOAT_SWITCH_PIN, INPUT_PULLUP);
  pinMode(VALVE_PIN_MN, OUTPUT);
  pinMode(VALVE_PIN_MP, OUTPUT);
  pinMode(VALVE_OPEN_FEEDBACK_PIN, INPUT_PULLUP);
  pinMode(VALVE_CLOSED_FEEDBACK_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(STROBE_PIN, OUTPUT);
  buzzerOff();
  strobeOff();  
  #if DEBUG_ENABLE
  Serial.begin(115200);
  #endif
  
}

void loop() {
  
  okToRun = !digitalRead(OK_TO_RUN_PIN);
  powerOk = !digitalRead(POWER_OK_PIN);
  if(!mutedigitalreadLow(MUTE_PIN))
  {
    turnOnbuzzer = false;
    buzzerOff();
    #if DEBUG_ENABLE
        Serial.println("mute buzzer");
    #endif
  }
  if(okToRun && powerOk) {  
    if(system_mode == FTD_MODE){
        switch (FTD_state)
        {
          case FTD_STATE_CHECK_FLOAT:
              floatSwitch = digitalRead(FLOAT_SWITCH_PIN);
              if (floatSwitch == FLOAT_DOWN) {  // float down =>turn on valve
                
                #if DEBUG_ENABLE
                Serial.println("float down => open valve");
                #endif
                openValve();
                valveOnOffTime = millis() + VALVE_TIMEOUT_MS;
                FTD_state = FTD_STATE_OPEN_VALVE;
              }
              else
              {
                #if DEBUG_ENABLE
                Serial.println("float up => close valve");
                #endif
                closeValve();
                valveOnOffTime = millis() + VALVE_TIMEOUT_MS; // timeout 
                FTD_state = FTD_STATE_CLOSE_VALVE;    
              }
            break;
          case FTD_STATE_OPEN_VALVE:
            openValve();
            if(millis()  < valveOnOffTime) // on waiting time
            {
              if(!digitalRead(VALVE_OPEN_FEEDBACK_PIN)) //valve ok
              {
                #if DEBUG_ENABLE
                Serial.println("valve opened");
                #endif
                FTD_state = FTD_STATE_DONE;
              }
            }
            else
            {
              #if DEBUG_ENABLE
                Serial.println("open valve timout");
              #endif
              turnOnbuzzer = true;
              FTD_state = FTD_STATE_ALARM;     // timeout 
            }
            break;
          case FTD_STATE_CLOSE_VALVE:
            closeValve();
            if(millis()  < valveOnOffTime) // on waiting time
            {
              if(!digitalRead(VALVE_CLOSED_FEEDBACK_PIN)) //valve ok
              {
                #if DEBUG_ENABLE
                  Serial.println("valve closed");
                #endif
                FTD_state = FTD_STATE_DONE; // closed
              }
            }
            else
            {
              #if DEBUG_ENABLE
                  Serial.println("valve close timout");
              #endif
              turnOnbuzzer = true;
              FTD_state = FTD_STATE_ALARM;     // timeout 
              
            }
            break;
          case FTD_STATE_ALARM:
            if(turnOnbuzzer)
              buzzerOn();
            else
              buzzerOff();
            strobeOn();
            closeValve();
            if(!digitalRead(VALVE_CLOSED_FEEDBACK_PIN)) //valve ok
            {
              #if DEBUG_ENABLE
                Serial.println("valve closed");
              #endif
              FTD_state = FTD_STATE_DONE; // closed
            }
            break;
          case FTD_STATE_DONE:
            buzzerOff();
            strobeOff();
            FTD_state = FTD_STATE_CHECK_FLOAT;
            break;
          default:
            break;
        }
    } else {
      //for FFD MODE
    }
  }
  else {
      closeValve();
      buzzerOff();
      strobeOff();
  }

}

void openValve() {
  digitalWrite(VALVE_PIN_MN, HIGH);    //need to modify
  digitalWrite(VALVE_PIN_MP, HIGH);
  valveOn = true;
}

void closeValve() {
  digitalWrite(VALVE_PIN_MN, LOW);
  digitalWrite(VALVE_PIN_MP, LOW);
  valveOn = false;
}

void buzzerOn() {
  digitalWrite(BUZZER_PIN, HIGH);
  IsbuzzerOn = true;
  buzzerOnTime = millis();
}

void buzzerOff() {
  digitalWrite(BUZZER_PIN, LOW);
  IsbuzzerOn = false;
}

void strobeOn() {
  digitalWrite(STROBE_PIN, HIGH);
  IsstrobeOn = true;
  strobeOnTime = millis();
}

void strobeOff() {
  digitalWrite(STROBE_PIN, LOW);
  IsstrobeOn = false;
}

int mutedigitalreadLow(int pin)
{
  if(!digitalRead(pin))
  {
    delay(20);
    if(!digitalRead(pin))
      return 0;
  }
  return 1;
}
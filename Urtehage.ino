/*
Needed Equipment:
3x Water Sensor
4x 5VDC Relays
1x Water Pump
1x Digital Buzzer
3x LED Growth Light Strips


Wiring Information:
  Water Sensors - Sump Low - Place in the lower collection tank. Used to stop the pump when the level is to low. Place slightly above the level of pump intake.
  GND Pin - Connect to common ground
  5V Pin - Connect to digital Pin 4
  Signal Pin - Connect to Analog pin A0

  Water Sensros - Sump High - Place in the lower collection tank. Used to start the pump when the levels is high. Place below overflow level.
  GND Pin - Connect to common ground
  5V Pin - Connect to digital Pin 5
  Signal Pin - Connect to Analog Pin A1

  Water Sensors - Tank Low - Place in the water tank. used to alert the user when water level is low. Place low in the tank.
  GND Pin - Connect to common ground
  5V Pin - Connect to digital Pin 6
  Signal Pin - Connect to Analog Pin A2

  Digital Buzzer 
  GND Pin - Connect to common ground
  Signal Pin - Connect to digital pin 1

  Relays - Water Pump control
  GND Pin - Connect to common ground
  5V Pin - Connect to common 5VDC connection
  Signal Pin - Connect to Digital Pin 7

  COMMON Pin - Connect to 6VDC connection
  NO Pin - Connect to Pump 6VDC connection

  Relays - Dawn Leds control
  GND Pin - Connect to common ground
  5V Pin - Connect to common 5VDC connection
  Signal Pin - Connect to Digital Pin 8

  COMMON Pin - Connect to 12VDC connection
  NO Pin - Connec to to LED + connection

  Relays - Mid Day control
  GND Pin - Connect to common ground
  5V Pin - Connect to common 5VDC connection
  Signal Pin - Connect to Digital Pin 9

  COMMON Pin - Connect to 12VDC connection
  NO Pin - Connect to LED + connection

  Relays - Dusk control
  GND Pin - Connect to common ground
  5V Pin - Connect to common 5VDC connection
  Signal Pin - Connect to Digitaø Pin 10

  COMMON Pin - Connect to 12VDC connection
  NO Pin - Connect to LED + connection

*/

//Holding water sensor pin info and latest sensor value
struct SENSOR
{
  int sensorPin;
  int controlPin;
  int sensorValue;
};

//Holding LED Control pin and on/off times
struct LED_CONTROL{
  int ledPin;
  float ledOn;
  float ledOff;
};

SENSOR sumpLow = {A0, 4, 0}; //Init sump low sensor. Read value from analog A0. Power is controlled by digital pin 4.
SENSOR sumpHigh = {A1, 5, 0}; //Init sump high sensor. Read value from analog A1. Power is controlled by digital pin 5.
SENSOR tankLow = {A2, 6, 0}; ////Init tankLow sensor. Read value from analog A2. Power is controlled by digital pin 6.

SENSOR SENSORS[3] = {sumpLow, sumpHigh, tankLow};

//There are three LED Strips angled diffrently, to simulate early morning ligth, mid day light and afternoon light
LED_CONTROL dawnLED = {8, 7.0, 12.0}; //Morning light. Active from 07:00 to 12:00
LED_CONTROL midDayLED = {9, 11.0, 16.0}; //Mid day light. Active from 11:00 to 16:00
LED_CONTROL duskLED = {10, 15.0, 18.0}; //Afternoon light. Active from 15:00 to 18:00

LED_CONTROL LED_CONTROLS[3] = {dawnLED, midDayLED, duskLED};

//Buzzer
#define buzzerPin 1 //Digital pin. Used for alerting on low tank level.
const int buzzInterval = 5; //On low tank level, alert user every 5 minutes
int buzzTimer = 0; //Timer
const int toneLength = 100; //Length of alert tone. Adjust as wanted. A value of 100 means a tone length of ~200ms.
const float buzzInactiveStart = 22.0; //Do not alert after 22:00
const float buzzInactiveStop = 7.0; //Start alerting again after 07:00
float alertActive = false; //True when a alert is active

//Pump
#define sumpPumpControl 7 //Digital pin. Used for starting and stopping the sump pump.
bool sumpState = false; //Sump pump state. Active or not

//Sensor read timer
int sensorTimer = 0; //Timer
unsigned long previousMillis = 0;
const int sensorUpdateTime = 10; // Check the water sensors every 10 seconds
int sensorIndex = 0;

//LED read timer
int LEDTimer = 0; //Timer
int LEDIndex = 0;
const int LEDUpdateTime = 60; //Check the LEDS every 60 seconds

//Time
unsigned long timeTimer = 0;
int seconds = 0;
int minutes = 0;
int hours = 0;
float time = 0.0; //Aggregate of hours and minutes

void setup() {
  //Init relay control
  pinMode(sumpPumpControl, OUTPUT); //Control is active low
  digitalWrite(sumpPumpControl, HIGH); //Set pump to off

  pinMode(dawnLED.ledPin, OUTPUT); //Control is active low
  digitalWrite(dawnLED.ledPin, HIGH); //Set leds to off
  pinMode(midDayLED.ledPin, OUTPUT); //Control is active low
  digitalWrite(midDayLED.ledPin, HIGH); //Set leds to off
  pinMode(duskLED.ledPin, OUTPUT); //Control is active low
  digitalWrite(duskLED.ledPin, HIGH); //Set led to off


  //Init watersensor control
  pinMode(sumpLow.controlPin, OUTPUT);
  digitalWrite(sumpLow.controlPin, LOW);
  pinMode(sumpHigh.controlPin, OUTPUT);
  digitalWrite(sumpHigh.controlPin, LOW);
  pinMode(tankLow.controlPin, OUTPUT);
  digitalWrite(tankLow.controlPin, LOW);

}

void loop() {
  //Timekeeping functions
  TimeDelta();
  TickTime();

  //Water level
  UpdateSensors();
  CheckSumpHigh();
  CheckSumpLow();
  CheckTankLow();

  //Check for user alert - Low water level
  CheckAlerts();

  //LEDS
  UpdateLEDS();

  

}

void UpdateSensors(){
  //Get water level. Cycle through the three sensors every 10 seconds
  if((sensorTimer / 1000) >= sensorUpdateTime){
    ReadSensor(SENSORS[sensorIndex]);
    sensorIndex++;
    sensorTimer = 0;

    //Make sure index is not out of bounds
    if(sensorIndex > 2){
      sensorIndex = 0;
    }
  }
}

void ReadSensor(SENSOR sensor){
  //Turn on the water sensor. Then get the value.
  //No idea if it's actually necessarry to turn on/off
  digitalWrite(sensor.controlPin, HIGH);

  sensor.sensorValue = analogRead(sensor.sensorPin);
  //Turn off after reading.
  digitalWrite(sensor.controlPin, LOW);
}

//Check high levels.
//If value is over 400. The sump tank is full. Start the pump.
void CheckSumpHigh(){
  //Return if the pump is allready running.
  if(sumpState == true)
    return;
  //Return if the sensor value is under 400. A value of <400 means the tank is not full
  if(sumpHigh.sensorValue < 400)
    return;

  //The tank is full. Start the pump
  digitalWrite(sumpPumpControl, LOW);
  sumpState = true;
}
//Check low levels.
//If value is under 300. The sump tank is empty. Stop the pump.
void CheckSumpLow(){
  //Return if the pump is allready stopped.
  if(sumpState == false)
    return;

  //Return if the sensor value is over 300. A value of >300 means the tank is not empty.
  if(sumpLow.sensorValue >= 300)
    return;

  digitalWrite(sumpPumpControl, HIGH);
  sumpState = false;
}

//Check low tank levels.
//If the value is under 300. The tank is empty. Alert user that refill is needed
void CheckTankLow(){
  //Water level is above min threshold. Return.
  if(tankLow.sensorValue > 300){
    alertActive = false;
    return;
  }
  
  alertActive = true;
}

//Check if user alert is active. If true, sound the buzzer
void CheckAlerts(){

  //If alert is inactive, return
  if(alertActive = false)
    return;
  //If time is after 22:00, return
  if(time > buzzInactiveStart)
    return;
  //If time is before 07:00, return
  if(time < buzzInactiveStop)
    return;
  //If timer has not tripped, return
  if((buzzTimer / 1000) < buzzInterval)
    return;

  Buzz_LowWaterLevel();
  buzzTimer = 0;

}

//Sound the buzzer. 
//Adjust tone length as needed
void Buzz_LowWaterLevel(){
  for(int i=0; i < toneLength; i++){
      digitalWrite(buzzerPin, HIGH);
      delay(1);
      digitalWrite(buzzerPin, LOW);
      delay(1);
    }
}

void TimeDelta(){
  unsigned long m = millis();
  //Check for rollover on millis. Takes about 50 days.
  if(previousMillis < m){
    //No rollover. Calculate delta
    int delta = (m - previousMillis);
    sensorTimer += delta;
    timeTimer += delta;
    LEDTimer += delta;
    buzzTimer += delta;
    previousMillis = m;
  } else {
    //Roll over. Do not increment. Reset previousMills
    previousMillis = m;
  }
}

void TickTime(){
  //Increment second counter
  if((timeTimer / 1000) > 1){
    seconds += 1;
    timeTimer -= 1000;
  } 

  //Increment minutes
  if(seconds > 59){
    minutes += 1;
    seconds = 0;
  }

  //Increment hours
  if(minutes > 59){
    hours += 1;
    minutes = 0;
  }

  //Roll over hours
  if(hours > 23)
    hours = 0;

  float min = minutes / 60;
  time = hours + min;
}

void UpdateLEDS(){
  if((LEDTimer / 1000) > LEDUpdateTime){
    LEDTimer = 0;

    //Loop through all elements in LED_CONTROLS array
    for(int i = 0; i < 3; i++){
      LED_CONTROL led = LED_CONTROLS[i];
      //Check if time is within the timewindow for the LEDS to be on
      if(time >= led.ledOn && time < led.ledOff)
        digitalWrite(led.ledPin, LOW); //Activate LEDS by setting the relay control pin to low
      else 
        digitalWrite(led.ledPin, HIGH); //Deactivate LEDS by setting relay control pin to high
    }
  }
}

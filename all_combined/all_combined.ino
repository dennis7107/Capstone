#include "Wire.h"
#include "DFRobot_MAX17043.h"

//flow sensor
const int FLOWSENSOR_INT                        = 4;
const int FLOWSENSOR_POWER                      = 2;
const float FLOWSENSOR_CALIBRATION              = 4.5;
volatile byte pulseCount                        = 0;
float flowRate                                  = 0;
unsigned int flowMilliLitres                    = 0;
unsigned long sensorOldTime                     = 0;

//solid-state relay
const int RELAY_PIN                             = 25;
const long RELAY_PRE_ON                         = 2000;
unsigned long relayOldTime                      = 0;
int FAKE_SOURCE                                 = 15;

//battery fuel gauge ic
DFRobot_MAX17043 gauge;
const int GAUGE_POWER                           = 5;
const long GAUGE_INTERVAL                       = 10000;
float batteryVoltage                            = 0;
unsigned long gaugeOldTime                      = 0;
int SOC                                         = 0;

//upload data
const unsigned long UPLOAD_INTERVAL             = 500;
unsigned long uploadOldTime                     = 0;
bool gaugeMeasure = false;
bool dataUpload = false;


void setup() {
  Serial.begin(115200);

  pinMode(FLOWSENSOR_INT, INPUT);
  attachInterrupt(digitalPinToInterrupt(FLOWSENSOR_INT), pulseCounter, FALLING);
  pinMode(FLOWSENSOR_POWER, OUTPUT);
  digitalWrite(FLOWSENSOR_POWER, HIGH);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  pinMode(FAKE_SOURCE, OUTPUT);
  digitalWrite(FAKE_SOURCE, HIGH);

  pinMode(GAUGE_POWER, OUTPUT);
  digitalWrite(GAUGE_POWER, HIGH);
  while(gauge.begin() != 0) {
    Serial.println("gauge begin failed!");
    delay(2000);
  }
  delay(2);
  Serial.println("gauge begin successful!");
}

void loop() {
  if ((millis() - sensorOldTime) > 1000)
  {
    flowSensorUpdate();
    sensorOldTime = millis();
  }
  if ((millis() - gaugeOldTime) > GAUGE_INTERVAL)
  {
    gaugeMeasure = true;
    digitalWrite(RELAY_PIN, HIGH);
    relayOldTime = millis();
    gaugeOldTime = millis();
    //Serial.println("gauge on");
  }
  if ((millis() - uploadOldTime) > UPLOAD_INTERVAL)
  {
    dataUpload = true;
    uploadOldTime = millis();
  }

  if (gaugeMeasure == true)
  {
    if ((millis() - relayOldTime) > RELAY_PRE_ON)
    {
    batteryVoltage = gauge.readVoltage();
    gaugeMeasure = false;
    digitalWrite(RELAY_PIN, LOW);
    
    }
  }

  if (dataUpload == true)
  {
    getStateOfCharge();
    Serial.print("Flow rate: ");
    Serial.print(flowRate);
    Serial.print("\tml per sec\tBattery voltage: ");
    Serial.print(batteryVoltage);
    Serial.print("\tmV\tSOC: ");
    Serial.print(SOC);
    Serial.println("\t%");

//    Serial.print("gaugeOldTime: ");
//    Serial.print((millis() - gaugeOldTime));
//    Serial.print("\trelayOldTime: ");
//    Serial.print((millis() - relayOldTime));
//    Serial.print("\tgaugeMeasure: ");
//    Serial.println(gaugeMeasure);
    dataUpload = false;
  }
}


void pulseCounter()
{
  pulseCount++;
}

void flowSensorUpdate()
{
  detachInterrupt(digitalPinToInterrupt(FLOWSENSOR_INT));
  flowRate = ((1000.0 / (millis() - sensorOldTime)) * pulseCount) / FLOWSENSOR_CALIBRATION;
  pulseCount = 0;
  attachInterrupt(digitalPinToInterrupt(FLOWSENSOR_INT), pulseCounter, FALLING);
}

void getStateOfCharge()
{
  if (batteryVoltage>12.4) {
    SOC = 100;
  }
  else if (batteryVoltage>=11.2) {
    SOC = 100 - 25*(12.4-batteryVoltage);
  } 
  else if (batteryVoltage>=11.0) {
    SOC = 70 - 250*(11.2-batteryVoltage);
  }
  else if (batteryVoltage>=10.0) {
    SOC = 20 - 20*(11-batteryVoltage);
  }
  else {
    SOC = 0;
  }
  //Serial.println("reading battery soc");
}

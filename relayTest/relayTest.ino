#include <Wire.h>

int relay = 12;
int ledPin = 13;
byte relayState;
unsigned long oldTime;
unsigned long relayOnTime;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(relay, OUTPUT);
  digitalWrite(relay, LOW);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);
  relayState = 0;
  oldTime = 0;
  relayOnTime = 0;
}

void loop() {
  // put your main code here, to run repeatedly:
  if ((millis() - oldTime) > 2000) {
    relayState = 1;
    digitalWrite(relay, HIGH);
    oldTime = millis();
    relayOnTime = millis();
  }
  if ((millis() - relayOnTime) > 1000) {
    relayState = 0;
    digitalWrite(relay, LOW);
  }
  Serial.print((millis() - relayOnTime));
  Serial.print("  ");
  Serial.println(relayState);
}

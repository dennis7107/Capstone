#include <Wire.h>

int relay = 25;
int ledPin = 15;
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
  if ((millis() - oldTime) > 10000) {
    relayState = 1;
    digitalWrite(relay, HIGH);
    oldTime = millis();
    relayOnTime = millis();
  }
  if ((millis() - relayOnTime) > 5000) {
    relayState = 0;
    digitalWrite(relay, LOW);
    relayOnTime = millis();
  }
  Serial.print((millis()- oldTime));
  Serial.print("  ");
  Serial.print((millis() - relayOnTime));
  Serial.print("  ");
  Serial.println(relayState);
}

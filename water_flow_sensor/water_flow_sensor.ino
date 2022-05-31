/*

YF‐ S201 Water Flow Sensor

Water Flow Sensor output processed to read in litres/hour

Adaptation Courtesy: hobbytronics.co.uk

*/

volatile int flow_frequency; // Measures flow sensor pulses

// Calculated litres/hour

float vol = 0.0,l_minute;

unsigned char flowsensor = 34; // Sensor Input

unsigned long currentTime;

unsigned long cloopTime;

void flow () // Interrupt function

{

   flow_frequency++;

}

void setup()

{

   pinMode(flowsensor, INPUT);

   digitalWrite(flowsensor, HIGH); // Optional Internal Pull-Up
   digitalWrite(32, HIGH);

   Serial.begin(9600);

   attachInterrupt(digitalPinToInterrupt(flowsensor), flow, RISING); // Setup Interrupt

   currentTime = millis();

   cloopTime = currentTime;

}

void loop ()

{

   currentTime = millis();

   // Every second, calculate and print litres/hour

   if(currentTime >= (cloopTime + 1000))

   {

    cloopTime = currentTime; // Updates cloopTime

    if(flow_frequency != 0){

      // Pulse frequency (Hz) = 7.5Q, Q is flow rate in L/min.

      l_minute = (flow_frequency / 7.5); // (Pulse frequency x 60 min) / 7.5Q = flowrate in L/hour

      l_minute = l_minute/60;

      vol = vol +l_minute;

      flow_frequency = 0; // Reset Counter

      Serial.print(l_minute, DEC); // Print litres/hour

      Serial.println(" L/Sec");

    }

    else {

      Serial.println(" flow rate = 0 ");

    }

   }

}

/*
 * rfbox source https://github.com/incmve/generic-rfbox
 * Generic RF sensor box for pimatic https://github.com/pimatic/pimatic
 * Current supported: ds18b20(multiple), DHT11, HC-SR04
 */

// Includes
#include <OneWire.h> // http://www.pjrc.com/teensy/arduino_libraries/OneWire.zip
#include <DallasTemperature.h> // http://download.milesburton.com/Arduino/MaximTemperature/DallasTemperature_LATEST.zip
#include <dht.h> // http://playground.arduino.cc/Main/DHTLib#.UyMXevldWCQ
#include <PimaticProbe.h> // Thanks to Oitzu for making the Generic class

// Define vars
#define DHT11_PIN 9
#define senderPin 4 // 
const int ledPin = 13; // internal LED PIN
#define ONE_WIRE_BUS 5 // DS18B20 PIN
#define echoPin 11 // Echo Pin
#define trigPin 12 // Trigger Pin

long codeKit = 1000;  // Your unique ID for your Arduino node
int Bytes[30]; 
int BytesData[30]; 
int maximumRange = 200; // Maximum range sonar
int minimumRange = 0; // Minimum range sonar
long duration, distance; // Duration used to calculate distance
PimaticProbe probe = PimaticProbe(senderPin, codeKit);

// Config which modules to use
boolean DHT11 = true;
boolean DS18B20 = true;
boolean ultrasonic = false;


// Start includes
OneWire oneWire(ONE_WIRE_BUS); // Setup a oneWire instance
DallasTemperature sensors(&oneWire); // Pass our oneWire reference to Dallas Temperature  
dht DHT;   
DeviceAddress insideThermometer;

void setup()
{
  pinMode(senderPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  Serial.begin(115200); // Open serial monitor at 115200 baud to see ping results.
  
  if (DS18B20) {
     //start up temp sensor
    sensors.begin();
    sensors.getAddress(insideThermometer, 0);
    int reso = sensors.getResolution(insideThermometer);
    Serial.println("Sensor resolution");
    Serial.println(reso);
    if (reso != 12) {
        Serial.print("Resolution of DS18B20 is not 12 but ");
        Serial.println("Sensor resolution");
        Serial.println(reso);
        Serial.println(" changing to 12\n");
      sensors.setResolution(insideThermometer, 12);
        Serial.print("Done\n");
    }
  }
}

void loop()
{
  if (DS18B20) {
 Serial.println("Begin ds18b20");
     // Read DS18B20 and transmit value as sensor 1
 float temperature;
 sensors.requestTemperatures(); // Get the temperature
 temperature = sensors.getTempCByIndex(0); // Get temperature in Celcius
 int temp1;
 int type = 1;
  if (temperature >= 0.0) {
      temp1 = temperature * 10;
      Serial.println("Positive temp");
      Serial.println(temp1);
      probe.transmit(true, temp1, type, 6); // ([postive/negative], CounterValue, [type], 6)
    }
    if (temperature < 0.0) {
      temp1 = temperature * -10;
      Serial.println("Negative temp");
      Serial.println("-");
      Serial.println(temp1);
     probe.transmit(false, temp1, type, 6); // ([postive/negative], CounterValue, [type], 6)
    }
 Blink(ledPin,1);
 delay(10000); // wait for 10 seconds to go to next sensor
  }

  if (DHT11) {
  Serial.println("Begin DHT11");
    // Read DHT11 and transmit value as sensor 2
    int chk = DHT.read11(DHT11_PIN);
    int type = 2;
    Serial.println(chk);
    switch (chk)
    {
      case DHTLIB_OK:
      float humfloat = DHT.humidity;
      int humidity = humfloat * 10;
      int BytesType[] = {0,0,1,0}; // type = 2
      probe.transmit(true, humidity, type, 6); // ([postive/negative], CounterValue, [type], 6)
      Blink(ledPin,2);
      Serial.println(humidity);
      delay(10000); // wait for 10 seconds to go to next sensor
      break;
    }
  }
  if (ultrasonic) {
        int type = 3;
    // Read Sonar and transmit value as sensor 3
    /* The following trigPin/echoPin cycle is used to determine the
	distance of the nearest object by bouncing soundwaves off of it. */ 
	digitalWrite(trigPin, LOW); 
	delayMicroseconds(2); 

	digitalWrite(trigPin, HIGH);
	delayMicroseconds(10); 
 
	digitalWrite(trigPin, LOW);
	duration = pulseIn(echoPin, HIGH);
 
	//Calculate the distance (in cm) based on the pace of sound. http://www.instructables.com/id/Using-a-SR04/
	distance = duration/58.2;
 
	if (distance >= maximumRange || distance <= minimumRange){
	/* Send a negative number to computer and Turn LED ON 
	to indicate "out of range" */
	Serial.println("Out of range");
 
	}
	else {
	/* Send the distance to the computer using Serial protocol,*/
	int BytesType[] = {0,0,1,1}; //transmit value as sensor 3
        probe.transmit(true, distance, type, 6); // ([postive/negative], CounterValue, [type], 6)
	Serial.println(distance);
        Blink(ledPin,3);
		}
	}
Serial.println("End of Loop");
delay(1800000); // wait for 30 minutes to restart loop, be aware if to short RF pollution will occur.
  
}
void Blink(int led, int times)
{
 for (int i=0; i< times; i++)
 {
  digitalWrite(ledPin,HIGH);
  delay (250);
  digitalWrite(ledPin,LOW);
  delay (250);
 }
}

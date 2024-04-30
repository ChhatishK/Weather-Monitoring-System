#include <WiFi.h>
#include <ThingSpeak.h>


//ESP8266 WiFi Module
#include <SoftwareSerial.h>
#define RX 10
#define TX 12
String AP = "Redmi 9 Power";       // CHANGE ME
String PASS = "12345678"; // CHANGE ME
String API = "NL68TDM7K3SQEBV8";   // CHANGE ME
String HOST = "api.thingspeak.com";
String PORT = "80";
//String field = "field1";
int countTrueCommand;
int countTimeCommand; 
boolean found = false; 
SoftwareSerial esp8266(RX,TX);


WiFiClient client;


// BMP Sensor
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BMP280 bmp;

//MQ135 Sensor
#include <LiquidCrystal.h>
#include "MQ135.h"
#include <SPI.h>
#define sensorPin A3
MQ135 gasSensor = MQ135(sensorPin);
int val;
//int sensorPin = A3;
int sensorValue = 0;


//UV Index Sensor
int UVOUT = A1;
int REF_3V3 = A2;
 


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  ThingSpeak.begin(client);
  
//  ESP8266 WiFi Module
  esp8266.begin(115200);
  sendCommand("AT",5,"OK");
  sendCommand("AT+CWMODE=1",5,"OK");
  sendCommand("AT+CWJAP=\""+ AP +"\",\""+ PASS +"\"",20,"OK");

    // Rain sensor
    pinMode(A0, INPUT);
    
    //BMP280 Sensor
    if (!bmp.begin(0x76)) {
    Serial.println("Could not find a valid BMP280 sensor, check wiring!");
    while (1);
  }

    // MQ135 Sensor
    pinMode(sensorPin, INPUT);

//    UV Index Sensor
     pinMode(UVOUT, INPUT);
     pinMode(REF_3V3, INPUT);


}
void loop() {
  
  // put your main code here, to run repeatedly:
//  float h = dht.readHumidity();
//  float t = dht.readTemperature();
//  Serial.println("Temperature: " +(String) t);
//  Serial.println("Humidity: " +(String) h);
//  ThingSpeak.writeField(myChannelNumber, 1, t, myWriteAPIKey);
//  ThingSpeak.writeField(myChannelNumber, 2, h, myWriteAPIKey);

  //  Rain Sensor
  int RainValue = analogRead(A0);
  Serial.print("Rain Value = ");
  Serial.println(RainValue);

  //BMP280 Sensor
  Serial.print("Temperature = ");
  int temp = bmp.readTemperature();
  Serial.print(temp);
  Serial.println("°C");

  Serial.print("Pressure = ");
  int pressure = bmp.readPressure() / 100.0F;
  Serial.print(pressure);
  Serial.println("hPa");

  Serial.print("Approx. Altitude = ");
  int altitude = bmp.readAltitude(SEALEVELPRESSURE_HPA);
  Serial.print(altitude);
  Serial.println("m");
  
  // MQ135 Sensor
  val = analogRead(sensorPin);
  Serial.print ("raw = ");
  Serial.println (val);
  float zero = gasSensor.getRZero();
  Serial.print ("rzero: ");
  Serial.print (zero);
  Serial.println("           ");
  float ppm = gasSensor.getPPM();
  Serial.print ("ppm: ");
  Serial.println (ppm);
  Serial.println();
  int AQIValue = (val + zero + ppm)/3;
  Serial.print("Air Quality Index = ");
  Serial.println(zero);
//  Serial.println();
//  ThingSpeak.writeField(myChannelNumber, 6, AQIValue, myWriteAPIKey);
  
//  UV Index Sensor
  int uvLevel = averageAnalogRead(UVOUT);
  int refLevel = averageAnalogRead(REF_3V3);
  
  //Use the 3.3V power pin as a reference to get a very accurate output value from sensor
  float outputVoltage = 3.3 / refLevel * uvLevel;
  
  float uvIntensity = mapfloat(outputVoltage, 0.99, 2.8, 0.0, 15.0); //Convert the voltage to a UV intensity level
 
  Serial.print("output: ");
  Serial.println(refLevel);
 
  Serial.print("ML8511 output: ");
  Serial.println(uvLevel);
 
  Serial.print(" / ML8511 voltage: ");
  Serial.println(outputVoltage);
  Serial.println();
  Serial.print("UV Intensity (mW/cm^2): ");
  Serial.println(uvIntensity);
//  
//  Serial.println();
//
//  ThingSpeak.writeField(myChannelNumber, 7, uvintensity, myWriteAPIKey);
//  delay(2000); 


//  ESP8266 WiFi Module
   String getData = "GET https://api.thingspeak.com/update?api_key="+ API +"&field1="+temp+"&field5="+pressure+"&field4="+altitude+"&field7"+uvIntensity+"&field6"+zero+"&field3="+RainValue+"\r\n";
   sendCommand("AT+CIPMUX=1",5,"OK");
   sendCommand("AT+CIPSTART=0,\"TCP\",\""+ HOST +"\","+ PORT,15,"OK");
   sendCommand("AT+CIPSEND=0," +String(getData.length()+4),4,">");
   esp8266.print(getData);
   delay(15000);
   countTrueCommand++;
   sendCommand("AT+CIPCLOSE=0",5,"OK");


}


// UV Index Sensor
int averageAnalogRead(int pinToRead)
{
  byte numberOfReadings = 8;
  unsigned int runningValue = 0; 
 
  for(int x = 0 ; x < numberOfReadings ; x++)
  runningValue += analogRead(pinToRead);
  runningValue /= numberOfReadings;
 
  return(runningValue);
}
 
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}




//ESP8266 WiFi Module
int getSensorData(){
  return random(1000); // Replace with 
}
void sendCommand(String command, int maxTime, char readReplay[]) {
  Serial.print(countTrueCommand);
  Serial.print(". at command => ");
  Serial.print(command);
  Serial.print(" ");
  while(countTimeCommand < (maxTime*1))
  {
    esp8266.println(command);//at+cipsend
    if(esp8266.find(readReplay))//ok
    {
      found = true;
      break;
    }
  
    countTimeCommand++;
  }
  
  if(found == true)
  {
    Serial.println("OYI");
    countTrueCommand++;
    countTimeCommand = 0;
  }
  
  if(found == false)
  {
    Serial.println("Fail");
    countTrueCommand = 0;
    countTimeCommand = 0;
  }
  
  found = false;
 }

#include <Wire.h>
#include <Adafruit_BMP085.h>
#include "DHT.h"
#include <WiFi.h>
#include <HTTPClient.h>

#include <LiquidCrystal_I2C.h> 
#define I2C_ADDR 0x27          
#define BACKLIGHT_PIN 3        

const char* ssid = "Amit";
const char* password = "12345678";
const char* server = "api.thingspeak.com";
const String apiKey = "UOGDF6SO3YSJT0YI"; 

const int sensorPin1 = 35; 
const int sensorPin2 = 36; 
#define DHT11PIN 18
#define DHTTYPE DHT11

Adafruit_BMP085 bmp;
DHT dht(DHT11PIN, DHTTYPE);

LiquidCrystal_I2C lcd(I2C_ADDR, 16, 2); 

int buttonPin = 2; 
int buttonState = 0; 
int displayMode = 1; 

void setup() {
  Serial.begin(9600);
  pinMode(buttonPin, INPUT_PULLUP); 

  lcd.init();
  lcd.begin(16,2);                      
  lcd.backlight();                 
  lcd.setCursor(0, 0);
  lcd.print("Initializing...");
  delay(2000);
  lcd.clear();

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  if (!bmp.begin()) {
    Serial.println("Could not find a valid BMP085/BMP180 sensor, check wiring!");
    while (1) {}
  }

  dht.begin();
}

void loop() {
  buttonState = digitalRead(buttonPin); 

  if (buttonState == LOW) { 
    changeDisplayMode();    
    delay(500);             
  }

  if (WiFi.status() == WL_CONNECTED) {
    float temperature = bmp.readTemperature();
    float pressure = bmp.readPressure();
    float altitude = bmp.readAltitude();
    float sealevelPressure = bmp.readSealevelPressure();
    float realAltitude = bmp.readAltitude(102000);
    float humidity = dht.readHumidity();
    int mqSensorValue1 = analogRead(sensorPin1);
    int mqSensorValue2 = analogRead(sensorPin2);

    // Display data on Serial Monitor
    Serial.println("Data:");
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" *C");
    Serial.print("Pressure: ");
    Serial.print(pressure);
    Serial.println(" Pa");
    Serial.print("Altitude: ");
    Serial.print(altitude);
    Serial.println(" meters");
    Serial.print("Pressure at sealevel (calculated): ");
    Serial.print(sealevelPressure);
    Serial.println(" Pa");
    Serial.print("Real altitude: ");
    Serial.print(realAltitude);
    Serial.println(" meters");
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println("%");
    Serial.print("MQ2 Sensor: ");
    Serial.print(mqSensorValue1);
    Serial.println(" ppm");
    Serial.print("MQ3 Sensor: ");
    Serial.print(mqSensorValue2);
    Serial.println(" ppm");
    Serial.println();

    switch (displayMode) {
      case 1:
        displayDataOnLCD("Temperature: ", temperature, " *C");
        break;
      case 2:
        displayDataOnLCD("Pressure: ", pressure, " Pa");
        break;
      case 3:
        displayDataOnLCD("Altitude: ", altitude, " meters");
        break;
      case 4:
        displayDataOnLCD("Sealevel Pressure: ", sealevelPressure, " Pa");
        break;
      case 5:
        displayDataOnLCD("Real Altitude: ", realAltitude, " meters");
        break;
      case 6:
        displayDataOnLCD("Humidity: ", humidity, "%");
        break;
      case 7:
        displayDataOnLCD("Gas Sensor: ", mqSensorValue1, " ppm");
        break;
      case 8:
        displayDataOnLCD("Alcohol Sensor: ", mqSensorValue2, " ppm");
        break;
    }

  
    sendDataToThingSpeak(temperature, pressure, altitude, sealevelPressure, realAltitude, humidity, mqSensorValue1, mqSensorValue2);

    delay(10000); 
  }
}

void displayDataOnLCD(String label, float value, String unit) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(label);
  lcd.setCursor(0, 1);
  lcd.print(value);
  lcd.print(unit);
}

void sendDataToThingSpeak(float temperature, float pressure, float altitude, float sealevelPressure, float realAltitude, float humidity, int mqSensorValue1, int mqSensorValue2) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    String url = "http://api.thingspeak.com/update?api_key=" + apiKey;
    url += "&field1=" + String(temperature);
    url += "&field2=" + String(pressure);
    url += "&field3=" + String(altitude);
    url += "&field4=" + String(sealevelPressure);
    url += "&field5=" + String(realAltitude);
    url += "&field6=" + String(humidity);
    url += "&field7=" + String(mqSensorValue1);
    url += "&field8=" + String(mqSensorValue2);

    Serial.println("Sending data to ThingSpeak...");
    Serial.println(url);

    http.begin(url);

    int httpResponseCode = http.GET();

    if (httpResponseCode == 200) {
      Serial.println("Data sent to ThingSpeak successfully!");
    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  } else {
    Serial.println("WiFi Disconnected! Unable to send data to ThingSpeak.");
  }
}

void changeDisplayMode() {
  displayMode++; 

  if (displayMode > 8) { 
    displayMode = 1;
  }
}
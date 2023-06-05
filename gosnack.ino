/*
  This a simple example of the aREST Library for the ESP32 WiFi chip.
  See the README file for more details.

  Written in 2017 by Marco Schwartz under a GPL license.
  
*/

// Import required libraries
#include <WiFi.h>
#include <aREST.h>
#include <HTTPClient.h>

// Create aREST instance
aREST rest = aREST();

// WiFi parameters
//const char* ssid = "RedeTeste";
//const char* password = "";
const char* ssid = ""; // TODO: insert your wifi SSID
const char* password = ""; // TODO: insert your wifi password
String serverIp = ""; // TODO: insert the cloud server IP

byte mac[6];


//Port ID

const int FALL_DETECTOR_PORT = 34;
const int TEMPERATURE_SENSOR_PORT = 35;


const int SLOT1_A = 18;
const int SLOT1_B = 5;
const int SLOT1_C = 17;
const int SLOT1_D = 16;

const int SLOT2_A = 21;
const int SLOT2_B = 22;
const int SLOT2_C = 19;
const int SLOT2_D = 23;



const int STEP_COUNT = 12;
const int STEP_DELAY = 30;

const int FALL_DETECTOR_THREASHOLD = 1000;

const String updateTemperaturePath = "http://"+serverIp+"/api/vending_machine/1/update_temperature";


// Create an instance of the server
WiFiServer server(80);

// Variables to be exposed to the API
float temperature;

unsigned long lastTime = 0;
unsigned long timerDelay = 1000 * 10;

// Declare functions to be exposed to the API
int dispense(String command);

void dispenseSlot1();
void dispenseSlot2();

void setup()
{
  
  // Start Serial
  Serial.begin(9600);

  // Init variables and expose them to REST API
  temperature = 0; // initial value
  rest.variable("temperature", &temperature);

  // Function to be exposed
  rest.function("dispense",dispense);

  // Give name & ID to the device (ID should be 6 characters long)
  rest.set_id("1");
  rest.set_name("esp32");

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());

  WiFi.macAddress(mac);

  Serial.print("MAC: ");
  Serial.print(mac[0], HEX);
  Serial.print(":");
  Serial.print(mac[1], HEX);
  Serial.print(":");
  Serial.print(mac[2], HEX);
  Serial.print(":");
  Serial.print(mac[3], HEX);
  Serial.print(":");
  Serial.print(mac[4], HEX);
  Serial.print(":");
  Serial.print(mac[5], HEX);
  Serial.println("");

  pinMode(SLOT1_A, OUTPUT);
  pinMode(SLOT1_B, OUTPUT);
  pinMode(SLOT1_C, OUTPUT);
  pinMode(SLOT1_D, OUTPUT);

  pinMode(SLOT2_A, OUTPUT);
  pinMode(SLOT2_B, OUTPUT);
  pinMode(SLOT2_C, OUTPUT);
  pinMode(SLOT2_D, OUTPUT);

}

void loop() {

  int temperature_sensor_val = analogRead(TEMPERATURE_SENSOR_PORT);
  temperature = ((float)(temperature_sensor_val - 600)) / 10;

  if ((millis() - lastTime) > timerDelay) {
    lastTime = millis();
    
    if (WiFi.status() == WL_CONNECTED) {
      WiFiClient client;
      HTTPClient http;

      String serverPath = updateTemperaturePath;
      String httpRequestData = "temperature=" + String(temperature);

      http.addHeader("Content-Type", "application/x-www-form-urlencoded");

      http.begin(client, serverPath);

      http.POST(httpRequestData);
      http.end();
    }
  }

  int fall_detector_val = analogRead(FALL_DETECTOR_PORT);

  Serial.printf("%f - %d\n", temperature, fall_detector_val);
  
  // Handle REST calls
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  while(!client.available()){
    delay(1);
  }
  rest.handle(client);
}

// Custom function accessible by the API
int dispense(String command) {

  Serial.println(command);

  if(command.equals("1")) {
    dispenseSlot1();  
  }
  else if (command.equals("2")) {
    dispenseSlot2();
    
  } else {
    return 1;
  }

  delay(1000);

  int productFallDetectorVal = analogRead(FALL_DETECTOR_PORT);

  if (productFallDetectorVal > FALL_DETECTOR_THREASHOLD)
  {
    return 2;

  } else {
    return 0;
  }
  
  
  //TODO: check if product fell

  // Return codes:
  //  0 - Product dispensed successfully
  //  1 - Unknown error
  //  2 - Failed to dispense a product (No product fell)
  return 0;
}

void dispenseSlot1() {

  for(int i = 0; i < STEP_COUNT; i++) {
    // AB
    digitalWrite(SLOT1_A, HIGH);
    digitalWrite(SLOT1_B, HIGH);
    digitalWrite(SLOT1_C, LOW);
    digitalWrite(SLOT1_D, LOW);

    delay(STEP_DELAY);

    // BC
    digitalWrite(SLOT1_A, LOW);
    digitalWrite(SLOT1_B, HIGH);
    digitalWrite(SLOT1_C, HIGH);
    digitalWrite(SLOT1_D, LOW);

    delay(STEP_DELAY);

    // CD
    digitalWrite(SLOT1_A, LOW);
    digitalWrite(SLOT1_B, LOW);
    digitalWrite(SLOT1_C, HIGH);
    digitalWrite(SLOT1_D, HIGH);

    delay(STEP_DELAY);

    // DA

    digitalWrite(SLOT1_A, HIGH);
    digitalWrite(SLOT1_B, LOW);
    digitalWrite(SLOT1_C, LOW);
    digitalWrite(SLOT1_D, HIGH);
    
    delay(STEP_DELAY);
  }

  // Shutdown coils at the end of the operation to avoid the stepper heating up
  digitalWrite(SLOT1_A, LOW);
  digitalWrite(SLOT1_B, LOW);
  digitalWrite(SLOT1_C, LOW);
  digitalWrite(SLOT1_D, LOW);
}

void dispenseSlot2() {
  for(int i = 0; i < STEP_COUNT; i++) {
    // AB
    digitalWrite(SLOT2_A, HIGH);
    digitalWrite(SLOT2_B, HIGH);
    digitalWrite(SLOT2_C, LOW);
    digitalWrite(SLOT2_D, LOW);

    delay(STEP_DELAY);

    // BC
    digitalWrite(SLOT2_A, LOW);
    digitalWrite(SLOT2_B, HIGH);
    digitalWrite(SLOT2_C, HIGH);
    digitalWrite(SLOT2_D, LOW);

    delay(STEP_DELAY);

    // CD
    digitalWrite(SLOT2_A, LOW);
    digitalWrite(SLOT2_B, LOW);
    digitalWrite(SLOT2_C, HIGH);
    digitalWrite(SLOT2_D, HIGH);

    delay(STEP_DELAY);

    // DA

    digitalWrite(SLOT2_A, HIGH);
    digitalWrite(SLOT2_B, LOW);
    digitalWrite(SLOT2_C, LOW);
    digitalWrite(SLOT2_D, HIGH);
    
    delay(STEP_DELAY);
  }

  // Shutdown coils at the end of the operation to avoid the stepper heating up
  digitalWrite(SLOT2_A, LOW);
  digitalWrite(SLOT2_B, LOW);
  digitalWrite(SLOT2_C, LOW);
  digitalWrite(SLOT2_D, LOW);
}

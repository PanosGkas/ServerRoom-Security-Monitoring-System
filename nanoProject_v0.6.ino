#include <Servo.h> // Servo Library
#include <DHT.h> // Humidity Temprature Library
#include <SPI.h> //RFID communication Library
#include <MFRC522.h> // RFID sensor Library
#include <WiFiNINA.h> //WiFi Library
#include <ThingsBoard.h> //ThingsBoard connection Library

//sensor pins definitions
#define smokeSensorPin A0
#define motionSensorPin 2
#define servoPin 3
#define buzzerPin 4
#define redLedPin 5
#define blueLedPin 6
#define greenLedPin 7
#define tempSensorPin 8
#define RST_PIN 9 
#define SS_PIN 10 
#define DHTTYPE DHT11 //humidity temprature type

//object creation and initial values definition
MFRC522 mfrc522(SS_PIN, RST_PIN); //object for RFID card
Servo servo; //object for servo
DHT tempSensor(tempSensorPin, DHTTYPE);    //object for humidity temprature sensor

//wifi variable declaration
char ssid[] = "WIFI-Name"; //change wifi credentials
char pass[] = "WIFI-Pass";
int keyIndex = 0;
int status = WL_IDLE_STATUS;

// thingsboard variables
char token[] = "ThingsBoard-Token"; //token of user's thingsboard device. change for connection to other account
char thingsboardServer[] = "demo.thingsboard.io";

// Initialize underlying client, used to establish a connection
WiFiClient wifiClient;
ThingsBoard tb(wifiClient);

//sensor variable declaration
int motionSensorInput = LOW;
int smokeSensorValue = 0;
int attemps = 0;
int entrances = 0;
bool ACStatus = false;

void setup() {
  //initialize serial and pin headers
  Serial.begin(9600);
  servo.attach(servoPin);
  servo.write(10);       //initiate servo angle to 0 degrees - door closed
  tempSensor.begin();  //initiate DHT sernsor reading
  pinMode(redLedPin, OUTPUT);
  pinMode(blueLedPin, OUTPUT); 
  pinMode(greenLedPin, OUTPUT);
  pinMode(motionSensorInput, INPUT); 
  pinMode(buzzerPin, OUTPUT);
  pinMode(smokeSensorPin, INPUT);

  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522
  delay(4);
  mfrc522.PCD_DumpVersionToSerial();
  Serial.println("Approximate your card to the reader...");

  //WiFi Connection
  if (WiFi.status() == WL_NO_MODULE) { // check for the WiFi module
    Serial.println("Communication with WiFi module failed!");
    while (true); // don't continue
  }
  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }
  while (status != WL_CONNECTED) {  // attempt to connect to WiFi network
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);

    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    delay(10000);
  }
  printWifiStatus();// print status of WiFi
  Serial.println("--------------------Server Room Security System Initialize--------------------------");
}

void loop() {

  if ( !tb.connected() ) {
    reconnect();
  }

  checkTemp();
  checkSmoke();
  checkRFID();
  if(entrances > 2){ // for presentation purposes we consider that after 3 successfull entrances the time is after working hours
    checkMotion();
  }
    
  tb.loop();
}


// ---------------------------------------Monitor Functions--------------------------------------------------//

//Temprature and Humidity Reading
void checkTemp() {
  int readData = tempSensor.read(tempSensorPin);
  float temp = tempSensor.readTemperature();        // Read temperature
  float hum = tempSensor.readHumidity();           // Read humidity

  if (isnan(hum) || isnan(temp)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.print("Temperature = ");
  Serial.print(temp);
  Serial.print("°C | ");
  Serial.print("Humidity = ");
  Serial.print(hum);
  Serial.println("% ");

  tb.sendTelemetryFloat("temperature", temp);
  tb.sendTelemetryFloat("humidity", hum);

  if(temp > 30 && ACStatus == false){
    openAirCondition();
  }
  else if (temp <= 30 && ACStatus == true){
    closeAirCondition();
  }
  else{    
  }
}

//Smoke detection
void checkSmoke(){
  smokeSensorValue = analogRead(smokeSensorPin);
  Serial.print("Smoke Sensor value : ");
  if(smokeSensorValue > 800){  //change smoke value relative to room atmosphere
    Serial.print(smokeSensorValue);
    tb.sendTelemetryInt("smoke value", smokeSensorValue);
    tb.sendTelemetryString("smoke", "Smoke detected");
    Serial.println(" | Smoke Detected!!!");
    soundFireAlarm();
    tb.sendTelemetryString("smoke", "No smoke detected");
  }
  else{
    Serial.println(smokeSensorValue);
    tb.sendTelemetryInt("smoke value", smokeSensorValue);
  }
  delay(2000);
}

//RFID Access check
void checkRFID(){
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()){
    return;
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()){
    return;
  }

  //Show UID on serial monitor
  Serial.print("UID tag :");
  String content= "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++){
     Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
     Serial.print(mfrc522.uid.uidByte[i], HEX);
     content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  Serial.print("Message : ");
  content.toUpperCase();
  if (content.substring(1) == "EE EE 13 58"){ //work pass UID
    Serial.println("Access Granted");
    Serial.println("User-1 has entered the Server Room.");
    tb.sendTelemetryString("Name", "User-1");
    tb.sendTelemetryString("attemptedEntryName", "User-1");
    openDoorRoutine();
    entrances++;
    attemps = 0;
    tb.sendTelemetryString("Name", "-");

  }
  else if(content.substring(1) == "23 5A F8 33"){// blue tag's UID
    Serial.println("Access Granted");
    Serial.println("User-2 has entered the Server Room.");
    tb.sendTelemetryString("Name", "User-2");
    tb.sendTelemetryString("attemptedEntryName", "User-2");
    openDoorRoutine();
    entrances++;
    attemps = 0;
    tb.sendTelemetryString("Name", "-");
  }
  else{
    Serial.println("Access denied");
    tb.sendTelemetryString("Name", "Unknown Subject");
    //tb.sendTelemetryString("door", "Access denied");
    tb.sendTelemetryString("attemptedEntryName", "Unknown");
    attemps++;
    if(attemps < 3){
      analogWrite(redLedPin, 255);
      delay(3000);
      analogWrite(redLedPin, 0);
    }
    else{
      soundIntrusionAlarm();
      attemps = 0;
    } 
    tb.sendTelemetryString("Name", "-");   
  }
  //mfrc522.PICC_DumpToSerial(&(mfrc522.uid)); if we want to see tag binary
} 

//Motion detection
void checkMotion(){
  motionSensorInput = digitalRead(motionSensorPin);
  if ( motionSensorInput == HIGH ){
    Serial.println("Motion detected!!!");
    tb.sendTelemetryString("Motion", "Motion detected!!!");
    soundIntrusionAlarm();
    tb.sendTelemetryString("Motion", "All is quiet...");
  } 
  else{
    Serial.println("All is quiet...");
  }
}

//--------------------------------------------------Routines-------------------------------------------------//

//open Air Condition routine
void openAirCondition(){
  ACStatus = true;  
  analogWrite(blueLedPin, 256);
  tb.sendTelemetryString("AC", "ON");
  digitalWrite(buzzerPin, HIGH);
  delay(1000);
  digitalWrite(buzzerPin, LOW);
}

//close Air Condition routine
void closeAirCondition(){
  ACStatus = false;
  analogWrite(blueLedPin, 0);
  tb.sendTelemetryString("AC", "OFF");
}

//smoke detected routine
void soundFireAlarm(){
  for(int i=0;i<10;i++){ //10 replays for presentation purposes
    analogWrite(redLedPin, 255);
    analogWrite(greenLedPin, 255);
    digitalWrite(buzzerPin, HIGH);
    delay(300);
    analogWrite(redLedPin, 0);
    analogWrite(greenLedPin, 0);
    digitalWrite(buzzerPin, LOW);
    delay(200);
  }
}

//function for open door routine
void openDoorRoutine(){
  analogWrite(greenLedPin, 255); 
  digitalWrite(buzzerPin, HIGH);
  delay(500); 
  digitalWrite(buzzerPin, LOW);
  servo.write(100);
  String door = String("OPEN"); // ?????
  tb.sendTelemetryString("door", "OPEN");
  delay(5000);
  servo.write(10);
  analogWrite(greenLedPin, 0);
  door = String("CLOSED");  // ?????
  tb.sendTelemetryString("door", "CLOSED");
  delay(1000);
}

//motion detected routine
void soundIntrusionAlarm(){
  for(int i=0;i<10;i++){ //10 replays for presentation purposes
    analogWrite(redLedPin, 255);
    analogWrite(blueLedPin, 255);
    digitalWrite(buzzerPin, HIGH);
    delay(300);
    analogWrite(redLedPin, 0);
    analogWrite(blueLedPin, 0);
    digitalWrite(buzzerPin, LOW);
    delay(200);
  }
}

//---------------------------------Connection to WiFi and ThingsBoard--------------------------------------------------//

//wifi status
void printWifiStatus() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

//reconnection to Thingsboard
void reconnect() {
  // Loop until we're reconnected
  while (!tb.connected()) {
    Serial.print("Connecting to ThingsBoard node ...");
    // Attempt to connect (clientId, username, password)
    if ( tb.connect(thingsboardServer, token) ) {
      Serial.println( "[DONE]" );
    } else {
      Serial.print( "[FAILED]" );
      Serial.println( " : retrying in 5 seconds" );
      // Wait 5 seconds before retrying
      delay( 5000 );
    }
  }
}

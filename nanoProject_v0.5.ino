#include <Servo.h> // Servo Library
#include <DHT.h> // Humidity Temprature Library
#include <SPI.h> //RFID communication Library
#include <MFRC522.h> // RFID sensor Library
#include <WiFiNINA.h> //WiFi Library

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

char ssid[] = "WIFI-Name";
char pass[] = "WIFI-Pass";
int keyIndex = 0;
int status = WL_IDLE_STATUS;

int motionSensorInput = LOW;
int smokeSensorValue = 0;
int attemps = 0;
bool ACStatus = false;

void setup() {
  //initialize serial and pin headers
  Serial.begin(9600);
  servo.attach(servoPin);
  servo.write(0);
  tempSensor.begin();

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
  Serial.println();

//  // check for the WiFi module:
//   if (WiFi.status() == WL_NO_MODULE) {
//     Serial.println("Communication with WiFi module failed!");
//     // don't continue
//     while (true);
//   }

//   String fv = WiFi.firmwareVersion();
//   if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
//     Serial.println("Please upgrade the firmware");
//   }

//   // by default the local IP address will be 192.168.4.1
//   // you can override it with the following:
//   // WiFi.config(IPAddress(10, 0, 0, 1));

//   // print the network name (SSID);
//   Serial.print("Creating access point named: ");
//   Serial.println(ssid);

//   // Create open network. Change this line if you want to create an WEP network:
//   status = WiFi.beginAP(ssid, pass);
//   if (status != WL_AP_LISTENING) {
//     Serial.println("Creating access point failed");
//     // don't continue
//     while (true);
//   }

//   // wait 10 seconds for connection:
//   delay(10000);

//   // you're connected now, so print out the status
//   printWiFiStatus();
}

  void loop(){
    checkMotion();
    checkSmoke();
    checkTemp();
    checkRFID();
  }

//------------------------------------------------------//
// Monitor Functions

//Motion detection
void checkMotion(){
  motionSensorInput = digitalRead(motionSensorPin);
  if ( motionSensorInput == HIGH ){
    Serial.println("Motion detected!!!");
    soundIntrusionAlarm();
  } 
  else{
    Serial.println("All is quiet...");
  }
}

//Smoke detection
void checkSmoke(){
  smokeSensorValue = analogRead(smokeSensorPin);
  Serial.print("Smoke Sensor value : ");
  if(smokeSensorValue > 680){
    Serial.print(smokeSensorValue);
    Serial.println(" | Smoke Detected!!!");
    soundFireAlarm();
  }
  else{
    Serial.println(smokeSensorValue);
  }
  delay(2000);
}

//Temprature and Humidity Reading
void checkTemp() {
  int readData = tempSensor.read(tempSensorPin);
  float temp = tempSensor.readTemperature();        // Read temperature
  float hum = tempSensor.readHumidity();           // Read humidity

  Serial.print("Temperature = ");
  Serial.print(temp);
  Serial.print("°C | ");
  Serial.print("Humidity = ");
  Serial.print(hum);
  Serial.println("% ");

  if(temp > 22 && ACStatus == false){
    openAirCondition();
  }
  else if (temp <= 22 && ACStatus == true){
    closeAirCondition();
  }
  else{    
  }
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
  if (content.substring(1) == "23 5A F8 33"){ //blue tag's UID
    Serial.println("Access Granted");
    openDoorRoutine();
    attemps = 0;
  }
 else{
    Serial.println("Access denied");
    attemps++;
    if(attemps < 3){
      analogWrite(redLedPin, 255);
      delay(3000);
      analogWrite(redLedPin, 0);
    }
    else{
      soundIntrusionAlarm();
    }    

  }

  //mfrc522.PICC_DumpToSerial(&(mfrc522.uid)); if we want to see tag binary
} 

//----------------------------------------------//
//Routines

//function for open door routine
void openDoorRoutine(){
  analogWrite(greenLedPin, 255); 
  digitalWrite(buzzerPin, HIGH);
  delay(500); 
  digitalWrite(buzzerPin, LOW);
  servo.write(90);
  delay(5000);
  servo.write(0);
  analogWrite(greenLedPin, 0);
  delay(1000);
}

//smoke detected routine
void soundFireAlarm(){
  for(int i=0;i<10;i++){ //10 replayes for presentation purposes
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

//motion detected routine
void soundIntrusionAlarm(){
  for(int i=0;i<10;i++){ //10 replayes for presentation purposes
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

//open Air Condition routine
void openAirCondition(){
  ACStatus = true;  
  analogWrite(blueLedPin, 256);
  digitalWrite(buzzerPin, HIGH);
  delay(1000);
  digitalWrite(buzzerPin, LOW);
}

//close Air Condition routine
void closeAirCondition(){
  ACStatus = false;
  analogWrite(blueLedPin, 0);
  // digitalWrite(buzzerPin, HIGH);
  // delay(1000);
  // digitalWrite(buzzerPin, LOW);
}


// void printWiFiStatus(){
//    // print the SSID of the network you're attached to:
//   Serial.print("SSID: ");
//   Serial.println(WiFi.SSID());

//   // print your WiFi shield's IP address:
//   //IPAddress ip = WiFi.localIP();
//   WiFi.config(IPAddress(192, 168, 1, 100));
//   Serial.print("IP Address: ");
//  //Serial.println(ip);
// }

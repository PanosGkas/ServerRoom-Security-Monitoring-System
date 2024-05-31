#include <Servo.h> // Servo Library
#include <DHT.h> // Humidity Temprature Library


#include <SPI.h>
#include <MFRC522.h> // RFID sensor Library

//define the sensor pins
#define smokeSensorPin A0
#define motionSensorPin 2
#define servoPin 3
#define buzzerPin 4
#define redLedPin 5
#define blueLedPin 6
#define greenLedPin 7
#define tempSensorPin 8 

#define DHTTYPE DHT11


#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);

//object creation and initial values definition
Servo servo;
DHT tempSensor(tempSensorPin, DHTTYPE);   
int motionSensorInput = LOW;
int smokeSensorValue;

void setup() {
  //initialize pin headers
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
  }

  void loop(){
    //checkMotion();
    //soundBuzzer();
    //servoMovement();
    //checkSmoke();
    //checkTemp();
    //testLED();
    checkRFID();
  }

//function for RFID
void checkRFID(){

  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) 
  {
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
    return;
  }

  //Show UID on serial monitor
  Serial.print("UID tag :");
  String content= "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
     Serial.print(mfrc522.uid.uidByte[i], HEX);
     content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  Serial.print("Message : ");
  content.toUpperCase();
  if (content.substring(1) == "23 5A F8 33") //εδώ πρέπει να αλλάξουμε το UID που θέλουμε εμείς
  {
    
    Serial.println("Authorized access");
    analogWrite(greenLedPin, 255); // με το delay το χρώμα θα κρατήσει για 3 sec
    Serial.println();
    soundBuzzer();
    stopBuzzer();
    openDoor();
    analogWrite(greenLedPin, 0);

  }
 
 else   {
    Serial.println("Access denied");
    analogWrite(redLedPin, 255);
    delay(3000);
    analogWrite(redLedPin, 0);
  }
    mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
} 


//function to test cathode LEDs
void testLED(){
analogWrite(blueLedPin, 256);
delay(2000);
analogWrite(blueLedPin, 0);
delay(1000);
analogWrite(redLedPin, 256);
delay(2000);
analogWrite(redLedPin, 0);
delay(1000);
analogWrite(greenLedPin, 256);
delay(2000);
analogWrite(greenLedPin, 0);
delay(1000);

}
//function to open door
void openDoor(){
  servo.write(90);
  delay(5000);
  servo.write(0);
  delay(1000);
}

//function for motion detection with buzzer alarm, blue LED indicator and serial print
void checkMotion(){
  motionSensorInput = digitalRead(motionSensorPin);
  if ( motionSensorInput == HIGH )
  {
    Serial.println("Motion detected!!!");
    soundBuzzer();
    //analogWrite(blueLedPin, 256);
    delay(1000);
  } 
  else
  {
    Serial.println("All is quiet...");
    digitalWrite(buzzerPin, LOW);
    //analogWrite(blueLedPin, 0);
  }
  while ( motionSensorInput == digitalRead(motionSensorPin) )
  {
      delay(100);
  }
}

//function for smoke detection above 630 with buzzer alarm, red LED indicator and serial print. use burning toothpick to make smoke
void checkSmoke(){
  smokeSensorValue = analogRead(smokeSensorPin);
  Serial.print("Sensor value : ");
  Serial.println(smokeSensorValue);
  stopBuzzer();
  //analogWrite(redLedPin, 0);
  if(smokeSensorValue > 680){
    Serial.print(" | Smoke Detected!!!");
    soundBuzzer();
    //analogWrite(redLedPin, 256);
  }
  delay(2000);
}

//buzzer functions for tone and no tone
void stopBuzzer(){
  digitalWrite(buzzerPin, LOW);
}

void soundBuzzer(){
  digitalWrite(buzzerPin, HIGH);
  delay(500); 
}

//!!!NOT TESTED!!!
//function for temprature detection above 50C with buzzer alarm, green LED indicator and Serial print 
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
  Serial.println("");

  if(temp > 50){
    soundBuzzer();
    analogWrite(greenLedPin, 256);
  }
  else{
    stopBuzzer();
    analogWrite(greenLedPin, 0);
  }

  delay(2000); // wait two seconds
}

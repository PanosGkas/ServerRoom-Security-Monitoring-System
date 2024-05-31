#include <Servo.h>
#include <DHT.h> 

//define the sensor pins
#define smokeSensorPin A0
#define buzzerPin 4
#define redLedPin 5
#define blueLedPin 6
#define greenLedPin 7
#define tempSensorPin 8 
#define motionSensorPin 9

//creation of objects
Servo servo;
//DHT tempSensor;   
int motionSensorInput = LOW;
int smokeSensorValue;

void setup() {
  //initialize pin headers
  Serial.begin(9600);
  servo.attach(2);
  servo.write(0);
  pinMode(redLedPin, OUTPUT);
  pinMode(blueLedPin, OUTPUT);
  pinMode(greenLedPin, OUTPUT);
  pinMode(motionSensorInput, INPUT); 
  pinMode(buzzerPin, OUTPUT);
  pinMode(smokeSensorPin, INPUT);
  }

  void loop(){
    //checkMotion();
    //soundBuzzer();
    //servoMovement();
    //checkSmoke();
    //checkTemp();
  }

//function for servo movement
void servoMovement(){
  servo.write(90);
  delay(2000);
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
  if(smokeSensorValue > 630){
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
  delay(1000); 
}

//!!!NOT TESTED!!!
//function for temprature detection above 50C with buzzer alarm, green LED indicator and Serial print 
// void checkTemp() {
//   int readData = tempSensor.read11(tempSensorPin);
//   float temp = tempSensor.temperature;        // Read temperature
//   float hum = tempSensor.humidity;           // Read humidity

//   Serial.print("Temperature = ");
//   Serial.print(temp);
//   Serial.print("°C | ");
//   Serial.print("Humidity = ");
//   Serial.print(hum);
//   Serial.println("% ");
//   Serial.println("");

//   if(temp > 50){
//     soundBuzzer();
//     //analogWrite(greenLedPin, 256);
//   }
//   else{
//     stopBuzzer();
//     //analogWrite(greenLedPin, 0);
//   }

//   delay(2000); // wait two seconds
// }
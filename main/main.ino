#define sensorPower 12
#define sensorPin A1
/* How to use the TEA5767 FM radio Module with Arduino
   More info: http://www.ardumotive.com/how-to-use-the-tea5767-fm-radio-module-en.html
   Dev: Vasilakis Michalis // Date: 21/9/2015 // www.ardumotive.com    */

//Libraries:
#include <TEA5767.h>
#include <Wire.h>
#include <Arduino_FreeRTOS.h>

//Constants:
TEA5767 Radio; //Pinout SLC and SDA - Arduino uno pins A5 and A4


float channels[4];
int index = 0;
int radioPin = 31;
// Value for storing water level
int val = 0;

int h1 = 5;
int h2 = 6;
int h3 = 7;
int h4 = 8;
int ENA = 9;
int ENB = 10;
int buzzer = 52;

int servoPin = 11;

int right = 0;
int depth = A1;

int A = 14;
int B = 15;
int C = 16;
int D = 17;

void TaskLKA( void *pvParameters );
void TaskRainSensor( void *pvParameters );
void TaskFuelLevel( void *pvParameters );
void TaskRadio( void *pvParameters );

void setup() {
  // put your setup code here, to run once:
  Wire.begin();
  Serial.begin(9600);
  init0();
  xTaskCreate(
    TaskLKA
    ,  "task1"   
    ,  128  
    ,  NULL
    ,  1  
    ,  NULL );

  xTaskCreate(
    TaskRainSensor
    ,  "task2"   
    ,  128  
    ,  NULL
    ,  1  
    ,  NULL );

  xTaskCreate(
    TaskFuelLevel
    ,  "task3"   
    ,  128  
    ,  NULL
    ,  1  
    ,  NULL );  
     
  xTaskCreate(
    TaskRadio
    ,  "task4"   
    ,  128  
    ,  NULL
    ,  1  
    ,  NULL );

  vTaskStartScheduler();
}

void init0(){
  pinMode(2, INPUT);
  pinMode(3, INPUT);
  pinMode(4, INPUT);

  pinMode(depth, INPUT);

  pinMode(h1, OUTPUT);
  pinMode(h2, OUTPUT);
  pinMode(h3, OUTPUT);
  pinMode(h4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  analogWrite(ENA, 127);
  analogWrite(ENB, 127);
  pinMode(buzzer, OUTPUT);
  
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(servoPin, OUTPUT);

  pinMode(A, OUTPUT);
  pinMode(B, OUTPUT);
  pinMode(C, OUTPUT);
  pinMode(D, OUTPUT);

   // Set D7 as an OUTPUT
  pinMode(sensorPower, OUTPUT);
  
  // Set to LOW so no power flows through the sensor
  digitalWrite(sensorPower, LOW);

  // Radio setup
  pinMode(radioPin, OUTPUT);
  Radio.init();
  channels[0] = 100.6;
  channels[1] = 107.4;
  channels[2] = 108;
  channels[3] = 104.2;
  Radio.set_frequency(channels[index]); 
  
}

void TaskLKA( void *pvParameters ){
  while(1){
    Hbridge();
  }
  
}

void TaskRainSensor( void *pvParameters ){
  while(1){
    rainSensor();
  }
}

void TaskFuelLevel( void *pvParameters ){
  while(1){
     levelsensor();
  }
}

void TaskRadio( void *pvParameters ){
  while(1){
    radio();
  }
}

void lineFollower(){
  int val2 = digitalRead(2);
  int val3 = digitalRead(3);
  int val4 = digitalRead(4);
  Serial.print(val2);
  Serial.print(" ");
  Serial.print(val3);
  Serial.print(" ");
  Serial.print(val4);
  Serial.println(" "); 
}

void Hbridge(){
  int val2 = digitalRead(2);
  int val3 = digitalRead(3);
  int val4 = digitalRead(4);

  if(val2 == 1 && val4 ==1){
    Serial.print("first");
    digitalWrite(buzzer,LOW);
    digitalWrite(h1,LOW);
    digitalWrite(h2,HIGH);
    digitalWrite(h3,LOW);
    digitalWrite(h4,HIGH);
  }
  else if(val2 == 0 && val4 ==1){ //the directions are wrong
    Serial.print("second");
    digitalWrite(buzzer,HIGH);
    digitalWrite(h1,LOW);
    digitalWrite(h2,HIGH);
    digitalWrite(h3,HIGH);
    digitalWrite(h4,LOW);
  }
  else if(val2 == 1 && val4 ==0){ //the directions are wrong
    Serial.print("3");
    digitalWrite(buzzer,HIGH);
    digitalWrite(h1,HIGH);
    digitalWrite(h2,LOW);
    digitalWrite(h3,LOW);
    digitalWrite(h4,HIGH);
  }
  else{
    Serial.print("4th");
    digitalWrite(buzzer,LOW);
    digitalWrite(h1,LOW);
    digitalWrite(h2,LOW);
    digitalWrite(h3,LOW);
    digitalWrite(h4,LOW);
  }
}

void servoRotation() {
  if (right == 0) {
    analogWrite(servoPin, 128);
    right = 1;
  }
  else {
    analogWrite(servoPin, 254);
    right = 0;
  }
  vTaskDelay( 200 / portTICK_PERIOD_MS );
  // todo

}
void sevenSegment(int val){
  if(val == 0){
    digitalWrite(A, LOW);
    digitalWrite(B, LOW);
    digitalWrite(C, LOW);
    digitalWrite(D, LOW);    
  }else if(val == 1){
    digitalWrite(A, HIGH);
    digitalWrite(B, LOW);
    digitalWrite(C, LOW);
    digitalWrite(D, LOW);    
  }else{
    digitalWrite(A, LOW);
    digitalWrite(B, HIGH);
    digitalWrite(C, LOW);
    digitalWrite(D, LOW);    
  }
}

void rainSensor() {
  // put your main code here, to run repeatedly:
  int sensorValue = analogRead(A0);
  Serial.println(sensorValue);
  if (sensorValue < 400) {

    servoRotation();
    //turn on servo
    Serial.println("Servo on");
    digitalWrite(LED_BUILTIN, HIGH);
  }
  else {
    //turn off servo
    analogWrite(servoPin,0);
    Serial.println("Servo off");
    digitalWrite(LED_BUILTIN, LOW);
  }
  //vTaskDelay( 1000 / portTICK_PERIOD_MS ); 
}

void levelsensor(){
      //get the reading from the function below and print it
       int level = readSensor();
    if(level < 40){
      sevenSegment(0);  
    }else if(level < 450    ){
      sevenSegment(1);      
    }else{
      sevenSegment(2);  
    }
    Serial.print("Water level: ");
    Serial.println(level);
    
    vTaskDelay( 1000 / portTICK_PERIOD_MS ); 
}

//This is a function used to get the reading
int readSensor() {
  digitalWrite(sensorPower, HIGH);  // Turn the sensor ON
  vTaskDelay( 10 / portTICK_PERIOD_MS ); 
  val = analogRead(sensorPin);    // Read the analog value form sensor
  digitalWrite(sensorPower, LOW);   // Turn the sensor OFF
  return val;             // send current reading
}
void radio()
{
    Wire.requestFrom(0x05, 1);    // request 1 byte from slave device #5

  while (Wire.available()) { // slave may send less than requested
    byte c = Wire.read(); // receive a byte as character
    if(c == 4){
      digitalWrite(radioPin, LOW);
    }else if(index != c){
      digitalWrite(radioPin, HIGH);
      Radio.set_frequency(channels[index]);     
    }
    index = c; 
    Serial.println(c);         // print the character
  }

//  Serial.println(channels[index]);
    vTaskDelay( 200 / portTICK_PERIOD_MS ); 
}
void loop() {
  
}

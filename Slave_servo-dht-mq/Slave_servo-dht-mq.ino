#include <MQUnifiedsensor.h>
#include <DHT.h>
#include <AM2320.h>
#include<Wire.h>
#include <Servo.h>

Servo servoDamper;
AM2320 th(&Wire);

// ********* Global variables **********
const byte servoMax = 30;  // Max-limit for servo
const byte servoMin = 0;   // Min-limit for servo
short int Quality = 0;     // Aire Quality
byte mog = 0;              // Mold index
short int currentStudentCounter = 0;

byte minTemp = 20;
byte maxTemp = 25;
byte maxHum = 87;
byte minHum = 25;
short int maxAirQuality = 500; 

unsigned long updateInterval = 600000;
unsigned long time_now = 0;

void setup() {
  
  Serial.begin(9600);
  
  Wire.begin(3);      //Start I2C connetion as slave with address 3
     
  pinMode(7,  OUTPUT);   // Green LED
  pinMode(4,  OUTPUT);   // Red LED
  pinMode(10, OUTPUT);   // Yellow LED
  pinMode(11, OUTPUT);   // Blue LED 
  pinMode(12, INPUT);    // Servo swish ON button
  pinMode(13, INPUT);    // Servo swish OFF button
  pinMode(1,  INPUT);    // Air quality sensor
  
  servoDamper.attach(2);       // Attach the Servo at pin 2
  servoDamper.write(servoMin); // Set the Servo at clos mod
  
  digitalWrite(10, LOW);       // set the yellow LED off
  digitalWrite(11, HIGH);      // set the blue LED on
   
  // Attach a function to trigger when something is received.
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
}


void loop() {
  
  //get_aensorsValus();     // This function used by technical support to test sensor functionality
  
  th.Read();                //Read Humi and temp from the sensor 
  Quality = analogRead(1);  //Read Air quality from the sensor 
  print_sensorsValues();

  //   ******** Check the manually start and stop buttons to servo *******
  if (digitalRead(13)== HIGH )
    {
      openDegree(20);      // manually oppen the servo for 20 persons
      delay(2000);
    }
    
  if (digitalRead(12)== HIGH )
    {
      openDegree(servoMin);  // manually close the servo
    }

  get_moldIndex();          // Get mold index

   if(millis() > time_now + updateInterval)
   {
    controlServo(th.cTemp,th.Humidity,analogRead(1));
    time_now = millis();
   }

  
  
  delay(1500);
 
}


                //  ************* Servo functions *************

void openDegree(short int studentCounter) 
{
  if (studentCounter > 30 )
    {
      
      studentCounter = servoMax;
    }
    else if( studentCounter < 0)
    {
      studentCounter = servoMin;
    }
  servoDamper.write(studentCounter*3); //Opens the servo due to how many students has tagged in
  
  if ( studentCounter > 0 )
   {    
     digitalWrite(10, HIGH);   // set the yellow LED on
     digitalWrite(11, LOW );   // set the blue LED off     
   }
   
  else
   {
     digitalWrite(10, LOW );   // set the yellow LED off
     digitalWrite(11, HIGH);   // set the blue LED on
   }  
}


// Automatic servo control by sensors values
void controlServo(float currentTemperature,float currentHumidity,short int currentQuality)
{
  
    if((currentTemperature > maxTemp )|| (currentHumidity > maxHum) || (currentQuality > maxAirQuality))
      {
          openDegree(servoMax);
          digitalWrite(7, LOW);         // Set the Green LED off
          digitalWrite(4, HIGH);        // Set the Red LED on   
      }

  else if ((currentTemperature < minTemp || currentHumidity < minHum ) && (currentQuality < maxAirQuality))
      {
          openDegree(servoMin);
          digitalWrite(7, LOW);         // Set the Green LED off
          digitalWrite(4, HIGH);        // Set the Red LED on    
      }

  else
  {
     openDegree(currentStudentCounter);
     digitalWrite(7,HIGH);           // Set the Green LED off
     digitalWrite(4, LOW);           // Set the Red LED on  
  }
  
}


//  ************* Connections functions *************
void receiveEvent() 
{ 
  currentStudentCounter = Wire.read();    // read one character from the I2C
  Serial.print(F("Studen counter = "));
  Serial.println(currentStudentCounter);       
  
  openDegree(currentStudentCounter);     // call servo function              
}

void requestEvent()
{
  String sendTemp = String (th.cTemp);
  String sendHumidity = String (th.Humidity);
  String sendQuality = String (Quality);
  
  //put all sensors values in one packet to send it
  String sensorsValues = sendTemp + sendHumidity + sendQuality;

  Serial.print(F("sensorsValues: "));
  Serial.println(sensorsValues);
  
  Wire.write(sensorsValues.c_str()); //Send sensors values to master
  
}


//  ************* check sensors Functionality ************
void chech_sensorsFunctionality()
{
  switch(th.Read()) {
    case 2:
      Serial.println(F("  CRC failed"));
      break;
    case 1:
      Serial.println(F("  Sensor offline"));
      break;
    case 0:
      Serial.print(F("  Humidity = "));
      Serial.print(th.Humidity);
      Serial.println(F("%"));
      Serial.print(F("  Temperature = "));
      Serial.print(th.cTemp);
      Serial.println(F("*C"));
      
      
      mog = calcMog(th.cTemp,th.Humidity);
      if(mog!=0)
        {
          digitalWrite(7, LOW);                  // set the Green LED off
          digitalWrite(4, HIGH);                // set the Red LED on
          Serial.print(F("  MogelIndex = "));
          Serial.println(mog);
        }
      else
        {
          digitalWrite(4, LOW);          // set the Red LED off 
          digitalWrite(7, HIGH);         // set the LED on
        }
      
      break;
  }
    
}


void print_sensorsValues()
{
  Serial.print(F(" Humidity = "));
  Serial.print(th.Humidity);
  Serial.println(F("%"));
  Serial.print(F(" Temperature = "));
  Serial.print(th.cTemp);
  Serial.println(F("*C"));
  
  Quality = analogRead(1); 
  Serial.print(" AirQuality = ");
  Serial.println (Quality);
  Serial.println();
}


//  ************* calculet th mogelindex ************
byte calcMog(float temp, int humi)
{
  byte mogel = 0;

  if (humi >= 78 && temp < 15 && temp >= 0)
  {
    // When the temperature drops 1 degree below 15, humidity should increase by 1.7%
    short int t = (short int)(15 - temp);
    humi = humi - (short int)(t * 1.7);

    if (humi >= 78 && humi < 81)
      mogel = 1;
    if (humi >= 81 && humi < 84)
      mogel = 2;
    if (humi >= 84 && humi < 87)
      mogel = 3;
    if (humi >= 87 && humi < 90)
      mogel = 4;
    if (humi >= 90 && humi < 93)
      mogel = 5;
    if (humi >= 93)
      mogel = 6;
  }
  if (humi >= 78 && temp >= 15)
  {
    short int t0 = (short int)temp - 15;
    humi = humi + (short int)(t0 * 1.7);

    if (humi >= 78 && humi < 81)
      mogel = 1;
    if (humi >= 81 && humi < 84)
      mogel = 2;
    if (humi >= 84 && humi < 87)
      mogel = 3;
    if (humi >= 87 && humi < 90)
      mogel = 4;
    if (humi >= 90 && humi < 93)
      mogel = 5;
    if (humi > 93)
      mogel = 6;
  }
  return mogel;

}

void get_moldIndex()
{
  mog = calcMog(th.cTemp,th.Humidity);
  if(mog!=0)
    {
      digitalWrite(7, LOW);                  // set the Green LED off
      digitalWrite(4, HIGH);                 // set the Red LED on
      Serial.print(F("  MogelIndex = "));
      Serial.println(mog);
    }
   else
    {
      digitalWrite(4, LOW);          // set the Red LED off 
      digitalWrite(7, HIGH);         // set the green LED on
    }
}

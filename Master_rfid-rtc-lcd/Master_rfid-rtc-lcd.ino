/*
 This is the Master-arduino in our system and using I2C-protocol to communicate with the slave-arduinos. The hardware used for this is: 1x Arduino UNO, 1x MFRC522 (RFID), 1x RTC (Real Time Clock), 1x LCD-sceen and 3x LEDs (green, yellow and red).
 PIN 9,10,11,12 and 13 is used for the RFID. The clock is using PIN SCL and SDA. 
 PIN 5,6 and 7 is used for LEDs where (5=green,6=yellow and 7=red). PIN A5 and A4 is used for the I2C connection inorder to communicate with the slave-arduinos.
 The purpose of this arduino is to keep track of the classroom-schedule,attendance and time. The idea was to store the schedule wich consists of Starttime, Endtime, 
 a valid list of students who are allowed to enter the class.
 This arduino will check the Tag-adress and compare it with the StudentID, and if he or she is a valid person for the class it will log their attendance.
 To this Sketch we have created three different ".h"-files wich contains classes with functions for attendance registration, a list to store valid students for the current classroom.
 we also created a "schedule"-class, where the idea was to send schedule to each different classroom. We ended up not using this one in order to save memory.
 This arduino also has a few build in-functions for the servo/"ventilation". When the class starts wich is defined with the schedule, master will send a signal for the servo 
 to open up for five people, when students starts tagging in, the servo will then open up more depending on how many students has tagged in. When class ends, servo will close.  
*/
//#include<Arduino.h>
#include <Wire.h>
#include "RTClib.h"
#include <deprecated.h>
#include <MFRC522.h>
#include <MFRC522Extended.h>
#include <require_cpp11.h>
#include "Attendance.h"
#include "ValidList.h"
//#include "scheduel.h"                                                                           // We do not use this .h-file in this version, due to memory. Instead we have declared a schedule with variables in this code.
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

RTC_DS1307 RTC;                                                                                   // Creates a real time clock-object.
LiquidCrystal_I2C lcd(0x27,16,2);                                                                 // Set the LCD address to 0x27 for a 16 chars and 2 line display.
 
Attendance* studentAttendanceList = new Attendance[10];                                           // This array contain the valid UID that authorized to get in the classroom.
ValidList *validList = new ValidList[10];                                                         // This array contain those valid UID that got in the classroom "presenceList".
Attendance* newAttendance  = new Attendance() ;
ValidList* newValidList = new ValidList();

const byte wifi = 2;                                                                              // Adress to slave-arduino with wifi and database syntax.
const byte servo = 3;                                                                             // Adress to slave-arduino with server and sensor syntax.

unsigned long updateInterval = 60000;                                                             // 1 Minute interval, used for sensorvalue reading.
unsigned long time_now = 0;

int x = 5;                                                                                        // Sets x as 5 and sends it to servo/"ventilation" when class starts.
uint8_t programStartMonth = 3;                                                                    // Startmonth for the schedule for the current class.
uint8_t programStartDay = 26;                                                                     // Startday for schedule for the current class.
uint8_t programStartHour = 12;                                                                    // Starthour for schedule for the current class.
uint8_t programStartMinute = 59;                                                                   // Startminute for schedule for the current class.

uint8_t scheduleEndtHour = 15;                                                                    // Endtime for class.
uint8_t scheduleEndMinute = 59;                                                                   // Endminute for class.

static short int counter;
static bool startServo;
static bool stopServo;

String recivedSensorsValues;
byte recivedHumi;
byte recivedQuality;

#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);                                                                 // Create MFRC522 instance.

 
void setup() 
{  
  counter = 0;
  startServo = false;
  stopServo = false;
  Serial.begin(9600);                                                                             // Initiate a serial communication.
  Wire.begin();                                                                                   // Initiate  I2C bus.
  SPI.begin();                                                                                    // Initiate  SPI bus.
  RTC.begin();                                                                                    // Start a real time clock.        
  if (! RTC.isrunning())                                                                          // Check to see if the RTC is keeping time. If it is, load the time from your computer. 
   {
     Serial.println("RTC is NOT running!");
     RTC.adjust(DateTime(__DATE__, __TIME__));                                                    // This will reflect the time that your sketch was compiled.
   }
  mfrc522.PCD_Init();                                                                             // Initiate MFRC522.
  lcd.init();                                                                                     // Initiate LCD.
    
  Serial.println(F("Approximate your card to the reader..."));
  Serial.println();
    
  //*********Bring the valid ID list for attendance*********
  
  newValidList->set_validPerson("47 3B D0 83" , 19001 , 0);
  validList[0]=(*newValidList);  
  newValidList->set_validPerson("97 FC 81 C3" , 19002 , 0);
  validList[1]=(*newValidList);  
  newValidList->set_validPerson("82 C2 6D CE" , 19003 , 0);
  validList[2]=(*newValidList);
  newValidList->set_validPerson("C9 FB B4 E4" , 19004 , 0);
  validList[3]=(*newValidList);
  newValidList->set_validPerson("C9 E6 BA E4" , 19005 , 0);
  validList[4]=(*newValidList);
  newValidList->set_validPerson("09 D1 B8 E4" , 19006 , 0);
  validList[5]=(*newValidList);
  newValidList->set_validPerson("C1 F1 CD 7E" , 19007 , 0);
  validList[6]=(*newValidList);
  newValidList->set_validPerson("4D 96 5B A7" , 19008 , 0);
  validList[7]=(*newValidList);
  newValidList->set_validPerson("72 E1 11 44" , 19009 , 0);
  validList[8]=(*newValidList);  
  newValidList->set_validPerson("A2 CA 61 8F" , 19010 , 0);
  validList[9]=(*newValidList);
  pinMode(5, OUTPUT);                                                                              // Sets PINMODE of the LEDs as outputs.
  pinMode(6, OUTPUT);                                                                              // Sets PINMODE of the LEDs as outputs.
  pinMode(7, OUTPUT);                                                                              // Sets PINMODE of the LEDs as outputs.   
}

void loop() 
{
     DateTime now = RTC.now();
     if(counter == 0)
     {
       Serial.print (now.month());
       Serial.print(" - ");
       Serial.println (now.day());
       Serial.print (now.hour());
       Serial.print (":");
       Serial.println (now.minute());
     }     
     String text1="";                                                                              // String declaration for LCD-print.
     String text2="";                                                                              // String declaration for LCD-print.
     start_LCD(" SAL 3    IOT19","  " + String(programStartHour) + ":" + String (programStartMinute) + " - " + String (scheduleEndtHour) + ":" + String (scheduleEndMinute));
         
   if ( now.month() == programStartMonth && now.day() == programStartDay )
    {
        if ( now.hour() >= programStartHour   && now.hour() <= scheduleEndtHour )
          {
            if (now.minute() >= programStartMinute   && now.minute() <= scheduleEndMinute )
              {                                                                                                 
                  if(startServo == false)                                                           // Open the ventilation for five persons when class starts.
                  {
                    Serial.println(F("The Servo is in normal open mode "));
                    Wire.beginTransmission(servo);                                                  // transmit to slave-device #3.
                    Wire.write(5);                                                                  // sends x. 
                    Wire.endTransmission();                                                         // stop transmitting.
                    startServo=true;
                    stopServo=false;
                    digitalWrite(7, LOW);
                  }

                  if(millis() > time_now + updateInterval)                                          // A timer for the read sensorvalues request.
                    {                      
                       Serial.println(F("S"));
                       Wire.requestFrom(servo,13);                                                  // Sends a request for servo/sensor-arduino to deliver sensorvalues.
                       while (Wire.available())         
                          { 
                            char c = Wire.read();                    
                            recivedSensorsValues+=c; // Builds a string                     
                          }                        
                       Serial.println(recivedSensorsValues);        
                       Wire.beginTransmission(wifi);                                                // Transmit to device #2 for uploading to database.
                       Wire.write(3);                                                               // Sends x. 
                       Wire.write(recivedSensorsValues.c_str());
                       Wire.endTransmission();                                                      // Stop transmitting.
                       recivedSensorsValues = "";
                       time_now = millis();
                    }
                
                if ( ! mfrc522.PICC_IsNewCardPresent())                                             // Look for new cards. 
                  {
                    return;
                  }                
                if ( ! mfrc522.PICC_ReadCardSerial())                                               // Select one of the cards. 
                  {
                    return;
                  }               
                String content= "";              
                for (byte i = 0; i < mfrc522.uid.size; i++)                     // For-loop to read the TAG-adress 
                   {
                      Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
                      Serial.print(mfrc522.uid.uidByte[i], HEX);
                      content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
                      content.concat(String(mfrc522.uid.uidByte[i], HEX));
                   }
               Serial.println();                              
               content.toUpperCase(); 
               String toSend = "";
               String toSend2 = "";
               unsigned int studentID = 0;
               short int index;             
               index = chack_UID(content.substring(1) , validList);
               Serial.println(index);
                              
// ********* Register attendance ******************

               if (index == -2)                                                                     // If the attendance list is missing.
                 {
                  Serial.println(F("The list is missing!"));
                  text1="The list is missing";
                 }              
              
              else if(index == -1)                                                                   // If the UID not in the valid list.
                {
                  Serial.println(F("Access Denied!"));
                  text1="Access Denied!";
                  digitalWrite(7, HIGH);
                }
                              
              else if(index >= 0)                                                                    // Register attendance.
                {
                  
              if(validList[index].get_attendanceStatus()== 1)                                        // If the student is already registered.
                {
                  short int index2;
                  Serial.println(F("You are already registered in the attendance list"));
                  text1="Already registered!";
                  text2="";
                  start_LCD(text1,text2);
                  digitalWrite(6, HIGH);                  
                  studentID = validList[index].get_SID();                                            // Call search SID function.
                  index2 = chack_SID(studentID,studentAttendanceList);                  
                  text1 = "Registered att:";
                  text2 =  String(studentAttendanceList[index2].get_logInHour())+ ":" + String (studentAttendanceList[index2].get_logInMinute());
                  Serial.println(text2);                             
                }
                                
                if( validList[index].get_attendanceStatus() == 0)                                    // Register the student in the attendance list.
                {                 
                  newAttendance = new Attendance;
                  studentID = validList[index].get_SID();
                  newAttendance->set_newAttendance(studentID,now.hour(),now.minute());
                  studentAttendanceList[counter]=*newAttendance;
                  counter++;                  
                  validList[index].set_attendanceStatus(1);                                          // Change attendance status.                  
                  Serial.println(F("registration successfully completed!"));
                  Serial.println(studentID);
                  text1="Successfully!";
                  text2="You are welcome!";
                  Serial.println(text2);
                                    
// ********* Start code Connect with another Arduino *********
                  time_now = millis();      
                  toSend = String(studentID);                                                       // Storing SID to send as a data-package.
                  toSend2 = String(now.hour());                                                     // Storing check-in hour.
                  toSend2+= ":";
                  toSend2+= String(now.minute());                                                   // Storing check-in minute.
                  toSend2+=":00";
                  Serial.println(toSend2);
                  Wire.beginTransmission(wifi);                                                      // Transmit to device #2.
                  Wire.write(1);                                                                     // Sends x. 
                  Wire.write(toSend.c_str());
                  Wire.endTransmission();                                                            // Stop transmitting.                                                                          
                  Wire.beginTransmission(wifi);                                                      // Transmit to device #2.
                  Wire.write(2);                                                                     // Sends x. 
                  Wire.write(toSend2.c_str());
                  Wire.endTransmission();                                                            // Stop transmitting.                  
                  if (counter > 5)                                                                   // Send to ventilaton
                    {
                      Wire.beginTransmission(servo);                                                 // Transmit to device #3.
                      Wire.write(counter);                                                           // Sends x (number of students for the servo to open). 
                      Wire.endTransmission();                                                        // Stop transmitting.
                    }             
                  digitalWrite(5, HIGH);
                  time_now = millis();
                }                
              }                      
               start_LCD(text1,text2);                                                               // Call LCD function.
               time_now = millis();                                                       
         }
     }     
 }  

    else
    {
    delay(500);
    }

    if ( programStartMonth==now.month() && programStartDay==now.day())
        {
            if ( now.hour()<= programStartHour || now.hour() >= scheduleEndtHour )
              { 
                Serial.println(F("Second Statment")); 
                if (now.minute() < programStartMinute  || now.minute()> scheduleEndMinute  )  
                {                         
                    if(stopServo==false)
                    {
                      Serial.println(F("The Servo is in normal close mode "));
                      Wire.beginTransmission(servo);                                              // Transmit to device #3.
                      Wire.write(0);                                                              // Sends x. 
                      Wire.endTransmission();                                                     // Stop transmitting.                        
                      stopServo=true;                                                             // Closing servo/"ventilation" when class is over.
                      startServo=false;
                      digitalWrite(7, HIGH);
                    }
                }
             }              
        }
} 

short int chack_UID(String uid,ValidList valid[])                                                  // Check if the UID is the valid list and return the index number.
{
    for(int i = 0 ; i < 10; i++)
    {
      if( valid[i].get_UID() == uid)
      {
        return i;
      }
      if (i == 9)
       {
        return -1;
       }
    }    
}
short int chack_SID(unsigned int sid,Attendance myAttendanceList[])                                 // Serch SID.
{
    for(int i = 0 ; i < 10; i++)
    {
      if( myAttendanceList[i].get_SID() == sid)
      {
        return i;
      }
      if (i == 9)
       {
        return -1;
       }
    }    
}

void start_LCD(String text1,String text2)                                                            // Function to print text to LCD.
{  
   lcd.backlight();
   lcd.setCursor(0,0);                                                                               // Set the cursor at line 1, position 3. The second parameter '0' represent line 1.
   lcd.print(text1);
   lcd.setCursor(0,1);
   lcd.print(text2);                                                                                 // Set the cursor at line 2, position 2.The second parameter '1' represent line 2.
   delay(500);
   lcd.init();                                                                                       // Initiate LCD.
   digitalWrite(5, LOW);
   digitalWrite(6, LOW);
   digitalWrite(7, LOW);
}

void get_sensorsvalues()                                                                             // Function to read sensorvalues from slave-arduino.
{
    Serial.println(F("S"));
    Wire.requestFrom(servo,13);
    while (Wire.available())         
       { 
          char c = Wire.read();                    
          recivedSensorsValues+=c;                     
       }                             
   Serial.println(recivedSensorsValues);      
}
void send_sensorsvalues()                                                                             // Function to pass the data forward to the other slave-arduino for uploading to database.
{
  Wire.beginTransmission(wifi);                                                                       // Transmit to device #2.
  Wire.write(3);                                                                                      // Sends x. 
  Wire.write(recivedSensorsValues.c_str());
  Wire.endTransmission();                                                                             // Stop transmitting.
  recivedSensorsValues = "";
}

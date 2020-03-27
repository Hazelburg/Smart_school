/*
 This is one of the Slave-arduinos in our system. The hardware used for this is: 1x Arduino UNO, 1x ESP8266. PIN 8 and 9 is used for the ESP8266-module, PIN A5 and A4 is used for the I2C connection.
 The purpose of this arduino is to recive and upload data to the database.
 Master-arduino will send SID,Date,Temp, Humidity and Quality. These will be uploaded to the database using two different querys inorder to store data in different tables.
 We use five different librarys for this Arduino (MySQL_connection and Cursor, WIFIesp, SoftwareSerial and Wire).
 We have had alot of touble with memory-usage in this sketch, hence you will find comments like this: "//Not using to save memory.". In order to make it work with two different
 querys we had to release more memory, simply by not using alot of the serial-monitor printouts.
*/

#include <MySQL_Connection.h>                                                       // Library for MySQL connection functions.
#include <MySQL_Cursor.h>                                                           // Library for MySQL functions.
#include "WiFiEsp.h"                                                                // Library for WIFI connection.
#include "SoftwareSerial.h"                                                         // Library used to allow serial communication on other digital pins. since we use ESP8266 in PIN 8 and 9.
#include <Wire.h>                                                                   // Library used for I2C-protocol.

SoftwareSerial Serial1(8, 9);                                                       // Declaration of PINs used by WIFI-module Transmit and Recive.
WiFiEspClient client;                                                               // Creates a WIFI-client.
MySQL_Connection conn((Client *)&client);                                           // Creates a MySQL connection-client.
char ssid[] = "COMHEM66";                                                            // Network name.              
char pass[] = "";                                                           // Network password.
IPAddress server_addr(192,168,0,43);                                               // Your local IV4-adress (since our database is running locally). 
char user[] = "user2";                                                              // Database-username.                    
char password[] = "user2";                                                          // Database-password.

byte x = 0;                                                                         // Declaration of "x" used by "trigger-function" to determine wich statement to run.

/* ********* Strings to store recived input and to uplod to database. ********* */
String receivedSID = "";
String receivedDate = "";
String receivedTemp = "";
//String receivedHumi = "";                                                         // Not using to save memory.
String receivedQuality = ""; 

bool get_SID = false;                                                               // Boolean to control the uploding-process to the database.
bool get_date = false;                                                              // Boolean to control the uploding-process to the database.
bool get_sensorsValues = false;                                                     // Boolean to control the uploding-process to the database.
byte counter = 0;                                                                   // A counter used to sort out temp,hum,quality-input from master (since input are coming as a single string)

//         ************* Setup ***************

void setup()
  {     
  Serial1.begin(9600);     
  WiFi.init(&Serial1);    
  Serial.begin(115200);
  //Serial.println(F("Initialising connection"));                                    // Not using to save memory.
  //Serial.print(F("Connecting to "));                                               // Not using to save memory.
  //Serial.println(ssid);                                                            // Not using to save memory.
  WiFi.begin(ssid, pass);                                                            // The WIFI-module starts trying to connect to the network.
  
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    //Serial.print(F("."));                                                          // Not using to save memory.
  }
  //Serial.println(F(""));                                                           // Not using to save memory.
  Serial.println(F("C"));                                                            // Printout to let us know we are connected to WIFI.
  //Serial.print(F("Assigned IP: "));                                                // Not using to save memory.
  //Serial.println(WiFi.localIP());                                                  // Not using to save memory.
    
  while (conn.connect(server_addr, 3306, user, password) != true)                    // Starts trying to connect to database.
  {
    delay(200);
    //Serial.print (F( "." ));                                                       // Not using to save memory.
  }
  
  //Serial.println(F(""));                                                           // Not using to save memory.
  Serial.println(F("DB"));                                                           // Let us know we are connected to database.   
  Wire.begin(2);                                                                     // Start the I2C connection as slave with the adress 2.
  Wire.onReceive(receiveEvent);                                                      // A trigger-function when reciveing input from Master.  
}

/*****************Recive from Master*******************
This function is called when this Arduino recive an input from Master. The function will run different statements due to the value of the first input ("X").
If recived x as 1 it will read the incoming data as SID and store it in the string "recivedSID".
If recived x as 2 it will read the incoming data as Date and store it in the string "recivedDate".
If recived x as 3 it will read the incoming data as Temperature, Humidity and Quality and store it in the strings "receivedTemp", "receivedHumi" and "receivedQuality". 
*/
void receiveEvent(byte bytes) 
{
  x = Wire.read();                                                                    // Reads X from Master to determine which statement to run.
  if (x == 1)
     {
      for (int i=0; i<5; i++)                                                         // SID consists of five numbers, for example: "19001", this loop counts to five and if the input are numbers it builds the string. 
        { 
          char c = Wire.read();                                                       // Start reading the incoming data-package.
          if ( c >= 48 && c <= 57 )                                                   // Since input is a string we are using ASCII-table to control that the incoming input are numbers. Input is a string.
            {
              receivedSID+=c;                                                         // Builds the string by adding the char c's together
            }
        }    
          get_SID=true;                                                               // Sets the boolean to "True" inorder to upload it to the database.              
     }
                        
  if (x == 2)
    {
        while (Wire.available())      
        {  
            char c = Wire.read();                                                      // Start reading the incoming data-package.
            if ( c >= 48 && c <= 58 )                                                  // Since input is a string we are using ASCII-table to control that the incoming input are numbers. Input is a string.
            {
              receivedDate+=c;                                                         // Builds the string by adding the char c's together.
            }        
          }    
       get_date = true;                                                                // Sets the boolean to "True" inorder to uplod it to the database.         
    }

  if (x == 3)
    {   
        while (Wire.available())      
          {  
             char c = Wire.read();                                                     // Start reading the incoming data-package.
             counter++;
             if ( (( c == 46)||(c >= 48 && c <= 57)) && (counter <= 5) )               // Since input is a string we are using ASCII-table to control that the incoming input are numbers. We also use the counter to sort out temp,hum and quality, since we recive it as a single string. The first 5 values is temp, value 0 to 5 is temp etc.
               {
                 receivedTemp+= c;                                                     // Builds a string from the first five values by adding the char c's together.
               }
               /*
             if ((( c == 46)||(c >= 48 && c <= 57)) && (counter > 5 && counter <= 10) )
             {
                receivedHumi+= c;                                                      // Builds a second string from the values between 5 and 10 by adding the char c's together.       
             }
             */
             if ( (c >= 48 && c <= 57) && (counter > 10 && counter <= 13) )
             {
                receivedQuality+= c;                                                   // Builds a third string from the values between 10 and 13 by adding the char c's together.       
             }
          }        
       get_sensorsValues = true;                                                       // Sets the boolean to "True" inorder to uplod it to the database.
       counter = 0;                                                                    // Resets the counter for the next incoming data-package.     
    }                         
}
  
void loop() 
{  
  //Serial.println(F("R"));                                                            // Not using to save memory.          
  MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);                                     // Creates a MySQL_Cursor-object. 
  if(get_SID == true && get_date == true)                                              // Statement wich run if both booleans is set to "true". 
   {
      //Serial.println(F("I do it!"));                                                 // Not using to save memory.       
       String query = "INSERT INTO `smart_school`.`attendance` (`Date`, `logInTime`, `SID`) VALUES (CURDATE(),  '" + String(receivedDate) + "' ,"+  String(receivedSID)+ ")";                   // first query, builds a string to upload Date and SID to the table "xxx" in the database. (SID is used to link a student to an ID wich is used by a tag, inorder to store attendence. Date is storing time when the student tagged).          
       char *q = query.c_str();                                                        // Converts the "String query" to a char-array.
       cur_mem->execute(q);                                                            // cur_mem executes/runs the char-array.
       //Serial.println(F("rID"));                                                     // Not using to save memory.
       //Serial.println(receivedDate);                                                 // Not using to save memory.
       receivedSID="";                                                                 // Resets the string for the next incoming data-package. 
       receivedDate="";                                                                // Resets the string for the next incoming data-package.
       get_SID = false;                                                                // Sets boolean as "false" again. In order to be able to re-run the code when next data-package is coming. 
       get_date = false;                                                               // Sets boolean as "false" again. In order to be able to re-run the code when next data-package is coming.              
   }
   
  if (get_sensorsValues == true )                                                      // Statement wich run if boolean is set to "true".
  {
    String query = "INSERT INTO `smart_school`.`sensorvalues` (`Datetime`, `Quality`, `Temp`) VALUES (now(), " + String(receivedQuality) + ", " + String(receivedTemp) + ")";             // Second query, builds a string to upload sensorvalues to the table "sensorvalues" in the database.
    char *q = query.c_str();                                                           // Converts the "String query" to a char-array. 
    cur_mem->execute(q);                                                               // cur_mem executes/runs the char-array.
    //Serial.println(receivedTemp);                                                    // Not using to save memory.
    //Serial.println(receivedHumi);                                                    // Not using to save memory.
    //Serial.println(receivedQuality);                                                 // Not using to save memory.
    receivedTemp = "";                                                                 // Resets the string for the next incoming data-package.
    //receivedHumi = "";                                                               // Resets the string for the next incoming data-package.
    receivedQuality = "";                                                              // Resets the string for the next incoming data-package.
    get_sensorsValues = false;                                                         // Sets boolean as "false" again. In order to be able to re-run the code when next data-package is coming.        
  }
           
    delete cur_mem;                                                                    // Deletes the cur_mem-object from heap, since it is declared with "new".          
    //Serial.println(F("delete!"));                                                    // Not using to save memory.  
    delay(500);  
}

#include <ArduinoSTL.h>

class Attendance
{
protected:
	unsigned int SID;
	uint8_t logInHour;
	uint8_t logInMinute;

  //uint8_t logInMonth;
  //uint8_t logInDay;

public:

	//void set_newAttendance(String ID, uint8_t Month, uint8_t Day, uint8_t Hour, uint8_t Minute)
	void set_newAttendance(unsigned int sID,uint8_t Hour,uint8_t Minute )
	{
		SID = sID;
		logInHour = Hour;
		logInMinute=Minute;

		//logInMonth = Month;
		//logInDay = Day;
	}
	
	unsigned int get_SID() {return SID;}
  uint8_t get_logInHour() { return logInHour; }
	uint8_t get_logInMinute() { return logInMinute; }

	//uint8_t get_logInMonth() { return logInMonth; }
	//uint8_t get_logInDay() { return logInDay; }


};



/*
	void registerAttendance(Attendance* attendance)
	{
		presenceList.push_back(attendance);
	//Serial.println(F("Registered OK"));
	}
	*/

	/*
	void checkOutAttendanceList()
	{
		for (size_t i = 0; i < presenceList.size(); i++)
		{
			Serial.print(F("Number "));
			Serial.println(i);

			Serial.print(F("ID "));
			Serial.println(presenceList[i]->get_UID());

	  //Serial.print(F("    Date: "));
	  //Serial.print(presenceList[i].get_logInDay());
	  //Serial.print(F("/"));
	  //Serial.print(presenceList[i].get_logInMonth());

	  Serial.print(F("Time: "));
		Serial.print(presenceList[i]->get_logInHour());

	  Serial.print(F(":"));
	  Serial.println(presenceList[i]->get_logInMinute());

		}

	}
	*/

	/*
	short int get_theNumberOfAttendance() { return presenceList.size(); }

	Attendance sendAttendanceList()
	{
		for (size_t i = 0; i < presenceList.size(); i++)
		{
			return *presenceList[i];
		}
	}

	*/

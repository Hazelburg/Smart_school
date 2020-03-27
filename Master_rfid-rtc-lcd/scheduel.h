#include <ArduinoSTL.h>


class Schedule
{
protected:
	
	uint8_t scheduleStartMonth;
	uint8_t scheduleStartDay;
	uint8_t scheduleStartHour;
	uint8_t scheduleStartMinute;

	uint8_t scheduleEndtHour;
	uint8_t scheduleEndMinute;
	

public:
	String courseName;

	void set_courseName(String course) { courseName = course; }
	String get_courseName() { return courseName; }

	void set_scheduleStartDate(uint8_t month, uint8_t day)
	{ 
		scheduleStartMonth = month; 
		scheduleStartDay = day;
	}

	void set_schedulestartTime(uint16_t StHour,uint16_t StMinute, uint16_t EnHour, uint16_t EnMinute)
	{
		scheduleStartHour = StHour;
		scheduleStartMinute = StMinute;

		scheduleEndtHour = EnHour;
		scheduleEndMinute = EnMinute;
	}

	uint8_t get_StartMonthe() { return scheduleStartMonth; }
	uint8_t get_StartDay() { return scheduleStartDay; }
	uint8_t get_StartHour() { return scheduleStartHour; }
	uint8_t get_StartMinute() { return scheduleStartMinute; }

	uint8_t get_EndtHour() { return scheduleEndtHour; }
	uint8_t get_EndMinute() { return scheduleEndMinute; }


	
};

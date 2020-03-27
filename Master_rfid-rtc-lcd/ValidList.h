#include <ArduinoSTL.h>


class ValidList
{
protected:
	String UID;
	unsigned int SID;
	byte attendanceStatus;

public:
	/*
	void set_UID(String uid) { UID = uid; }
	void set_SID(unsigned int sid) { SID = sid; }
	void set_attendanceStatus(byte attendStatu) { attendanceStatus = attendStatu; }
	*/
  
  void set_attendanceStatus(byte attendStatu) { attendanceStatus = attendStatu; }
  
	void set_validPerson(String uid, unsigned int sid, byte attendStatu)
	{
		UID = uid;
		SID = sid;
		attendanceStatus = attendStatu;
	}

	String get_UID() { return UID; }
	unsigned int get_SID(){ return SID; }
	byte get_attendanceStatus() { return attendanceStatus; }


};

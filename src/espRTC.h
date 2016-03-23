/**************************************************************************
= NTP RTC Library for ESP8266 =

This library will use a combination of NTP and a virtual internal RTC to keep
time. It will sync up with NTP every 12 hours and use the internal timer to 
keep accurate time in between updates.

Written by: Richard Wardlow @ Circuits for Fun, LLC
GNU GPL, include above text in redistribution
***************************************************************************/

#include <Ticker.h>
#include <WiFiUdp.h>

#define LEAP_YEAR(Y)     ( ((1970+Y)>0) && !((1970+Y)%4) && ( ((1970+Y)%100) || !((1970+Y)%400) ) )
#define NTP_PACKET_SIZE  (0x30)
static  const uint8_t monthDays[] = { 31,28,31,30,31,30,31,31,30,31,30,31 }; // API starts months from 1, this array starts from 0

// Struct for date/time variables
struct _espTime{
    int day;
    int month;
    int year;
    int hour;
    int minute;
    int second;
    int updateCounter;
};

////////////////////////////////////////
// espRTC Class
////////////////////////////////////////
class espRTC
{
	public:
		espRTC(const String& ntpServer, int gmtOffset, bool dstFlag);
		espRTC(const char* ntpServer, int gmtOffset, bool dstFlag);
        	~espRTC();
        	unsigned long sendNTPpacket(IPAddress& address);
	        void breakTime(unsigned long timeInput);
	        void ParseTimePacket();
		void begin();
	        void update();
	        int getDay();
	        int getMonth();
	        int getYear();
	        int getSecond();
	        int getMinute();
	        int getHour();
	private:
        	_espTime _rtcTime;
		uint8_t _RTCpacketBuffer[NTP_PACKET_SIZE];
		WiFiUDP _RTCudp;
        	Ticker _updateRTC;
		const char* _ntpServer;
		int _gmtOffset;
		bool _dstFlag;
};

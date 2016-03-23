/**************************************************************************
= NTP RTC Library for ESP8266 =

This library will use a combination of NTP and a virtual internal RTC to keep
time. It will sync up with NTP every 12 hours and use the internal timer to 
keep accurate time in between updates.

Written by: Richard Wardlow @ Circuits for Fun, LLC
GNU GPL, include above text in redistribution
***************************************************************************/
#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <WiFiUdp.h>
#include "espRTC.h"

////////////////////////////////////////
// espRTC Global Functions
////////////////////////////////////////
void _espRTCUpdate(_espTime *_time) // Timer callback for internal update per second
{
    _time->second += 1;
    if(_time->second == 60)
    {
        _time->second = 0;
        _time->minute += 1;
    }
    if(_time->minute == 60)
    {
        _time->minute = 0;
        _time->hour += 1;
        _time->updateCounter += 1;
    }
    if(_time->hour == 24)
    {
        _time->hour = 0;
    }
}

////////////////////////////////////////
// espRTC Class Methods
////////////////////////////////////////
espRTC::espRTC(const String& ntpServer, int gmtOffset, bool dstFlag)
{
	_ntpServer = ntpServer.c_str();
	_gmtOffset = gmtOffset;
	_dstFlag = dstFlag;
	_rtcTime.day = 0;
	_rtcTime.month = 0;
	_rtcTime.year = 0;
	_rtcTime.hour = 0;
	_rtcTime.minute = 0;
	_rtcTime.second = 0;
	_rtcTime.updateCounter = 255; // Init Value let's update know it needs to fetch NTP time
}

espRTC::espRTC(const char* ntpServer, int gmtOffset, bool dstFlag)
{
	_ntpServer = ntpServer;
	_gmtOffset = gmtOffset;
	_dstFlag = dstFlag;
}

espRTC::~espRTC()
{
    _updateRTC.detach();
}

void espRTC::begin()
{
    _RTCudp.begin(2390);
    delay(500);
    this->update();
    delay(500);
    _updateRTC.attach(1, _espRTCUpdate, &_rtcTime);
    
}

void espRTC::update()
{
    if(_rtcTime.updateCounter == 12 || _rtcTime.updateCounter == 255 || _rtcTime.year == 0)
    {
        IPAddress timeServerIP;
        WiFi.hostByName(_ntpServer, timeServerIP);
        sendNTPpacket(timeServerIP); // send an NTP packet to a time server
        _rtcTime.updateCounter = 0;
        delay(200);
    }
    int cb = _RTCudp.parsePacket();
    if (cb) {
        this->ParseTimePacket();
    }
}

int espRTC::getDay()
{
    return _rtcTime.day;
}

int espRTC::getMonth()
{
    return _rtcTime.month;
}

int espRTC::getYear()
{
    return _rtcTime.year;
}

int espRTC::getSecond()
{
    return _rtcTime.second;
}

int espRTC::getMinute()
{
    return _rtcTime.minute;
}

int espRTC::getHour()
{
    return _rtcTime.hour;
}

void espRTC::ParseTimePacket(){
    // We've received a packet, read the data from it
    _RTCudp.read(_RTCpacketBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, extract the two words:

    unsigned long highWord = word(_RTCpacketBuffer[40], _RTCpacketBuffer[41]);
    unsigned long lowWord = word(_RTCpacketBuffer[42], _RTCpacketBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;

    // now convert NTP time into everyday time:
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    unsigned long epoch = secsSince1900 - seventyYears;

    // Break epoch time into Date / Time components
    breakTime(epoch);
}

unsigned long espRTC::sendNTPpacket(IPAddress& address)
{
  // set all bytes in the buffer to 0
  memset(_RTCpacketBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  _RTCpacketBuffer[0] = 0b11100011;   // LI, Version, Mode
  _RTCpacketBuffer[1] = 0;     // Stratum, or type of clock
  _RTCpacketBuffer[2] = 6;     // Polling Interval
  _RTCpacketBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  _RTCpacketBuffer[12]  = 49;
  _RTCpacketBuffer[13]  = 0x4E;
  _RTCpacketBuffer[14]  = 49;
  _RTCpacketBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  _RTCudp.beginPacket(address, 123); //NTP requests are to port 123
  _RTCudp.write(_RTCpacketBuffer, NTP_PACKET_SIZE);
  _RTCudp.endPacket();
}

void espRTC::breakTime(unsigned long timeInput){
// break the given time_t into time components
// this is a more compact version of the C library localtime function
// note that year is offset from 1970 !!!
  uint8_t year;
  uint8_t month, monthLength;
  uint32_t time;
  unsigned long days;
  uint8_t Second;
  uint8_t Wday;
  uint8_t Month;
  uint8_t Day;
  uint16_t Year;
  uint8_t Minute;
  uint8_t Hour;
  bool inDST = false;

  // Adjust for GMT Offset
  long timeOffset = ((_gmtOffset * 60) * 60);
  timeInput += timeOffset; // Time in Seconds 
  
  time = (uint32_t)timeInput;
  Second = time % 60;
  time /= 60; // now it is minutes
  Minute = time % 60;
  time /= 60; // now it is hours
  Hour = time % 24;
  time /= 24; // now it is days
  Wday = ((time + 4) % 7) + 1;  // Sunday is day 1 
  
  year = 0;  
  days = 0;
  while((unsigned)(days += (LEAP_YEAR(year) ? 366 : 365)) <= time) {
    year++;
  }
  Year = 2000 + (year - 30); // Convert Year to Full Year Number
  
  days -= LEAP_YEAR(year) ? 366 : 365;
  time  -= days; // now it is days in this year, starting at 0
  
  days=0;
  month=0;
  monthLength=0;
  for (month=0; month<12; month++) {
    if (month==1) { // february
      if (LEAP_YEAR(year)) {
        monthLength=29;
      } else {
        monthLength=28;
      }
    } else {
      monthLength = monthDays[month];
    }
    
    if (time >= monthLength) {
      time -= monthLength;
    } else {
        break;
    }
  }
  Month = month + 1;  // jan is month 1  
  Day = time + 1;     // day of month
  
  if (_dstFlag == true)
  {
      if(Month >= 3 && Month <= 11)
      {
        inDST = true;
        if(Month == 3 && Day < 13)
            inDST = false;
        if(Month == 11 && Day > 6)
            inDST = false;
      }
      if(inDST == true)
      {
          // Only recalculating time for DST
          // (this could present a problem at midnight, it will still be the next day)
          // Ideally we would recalculate the month, day and year as well
          timeInput += 3600; // 1 hour offset in seconds for DST 
          time = (uint32_t)timeInput;
          Second = time % 60;
          time /= 60; // now it is minutes
          Minute = time % 60;
          time /= 60; // now it is hours
          Hour = time % 24;
      }
  }
  
  // Assign class variables
  _rtcTime.day = Day;
  _rtcTime.month = Month;
  _rtcTime.year = Year;
  _rtcTime.hour = Hour;
  _rtcTime.minute = Minute;
  _rtcTime.second = Second;
  
}





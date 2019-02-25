#include "StdAfx.h"
#include "datetime.h"
#include "Common.h"

CDateTime::CDateTime(void)
{
	year	= 0;
	month = 0;
	day		= 0;
	hour	= 0;
	minute= 0;
	second= 0;
}

CDateTime::CDateTime(const CDateTime &t2){
	year	= t2.year;
	month = t2.month;
	day		= t2.day;
	hour	= t2.hour;
	minute= t2.minute;
	second= t2.second;
}

CDateTime::CDateTime(int y, int mo, int d, int h, int mi, int s){
	year		= y;
	month		= mo;
	day			= d;
	hour		= h;
	minute	= mi;
	second	= s;
}

CDateTime::~CDateTime(void)
{
}

int CDateTime::operator <(const CDateTime &t2) const{
	if(this->year < t2.year)
		return 1;
	if(this->year > t2.year)
		return 0;
	if(this->month < t2.month)
		return 1;
	if(this->month > t2.month)
		return 0;
	if(this->day < t2.day)
		return 1;
	if(day > t2.day)
		return 0;
  if(this->hour < t2.hour)
    return 1;
  if(this->hour > t2.hour)
    return 0;
	if(this->minute < t2.minute)
    return 1;
  if(this->minute > t2.minute)
    return 0;
	if(this->second < t2.second)
    return 1;
  if(this->second > t2.second)
    return 0;

	// equal
  return 0;
}

bool CDateTime::operator==(const CDateTime& t2) const{
	if(this->second != t2.second)
		return false;
	if(this->minute != t2.minute)
		return false;
	if(this->hour != t2.hour)
		return false;
	if(this->day != t2.day)
		return false;
	if(this->month != t2.month)
		return false;
	if(this->year != t2.year)
		return false;
	return true;
}

CDateTime	& CDateTime::operator=(const CDateTime& t2){
	this->year	= t2.year;
	this->month	= t2.month;
	this->day		= t2.day;
	this->hour	= t2.hour;
	this->minute= t2.minute;
	this->second= t2.second;
	return *this;
}

void CDateTime::SetToNow(){
  struct tm *tim;
  time_t t;
  time(&t);
  tim	=	localtime(&t);

	this->year	= 1900 + tim->tm_year;
	this->month	=	1 + tim->tm_mon;
	this->day		= tim->tm_mday;
	this->hour	= tim->tm_hour;
	this->minute= tim->tm_min;
	this->second= tim->tm_sec;
}

/** Calculates the difference, in seconds, between two times.
		If t2 is later than t1, then the result will be negative. */
double	CDateTime::Difference(const CDateTime &t1, const CDateTime &t2){
	struct tm tid1, tid2;
	tid1.tm_year	= t1.year - 1900;
	tid1.tm_mon		= t1.month - 1;
	tid1.tm_mday	= t1.day;
	tid1.tm_hour	= t1.hour;
	tid1.tm_min		= t1.minute;
	tid1.tm_sec		= t1.second;
	tid1.tm_wday	= 0;
	tid1.tm_yday	= 0;
	tid1.tm_isdst	= 0;

	tid2.tm_year	= t2.year - 1900;
	tid2.tm_mon		= t2.month - 1;
	tid2.tm_mday	= t2.day;
	tid2.tm_hour	= t2.hour;
	tid2.tm_min		= t2.minute;
	tid2.tm_sec		= t2.second;
	tid2.tm_wday	= 0;
	tid2.tm_yday	= 0;
	tid2.tm_isdst	= 0;

	time_t t_1 = mktime(&tid1);
	time_t t_2 = mktime(&tid2);

	return difftime(t_1, t_2);
}

/** Increments the current time with the supplied number of seconds */
void CDateTime::Increment(int secs){
	// calculate the number of seconds since midnight this is...
	long nSecsSinceMidnight = second + 60 * minute + 3600 * hour;
	
	if(secs <= (86400-nSecsSinceMidnight)){
		// this is a small change, only change within the same day
		nSecsSinceMidnight += secs;
		
		this->hour   = nSecsSinceMidnight / 3600;
		this->minute = (nSecsSinceMidnight - 3600 * hour) / 60;
		this->second = (nSecsSinceMidnight % 60);
		
		return;
	}else{
		// go to the next before
		IncrementOneDay();

		while(secs > 86400){
			DecrementOneDay();
			secs -= 86400;
		}

		// we should increment less than 86400 seconds (less than one day)
		nSecsSinceMidnight = secs - (86400 - hour * 3600 - minute * 60 - second);

		this->hour   = nSecsSinceMidnight / 3600;
		this->minute = (nSecsSinceMidnight - 3600 * hour) / 60;
		this->second = (nSecsSinceMidnight % 60);
		
		return;
	}
}

/** Decrements the current time with the supplied number of seconds */
void CDateTime::Decrement(int secs){
	// calculate the number of seconds since midnight this is...
	long nSecsSinceMidnight = second + 60 * minute + 3600 * hour;
	
	if(secs <= nSecsSinceMidnight){
		// this is a small change, only change within the same day
		nSecsSinceMidnight -= secs;
		
		this->hour   = nSecsSinceMidnight / 3600;
		this->minute = (nSecsSinceMidnight - 3600 * hour) / 60;
		this->second = (nSecsSinceMidnight % 60);
		
		return;
	}else{
		// go to the day before
		DecrementOneDay();

		while(secs > 86400){
			DecrementOneDay();
			secs -= 86400;
		}

		// we should decrement less than 86400 seconds (less than one day)
		nSecsSinceMidnight = 86400 - secs;

		this->hour   = nSecsSinceMidnight / 3600;
		this->minute = (nSecsSinceMidnight - 3600 * hour) / 60;
		this->second = (nSecsSinceMidnight % 60);
		
		return;
	}
}

/** Decrements the current time to the same time the day before */
void CDateTime::IncrementOneDay(){
	long daysInThisMonth = Common::DaysInMonth(this->year, this->month);

	if(day < daysInThisMonth){
		day += 1;
		return;
	}else{
		if(this->month < 12){
			day		= Common::DaysInMonth(this->year, this->month);
			month	+= 1;
		}else{
			day		= 01;
			month	= 01;
			year	+= 1;
		}
	}
}

/** Decrements the current time to the same time the day before */
void CDateTime::DecrementOneDay(){
	if(day > 1){
		day -= 1;
		return;
	}else{
		if(this->month > 1){
			day		= Common::DaysInMonth(this->year, this->month - 1);
			month	-= 1;
		}else{
			day		= 31;
			month	= 12;
			year	-= 1;
		}
	}
}

#ifndef TIME_HPP
#define TIME_HPP
// Copyright (c) 2015, <Abdallah Aly> <aaly90@gmail.com>
//
// Part of Mission14 programming language
//
// See file "license" for license

#include <time.h>       /* time_t, struct tm, difftime, time, mktime */
#include <iostream>
#include <string>

using namespace std;

typedef struct tm;
typedef time_t time_t;

class Time;

class Time
{
	public:
		Time();
		Time (int SecondsSinceEpoch);
		Time (int year, int month, int day, int hour, int minute, int second, bool useLocalTime);
		//Time (Time& other);
		~Time();
 	
		int 	useLocalTime(bool);
		int		useGMTTime(bool);
		int 	getYear();
		int 	getMonth();
		string 	getMonthName(bool threeLetterVersion);
		int 	getDayOfMonth();
		int 	getDayOfWeek();
		int 	getDayOfYear();
		string 	getWeekdayName(bool threeLetterVersion);
		int 	getHours();
		//bool 	isAfternoon();
		int 	getHoursInAmPmFormat();
		int 	getMinutes();
		int 	getSeconds();
		int 	getMilliSeconds();
		bool 	isDaylightSavingTime();
		//String 	getTimeZone () const noexcept
		//Returns a 3-character string to indicate the local timezone. More...
		string 	toString (bool, bool, bool, bool);
		string 	toFormattedString (const string &format);
		//Time	Substract(Time&);
		int	Substract(Time&);
		bool	isLaterThan(Time&);
		//Time & 	operator+= (RelativeTime delta) noexcept
		//Time & 	operator-= (RelativeTime delta) noexcept
		//bool		setSystemTimeToThisTime () const 	
		
		int				getSecondsSinceEpoch();
		static Time		getCurrentTime();
		static string 	getWeekdayName (int dayNumber, bool threeLetterVersion);
		static string 	getMonthName (int monthNumber, bool threeLetterVersion);
		//static int	 	currentTimeMillis();
		int 	updateTime();
		time_t	getInternalTime();
			
 	private:
		struct tm timeTM;
		time_t	timeT;
		
		bool	useLocalTimeCond;
};

#endif

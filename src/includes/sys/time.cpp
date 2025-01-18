// Copyright (c) 2010, <Abdallah Aly> <aaly90@gmail.com>
//
// Part of Mission14 programming language
//
// See file "license" for license
#include "time.hpp"

Time::Time() { timeTM = {0}; };

Time::Time(int SecondsSinceEpoch) {
  timeT = SecondsSinceEpoch;
  updateTime();
};

Time::Time(int year, int month, int day, int hour, int minute, int second = 0,
           bool useLocalTime = true) {
  timeTM.tm_sec = second;
  timeTM.tm_min = minute;
  timeTM.tm_hour = hour;
  timeTM.tm_mday = day;
  timeTM.tm_mon = month;
  timeTM.tm_year = year;
  Time(mktime(&timeTM));
};

// Time (const Time& other)
//{
//	Time(other.getSecondsSinceEpoch());
// };

Time::~Time() {

};

int Time::updateTime() {
  time(&timeT);

  if (useLocalTimeCond) {
    timeTM = *localtime(&timeT);
  } else {
    timeTM = *gmtime(&timeT);
  }

  return 0;
}

int Time::getYear() {
  updateTime();
  return timeTM.tm_year + 1900;
};

int Time::getMonth() {
  updateTime();
  return timeTM.tm_mon + 1;
};

int Time::getMinutes() {
  updateTime();
  return timeTM.tm_min;
};

int Time::getSeconds() {
  updateTime();
  // return timeTM.tm_sec/10;
  return timeTM.tm_sec;
};

int Time::getMilliSeconds() {
  updateTime();
  return timeTM.tm_sec % 100;
};

int Time::getHours() {
  updateTime();
  return timeTM.tm_hour;
}

int Time::getDayOfMonth() {
  updateTime();
  return timeTM.tm_mday;
};

int Time::getDayOfWeek() {
  updateTime();
  return timeTM.tm_wday + 1;
};

int Time::getDayOfYear() {
  updateTime();
  return timeTM.tm_yday + 1;
};

bool Time::isDaylightSavingTime() {
  updateTime();
  return timeTM.tm_isdst;
};

int Time::useLocalTime(bool cond) {
  useLocalTimeCond = cond;
  return cond;
};

int Time::useGMTTime(bool cond) {
  useLocalTimeCond = !cond;
  return cond;
};

int Time::getSecondsSinceEpoch() {
  updateTime();
  return timeT;
}
Time Time::getCurrentTime() { return Time(time(0)); };

string Time::getWeekdayName(int dayNumber, bool threeLetterVersion) {
  return "";
};

string Time::getMonthName(int monthNumber, bool threeLetterVersion) {
  return "";
};

string Time::toString(bool includeDate, bool includeTime,
                      bool includeSeconds = true, bool use24HourClock = false) {
  string format;

  if (includeDate) {
    format += "%d/%m/%Y";
  }

  if (includeTime) {
    if (use24HourClock) {
      format += " %H:%M";
    } else {
      format += " %l:%M";
    }
  }

  if (includeSeconds) {
    format += ":%S";
  }

  if (includeTime && !use24HourClock) {
    format += " %p";
  }
  return toFormattedString(format);
};

string Time::toFormattedString(const string &format) {
  char buffer[80];
  strftime(buffer, 80, format.c_str(), &timeTM);
  return string(buffer);
};

time_t Time::getInternalTime() { return timeT; }

/*Time Time::Substract(Time& t)
{
        return Time(difftime(timeT, t.getInternalTime()));
};*/

int Time::Substract(Time &t) { return difftime(timeT, t.getInternalTime()); };

bool Time::isLaterThan(Time &t) {
  return timeT > t.getSecondsSinceEpoch() ? true : false;
};

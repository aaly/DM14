// Copyright (c) 2015, <Abdallah Aly> <aaly90[@]gmail.com>
//
// Part of Distributed Mission14 programming language
//
// See file "license" for license

#include "string.hpp"

int	str_size(const string& str)
{
	return str.size();
};
char str_at(const string& str, const unsigned int& pos)
{
	if(pos > str.size())
	{
		return -1;
	}
	
	return str.at(pos);
};

bool str_isEmpty(const string& str)
{
	return str.size() ? true : false;
};


string numberToStr(int number)
{
	stringstream SS;
	SS << number;
	return SS.str();
};

string floatToStr(float number)
{
	stringstream SS;
	SS << number;
	return SS.str();
};

int strToNumber(const string& str)
{
	stringstream SS;
	SS << str;
	int num = 0;
	SS >> num;
	return num;
};


float strToFloat(const string& str)
{
	stringstream SS;
	SS << str;
	float num = 0;
	SS >> num;
	return num;
};






long getPosition(int order, const string& str, const char& ch)
{
	if (order == 0)
	{
		return 0;
	}
	
	int tmp = 0;
	
	for (unsigned int i =0; i < str.size(); i++)
	{
		if(str.at(i) == ch)
		{
			tmp++;
		}
		
		if(tmp == order)
		{
			return i+1;
		}
	}
	
	return -1;
}

string getToken(const string& str, const char& ch, const int& pos)
{
	long from = getPosition(pos, str, ch);
	
	long to;
	
	
	to  = getPosition(pos+1, str, ch)-getPosition(pos, str, ch)-1;
	
	if(from != -1 && to != -1)
	{
		return str.substr(from, to);
	}
	
	return "";
}


int	tokenCount(const string& str, const char& ch)
{
	unsigned int count = 0;
	for (unsigned int i =0; i < str.size(); i++)
	{
		if(str.at(i) == ch)
		{
			count++;
		}
	}
	
	return count;
}

string	str_sub(const string& str, const int& from, const int& to)
{
	return str.substr(from, to);
}

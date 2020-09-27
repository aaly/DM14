// Copyright (c) 2010, <Abdallah Aly> <l3thal8@gmail.com>
//
// Part of Mission14 programming language
//
// See file "license" for bsd license

#include "common.hpp"

#include "sstream"
#ifdef WIN32
string pathSeperator = "\\";
#elseif define LINUX
string pathSeperator = "/";
#endif



enum Color
{
	Black, 
	DarkRed,
	DarkGreen,
	DarkYellow,
	DarkBlue,
	DarkMagenta,
	DarkCyan,
	Gray,
	
	// Light colors
	DarkGray,
	Red,
	Green,
	Yellow,
	Blue,
	Magenta,
	Cyan,
	White,
	
	// Reset sequence
	Reset
}; // END Color enum

enum LogLevelFlags
{
	LOG_LEVEL_DEBUG, 
	LOG_LEVEL_MESSAGE, 
	LOG_LEVEL_WARNING, 
	LOG_LEVEL_CRITICAL, 
	LOG_LEVEL_SUCCESS, 
	LOG_LEVEL_CUSTOM, 
}; // END LogLevelFlags enum


std::string get_color_code(Color color, bool foreground)
{
		int color_id;

		bool light = false;
				bool reset = false;

				std::string str_;
				std::stringstream strout;
				std::string color_str;

				switch ( color)
				{
					// Dark colors
					case Black:
						color_id = 0;
					break;

					case DarkRed:
						color_id = 1;
					break;

					case DarkGreen:
						color_id = 2;
					break;

					case DarkYellow:
						color_id = 3;
					break;

					case DarkBlue:
						color_id = 4;
					break;

					case DarkMagenta:
						color_id = 5;
					break;

					case DarkCyan:
						color_id = 6;
					break;

					case Gray:
						color_id = 7;
					break;

					// Light colors
					case DarkGray:
						color_id = 0;
						light = true;
					break;

					case Red:
						color_id = 1;
						light = true;
					break;

					case Green:
						color_id = 2;
						light = true;
					break;

					case Yellow:
						color_id = 3;
						light = true;
					break;

					case Blue:
						color_id = 4;
						light = true;
					break;

					case Magenta:
						color_id = 5;
						light = true;
					break;

					case Cyan:
						color_id = 6;
						light = true;
					break;

					case White:
						color_id = 7;
						light = true;
					break;

					// Reset sequence
					case Reset:
						reset = true;
					break;
					default :
						color_id = 7;
						light = true;
						break;
				}

				if ( reset)
				{
					#ifdef OS_WINDOWS
						//
					#elif __linux
						return "\x001b[0m";
					#elif OSX
						//
					#elif __unix
						//
					#else
						//
					#endif
				}

				color_id += (foreground ? 30 : 40) + (light ? 60 : 0);

				strout << color_id;
				str_ = strout.str();

				#ifdef OS_WINDOWS
					color_str = system("color");
				#elif __linux
					color_str = "\x001b[";
					color_str += str_;
					color_str += "m";
				#elif OSX
					//
				#elif __unix
					//
				#else
					//
				#endif
				return color_str;
			} // END get_color_code function

			
			std::string bg_color(Color code)
			{
				return get_color_code(code, false);
			} // END bg_color function

			// -----------------------------------------------------------------

			/**
			 * 
			 */
			std::string fg_color(Color code)
			{
				return get_color_code(code, true);
			} // END fg_color function


int displayDebug(const string& message)
{
	cerr << fg_color(Cyan) << "[Debug]   : " << fg_color(Reset) << fg_color(White) << message << fg_color(Reset) << endl << flush;
	return (0);
};

int displayInfo(const string& fName, const int& line, const int& column, const string& message)
{
	cerr << fg_color(Blue) << "[Info]    : " << fg_color(Reset) << fg_color(Magenta) << "[File] : " << fg_color(Reset) << fName << " line : " << line << " => " << fg_color(White) << message << fg_color(Reset) << endl << flush;
	return (0);
};

int displayInfo(const string& message)
{
	cerr << fg_color(Blue) << "[Info]    : " << fg_color(Reset) << fg_color(White) << message << fg_color(Reset) << endl;
	return (0);
};
	
	
	
int displayWarning(const string& fName, const int& line, const int& column, const string& message)
{
	cerr << fg_color(Yellow) << "[Warning] : " << fg_color(Reset) << fg_color(Magenta) << "[File] : " << fg_color(Reset) << fName << " line : " << line << " => " << fg_color(White) << message << fg_color(Reset) << endl << flush;
	return 0;
};
	
	
		
int displayWarning(const string& message)
{
	cerr << fg_color(Yellow) << "[Warning] : " << fg_color(Reset) << fg_color(White) << message << fg_color(Reset) << endl << flush;
	return 0;
};
	
int displayError(const string& message)
{
	cerr << fg_color(Red) << "[Error]   : " << fg_color(Reset) << fg_color(White) << message << fg_color(Reset) << endl << flush;
	return (0);
};

int displayError(const string& fName, const int& line, const int& column, const string& message, bool Exit)
{
	if (line == -1 )
	{
		cerr << fg_color(Red) << "[Error]   : " << fg_color(Reset) << fg_color(Magenta) << "[File] " << fName << fg_color(Reset) << fg_color(White) << " => " << message << fg_color(Reset) << endl << flush;
	}
	else
	{
		cerr << fg_color(Red) << "[Error]   : " << fg_color(Reset) << fg_color(Magenta) << "[File] " << fName << fg_color(Reset) << fg_color(White) << " line : " << line << ":" << column << " => " << message << fg_color(Reset) << endl << flush;
	}
	
	if (Exit)
	{
		exit (1);
	}
	
	return 0;
};

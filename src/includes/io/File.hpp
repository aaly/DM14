// Copyright (c) 2015, <Abdallah Aly> <aaly90@gmail.com>
//
// Part of DM14 programming language
//
// See file "license" for license

#ifndef DM14_FILE_HPP
#define DM14_FILE_HPP


#include <fstream>
#include <string>

typedef std::fstream FStream;
class File
{
	public:
		File();
		File(const std::string& path);
		~File();
		int open(const std::string& path);
		int close();
		std::string readLine();
		int writeLine(std::string& line);
		bool isOpen();

	private:
		FStream fileStream;
		
};
#endif // DM14_FILE_HPP

           

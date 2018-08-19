// Copyright (c) 2015, <Abdallah Aly> <aaly90[@]gmail.com>
//
// Part of Distributed Mission14 programming language
//
// See file "license" for license

#include "File.hpp"


File::File()
{
	
};

File::File(const string& path)
{
	open(path);
};

File::~File()
{
	
};

int File::open(const string& path)
{
	fileStream.open(path.c_str(), std::fstream::in | std::fstream::out | std::fstream::app);
	return fileStream.is_open();
};

int File::close()
{
	fileStream.close();
	return fileStream.is_open();
};

string File::readLine()
{
	string buffer;
	getline(fileStream, buffer);
	return buffer;
};

int File::writeLine(string& line)
{
	return 0;
};

bool File::isOpen()
{
	return fileStream.is_open();
};



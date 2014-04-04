#pragma once

#include <fstream>
#include <string>
#include <iostream>
#include <strstream>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace std;
class Interpreter
{	
public:
	Interpreter(){}
	static Interpreter& instance();
	bool getLine(std::string line);
	void parse(void);
	void clear()
	{
		query = "";
	}
private:
	std::string query ;
	void execfile(std::string filename);
};


inline Interpreter& Interpreter::instance()
{
	static Interpreter obj;
	return obj;
}
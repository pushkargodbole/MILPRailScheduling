#include <iostream>
#include <string.h>

using namespace std;

string stringstrip(string str)
{
	string out = str;
	string whitespaces (" \t\f\v\n\r");
	size_t white = string::npos;
	white = out.find_first_not_of(whitespaces);
	if (white!=0) out.erase(0,white);
	white = string::npos;
	white = out.find_last_not_of(whitespaces);
	if (white!=string::npos) out.erase(white+1);
	return out;
}

string rm_mph(string str)
{
	string out = str;
	size_t mph;
	mph = str.find("mph");
	if(mph!=string::npos) out.erase(mph);
	return out;
}

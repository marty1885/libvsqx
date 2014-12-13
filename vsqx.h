#include <iostream>
#include <string.h>
#include <stdint.h>
#include <vector>

#include <tinyxml2.h>
using namespace tinyxml2;

class ParameterList
{
}

class VsqxDoc
{
public:
	VsqxDoc();
	~VsqxDoc();

	void setPath(std::string filePath);
	bool isVsqx();
	int load();

	int getError(char* buffer,int bufSize, bool keepMessage);
protected:
	std::string* path;
	std::string errorString;

	void setError(const char* format,...);

};

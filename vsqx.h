#include <iostream>
#include <string.h>
#include <stdint.h>
#include <vector>

#include <tinyxml2.h>


class VVoiceInfo
{
public:
	int language;
	int index;
	std::string compId;
	std::string voiceName;
};

class VsqxInfo
{
public:
	VsqxInfo();
	~VsqxInfo();
	void setVender(std::string str);
	void setVersion(std::string str);
	const char* getVender();
	const char* getVersion();

	std::string *vender;
	std::string *version;
};

class VsqxDoc
{
public:
	VsqxDoc();
	VsqxDoc(std::string filePath);
	~VsqxDoc();

	void setPath(std::string filePath);
	bool isVsqx();
	int load();

	int getError(char* buffer,int bufSize, bool keepMessage = false);
	VsqxInfo* getInfo();
	int getVoiceInfoNum();
	VVoiceInfo** getVoiceInfo();

protected:
	std::string *path;
	std::string errorString;
	VsqxInfo *info;
	std::vector<VVoiceInfo*> voiceInfo;

	void setError(const char* format,...);
	void init();

};

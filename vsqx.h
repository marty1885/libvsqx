#include <iostream>
#include <string.h>
#include <stdint.h>
#include <vector>

#include <tinyxml2.h>

class VParameter
{
public:
	int value;
	int clock;
};

class VParameterList
{
public:
	void setName(std::string str);
	const char* getName();
	int addParameter(int clock,int val);
	int getParameter(int clock = 0);
	int remove(int startClock, int endClock);

	std::string name;
	std::vector<VParameter*> value;
};

class VParameterMatrix
{
public:
	std::vector<VParameterList*> parameterList;

	int addParameter(const char* name, int clock,int val);
	int getParameter(const char* name, int clock);
};

class VVoiceParameter
{
public:
	int bre;
	int bri;
	int cle;
	int gen;
	int ope;
};

class VVoiceInfo
{
public:
	int language;
	int index;
	std::string compId;
	std::string voiceName;
	VVoiceParameter voiceParameter;

	int getLanguageId();
	const char* getLanguageString();
};

class VTimeSignature
{
public:
	int posMes;
	int nume;
	int denomi;
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

protected:
	std::string *vender;
	std::string *version;
};

class VMasterTrack
{
public:
	std::string name;
	std::string comment;
	int resolution;
	int preMeasure;

	std::vector<VTimeSignature*> timeSignature;

	int addTimeSignature(int posMes, int nume, int denomi);
	int getTimeSignatureNum();
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
	VMasterTrack* getMasterTrack();

protected:
	std::string *path;
	std::string errorString;
	VsqxInfo *info;
	std::vector<VVoiceInfo*> voiceInfo;
	VMasterTrack* masterTrack;

	void setError(const char* format,...);
	void init();

};

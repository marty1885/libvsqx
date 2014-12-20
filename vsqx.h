#include <iostream>
#include <string.h>
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

class VVstPlugin
{
public:
	std::string id;
	std::string name;
	int sdkVersion;
	int parameterNum;
	int presetNum;
	std::vector<int> value;
	int enable;
	int bypass;

	int loadInfo(tinyxml2::XMLElement* vstPluginElement);
	VVstPlugin();
};

class VMixerUnit
{
public:
	int trackNo;
	int inGain;
	VVstPlugin *vstPlugin;
	int sendLevel;
	int sendEnable;
	int mute;
	int solo;
	int pan;
	int vol;

	int loadInfo(tinyxml2::XMLElement *vsUnitElement);
	VMixerUnit();
	~VMixerUnit();
};

class VMasterUnit
{
public:
	VMasterUnit();
	~VMasterUnit();

	int outDev;
	VVstPlugin *vstPlugin;
	int retLevel;
	int vol;
};

class VMixer
{
public:
	VMasterUnit masterUnit;
	std::vector<VMixerUnit*> vsUnit;
	VMixerUnit* seUnit;
	VMixerUnit* karaokeUnit;

	VMixer();
	~VMixer();
	int getVsUnitNum();
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

class VTempo
{
public:
	int posTick;
	int bpm;
};

class VPartStyle
{
public:
	virtual int loadInfo(tinyxml2::XMLElement *partStyleElement);

	int accent;
	int bendDep;
	int bendLen;
	int decay;
	int fallPort;
	int opening;
	int risePort;
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

class VStylePlugin
{
public:
	VStylePlugin();
	int loadInfo(tinyxml2::XMLElement *stylePluginElement);
	
	std::string stylePluginId;
	std::string stylePluginName;
	std::string version;
};

class VNoteStyle : public VPartStyle
{
};

class VNote
{
public:
	int loadInfo(tinyxml2::XMLElement *noteElement);

	int posTick;
	int durTick;
	int noteNum;
	int velocity;
	std::string lyric;
	std::string phnms;
	VNoteStyle noteStyle;
};

class VMusicalPart
{
public:
	int posTick;
	int playTime;
	std::string partName;
	std::string comment;
	VStylePlugin stylePlugin;
	VPartStyle partStyle;
	VVoiceInfo singer;
	std::vector<VNote*> note;

	int loadInfo(tinyxml2::XMLElement *musicalTrackElement);
};

class VTrack
{
public:
	int trackNo;
	std::string name;
	std::string comment;
	std::vector<VMusicalPart*> musicalPart;

	int loadInfo(tinyxml2::XMLElement trackElement);
};

class VMasterTrack
{
public:
	std::string name;
	std::string comment;
	int resolution;
	int preMeasure;

	std::vector<VTimeSignature*> timeSignature;
	std::vector<VTempo*> tempo;

	int addTimeSignature(int posMes, int nume, int denomi);
	int getTimeSignatureNum();
	
	int addTempo(int posTick, int bpm);
	int getTempoNum();
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
	VMixer* getMixer();
	VVoiceInfo** getVoiceInfo();
	VMasterTrack* getMasterTrack();
	int getTrackNum();
	VTrack** getTrack();

protected:
	std::string *path;
	std::string errorString;
	VsqxInfo *info;
	std::vector<VVoiceInfo*> voiceInfo;
	VMasterTrack* masterTrack;
	VMixer* mixer;
	std::vector<VTrack*> track;

	void setError(const char* format,...);
	void init();

};

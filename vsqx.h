#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <fstream>

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
	int getSize();
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
	tinyxml2::XMLElement* createXml(tinyxml2::XMLDocument *doc);
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
	tinyxml2::XMLElement* createXml(tinyxml2::XMLDocument *doc);
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

class VAux
{
public:
		std::string auxId;
		std::string content;
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

class VNoteStyle
{
public:
	int accent;
	int bendDep;
	int bendLen;
	int decay;
	int fallPort;
	int opening;
	int risePort;
	int vibLen;
	int vibType;

	int loadInfo(tinyxml2::XMLElement* noteStyleElement);
	tinyxml2::XMLElement* createXml(tinyxml2::XMLDocument *doc);
};

class VNote
{
public:
	int posTick;
	int durTick;
	int noteNum;
	int velocity;
	std::string lyric;
	std::string phnms;
	VNoteStyle noteStyle;
	VParameterList vibDep;
	VParameterList vibRate;
	

	int loadInfo(tinyxml2::XMLElement *noteElement);
	tinyxml2::XMLElement* createXml(tinyxml2::XMLDocument* doc);
	const char* getNoteName();
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
	
	VParameterList dyn;
	VParameterList bre;
	VParameterList bri;
	VParameterList cle;
	VParameterList gen;
	VParameterList por;
	VParameterList pit;
	VParameterList pbs;

	int loadInfo(tinyxml2::XMLElement *musicalTrackElement);
	tinyxml2::XMLElement* createXml(tinyxml2::XMLDocument *doc);
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

class VWavPart
{
public:
	int posTick;
	int playTime;
	std::string partName;
	std::string comment;
	int sampleRate;
	int sampleReso;
	int channels;
	std::string filePath;
	
	int loadInfo(tinyxml2::XMLElement *wavPartElement);
	tinyxml2::XMLElement* createXml(tinyxml2::XMLDocument *doc,std::string name = "track");
};

class VWavTrack
{
public:
	std::vector<VWavPart*> wavPart;
	
	int loadInfo(tinyxml2::XMLElement *wavTrackElemnt);
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
	int safe(std::string  filePath = "");

	int getError(char* buffer,int bufSize, bool keepMessage = false);
	VsqxInfo* getInfo();
	int getVoiceInfoNum();
	VMixer* getMixer();
	VVoiceInfo** getVoiceInfo();
	VMasterTrack* getMasterTrack();
	int getTrackNum();
	VTrack** getTrack();
	VAux* getAux();
	VWavTrack** getWavTrack();

protected:
	std::string *path;
	std::string errorString;
	VsqxInfo *info;
	std::vector<VVoiceInfo*> voiceInfo;
	VMasterTrack* masterTrack;
	VMixer* mixer;
	VAux aux;
	std::vector<VTrack*> track;
	VWavTrack *wavTrack;

	void setError(const char* format,...);
	void init();

};

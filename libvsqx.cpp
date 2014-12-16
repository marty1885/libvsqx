#include <vsqx.h>
using namespace std;
using namespace tinyxml2;

#define CHECK_ROOT_ELEMENT	0
#define CHECK_BEFORE_LOAD	0

static const int vsqxDefaultValue = 0;

VsqxDoc::VsqxDoc()
{
	init();
}

VsqxDoc::VsqxDoc(string filePath)
{
	init();
	setPath(filePath);
}

VsqxDoc::~VsqxDoc()
{
	delete path;
	delete info;
	delete masterTrack;
}

void VsqxDoc::init()
{
	path = new string;
	path->clear();

	info = new VsqxInfo;
	masterTrack = new VMasterTrack;
}

void VsqxDoc::setPath(std::string filePath)
{
	path->append(filePath.c_str());
}

bool VsqxDoc::isVsqx()
{
	XMLDocument doc;
	doc.LoadFile(path->c_str());
	if(doc.Error() == true)//Have Error
		return false;
#if CHECK_ROOT_ELEMENT == 1
	if(strcmp(doc.RootElement()->Name(),"vsq3") != 0)//Vocaloid 3 root element name
		return false;
#endif
	return true;
}

int VsqxDoc::load()
{
#if CHECK_BEFORE_LOAD == 1
	if(isVsqx() == false)
	{
		setError("%s: Faild to load %s.\nNot a VSQx vailid file",__func__,path->c_str());
		return 0;
	}
#endif
	XMLDocument doc;
	doc.LoadFile(path->c_str());
	XMLElement *rootElement = doc.RootElement();

	XMLElement *venderElement = rootElement->FirstChildElement("vender");
	if(venderElement == NULL)//vender not Found
		info->setVender("Unknown");
	else
		info->setVender(venderElement->GetText());

	XMLElement *versionElement = rootElement->FirstChildElement("version");
	if(versionElement == NULL)//vender not Found
		info->setVersion("0.0.0");//Unknown
	else
		info->setVersion(versionElement->GetText());

	XMLElement *voiceTableElement = rootElement->FirstChildElement("vVoiceTable");
	if(voiceTableElement != NULL)//voice data found
	{
		XMLElement *element = voiceTableElement->FirstChildElement("vVoice");//just so we won't loadup something weird
		while(element != NULL)
		{
			VVoiceInfo *voiceInfomation = new VVoiceInfo;
			voiceInfo.push_back(voiceInfomation);
			voiceInfomation->language = atoi(element->FirstChildElement("vBS")->GetText());
			voiceInfomation->index =  atoi(element->FirstChildElement("vPC")->GetText());
			voiceInfomation->voiceName = element->FirstChildElement("vVoiceName")->GetText();
			voiceInfomation->compId = element->FirstChildElement("compID")->GetText();

			XMLElement *voiceParam = element->FirstChildElement("vVoiceParam");
			voiceInfomation->voiceParameter.bre = atoi(voiceParam->FirstChildElement("bre")->GetText());
			voiceInfomation->voiceParameter.bri = atoi(voiceParam->FirstChildElement("bri")->GetText());
			voiceInfomation->voiceParameter.cle = atoi(voiceParam->FirstChildElement("cle")->GetText());
			voiceInfomation->voiceParameter.gen = atoi(voiceParam->FirstChildElement("gen")->GetText());
			voiceInfomation->voiceParameter.ope = atoi(voiceParam->FirstChildElement("ope")->GetText());

			element = element->NextSiblingElement("vVoice");
		}

	}
	XMLElement *masterTrackElement = rootElement->FirstChildElement("masterTrack");
	masterTrack->name = masterTrackElement->FirstChildElement("seqName")->GetText();
	masterTrack->comment = masterTrackElement->FirstChildElement("comment")->GetText();
	masterTrack->resolution = atoi(masterTrackElement->FirstChildElement("resolution")->GetText());
	masterTrack->preMeasure = atoi(masterTrackElement->FirstChildElement("preMeasure")->GetText());

	XMLElement *timeSignatureElement = masterTrackElement->FirstChildElement("timeSig");
	while(timeSignatureElement != NULL)
	{
		int posMes = atoi(timeSignatureElement->FirstChildElement("posMes")->GetText());
		int nume = atoi(timeSignatureElement->FirstChildElement("nume")->GetText());
		int denomi = atoi(timeSignatureElement->FirstChildElement("denomi")->GetText());

		masterTrack->addTimeSignature(posMes,nume,denomi);

		timeSignatureElement = timeSignatureElement->NextSiblingElement("timeSig");
	}
	
	return 1;
}

VsqxInfo* VsqxDoc::getInfo()
{
	return info;
}

VVoiceInfo** VsqxDoc::getVoiceInfo()
{
	static VVoiceInfo** ptr = NULL;
	if(ptr != NULL)
		delete [] ptr;
	int size = voiceInfo.size();
	ptr = new VVoiceInfo*[size];
	for(int i=0;i<size;i++)
		ptr[i] = voiceInfo[i];
	return ptr;
}


int VsqxDoc::getError(char* buffer,int bufSize, bool keepMessage)
{
	int length = errorString.length();
	if(length != 0)//have error
	{
		//bigger than -1, a valed buffer size
		if(bufSize >= 0)
		{
			int copyLength = bufSize-1 > length ? length : bufSize-1;
			memcpy(buffer,errorString.c_str(),length);
			buffer[copyLength] = 0;
		}
		else//else, just copy
			strcpy(buffer,errorString.c_str());

		if(keepMessage == false)
			errorString.clear();

		return 1;
	}
	if(bufSize > 0)
		buffer[0] = 0;
	return 0;
}

void VsqxDoc::setError(const char* format,...)
{
	if(errorString.length() != 0)
		errorString.clear();

	char* str = new char[2000];
	va_list arglist;
	va_start(arglist,format);
	vsprintf(str,format,arglist);
	va_end(arglist);

	errorString.append(str);

	delete [] str;
}

int VsqxDoc::getVoiceInfoNum()
{
	return voiceInfo.size();
}

VMasterTrack* VsqxDoc::getMasterTrack()
{
	return masterTrack;
}
////////////////////////////////////////////////
//VVoiceInfo
////////////////////////////////////////////////
int VVoiceInfo::getLanguageId()
{
	return language;
}


const char* VVoiceInfo::getLanguageString()
{
	//TODO: Add more id to language here
	switch(language)
	{
	case 0:
		return "JP";
		break;

	case 1:
		return "EN";
		break;

	case 3:
		return "CH";
		break;
	}
	char* str = new char[4];
	sprintf(str,"%d",language);
	return str;
}
////////////////////////////////////////////////
//VMasterTrack
////////////////////////////////////////////////
int VMasterTrack::getTimeSignatureNum()
{
	return timeSignature.size();
}

int VMasterTrack::addTimeSignature(int posMes, int nume, int denomi)
{
	int size = timeSignature.size();
	if(size == 0)//nothing in vector
	{
		VTimeSignature *timeSig = new VTimeSignature;
		timeSig->posMes = posMes;
		timeSig->nume = nume;
		timeSig->denomi = denomi;

		timeSignature.push_back(timeSig);
	}
	else
	{
		int id = 0;
		for(int i=0;i<size;i++)
		{
			if(timeSignature[i]->posMes > posMes)
			{
				id = i-1;
				break;
			}
		}
		if(timeSignature[id]->posMes == posMes)//a exact match
		{
			timeSignature[id]->nume = nume;
			timeSignature[id]->denomi = denomi;
		}
		else
		{
			VTimeSignature *timeSig = new VTimeSignature;
			timeSig->posMes = posMes;
			timeSig->nume = nume;
			timeSig->denomi = denomi;

			timeSignature.insert(timeSignature.begin()+id+1,timeSig);
		}
	}
	return 1;
}
////////////////////////////////////////////////
//VsqxInfo
////////////////////////////////////////////////
VsqxInfo::VsqxInfo()
{
	vender = new string("");
	version = new string("");
}


VsqxInfo::~VsqxInfo()
{
	delete vender;
	delete version;
}

void VsqxInfo::setVender(string str)
{
	*vender = str;
}

void VsqxInfo::setVersion(string str)
{
	*version = str;
}

const char* VsqxInfo::getVender()
{
	return vender->c_str();
}
const char* VsqxInfo::getVersion()
{
	return version->c_str();
}

///////////////////////////////////////////////
//VParameterList
///////////////////////////////////////////////
void VParameterList::setName(string str)
{
	name = str;
}

const char* VParameterList::getName()
{
	return name.c_str();
}

int VParameterList::addParameter(int clock, int val)
{
	int size = value.size();
	int id = 0;
	for(int i=0;i < size;i++)//find the cloest value point
	{
		if(value[i]->clock <= clock)
			id = i;
		else
			break;
	}
	if(value[id]->clock == clock)//we have a value point already
		value[id]->value = val;
	else
	{
		VParameter *para = new VParameter;
		para->clock = clock;
		para->value = val;
		value.insert(value.begin()+id+1,para);
	}
	return 1;
}

int VParameterList::getParameter(int clock)
{
	int size = value.size();
	if(size == 0)//No value
		return vsqxDefaultValue;

	int id = 0;
	for(int i=0;i < size;i++)//find the cloest value point
	{
		if(value[i]->clock <= clock)
			id = i;
		else
			break;
	}
	return value[id]->value;
}

int VParameterList::remove(int startClock, int endClock)
{
	int size = value.size();
	int startId = 0;
	int endId = 0;

	if(startClock > endClock)//Do nothing
		return 0;

	for(int i=0;i < size;i++)
		if(value[i]->clock >= startClock)
			startId = i;

	for(int i=startId;i < size;i++)
		if(value[i]->clock >= endClock)
			startId = i-1;

	value.erase(value.begin()+startId,value.begin()+endId);
	return endId - startId + 1;
}

/////////////////////////////////////////////
//VParameterMatrix
/////////////////////////////////////////////
int VParameterMatrix::addParameter(const char* name, int clock,int val)
{
	int size = parameterList.size();
	int id = -1;
	for(int i=0;i<size;i++)
		if(strcmp(parameterList[i]->getName(),name)==0)
		{
			id = i;
			break;
		}
	VParameterList *paraList = NULL;
	if(id = -1)//parameterlist with given name not found
	{
		paraList = new VParameterList;
		parameterList.push_back(paraList);
	}
	else
		paraList = parameterList[id];
	paraList->addParameter(clock,val);
	return 1;
}
int VParameterMatrix::getParameter(const char* name, int clock)
{
	int size = parameterList.size();
	int id = -1;
	for(int i=0;i<size;i++)
		if(strcmp(parameterList[i]->getName(),name)==0)
		{
			id = i;
			break;
		}
	if(id == -1)//not found
		return vsqxDefaultValue;

	return parameterList[id]->getParameter(clock);
}

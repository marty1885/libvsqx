#include <vsqx.h>
using namespace std;

#define CHECK_ROOT_ELEMENT	0
#define CHECK_BEFORE_LOAD	0

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
}

void VsqxDoc::init()
{
	path = new string;
	path->clear();

	info = new VsqxInfo;
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
		setError("%s: Faild to load %s.\n",__func__,path->c_str());
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
	if(voiceElement != NULL)//voice data found
	{
		XMLElement element = voiceTableElement->FirstChildElement("vVoice");//just so we won't loadup something weird
	}
	return 1;
}

VsqxInfo* VsqxDoc::getInfo()
{
	return info;
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


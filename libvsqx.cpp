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
	delete mixer;
	delete [] wavTrack;
}

void VsqxDoc::init()
{
	path = new string;
	path->clear();

	info = new VsqxInfo;
	masterTrack = new VMasterTrack;
	mixer = new VMixer;
	wavTrack = new VWavTrack[2];
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
	//Check if is a valid VSQx file
	if(isVsqx() == false)
	{
		setError("%s: Faild to load %s.\nNot a VSQx vailid file",__func__,path->c_str());
		return 0;
	}
#endif
	XMLDocument doc;
	doc.LoadFile(path->c_str());
	XMLElement *rootElement = doc.RootElement();
	if(rootElement == NULL)
	{
		setError("Error: Unable to load VSQx: %s",path->c_str());
		return 0;
	}

	//Load basic info of the file
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

	//Load voice(singer) info
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

	//load mixer info
	XMLElement *mixerElement = rootElement->FirstChildElement("mixer");
	XMLElement *masterUnitElement = mixerElement->FirstChildElement("masterUnit");
	XMLElement *masterVstPluginElement = masterUnitElement->FirstChildElement("vstPlugin");


	mixer->masterUnit.outDev = atoi(masterUnitElement->FirstChildElement("outDev")->GetText());
	mixer->masterUnit.retLevel = atoi(masterUnitElement->FirstChildElement("retLevel")->GetText());
	mixer->masterUnit.vol = atoi(masterUnitElement->FirstChildElement("vol")->GetText());
	for(int i=0;masterVstPluginElement != NULL;i++)
	{
		mixer->masterUnit.vstPlugin[i].loadInfo(masterVstPluginElement);
		masterVstPluginElement = masterVstPluginElement->NextSiblingElement("vstPlugin");
	}
	masterVstPluginElement = masterUnitElement->FirstChildElement("vstPluginSR");
	if(masterVstPluginElement != NULL)//we might not have this
		mixer->masterUnit.vstPlugin[2].loadInfo(masterVstPluginElement);

	XMLElement *vsUnitElement = mixerElement->FirstChildElement("vsUnit");
	//In vsqx, the number of vsUnit we have desides how much track we have.
	//count howmuch track we have and prepare ram for it
	while(vsUnitElement != NULL)
	{
		VMixerUnit* vsUnit = new VMixerUnit;
		mixer->vsUnit.push_back(vsUnit);
		vsUnitElement = vsUnitElement->NextSiblingElement("vsUnit");
	}
	vsUnitElement = mixerElement->FirstChildElement("vsUnit");
	//load evey vsUnit and make it in order
	while(vsUnitElement != NULL)
	{
		int id = atoi(vsUnitElement->FirstChildElement("vsTrackNo")->GetText());
		mixer->vsUnit[id]->loadInfo(vsUnitElement);
		vsUnitElement = vsUnitElement->NextSiblingElement("vsUnit");
	}

	XMLElement *seUnitElement = mixerElement->FirstChildElement("seUnit");
	mixer->seUnit->loadInfo(seUnitElement);

	XMLElement *karaokeUnitElement = mixerElement->FirstChildElement("karaokeUnit");
	mixer->karaokeUnit->loadInfo(karaokeUnitElement);

	//Load mster track
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

	XMLElement *tempoElement = masterTrackElement->FirstChildElement("tempo");
	while(tempoElement != NULL)
	{
		int posTick = atoi(tempoElement->FirstChildElement("posTick")->GetText());
		int bpm = atoi(tempoElement->FirstChildElement("bpm")->GetText());

		masterTrack->addTempo(posTick,bpm);

		tempoElement = tempoElement->NextSiblingElement("tempo");
	}

	//load track
	XMLElement *vsTrackElement =  rootElement->FirstChildElement("vsTrack");
	while(vsTrackElement != NULL)
	{
		VTrack *vsTrack = new VTrack;
		vsTrack->trackNo = atoi(vsTrackElement->FirstChildElement("vsTrackNo")->GetText());
		vsTrack->name = vsTrackElement->FirstChildElement("trackName")->GetText();
		vsTrack->comment = vsTrackElement->FirstChildElement("comment")->GetText();
		XMLElement *musicalPartElement = vsTrackElement->FirstChildElement("musicalPart");
		while(musicalPartElement != NULL)//In some case, we don't have musicalPart.
		{
			VMusicalPart *musicalPart = new VMusicalPart;
			vsTrack->musicalPart.push_back(musicalPart);
			musicalPart->loadInfo(musicalPartElement);
			
			musicalPartElement = musicalPartElement->NextSiblingElement("musicalPart");
		}

		track.push_back(vsTrack);
		vsTrackElement = vsTrackElement->NextSiblingElement("vsTrack");
	}
	XMLElement *seTrackElement = rootElement->FirstChildElement("seTrack");
	wavTrack[0].loadInfo(seTrackElement);
	
	XMLElement *karaokeTrackElement = rootElement->FirstChildElement("karaokeTrack");
	wavTrack[1].loadInfo(karaokeTrackElement );
	
	//load aux
	XMLElement *auxElement = rootElement->FirstChildElement("aux");
	aux.auxId = auxElement->FirstChildElement("auxID")->GetText();
	aux.content = auxElement->FirstChildElement("content")->GetText();

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

int VsqxDoc::safe(string filePath)
{
	string *writePath = &filePath;
	if(filePath.length() == 0)//no text
		writePath = path;
	
	 tinyxml2::XMLDocument doc;
	
	XMLDeclaration* decl = doc.NewDeclaration("xml version=\"1.0\" encoding=\"UTF-8\"  standalone=\"no\"");  
	doc.LinkEndChild(decl);
	//HACK: A hack since tinyxml does not support schema, another hack down there is required
	XMLElement *rootElement = doc.NewElement("vsq3 xmlns=\"http://www.yamaha.co.jp/vocaloid/schema/vsq3/\"\n\
	xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n\
	xsi:schemaLocation=\"http://www.yamaha.co.jp/vocaloid/schema/vsq3/ vsq3.xsd\"");
	doc.InsertEndChild(rootElement);

	XMLElement *venderElementy = doc.NewElement("vender");
	rootElement->InsertEndChild(venderElementy);
	XMLText* venderText = doc.NewText(info->getVender());
	venderText->SetCData(true);
	venderElementy->InsertEndChild(venderText);
	XMLElement *versionElement = doc.NewElement("version");
	rootElement->InsertEndChild(versionElement);
	XMLText *versionText = doc.NewText(info->getVersion());
	versionText->SetCData(true);
	versionElement->InsertEndChild(versionText);
	
	XMLElement *voiceTableElement = doc.NewElement("vVoiceTable");
	rootElement->InsertEndChild(voiceTableElement);
	int size = 0;
	size = getVoiceInfoNum();
	for(int i=0;i<size;i++)
	{
		VVoiceInfo *voiceInfomation = voiceInfo[i];
		XMLElement *voiceElement = doc.NewElement("vVoice");
		XMLElement *vBsElement = doc.NewElement("vBS");
		vBsElement->SetText(voiceInfomation->language);
		voiceElement->InsertEndChild(vBsElement);
		XMLElement *vPcElement = doc.NewElement("vPC");
		vPcElement->SetText(voiceInfomation->index);
		voiceElement->InsertEndChild(vPcElement);
		XMLElement *compIdElement = doc.NewElement("compID");
		XMLText *compIdText = doc.NewText(voiceInfomation->compId.c_str());
		compIdElement->InsertEndChild(compIdText);
		compIdText->SetCData(true);
		voiceElement->InsertEndChild(compIdElement);
		XMLElement *nameElement = doc.NewElement("vVoiceName");
		XMLText *nameText = doc.NewText(voiceInfomation->voiceName.c_str());
		nameText->SetCData(true);
		nameElement->InsertEndChild(nameText);
		voiceElement->InsertEndChild(nameElement);

		VVoiceParameter  *para = &voiceInfomation->voiceParameter;
		XMLElement *voiceParamElement = doc.NewElement("vVoiceParam");
		XMLElement *breElement = doc.NewElement("bre");
		breElement->SetText(para->bre);
		voiceParamElement->InsertEndChild(breElement);
		XMLElement *briElement = doc.NewElement("bri");
		briElement->SetText(para->bri);
		voiceParamElement->InsertEndChild(briElement);
		XMLElement *cleElement = doc.NewElement("cle");
		cleElement->SetText(para->cle);
		voiceParamElement->InsertEndChild(cleElement);
		XMLElement *genElement = doc.NewElement("gen");
		genElement->SetText(para->gen);
		voiceParamElement->InsertEndChild(genElement);
		XMLElement *opeElement = doc.NewElement("ope");
		opeElement->SetText(para->ope);
		voiceParamElement->InsertEndChild(opeElement);
		
		voiceElement->InsertEndChild(voiceParamElement);
		
		voiceTableElement->InsertEndChild(voiceElement);
	}

	XMLElement *mixerElement = doc.NewElement("mixer");
	rootElement->InsertEndChild(mixerElement);
	XMLElement *masterUnitElement = doc.NewElement("masterUnit");
	mixerElement->InsertEndChild(masterUnitElement);
	XMLElement *outDevElement = doc.NewElement("outDev");
	masterUnitElement->InsertEndChild(outDevElement);
	outDevElement->SetText(mixer->masterUnit.outDev);
	
	for(int i=0;i<2;i++)
	{
		XMLElement *vstPluginElement = mixer->masterUnit.vstPlugin[i].createXml(&doc);
		masterUnitElement->InsertEndChild(vstPluginElement);
	}

	XMLElement *vstPluginSrElement = mixer->masterUnit.vstPlugin[2].createXml(&doc);
	vstPluginSrElement->SetName("vstPluginSR");
	masterUnitElement->InsertEndChild(vstPluginSrElement);
	
	XMLElement *retLevelElement = doc.NewElement("retLevel");
	retLevelElement->DeleteChild(retLevelElement->FirstChildElement("vsTrackNo"));
	retLevelElement->SetText(mixer->masterUnit.retLevel);
	masterUnitElement->InsertEndChild(retLevelElement);

	XMLElement *volElement = doc.NewElement("vol");
	volElement->SetText(mixer->masterUnit.vol);
	masterUnitElement->InsertEndChild(volElement);

	size = mixer->vsUnit.size();
	for(int i=0;i<size;i++)
	{
		XMLElement *vsUnitElement = mixer->vsUnit[i]->createXml(&doc);
		mixerElement->InsertEndChild(vsUnitElement);
	}

	XMLElement* seUnitElement = mixer->seUnit->createXml(&doc);
	seUnitElement->DeleteChild(seUnitElement->FirstChildElement("vsTrackNo"));
	seUnitElement->SetName("seUnit");
	mixerElement->InsertEndChild(seUnitElement);

	XMLElement *karaokeUnitElement = mixer->karaokeUnit->createXml(&doc);
	seUnitElement->DeleteChild(karaokeUnitElement->FirstChildElement("vsTrackNo"));
	seUnitElement->DeleteChild(karaokeUnitElement->FirstChildElement("sendLevel"));
	seUnitElement->DeleteChild(karaokeUnitElement->FirstChildElement("sendEnable"));
	seUnitElement->DeleteChild(karaokeUnitElement->FirstChildElement("pan"));
	karaokeUnitElement->SetName("karaokeUnit");
	mixerElement->InsertEndChild(karaokeUnitElement);

	XMLElement *masterTrackElement = doc.NewElement("masterTrack");
	rootElement->InsertEndChild(masterTrackElement);
	XMLElement *seqNameElement = doc.NewElement("seqName");
	masterTrackElement->InsertEndChild(seqNameElement);
	XMLText *seqNameText = doc.NewText(masterTrack->name.c_str());
	seqNameElement->InsertEndChild(seqNameText);
	seqNameText->SetCData(true);

	XMLElement *commentElement = doc.NewElement("comment");
	masterTrackElement->InsertEndChild(commentElement);
	XMLText *commentText = doc.NewText(masterTrack->comment.c_str());
	commentElement->InsertEndChild(commentText);
	commentText->SetCData(true);

	XMLElement *resolutionElement = doc.NewElement("resolution");
	resolutionElement->SetText(masterTrack->resolution);
	masterTrackElement->InsertEndChild(resolutionElement);

	XMLElement *preMeasureElement = doc.NewElement("preMeasure");
	preMeasureElement->SetText(masterTrack->preMeasure);
	masterTrackElement->InsertEndChild(preMeasureElement);

	size = masterTrack->getTimeSignatureNum();
	for(int i=0;i<size;i++)
	{
		XMLElement *timeSigElement = doc.NewElement("timeSig");
		masterTrackElement->InsertEndChild(timeSigElement);
		XMLElement *posMesElement = doc.NewElement("posMes");
		timeSigElement->InsertEndChild(posMesElement);
		posMesElement->SetText(masterTrack->timeSignature[i]->posMes);
		XMLElement *numeElement = doc.NewElement("nume");
		timeSigElement->InsertEndChild(numeElement);
		numeElement->SetText(masterTrack->timeSignature[i]->nume);
		XMLElement *denomiElement = doc.NewElement("denomi");
		timeSigElement->InsertEndChild(denomiElement);
		denomiElement->SetText(masterTrack->timeSignature[i]->denomi);
		//TODO:add timeSig write3
	}

	for(int i=0;i<size;i++)
	{
		XMLElement *tempoElement = doc.NewElement("tempo");
		masterTrackElement->InsertEndChild(tempoElement);
		XMLElement *posTickElement = doc.NewElement("posTick");
		tempoElement->InsertEndChild(posTickElement);
		posTickElement->SetText(masterTrack->tempo[i]->posTick);
		XMLElement *bpmElement = doc.NewElement("bpm");
		tempoElement->InsertEndChild(bpmElement);
		bpmElement->SetText(masterTrack->tempo[i]->bpm);
	}

	size = track.size();
	for(int i=0;i<size;i++)
	{
		XMLElement *vsTrackElement = doc.NewElement("vsTrack");
		rootElement->InsertEndChild(vsTrackElement);
		XMLElement *vsTrackNoElement = doc.NewElement("vsTrackNo");
		vsTrackElement->InsertEndChild(vsTrackNoElement);
		vsTrackNoElement->SetText(track[i]->trackNo);

		XMLElement *trackNameElement = doc.NewElement("trackName");
		vsTrackElement->InsertEndChild(trackNameElement);
		XMLText *trackNameText = doc.NewText(track[i]->name.c_str());
		trackNameElement->InsertEndChild(trackNameText);
		trackNameText->SetCData(true);
		int musicalPartNum = track[i]->musicalPart.size();

		XMLElement *commentElement = doc.NewElement("comment");
		vsTrackElement->InsertEndChild(commentElement);
		XMLText *commentText = doc.NewText(track[i]->comment.c_str());
		commentElement->InsertEndChild(commentText);
		commentText->SetCData(true);

		for(int w=0;w<musicalPartNum;w++)
		{
			XMLElement *musicalPartElement = track[i]->musicalPart[w]->createXml(&doc);
			vsTrackElement->InsertEndChild(musicalPartElement);
		}
	}

	XMLElement *seTrackElement = wavTrack[0].createXml(&doc);
	rootElement->InsertEndChild(seTrackElement,"seTrack");


	//TODO: write more thing

	//doc.Print();
	//doc.SaveFile(writePath->c_str());

	//HACK: to remove scrap from the end
	//XXX: TRY ANYEAY TO REMVOE THIS hack, IT'S SLOW AND NASTY
	XMLPrinter printer;
	doc.Print(&printer);

	string file(printer.CStr());
	const char *str1= file.c_str();
	char *processStr = new char[file.size()+10];
	size = file.size();
	strcpy(processStr,str1);
	for(int i=size;i>=0;i--)
	{
		if(processStr[i] == '<')
		{
			strcpy(&processStr[i],"</vsq3>");
			break;
		}
		else
			processStr[i] = '\0';
	}
	FILE *out = fopen(writePath->c_str(),"w+");
	fprintf(out,"%s\n",processStr);
	//printf("%s\n",processStr);
	fclose(out);
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

VMixer* VsqxDoc::getMixer()
{
	return mixer;
}

int VsqxDoc::getTrackNum()
{
	return track.size();
}

VTrack** VsqxDoc::getTrack()
{
	static VTrack** ptr = NULL;
	if(ptr != NULL)
		delete [] ptr;
	int size = track.size();
	ptr = new VTrack*[size];
	for(int i=0;i<size;i++)
		ptr[i] = track[i];
	return ptr;
}
VAux* VsqxDoc::getAux()
{
	return &aux;
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

	case 4:
		return "CH";
		break;
	}
	static char* str = NULL;
	if(str != NULL)
		delete [] str;
	str = new char[4];
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

int VMasterTrack::getTempoNum()
{
	return tempo.size();
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
//VMasterUnit
////////////////////////////////////////////////

VMasterUnit::VMasterUnit()
{
	vstPlugin = new VVstPlugin[3];
}

VMasterUnit::~VMasterUnit()
{
	delete [] vstPlugin;
}

int VMasterTrack::addTempo(int posTick, int bpm)
{
	int size = tempo.size();
	if(size == 0)//nothing in vector
	{
		VTempo *tem = new VTempo;
		tem->posTick = posTick;
		tem->bpm = bpm;

		tempo.push_back(tem);
	}
	else
	{
		int id = 0;
		for(int i=0;i<size;i++)
		{
			if(tempo[i]->posTick > posTick)
			{
				id = i-1;
				break;
			}
		}
		if(tempo[id]->posTick == posTick)//a exact match
		{
			tempo[id]->bpm = bpm;
		}
		else
		{
			VTempo *tem = new VTempo;
			tem->posTick = posTick;
			tem->bpm = bpm;

			tempo.insert(tempo.begin()+id+1,tem);
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

int VParameterList::getSize()
{
	return value.size();
}
int VParameterList::addParameter(int clock, int val)
{
	int size = value.size();
	int id = 0;
	if(value.size() == 0)
	{
		VParameter *para = new VParameter;
		para->clock = clock;
		para->value = val;
		value.push_back(para);
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

/////////////////////////////////////////////
//VVstPlugin
////////////////////////////////////////////
VVstPlugin::VVstPlugin()
{
	id = "    ";//No reason. Vocaloid default.
	name.clear();
	sdkVersion = 0;
	parameterNum = 0;
	presetNum = 0;
	value.clear();
	enable = 0;
	bypass = 0;

}
int VVstPlugin::loadInfo(XMLElement* vstPluginElement)
{
	id = vstPluginElement->FirstChildElement("vstPluginID")->GetText();
	name = vstPluginElement->FirstChildElement("vstPluginName")->GetText();
	sdkVersion = atoi(vstPluginElement->FirstChildElement("vstSDKVersion")->GetText());
	parameterNum = atoi(vstPluginElement->FirstChildElement("vstParamNum")->GetText());
	presetNum = atoi(vstPluginElement->FirstChildElement("vstPresetNo")->GetText());
	enable = atoi(vstPluginElement->FirstChildElement("enable")->GetText());
	bypass = atoi(vstPluginElement->FirstChildElement("bypass")->GetText());

	XMLElement *paramValElement = vstPluginElement->FirstChildElement("vstParamVal");
	if(paramValElement != NULL)
	{
		XMLElement *valElement = paramValElement->FirstChildElement("val");
		for(int i=0;i<parameterNum;i++)
		{
			value.push_back(atoi(valElement->GetText()));
			valElement = valElement->NextSiblingElement("val");
		}
	}
	return 1;
}

XMLElement* VVstPlugin::createXml(XMLDocument *doc)
{
	VVstPlugin *vst = this;
		
	
	XMLElement *vstPluginElement = doc->NewElement("vstPlugin");
		
	XMLElement *vstPluginIdElement = doc->NewElement("vstPluginID");
	XMLText *vstPluginIdText = doc->NewText(vst->id.c_str());
	vstPluginIdText->SetCData(true);
	vstPluginIdElement->InsertEndChild(vstPluginIdText);
	vstPluginElement->InsertEndChild(vstPluginIdElement);
		
	XMLElement *vstPluginNameElement = doc->NewElement("vstPluginName");
	XMLText *vstPluginNameText = doc->NewText(vst->name.c_str());
	vstPluginNameText->SetCData(true);
	vstPluginNameElement->InsertEndChild(vstPluginNameText);
	vstPluginElement->InsertEndChild(vstPluginNameElement);
	vstPluginNameElement->SetText(vst->name.c_str());
		
	XMLElement *vstSdkVersionElement = doc->NewElement("vstSDKVersion");
	vstPluginElement->InsertEndChild(vstSdkVersionElement);
	vstSdkVersionElement->SetText(vst->sdkVersion);
		
	int paraNum = vst->parameterNum;
	XMLElement *vstParamNumElement = doc->NewElement("vstParamNum");
	vstPluginElement->InsertEndChild(vstParamNumElement);
	vstParamNumElement->SetText(paraNum);
	if(paraNum != 0)
	{
		XMLElement *vstParamValElement = doc->NewElement("vstParamVal");
		vstPluginElement->InsertEndChild(vstParamValElement);
		for(int w=0;w<paraNum;w++)
		{
			XMLElement *valElement = doc->NewElement("val");
			valElement->SetText(vst->value[w]);
			vstParamValElement->InsertEndChild(valElement);
		}
	}
	
	XMLElement *vstPresetNoElement = doc->NewElement("vstPresetNo");
	vstPluginElement->InsertEndChild(vstPresetNoElement);
	vstPresetNoElement->SetText(vst->presetNum);
	
	XMLElement *enableElement = doc->NewElement("enable");
	vstPluginElement->InsertEndChild(enableElement);
	enableElement->SetText(vst->enable);
		
	XMLElement *bypassElement = doc->NewElement("bypass");
	vstPluginElement->InsertEndChild(bypassElement);
	bypassElement->SetText(vst->bypass);
	
	return vstPluginElement;
}
/////////////////////////////////////////////
//VMixer
////////////////////////////////////////////
VMixer::VMixer()
{
	seUnit = new VMixerUnit;
	karaokeUnit = new VMixerUnit;
}

VMixer::~VMixer()
{
	int size = vsUnit.size();
	for(int i=0;i<size;i++)
		delete vsUnit[i];

	delete seUnit;
	delete karaokeUnit;
}

int VMixer::getVsUnitNum()
{
	return vsUnit.size();
}

/////////////////////////////////////////////
//VMixerUnit
////////////////////////////////////////////
VMixerUnit::VMixerUnit()
{
	vstPlugin = new VVstPlugin[2];
}

VMixerUnit::~VMixerUnit()
{
	delete [] vstPlugin;
}

int VMixerUnit::loadInfo(XMLElement *vsUnitElement)
{

	XMLElement *trackNoElement = vsUnitElement->FirstChildElement("vsTrackNo");
	XMLElement *inGainElement = vsUnitElement->FirstChildElement("inGain");
	XMLElement *sendLevelElement = vsUnitElement->FirstChildElement("sendLevel");
	XMLElement *sendEnableElement = vsUnitElement->FirstChildElement("sendEnable");
	XMLElement *panElement = vsUnitElement->FirstChildElement("pan");
	XMLElement *soloElement = vsUnitElement->FirstChildElement("solo");
	XMLElement *muteElement = vsUnitElement->FirstChildElement("mute");
	XMLElement *volElement = vsUnitElement->FirstChildElement("vol");

	if(trackNoElement != NULL)
		trackNo = atoi(trackNoElement->GetText());
	if(inGainElement != NULL)
		inGain = atoi(inGainElement->GetText());
	if(sendLevelElement != NULL)
		sendLevel = atoi(sendLevelElement->GetText());
	if(sendEnableElement != NULL)
		sendEnable = atoi(sendEnableElement->GetText());
	if(muteElement != NULL)
		mute = atoi(muteElement->GetText());
	if(soloElement != NULL)
		solo = atoi(soloElement->GetText());
	if(panElement != NULL)
		pan = atoi(panElement->GetText());
	if(volElement != NULL)
		vol = atoi(volElement->GetText());

	XMLElement *vstElement = vsUnitElement->FirstChildElement("vstPlugin");
	for(int i=0;vstElement != NULL;i++)
	{
		vstPlugin[0].loadInfo(vstElement);
		vstElement = vstElement->NextSiblingElement("vstPlugin");
	}
	return 1;
}

XMLElement* VMixerUnit::createXml(XMLDocument *doc) 
{
	XMLElement *vsUnitElement = doc->NewElement("vsUnit");
	
	XMLElement *trackNoElement = doc->NewElement("vsTrackNo");
	vsUnitElement->InsertEndChild(trackNoElement);
	trackNoElement->SetText(trackNo);
	
	XMLElement *inGainElement = doc->NewElement("inGain");
	vsUnitElement->InsertEndChild(inGainElement);
	inGainElement->SetText(inGain);
		
	for(int w=0;w<2;w++)
	{
		XMLElement *vstPluginElement = vstPlugin[w].createXml(doc);
		vsUnitElement->InsertEndChild(vstPluginElement);
	}
		
	XMLElement *sendLevelElement = doc->NewElement("sendLevel");
	sendLevelElement->SetText(sendLevel);
	vsUnitElement->InsertEndChild(sendLevelElement);
		
	XMLElement *sendEnableElement = doc->NewElement("sendEnable");
	sendEnableElement->SetText(sendEnable);
	vsUnitElement->InsertEndChild(sendEnableElement);
	
	XMLElement *muteElement = doc->NewElement("mute");
	muteElement->SetText(mute);
	vsUnitElement->InsertEndChild(muteElement);
	
	XMLElement *soloElement = doc->NewElement("solo");
	soloElement->SetText(solo);
	vsUnitElement->InsertEndChild(soloElement);
	
	XMLElement *panElement = doc->NewElement("pan");
	panElement->SetText(pan);
	vsUnitElement->InsertEndChild(panElement);
		
	XMLElement *volElement = doc->NewElement("vol");
	volElement->SetText(vol);
	vsUnitElement->InsertEndChild(volElement);
		
	return vsUnitElement;
}

//////////////////////////////////////////////
//VMusicalPart
/////////////////bo/////////////////////////////
int VMusicalPart::loadInfo(XMLElement *musicalTrackElement)
{
	posTick = atoi(musicalTrackElement->FirstChildElement("posTick")->GetText());
	playTime = atoi(musicalTrackElement->FirstChildElement("playTime")->GetText());
	partName = musicalTrackElement->FirstChildElement("partName")->GetText();
	comment = musicalTrackElement->FirstChildElement("comment")->GetText();

	stylePlugin.loadInfo(musicalTrackElement->FirstChildElement("stylePlugin"));
	
	XMLElement *partStyleElement = musicalTrackElement->FirstChildElement("partStyle");
	partStyle.loadInfo(partStyleElement);
	
	XMLElement *singerElement = musicalTrackElement->FirstChildElement("singer");
	singer.index = atoi(singerElement->FirstChildElement("vPC")->GetText());
	singer.language = atoi(singerElement->FirstChildElement("vBS")->GetText());
	
	XMLElement *ctrlElement = musicalTrackElement->FirstChildElement("mCtrl");
	char paraName[][10] = {"DYN","BRE","BRI","CLE","GEN","POR","PIT","PBS"};
	VParameterList* paraList[] = {&dyn,&bre,&bri,&cle,&gen,&por,&pit,&pbs};
	while(ctrlElement != NULL)
	{
		XMLElement *attrElement = ctrlElement->FirstChildElement("attr");
		string str(attrElement->FirstAttribute()->Value());
		int index = -1;
		for(int i=0;i<8;i++)
			if(str.compare(paraName[i]) == 0)
			{
				index = i;
				break;
			}
		if(index == -1)//found a unknown parameter
		{
			printf("Found unknown parameter %s\n",str.c_str());
			continue;
		}
		VParameterList *para = paraList[index];
		para->addParameter(atoi(ctrlElement->FirstChildElement("posTick")->GetText())
				,atoi(attrElement->GetText()));
		ctrlElement = ctrlElement->NextSiblingElement("mCtrl");
	}

	XMLElement *noteElement = musicalTrackElement->FirstChildElement("note");
	while(noteElement != NULL)
	{
		VNote *notePtr = new VNote;
		notePtr->loadInfo(noteElement);
		note.push_back(notePtr);
		noteElement = noteElement->NextSiblingElement("note");
	}
	return 1;
}

XMLElement* VMusicalPart::createXml(XMLDocument *doc)
{
	XMLElement *musicalPartElement = doc->NewElement("musicalPart");

	XMLElement *posTickElement = doc->NewElement("posTick");
	musicalPartElement->InsertEndChild(posTickElement);
	posTickElement->SetText(posTick);

	XMLElement *playTimeElement = doc->NewElement("playTime");
	musicalPartElement->InsertEndChild(playTimeElement);
	playTimeElement->SetText(playTime);

	XMLElement *partNameElement = doc->NewElement("partName");
	musicalPartElement->InsertEndChild(partNameElement);
	XMLText *partNameText = doc->NewText(partName.c_str());
	partNameElement->InsertEndChild(partNameText);
	partNameText->SetCData(true);

	XMLElement *commentElement = doc->NewElement("comment");
	musicalPartElement->InsertEndChild(commentElement);
	XMLText *commentText = doc->NewText(comment.c_str());
	commentElement->InsertEndChild(commentText);
	commentText->SetCData(true);

	XMLElement *stylePluginElement = doc->NewElement("stylePlugin");
	musicalPartElement->InsertEndChild(stylePluginElement);

	XMLElement *stylePluginIdElement = doc->NewElement("stylePluginID");
	stylePluginElement->InsertEndChild(stylePluginIdElement);
	XMLText *stylePluginIdText = doc->NewText(stylePlugin.stylePluginId.c_str());
	stylePluginIdText->SetCData(true);
	stylePluginIdElement->InsertEndChild(stylePluginIdText);

	XMLElement *stylePluginNameElement = doc->NewElement("stylePluginName");
	stylePluginElement->InsertEndChild(stylePluginNameElement);
	XMLText *stylePluginNameText = doc->NewText(stylePlugin.stylePluginName.c_str());
	stylePluginNameText->SetCData(true);
	stylePluginNameElement->InsertEndChild(stylePluginNameText);

	XMLElement *versionElement = doc->NewElement("version");
	stylePluginElement->InsertEndChild(versionElement);
	XMLText *versionText = doc->NewText(stylePlugin.version.c_str());
	versionText->SetCData(true);
	versionElement->InsertEndChild(versionText);

	XMLElement *partStyleElement = doc->NewElement("partStyle");
	musicalPartElement->InsertEndChild(partStyleElement);

	static const char idList[][10] = {"accent","bendDep","bendLen","decay","fallPort","opening","risePort"};
	const int valueList[] = {partStyle.accent,partStyle.bendDep,partStyle.bendLen,partStyle.decay,partStyle.fallPort,
		partStyle.opening,partStyle.risePort};

	for(int i=0;i<7;i++)
	{
		XMLElement *attrElement = doc->NewElement("attr");
		partStyleElement->InsertEndChild(attrElement);
		attrElement->SetAttribute("id",idList[i]);
		attrElement->SetText(valueList[i]);
	}

	XMLElement *singerElement = doc->NewElement("singer");
	musicalPartElement->InsertEndChild(singerElement);

	posTickElement = doc->NewElement("posTick");
	singerElement->InsertEndChild(posTickElement);
	posTickElement->SetText("0");

	XMLElement *vBsElement = doc->NewElement("vBS");
	singerElement->InsertEndChild(vBsElement);
	vBsElement->SetText(singer.language);

	XMLElement *vPcElement = doc->NewElement("vPC");
	singerElement->InsertEndChild(vPcElement);
	vPcElement->SetText(singer.index);

	VParameterList *paraList[] = {&dyn,&bre,&bri,&cle,&gen,&por,&pit,&pbs};
	char paraIdList[][4] = {"DYN","BRE","BRI","CLE","GEN","POR","PIT","PBS"};

	int size = sizeof(paraList)/sizeof(VParameterList*);
	for(int i=0;i<size;i++)
	{
		int length = paraList[i]->getSize();
		for(int w=0;w<length;w++)
		{
			VParameter *para = paraList[i]->value[w];
			XMLElement *ctrlElement = doc->NewElement("mCtrl");
			musicalPartElement->InsertEndChild(ctrlElement);
			XMLElement *posTickElement = doc->NewElement("posTick");
			posTickElement->SetText(para->clock);
			ctrlElement->InsertEndChild(posTickElement);

			XMLElement *attrElement = doc->NewElement("attr");
			attrElement->SetAttribute("id",paraIdList[i]);
			attrElement->SetText(para->value);
			ctrlElement->InsertEndChild(attrElement);
		}
	}

	size = note.size();
	for(int i=0;i<size;i++)
	{
		XMLElement *noteElement = note[i]->createXml(doc);
		musicalPartElement->InsertEndChild(noteElement);
	}

	return musicalPartElement;
}

/////////////////////////////////////////
//VWavTrack
/////////////////////////////////////////

XMLElement* VWavTrack::createXml(XMLDocument *doc, string name)
{
	XMLElement *trackElement = doc->NewElement(name.c_str());
	XMLElement *wavPartElement = doc->NewElement("wavPart");
	trackElement->InsertEndChild(wavPartElement);
	//TODO:wirte something

	return trackElement;
}

VWavTrack** VsqxDoc::getWavTrack()
{
	static VWavTrack **ptr = NULL;
	//The won't update. it's fine to do this
	if(ptr == NULL)
	{
		ptr = new VWavTrack*[2];
		ptr[0] = &wavTrack[0];
		ptr[1] = &wavTrack[1];
	}
	
	return ptr;
}

//////////////////////////////////////////////
//VStylePlugin
//////////////////////////////////////////////
VStylePlugin::VStylePlugin()
{
	//some vocaloid defult
	stylePluginId = "ACA9C502-A04B-42b5-B2EB-5CEA36D16FCE";
	stylePluginName = "VOCALOID2 Compatible Style";
	version = "3.0.0.1";
}

int VStylePlugin::loadInfo(XMLElement *stylePluginElement)
{
	stylePluginId = stylePluginElement->FirstChildElement("stylePluginID")->GetText();
	stylePluginName = stylePluginElement->FirstChildElement("stylePluginName")->GetText();
	version = stylePluginElement->FirstChildElement("version")->GetText();

	return 1;
}

////////////////////////////////////////////////
//VPartStyle
////////////////////////////////////////////////
int VPartStyle::loadInfo(XMLElement *partStyleElement)
{
	XMLElement *element = partStyleElement->FirstChildElement("attr");
	while(element != NULL)
	{
		int value = atoi(element->GetText());
		string str = element->FirstAttribute()->Value();
		//XXX: there must be a better way fo doing this
		if(str.compare("accent") == 0)
			accent = value;
		else if(str.compare("bendDep") == 0)
			bendDep = value;
		else if(str.compare("bendLen") == 0)
			bendLen = value;
		else if(str.compare("decay") == 0)
			decay = value;
		else if(str.compare("fallPort") == 0)
			fallPort = value;
		else if(str.compare("opening") == 0)
			opening = value;
		else if(str.compare("risePort") == 0)
			risePort = value;

		element = element->NextSiblingElement("attr");
	}
}

/////////////////////////////////////////////////
//VNoteStyle
/////////////////////////////////////////////////

int VNoteStyle::loadInfo(XMLElement *noteStyleElement)
{
	static const char idNameList[][15] = {"accent","bendDep","bendLen","decay","fallPort","opening","risePort","vibLen",
		"vibType"};
	int *paraList[] = {&accent,&bendDep,&bendLen,&decay,&fallPort,&opening,&risePort,&vibLen,&vibType};

	XMLElement *attrElement = noteStyleElement->FirstChildElement("attr");
	while(attrElement != NULL)
	{
		string str = attrElement->FirstAttribute()->Value();
		int size = sizeof(paraList)/sizeof(int*);
		int index = 0;
		for(int i=0;i<size;i++)
		{
			if(str.compare(idNameList[i]) == 0)
			{
				index = i;
				break;
			}
		}
		*paraList[index] = atoi(attrElement->GetText());
		attrElement = attrElement->NextSiblingElement("attr");
	}
	return 1;
}

//////////////////////////////////////////////////
//VNote
//////////////////////////////////////////////////

int VNote::loadInfo(XMLElement *noteElement)
{
	posTick = atoi(noteElement->FirstChildElement("posTick")->GetText());
	durTick = atoi(noteElement->FirstChildElement("durTick")->GetText());
	noteNum = atoi(noteElement->FirstChildElement("noteNum")->GetText());
	velocity = atoi(noteElement->FirstChildElement("velocity")->GetText());
	lyric = noteElement->FirstChildElement("lyric")->GetText();
	phnms =  noteElement->FirstChildElement("phnms")->GetText();
	XMLElement *noteStyleElement = noteElement->FirstChildElement("noteStyle");
	if(noteStyleElement != NULL)
		noteStyle.loadInfo(noteStyleElement);

	XMLElement *seqAttrElement = noteStyleElement->FirstChildElement("seqAttr");
	while(seqAttrElement != NULL)
	{
		string str(seqAttrElement->FirstAttribute()->Value());
		XMLElement *elemElement = seqAttrElement->FirstChildElement("elem");
		VParameterList *para = NULL;
		if(str.compare("vibDep") == 0)
			para = &vibDep;
		else if(str.compare("vibRate") == 0)
			para = &vibRate;
		para->setName(str.c_str());
		while(elemElement != NULL && para != NULL)
		{
			int clock = atoi(elemElement->FirstChildElement("posNrm")->GetText());
			int elv = atoi(elemElement->FirstChildElement("elv")->GetText());//Why they name it elv?
			para->addParameter(clock,elv);
			elemElement = elemElement->NextSiblingElement("elem");
		}
		seqAttrElement = seqAttrElement->NextSiblingElement("seqAttr");
	}
	return 1;
}

XMLElement* VNote::createXml(XMLDocument* doc)
{
	XMLElement *noteElement = doc->NewElement("note");

	XMLElement *posTickElement = doc->NewElement("posTick");
	noteElement->InsertEndChild(posTickElement);
	posTickElement->SetText(posTick);

	XMLElement *durTickElement = doc->NewElement("durTick");
	noteElement->InsertEndChild(durTickElement);
	durTickElement->SetText(durTick);

	XMLElement *noteNumElement = doc->NewElement("noteNum");
	noteElement->InsertEndChild(noteNumElement);
	noteNumElement->SetText(noteNum);

	XMLElement *velocityElement = doc->NewElement("velocity");
	noteElement->InsertEndChild(velocityElement);
	velocityElement->SetText(velocity);

	XMLElement *lyricElement = doc->NewElement("lyric");
	noteElement->InsertEndChild(lyricElement);
	XMLText *lyricText = doc->NewText(lyric.c_str());
	lyricElement->InsertEndChild(lyricText);
	lyricText->SetCData(true);

	XMLElement *phnmsElement = doc->NewElement("phnms");
	noteElement->InsertEndChild(phnmsElement);
	XMLText *phnmsText = doc->NewText(phnms.c_str());
	phnmsElement->InsertEndChild(phnmsText);
	phnmsText->SetCData(true);

	XMLElement *noteStyleElement = doc->NewElement("noteStyle");
	noteElement->InsertEndChild(noteStyleElement);
	static const char idNameList[][15] = {"accent","bendDep","bendLen","decay","fallPort","opening","risePort","vibLen",
		"vibType"};
	int paraList[] = {noteStyle.accent,noteStyle.bendDep,noteStyle.bendLen,noteStyle.decay,noteStyle.fallPort
		,noteStyle.opening,noteStyle.risePort,noteStyle.vibLen,noteStyle.vibType};
	int size = sizeof(paraList)/sizeof(int);
	for(int i=0;i<size;i++)
	{
		XMLElement *attrElement = doc->NewElement("attr");
		noteStyleElement->InsertEndChild(attrElement);
		attrElement->SetAttribute("id",idNameList[i]);
		attrElement->SetText(paraList[i]);
	}

	size = vibDep.getSize();
	if(size != 0)
	{
		XMLElement *seqAttrElement = doc->NewElement("seqAttr");
		noteStyleElement->InsertEndChild(seqAttrElement);
		seqAttrElement->SetAttribute("id","vibDep");
		for(int i=0;i<size;i++)
		{
			XMLElement *elemElement = doc->NewElement("elem");
			seqAttrElement->InsertEndChild(elemElement);

			XMLElement *posNrmElement = doc->NewElement("posNrm");
			elemElement->InsertEndChild(posNrmElement);
			posNrmElement->SetText(vibDep.value[i]->clock);

			XMLElement *elvElement = doc->NewElement("elv");
			elemElement->InsertEndChild(elvElement);
			elvElement->SetText(vibDep.value[i]->value);
		}
	}

	size = vibRate.getSize();
	if(size != 0)
	{
		XMLElement *seqAttrElement = doc->NewElement("seqAttr");
		noteStyleElement->InsertEndChild(seqAttrElement);
		seqAttrElement->SetAttribute("id","vibRate");
		for(int i=0;i<size;i++)
		{
			XMLElement *elemElement = doc->NewElement("elem");
			seqAttrElement->InsertEndChild(elemElement);

			XMLElement *posNrmElement = doc->NewElement("posNrm");
			elemElement->InsertEndChild(posNrmElement);
			posNrmElement->SetText(vibRate.value[i]->clock);

			XMLElement *elvElement = doc->NewElement("elv");
			elemElement->InsertEndChild(elvElement);
			elvElement->SetText(vibRate.value[i]->value);
		}
	}

	return noteElement;
}

const char* VNote::getNoteName()
{
	static const char noteName[][5]={"C","C#","D","Eb","E","F","F#","G","G#","A","Bb","B"};
	int index = noteNum % 12;
	int level = noteNum/12 - 2;
	static char *str = NULL;

	if(str != NULL)
		delete [] str;
	str = new char [10];
	sprintf(str,"%s%d",noteName[index],level);

	return (const char*)str;
}
/////////////////////////////////////////////////
//VWavPart
/////////////////////////////////////////////////
int VWavTrack::loadInfo(XMLElement *wavTrackElement)
{
	XMLElement *wavPartElement = wavTrackElement->FirstChildElement("wavPart");
	while(wavPartElement != NULL)
	{
		VWavPart *part = new VWavPart;
		wavPart.push_back(part);
		part->loadInfo(wavPartElement);
		
		wavPartElement = wavPartElement->NextSiblingElement("wavPart");
	}
}
/////////////////////////////////////////////////
//VWavPart
/////////////////////////////////////////////////
int VWavPart::loadInfo(XMLElement *wavPartElement)
{
	posTick = atoi(wavPartElement->FirstChildElement("posTick")->GetText());
	playTime = atoi(wavPartElement->FirstChildElement("playTime")->GetText());
	partName = wavPartElement->FirstChildElement("partName")->GetText();
	comment = wavPartElement->FirstChildElement("comment")->GetText();
	sampleRate = atoi(wavPartElement->FirstChildElement("sampleRate")->GetText());
	sampleReso = atoi(wavPartElement->FirstChildElement("sampleReso")->GetText());
	channels =  atoi(wavPartElement->FirstChildElement("channels")->GetText());
	filePath = wavPartElement->FirstChildElement("filePath")->GetText();
}

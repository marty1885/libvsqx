#include <vsqx.h>
#include <iostream>
#include <stdlib.h>

using namespace std;
int main()
{
	string path("./test.vsqx");

	VsqxDoc vsqxFile;
	vsqxFile.setPath(path);
	cout << "libVsqx test1." << endl;

	if(vsqxFile.isVsqx() == false)
		cout << "Error :" << path << " is NOT a vsqx file" << endl;
	cout << path << " is a vsqx file" << endl;

	if(vsqxFile.load() == 0)//fail
	{
		char* str = new char[200];
		vsqxFile.getError(str,199,false);
		cout << str << endl;
		exit(0);
	}
	cout << "VSQx File successfuly loaded" << endl;

	cout << endl << "VSQx info:" << endl
		<< "Vender:\t\t" << vsqxFile.getInfo()->getVender() << endl
		<< "version:\t" << vsqxFile.getInfo()->getVersion() << endl;

	cout << endl << "Total " << vsqxFile.getVoiceInfoNum() << " voice info loaded" << endl;
	for(int i=0;i<vsqxFile.getVoiceInfoNum();i++)
	{
		cout << "Voice " << i << endl
			<< "\tLanguage:\t" << vsqxFile.getVoiceInfo()[i]->getLanguageString() << endl
			<< "\tindex:\t\t" << vsqxFile.getVoiceInfo()[i]->index << endl
			<< "\tname\t\t" << vsqxFile.getVoiceInfo()[i]->voiceName << endl
			<< "\tcompId:\t\t" << vsqxFile.getVoiceInfo()[i]->compId << endl
			<< "\tVoiceParameter:\t" << "BRI : " << vsqxFile.getVoiceInfo()[i]->voiceParameter.bri
			<< " BRE : " << vsqxFile.getVoiceInfo()[i]->voiceParameter.bre
			<< " CLE : " << vsqxFile.getVoiceInfo()[i]->voiceParameter.cle
			<< " GEN : " << vsqxFile.getVoiceInfo()[i]->voiceParameter.gen 
			<< " OPE : " << vsqxFile.getVoiceInfo()[i]->voiceParameter.ope << endl;
	}

	cout << endl << "Mixer info:" << endl
		<< "\tMaster unit :" << endl
		<< "\t\tOutput Device : " << vsqxFile.getMixer()->masterUnit.outDev << endl;

	for(int i=0;i<3;i++)
	{
		if(strcmp(vsqxFile.getMixer()->masterUnit.vstPlugin[i].id.c_str(),"    ") == 0)//no VST
			cout << "\t\tNo VST in mster unit slot " << i << endl;
		else
		{
			cout << "\t\tSlot " << i << endl
				<<"\t\t\t Id :\t" << vsqxFile.getMixer()->masterUnit.vstPlugin[i].id.c_str() << endl
				<<"\t\t\t name :\t" << vsqxFile.getMixer()->masterUnit.vstPlugin[i].name.c_str() << endl;
		}
	}
	cout << "\tvsUnit :" << endl << "\t\tFound " << vsqxFile.getMixer()->getVsUnitNum() << " tracks" << endl;
	cout << "\tseUnit :" << endl << "\t\tInput Gain : " << vsqxFile.getMixer()->seUnit->inGain << endl;
	cout << "\tkaraokeUnit :" << endl << "\t\tInput Gain : " << vsqxFile.getMixer()->karaokeUnit->inGain << endl;

	cout << endl << "Master track Info:" << endl
		<< "Name :\t\t" << vsqxFile.getMasterTrack()->name << endl
		<< "Comment:\t" << vsqxFile.getMasterTrack()->comment << endl
		<< "Resolution:\t" << vsqxFile.getMasterTrack()->resolution << endl
		<< "preMeasure:\t" << vsqxFile.getMasterTrack()->preMeasure << endl;

	cout << "Counted " << vsqxFile.getMasterTrack()->getTimeSignatureNum() << " time signatures and "
		<< vsqxFile.getMasterTrack()->getTempoNum() << " tempo." << endl;

	return 1;
}

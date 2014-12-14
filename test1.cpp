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
}

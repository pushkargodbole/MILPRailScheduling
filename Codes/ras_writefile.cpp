#include <iostream>
#include <string.h>
#include <vector>
#include <fstream>

using namespace std;

int writefile(string filename_str, string territory, vector<train_datatype> trains, vector<path_datatype> paths, vector<vector<float> > times)
{
	char * filename;
	filename = new char [filename_str.length()+1];
	strcpy(filename,filename_str.c_str());
	ofstream myfile;
	myfile.open(filename);
	if(myfile.is_open())
	{
		myfile << "########################################################################### \n";
		myfile << "<solution territory='" << territory << "'>\n";
		myfile << "\t<trains>\n";
		for(int i=0;i< trains.size();i++)
		{
			myfile << "\t\t<train id='" <<  trains[i].header << "'>\n";
			myfile << "\t\t\t<movements>\n";
			for(int j=0;j<paths[i].path.size();j++) myfile << "\t\t\t\t<movement arc='(" << paths[i].path[j].node1 << "," << paths[i].path[j].node2 << ")' entry='" << times[i][j] << "' exit='" << times[i][j+1] << "'/>\n";
			myfile << "\t\t\t\t<destination entry='" << times[i][times[i].size()-1] << "'/>\n";
			myfile << "\t\t\t</movements>\n";
			myfile << "\t\t</train>\n";
		}
		myfile << "\t</trains>\n";
		myfile << "</solution>\n";
		myfile << "###########################################################################";
		myfile.close();
		return 0;
	}
	else
	{
		cout << "Unable to create/edit output file" << endl;
		return 1;
	}
}

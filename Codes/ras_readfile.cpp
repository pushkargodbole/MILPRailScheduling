#include <iostream>
#include <fstream>
#include <string.h>
#include <sstream>
#include <vector>

using namespace std;
		
class readfile
{
	public:
		char * filename;
		string territory;
		float west2east;
		float east2west;
		float sidingspeed;
		float crossoverspeed;
		int mow_count;
		vector<mow_datatype> mow;
		int arc_count;
		int maintrack_count;
		vector<arc_datatype> arc;
		int train_count;
		vector<train_datatype> train;
		readfile(string);
		int getdata();
};

readfile::readfile(string filename_str)
{
	filename = new char [filename_str.length()+1];
	strcpy(filename,filename_str.c_str());
}

int readfile::getdata()
{
	ifstream myfile;
    myfile.open(filename);
    if (myfile.is_open())
    {
		string line;
		int i = 0;
		int train_no = 0;
		int j;
		size_t found;
		size_t colon;
		string track_field[] = {"TERRITORY","WEST -> EAST","EAST -> WEST","MAXIMUM SPEED on SIDINGS","MAXIMUM SPEED on CROSSOVERS","MOW","ARC ID","TRACKTYPE","LENGTH","TRAINS"};
		string train_field[] = {"HEADER","ENTRY TIME","ORIGIN NODE","DESTINATION NODE","DIRECTION","SPEED MULTIPLIER","TRAIN LENGTH","TOB","HAZMAT","SA STATUS","SCHEDULED ARRIVAL","TERMINAL WANT TIME"};
		mow_count = 0;
		arc_count = 0;
		int arc_no;
		while(i < sizeof(track_field)/sizeof(track_field[0]) && !myfile.eof())
		{
			getline(myfile,line);
			found = line.find(track_field[i]);
			if(found!=string::npos)
			{
				colon = line.find_first_of(":");
				if(colon!=string::npos) line.erase(0,colon+1);
				switch(i)
				{
					case 0: //TERRITORY
					{
						territory = stringstrip(line);
						break;
					}
					case 1: //WEST TO EAST SPEED
					{
						line = rm_mph(line);
						line = stringstrip(line);
						stringstream(line) >> west2east;
						break;
					}
					case 2: //EAST TO WEST SPEED
					{
						line = rm_mph(line);
						line = stringstrip(line);
						stringstream(line) >> east2west;
						break;
					}
					case 3: //SIDING SPEED
					{
						line = rm_mph(line);
						line = stringstrip(line);
						stringstream(line) >> sidingspeed;
						break;
					}
					case 4: //CROSSOVER SPEED
					{
						line = rm_mph(line);
						line = stringstrip(line);
						stringstream(line) >> crossoverspeed;
						break;
					}
					case 5: //MOW
					{
						while(found!=string::npos)
						{
							mow_datatype newmow;
							mow.push_back(newmow);
							string mow_str[4];
							int mownodes_temp[2];
							float mowtimes_temp[2];
							size_t mow_posstart,mow_posend,mow_poslen;
							for(int mow_i=0;mow_i<4;mow_i++)
							{
								mow_posstart = line.find_first_of("0123456789");
								line.erase(0,mow_posstart);
								mow_posstart = line.find_first_of("0123456789");
								mow_posend = line.find_first_not_of("0123456789");
								mow_poslen = mow_posend-mow_posstart;
								mow_str[mow_i] = line.substr(mow_posstart,mow_poslen);
								if(mow_i<2) stringstream(mow_str[mow_i]) >> mownodes_temp[mow_i];
								else stringstream(mow_str[mow_i]) >> mowtimes_temp[mow_i-2];
								line.erase(0,mow_posend);
							}
							mow[mow_count].startnode = mownodes_temp[0];
							mow[mow_count].endnode = mownodes_temp[1];
							mow[mow_count].starttime = mowtimes_temp[0];
							mow[mow_count].endtime = mowtimes_temp[1];
							int pos = myfile.tellg();
							getline(myfile,line);
							found = line.find("between nodes");
							if(found==string::npos) myfile.seekg(pos);
							mow_count++;
						}
						break;
					}
					case 6: //ARC IDs
					{
						string arcnode1_str,arcnode2_str;
						size_t arc_posstart,arc_posend,arc_poslen;
						arc_posstart = line.find_first_of("0123456789");
						while(arc_posstart!=string::npos)
						{
							arc_datatype newarc;
							arc.push_back(newarc);
							line.erase(0,arc_posstart);
							arc_posstart = line.find_first_of("0123456789");
							arc_posend = line.find_first_not_of("0123456789");
							arc_poslen = arc_posend - arc_posstart;
							arcnode1_str = line.substr(arc_posstart,arc_poslen);
							stringstream(arcnode1_str) >> arc[arc_count].node1;
							line.erase(0,arc_posend);
							arc_posstart = line.find_first_of("0123456789");
							line.erase(0,arc_posstart);
							arc_posstart = line.find_first_of("0123456789");
							arc_posend = line.find_first_not_of("0123456789");
							arc_poslen = arc_posend - arc_posstart;
							arcnode2_str = line.substr(arc_posstart,arc_poslen);
							stringstream(arcnode2_str) >> arc[arc_count].node2;
							line.erase(0,arc_posend);
							arc_posstart = line.find_first_of("0123456789");
							arc_count++;
						}
						break;
					}
					case 7: //ARC TRACKTYPES
					{
						maintrack_count = 0;
						arc_no = 0;
						string tracktype_str;
						size_t tracktype_posstart,tracktype_posend,tracktype_poslen,is_main;
						int maintrack_temp;
						tracktype_posstart = line.find_first_of("0123456789SC");
						while(tracktype_posstart!=string::npos)
						{
							line.erase(0,tracktype_posstart);
							tracktype_posstart = line.find_first_of("0123456789SC");
							tracktype_posend = line.find_first_not_of("0123456789SCW");
							tracktype_poslen = tracktype_posend - tracktype_posstart;
							tracktype_str = line.substr(tracktype_posstart,tracktype_poslen);
							is_main = tracktype_str.find_first_of("0123456789");
							if(is_main!=string::npos)
							{
								stringstream(tracktype_str) >> maintrack_temp;
								if(maintrack_temp+1 > maintrack_count) maintrack_count = maintrack_temp+1;
							}
							arc[arc_no].tracktype = tracktype_str;
							line.erase(0,tracktype_posend);
							tracktype_posstart = line.find_first_of("0123456789SC");
							arc_no++;
						}
						break;
					}
					case 8: //ARC LENGTHS
					{
						arc_no = 0;
						string tracklen_str;
						size_t tracklen_posstart,tracklen_posend,tracklen_poslen;
						tracklen_posstart = line.find_first_of("0123456789.");
						while(tracklen_posstart!=string::npos)
						{
							line.erase(0,tracklen_posstart);
							tracklen_posstart = line.find_first_of("0123456789.");
							tracklen_posend = line.find_first_not_of("0123456789.");
							tracklen_poslen = tracklen_posend - tracklen_posstart;
							tracklen_str = line.substr(tracklen_posstart,tracklen_poslen);
							stringstream(tracklen_str) >> arc[arc_no].tracklen;
							line.erase(0,tracklen_posend);
							tracklen_posstart = line.find_first_of("0123456789.");
							arc_no++;
						}
						break;
					}
					case 9: //TOTAL No. OF TRAINS
					{
						line = stringstrip(line);
						stringstream(line) >> train_count;
						break;
					}
				}
				i++;
			}
		}
		while(train_no<train_count)
		{
			train_datatype newtrain;
			train.push_back(newtrain);
			j = 0;
			while(j<sizeof(train_field)/sizeof(train_field[0]) && !myfile.eof())
			{
				getline(myfile,line);
				found = line.find(train_field[j]);
				if(found!=string::npos)
				{
					colon = line.find_first_of(":");
					if(colon!=string::npos) line.erase(0,colon+1);
					line = stringstrip(line);
					switch(j)
					{
						case 0: //TRAIN HEADER
						{
							train[train_no].header = line;
							break;
						}
						case 1: //TRAIN ENTRY TIME
						{
							stringstream(line) >> train[train_no].entry;
							break;
						}
						case 2: //TRAIN ORIGIN NODE
						{
							stringstream(line) >> train[train_no].origin;
							break;
						}
						case 3: //TRAIN DESTINATION NODE
						{
							stringstream(line) >> train[train_no].destination;
							break;
						}
						case 4: //TRAIN DIRECTION (EASTBOUND/WESTBOUND)
						{
							train[train_no].direction = line;
							break;
						}
						case 5: //TRAIN SPEED MULTIPLIER
						{
							stringstream(line) >> train[train_no].speed_m;
							break;
						}
						case 6: //TRAIN LENGTH
						{
							stringstream(line) >> train[train_no].trainlen;
							break;
						}
						case 7: //TRAIN TOB
						{
							stringstream(line) >> train[train_no].tob;
							break;
						}
						case 8: //TRAIN HAZMAT STATUS (YES/NO)
						{
							colon = line.find_first_of(":");
							if(colon!=string::npos) line.erase(0,colon+1);
							line = stringstrip(line);
							train[train_no].hazmat = line;
							break;
						}
						case 9: //SA STATUS AT ORIGIN
						{
							stringstream(line) >> train[train_no].sa_status_atorig;
							break;
						}
						case 10: //TRAIN SCHEDULE ADDHERENCE CONDITIONS
						{
							colon = line.find_first_of(":");
							if(colon!=string::npos) line.erase(0,colon+1);
							sch_arr_datatype new_sch_arr;
							string sch_arr_str;
							train[train_no].sch_arr_count = 0;
							size_t sch_arr_posstart,sch_arr_posend,sch_arr_poslen;
							sch_arr_posstart = line.find_first_of("0123456789");
							while(sch_arr_posstart!=string::npos)
							{
								train[train_no].sch_arr.push_back(new_sch_arr);
								line.erase(0,sch_arr_posstart);
								sch_arr_posstart = line.find_first_of("0123456789");
								sch_arr_posend = line.find_first_not_of("0123456789");
								sch_arr_poslen = sch_arr_posend - sch_arr_posstart;
								sch_arr_str = line.substr(sch_arr_posstart,sch_arr_poslen);
								stringstream(sch_arr_str) >> train[train_no].sch_arr[train[train_no].sch_arr_count].node;
								line.erase(0,sch_arr_posend);
								sch_arr_posstart = line.find_first_of("0123456789.-");
								line.erase(0,sch_arr_posstart);
								sch_arr_posstart = line.find_first_of("0123456789.-");
								sch_arr_posend = line.find_first_not_of("0123456789.-");
								sch_arr_poslen = sch_arr_posend - sch_arr_posstart;
								sch_arr_str = line.substr(sch_arr_posstart,sch_arr_poslen);
								stringstream(sch_arr_str) >> train[train_no].sch_arr[train[train_no].sch_arr_count].time;
								line.erase(0,sch_arr_posend);
								sch_arr_posstart = line.find_first_of("0123456789");
								train[train_no].sch_arr_count++;
							}
							break;	
						}
						case 11: //TRAIN TERMINAL WANT TIME
						{
							colon = line.find_first_of(":");
							while(colon!=string::npos)
							{
								line.erase(0,colon+1);
								colon = line.find_first_of(":");
							}
							line = stringstrip(line);
							stringstream(line) >> train[train_no].twt;
							break;
						}
					}
					j++;
				}
			}
			train_no++;
		}
		if(i == sizeof(track_field)/sizeof(track_field[0]) && train_no == train_count)
		{
			
			return 0;
		}
		else
		{
			cout << "ERROR!" << endl;
			cout << "i = " << i << endl;
			cout << "train_no = " << train_no << endl;
			return 1;
		}
	}  
	else
	{
		cout << "No file named " << "\'" << filename << "\'" << " in the directory."<< endl;
		return 1;
	}
}


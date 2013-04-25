#include <iostream>
#include <string.h>
#include <vector>

using namespace std;

struct mow_datatype
{
	public:
		int startnode;
		int endnode;
		float starttime;
		float endtime;
};

struct arc_datatype
{
	public:
		int node1;
		int node2;
		string tracktype;
		float tracklen;
		float req_time;
};

struct sch_arr_datatype
{
	public:
		int node;
		float time;
};

struct train_datatype
{
	public:
		string header;
		float entry;
		int origin;
		int destination;
		string direction;
		float speed_m;
		float trainlen;
		float tob;
		string hazmat;
		float sa_status_atorig;
		int sch_arr_count;
		vector<sch_arr_datatype> sch_arr;
		float twt;
		
};

struct path_datatype
{
	public:
		vector<arc_datatype> path;
		string pathid;
};

#include <iostream>
#include <vector>
#include <sstream>

using namespace std;

vector<path_datatype> getallpaths(int start, int end, vector<vector<arc_datatype> > connects)
{
	vector<path_datatype> allpaths;
	vector<vector<arc_datatype> > splitpaths;
	vector<int> splitnodes;
	vector<arc_datatype> open;
	vector<arc_datatype> path;
	int curr_node;
	do
	{
		if(open.size()!=0)
		{
			path.clear();
			start = open[open.size()-1].node2;
			while(open[open.size()-1].node1 != splitnodes[splitnodes.size()-1])
			{
				splitnodes.pop_back();
				splitpaths.pop_back();
			}
			path = splitpaths[splitpaths.size()-1];
			path.push_back(open[open.size()-1]);
			open.pop_back();
		}
		curr_node = start;
		while(connects[curr_node].size()!=0 && curr_node!=end)
		{
			for(int i=0;i<connects[curr_node].size();i++) open.push_back(connects[curr_node][i]);
			if(connects[curr_node].size()>1)
			{
				splitnodes.push_back(curr_node);
				splitpaths.push_back(path);
			}
			curr_node = open[open.size()-1].node2;
			path.push_back(open[open.size()-1]);
			open.pop_back();
		}
		if(path.size()!=0)
		{
			if(path[path.size()-1].node2 == end)
			{
				path_datatype newpath;
				newpath.path = path;
				string path_id = "id#";
				for(int k=0;k<path.size();k++)
				{
					if(path[k].tracktype=="S")
					{
						path_id = path_id + "S";
						stringstream str1;
						str1 << path[k].node1;
						str1 << "-";
						str1 << path[k].node2;
						str1 << ":";
						path_id = path_id + str1.str();
					}
				}
				for(int j=0;j<path.size();j++)
				{
					if(path[j].tracktype=="C")
					{
						stringstream str2;
						str2 << path[j].node1;
						str2 << "-";
						str2 << path[j].node2;
						str2 << ":";
						path_id = path_id + str2.str();
					}
				}
				newpath.pathid = path_id;
				allpaths.push_back(newpath);
			}
		}
	}while(open.size()!=0);
	return allpaths;
}


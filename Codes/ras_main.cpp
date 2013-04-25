#include <iostream>
#include <stdlib.h>
#include<time.h>
#include "ras_stringfuncs.cpp"
#include "mergesort_vec_lookup.cpp"
#include "ras_datatypes.cpp"
#include "ras_readfile.cpp"
#include "ras_allpaths.cpp"
#include "ras_solve.cpp"
#include "ras_writefile.cpp"

using namespace std;

int main()
{
	// GET DATA FROM FILE ******************************************************************************************************************
	
	int status;
	string file_name;
	cout << endl <<"Team : Xyon" << endl;
	cout << "Team members : Pushkar Godbole" << endl << endl;
	cout << "Enter name of the input file : ";
	cin >> file_name;
	cout << endl;
	float hor_mins;
	cout << "Enter planning horizon in minutes : ";
	cin >> hor_mins;
	cout << endl;
	double horizon = hor_mins*60.0;
	readfile file (file_name);
	status = file.getdata();
	if(status==0)
	{
		cout << file.territory << endl << endl;
		
	// FIND CONNECTS ***********************************************************************************************************************
	
		vector<int> arcs_node1;
		for(int a=0;a<file.arc_count;a++) arcs_node1.push_back(file.arc[a].node1);
		file.arc = mergesort_lookup(arcs_node1,file.arc);
		arcs_node1.clear();
		vector<vector<arc_datatype> > node_connect;
		int b = 0;
		int fwsplit_count;
		vector<int> fwsplits;
		vector<int> node2_done;
		int node2_found;
		vector<int> bwsplits;
		for(int node_no=0;node_no<file.arc_count;node_no++)
		{
			vector<arc_datatype> newnode;
			node_connect.push_back(newnode);
			fwsplit_count = 0;
			while(file.arc[b].node1 == node_no)
			{
				node_connect[node_no].push_back(file.arc[b]);
				fwsplit_count++;
				node2_found = 0;
				for(int a1=0;a1<node2_done.size();a1++) if(file.arc[b].node2==node2_done[a1]) node2_found = 1;
				if(node2_found==1) bwsplits.push_back(file.arc[b].node2);
				else node2_done.push_back(file.arc[b].node2);
				b++;
			}
			if(fwsplit_count>1) fwsplits.push_back(file.arc[b-1].node1);
		}
	
	// FIND END NODES **********************************************************************************************************************
		int terminal1 = file.arc[0].node1;
		int terminal2;
		vector<vector<int> > end_nodes;
		int newpair = 1;
		int d = 0;
		for(int c=0;c<file.train_count;c++)
		{
			for(d=0;d<end_nodes.size();d++) if(file.train[c].origin == end_nodes[d][0] && file.train[c].destination == end_nodes[d][1]) newpair = 0;
			if(newpair == 1)
			{
				if(file.train[c].destination != terminal1) terminal2 = file.train[c].destination;
				vector<int> newpair_temp;
				end_nodes.push_back(newpair_temp);
				end_nodes[d].push_back(file.train[c].origin);
				end_nodes[d].push_back(file.train[c].destination);
			}
			newpair = 1;
		}
	
	// FIND REQUIRED PATHS ****************************************************************************************************************
		
		vector<vector<path_datatype> > req_paths;
		for(int e=0;e<end_nodes.size();e++)
		{
			vector<path_datatype> paths_temp;
			paths_temp = getallpaths(end_nodes[e][0],end_nodes[e][1],node_connect);
			if(paths_temp.size()==0)
			{
				paths_temp = getallpaths(end_nodes[e][1],end_nodes[e][0],node_connect);
				for(int f=0;f<paths_temp.size();f++)
				{
					for(int g=0;g<paths_temp[f].path.size()/2;g++)
					{
						arc_datatype swaparc = paths_temp[f].path[g];
						paths_temp[f].path[g].node1 = paths_temp[f].path[paths_temp[f].path.size()-g-1].node2;
						paths_temp[f].path[g].node2 = paths_temp[f].path[paths_temp[f].path.size()-g-1].node1;
						paths_temp[f].path[g].tracktype = paths_temp[f].path[paths_temp[f].path.size()-g-1].tracktype;
						paths_temp[f].path[g].tracklen = paths_temp[f].path[paths_temp[f].path.size()-g-1].tracklen;
						paths_temp[f].path[paths_temp[f].path.size()-g-1].node1 = swaparc.node2;
						paths_temp[f].path[paths_temp[f].path.size()-g-1].node2 = swaparc.node1;
						paths_temp[f].path[paths_temp[f].path.size()-g-1].tracktype = swaparc.tracktype;
						paths_temp[f].path[paths_temp[f].path.size()-g-1].tracklen = swaparc.tracklen;
					}
					if(paths_temp[f].path.size()%2!=0)
					{
						int swapnode = paths_temp[f].path[paths_temp[f].path.size()/2].node1;
						paths_temp[f].path[paths_temp[f].path.size()/2].node1 = paths_temp[f].path[paths_temp[f].path.size()/2].node2;
						paths_temp[f].path[paths_temp[f].path.size()/2].node2 = swapnode;
					}
				}
			}
			for(int b1=0;b1<paths_temp.size();b1++)
			{
				int sidingcount = 0;
				for(int c1=0;c1<paths_temp[b1].path.size();c1++)
				{
					if(paths_temp[b1].path[c1].tracktype=="S") sidingcount++;
					if(sidingcount>1)
					{
						paths_temp.erase(paths_temp.begin()+b1);
						b1--;
						break;
					}
				}
			}
			req_paths.push_back(paths_temp);
			paths_temp.clear();
		}
	
	// INSERT PATHS TRAINSWISE ********************************************************************************************************
	
		vector<vector<path_datatype> > train_paths;
		for(int g=0;g<file.train_count;g++)
		{
			int h;
			for(h=0;h<end_nodes.size();h++) if(file.train[g].origin==end_nodes[h][0] && file.train[g].destination==end_nodes[h][1]) break;
			train_paths.push_back(req_paths[h]);
			float trainspeed;
			if(file.train[g].direction=="EASTBOUND") trainspeed = file.west2east*file.train[g].speed_m;
			else trainspeed = file.east2west*file.train[g].speed_m;
			for(int i=0;i<train_paths[g].size();i++)
			{
				for(int j=0;j<train_paths[g][i].path.size();j++)
				{
					if(train_paths[g][i].path[j].tracktype=="S") train_paths[g][i].path[j].req_time = train_paths[g][i].path[j].tracklen/file.sidingspeed*3600;
					else if(train_paths[g][i].path[j].tracktype=="SW" || train_paths[g][i].path[j].tracktype=="C") train_paths[g][i].path[j].req_time = train_paths[g][i].path[j].tracklen/file.crossoverspeed*3600;
					else train_paths[g][i].path[j].req_time = train_paths[g][i].path[j].tracklen/trainspeed*3600;
				}
			}
		}
		req_paths.clear();
		end_nodes.clear();
		
	// ELIMINATE HAZMAT & LONG TRAIN CONFLICTING PATHS *************************************************************************************
	
		for(int k=0;k<file.train_count;k++)
		{
			for(int l=0;l<train_paths[k].size();l++)
			{
				for(int m=0;m<train_paths[k][l].path.size();m++)
				{
					if(train_paths[k][l].path[m].tracktype=="S")
					{
						if(file.train[k].hazmat=="YES" || file.train[k].trainlen > train_paths[k][l].path[m].tracklen)
						{
							train_paths[k].erase(train_paths[k].begin()+l);
							l--;
							break;
						}
					}
				}
			}	
		}
	
	// ELIMINATE PATHS MISSING SCHEDULED ARRIVAL NODES ************************************************************************************
	
		for(int n=0;n<file.train_count;n++)
		{
			for(int o=0;o<train_paths[n].size();o++)
			{
				int arrival_count = 0;
				for(int p=0;p<train_paths[n][o].path.size();p++) for(int q=0;q<file.train[n].sch_arr_count;q++) if(train_paths[n][o].path[p].node2==file.train[n].sch_arr[q].node) arrival_count++;
				if(arrival_count!=file.train[n].sch_arr_count)
				{
					train_paths[n].erase(train_paths[n].begin()+o);
					o--;
				}
			}
		}
		vector<int> priority;
		for(int p=0;p<train_paths.size();p++)
		{
			for(int q=0;q<train_paths[p].size();q++)
			{
				int priority_temp = 0;
				int utu_flag = 0;
				for(int r=0;r<train_paths[p][q].path.size();r++)
				{
					if(train_paths[p][q].path[r].tracktype=="S" || train_paths[p][q].path[r].tracktype=="C") priority_temp++;
					if(file.train[p].direction=="EASTBOUND" && train_paths[p][q].path[r].tracktype=="1" || file.train[p].direction=="WESTBOUND" && train_paths[p][q].path[r].tracktype=="2") utu_flag = 1;
				}
				if(utu_flag==1) priority_temp = priority_temp + 100000;
				priority.push_back(priority_temp);
			}
			train_paths[p] = mergesort_lookup(priority, train_paths[p]);
			priority.clear();
		}
		
		vector<int> traintypes;
		int M = 100000;
		for(int d1=0;d1<file.train_count;d1++)
		{
			if(file.train[d1].tob > 100) traintypes.push_back(file.train[d1].entry);
			else
			{
				if(file.train[d1].header[0]=='A') traintypes.push_back(M + file.train[d1].entry);
				else if(file.train[d1].header[0]=='B') traintypes.push_back(2*M + file.train[d1].entry);
				else if(file.train[d1].header[0]=='C') traintypes.push_back(3*M + file.train[d1].entry);
				else if(file.train[d1].header[0]=='D') traintypes.push_back(4*M + file.train[d1].entry);
				else if(file.train[d1].header[0]=='E') traintypes.push_back(5*M + file.train[d1].entry);
				else if(file.train[d1].header[0]=='F') traintypes.push_back(6*M + file.train[d1].entry);
			}
		}
		file.train = mergesort_lookup(traintypes, file.train);
		train_paths = mergesort_lookup(traintypes, train_paths);
		traintypes = mergesort_lookup(traintypes, traintypes);
		int chunk;
		int l1;
		if(file.train_count<=12) chunk = 12;
		else
		{
			for(l1=0;l1<traintypes.size();l1++) if(traintypes[l1]>=500000) break;
			if(l1<=12) chunk = 12;
			else if(l1<=15) chunk = l1;
			else chunk = 15;
		}
		cout << "Chunk size = " << chunk << endl;
		int subproblems;
		if(file.train_count%chunk==0) subproblems = file.train_count/chunk;
		else subproblems = file.train_count/chunk+1;
		cout << "Number of subproblems = " << subproblems << endl << endl;
		float g_starttime = 0.0;
		float g_starttime_tmp = 0.0;
		float cost;
		vector<vector<float> > times;
		vector<vector<float> > delay;
		vector<path_datatype> chosenpaths_fin;
		float cost_fin = 0.0;
		for(int j1=0;j1<file.train_count/(chunk+1)+1;j1++)
		{
			float sub_cost = 0.0;
			float cost_temp;
			vector<vector<float> > times_temp;
			vector<vector<float> > delay_temp;
			vector<path_datatype> chosenpaths_temp;
			vector<path_datatype> chosenpaths;
			vector<string> takenpaths;
			vector<train_datatype> trains;
			int traincount = ((j1+1)*chunk<file.train_count)?(j1+1)*chunk:file.train_count;
			for(int e1=j1*chunk;e1<traincount;e1++)
			{
				int f1 = 0;
				int path_taken;
				do
				{
					if(file.train[e1].tob > 100)
					{
						chosenpaths.push_back(train_paths[e1][0]);
						takenpaths.push_back(train_paths[e1][0].pathid);
						path_taken = 0;
					}
					else
					{
						path_taken = 0;
						for(int g1=0;g1<takenpaths.size();g1++)
						{
							if(train_paths[e1][f1].pathid==takenpaths[g1])
							{
								path_taken = 1;
								f1++;
								if(f1 > train_paths[e1].size()-1)
								{
									f1 = 0;
									path_taken = 0;
								}
								break;
							}
						}
						if(path_taken==0)
						{
							chosenpaths.push_back(train_paths[e1][f1]);
							takenpaths.push_back(train_paths[e1][f1].pathid);
						}
					}
				}while(path_taken==1);
			}
			for(int k1=0;k1<chosenpaths.size();k1++) trains.push_back(file.train[k1+j1*chunk]);
			float minval = 100000.0;
			float minval_fin = 100000.0;
			vector<vector<string> > done_comb;
			path_datatype trainpath1;
			path_datatype trainpath2;
			int rand_train1, rand_train2;
			for(int m1=0;m1<7;m1++)
			{
				vector<string> new_comb;
				for(int p1=0;p1<chosenpaths.size();p1++) new_comb.push_back(chosenpaths[p1].pathid);
				done_comb.push_back(new_comb);
				new_comb.clear();
				cout << "done combinations : " << endl;
				for(int d2=0;d2<done_comb.size();d2++)
				{
					for(int e2=0;e2<done_comb[d2].size();e2++) cout << done_comb[d2][e2] << " ";
					cout << endl;
				}
				cout << endl;
				cout << "Paths : " << endl;
				for(int h1=0;h1<chosenpaths.size();h1++)
				{
					cout << trains[h1].header << " : " << chosenpaths[h1].pathid << " ";
					for(int i1=0;i1<chosenpaths[h1].path.size();i1++) cout << chosenpaths[h1].path[i1].node1 << ", ";
					cout << chosenpaths[h1].path[chosenpaths[h1].path.size()-1].node2 << endl;
				}
				cout << endl;
				IloEnv env;
				IloModel model(env);
				NumVarMatrix nodetime(env, chosenpaths.size());
				IloNumVarArray delay_p0(env, chosenpaths.size(), 0, 12.0*3600.0);
				NumVarMatrix delay_p(env, chosenpaths.size());
				IloNumVarArray sa_p(env);
				IloNumVarArray twt_pl(env, chosenpaths.size(), 0, IloInfinity);
				IloNumVarArray twt_pu(env, chosenpaths.size(), 0, IloInfinity);
				IloNumVarArray utu_p(env);
				populate (file.mow, trains, chosenpaths, terminal1, terminal2, g_starttime, model, nodetime, fwsplits, bwsplits, delay_p0, delay_p, sa_p, twt_pl, twt_pu, utu_p, horizon);
				IloCplex cplex(model);
				//cplex.setOut(env.getNullStream());
				//cplex.setParam(IloCplex::RootAlg, 3);Values
				cplex.setParam(IloCplex::PopulateLim, 1);
				cplex.setParam(IloCplex::TiLim, 150);
				//cplex.setParam(IloCplex::EpGap, 0.2);
				bool solve_status = cplex.solve();
				if(solve_status==1)
				{
					float newval = cplex.getObjValue();
					if(newval < minval || newval < minval_fin)
					{
						minval = newval;
						//IloNumArray vals(env);
						vector<path_datatype> chosenpaths2;
						for(int s1=0;s1<chosenpaths.size();s1++)
						{
							int sidingdone = 0;
							for(int t1=0;t1<chosenpaths[s1].path.size();t1++)
							{
								if(chosenpaths[s1].path[t1].tracktype=="S" && cplex.getValue(delay_p[s1][t1]<=0.0))
								{
									sidingdone = 1;
									int u1, v1, i2;
									if(t1>=2)
									{
										u1 = t1-2;
										v1 = 2;
										i2 = 1;
									
									}
									else
									{
										u1 = t1+2;
										v1 = -2;
										i2 = -1;
									}
									int isok = 0;
									for(int w1=0;w1<chosenpaths.size();w1++)
									{
										if(w1!=s1)
										{
											for(int x1=1;x1<chosenpaths[w1].path.size();x1++)
											{
												if(chosenpaths[w1].path[x1].node1==chosenpaths[s1].path[u1].node1 && chosenpaths[w1].path[x1].node2==chosenpaths[s1].path[u1].node2)
												{
													if(chosenpaths[w1].path[x1+i2].tracktype!="SW")
													{
														if(cplex.getValue(delay_p[w1][x1+v1])>0.0)
														{
															if(cplex.getValue(nodetime[w1][x1+v1])<cplex.getValue(nodetime[s1][t1]) && cplex.getValue(nodetime[w1][x1+v1+1])>cplex.getValue(nodetime[s1][t1])) isok = 1;
														}
													}
													break;
												}
												else if(chosenpaths[w1].path[x1].node1==chosenpaths[s1].path[u1].node2 && chosenpaths[w1].path[x1].node2==chosenpaths[s1].path[u1].node1)
												{

													if(chosenpaths[w1].path[x1-i2].tracktype!="SW")
													{
														if(cplex.getValue(delay_p[w1][x1-v1])>0.0)
														{
															if(cplex.getValue(nodetime[w1][x1-v1])<cplex.getValue(nodetime[s1][t1]) && cplex.getValue(nodetime[w1][x1-v1+1])>cplex.getValue(nodetime[s1][t1])) isok = 1;
														}
													}
													break;
												}
											}
											if(isok == 1) break;
										}
									}
									if(isok == 1) chosenpaths2.push_back(chosenpaths[s1]);
									else
									{
										size_t colon = chosenpaths[s1].pathid.find_first_of(":");
										string newpathid = "id#" + chosenpaths[s1].pathid.substr(colon+1);
										for(int y1=0;y1<train_paths[j1*chunk+s1].size();y1++)
										{
											if(train_paths[j1*chunk+s1][y1].pathid==newpathid)
											{
												chosenpaths2.push_back(train_paths[j1*chunk+s1][y1]);
												break;
											}
										}
									}
								}
							}
							if(sidingdone == 0) chosenpaths2.push_back(chosenpaths[s1]);
						}
						cout << endl << "Eliminating redundant siding usages..." << endl << endl;
						cout << "New Paths : " << endl;
						for(int f2=0;f2<chosenpaths2.size();f2++)
						{
							cout << trains[f2].header << " : " << chosenpaths2[f2].pathid << " ";
							for(int g2=0;g2<chosenpaths2[f2].path.size();g2++) cout << chosenpaths2[f2].path[g2].node1 << ", ";
							cout << chosenpaths2[f2].path[chosenpaths2[f2].path.size()-1].node2 << endl;
						}
						cout << endl;
						IloEnv env2;
						IloModel model2(env2);
						NumVarMatrix nodetime2(env2, chosenpaths2.size());
						IloNumVarArray delay_p02(env2, chosenpaths2.size(), 0, 12.0*3600.0);
						NumVarMatrix delay_p2(env2, chosenpaths2.size());
						IloNumVarArray sa_p2(env2);
						IloNumVarArray twt_pl2(env2, chosenpaths2.size(), 0, IloInfinity);
						IloNumVarArray twt_pu2(env2, chosenpaths2.size(), 0, IloInfinity);
						IloNumVarArray utu_p2(env2);
						populate (file.mow, trains, chosenpaths2, terminal1, terminal2, g_starttime, model2, nodetime2, fwsplits, bwsplits, delay_p02, delay_p2, sa_p2, twt_pl2, twt_pu2, utu_p2, horizon);
						IloCplex cplex2(model2);
						//cplex2.setOut(env.getNullStream());
						cplex2.setParam(IloCplex::PopulateLim, 1);
						cplex2.setParam(IloCplex::TiLim, 200);
						bool solve_status2 = cplex2.solve();
						if(solve_status2==1)
						{
							float newval_fin = cplex2.getObjValue();
							if(newval_fin < minval_fin)
							{
								times_temp.clear();
								delay_temp.clear();
								chosenpaths_temp.clear();
								minval_fin = newval_fin;
								IloNumArray vals(env2);
								g_starttime_tmp = 0.0;
								chosenpaths_temp = chosenpaths2;
								cout << endl << "************* New solution found *************" << endl << endl;
								env2.out() << "Solution status = " << cplex2.getStatus() << endl;
								env2.out() << "Solution value  = " << cplex2.getObjValue() << endl;
								sub_cost = cplex2.getObjValue();
								for(int z1=0;z1<chosenpaths2.size();z1++)
								{
									if(g_starttime_tmp < cplex2.getValue(nodetime2[z1][nodetime2[z1].getSize()-1])) g_starttime_tmp = cplex2.getValue(nodetime2[z1][nodetime2[z1].getSize()-1]);
									vector<float> newtimes;
									for(int a2=0;a2<nodetime2[z1].getSize();a2++)
									{
										if(cplex2.getValue(nodetime2[z1][a2])<0) newtimes.push_back(0.0);
										else newtimes.push_back(cplex2.getValue(nodetime2[z1][a2]));
									}
									times_temp.push_back(newtimes);
									newtimes.clear();
									cout << trains[z1].header << " : ";
									cplex2.getValues(vals, nodetime2[z1]);
									env2.out() << "t (sec) = " << vals << endl;
								}
								cplex2.getValues(vals, delay_p02);
								env2.out() << "Entry delay Values (sec) = " << vals << endl;
								for(int b2=0;b2<chosenpaths.size();b2++)
								{
									vector<float> newdelay;
									for(int c2=0;c2<delay_p2[b2].getSize();c2++)
									{
										if(cplex2.getValue(delay_p2[b2][c2])<0) newdelay.push_back(0.0);
										else newdelay.push_back(cplex2.getValue(delay_p2[b2][c2]));
									}
									delay_temp.push_back(newdelay);
									newdelay.clear();
									cout << trains[b2].header << " : ";
									cplex2.getValues(vals, delay_p2[b2]);
									env2.out() << "In-path delay Values (sec) = " << vals << endl;
								}
								cplex2.getValues(vals, sa_p2);
								env2.out() << "SA delay values (sec) = " << vals << endl;
								cplex2.getValues(vals, twt_pl2);
								env2.out() << "TWT early entry penalty Values (sec) = " << vals << endl;
								cplex2.getValues(vals, twt_pu2);
								env2.out() << "TWT late entry penalty Values (sec) = " << vals << endl;
								cplex2.getValues(vals, utu_p2);
								env2.out() << "UTU penalty Values (sec) = " << vals << endl << endl;
								cout << "***********************************************" << endl << endl;
							}
						}
						env2.end();
						chosenpaths2.clear();
					}
					else
					{
						chosenpaths[rand_train1] = trainpath1;
						chosenpaths[rand_train2] = trainpath2;
					}
				}
				env.end();
				int swapdone = 0;
				int givup = 0;
				srand(time(NULL));
				while(swapdone==0 && givup < 100)
				{
					rand_train1 = rand()%chosenpaths.size();
					rand_train2 = rand()%chosenpaths.size();
					int match1 = 0;
					int match2 = 0;
					int n1, o1;
					if(rand_train1 != rand_train2 && trains[rand_train1].tob < 100 && trains[rand_train2].tob < 100)
					{
						for(n1=0;n1<train_paths[rand_train1+j1*chunk].size();n1++)
						{
							if(train_paths[rand_train1+j1*chunk][n1].pathid==chosenpaths[rand_train2].pathid)
							{	
								match1 = 1;
								break;
							}
						}
						for(o1=0;o1<train_paths[rand_train2+j1*chunk].size();o1++)
						{
							if(train_paths[rand_train2+j1*chunk][o1].pathid==chosenpaths[rand_train1].pathid)
							{	
								match2 = 1;
								break;
							}
						}
						vector<string> chosenpathids_temp;
						int comb_match;
						for(int q1=0;q1<chosenpaths.size();q1++) chosenpathids_temp.push_back(chosenpaths[q1].pathid);
						if(match1 == 1 && match2 == 1)
						{
							chosenpathids_temp[rand_train1] = train_paths[rand_train1+j1*chunk][n1].pathid;
							chosenpathids_temp[rand_train2] = train_paths[rand_train2+j1*chunk][o1].pathid;
							for(int r1=0;r1<done_comb.size();r1++)
							{
								comb_match = 1;
								for(int s1=0;s1<chosenpaths.size();s1++) if(chosenpathids_temp[s1]!=done_comb[r1][s1]) comb_match = 0;
								if(comb_match==1) break;
							}
							if(comb_match==0)
							{
								trainpath1 = chosenpaths[rand_train1];
								trainpath2 = chosenpaths[rand_train2];
								chosenpaths[rand_train1] = train_paths[rand_train1+j1*chunk][n1];
								chosenpaths[rand_train2] = train_paths[rand_train2+j1*chunk][o1];
								cout << endl << "swapping : " << endl;
								cout << endl << trains[rand_train1].header << " : " << chosenpaths[rand_train1].pathid << " <---> " << trains[rand_train2].header << " : " << chosenpaths[rand_train2].pathid << endl << endl;
								swapdone = 1;
							}
						}
						chosenpathids_temp.clear();
						givup++;
					}
				}
				if(givup >= 100)
				{
					cout << endl << "Possible combinations exhausted..." << endl << endl;
					break;
				}
			}
			for(int h2=0;h2<times_temp.size();h2++) times.push_back(times_temp[h2]);
			for(int j2=0;j2<delay_temp.size();j2++) delay.push_back(delay_temp[j2]);
			for(int k2=0;k2<chosenpaths_temp.size();k2++) chosenpaths_fin.push_back(chosenpaths_temp[k2]);
			cost_fin = cost_fin + sub_cost;
			g_starttime = g_starttime_tmp;
			chosenpaths.clear();
			takenpaths.clear();
			trains.clear();
		}
		cout << "********************* Final solution ***********************" << endl << endl;
		if(times.size()!=file.train_count) cout << "Unable to find complete solution" << endl << endl;
		cout << "Total cost = " << cost_fin << endl << endl;
		for(int l2=0;l2<file.train_count;l2++)
		{
			cout << file.train[l2].header << endl;
			cout << "Path : ";
			for(int m2=0;m2<chosenpaths_fin[l2].path.size();m2++) cout << chosenpaths_fin[l2].path[m2].node1 << " - ";
			cout << chosenpaths_fin[l2].path[chosenpaths_fin[l2].path.size()-1].node2 << endl;
			cout << "Times : ";
			for(int n2=0;n2<times[l2].size()-1;n2++) cout << times[l2][n2] << " - ";
			cout << times[l2][times[l2].size()-1] << endl <<endl;
		}
		cout << "*************************************************************" << endl << endl;
		string filename = file.territory + " OUTPUT.xml";
		writefile(filename, file.territory, file.train, chosenpaths_fin, times);
	}
    return 0;
}

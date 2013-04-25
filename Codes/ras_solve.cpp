#include <iostream>
#include<string>
#include <vector>
#include <ilcplex/ilocplex.h>

ILOSTLBEGIN

typedef IloArray<IloNumVarArray> NumVarMatrix;
typedef IloArray<IloRangeArray> RangeMatrix;


static void
populate (vector<mow_datatype> mow_cp, vector<train_datatype> train_cp, vector<path_datatype> chosenpaths_cp, int term1, int term2, float global_starttime, IloModel model, NumVarMatrix t, vector<int> fwsplits_cp, vector<int> bwsplits_cp, IloNumVarArray dp0, NumVarMatrix dp, IloNumVarArray sap, IloNumVarArray twtpl, IloNumVarArray twtpu, IloNumVarArray utup, double H)
{
	IloEnv env = model.getEnv();
	IloObjective obj = IloMinimize(env);
	RangeMatrix c(env, chosenpaths_cp.size());
	double M = 100000.0;
	for(int a=0;a<chosenpaths_cp.size();a++)
	{
		float entry_time = (global_starttime>train_cp[a].entry*60.0)?global_starttime:train_cp[a].entry*60.0;
		t[a] = IloNumVarArray(env, chosenpaths_cp[a].path.size()+1, entry_time, IloInfinity, ILOFLOAT);
		IloNumArray c_lbs(env);
		IloNumArray c_ubs(env);
		for(int b=0;b<chosenpaths_cp[a].path.size();b++)
		{
			c_lbs.add(chosenpaths_cp[a].path[b].req_time);
			int bu1 = (b<chosenpaths_cp[a].path.size()-1)?b+1:b;
			int bl1 = (b>0)?b-1:b;
			int issplit;
			int cubs_done = 0;
			if(chosenpaths_cp[a].path[b].node1 < chosenpaths_cp[a].path[b].node2)
			{
				issplit = 0;
				for(int a1=0; a1<bwsplits_cp.size();a1++)
				{
					if(chosenpaths_cp[a].path[bu1].node2==bwsplits_cp[a1])
					{
						issplit = 1;
						break;
					}
				}
				for(int a1=0; a1<fwsplits_cp.size();a1++)
				{
					if(chosenpaths_cp[a].path[bl1].node1==fwsplits_cp[a1])
					{
						issplit = 1;
						break;
					}
				}
				if(issplit==1)
				{
					c_ubs.add(IloInfinity);
					cubs_done = 1;
				}
			}
			else if(chosenpaths_cp[a].path[b].node1 > chosenpaths_cp[a].path[b].node2)
			{
				issplit = 0;
				for(int b1=0; b1<fwsplits_cp.size();b1++)
				{
					if(chosenpaths_cp[a].path[bu1].node2==fwsplits_cp[b1])
					{
						issplit = 1;
						break;
					}
				}
				for(int b1=0; b1<bwsplits_cp.size();b1++)
				{
					if(chosenpaths_cp[a].path[bl1].node1==bwsplits_cp[b1])
					{
						issplit = 1;
						break;
					}
				}
				if(issplit==1)
				{
					c_ubs.add(IloInfinity);
					cubs_done = 1;
				}
			}
			int issch_arr = 0;
			for(int c1=0;c1<train_cp[a].sch_arr.size();c1++) if(chosenpaths_cp[a].path[b].node1==train_cp[a].sch_arr[c1].node) issch_arr = 1;
			if(issch_arr==1)
			{
				//cout << train_cp[a].header << " : " << chosenpaths_cp[a].path[b].node1 << endl;
				c_ubs.add(IloInfinity);
				cubs_done = 1;
			}
			if(cubs_done==0) c_ubs.add(chosenpaths_cp[a].path[b].req_time);
		}
		//cout << c_ubs.getSize() << " " << c_lbs.getSize() << endl;
		c[a] = IloRangeArray(env, c_lbs, c_ubs);
		//cout << "woohoo1" << endl;
		c_lbs.clear();
		c_ubs.clear();
	}
	for(int d=0;d<chosenpaths_cp.size();d++)
	{
		for(int e=0;e<chosenpaths_cp[d].path.size();e++)
		{
			c[d][e].setLinearCoef(t[d][e], -1.0);
			c[d][e].setLinearCoef(t[d][e+1], 1.0);
		}
	}
	IloRangeArray midentrycon(env);
	int midentrycon_count = 0;
	for(int a1=0;a1<chosenpaths_cp.size();a1++)
	{
		if(chosenpaths_cp[a1].path[0].node1 != term1 && chosenpaths_cp[a1].path[0].node1 != term2)
		{
			for(int y=0;y<chosenpaths_cp.size();y++)
			{
				if(y!=a1)
				{
					for(int z=1;z<chosenpaths_cp[y].path.size();z++)
					{
						if(chosenpaths_cp[a1].path[0].node1 == chosenpaths_cp[y].path[z].node1 && chosenpaths_cp[a1].path[0].node2 == chosenpaths_cp[y].path[z].node2)
						{
							//cout << train_cp[a1].header << " : " << train_cp[y].header << endl;
							midentrycon.add(IloRange(env, 300, IloInfinity));
							midentrycon[midentrycon_count].setLinearCoef(t[y][z-1], 1.0);
							midentrycon[midentrycon_count].setLinearCoef(t[a1][0], -1.0);
							midentrycon_count++;
						}
					}
				}
			}
		}
	}
	//cout << "woohoo" << endl;
	IloNumVarArray x(env);
	IloRangeArray xcon1(env);
	IloRangeArray xcon2(env);
	int x_count = 0;
	for(int f=0;f<chosenpaths_cp.size();f++)
	{
		for(int g=0;g<chosenpaths_cp[f].path.size();g++)
		{
			for(int h=f+1;h<chosenpaths_cp.size();h++)
			{
				for(int i=0;i<chosenpaths_cp[h].path.size();i++)
				{
					if(chosenpaths_cp[f].path[g].node1 == chosenpaths_cp[h].path[i].node1 && chosenpaths_cp[f].path[g].node2 == chosenpaths_cp[h].path[i].node2 || chosenpaths_cp[f].path[g].node1 == chosenpaths_cp[h].path[i].node2 && chosenpaths_cp[f].path[g].node2 == chosenpaths_cp[h].path[i].node1)
					{
						int gl1 = (g>0)?g-1:0;
						int gu1 = (g<chosenpaths_cp[f].path.size()-1)?g+1:chosenpaths_cp[f].path.size();
						int gu2 = (g<chosenpaths_cp[f].path.size()-2)?g+2:chosenpaths_cp[f].path.size();
						int gu1_index = (g<chosenpaths_cp[f].path.size()-1)?g+1:chosenpaths_cp[f].path.size()-1;
						int il1 = (i>0)?i-1:0;
						int iu1 = (i<chosenpaths_cp[h].path.size()-1)?i+1:chosenpaths_cp[h].path.size();
						int iu2 = (i<chosenpaths_cp[h].path.size()-2)?i+2:chosenpaths_cp[h].path.size();
						int iu1_index = (i<chosenpaths_cp[h].path.size()-1)?i+1:chosenpaths_cp[h].path.size()-1;
						if(chosenpaths_cp[f].path[g].node1 == chosenpaths_cp[h].path[i].node2 && chosenpaths_cp[f].path[g].node2 == chosenpaths_cp[h].path[i].node1)
						{
							if((chosenpaths_cp[f].path[gl1].tracktype=="SW" || chosenpaths_cp[f].path[gl1].tracktype=="C" || chosenpaths_cp[h].path[iu1_index].tracktype=="SW" || chosenpaths_cp[h].path[iu1_index].tracktype=="C") && (chosenpaths_cp[f].path[gl1].node1 != chosenpaths_cp[h].path[iu1_index].node2 || chosenpaths_cp[f].path[gl1].node2 != chosenpaths_cp[h].path[iu1_index].node1))
							{
								x.add(IloNumVar(env, 0, 1, ILOINT));
								xcon1.add(IloRange(env, 300.0, IloInfinity));
								xcon2.add(IloRange(env, 300.0-M, IloInfinity));
								xcon1[x_count].setLinearCoef(x[x_count], M);
								xcon1[x_count].setLinearCoef(t[f][gl1], 1.0);
								xcon1[x_count].setLinearCoef(t[h][iu2], -1.0);
								xcon2[x_count].setLinearCoef(x[x_count], -M);
								xcon2[x_count].setLinearCoef(t[h][i], 1.0);
								xcon2[x_count].setLinearCoef(t[f][gu1], -1.0);
								x_count++;
							}
							else if((chosenpaths_cp[h].path[il1].tracktype=="SW" || chosenpaths_cp[h].path[il1].tracktype=="C" || chosenpaths_cp[f].path[gu1_index].tracktype=="SW" || chosenpaths_cp[f].path[gu1_index].tracktype=="C") && (chosenpaths_cp[h].path[il1].node1 != chosenpaths_cp[f].path[gu1_index].node2 || chosenpaths_cp[h].path[il1].node2 != chosenpaths_cp[f].path[gu1_index].node1))
							{
								x.add(IloNumVar(env, 0, 1, ILOINT));
								xcon1.add(IloRange(env, 300.0, IloInfinity));
								xcon2.add(IloRange(env, 300.0-M, IloInfinity));
								xcon1[x_count].setLinearCoef(x[x_count], M);
								xcon1[x_count].setLinearCoef(t[h][il1], 1.0);
								xcon1[x_count].setLinearCoef(t[f][gu2], -1.0);
								xcon2[x_count].setLinearCoef(x[x_count], -M);
								xcon2[x_count].setLinearCoef(t[f][g], 1.0);
								xcon2[x_count].setLinearCoef(t[h][iu1], -1.0);
								x_count++;
							}
						}
						else if(chosenpaths_cp[f].path[g].node1 == chosenpaths_cp[h].path[i].node1 && chosenpaths_cp[f].path[g].node2 == chosenpaths_cp[h].path[i].node2)
						{
							if((chosenpaths_cp[f].path[gl1].tracktype=="SW" || chosenpaths_cp[f].path[gl1].tracktype=="C" || chosenpaths_cp[h].path[il1].tracktype=="SW" || chosenpaths_cp[h].path[il1].tracktype=="SW") && (chosenpaths_cp[f].path[gl1].node1 != chosenpaths_cp[h].path[il1].node1 || chosenpaths_cp[f].path[gl1].node2 != chosenpaths_cp[h].path[il1].node2))
							{
								x.add(IloNumVar(env, 0, 1, ILOINT));
								xcon1.add(IloRange(env, 300.0, IloInfinity));
								xcon2.add(IloRange(env, 300.0-M, IloInfinity));
								xcon1[x_count].setLinearCoef(x[x_count], M);
								xcon1[x_count].setLinearCoef(t[f][gl1], 1.0);
								xcon1[x_count].setLinearCoef(t[h][i], -1.0);
								xcon2[x_count].setLinearCoef(x[x_count], -M);
								xcon2[x_count].setLinearCoef(t[h][il1], 1.0);
								xcon2[x_count].setLinearCoef(t[f][g], -1.0);
								x_count++;
							}
						}
						x.add(IloNumVar(env, 0, 1, ILOINT));
						xcon1.add(IloRange(env, 300.0, IloInfinity));
						xcon2.add(IloRange(env, 300.0-M, IloInfinity));
						xcon1[x_count].setLinearCoef(x[x_count], M);
						xcon1[x_count].setLinearCoef(t[h][i], 1.0);
						xcon1[x_count].setLinearCoef(t[f][gu1], -1.0);
						xcon2[x_count].setLinearCoef(x[x_count], -M);
						xcon2[x_count].setLinearCoef(t[f][g], 1.0);
						xcon2[x_count].setLinearCoef(t[h][iu1], -1.0);
						x_count++;
					}
				}
			}
		}
	}
	IloNumVarArray mow_x(env);
	IloRangeArray mowcon1(env);
	IloRangeArray mowcon2(env);
	int mowcon_count = 0;
	for(int j=0;j<chosenpaths_cp.size();j++)
	{
		for(int l=0;l<mow_cp.size();l++)
		{
			for(int k=0;k<chosenpaths_cp[j].path.size();k++)
			{
				if(chosenpaths_cp[j].path[k].node1 == mow_cp[l].startnode || chosenpaths_cp[j].path[k].node1 == mow_cp[l].endnode)
				{
					int k_temp = k;
					int hasmow = 0;
					while(k_temp < chosenpaths_cp[j].path.size())
					{
						if(chosenpaths_cp[j].path[k_temp].node2 == mow_cp[l].startnode || chosenpaths_cp[j].path[k_temp].node2 == mow_cp[l].endnode)
						{
							hasmow = 1;
							break;
						}
						k_temp++;
					}
					if(hasmow==1)
					{
						mow_x.add(IloNumVar(env, 0, 1, ILOINT));
						mowcon1.add(IloRange(env, mow_cp[l].endtime*60.0, IloInfinity));
						mowcon1[mowcon_count].setLinearCoef(mow_x[mowcon_count], M);
						mowcon1[mowcon_count].setLinearCoef(t[j][k], 1.0);
						while((chosenpaths_cp[j].path[k].node2 != mow_cp[l].startnode && chosenpaths_cp[j].path[k].node2 != mow_cp[l].endnode) && (k < chosenpaths_cp[j].path.size())) k++;
						mowcon2.add(IloRange(env, -IloInfinity, (M+mow_cp[l].starttime*60.0-1)));
						mowcon2[mowcon_count].setLinearCoef(mow_x[mowcon_count], M);
						mowcon2[mowcon_count].setLinearCoef(t[j][k+1], 1.0);
						mowcon_count++;
						break;
					}
				}
			}
		}
	}
	IloNumVarArray dp0_horslack(env, chosenpaths_cp.size(), 0, IloInfinity);
	IloRangeArray horcon10(env, chosenpaths_cp.size(), -IloInfinity, (M+H));
	IloRangeArray horcon20(env, chosenpaths_cp.size(), H, IloInfinity);
	IloRangeArray dp0valcon(env);
	IloRangeArray dp0_horcon1(env, chosenpaths_cp.size(), H, IloInfinity);
	IloRangeArray dp0_horcon2(env, chosenpaths_cp.size(), -IloInfinity, M);
	
	NumVarMatrix hor_x(env, chosenpaths_cp.size());
	RangeMatrix horcon1(env, chosenpaths_cp.size());
	RangeMatrix horcon2(env, chosenpaths_cp.size());
	
	NumVarMatrix dp_horslack(env, chosenpaths_cp.size());
	RangeMatrix dpvalcon(env, chosenpaths_cp.size());
	RangeMatrix dp_horcon1(env, chosenpaths_cp.size());
	RangeMatrix dp_horcon2(env, chosenpaths_cp.size());
	
	IloNumVarArray sap_slack(env);
	IloNumVarArray sap_horslack(env);
	IloNumVarArray sa_x(env);
	IloRangeArray sa_xcon1(env);
	IloRangeArray sa_xcon2(env);
	IloRangeArray savalcon(env);
	IloRangeArray sacon1(env);
	IloRangeArray sacon2(env);
	IloRangeArray sa_horcon1(env);
	IloRangeArray sa_horcon2(env);
	int sa_count = 0;
	
	IloNumVarArray twtpl_slack(env, chosenpaths_cp.size(), -IloInfinity, 0);
	IloNumVarArray twtpl_horslack(env, chosenpaths_cp.size(), 0, IloInfinity);
	IloNumVarArray twtl_x(env, chosenpaths_cp.size(), 0, 1, ILOINT);
	IloRangeArray twtl_xcon1(env);
	IloRangeArray twtl_xcon2(env);
	IloRangeArray twtlvalcon(env);
	IloRangeArray twtlcon1(env);
	IloRangeArray twtlcon2(env);
	IloRangeArray twtl_horcon1(env);
	IloRangeArray twtl_horcon2(env);
	
	IloNumVarArray twtpu_slack(env, chosenpaths_cp.size(), -IloInfinity, 0);
	IloNumVarArray twtpu_horslack(env, chosenpaths_cp.size(), 0, IloInfinity);
	IloNumVarArray twtu_x(env, chosenpaths_cp.size(), 0, 1, ILOINT);
	IloRangeArray twtu_xcon1(env);
	IloRangeArray twtu_xcon2(env);
	IloRangeArray twtuvalcon(env);
	IloRangeArray twtucon1(env);
	IloRangeArray twtucon2(env);
	IloRangeArray twtu_horcon1(env);
	IloRangeArray twtu_horcon2(env);
	
	IloNumVarArray utup_horslack(env);
	IloNumVarArray utu_x(env);
	IloRangeArray utuvalcon(env);
	IloRangeArray utu_horcon1(env);
	IloRangeArray utu_horcon2(env);
	int utu_count = 0;
	
	for(int m=0;m<chosenpaths_cp.size();m++)
	{		
		hor_x[m] = IloNumVarArray(env, chosenpaths_cp[m].path.size()+1, 0, 1, ILOINT);
		horcon10[m].setLinearCoef(hor_x[m][0], M);
		horcon10[m].setLinearCoef(t[m][0], 1.0);
		horcon20[m].setLinearCoef(hor_x[m][0], M);
		horcon20[m].setLinearCoef(t[m][0], 1.0);
		
		dp0valcon.add(IloRange(env, train_cp[m].entry*60.0, train_cp[m].entry*60.0));
		dp0valcon[m].setLinearCoef(t[m][0], 1.0);
		dp0valcon[m].setLinearCoef(dp0[m], -1.0);
		dp0valcon[m].setLinearCoef(dp0_horslack[m], -1.0);
		dp0_horcon1[m].setLinearCoef(hor_x[m][0], M);
		dp0_horcon1[m].setLinearCoef(dp0[m], 1.0);
		dp0_horcon2[m].setLinearCoef(hor_x[m][0], M);
		dp0_horcon2[m].setLinearCoef(dp0_horslack[m], 1.0);
		
		horcon1[m] = IloRangeArray(env, chosenpaths_cp[m].path.size(), -IloInfinity, (M+H));
		horcon2[m] = IloRangeArray(env, chosenpaths_cp[m].path.size(), H, IloInfinity);
		dp[m] = IloNumVarArray(env, chosenpaths_cp[m].path.size(), 0, IloInfinity);
		dp_horslack[m] = IloNumVarArray(env, chosenpaths_cp[m].path.size(), 0, IloInfinity);
		IloNumArray dpvalcon_bound(env);
		for(int p=0;p<chosenpaths_cp[m].path.size();p++) dpvalcon_bound.add(chosenpaths_cp[m].path[p].req_time);
		dpvalcon[m] = IloRangeArray(env, dpvalcon_bound, dpvalcon_bound);
		dpvalcon_bound.clear();
		dp_horcon1[m] = IloRangeArray(env, chosenpaths_cp[m].path.size(), -IloInfinity, 0);
		dp_horcon2[m] = IloRangeArray(env, chosenpaths_cp[m].path.size(), -IloInfinity, M);
		for(int n=0;n<chosenpaths_cp[m].path.size();n++)
		{
			horcon1[m][n].setLinearCoef(hor_x[m][n+1], M);
			horcon1[m][n].setLinearCoef(t[m][n+1], 1.0);
			horcon2[m][n].setLinearCoef(hor_x[m][n+1], M);
			horcon2[m][n].setLinearCoef(t[m][n+1], 1.0);
			 
			dpvalcon[m][n].setLinearCoef(t[m][n+1], 1.0);
			dpvalcon[m][n].setLinearCoef(t[m][n], -1.0);
			dpvalcon[m][n].setLinearCoef(dp[m][n], -1.0);
			dpvalcon[m][n].setLinearCoef(dp_horslack[m][n], -1.0);
			dp_horcon1[m][n].setLinearCoef(hor_x[m][n], -M);
			dp_horcon1[m][n].setLinearCoef(dp[m][n], 1.0);
			dp_horcon2[m][n].setLinearCoef(hor_x[m][n], M);
			dp_horcon2[m][n].setLinearCoef(dp_horslack[m][n], 1.0);
			
			for(int o=0;o<train_cp[m].sch_arr_count;o++)
			{
				if(train_cp[m].sch_arr[o].node == chosenpaths_cp[m].path[n].node2)
				{
					sap.add(IloNumVar(env, 0, IloInfinity));
					sap_slack.add(IloNumVar(env, -IloInfinity, 0));
					sap_horslack.add(IloNumVar(env, 0, IloInfinity));
					sa_x.add(IloNumVar(env, 0, 1, ILOINT));
					savalcon.add(IloRange(env, (train_cp[m].sch_arr[o].time+120.0)*60.0, (train_cp[m].sch_arr[o].time+120.0)*60.0));
					savalcon[sa_count].setLinearCoef(t[m][n+1], 1.0);
					savalcon[sa_count].setLinearCoef(sap[sa_count], -1.0);
					savalcon[sa_count].setLinearCoef(sap_slack[sa_count], -1.0);
					savalcon[sa_count].setLinearCoef(sap_horslack[sa_count], -1.0);
					sa_xcon1.add(IloRange(env, (train_cp[m].sch_arr[o].time+120.0)*60.0, IloInfinity));
					sa_xcon1[sa_count].setLinearCoef(sa_x[sa_count], M);
					sa_xcon1[sa_count].setLinearCoef(t[m][n+1], 1.0);
					sa_xcon2.add(IloRange(env, -IloInfinity, (train_cp[m].sch_arr[o].time+120.0)*60.0+M));
					sa_xcon2[sa_count].setLinearCoef(sa_x[sa_count], M);
					sa_xcon2[sa_count].setLinearCoef(t[m][n+1], 1.0);
					sacon1.add(IloRange(env, -IloInfinity, M));
					sacon1[sa_count].setLinearCoef(sa_x[sa_count], M);
					sacon1[sa_count].setLinearCoef(sap[sa_count], 1.0);
					sacon2.add(IloRange(env, 0, IloInfinity));
					sacon2[sa_count].setLinearCoef(sa_x[sa_count], M);
					sacon2[sa_count].setLinearCoef(sap_slack[sa_count], 1.0);
					sa_horcon1.add(IloRange(env, -IloInfinity, 0));
					sa_horcon1[sa_count].setLinearCoef(hor_x[m][n+1], -M);
					sa_horcon1[sa_count].setLinearCoef(sap[sa_count], 1.0);
					sa_horcon2.add(IloRange(env, -IloInfinity, M));
					sa_horcon2[sa_count].setLinearCoef(hor_x[m][n+1], M);
					sa_horcon2[sa_count].setLinearCoef(sap_horslack[sa_count], 1.0);
					sa_count++;
				}
			}
			int nl1 = (n>0)?n-1:0;
			int nl2 = (n>1)?n-2:0;
			int nu1 = (n<chosenpaths_cp[m].path.size()-1)?n+1:chosenpaths_cp[m].path.size()-1;
			if((train_cp[m].direction=="WESTBOUND" && (chosenpaths_cp[m].path[n].tracktype=="2" || chosenpaths_cp[m].path[n].tracktype=="SW" && chosenpaths_cp[m].path[nl1].tracktype=="2" || chosenpaths_cp[m].path[n].tracktype=="SW" && chosenpaths_cp[m].path[nu1].tracktype=="2" || chosenpaths_cp[m].path[n].tracktype=="S" && chosenpaths_cp[m].path[nl2].tracktype=="2")) || (train_cp[m].direction=="EASTBOUND" && (chosenpaths_cp[m].path[n].tracktype=="1" || chosenpaths_cp[m].path[n].tracktype=="SW" && chosenpaths_cp[m].path[nl1].tracktype=="1" || chosenpaths_cp[m].path[n].tracktype=="SW" && chosenpaths_cp[m].path[nu1].tracktype=="1" || chosenpaths_cp[m].path[n].tracktype=="S" && chosenpaths_cp[m].path[nl2].tracktype=="1")))
			{
				utup.add(IloNumVar(env, 0, IloInfinity));
				utup_horslack.add(IloNumVar(env, 0, IloInfinity));
				utu_x.add(IloNumVar(env, 0, 1, ILOINT));
				utuvalcon.add(IloRange(env, 0, 0));
				utuvalcon[utu_count].setLinearCoef(t[m][n+1], 1.0);
				utuvalcon[utu_count].setLinearCoef(t[m][n], -1.0);
				utuvalcon[utu_count].setLinearCoef(utup[utu_count], -1.0);
				utuvalcon[utu_count].setLinearCoef(utup_horslack[utu_count], -1.0);
				utu_horcon1.add(IloRange(env, -IloInfinity, 0));
				utu_horcon1[utu_count].setLinearCoef(hor_x[m][n], -M);
				utu_horcon1[utu_count].setLinearCoef(utup[utu_count], 1.0);
				utu_horcon2.add(IloRange(env, -IloInfinity, M));
				utu_horcon2[utu_count].setLinearCoef(hor_x[m][n], M);
				utu_horcon2[utu_count].setLinearCoef(utup_horslack[utu_count], 1.0);
				utu_count++;
			}
		}
		twtlvalcon.add(IloRange(env, (train_cp[m].twt-60.0)*60.0, (train_cp[m].twt-60.0)*60.0));
		twtlvalcon[m].setLinearCoef(t[m][chosenpaths_cp[m].path.size()], 1.0);
		twtlvalcon[m].setLinearCoef(twtpl[m], 1.0);
		twtlvalcon[m].setLinearCoef(twtpl_slack[m], 1.0);
		twtlvalcon[m].setLinearCoef(twtpl_horslack[m], 1.0);
		twtl_xcon1.add(IloRange(env, (train_cp[m].twt-60.0)*60.0, IloInfinity));
		twtl_xcon1[m].setLinearCoef(twtl_x[m], M);
		twtl_xcon1[m].setLinearCoef(t[m][chosenpaths_cp[m].path.size()], 1.0);
		twtl_xcon2.add(IloRange(env, -IloInfinity, M+(train_cp[m].twt-60.0)*60.0));
		twtl_xcon2[m].setLinearCoef(twtl_x[m], M);
		twtl_xcon2[m].setLinearCoef(t[m][chosenpaths_cp[m].path.size()], 1.0);
		twtlcon1.add(IloRange(env, -IloInfinity, 0));
		twtlcon1[m].setLinearCoef(twtl_x[m], -M);
		twtlcon1[m].setLinearCoef(twtpl[m], 1.0);
		twtlcon2.add(IloRange(env, -M, IloInfinity));
		twtlcon2[m].setLinearCoef(twtl_x[m], -M);
		twtlcon2[m].setLinearCoef(twtpl_slack[m], 1.0);
		twtl_horcon1.add(IloRange(env, -IloInfinity, 0));
		twtl_horcon1[m].setLinearCoef(hor_x[m][chosenpaths_cp[m].path.size()], -M);
		twtl_horcon1[m].setLinearCoef(twtpl[m], 1.0);
		twtl_horcon2.add(IloRange(env, -IloInfinity, M));
		twtl_horcon2[m].setLinearCoef(hor_x[m][chosenpaths_cp[m].path.size()], M);
		twtl_horcon2[m].setLinearCoef(twtpl_horslack[m], 1.0);
		
		twtuvalcon.add(IloRange(env, (train_cp[m].twt+180.0)*60.0, (train_cp[m].twt+180.0)*60.0));
		twtuvalcon[m].setLinearCoef(t[m][chosenpaths_cp[m].path.size()], 1.0);
		twtuvalcon[m].setLinearCoef(twtpu[m], -1.0);
		twtuvalcon[m].setLinearCoef(twtpu_slack[m], -1.0);
		twtuvalcon[m].setLinearCoef(twtpu_horslack[m], -1.0);
		twtu_xcon1.add(IloRange(env, -IloInfinity, (train_cp[m].twt+180.0)*60.0));
		twtu_xcon1[m].setLinearCoef(twtu_x[m], -M);
		twtu_xcon1[m].setLinearCoef(t[m][chosenpaths_cp[m].path.size()], 1.0);
		twtu_xcon2.add(IloRange(env, -IloInfinity, M-(train_cp[m].twt+180.0)*60.0));
		twtu_xcon2[m].setLinearCoef(twtu_x[m], M);
		twtu_xcon2[m].setLinearCoef(t[m][chosenpaths_cp[m].path.size()], -1.0);
		twtucon1.add(IloRange(env, -M, IloInfinity));
		twtucon1[m].setLinearCoef(twtu_x[m], -M);
		twtucon1[m].setLinearCoef(twtpu_slack[m], 1.0);
		twtucon2.add(IloRange(env, -IloInfinity, 0));
		twtucon2[m].setLinearCoef(twtu_x[m], -M);
		twtucon2[m].setLinearCoef(twtpu[m], 1.0);
		twtu_horcon1.add(IloRange(env, -IloInfinity, 0));
		twtu_horcon1[m].setLinearCoef(hor_x[m][chosenpaths_cp[m].path.size()-1], -M);
		twtu_horcon1[m].setLinearCoef(twtpu[m], 1.0);
		twtu_horcon2.add(IloRange(env, -IloInfinity, M));
		twtu_horcon2[m].setLinearCoef(hor_x[m][chosenpaths_cp[m].path.size()-1], M);
		twtu_horcon2[m].setLinearCoef(twtpu_horslack[m], 1.0);
	}
	float dp_m, sap_m;
	float twtp_m = 75.0/3600.0;
	float utup_m = 50.0/3600.0;
	//cout << "woohoo" << endl;
	for(int r=0;r<train_cp.size();r++)
	{
		if(train_cp[r].header[0]=='A')
		{
			dp_m = 600.0/3600.0;
			sap_m = 200.0/3600.0;
		}
		else if(train_cp[r].header[0]=='B')
		{
			dp_m = 500.0/3600.0;
			sap_m = 200.0/3600.0;
		}
		else if(train_cp[r].header[0]=='C')
		{
			dp_m = 400.0/3600.0;
			sap_m = 200.0/3600.0;
		}
		else if(train_cp[r].header[0]=='D')
		{
			dp_m = 300.0/3600.0;
			sap_m = 200.0/3600.0;
		}
		else if(train_cp[r].header[0]=='E')
		{
			dp_m = 150.0/3600.0;
			sap_m = 0.0;
		}
		else if(train_cp[r].header[0]=='F')
		{
			dp_m = 100.0/3600.0;
			sap_m = 0.0;
		}
		//cout << "dp_m = " << dp_m << endl;
		for(int s=0;s<chosenpaths_cp[r].path.size();s++) obj.setLinearCoef(dp[r][s], dp_m);
		obj.setLinearCoef(dp0[r], dp_m);
		obj.setLinearCoef(twtpl[r], twtp_m);
		obj.setLinearCoef(twtpu[r], twtp_m);
	}
	for(int u=0;u<sa_count;u++) obj.setLinearCoef(sap[u], sap_m);
	for(int v=0;v<utu_count;v++) obj.setLinearCoef(utup[v], utup_m);
	
	model.add(midentrycon);
	model.add(xcon1);
	model.add(xcon2);
	model.add(mowcon1);
	model.add(mowcon2);
	model.add(horcon10);
	model.add(horcon20);
	model.add(dp0valcon);
	model.add(dp0_horcon1);
	model.add(dp0_horcon2);
	for(int w=0;w<chosenpaths_cp.size();w++)
	{
		model.add(c[w]);
		model.add(horcon1[w]);
		model.add(horcon2[w]);
		model.add(dpvalcon[w]);
		model.add(dp_horcon1[w]);
		model.add(dp_horcon2[w]);
	}
	model.add(sa_xcon1);
	model.add(sa_xcon2);
	model.add(sa_horcon1);
	model.add(sa_horcon2);
	model.add(sacon1);
	model.add(sacon2);
	model.add(savalcon);
	model.add(twtl_xcon1);
	model.add(twtl_xcon2);
	model.add(twtl_horcon1);
	model.add(twtl_horcon2);
	model.add(twtlcon1);
	model.add(twtlcon2);
	model.add(twtlvalcon);
	model.add(twtu_xcon1);
	model.add(twtu_xcon2);
	model.add(twtu_horcon1);
	model.add(twtu_horcon2);
	model.add(twtucon1);
	model.add(twtucon2);
	model.add(twtuvalcon);
	model.add(utuvalcon);
	model.add(utu_horcon1);
	model.add(utu_horcon2);
	model.add(obj);
	IloNumArray vals(env);
}
	

/*
 * main.cpp
 *
 */

#include "ABC.h"
#include "Circuit.h"
#include <map>
#include <dirent.h>
#include <queue>
#include <cstdio>
#include <assert.h>
#include <string>
#include <vector>
#include <algorithm>

#define MAXDEP 100

using namespace std;

FILE *out1, *out2;
int Luts = 6, Dep = 0;
map<string, bool> vis;
vector<Cut> L[MAXDEP];

void get_file_name(string path, vector<string> &files) {
    struct dirent *ptr;
    DIR *dir = opendir(path.c_str());
    while ((ptr = readdir(dir)) != NULL) {
        if (ptr->d_name[0] != '.') {  // skip . and ..
            files.push_back(ptr->d_name);
        }
    }
    closedir(dir);
}

bool cmp_Depth(Cut A, Cut B) {
    return A.mindep < B.mindep || (A.mindep == B.mindep && A.AreaFlow < B.AreaFlow)
            || (A.mindep == B.mindep && A.AreaFlow == B.AreaFlow && A.Area < B.Area);
}

bool cmp_Depth2(Cut A, Cut B) {
    return A.mindep < B.mindep || (A.mindep == B.mindep && A.Area < B.Area)
            || (A.mindep == B.mindep && A.Area == B.Area && A.AreaFlow < B.AreaFlow);
}

bool Dcmp_Depth(DoubleCut A, DoubleCut B) {
    return A.mindep < B.mindep || (A.mindep == B.mindep && A.AreaFlow < B.AreaFlow)
            || (A.mindep == B.mindep && A.AreaFlow == B.AreaFlow && A.Area < B.Area);
}

bool Dcmp_Depth2(DoubleCut A, DoubleCut B) {
    return A.mindep < B.mindep || (A.mindep == B.mindep && A.Area < B.Area)
            || (A.mindep == B.mindep && A.Area == B.Area && A.AreaFlow < B.AreaFlow);
}

void Output(Circuit& c) {
    map<string, bool> Flag;
    Flag.clear();
    queue<string> Q;
    vis.clear();
    int area = 0, dep = 0;
    for (string s : c.output) {
        Q.push(s);
        vis[s] = 1;
        dep = max(dep, c.graph[s]->mindep);
    }
    while (!Q.empty()) {
        string now = Q.front();
        Q.pop();
        if (c.graph[now]->pre.size() == 0)
            continue;
        fprintf(out1, "input:\n");
        area++;
        for (string st : c.graph[now]->Rcut) {
            fprintf(out1, "%s ", st.c_str());
            if (!vis[st]) {
                Q.push(st);
                vis[st] = 1;
            }
        }
        fprintf(out1, "%s\n", ("\noutput:\n" + now).c_str());
    }
    fprintf(stdout, "Area: %d\nDepth: %d\n", area, dep); 
    c.Dep = dep;
}
void Output2(Circuit& c) {
    map<string, bool> Flag;
    Flag.clear();
    queue<string> Q;
    vis.clear();
    int area = 0;
    for (string s : c.output) {
        Q.push(s);
        vis[s] = 1;
    }
    while (!Q.empty()) {
        string now = Q.front();
        Q.pop();
        if (c.graph[now]->pre.size() == 0)
            continue;
	if (vis[now] > 1) continue;
        fprintf(out2, "input:\n");
        area++;
	if (c.graph[now]->Rdcut.empty()) {
            for (string st : c.graph[now]->Rcut) {
                fprintf(out2, "%s ", st.c_str());
                if (!vis[st]) {
                    Q.push(st);
                    vis[st] = 1;
                }
            }
            fprintf(out2, "%s\n", ("\noutput:\n" + now).c_str());
        }else {
	    printf("Hello\n");
            for (string st : c.graph[now]->Rdcut) {
                fprintf(out2, "%s ", st.c_str());
                if (!vis[st]) {
                    Q.push(st);
                    vis[st] = 1;
                }
            }
	    string p = c.graph[now]->Partner; 
	    vis[p] = 2;
	    printf("%s %s\n", now.c_str(), p.c_str());
            fprintf(out2, "%s\n", ("\noutput:\n" + now + " " + p).c_str());
        }
    }
    fprintf(stdout, "Area: %d\nDepth: %d\n", area, c.Dep);
}
int main(int argc, char *argv[]) {
    vector<string> benchmark;
    queue<string> Q;
    map<string, int> ind;
    unsigned int C = 15;  // record the most C Cuts
    string dir = "benchmark/ISCAS85";
    string outdir = "result/ISCAS85";
    get_file_name(dir, benchmark);

//    for (string str : benchmark) {
//	cout << str  << endl;
	string str = "c1355.blif"; {
        double total_time;
        clock_t start = clock();

        Circuit c = Circuit(dir + "/" + str);  // already synthesized
        //c.write_dot();

        out1 = fopen((outdir + "/" + str + ".out1").c_str(), "w");
        out2 = fopen((outdir + "/" + str + ".out2").c_str(), "w");

	for (int i = 0; i < MAXDEP; i++)
	    L[i].clear();

        for (pair<string, Var*> p : c.graph) {
            ind[p.first] = p.second->pre.size();
	    p.second->mindep = 0;
	    p.second->Rcut.clear();
	    p.second->Rdcut.clear();
	}

        for (string s : c.input) {           // Initial queue
            c.graph[s]->cuts[1].insert(set<string> { s });
            c.graph[s]->mindep = 0;
            c.graph[s]->Area = 0;
            c.graph[s]->AreaFlow = 0;
            for (string nt : c.graph[s]->suc) {
                ind[nt]--;
                if (ind[nt] == 0)
                    Q.push(nt);
            }
        }

        //Enumerate Cuts
        while (!Q.empty()) {
            string now = Q.front();
            //cout << now << endl;
            Q.pop();
            c.graph[now]->cuts[1].insert(set<string> { now });
            assert(c.graph[now]->pre.size() <= 2);
            if (c.graph[now]->pre.size() == 2) {
                string input1 = c.graph[now]->pre[0];
                string input2 = c.graph[now]->pre[1];
		for (int t = 1; t <= Luts; t++)
	       	    for (set<string> c1 : c.graph[input1]->cuts[t]) {
			for (int k = 1; k <= Luts; k++)
                             for (set<string> c2 : c.graph[input2]->cuts[k]) {
                                set<string> st = c1;
                                st.insert(c2.begin(), c2.end());
                                if (st.size() > (unsigned) Luts)
                                    continue;
                                c.graph[now]->cuts[st.size()].insert(st);
                            }
                        }
            }

            if (c.graph[now]->pre.size() == 1) {
                string input1 = c.graph[now]->pre[0];
		for (int k = 1; k <= Luts; k++)
                    for (set<string> c1 : c.graph[input1]->cuts[k])
                        c.graph[now]->cuts[k].insert(c1);
            }

            for (string nt : c.graph[now]->suc) {
                ind[nt]--;
                if (ind[nt] == 0)
                    Q.push(nt);
            }

            /*Calculate dep, areaflow, exact area*/
            vector<Cut> Cuts[10], Ct;
            c.graph[now]->mindep = INF;
            c.graph[now]->Area = INF;
            c.graph[now]->AreaFlow = inf;
	    for (int k = 1; k <= Luts; k++) {
	        for (set<string> ct : c.graph[now]->cuts[k]) {
                    int dep = 0, area = 0;
                    double AF = 0;
                    for (string nt : ct) {
                        dep = max(dep, c.graph[nt]->mindep);
                        area += c.graph[nt]->Area;
                        AF += c.graph[nt]->AreaFlow;
                    }
                    dep++, area++;
                    AF /= max(1.0, 1.0 * c.graph[now]->suc.size());

                    Cuts[k].push_back(Cut(ct, now, dep, area, AF));
		    Ct.push_back(Cut(ct, now, dep, area, AF));
		}
            }
            for (int k = 1; k <= Luts; k++) {
                sort(Cuts[k].begin(), Cuts[k].end(), cmp_Depth2);
            	c.graph[now]->cuts[k].clear();
            	unsigned int len = Cuts[k].size();
            //cout << "now:" << now << endl;
            	for (unsigned int i = 0; i < min(C, len); i++) {
                	c.graph[now]->cuts[k].insert(Cuts[k][i].names);
                //cout << Cuts[i].mindep << endl;
            	}
            }		
            //if (c.graph[now]->pre.size() != 1)
            c.graph[now]->cuts[1].insert(set<string> { now });
	    sort(Ct.begin(), Ct.end(), cmp_Depth2);
	    int Depth = Ct[0].mindep;
            c.graph[now]->mindep = Ct[0].mindep;
            c.graph[now]->AreaFlow = Ct[0].AreaFlow;
            c.graph[now]->Area = Ct[0].Area;
            c.graph[now]->Rcut = Ct[0].names;
            for (unsigned int i = 0; i < Ct.size(); i++) {
		if (Ct[i].mindep == Depth)
		   L[Depth].push_back(Ct[i]);
		if (i > 40) break;
            }
		
            //cout << c.graph[now]->mindep << endl;
        }
        Output(c);
	vector<DoubleCut> dc[MAXDEP];
	for (int d = 1; d <= c.Dep; d++) {
	    dc[d].clear();
	    printf("%d\n", L[d].size());
	    for (unsigned int i = 0; i < L[d].size(); i++) {
		if (L[d][i].names.size() == (unsigned)Luts) continue;
		for (unsigned int j = 0; j < L[d].size(); j++) {
		    if (i == j) continue;
		    if (L[d][i].Name == L[d][j].Name) continue;
		    //printf("%d %d\n", i, j);
		    set<string> st = L[d][i].names;
		    st.insert(L[d][j].names.begin(), L[d][j].names.end());
		    if (st.size() >= (unsigned)Luts) continue;
		    string name1 = L[d][i].Name;
		    string name2 = L[d][j].Name;
                    int area = 0, dep = d;
                    double AF = 0;
                    for (string nt : st) {
                        area += c.graph[nt]->Area;
                        AF += c.graph[nt]->AreaFlow;
                    }
                    area++;
		    /*Calculate Fout*/
		    set<string> temp;
		    temp.clear();
		    for (string strg : c.graph[name1]->suc) temp.insert(strg);
                    for (string strg : c.graph[name2]->suc) temp.insert(strg);
		    /*              */
		    AF /= max(1.0, 1.0 * temp.size());
		    if (area < c.graph[name1]->Area + c.graph[name2]->Area /*|| 
			(area == c.graph[name1]->Area + c.graph[name2]->Area && 
			AF < c.graph[name1]->AreaFlow + c.graph[name2]->AreaFlow)*/)
		    dc[d].push_back(DoubleCut(st, name1, name2, dep, area, AF)); 
                }
            }
	    sort(dc[d].begin(), dc[d].end(), Dcmp_Depth2);
	    printf("%d\n", dc[d].size());
	    for (unsigned int i = 0; i < dc[d].size(); i++) {
		string name1 = dc[d][i].Name1;
		string name2 = dc[d][i].Name2;
		if (!c.graph[name1]->Rdcut.empty() || !c.graph[name2]->Rdcut.empty()) continue;
		//printf("%s %s\n", name1.c_str(), name2.c_str());
		c.graph[name1]->Rdcut = dc[d][i].names;
		c.graph[name2]->Rdcut = dc[d][i].names;
		c.graph[name1]->Partner = name2;
		c.graph[name2]->Partner = name1;
            }
        }
	printf("Flag!!!\n");
        Output2(c);
	//cout << c.abc_lut_area << endl;
        clock_t finish = clock();
        total_time = (double) (finish - start) / CLOCKS_PER_SEC;
        cout << "Run time: " << total_time << "s" << endl;
   }

    return 0;
}

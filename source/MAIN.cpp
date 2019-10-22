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

using namespace std;

FILE *out;
map<string, bool> vis;

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

void Output(Circuit c) {
    queue<string> Q;
    vis.clear();
    int area = 0, dep = 0;
    for (string s : c.output) {
        Q.push(s);
        vis[s] = 1;
        dep = max(dep, c.graph[s]->mindep);
        //cout << s << ":" << c.graph[s]->mindep << endl;
    }
    while (!Q.empty()) {
        string now = Q.front();
        Q.pop();
        if (c.graph[now]->pre.size() == 0)
            continue;
        fprintf(out, "input:\n");
        //cout << now << ":" << c.graph[now]->mindep << endl;
        area++;
        for (string st : c.graph[now]->Rcut) {
            fprintf(out, "%s ", st.c_str());
            if (!vis[st]) {
                Q.push(st);
                vis[st] = 1;
            }
        }
        fprintf(out, "%s\n", ("\noutput:\n" + now).c_str());
    }
    fprintf(stdout, "Area: %d\nDepth: %d\n", area, dep - 1);
}

int main(int argc, char *argv[]) {
    vector<string> benchmark;
    queue<string> Q;
    map<string, int> ind;
    int Luts = 6;
    unsigned int C = 15;  // record the most C Cuts
    string dir = "benchmark/ISCAS85";
    string outdir = "result/ISCAS85";
    get_file_name(dir, benchmark);

    for (string str : benchmark) {
	cout << str << endl;
        double total_time;
        clock_t start = clock();

        Circuit c = Circuit(dir + "/" + str);  // already synthesized
        //c.write_dot();

        out = fopen((outdir + "/" + str + ".out").c_str(), "w");

        for (pair<string, Var*> p : c.graph) {
            ind[p.first] = p.second->pre.size();
	    p.second->mindep = 0;
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
	    for (int k = 1; k <= Luts; k++)
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

                    Cuts[k].push_back(Cut(ct, dep, area, AF));
		    Ct.push_back(Cut(ct, dep, area, AF));
                /*Compare dep*/
                /*if (dep < c.graph[now]->mindep) {
                 c.graph[now]->mindep = dep;
                 c.graph[now]->AreaFlow = AF;
                 c.graph[now]->Area = area;
                 c.graph[now]->Rcut = ct;
                 continue;
                 }*/

                /*Compare Area Flow*/
                /*if (dep == c.graph[now]->mindep && AF < c.graph[now]->AreaFlow) {
                 c.graph[now]->mindep = dep;
                 c.graph[now]->AreaFlow = AF;
                 c.graph[now]->Area = area;
                 c.graph[now]->Rcut = ct;
                 continue;
                 }*/

                /*Compare Area*/
                /*if (dep == c.graph[now]->mindep && AF == c.graph[now]->AreaFlow && area < c.graph[now]->Area) {
                 c.graph[now]->mindep = dep;
                 c.graph[now]->AreaFlow = AF;
                 c.graph[now]->Area = area;
                 c.graph[now]->Rcut = ct;
                 continue;
                 }*/

            }
	    //if (now == "G473") 
            //		cout << Cuts.size() << endl;
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
            /*for (string s : Cuts[0].names)
             cout << s << endl;*/
	    sort(Ct.begin(), Ct.end(), cmp_Depth2);
            c.graph[now]->mindep = Ct[0].mindep;
            c.graph[now]->AreaFlow = Ct[0].AreaFlow;
            c.graph[now]->Area = Ct[0].Area;
            c.graph[now]->Rcut = Ct[0].names;
            //cout << c.graph[now]->mindep << endl;
        }
        Output(c);
	//cout << c.abc_lut_area << endl;
        clock_t finish = clock();
        total_time = (double) (finish - start) / CLOCKS_PER_SEC;
        cout << "Run time: " << total_time << "s" << endl;
    }

    return 0;
}

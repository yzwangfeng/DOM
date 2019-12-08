/*
 * main.cpp
 *
 */

#include "ABC.h"
#include "Circuit.h"
#include <map>
#include <set>
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
map<string, int> vis;

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
    multiset<Cut> Q;
    vis.clear();
    int area = 0;
    for (string s : c.output) {
		if (c.graph[s]->pre.size() == 0) continue;
        Q.insert(c.graph[s]->Fcut);
		vis[s] = 1;
		//cout << c.graph[s]->Fcut.Name << endl;
		//cout << c.graph[s]->Fcut.mindep << endl;
    }
    //printf("%d\n", Q.size());
    while (!Q.empty()) {
		//printf("start\n");
        set<Cut>::iterator nw = Q.begin();
		string now = nw->Name;
        Q.erase(nw);
		//cout << now << endl;
        if (c.graph[now]->pre.size() == 0)
            continue;
		if (c.graph[now]->Rcut.size() == Luts) {
			fprintf(out2, "input:\n");
			area++;
			for (string st : c.graph[now]->Rcut) {
				fprintf(out2, "%s ", st.c_str());
				if (!vis[st]) {
					Q.insert(c.graph[st]->Fcut);
					vis[st] = 1;
				}
			}
			fprintf(out2, "%s\n", ("\noutput:\n" + now).c_str());
		} else {
			vector<DoubleCut> L;
			L.clear();
			for (Cut itr : Q) {
				//cout << "HH" << endl;
				string nxt = itr.Name;
				if (nxt == now) continue;
				//cout << "HH" + nxt << endl;
				set<string> tp;
				tp.clear();
				tp.insert(c.graph[now]->suc.begin(), c.graph[now]->suc.end());
				tp.insert(c.graph[nxt]->suc.begin(), c.graph[nxt]->suc.end());
				int sucsize = tp.size();
				//printf("%d %s\n", sucsize, nxt.c_str());
				for (int i = 2; i < Luts; i++) {
					for (set<string> ct : c.graph[nxt]->cuts[i]) {
						set<string> tmp = c.graph[now]->Rcut;
						tmp.insert(ct.begin(), ct.end());
						if (tmp.size() >= Luts) continue;
						int dep = 0, area = 0;
						double AF = 0;
						for (string nt : tmp) {
							dep = max(dep, c.graph[nt]->mindep);
							area += c.graph[nt]->Area;
							AF += c.graph[nt]->AreaFlow;
						}
						dep++, area++;
						AF /= max(1.0, 1.0 * sucsize);

						L.push_back(DoubleCut(tmp, now, nxt, dep, area, AF));
					}
				}
			}
			//printf("size %d\n", L.size());
			sort(L.begin(), L.end(), Dcmp_Depth2);
			if (L.size() == 0) {
				fprintf(out2, "input:\n");
				area++;
				for (string st : c.graph[now]->Rcut) {
					fprintf(out2, "%s ", st.c_str());
					if (!vis[st]) {
						Q.insert(c.graph[st]->Fcut);
						vis[st] = 1;
					}
				}
				fprintf(out2, "%s\n", ("\noutput:\n" + now).c_str());
			}
			else {
				fprintf(out2, "input:\n");
				area++;
				for (string st : L[0].names) {
					//cout << st << endl;
					fprintf(out2, "%s ", st.c_str());
					if (!vis[st]) {
						Q.insert(c.graph[st]->Fcut);
						vis[st] = 1;
					}
				}
				fprintf(out2, "%s\n", ("\noutput:\n" + L[0].Name1 + " " + L[0].Name2).c_str());
				set<Cut>::iterator it;
				//cout << "HH\n";
				for (it = Q.begin(); it != Q.end(); it++)
					if (it->Name == L[0].Name2) break;
				Q.erase(it);
			}
		}
		//Q.erase(nw);
		//for (Cut stt : Q)
		//	cout << stt.Name << endl;
	}
    fprintf(stdout, "Area: %d\nDepth: %d\n", area, c.Dep);
}
int main(int argc, char *argv[]) {
    vector<string> benchmark;
    queue<string> Q;
    map<string, int> ind;
    unsigned int C = 8;  // record the most C Cuts
    string dir = "benchmark/ISCAS85";
    string outdir = "result/ISCAS85";
    get_file_name(dir, benchmark);

   for (string str : benchmark) {
        cout << str << endl;
	//string str = "arbiter.blif"; {
        double total_time;
        clock_t start = clock();

        Circuit c = Circuit(dir + "/" + str);  // already synthesized
        //c.write_dot();
exit(1);
        out1 = fopen((outdir + "/" + str + ".out1").c_str(), "w");
        out2 = fopen((outdir + "/" + str + ".out2").c_str(), "w");

		//for (int i = 0; i < MAXDEP; i++)
		//	L[i].clear();

        for (pair<string, Var*> p : c.graph) {
            ind[p.first] = p.second->pre.size();
			p.second->mindep = 0;
			p.second->Rcut.clear();
			//			p.second->Rdcut.clear();
		}

        for (string s : c.input) {           // Initial queue
            c.graph[s]->cuts[1].insert(set<string> { s });
            c.graph[s]->mindep = 0;
            c.graph[s]->Area = 0;
            c.graph[s]->AreaFlow = 0;
			c.graph[s]->Fcut = Cut(set<string>{s}, s, 0, 0, 0);
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
                sort(Cuts[k].begin(), Cuts[k].end(), cmp_Depth);
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
			sort(Ct.begin(), Ct.end(), cmp_Depth);
			int Depth = Ct[0].mindep;
            c.graph[now]->mindep = Ct[0].mindep;
            c.graph[now]->AreaFlow = Ct[0].AreaFlow;
            c.graph[now]->Area = Ct[0].Area;
            c.graph[now]->Rcut = Ct[0].names;
			c.graph[now]->Fcut = Ct[0];
		
            //cout << c.graph[now]->mindep << endl;
        }
        Output(c);
        Output2(c);
        clock_t finish = clock();
        total_time = (double) (finish - start) / CLOCKS_PER_SEC;
        cout << "Run time: " << total_time << "s" << endl;
	}

    return 0;
}

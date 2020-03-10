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
vector<string> Top;
map<string, int> counter;
map<string, int> vis;
map<string, int> mark;

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
    return A.mindep < B.mindep || (A.mindep == B.mindep && A.Area < B.Area)
            || (A.mindep == B.mindep && A.Area == B.Area && A.AreaFlow < B.AreaFlow);
}

bool cmp_AreaFlow(Cut A, Cut B) {
    return A.AreaFlow < B.AreaFlow || (A.AreaFlow == B.AreaFlow && A.fin < B.fin)
            || (A.AreaFlow == B.AreaFlow && A.fin == B.fin && A.mindep < B.mindep);
}

bool cmp_Area(Cut A, Cut B) {
    return A.Area < B.Area || (A.Area == B.Area && A.fin < B.fin)
            || (A.Area == B.Area && A.fin == B.fin && A.mindep < B.mindep);
}

bool Dcmp_Depth(DoubleCut A, DoubleCut B) {
    return A.mindep < B.mindep || (A.mindep == B.mindep && A.AreaFlow < B.AreaFlow)
            || (A.mindep == B.mindep && A.AreaFlow == B.AreaFlow && A.Area < B.Area);
}

bool Dcmp_Depth2(DoubleCut A, DoubleCut B) {
    return A.mindep < B.mindep || (A.mindep == B.mindep && A.Area < B.Area)
            || (A.mindep == B.mindep && A.Area == B.Area && A.AreaFlow < B.AreaFlow);
}
int DFS(Circuit &c, string now) {
	int res = 1;
	if (c.graph[now]->Rcut.size() == 0)
		return 1;
	mark[now] = 1;
	for (string s : c.graph[now]->Rcut) {
		if (counter[s] == 1 && mark[s] == 0)
			res += DFS(c, s);
	}
	return res;
}
void Recovery(Circuit &c) {
	queue<string> Q;
    vis.clear();
	for (string st : Top) {
		counter[st] = 0;
	}
    int area = 0, dep = 0;
    for (string s : c.output) {
        Q.push(s);
        vis[s] = 1;
        dep = max(dep, c.graph[s]->mindep);
    }
    while (!Q.empty()) {
        string now = Q.front();
        Q.pop();
		c.graph[now]->fin = counter[now];
        if (c.graph[now]->pre.size() == 0)
            continue;
        area++;
        for (string st : c.graph[now]->Fcut.names) {
			counter[st] = counter[st] + 1;
            if (!vis[st]) {
                Q.push(st);
                vis[st] = 1;
            }
        }
    }
	for (string now : Top) {
		if (c.graph[now]->pre.size() == 0) {
			c.graph[now]->Area = 1;
			c.graph[now]->AreaFlow = 1.0 / counter[now];
			continue;
		}
		if (vis[now] == 0) {
			c.graph[now]->Area = 1;
			c.graph[now]->AreaFlow = 1.0;
			continue;
		}
		mark.clear();
		c.graph[now]->Area = DFS(c, now);
		c.graph[now]->AreaFlow = c.graph[now]->Area;
		for (string st : c.graph[now]->Rcut)
			c.graph[now]->AreaFlow += c.graph[st]->Area;
		c.graph[now]->AreaFlow = c.graph[now]->AreaFlow / max(1.0, 1.0 * counter[now]);
	}
}
pair<int, int> Output(Circuit &c) {
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
        for (string st : c.graph[now]->Fcut.names) {
            fprintf(out1, "%s ", st.c_str());
            if (!vis[st]) {
                Q.push(st);
                vis[st] = 1;
            }
        }
        fprintf(out1, "%s\n", ("\noutput:\n" + now).c_str());
    }
    fprintf(stdout, "DOLM single_area = %d, single_depth = %d\n", area, dep);
    c.Dep = dep;
    return make_pair(area, dep);
}
pair<int, int> Output2(Circuit &c) {
    multiset<Cut> Q;
    vis.clear();
    int area = 0;
    for (string s : c.output) {
        if (c.graph[s]->pre.size() == 0)
            continue;
        Q.insert(c.graph[s]->Fcut);
        vis[s] = 1;
        //cout << c.graph[s]->Fcut.Name << endl;
        //cout << c.graph[s]->Fcut.mindep << endl;
    }
    //printf("%d\n", Q.size());
    while (!Q.empty()) {
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
                string nxt = itr.Name;
                if (nxt == now)
                    continue;
                set<string> tp;
                tp.clear();
                tp.insert(c.graph[now]->suc.begin(), c.graph[now]->suc.end());
                tp.insert(c.graph[nxt]->suc.begin(), c.graph[nxt]->suc.end());
                int sucsize = tp.size();
                for (int i = 2; i < Luts; i++) {
                    for (set<string> ct : c.graph[nxt]->cuts[i]) {
                        set<string> tmp = c.graph[now]->Rcut;
                        tmp.insert(ct.begin(), ct.end());
                        if (tmp.size() >= Luts)
                            continue;
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
            } else {
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
                    if (it->Name == L[0].Name2)
                        break;
                Q.erase(it);
            }
        }
    }
    fprintf(stdout, "DOLM dual_area = %d, dual_depth = %d\n", area, c.Dep);
    return make_pair(area, c.Dep);
}

int main(int argc, char *argv[]) {
    vector<string> benchmark;
    queue<string> Q;
    map<string, int> ind;
    unsigned int C = 10;  // record the most C Cuts
    string benchmark_set = "ISCAS85";
    string dir = "benchmark/" + benchmark_set;
    string outdir = "result/" + benchmark_set;
    get_file_name(dir, benchmark);

    fstream fout(outdir + "/overall.csv", ios::out);
    if (!fout.is_open()) {
        return 0;
    }
    fout << "case,PI,PO,size,area_single,depth_single,area_dual,depth_dual,abc_dep,abcarea_single,abcarea_dual\n";
    for (string str : benchmark) {
        cout << str << endl;
        //string str = "s38417.blif"; {
        double total_time;
        clock_t start = clock();

        Circuit c = Circuit(dir + "/" + str);  // already synthesized
        //c.write_dot();

        out1 = fopen((outdir + "/" + str + ".out1").c_str(), "w");
        out2 = fopen((outdir + "/" + str + ".out2").c_str(), "w");

        Top.clear();
		counter.clear();

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
			c.graph[s]->fin = 0;
			c.graph[s]->Rcut = set<string> { s };
            c.graph[s]->Fcut = Cut(set<string> { s }, s, 0, 0, 0, 0);
			Top.push_back(s);
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
			Top.push_back(now);
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
			c.graph[now]->fin = 0;
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

                    Cuts[k].push_back(Cut(ct, now, dep, area, 0, AF));
                    Ct.push_back(Cut(ct, now, dep, area, 0, AF));
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
			unsigned int Ctmp = Ct.size();
			c.graph[now]->Rcut = Ct[0].names;
            c.graph[now]->Fcut = Ct[0];
        }

		for (int turn = 1; turn <= 10; turn++) {
		Recovery(c);

		for (string now : Top) {
			//cout << now << " " << c.graph[now]->AreaFlow << " "  << c.graph[now]->Area << endl;
			if (c.graph[now]->pre.size() == 0)
				continue;
			for (int i = 0; i <= Luts; i++)
				c.graph[now]->cuts[i].clear();
		}
		
		for (string now : Top) {
			if (c.graph[now]->pre.size() == 0)
				continue;
            //c.graph[now]->cuts[1].insert(set<string> { now });
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

            /*Calculate dep, areaflow, exact area*/
            vector<Cut> Cuts[10], Ct;
            for (int k = 1; k <= Luts; k++) {
                for (set<string> ct : c.graph[now]->cuts[k]) {
                    int dep = 0, area = 0, fin = 0;
                    double AF = c.graph[now]->Area;
                    for (string nt : ct) {
                        dep = max(dep, c.graph[nt]->mindep);
                        AF += c.graph[nt]->AreaFlow;
                        fin += c.graph[nt]->fin;
                    }
                    dep++;
                    AF /= max(1.0, 1.0 * c.graph[now]->fin);

					if (dep <= c.graph[now]->mindep) {
						Cuts[k].push_back(Cut(ct, now, dep, area, fin, AF));
						Ct.push_back(Cut(ct, now, dep, area, fin, AF));
					}
                }
            }
            for (int k = 1; k <= Luts; k++) {
                sort(Cuts[k].begin(), Cuts[k].end(), cmp_AreaFlow);
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
            sort(Ct.begin(), Ct.end(), cmp_AreaFlow);
			if (Ct.size() == 0) continue;
            int Depth = Ct[0].mindep;
			if (c.graph[now]->Area > Ct[0].Area) {
				c.graph[now]->AreaFlow = Ct[0].AreaFlow;
				unsigned int Ctmp = Ct.size();
				c.graph[now]->Rcut = Ct[0].names;
				c.graph[now]->Fcut = Ct[0];
			}
		}

		Recovery(c);

		for (string now : Top) {
			if (c.graph[now]->pre.size() == 0)
				continue;
			for (int i = 0; i <= Luts; i++)
				c.graph[now]->cuts[i].clear();
		}
		
		for (string now : Top) {
			if (c.graph[now]->pre.size() == 0)
				continue;
            //c.graph[now]->cuts[1].insert(set<string> { now });
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

            /*Calculate dep, areaflow, exact area*/
            vector<Cut> Cuts[10], Ct;
            for (int k = 1; k <= Luts; k++) {
                for (set<string> ct : c.graph[now]->cuts[k]) {
                    int dep = 0, area = 0, fin = 0;
                    double AF = 1;
                    for (string nt : ct) {
                        dep = max(dep, c.graph[nt]->mindep);
						mark.clear();
                        area += DFS(c, nt);
                        fin += c.graph[nt]->fin;
                    }
                    dep++, area++;

					if (dep <= c.graph[now]->mindep) {
						Cuts[k].push_back(Cut(ct, now, dep, area, fin, AF));
						Ct.push_back(Cut(ct, now, dep, area, fin, AF));
					}
                }
            }
            for (int k = 1; k <= Luts; k++) {
                sort(Cuts[k].begin(), Cuts[k].end(), cmp_Area);
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
            sort(Ct.begin(), Ct.end(), cmp_Area);
			if (Ct.size() == 0) continue;
            int Depth = Ct[0].mindep;
			if (c.graph[now]->Area > Ct[0].Area) {
				c.graph[now]->Area = Ct[0].Area;
				unsigned int Ctmp = Ct.size();
				c.graph[now]->Rcut = Ct[0].names;
				c.graph[now]->Fcut = Ct[0];
			}
		}
		}

        pair<int, int> single = Output(c);
        pair<int, int> dual = Output2(c);
        fout << str.substr(0, str.find(".")) << ',' << c.input.size() << ',' << c.output.size() << ','
                << c.graph.size() << ',' << single.first << ',' << single.second << ',' << dual.first << ','
			 << dual.second << "," << c.abc_res.second.second << "," << c.abc_res.first << "," << c.abc_res.second.first << endl;

        clock_t finish = clock();
        total_time = (double) (finish - start) / CLOCKS_PER_SEC;
        cout << "Run time = " << total_time << "s" << endl;
        cout << "--------------------------------------------------------------------------" << endl;
    }
    fout.close();
    return 0;
}

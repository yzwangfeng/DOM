/*
 * main.cpp
 *
 */

#include "ABC.h"
#include "Circuit.h"
#include "Refine.h"
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
map<string, int> counter;   // fanin ref (= fanout)
map<string, int> used;
map<string, int> vis;
map<string, int> mark;      // only used in DFS

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

int DFS(Circuit &c, string now) {       // compute |MFFC(now)| by DFS
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
int Recovery(Circuit &c) {      // re-compute heuristic functions after an iteration
    queue<string> Q;
    vis.clear();
    used.clear();
    counter.clear();
    int area = 0, dep = 0;
    for (string s : c.output) {
        Q.push(s);
        vis[s] = 1;
        used[s] = 1;
        dep = max(dep, c.graph[s]->mindep);
    }

    //for (string s : c.output)
    //	c.graph[s]->mindep = dep;

    while (!Q.empty()) {
        string now = Q.front();
        Q.pop();
        //c.graph[now]->fin = counter[now];
        c.graph[now]->fin = (1.0 * c.graph[now]->fin + 2 * counter[now]) / 3;   // fanin refs used in abc
        if (c.graph[now]->pre.size() == 0)
            continue;
        area++;
        for (string st : c.graph[now]->Fcut.names) {
            //int ndep = c.graph[now]->mindep - 1;
            counter[st] = counter[st] + 1;
            used[st] = 1;
            //c.graph[st]->mindep = min(c.graph[st]->mindep, ndep);
            if (!vis[st]) {
                //c.graph[st]->mindep = ndep;
                Q.push(st);
                vis[st] = 1;
            }
        }
    }
    printf("%d %d\n", area, dep);
    return area;
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
    fclose(out1);
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
                        int tmpdep = 0;
                        for (string nt : ct)
                            tmpdep = max(tmpdep, c.graph[nt]->mindep);
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
    fclose(out2);
    return make_pair(area, c.Dep);
}

void Dereference(Circuit &c, string now, int &mem) {   // update fanin refs according to the best cuts in this iteration
    if (c.graph[now]->pre.size() == 0)
        return;
    mem++;
    for (string s : c.graph[now]->Rcut) {
        if (counter[s] > 0) {
            counter[s] = counter[s] - 1;
            if (counter[s] == 0) {
                used[s] = 0;
                Dereference(c, s, mem);
            }
        }
    }
}

int getArea(Circuit &c, string now) {
    if (c.graph[now]->pre.size() == 0)
        return 0;

    int res = 1;
    for (string s : c.graph[now]->Rcut) {
        if (counter[s] == 0)
            res += getArea(c, s);
    }
    return res;
}

void reference(Circuit &c, string now) {       // opposite to dereference
    if (c.graph[now]->pre.size() == 0)
        return;
    vis[now] = 1;
    for (string s : c.graph[now]->Rcut) {
        used[s] = 1;
        counter[s]++;
        if (vis[s] == 0)
            reference(c, s);
    }
}

int main(int argc, char *argv[]) {
    vector<string> benchmark;
    queue<string> Q;    // for topological sort
    map<string, int> ind;
    unsigned int C = 20;   // record the most C Cuts
    string benchmark_set = "EPFL";
    string dir = "benchmark/" + benchmark_set;
    string outdir = "result/" + benchmark_set;
    get_file_name(dir, benchmark);

    fstream fout(outdir + "/overall.csv", ios::out);
    fstream fout1(outdir + "/lut1.csv", ios::out);
    fstream fout2(outdir + "/lut2.csv", ios::out);
    if (!fout.is_open()) {
        return 0;
    }
    fout
            << "case,PI,PO,size,area_single,depth_single,area_dual,depth_dual,final_area,abc_dep,abcarea_single,abcarea_dual,if_time,DOLM_time\n";
    fout1 << "case,1,2,3,4,5,6\n";
    fout2 << "case,1,2,3,4,5,6\n";
    for (string str : benchmark) {
        cout << str << endl;
        //string str = "voter.blif"; {
        double total_time;
        clock_t start = clock();

        Circuit c = Circuit(dir + "/" + str);  // invoke abc, already synthesized
        //c.write_dot();

        clock_t IF_END = clock();

        out1 = fopen((outdir + "/" + str + ".out1").c_str(), "w");
        out2 = fopen((outdir + "/" + str + ".out2").c_str(), "w");

        Top.clear();        // store the topological order
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
                ind[nt]--;      // in degree
                if (ind[nt] == 0)
                    Q.push(nt);
            }
        }

        //Enumerate Priority Cuts
        while (!Q.empty()) {
            string now = Q.front();
            //cout << now << endl;
            Top.push_back(now); // Top = Q
            Q.pop();
            c.graph[now]->cuts[1].insert(set<string> { now });  // the trivial cut
            assert(c.graph[now]->pre.size() <= 2);
            if (c.graph[now]->pre.size() == 2) {        // merge cuts of its two children
                string input1 = c.graph[now]->pre[0];
                string input2 = c.graph[now]->pre[1];
                c.graph[input1]->fin++;
                c.graph[input2]->fin++;
                for (int t = 1; t <= Luts; t++)
                    for (set<string> c1 : c.graph[input1]->cuts[t]) {
                        for (int k = 1; k <= Luts; k++)
                            for (set<string> c2 : c.graph[input2]->cuts[k]) {
                                set<string> st = c1;
                                st.insert(c2.begin(), c2.end());
                                if (st.size() > (unsigned) Luts)    // constraint
                                    continue;
                                c.graph[now]->cuts[st.size()].insert(st);
                            }
                    }
            }

            if (c.graph[now]->pre.size() == 1) {    // inherit cuts from its child
                string input1 = c.graph[now]->pre[0];
                c.graph[input1]->fin++;
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
                for (unsigned int i = 0; i < min(C, len); i++) {        // store C priority cuts for each input
                    c.graph[now]->cuts[k].insert(Cuts[k][i].names);
                    //cout << Cuts[i].mindep << endl;
                }
            }
            //if (c.graph[now]->pre.size() != 1)
            c.graph[now]->cuts[1].insert(set<string> { now });
            sort(Ct.begin(), Ct.end(), cmp_Depth);
            c.graph[now]->mindep = Ct[0].mindep;
            c.graph[now]->AreaFlow = Ct[0].AreaFlow;
            c.graph[now]->Area = Ct[0].Area;
            c.graph[now]->Rcut = Ct[0].names;
            c.graph[now]->Fcut = Ct[0];
        }

        for (int lastarea = (1 << 30);;) {  // a large number, iterate until convergence
            int tmper = Recovery(c);
            if (tmper == lastarea)
                break;
            lastarea = tmper;
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

                set<string> Rcut_mem = c.graph[now]->Rcut;
                int Area_mem = 0;

                if (used[now])
                    Dereference(c, now, Area_mem);

                /*Calculate dep, areaflow, exact area*/
                vector<Cut> Cuts[10], Ct;
                for (int k = 1; k <= Luts; k++) {
                    for (set<string> ct : c.graph[now]->cuts[k]) {
                        int dep = 0, area = 0, fin = 0;
                        double AF = 0;
                        for (string nt : ct) {
                            dep = max(dep, c.graph[nt]->mindep);
                            AF += c.graph[nt]->AreaFlow;
                            fin += counter[nt];
                        }
                        dep++;
                        if (dep > c.graph[now]->mindep)
                            continue;
                        c.graph[now]->Rcut = ct;
                        area = getArea(c, now);
                        AF = (AF + area) / max(1.0, 1.0 * fin);
                        //printf("%d\n", area);
                        Cuts[k].push_back(Cut(ct, now, dep, area, fin, AF));
                        Ct.push_back(Cut(ct, now, dep, area, fin, AF));
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
                if (Ct.size() == 0 || Ct[0].Area > Area_mem)
                    c.graph[now]->Rcut = Rcut_mem;
                else {
                    c.graph[now]->AreaFlow = Ct[0].AreaFlow;
                    c.graph[now]->mindep = Ct[0].mindep;
                    c.graph[now]->Rcut = Ct[0].names;
                    c.graph[now]->Fcut = Ct[0];
                }
                vis.clear();
                if (used[now])
                    reference(c, now);
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

                set<string> Rcut_mem = c.graph[now]->Rcut;
                int Area_mem = 0;

                if (used[now])
                    Dereference(c, now, Area_mem);

                /*Calculate dep, areaflow, exact area*/
                vector<Cut> Cuts[10], Ct;
                for (int k = 1; k <= Luts; k++) {
                    for (set<string> ct : c.graph[now]->cuts[k]) {
                        int dep = 0, area = 0, fin = 0;
                        double AF = c.graph[now]->Area;
                        for (string nt : ct) {
                            dep = max(dep, c.graph[nt]->mindep);
                            fin += counter[nt];
                        }
                        dep++;
                        if (dep > c.graph[now]->mindep)
                            continue;
                        c.graph[now]->Rcut = ct;
                        area = getArea(c, now);
                        //printf("%d\n", area);
                        Cuts[k].push_back(Cut(ct, now, dep, area, fin, AF));
                        Ct.push_back(Cut(ct, now, dep, area, fin, AF));
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
                if (Ct.size() == 0 || Ct[0].Area > Area_mem)
                    c.graph[now]->Rcut = Rcut_mem;
                else {
                    //c.graph[now]->AreaFlow = Ct[0].AreaFlow;
                    c.graph[now]->mindep = Ct[0].mindep;
                    c.graph[now]->Rcut = Ct[0].names;
                    c.graph[now]->Fcut = Ct[0];
                }
                vis.clear();
                if (used[now])
                    reference(c, now);
            }

        }

        pair<int, int> single = Output(c);
        pair<int, int> dual = Output2(c);

        Refine *match1, *match2;
        pair<vector<int>, vector<int> > lut_display;
        match1 = new Refine();
        match2 = new Refine();
        int v1 = match1->getMatch(outdir + "/" + str + ".out1");
        int v2 = match2->getMatch(outdir + "/" + str + ".out2");
        int Refine_res = min(v1, v2);
        printf("%d %d %s\n", v1, v2, (outdir + "/" + str + ".out").c_str());
        if (v1 < v2)
            lut_display = match1->output(outdir + "/" + str + ".out");
        else
            lut_display = match2->output(outdir + "/" + str + ".out");

        clock_t finish = clock();

        fout << str.substr(0, str.find(".")) << ',' << c.input.size() << ',' << c.output.size() << ','
                << c.graph.size() << ',' << single.first << ',' << single.second << ',' << dual.first << ','
                << dual.second << "," << Refine_res << "," << c.abc_res.second.second << ","
                << c.abc_res.first << "," << c.abc_res.second.first << ","
                << (double) (IF_END - start) / CLOCKS_PER_SEC << ","
                << (double) (finish - IF_END) / CLOCKS_PER_SEC << endl;
        fout1 << str.substr(0, str.find("."));
        for (int i = 1; i <= 6; i++)
            fout1 << "," << lut_display.first[i];
        fout1 << endl;
        fout2 << str.substr(0, str.find("."));
        for (int i = 1; i <= 6; i++)
            fout2 << "," << lut_display.second[i];
        fout2 << endl;
        total_time = (double) (finish - start) / CLOCKS_PER_SEC;
        cout << "Run time = " << total_time << "s" << endl;
    }

    fout.close();
    return 0;
}

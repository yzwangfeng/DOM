/*
 * main.cpp
 *
 */

#include "ABC.h"
#include "Circuit.h"
#include <dirent.h>
#include <queue>
#include <assert.h>
#include <string>
#include <vector>
#include <algorithm>

#define inf (1e20)

using namespace std;

void get_file_name(string path, vector<string> &files) {
    struct dirent *ptr;
    DIR *dir = opendir(path.c_str());
    while ((ptr = readdir(dir)) != NULL) {
        if (ptr->d_name[0] != '.') {  // skip . and ..
            files.push_back(ptr->d_name);
	    cout << ptr->d_name << endl;
        }
    }
    closedir(dir);
}

bool cmp_Depth(Cut A, Cut B)
{
    return A.mindep < B.mindep || (A.mindep == B.mindep && A.AreaFlow < B.AreaFlow) || (A.mindep == B.mindep && A.AreaFlow == B.AreaFlow && A.Area < B.Area);
}

bool cmp_Depth2(Cut A, Cut B)
{
    return A.mindep < B.mindep || (A.mindep == B.mindep && A.Area < B.Area) || (A.mindep == B.mindep && A.Area == B.Area && A.AreaFlow < B.AreaFlow);
}


void Output(Circuit c) {
     queue <string> Q;
     int area = 0;
     for (string s : c.output)
	Q.push(s);
     while (!Q.empty()) {
	string now = Q.front();
	Q.pop();
	if (c.graph[now]->pre.size() == 0) continue;
	cout << "input:\n";
	for (string st : c.graph[now]->Rcut) {
            cout << st << " ";
	    Q.push(st);
        }
	cout << "\noutput:\n" << now << endl;
     }
}

int main(int argc, char *argv[]) {
    vector<string> benchmark;
    queue <string> Q;
    map<string, int> ind;
    int Luts = 4;
    unsigned int C = 8; // record the most C Cuts
    string dir = "benchmark/test1";
    get_file_name(dir, benchmark);

    for (string str : benchmark) {
        double total_time;
        clock_t start = clock();

        Circuit c = Circuit(dir + "/" + str);  // already synthesized
	//c.write_dot();

	for (pair<string, Var*> p : c.graph) 
	    ind[p.first] = p.second->pre.size();

	for (string s : c.input) {           // Initial queue
            c.graph[s]->cuts.insert(set<string>{s});
	    c.graph[s]->mindep = 0;
	    c.graph[s]->Area = 0;
	    c.graph[s]->AreaFlow = 0;
	    for (string nt : c.graph[s]->suc) {
		ind[nt]--;
		if (ind[nt] == 0) Q.push(nt);
	    }
	}

	//Enumerate Cuts
	while (!Q.empty()) {
	    string now = Q.front();
	    //cout << now << endl;
	    Q.pop();
	    c.graph[now]->cuts.insert(set<string>{now});
	    assert(c.graph[now]->pre.size() <= 2);
	    if (c.graph[now]->pre.size() == 2) {
	        string input1 = c.graph[now]->pre[0];
	        string input2 = c.graph[now]->pre[1];

	        for (set<string> c1 : c.graph[input1]->cuts) {
		    for (set<string> c2: c.graph[input2]->cuts) {
		        set<string> st = c1;
		        st.insert(c2.begin(), c2.end());
		        if (st.size() > (unsigned)Luts) continue;
		        c.graph[now]->cuts.insert(st);
		    }
	        }
	    }

	    if (c.graph[now]->pre.size() == 1) {
                string input1 = c.graph[now]->pre[0];
                for (set<string> c1 : c.graph[input1]->cuts)
                    c.graph[now]->cuts.insert(c1);
            }

            for (string nt : c.graph[now]->suc) {
                ind[nt]--;
                if (ind[nt] == 0) Q.push(nt);
            }

	    /*Calculate dep, areaflow, exact area*/	
	    vector<Cut> Cuts;
	    c.graph[now]->mindep = INF;
            c.graph[now]->Area = INF;
            c.graph[now]->AreaFlow = inf;
	    for (set<string> ct : c.graph[now]->cuts) {
		int dep = 0, area = 0;
		double AF = 0;
		for (string nt : ct) {
		    dep = max(dep, c.graph[nt]->mindep);
		    area += c.graph[nt]->Area;
		    AF += c.graph[nt]->AreaFlow;
		}
		dep++, area++;
		AF /= max(1.0, 1.0 * c.graph[now]->suc.size());

		Cuts.push_back(Cut(ct, dep, area, AF));		
		
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
                /*if (dep == c.graph[now]->mindep &
 * & AF == c.graph[now]->AreaFlow && area < c.graph[now]->Area) {
                    c.graph[now]->mindep = dep;
                    c.graph[now]->AreaFlow = AF;
                    c.graph[now]->Area = area;
                    c.graph[now]->Rcut = ct;
                    continue;
                }*/

	    }
	    sort(Cuts.begin(), Cuts.end(), cmp_Depth);
	    c.graph[now]->cuts.clear();
	    unsigned int len = Cuts.size();
            //cout << "now:" << now << endl;
	    for (unsigned int i = 0; i < min(C, len); i++)
            {
		c.graph[now]->cuts.insert(Cuts[i].names);
		//cout << Cuts[i].mindep << endl;
            }
	    c.graph[now]->cuts.insert(set<string>{now});
            /*for (string s : Cuts[0].names)
		cout << s << endl;*/
	    c.graph[now]->mindep = Cuts[0].mindep;
            c.graph[now]->AreaFlow = Cuts[0].AreaFlow;
            c.graph[now]->Area = Cuts[0].Area;
            c.graph[now]->Rcut = Cuts[0].names;
        }
	Output(c);
        clock_t finish = clock();
        total_time = (double) (finish - start) / CLOCKS_PER_SEC;
        cout << "Run time: " << total_time << "s" << endl;
    }

    return 0;
}

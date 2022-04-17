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

map<string, int> id;
map<set<string>,Truthtable* > cut_to_truth[1000005];

map<char, int> ys;
struct Tre{
    int to[2];
    bool tag;
	Tre(){
        to[1]=0;
        to[0]=0;
        tag=0;
    }
}Trie[2000005];
map<int, double> area_map;
map<int, double> area_dual_map;
map<int, int> dual_match;
map<int, int> belong;
int f_trie[513], val_trie[513];
double areanow;
char s_trie[513];
int nstr[513];
int cnt_trie,my_cnt_trie,k_trie;
bool expression_bool();
bool term_bool();
bool factor_bool();
ifstream fffin("444.txt", ios::in);

bool term_bool(){
	char op=s_trie[my_cnt_trie];
	if(op=='!'){
		my_cnt_trie++;
		return !factor_bool();
	}
    return factor_bool();

}
bool expression_bool(){
	bool result=term_bool();
	bool more=true;
	while(more){
		char op=s_trie[my_cnt_trie];
		if(op==' '||op=='+'||op=='^'){
			my_cnt_trie++;
			bool value=term_bool();
			if(op=='^')result=result^value;
			else if(op==' ') result=result&value;		
            else result=result|value;
		}else{
			more=false;
		}
	}
 
	return result;
}
bool factor_bool(){
	bool result=0;
	char c=s_trie[my_cnt_trie];
	if(c=='('){
		my_cnt_trie++;
		result=expression_bool();
		my_cnt_trie++;
	}
    else if(c>='A'&&c<='Z'){
		result=val_trie[f_trie[ys[c]]];
		my_cnt_trie++;
	}
	else if(c=='!')result=term_bool();
	return result;
}
void ins_trie(int s_trie[],int match,int countc){
    int x=0,i=0;
    while(i<(1<<k_trie)){
		int j=s_trie[i];
		if(!Trie[x].to[j])Trie[x].to[j]=++cnt_trie,x=cnt_trie;
		else x=Trie[x].to[j];
		i++;
		}
		if(!match)area_map[x] = areanow;
        else{
            if(area_map.find(x)==area_map.end())
                area_map[x] = areanow;
            else
                area_map[x] = min(area_map[x], areanow);
            area_dual_map[x] = areanow;
        }
      //  cout << "###" << x << ' ' << Trie[19].tag << ' ' << areanow << ' ' << match << '\n';
      //  cout << Trie[0].to[1] << ' ' << Trie[Trie[0].to[1]].to[0] << '\n';
       cout << "##gg "<<x << ' ' ;
        Trie[x].tag = 1;
        belong[x] = countc;
        if (match != 0)
            dual_match[x] = (1 << match);
}
int getnum(char* s_trie,int len){
	int jsq = 0;
	for (int i = 0; i < len;i++){
		if(s_trie[i]>='A'&&s_trie[i]<='Z'){
			if(!ys[s_trie[i]])
				ys[s_trie[i]] = ++jsq;
		}
	}
	return jsq;
}
int ppppp = 0, qqqqq = 0;
void ins(int match, int countc)
{
    memset(s_trie,0,sizeof(s_trie));
	ys.clear();
	fffin.getline(s_trie, 256);
	if (strlen(s_trie) <= 1)
		fffin.getline(s_trie, 256);
	int len = strlen(s_trie);
    if(len<=1)
        return;
    k_trie = getnum(s_trie, len);
    //    cout << k_trie<<' '<<s_trie<<' '<<areanow << '\n';
	int num=1;
    for(int i=1;i<=k_trie;i++)num*=i,f_trie[i]=i;
    
    for(int i=0;i<num;i++){
        for(int t=0;t<(1<<k_trie);t++){
            for(int j=0;j<k_trie;j++)val_trie[j+1]=(t>>j)&(1);
			my_cnt_trie = 0;
			nstr[t]=expression_bool();
		}
	//	for(int j=0;j<(1<<k_trie);j++)cout<<nstr[j]<<' ';
      // cout<<"\n";

        ins_trie(nstr,match,countc);
      //  printf("%s\n", s_trie);
       cout <<'\n'<< s_trie<<' '<<match << '\n';
        next_permutation(f_trie + 1, f_trie + k_trie + 1);
    }
}
void presettings(){
	int ff = 0;
	int jsq = 0;
    int cnt=0;
    int logical;
    while (1)
    {
        fffin >>ff>> areanow;
       // cout << ff << ' ' << areanow <<' '<<logical<< "###\n";

        if (ff == 0)
            break;
		if(ff==1)ins(0,++cnt);
		else ins(++jsq,++cnt),ins(jsq,cnt);
    }
}
int check_equivalence(Truthtable *zz){
    int flag=1,rt=0;
    for(int i=0;i<(1<<zz->n);i++){
        if(Trie[rt].to[zz->num[i]])
            rt=Trie[rt].to[zz->num[i]];
        else return false;
        if(i==(1<<zz->n)-1)
            if(Trie[rt].tag)return rt;
            else return 0;
    }
}



void get_file_name(string path, vector<string> &files) {
    struct dirent *ptr;
    //path = "benchmark";
    DIR *dir = opendir(path.c_str());
    while ((ptr = readdir(dir)) != NULL) {
      //  printf("1122 %s\n", ptr->d_name);
        if (ptr->d_name[0] != '.')
        { // skip . and ..
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
double Recovery(Circuit &c) {      // re-compute heuristic functions after an iteration
    queue<string> Q;
    vis.clear();
    used.clear();
    counter.clear();
    double area = 0;
    int dep = 0;
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
        area += area_map[check_equivalence(cut_to_truth[id[now]][c.graph[now]->Fcut.names])];
     //   cout<<now<<' '<<check_equivalence(cut_to_truth[id[now]][c.graph[now]->Fcut.names])<<'\n';
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
    //printf("%lf %d\n", area, dep);
    return area;
}
pair<int, int> Output(Circuit &c) {
    queue<string> Q;
    vis.clear();
    double area = 0;
    int dep = 0;
    for(auto s : Top){
        cout<<"Gate: "<<s<<"\n";
        for(int t=3;t<=3;t++){
           // cout<<"luts count: "<<t<<'\n'; 
            
            for (auto c1 : c.graph[s]->cuts[t]){
               cout<<"value is: "<<check_equivalence(cut_to_truth[id[s]][c1])<<' '<<s<<"   ";
            
            for(auto x:c1){
                cout<<x<<' ';
            }
            cout<<'\n';
            }
        }
        //cout<<'\n';
        
            

    }
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
      //  cout << "wtf" << check_equivalence(cut_to_truth[id[now]][c.graph[now]->Fcut.names]) << '\n';
        area += area_map[check_equivalence(cut_to_truth[id[now]][c.graph[now]->Rcut])];
        // printf("single %s %lf\n", now.c_str(), area);
        for (string st : c.graph[now]->Fcut.names)
        {
            fprintf(out1, "%s ", st.c_str());
            if (!vis[st]) {
                Q.push(st);
                vis[st] = 1;
            }
        }
        fprintf(out1, "%s\n", ("\noutput:\n" + now).c_str());
    }
    fprintf(stdout, "DOLM single_area = %lf, single_depth = %d\n", area, dep);
    c.Dep = dep;
    fclose(out1);
    return make_pair(area, dep);
}
pair<int, int> Output2(Circuit &c) {
    multiset<Cut> Q;
    vis.clear();
    double area = 0;
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
        ///cout << now << endl; 
       // printf("important %s %d\n",now.c_str(),check_equivalence(cut_to_truth[id[now]][c.graph[now]->Rcut]));

        if (c.graph[now]->pre.size() == 0)
            continue;
        if (c.graph[now]->Rcut.size() == Luts) {
            fprintf(out2, "input:\n");
            area+=area_map[check_equivalence(cut_to_truth[id[now]][c.graph[now]->Rcut])];
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

               //         printf("merging %s %s %d %d\n", now.c_str(), nxt.c_str(),tmp.size(),Luts);
                        if (tmp.size() >= Luts )
                            continue;
                       
                        int ttt = dual_match[check_equivalence(cut_to_truth[id[now]][c.graph[now]->Rcut])] & dual_match[check_equivalence(cut_to_truth[id[nxt]][ct])];
                    //    printf("meg: %d %d %d\n", ttt, check_equivalence(cut_to_truth[id[now]][c.graph[now]->Rcut]),check_equivalence(cut_to_truth[id[nxt]][ct]));
                        if (belong[check_equivalence(cut_to_truth[id[now]][c.graph[now]->Rcut])] == belong[check_equivalence(cut_to_truth[id[nxt]][ct])])
                            continue;
                        if (ttt == 0)
                            continue;
                        int dep = 0;
                        double area = 0;
                        double AF = 0;
                        for (string nt : tmp) {
                            dep = max(dep, c.graph[nt]->mindep);
                            area += c.graph[nt]->Area;
                            AF += c.graph[nt]->AreaFlow;
                        }
                        dep++;
                        
                        area += area_dual_map[check_equivalence(cut_to_truth[id[now]][c.graph[now]->Rcut])];
                        AF /= max(1.0, 1.0 * sucsize);

                        L.push_back(DoubleCut(c.graph[now]->Rcut,tmp, now, nxt, dep, area, AF));
                    }
                }
            }
            //printf("size %d\n", L.size());
            sort(L.begin(), L.end(), Dcmp_Depth2);
            if (L.size() == 0) {
                fprintf(out2, "input:\n");
                area += area_map[check_equivalence(cut_to_truth[id[now]][c.graph[now]->Rcut])];
                //printf("dual 1 %s %lf\n", now.c_str(),area);
                for (string st : c.graph[now]->Rcut)
                {
                    fprintf(out2, "%s ", st.c_str());
                    if (!vis[st]) {
                        Q.insert(c.graph[st]->Fcut);
                        vis[st] = 1;
                    }
                }
                fprintf(out2, "%s\n", ("\noutput:\n" + now).c_str());
            } else {
                fprintf(out2, "input:\n");
               // cout << "importance: " << L[0].Name1.c_str() << ' ' <<' '<<L[0].Name2.c_str() << '\n';
               // cout << "names: ";
               // for (string st : L[0].names)
                 //   cout << st.c_str() << ' ';
                //cout << '\n';
                area += area_dual_map[check_equivalence(cut_to_truth[id[L[0].Name1]][L[0].names1])];
                 cout << "happy"<<check_equivalence(cut_to_truth[id[L[0].Name1]][L[0].names1])<< "\n";
		// cout<<area_dual_map[check_equivalence(cut_to_truth[id[L[0].Name1]][L[0].names1])]<<' '<<area_map[check_equivalence(cut_to_truth[id[L[0].Name1]][L[0].names2])]<<'\n';
                // printf("dual 2 %s %s %lf\n", L[0].Name1.c_str(),L[0].Name2.c_str(),area);
               // cout << "importance"<<L[0].Name1.c_str() << '\n';
                for (string st : L[0].names) {
                   // cout << "ggg"<< st.c_str()  << '\n';
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
                if(it!=Q.end())puts("right");
	       	Q.erase(it);
            }
        }
        //printf("lalal\n");
    }
    fprintf(stdout, "DOLM dual_area = %lf, dual_depth = %d\n", area, c.Dep);
    fclose(out2);
    return make_pair(area, c.Dep);
}

void Dereference(Circuit &c, string now, double &mem) {   // update fanin refs according to the best cuts in this iteration
    if (c.graph[now]->pre.size() == 0)
        return;
    mem+= area_map[check_equivalence(cut_to_truth[id[now]][c.graph[now]->Rcut])];
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

    int res = area_map[check_equivalence(cut_to_truth[id[now]][c.graph[now]->Rcut])];
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
    if (!fout.is_open())    
    {
        return 0;
    }
    presettings();
    fout << "case,PI,PO,size,area_single,depth_single,area_dual,depth_dual,final_area,abc_dep,abcarea_single,abcarea_dual,if_time,DOLM_time\n";
    fout1 << "case,1,2,3,4,5,6\n";
    fout2 << "case,1,2,3,4,5,6\n";
 /*   for (string str : benchmark) {
        cout << str << endl;
        if(str=="hyp.blif")
            continue;*/
    string str = "";
    str += argv[1];
    str += ".blif";
    cout << str << '\n';
    {
        int count_id=0;
        id.clear();
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
            if(!id[s])id[s] = ++count_id;
            c.graph[s]->cuts[1].insert(set<string>{s});
            c.graph[s]->mindep = 0;
            c.graph[s]->Area = 0;
            c.graph[s]->AreaFlow = 0;
            c.graph[s]->fin = 0;
            c.graph[s]->Rcut = set<string> { s };
            c.graph[s]->Fcut = Cut(set<string> { s }, s, 0, 0, 0, 0);
            Truthtable *ff = new Truthtable(s);
            cut_to_truth [id [s]].insert(pair<set<string>, Truthtable *>( set<string> { s } , ff));
            Top.push_back(s);
            for (string nt : c.graph[s]->suc) {
                ind[nt]--;      // in degree
                if (ind[nt] == 0)
                    Q.push(nt);
            }
        }

        //Enumerate Priority Cuts
        while (!Q.empty())
        {
            string now = Q.front();
            if(!id[now])
                id[now] = ++count_id;
            Top.push_back(now); // Top  是 拓扑排序的结果
            Q.pop();
            Truthtable *ff;
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
                                if (st.size() > (unsigned) Luts)continue;
                                Truthtable *ff = new Truthtable();
                                ff->get2(cut_to_truth[id[input1]][c1], cut_to_truth[id[input2]][c2], c.graph[now]->plex);
                                if(check_equivalence(ff)){cout<<check_equivalence(ff)<<' '<< now.c_str() <<"##\n";for(auto x:st){cout<<x.c_str()<<" ";};cout<<" ### \n";c.graph[now]->cuts[st.size()].insert(st),cut_to_truth[id[now]].insert(pair<set<string>, Truthtable *>(st, ff)),ppppp++;}
                                else delete ff;
                            }
                    }
            }
            if (c.graph[now]->pre.size() == 1) {    // inherit cuts from its child
                string input1 = c.graph[now]->pre[0];
                c.graph[input1]->fin++;
                for (int k = 1; k <= Luts; k++)
                    for (set<string> c1 : c.graph[input1]->cuts[k]){
                            Truthtable *ff = new Truthtable();
                            
                            ff->get1(cut_to_truth[id[input1]][c1],c.graph[now]->plex);
                            if (check_equivalence(ff))
                                c.graph[now]->cuts[k].insert(c1), cut_to_truth[id[now]].insert(pair<set<string>, Truthtable *>(c1, ff)),ppppp++;
                            else delete ff;
                            qqqqq++;
                    }
            }
            for (string nt : c.graph[now]->suc)
            {
                ind[nt]--;
                if (ind[nt] == 0)
                    Q.push(nt);
            }
            /*Calculate dep, areaflow, exact area*/
            vector<Cut> Cuts[11], Ct;
            c.graph[now]->mindep = INF;
            c.graph[now]->Area = INF;
            c.graph[now]->AreaFlow = inf;
            c.graph[now]->fin = 0;
            for (int k = 1; k <= Luts; k++)
            {
                for (set<string> ct : c.graph[now]->cuts[k]) {
                    int dep = 0;
                    double area = 0;
                    double AF = 0;
                    area = area_map[check_equivalence(cut_to_truth[id[now]][ct])];

                  // }
                    //printf("\n");
                    string curr;
                    for (string nt : ct)
                    {
                        curr=nt;
                        dep = max(dep, c.graph[nt]->mindep);
                        area += c.graph[nt]->Area;
                        AF += c.graph[nt]->AreaFlow;
                    }
                    dep++;
                    AF /= max(1.0, 1.0 * c.graph[now]->suc.size());
                    Cuts[k].push_back(Cut(ct, now, dep, area, 0, AF));
                    Ct.push_back(Cut(ct, now, dep, area, 0, AF));
                }
            }
            for (int k = 1; k <= Luts; k++) {
                sort(Cuts[k].begin(), Cuts[k].end(), cmp_Depth);
                c.graph[now]->cuts[k].clear();
                unsigned int len = Cuts[k].size();
                for (unsigned int i = 0; i < min(C, len); i++) {        // store C priority cuts for each input
                    c.graph[now]->cuts[k].insert(Cuts[k][i].names);
                }
            }
            c.graph[now]->cuts[1].insert(set<string> { now });
            try{
            ff = new Truthtable(now);
            } catch(const bad_alloc&e){
                cout<<"wtf"<<endl;
            return 0;
            }
            cut_to_truth[id[now]].insert(pair<set<string>, Truthtable *>(set<string> { now }, ff));
            sort(Ct.begin(), Ct.end(), cmp_Depth);
            c.graph[now]->mindep = Ct[0].mindep;
            c.graph[now]->AreaFlow = Ct[0].AreaFlow;
            c.graph[now]->Area = Ct[0].Area;
            c.graph[now]->Rcut = Ct[0].names;
            c.graph[now]->Fcut = Ct[0];
        }

        
       for (double lastarea =1e10;;)
        { // a large number, iterate until convergence
            double tmper = Recovery(c);
            if (tmper == lastarea)
                break;
            lastarea = tmper;
            for (string now : Top) {              
                if (c.graph[now]->pre.size() == 0)
                continue;
                for (int i = 0; i <= Luts; i++)
                    c.graph[now]->cuts[i].clear();
            }
            for (string now : Top) {
                if (c.graph[now]->pre.size() == 0)
                continue;
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
                                    Truthtable *ff = new Truthtable();
                                    ff->get2(cut_to_truth[id[input1]][c1],cut_to_truth[id[input2]][c2],c.graph[now]->plex);
                                  //  if(ff->n<=10&&ff->n>=8)output_truthtable(ff);
                                    
                                    if(check_equivalence(ff)){cout<<check_equivalence(ff)<<' '<< now.c_str() <<" ** \n";for(auto x:st){cout<<x.c_str()<<" ";};cout<<" ## \n";c.graph[now]->cuts[st.size()].insert(st),cut_to_truth[id[now]].insert(pair<set<string>, Truthtable *>(st, ff)),ppppp++;}
                                    else
                                        delete ff;
                                }
                        }
                }

                if (c.graph[now]->pre.size() == 1) {
                    string input1 = c.graph[now]->pre[0];
                    for (int k = 1; k <= Luts; k++)
                        for (set<string> c1 : c.graph[input1]->cuts[k]){
                            Truthtable *ff = new Truthtable();
                            ff->get1(cut_to_truth[id[input1]][c1],c.graph[now]->plex);
                            if(check_equivalence(ff))c.graph[now]->cuts[k].insert(c1),cut_to_truth[id[now]].insert(pair<set<string>, Truthtable *>(c1, ff)),ppppp++;
                            else delete ff;
                            qqqqq++;
                        }
                }
                set<string> Rcut_mem = c.graph[now]->Rcut;
                double Area_mem = 0;

                if (used[now])
                    Dereference(c, now, Area_mem);

                //Calculate dep, areaflow, exact area
                vector<Cut> Cuts[11], Ct;
                for (int k = 1; k <= Luts; k++) {
                    for (set<string> ct : c.graph[now]->cuts[k]) {
                        int dep = 0,fin = 0;
                        double area = 0;
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
                cout << "now in 2nd:" << now << endl;
                for (unsigned int i = 0; i < min(C, len); i++) {
                    c.graph[now]->cuts[k].insert(Cuts[k][i].names);
                    for(auto x:Cuts[k][i].names){
                    cout<<x<<" ";
                    }
                cout<<" ##\n";
                    //cout << Cuts[i].mindep << endl;
                }
                c.graph[now]->cuts[1].insert(set<string> { now });
                Truthtable *ff;
                try{
                ff = new Truthtable(now);
                } catch(const bad_alloc&e){
                    cout<<"wtf"<<endl;
                return 0;
                }
                cut_to_truth[id[now]].insert(pair<set<string>, Truthtable *>(set<string> { now }, ff));
            }

                //printf("yycccy %s \n", now.c_str());
                sort(Ct.begin(), Ct.end(), cmp_AreaFlow);
                //printf("yyxxxy %s \n", now.c_str());
                //printf("1213888 %s\n", now.c_str());
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
                //printf("#### %s \n", now.c_str());
            }
            //printf("woliekai");
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
                                    Truthtable *ff = new Truthtable();
                                    ff->get2(cut_to_truth[id[input1]][c1],cut_to_truth[id[input2]][c2],c.graph[now]->plex);
                                //    if(ff->n<=10&&ff->n>=8)output_truthtable(ff);
                                    if(check_equivalence(ff))c.graph[now]->cuts[st.size()].insert(st),cut_to_truth[id[now]].insert( pair<set<string>,Truthtable* >(st, ff) ),ppppp++;
                                    else
                                        delete ff;
                                    qqqqq++;
                                    //   cout << "important" << ' ' << check_equivalence(ff) << "\n";
                                }
                        }
                }
                if (c.graph[now]->pre.size() == 1) {
                    string input1 = c.graph[now]->pre[0];
                    for (int k = 1; k <= Luts; k++)
                        for (set<string> c1 : c.graph[input1]->cuts[k]){
                            Truthtable *ff = new Truthtable();
                            ff->get1(cut_to_truth[id[input1]][c1],c.graph[now]->plex);
                            if(check_equivalence(ff))c.graph[now]->cuts[k].insert(c1),cut_to_truth[id[now]].insert(pair<set<string>, Truthtable *>(c1, ff)),ppppp++;
                            else delete ff;
                            qqqqq++;
                        }
                }

                set<string> Rcut_mem = c.graph[now]->Rcut;
                double Area_mem = 0;

                if (used[now])
                    Dereference(c, now, Area_mem);

                //Calculate dep, areaflow, exact area
                vector<Cut> Cuts[11], Ct;
                for (int k = 1; k <= Luts; k++) {
                    for (set<string> ct : c.graph[now]->cuts[k]) {
                        int dep = 0, fin = 0;
                        double area = 0;
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
                Truthtable *ff;
                try{
                ff = new Truthtable(now);
                } catch(const bad_alloc&e){
                    cout<<"wtf"<<endl;
                return 0;
                }
                cut_to_truth[id[now]].insert(pair<set<string>, Truthtable *>(set<string> { now }, ff));
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
      //  cout << "eeeeeeeeeeeeeeeeeeeeeeeeend\n";
        pair<int, int> single = Output(c);
        clock_t finish1 = clock();
        pair<int, int> dual = Output2(c);
        double tot1 = (double)(finish1 - start) / CLOCKS_PER_SEC;
        cout << "Run time of single = " << tot1 << "s" << endl;

        //cout << ppppp << ' ' << qqqqq << ' ' << ((double)ppppp) / qqqqq << '\n';
        /*Refine *match1, *match2;
        pair<vector<int>, vector<int> > lut_display;
        match1 = new Refine();
        match2 = new Refine();
        int v1 = match1->getMatch(outdir + "/" + str + ".out1");
        int v2 = match2->getMatch(outdir + "/" + str + ".out2");
        int Refine_res = min(v1, v2);
        
        
        //printf("%d %d %s\n", v1, v2, (outdir + "/" + str + ".out").c_str());
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
        total_time = (double)(finish - start) / CLOCKS_PER_SEC;
        cout << "Run time of dual = " << total_time << "s" << endl;*/
    }

    fout.close();
    return 0;
}

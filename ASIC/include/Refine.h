/*
 * blossom algorithm
 */

#include "Circuit.h"

#define mem(x) memset(x, 0, sizeof(x))

using namespace std;

const int N = 1000005;

struct Refine{
	int u[N], v[N], next[N];
	int n, m, ans, cnt, MK;
    int maxdep = 0;
	set<string> st[N];
	string name[N], name2[N];
	queue<int> Q;
	map<string, int> id;
	int h[N], mat[N], col[N], nex[N], ufs[N], fa[N], vis[N], mark[N], dep[N];
	Refine() {}
	int getMatch(string dir);
	int Find(int x);
	int LCA(int x,int y);
	int Getdep(int k);
	int Get(int k);
	void merge(int x,int y,int lca);
	void augment(int x);
	void match(int s);
	pair<vector<int>, vector<int> > output(string outdir);
	vector<string> splits(string str, string separator);
};

vector<string> Refine::splits(string str, string separator) {    // split a string by the separator
    vector<string> dest;
    string substring;
    string::size_type start = 0, index;
    do {
        index = str.find_first_of(separator, start);
        if (index != string::npos) {
            substring = str.substr(start, index - start);
            dest.push_back(substring);
            start = str.find_first_not_of(separator, index);
            if (start == string::npos) {
                return dest;
            }
        }
    } while (index != string::npos);
    substring = str.substr(start);  // the last substring
    dest.push_back(substring);
    return dest;
}



int Refine::Find(int x)
{
	return ufs[x] == x ? x : ufs[x] = Find(ufs[x]);
}

int Refine::LCA(int x,int y)
{
	MK++;
	for (;;)
	{
		if (x)
		{
			if (mark[x] == MK)  return x;
			mark[x] = MK;
			x = Find(fa[mat[x]]);
		}
		swap(x, y);
	}
}

void Refine::merge(int x,int y,int lca)
{
	while (Find(x) != lca)
	{
		nex[x] = y, y = mat[x];
		if (col[y])
			col[y] = 0, Q.push(y);
		if (ufs[x] == x)
			ufs[x]=lca;
		if (ufs[y] == y)
			ufs[y]=lca;
		x=nex[y];
	}
}

void Refine::augment(int x)
{
	int y = mat[x];
	mat[x] = 0, x = nex[y];
	while (x)
	{
		mat[y]=x,swap(mat[x],y);
		x=nex[y];
	}
}

void Refine::match(int s)
{
	int x, y;
	for (int i = 1; i <= n; i++)
		fa[i] = nex[i] = 0, ufs[i] = i;
	while (!Q.empty())
		Q.pop();
	Q.push(s), vis[s] = ++cnt;
	while (!Q.empty())
	{
		int i;
		for (i = h[x = Q.front()], Q.pop(); y = v[i], i; i = next[i])
			if (vis[y] != cnt)
				if (!mat[y])
				{
					augment(x), mat[x] = y, mat[y] = x, ans++;
					return;
				}
				else
				{
					fa[y] = nex[y] = x,vis[y] = vis[mat[y]] = cnt;
					col[y] = 1, col[mat[y]] = 0, Q.push(mat[y]);
				}
			else if (col[y] == 0 && Find(y) != Find(x))
			{
				int lca = LCA(ufs[x], ufs[y]);
				merge(x, y, lca), merge(y, x, lca);
			}
	}
}

int Refine::Getdep(int k) {
    int depth = 0;
    if (dep[k] > 0)
		return dep[k];
    for (string s : st[k]) {
		if (id[s] == 0) continue;
		if (id[s] == k) continue;
		//cout << s << " " << dep[id[s]] << endl;
		depth = max(depth, Getdep(id[s]));
    }
    dep[k] = depth + 1;
    return depth + 1;
}

int Refine::Get(int k) {
    int mem = 0;
	string nt;
    //if (dep[k] > 0)
	//	return dep[k];
    for (string s : st[k]) {
		//cout << s << endl;
		if (id[s] == 0) continue;
		if (id[s] == k) continue;
		if (dep[id[s]] > mem)
			mem = dep[id[s]], nt = s;
		//depth = max(depth, Getdep(id[s]));
    }
	if (mem > 0)
		 Get(id[nt]);
	return 0;
}

int Refine::getMatch(string dir)
{
//	printf("%s\n", dir.c_str());
	ifstream fin(dir, ios::in);
	string s;
	n = 0, m = 0, ans = 0, cnt = 0, MK = 0;
    if (!fin.is_open()) {
        return -1;
    }
    while (getline(fin, s)) {
		getline(fin, s);
		//cout << s << endl;
        ++n;
        st[n].clear();
		vector<string> names;
		names.clear();
        string tp[10];
		names = splits(s, " ");
		//cout << s << endl;
		for (int i = 0; i < names.size(); i++) {
			st[n].insert(names[i]);
			//cout << names[i] << " ";
		}
		//cout << endl;
		getline(fin, s);
		getline(fin, s);
		names.clear();
		names = splits(s, " ");
		name[n] = names[0];
		id[name[n]] = n;
		//printf("%d\n", names.size());
		if (names.size() > 1) {
			name2[n] = names[1];
			id[name2[n]] = n;
		}
		else
			name2[n] = "";
    }
	cout << n << endl;
    maxdep = 0;
	string memstr;
    int mx = 0;
    for (int i = 1; i <= n; i++)
		if (dep[i] == 0) {
			dep[i] = Getdep(i);
			if (dep[i] > mx)
				mx = dep[i], memstr = name[i];
		}
	//cout << memstr << endl;
	//	Get(id[memstr]);
    maxdep = mx;
//	printf("Depth %d\n", maxdep);
/*    for (int i = 1; i <= n; i++)
	printf("%d\n", dep[i]);*/
    fin.close();
	int tot = 0;
	for (int i = 1; i < n; i++)
	{
		if (name2[i].size() > 0) continue;
		for (int j = i + 1; j <= n; j++)
		{
			if (name2[j].size() > 0) continue;
			set<string> str = st[i];
			str.insert(st[j].begin(), st[j].end());
			if (str.size() > 5 || dep[i] != dep[j]) continue;
			++tot;
			next[tot] = h[i], u[tot] = i, v[tot] = j;
			h[i] = tot;
			++tot;
			next[tot] = h[j], u[tot] = j, v[tot] = i;
			h[j] = tot;
		}
	}
	for (int i = 1; i <= n; i++)
		if (!mat[i])
			match(i);        
	return n - ans;
}

pair<vector<int>, vector<int> > Refine::output(string outdir) {
	//vector<string> dirs = splits(dir, "/");
	//vector<string> d = splits(dirs[2], ".");
	//string outdir = "result/" + dirs[1] + "/" + dirs[2] + ".rout";
	//cout << outdir << endl;
    vector<int> v1, v2;
    for (int i = 1; i <= 7; i++)
        v1.push_back(0), v2.push_back(0);
	FILE *out;
	out = fopen(outdir.c_str(), "w");
	for (int i = 1; i <= n; i++)
	{
	    if (mat[i] == -1) continue;
	    if (mat[i] == 0) {
			fprintf(out, "input:\n");
			for (string sts : st[i])
				fprintf(out, "%s ", sts.c_str());
			if (name2[i].size() > 0) {
				fprintf(out, ("\noutput:\n" + name[i] + " " + name2[i] + "\n").c_str());
                v2[st[i].size()]++;
            }
			else {
				fprintf(out, ("\noutput:\n" + name[i] + "\n").c_str());
                v1[st[i].size()]++;
            }
	    } else {
			fprintf(out, "input:\n");
			set<string> nst = st[mat[i]];
			nst.insert(st[i].begin(), st[i].end());
			for (string sts : nst)
				fprintf(out, "%s ", sts.c_str());
			fprintf(out, ("\noutput:\n" + name[i] + " " + name[mat[i]]+ "\n").c_str());
            v2[nst.size()]++;
			mat[mat[i]] = -1;
	    }
	}
    return make_pair(v1, v2);
}

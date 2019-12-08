#include "Circuit.h"

#define mem(x) memset(x, 0, sizeof(x))

using namespace std;

const int N = 4005;

/*vector<string> Match::splits(string str, string separator) {    // split a string by the separator
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
}*/

struct Match{
	int u[N * N], v[N * N], next[N * N];
	int n, m, ans, cnt, MK;
	set<string> st[N];
	string name[N];
	queue<int> Q;
	int h[N], mat[N], col[N], nex[N], ufs[N], fa[N], vis[N], mark[N];
	Match() {}
	int getMatch(string dir);
	int Find(int x);
	int LCA(int x,int y);
	void merge(int x,int y,int lca);
	void augment(int x);
	void match(int s);
	vector<string> splits(string str, string separator);
};

vector<string> Match::splits(string str, string separator) {    // split a string by the separator
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



int Match::Find(int x)
{
	return ufs[x] == x ? x : ufs[x] = Find(ufs[x]);
}

int Match::LCA(int x,int y)
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

void Match::merge(int x,int y,int lca)
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

void Match::augment(int x)
{
	int y = mat[x];
	mat[x] = 0, x = nex[y];
	while (x)
	{
		mat[y]=x,swap(mat[x],y);
		x=nex[y];
	}
}

void Match::match(int s)
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

int Match::getMatch(string dir)
{
	ifstream fin(dir, ios::in);
	string s;
	n = 0, m = 0, ans = 0, cnt = 0, MK = 0;
	mem(u), mem(v), mem(next);
	mem(h), mem(mat), mem(col), mem(nex), mem(ufs), mem(fa), mem(vis), mem(mark);
    if (!fin.is_open()) {
        return -1;
    }
    while (true) {
        bool end = false;
        while (fin >> s) {
            if (s == ".names") {
                break;
            }
            if (s == ".end") {
                end = true;
                break;
            }
        }
        if (end) {
            break;
        }

        getline(fin, s);
        vector<string> names = splits(s, " ");
		++n;
		st[n].clear();
		for (int i = 1; i < names.size() - 1; i++) {
			st[n].insert(names[i]);
			//cout << names[i] << endl;
		//cout << i << endl;	
		}
	if (names[names.size() - 1] != "\\")
	   name[n] = names[names.size() - 1];
	else {
	   fin >> name[n];
        }
	//cout << n << endl;
    }
    fin.close();
	int tot = 0;
	for (int i = 1; i < n; i++)
	{
		for (int j = i + 1; j <= n; j++)
		{
			set<string> str = st[i];
			str.insert(st[j].begin(), st[j].end());
			if (str.size() > 5) continue;
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
	vector<string> dirs = splits(dir, "/");
    	string outdir = "result/" + dirs[1] + "/" + dirs[2] + ".out";
	cout << outdir << endl;
	FILE *out;
        out = fopen(outdir.c_str(), "w");
	for (int i = 1; i <= n; i++)
        {
	    if (mat[i] == -1) continue;
	    if (mat[i] == 0) {
		fprintf(out, "input:\n");
		for (string sts : st[i])
		    fprintf(out, "%s ", sts.c_str());
		fprintf(out, ("\noutput:\n" + name[i] + "\n").c_str());
	    } else {
                fprintf(out, "input:\n");
		set<string> nst = st[mat[i]];
		nst.insert(st[i].begin(), st[i].end());
                for (string sts : nst)
                    fprintf(out, "%s ", sts.c_str());
                fprintf(out, ("\noutput:\n" + name[i] + " " + name[mat[i]]+ "\n").c_str());
		mat[i] = -1;
	    }
	}
	return ans;
}


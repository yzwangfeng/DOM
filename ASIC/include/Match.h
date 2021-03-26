/*
 * blossom algorithm
 */

#include "Circuit.h"

#define mem(x) memset(x, 0, sizeof(x))

using namespace std;

const int N = 1000005;

struct Match {
    int u[N], v[N], next[N];
    int n, m, ans, cnt, MK;
    int maxdep = 0;
    set<string> st[N];
    string name[N];
    queue<int> Q;
    map<string, int> id;
    int h[N], mat[N], col[N], nex[N], ufs[N], fa[N], vis[N], mark[N], dep[N];
    Match() {
    }
    int getMatch(string dir);
    int Find(int x);
    int LCA(int x, int y);
    int Getdep(int k);
    void merge(int x, int y, int lca);
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

int Match::Find(int x) {
    return ufs[x] == x ? x : ufs[x] = Find(ufs[x]);
}

int Match::LCA(int x, int y) {
    MK++;
    for (;;) {
        if (x) {
            if (mark[x] == MK)
                return x;
            mark[x] = MK;
            x = Find(fa[mat[x]]);
        }
        swap(x, y);
    }
}

void Match::merge(int x, int y, int lca) {
    while (Find(x) != lca) {
        nex[x] = y, y = mat[x];
        if (col[y])
            col[y] = 0, Q.push(y);
        if (ufs[x] == x)
            ufs[x] = lca;
        if (ufs[y] == y)
            ufs[y] = lca;
        x = nex[y];
    }
}

void Match::augment(int x) {
    int y = mat[x];
    mat[x] = 0, x = nex[y];
    while (x) {
        mat[y] = x, swap(mat[x], y);
        x = nex[y];
    }
}

void Match::match(int s) {
    int x, y;
    for (int i = 1; i <= n; i++)
        fa[i] = nex[i] = 0, ufs[i] = i;
    while (!Q.empty())
        Q.pop();
    Q.push(s), vis[s] = ++cnt;
    while (!Q.empty()) {
        int i;
        for (i = h[x = Q.front()], Q.pop(); y = v[i], i; i = next[i])
            if (vis[y] != cnt)
                if (!mat[y]) {
                    augment(x), mat[x] = y, mat[y] = x, ans++;
                    return;
                } else {
                    fa[y] = nex[y] = x, vis[y] = vis[mat[y]] = cnt;
                    col[y] = 1, col[mat[y]] = 0, Q.push(mat[y]);
                }
            else if (col[y] == 0 && Find(y) != Find(x)) {
                int lca = LCA(ufs[x], ufs[y]);
                merge(x, y, lca), merge(y, x, lca);
            }
    }
}

int Match::Getdep(int k) {
    int depth = 0;
    if (dep[k] > 0)
        return dep[k];
//printf("%d %s\n", k, name[k].c_str());
    for (string s : st[k]) {
        if (id[s] == 0)
            continue;
        if (id[s] == k)
            continue;
        depth = max(depth, Getdep(id[s]));
    }
    dep[k] = depth + 1;
    return depth + 1;
}

int Match::getMatch(string dir) {
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

        string tp[10];
        int pos = 0;
        ++n;
        st[n].clear();
        vector<string> names;
        names.clear();
        do {
            getline(fin, s);
            names = splits(s, " ");
            for (int i = 1; i < names.size(); i++)
                if (names[i] != "\\") {
                    tp[pos] = names[i];
                    pos++;
                }
            //cout << names[i] << endl;
            //cout << i << endl;
        } while (names[names.size() - 1] == "\\");
        for (int i = 0; i < pos - 1; i++)
            st[n].insert(tp[i]);
        name[n] = tp[pos - 1];
        id[name[n]] = n;
        //cout << n << endl;
    }
    maxdep = 0;
    int mx = 0;
    for (int i = 1; i <= n; i++)
        if (dep[i] == 0) {
            dep[i] = Getdep(i);
            mx = max(mx, dep[i]);
        }
    maxdep = mx;
    /*    for (int i = 1; i <= n; i++)
     printf("%d\n", dep[i]);*/
    fin.close();
    int tot = 0;
    int p = 6, a1 = 5;
    for (int i = 1; i < n; i++) {
        for (int j = i + 1; j <= n; j++) {
            set<string> str = st[i];
            str.insert(st[j].begin(), st[j].end());
            if (str.size() > 4 || dep[i] != dep[j])
                continue;
            a1 = (a1 + 1) % p;
            if (a1 != 0)
                continue;
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
    //cout << outdir << endl;
    FILE *out;
    out = fopen(outdir.c_str(), "w");
    for (int i = 1; i <= n; i++) {
        if (mat[i] == -1)
            continue;
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
            fprintf(out, ("\noutput:\n" + name[i] + " " + name[mat[i]] + "\n").c_str());
            mat[i] = -1;
        }
    }

    return ans;
}


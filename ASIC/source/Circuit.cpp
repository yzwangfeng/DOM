#include "Circuit.h"
#include "Match.h"

vector<string> split(string str, string separator) {    // split a string by the separator
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

Var::Var(string name_, bool is_in_, bool is_out_)
        :
        name(name_), is_in(is_in_), is_out(is_out_) {
    depth = is_in ? 0 : -1;
    mindep = Area = 0;
    AreaFlow = 0;
}

Circuit::Circuit(string benchmark_)
        :
        benchmark(benchmark_) {
    Dep = 0;

    abc_res = get_abc_result();

    standard_cell_map("ALL.genlib");

    //cout << "PI = " << input.size() << ", PO = " << output.size() << ", size = " << graph.size() << endl;

//    for (auto p : graph) {
//        cout << p.first << ' ' << p.second->depth << ' ';
//        for (string s : p.second->pre) {
//            cout << s << ' ' << graph[s]->depth << ' ';
//        }
//        cout << endl;
//    }

//    write_dot();
}

Circuit::~Circuit() {
    char lut_file[256];
    sprintf(lut_file, "%s_lut.blif", benchmark.c_str());            // lut mapping file
    //remove(lut_file);

    char std_file[256];
    sprintf(std_file, "%s_standard.blif", benchmark.c_str());            // lut mapping file
   // remove(std_file);

    char abc_lut_file[256];
    sprintf(abc_lut_file, "%s_abc_lut.blif", benchmark.c_str());    // abc lut mapping file
    //remove(abc_lut_file);
}

void Circuit::standard_cell_map(string lib) {
    abc_map(benchmark, "write_blif", benchmark + "_standard.blif", lib);
    ifstream fin(benchmark + "_standard.blif", ios::in);
    if (!fin.is_open()) {
        return;
    }

    string s;
    while (fin >> s && s != ".inputs") {
    }
    while (fin >> s && s != ".outputs") {
        if (s != "\\") {
            input.push_back(s);
            graph[s] = new Var(s, true, false);
        }
    }
    while (fin >> s && s != ".gate") {
        if (s != "\\") {
            output.push_back(s);
            graph[s] = new Var(s, false, true);
        }
    }
    double ans = 0;
    printf("now is running on blif:%s\n", benchmark.c_str());
    do
    {
        string gate;
        fin >> gate;
        int plex=0;
        if(gate=="SET")plex = 1;
        if(gate=="RESET")plex = 2;
        if(gate=="BUFF")plex = 3;
        if(gate=="NOT")plex = 4;
        if(gate=="OR2X1")plex = 5;
        if(gate=="NAND2X1")plex = 6,ans+=1.877200;
        if(gate=="X1")plex = 7;
        if(gate=="X2")plex = 8;
        if(gate=="AND2X1")plex = 9,ans+=2.346500;
        if(gate=="NOR2X1")plex = 10;
        if(gate=="XOR2X1")plex = 11;
        if(gate=="XNOR2X1")plex = 12;
        
        getline(fin, s);
        vector<string> cells = split(s, " ");

        string out_cell = (*cells.rbegin()).substr(2);
        //printf("%s", benchmark.c_str());
        //printf("important %s\n", out_cell.c_str());
        if (find(output.begin(), output.end(), out_cell) == output.end())
        {
            graph[out_cell] = new Var(out_cell, false, false);
        }
        cells.pop_back();
        cells.erase(cells.begin());

        for (string cell : cells) {
            cell = cell.substr(2);
            graph[cell]->suc.push_back(out_cell);
            graph[out_cell]->pre.push_back(cell);
            graph[out_cell]->plex = plex;
        }
    } while (fin >> s && s == ".gate");
    cout <<"the standard blif area is "<<ans << "\n";
    fin.close();
}
pair<int, pair<int, int> > Circuit::get_abc_result() {
    memset(abc_lut, 0, sizeof(abc_lut));
    abc_lut_area = 0;

    string s;

    int area[10] = { }, delay[10] = { };
    ifstream fin_lut("abclib/6LUT.lutlib", ios::in);
    if (!fin_lut.is_open()) {
        return make_pair(0, make_pair(0, 0));
    }
    getline(fin_lut, s);
    getline(fin_lut, s);
    int k;
    while (fin_lut >> k) {
        fin_lut >> area[k] >> delay[k];
    }
    fin_lut.close();

    abc_lutpack(benchmark, "write_blif", benchmark + "_abc_lut.blif", "6LUT.lutlib");

    ifstream fin(benchmark + "_abc_lut.blif", ios::in);
    if (!fin.is_open()) {
        return  make_pair(0, make_pair(0, 0));
    }
    while (fin >> s && s != ".names") {
    }

    set<string> all_cells;
    do {
        if (s == ".names") {
            getline(fin, s);
            if (s[s.length() - 1] == '\\') {
                string next;
                getline(fin, next);
                s = s.substr(0, s.length() - 1) + next.substr(1);
            }
            vector<string> cells = split(s.substr(1), " ");
            for (string cell : cells){
                all_cells.insert(cell);
            }
            ++abc_lut[cells.size() - 1];
        }
    } while (fin >> s && s != ".end");
    fin.close();

    for (int i = 0; i < 10; ++i) {
        abc_lut_area += abc_lut[i] * area[i];
    }

    int cell_number = all_cells.size();
    for (int i = 3; i < 10; ++i) {
        cell_number += abc_lut[i] * (i - 2);
    }

	Match *mt = new Match();
	int r1 = abc_lut_area;
	int r2 = abc_lut_area - mt->getMatch(benchmark + "_abc_lut.blif");
    int r3 = mt->maxdep;
    cout << "ABC size = " << cell_number << ", single_area = " << r1 << ", dual_area = " << r2 << ", depth = " << r3 << endl;
    return make_pair(r1, make_pair(r2, r3));
}

void Truthtable:: get2(Truthtable *A,Truthtable *B,int plex){
    int x1 = A->n, x2 = B->n;
    if (plex == 1){
        n = x1;
        for (int i = 0; i < (1<<n); i++)
            num[i] = 0;
    }
    if (plex == 2){
        n = x1;
        for (int i = 0; i < (1<<n); i++)
            num[i] = 1;
    }
    if (plex == 3){
        n = x1;
        for (int i = 0; i < (1<<n); i++)
            num[i] = A->num[i];  
    }
    if (plex == 4){
        n = x1;
        for (int i = 0; i < (1<<n); i++)
            num[i] = !A->num[i];
    }

    if (plex == 5)
    {
        n = x1 + x2;
        for (int i = 0; i < (1<<x1); i++)
            for (int j = 0; j < (1<<x2); j++){
                num[(i << x2) + j] = A->num[i] | B->num[j];  //Y=A+B;	
            }
    }
    if (plex == 6)
    {
        n = x1 + x2;
        for (int i = 0; i < (1<<x1); i++)
            for (int j = 0; j < (1<<x2); j++){
                num[(i << x2) + j] = !(A->num[i] & B->num[j]); //Y=!A+!B;
            }
    }
    if (plex == 7)
    {
        n = x1 + x2;
        for (int i = 0; i < (1<<x1); i++)
            for (int j = 0; j < (1<<x2); j++){
                num[(i << x2) + j] = (!A->num[i]) | B->num[j]; //Y=!A+B;	
            }
    }
    if (plex == 8)
    {
        n = x1 + x2;
        for (int i = 0; i < (1<<x1); i++)
            for (int j = 0; j < (1<<x2); j++){
                num[(i << x2) + j] = (!A->num[i]) & B->num[j]; //Y=!A*B;	
            }
    }
    if (plex == 9)
    {
        n = x1 + x2;
        for (int i = 0; i < (1<<x1); i++)
            for (int j = 0; j < (1<<x2); j++){
                num[(i << x2) + j] = A->num[i] & B->num[j]; //Y=A*B;
            }
    }
    if (plex == 10)
    {
        n = x1 + x2;
        for (int i = 0; i < (1<<x1); i++)
            for (int j = 0; j < (1<<x2); j++){
                num[(i << x2) + j] = !(A->num[i]|B->num[j]); //Y=!A*!B;
            }
    }
    if (plex == 11)
    {
        n = x1 + x2;
        for (int i = 0; i < (1<<x1); i++)
            for (int j = 0; j < (1<<x2); j++){
                num[(i << x2) + j] = A->num[i]^B->num[j]; //Y=(A*!B)+(!A*B);
            }
    }
    if (plex == 12)
    {
        n = x1 + x2;
        for (int i = 0; i < (1<<x1); i++)
            for (int j = 0; j < (1<<x2); j++){
                num[(i << x2) + j] = !(A->num[i]^B->num[j]); //Y=(A*B)+(!A*!B);
            }
    }
}

void Truthtable:: get1(Truthtable *A,int plex){
    int x = A->n;
    n = x;
    if(plex==1)for (int i = 0; i < (1<<n); i++)num[i] = 0;
    if(plex==2)for (int i = 0; i < (1<<n); i++)num[i] = 1;
    if(plex==3)for (int i = 0; i < (1<<n); i++)num[i] = A->num[i];
    if(plex==4)for (int i = 0; i < (1<<n); i++)num[i] = !A->num[i];
}
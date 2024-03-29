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

    cout << "PI = " << input.size() << ", PO = " << output.size() << ", size = " << graph.size() << endl;

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
    remove(lut_file);

    char std_file[256];
    sprintf(std_file, "%s_standard.blif", benchmark.c_str());            // lut mapping file
    remove(std_file);

    char abc_lut_file[256];
    sprintf(abc_lut_file, "%s_abc_lut.blif", benchmark.c_str());    // abc lut mapping file
    remove(abc_lut_file);
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

    do {
        string gate;
        fin >> gate;

        getline(fin, s);
        vector<string> cells = split(s, " ");

        string out_cell = (*cells.rbegin()).substr(2);
        if (find(output.begin(), output.end(), out_cell) == output.end()) {
            graph[out_cell] = new Var(out_cell, false, false);
        }
        cells.pop_back();
        cells.erase(cells.begin());

        for (string cell : cells) {
            cell = cell.substr(2);
            graph[cell]->suc.push_back(out_cell);
            graph[out_cell]->pre.push_back(cell);
        }
    } while (fin >> s && s == ".gate");
    fin.close();
}

void Circuit::lut_map(string lib) {
    abc_lutpack(benchmark, "write_blif", benchmark + "_lut.blif", lib);

    ifstream fin(benchmark + "_lut.blif", ios::in);
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
    while (fin >> s && s != ".names") {
        if (s != "\\") {
            output.push_back(s);
            graph[s] = new Var(s, false, true);
        }
    }

    do {
        if (s == ".names") {
            getline(fin, s);
            if (s[s.length() - 1] == '\\') {
                string next;
                getline(fin, next);
                s = s.substr(0, s.length() - 1) + next.substr(1);
            }
            vector<string> cells = split(s.substr(1), " ");

            for (string cell : cells) {
                if (graph.find(cell) == graph.end()) {
                    graph[cell] = new Var(cell, false, false);
                }
            }
            string out_cell = (*cells.rbegin());
            cells.pop_back();
            for (string cell : cells) {
                graph[cell]->suc.push_back(out_cell);
                graph[out_cell]->pre.push_back(cell);
            }
        }
    } while (fin >> s && s != ".end");
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
    cout << "ABC size = " << cell_number << ", single_area = " << r1 << ", dual_area = " <<  r2 << ", depth = " << r3 << endl;
	return make_pair(r1, make_pair(r2, r3));
}

void Circuit::write_dot() {
    char dot_file[256];
    sprintf(dot_file, "%s.dot", benchmark.c_str());
    ofstream fout(dot_file, ios::out);
    if (!fout.is_open()) {
        return;
    }

    fout << "digraph G {\nsize = \"7.5,10\";\ncenter = true;" << endl;
    for (pair<string, Var*> p : graph) {
        //for (int i = 0; i < p.first.length(); i++)
        //    if (p.first[i] != '[' && p.first[i] != ']')
        fout << p.first;
        if (p.second->is_in) {
            fout << "[shape = invtriangle, color = coral, fillcolor = coral];" << endl;
        } else if (p.second->is_out) {
            fout << "[shape = triangle, color = coral, fillcolor = coral];" << endl;
        } else {
            fout << "[shape = ellipse];" << endl;
        }
    }
	
    fout << endl;
    for (pair<string, Var*> p : graph) {
        for (string s : p.second->pre) {
            fout << s << " -> " << p.first << endl;
        }
    }
    fout << '}' << endl;

    fout.close();
}

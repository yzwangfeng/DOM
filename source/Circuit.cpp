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

    memset(abc_lut, 0, sizeof(abc_lut));
    abc_lut_area = 0;

    //standard_cell_map("2INPUT.genlib");

    lut_map("6LUT.lutlib");     // change it to your own LUT library
    for (string s : output) {
        decompose(s);
    }

    //write_dot();
}

Circuit::~Circuit() {
    char lut_file[256];
    sprintf(lut_file, "%s_lut.blif", benchmark.c_str());            // abc lut mapping file
    remove(lut_file);
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
    string s;

    int area[10] = { }, delay[10] = { };
    ifstream fin_lut("abclib/" + lib, ios::in);
    if (!fin_lut.is_open()) {
        return;
    }
    getline(fin_lut, s);
    getline(fin_lut, s);
    int k;
    while (fin_lut >> k) {
        fin_lut >> area[k] >> delay[k];
    }
    fin_lut.close();

    //abc_lutpack(benchmark, "write_blif", benchmark + "_lut.blif", lib);
    abc_wiremap(benchmark, "write_blif", benchmark + "_lut.blif", lib);

    ifstream fin(benchmark + "_lut.blif", ios::in);
    if (!fin.is_open()) {
        return;
    }

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
            while (s[s.length() - 1] == '\\') {
                string next;
                getline(fin, next);
                s = s.substr(0, s.length() - 1) + next;
            }
            vector<string> cells = split(s.substr(1), " ");

            string out_cell = (*cells.rbegin());
            if (find(output.begin(), output.end(), out_cell) == output.end()) {
                graph[out_cell] = new Var(out_cell, false, false);
            }
            cells.pop_back();

            for (string cell : cells) {
                if (find(input.begin(), input.end(), cell) == input.end()) {
                    graph[cell] = new Var(cell, false, false);
                }
                graph[cell]->suc.push_back(out_cell);
                graph[out_cell]->pre.push_back(cell);
            }
            ++abc_lut[cells.size()];
        }
    } while (fin >> s && s != ".end");
    fin.close();

    for (int i = 0; i < 10; ++i) {
        abc_lut_area += abc_lut[i] * area[i];
    }
    cout << "ABC Area1: " << abc_lut_area << endl;
    Match *mt = new Match();
    cout << "ABC Area2: " << abc_lut_area - mt->getMatch(benchmark + "_lut.blif") << endl;
}

void Circuit::decompose(string var) {
    if (graph[var]->depth != -1) {
        return;
    }
    for (string s : graph[var]->pre) {
        decompose(s);
    }
    while (graph[var]->pre.size() > 2) {
        string min_depth_pre[2] = { "-1", "-1" };
        for (string s : graph[var]->pre) {
            if (min_depth_pre[0] == "-1" || graph[s]->depth < graph[min_depth_pre[0]]->depth) {
                min_depth_pre[0] = s;
                if (min_depth_pre[1] == "-1"
                        || graph[min_depth_pre[0]]->depth < graph[min_depth_pre[1]]->depth) {
                    swap(min_depth_pre[0], min_depth_pre[1]);
                }
            }
        }
        string dcp_var_name = "dcp_" + to_string(graph.size());
        graph[dcp_var_name] = new Var(dcp_var_name, false, false);
        graph[dcp_var_name]->depth = graph[min_depth_pre[0]]->depth + 1;
        graph[dcp_var_name]->suc.push_back(var);
        for (int i = 0; i < 2; ++i) {
            graph[dcp_var_name]->pre.push_back(min_depth_pre[i]);
            for (vector<string>::iterator iter = graph[var]->pre.begin(); iter != graph[var]->pre.end();
                    ++iter) {
                if (*iter == min_depth_pre[i]) {
                    graph[var]->pre.erase(iter);
                    break;
                }
            }
        }
        graph[var]->pre.push_back(dcp_var_name);
    }
    for (string s : graph[var]->pre) {
        graph[var]->depth = max(graph[var]->depth, graph[s]->depth);
    }
    ++graph[var]->depth;
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

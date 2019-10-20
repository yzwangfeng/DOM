#include "Circuit.h"

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
    out_degree = 0;
}

Circuit::Circuit(string benchmark_)
        :
        benchmark(benchmark_) {
    abc_iter = graph_size = 0;
    memset(abc_lut, 0, sizeof(abc_lut));
    abc_lut_area = 0;

    abc_synthesize();
    read_blif();

    abc_lut_map((char*) "6LUT.lutlib");     // change it to your own LUT library

    //write_dot();
}

Circuit::~Circuit() {
    char blif_file[256], lut_file[256];
    sprintf(blif_file, "%s_%d.blif", benchmark.c_str(), abc_iter);  // abc synthesis result
    sprintf(lut_file, "%s_lut.blif", benchmark.c_str());            // abc lut mapping file
    remove(blif_file);
    remove(lut_file);
}

void Circuit::abc_synthesize() {
    int min_cycle = inf, last_cycle = inf;
    char file[256], new_file[256];
    for (abc_iter = 1; abc_iter <= 100; ++abc_iter) {
        if (abc_iter == 1) {
            sprintf(file, "%s", benchmark.c_str());
        } else {
            strcpy(file, new_file);
        }
        sprintf(new_file, "%s_%d.blif", benchmark.c_str(), abc_iter);
        abc_map(file, new_file, (char*) "NOR2.genlib");

        ifstream fin(new_file, ios::in);
        if (!fin.is_open()) {
            return;
        }
        string s;
        int cycle = 0;
        while (fin >> s) {
            cycle += s == ".gate";
        }

        fin.close();
        if (abc_iter != 1) {
            remove(file);
        }

        min_cycle = min(min_cycle, cycle);
        if (last_cycle == cycle) {
            cout << "Total cycle: " << cycle << endl;
            break;
        }
        last_cycle = cycle;
    }
}

void Circuit::read_blif() {
    char blif_file[256];
    sprintf(blif_file, "%s_%d.blif", benchmark.c_str(), abc_iter);
    ifstream fin(blif_file, ios::in);
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

    graph_size = 0;
    do {
        string gate;
        fin >> gate;

        getline(fin, s);
        vector<string> cells = split(s, " ");

        string out_cell = (*cells.rbegin()).substr(2);
        if (find(output.begin(), output.end(), out_cell) == output.end()) {
            graph[out_cell] = new Var(out_cell, false, false);
        }

        cells.erase(cells.begin());

        for (string cell : cells) {
            cell = cell.substr(2);
            if (cell != out_cell) {
                graph[cell]->suc.push_back(out_cell);
                ++graph[cell]->out_degree;
                graph[out_cell]->pre.push_back(cell);
            }
        }

        ++graph_size;
    } while (fin >> s && s == ".gate");
    fin.close();
}

void Circuit::abc_lut_map(char *lib) {
    char input_file[256], lut_file[256], lutlib_file[256];
    sprintf(input_file, "%s", benchmark.c_str());
    sprintf(lut_file, "%s_lut.blif", benchmark.c_str());
    sprintf(lutlib_file, "abclib/%s.blif", lib);
    abc_lutpack(input_file, lut_file, lib);

    int area[10] = { }, delay[10] = { };
    sprintf(lutlib_file, "abclib/%s.blif", lib);
    ifstream fin_lut(lutlib_file, ios::in);
    if (!fin_lut.is_open()) {
        return;
    }
    string s;
    getline(fin_lut, s);
    getline(fin_lut, s);
    int k;
    while (fin_lut >> k) {
        fin_lut >> area[k] >> delay[k];
    }
    fin_lut.close();

    ifstream fin(lut_file, ios::in);
    if (!fin.is_open()) {
        return;
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
        vector<string> names = split(s, " ");
        ++abc_lut[names.size() - 2];
    }
    fin.close();

    for (int i = 0; i < 10; ++i) {
        abc_lut_area += abc_lut[i] * area[i];
    }
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

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
    abc_synthesize();
    read_blif();
    //circuit.write_dot();
}

Circuit::~Circuit() {
    char blif_file[256];
    sprintf(blif_file, "%s_%d.blif", benchmark.c_str(), abc_iter);
    remove(blif_file);
}

void Circuit::abc_synthesize() {
    int min_cycle = INF, last_cycle = INF;
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

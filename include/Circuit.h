/*
 * Circuit.h
 *
 * circuit structure
 */

#ifndef CIRCUIT_H_
#define CIRCUIT_H_

#include <map>
#include <set>
#include <queue>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <cmath>

#include "ABC.h"

#define inf 100000000
#define INF 100000000

struct DoubleCut {
    set<string> names;
    string Name1, Name2;
    int mindep, Area;
    double AreaFlow;
    DoubleCut() {
        Name1 = "", Name2 = "";
        mindep = 0;
        Area = 0;
        AreaFlow = 0;
    }
    DoubleCut(set<string> n, string nm1, string nm2, int dep, int area, double AF) {
        names = n;
        Name1 = nm1, Name2 = nm2;
        mindep = dep;
        Area = area;
        AreaFlow = AF;
    }
};


struct Cut {
    set<string> names;
    string Name;
    int mindep, Area;
    double AreaFlow;
    Cut() {
	Name = "";
        mindep = 0;
        Area = 0;
        AreaFlow = 0;
    }
    Cut(set<string> n, string nm, int dep, int area, double AF) {
        names = n;
	Name = nm;
        mindep = dep;
        Area = area;
        AreaFlow = AF;
    }
};

struct Var {
    string name;
    bool is_in, is_out;

    vector<string> pre;     // precursors
    vector<string> suc;     // successors
    set<set<string> > cuts[10];  //K cuts
    set<string> Rcut;      //represent cut
    set<string> Rdcut;
    string Partner;

    int out_degree;         // = suc.size()
    int mindep, Area;
    double AreaFlow;

    Var(string name_, bool is_in_, bool is_out_);
};

struct Circuit {
    string benchmark;
    int abc_iter;           // file name = $benchmark$_$abc_iter$.blif

    map<string, Var*> graph;
    int graph_size;         // = graph.size()
    int Dep;

    vector<string> input;
    vector<string> output;

    int abc_lut[10];        // there are abc_lut[k] k-input LUTs after abc lut mapping
    int abc_lut_area;       // total area after abc lut mapping

    Circuit(string benchmark_);
    ~Circuit();

    void abc_synthesize();  // synthesize the circuit using a customized library
    void read_blif();
    void abc_lut_map(char* lib);

    void write_dot();       // visualize the circuit
};

#endif /* CIRCUIT_H_ */

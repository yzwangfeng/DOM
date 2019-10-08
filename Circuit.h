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

#define INF 1000000

struct Cut{
    set<string> names;
    int mindep, Area;
    double AreaFlow;
    Cut() {
	mindep = 0;
	Area = 0;
	AreaFlow = 0;
    }
    Cut(set<string> n, int dep, int area, double AF) {
	names = n;
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
    set<set<string> > cuts; //cuts
    set<string> Rcut;      //represent cut

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

    vector<string> input;
    vector<string> output;

    Circuit(string benchmark_);
    ~Circuit();

    void abc_synthesize();  // synthesize the circuit using a customized library
    void read_blif();

    void write_dot();       // visualize the circuit
};

#endif /* CIRCUIT_H_ */

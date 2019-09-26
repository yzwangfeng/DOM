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
#include <cmath>

#include "ABC.h"

#define INF 1000000

struct Var {
    string name;
    bool is_in, is_out;

    vector<string> pre;     // precursors
    vector<string> suc;     // successors

    int out_degree;         // = suc.size()

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

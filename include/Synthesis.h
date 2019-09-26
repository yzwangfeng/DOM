/*
 * Synthesis.h
 *
 * one-line synthesis
 */

#ifndef SYNTHESIS_H_
#define SYNTHESIS_H_

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

    vector<string> pre;
    vector<string> next;

    int out_degree;

    Var(string name_, bool is_in_, bool is_out_);
};

struct Circuit {
    string benchmark;
    int abc_iter;

    map<string, Var*> graph;
    int ope_num;

    vector<string> input;
    vector<string> output;

    Circuit(string benchmark_);
    ~Circuit();

    void abc_synthesize();  // synthesize a serial operation using abc library
    void read_blif();

    void write_dot();   // visualize the circuit
};

#endif /* SYNTHESIS_H_ */

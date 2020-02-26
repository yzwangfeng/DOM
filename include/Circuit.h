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
    int mindep, Area, fin;
    double AreaFlow;
    Cut() {
	    Name = "";
        mindep = 0;
        Area = 0;
        AreaFlow = 0;
    }
    Cut(set<string> n, string nm, double dep, double area, double f, double AF) {
        names = n;
	    Name = nm;
        mindep = dep;
        Area = area;
        fin = f;
        AreaFlow = AF;
    }
    bool operator < (const Cut A) const{
        return mindep > A.mindep || (mindep == A.mindep && Area < A.Area) 
               || (mindep == A.mindep && Area == A.Area && AreaFlow < A.AreaFlow);
    }
};

struct Var {
    string name;
    bool is_in, is_out;

    int depth;

    vector<string> pre;     // precursors
    vector<string> suc;     // successors

    set<set<string> > cuts[10];  //K cuts
    set<string> Rcut;      //represent cut
    //set<string> Rdcut;
    Cut Fcut; // First Cut
    string Partner;

    int mindep, Area, fin;
    double AreaFlow;

    Var(string name_, bool is_in_, bool is_out_);
};

struct Circuit {
    string benchmark;

    map<string, Var*> graph;
    int Dep;

    vector<string> input;
    vector<string> output;

    int abc_lut[10];        // there are abc_lut[k] k-input LUTs after abc lut mapping
    int abc_lut_area;       // total area after abc lut mapping

    Circuit(string benchmark_);
    ~Circuit();

    void standard_cell_map(string lib);
    void lut_map(string lib);
    void decompose(string var);

    void get_abc_result();

    void write_dot();       // visualize the circuit
};

#endif /* CIRCUIT_H_ */

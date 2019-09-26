/*
 * main.cpp
 *
 */

#include "ABC.h"
#include "Circuit.h"
#include <dirent.h>
#include <string>
#include <vector>
using namespace std;

void get_file_name(string path, vector<string> &files) {
    struct dirent *ptr;
    DIR *dir = opendir(path.c_str());
    while ((ptr = readdir(dir)) != NULL) {
        if (ptr->d_name[0] != '.') {  // skip . and ..
            files.push_back(ptr->d_name);
        }
    }
    closedir(dir);
}

int main(int argc, char *argv[]) {
    vector<string> benchmark;
    string dir = "benchmark/IWLS93";
    get_file_name(dir, benchmark);

    for (string s : benchmark) {
        double total_time;
        clock_t start = clock();

        Circuit circuit(dir + "/" + s);  // already synthesized

        clock_t finish = clock();
        total_time = (double) (finish - start) / CLOCKS_PER_SEC;
        cout << "Run time: " << total_time << "s" << endl;
    }

    return 0;
}

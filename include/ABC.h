/*
 * abc.h
 *
 * invoke abc
 */

#ifndef ABC_H_
#define ABC_H_

#include <iostream>
#include <fstream>
using namespace std;

extern "C" void Abc_Start();
extern "C" void Abc_Stop();
extern "C" void * Abc_FrameGetGlobalFrame();
extern "C" int Cmd_CommandExecute(void * pAbc, char * sCommand);

int abc_simplify(char* in_file, char* write_cmd, char* out_file, int fUseResyn = 2);
int abc_map(char* in_file, char* out_file, char* lib);
int abc_lutpack(char* in_file, char* out_file, char* lib);

#endif /* ABC_H_ */

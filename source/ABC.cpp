#include "ABC.h"

int abc_simplify(char *in_file, char *write_cmd, char *out_file) {  // 调用abc
    void *pAbc;
    char Command[1000];

    Abc_Start();
    pAbc = Abc_FrameGetGlobalFrame();

    sprintf(Command, "read %s", in_file);
    if (Cmd_CommandExecute(pAbc, Command)) {
        fprintf( stdout, "Cannot execute command \"%s\".\n", Command);
        return 1;
    }

#ifdef DEBUG
	sprintf(Command, "print_stats");
	if (Cmd_CommandExecute(pAbc, Command)) {
		fprintf( stdout, "Cannot execute command \"%s\".\n", Command);
		return 1;
	}
#endif

    sprintf(Command, "fraig_store; balance; rewrite; rewrite -z; balance; rewrite -z; balance;");
    if (Cmd_CommandExecute(pAbc, Command)) {
        fprintf( stdout, "Cannot execute command \"%s\".\n", Command);
        return 1;
    }

    sprintf(Command,
            "fraig_store; balance; rewrite; refactor; balance; rewrite; rewrite -z; balance; refactor -z; rewrite -z; balance;");
    if (Cmd_CommandExecute(pAbc, Command)) {
        fprintf( stdout, "Cannot execute command \"%s\".\n", Command);
        return 1;
    }

    sprintf(Command, "fraig_store; fraig_restore;");
    if (Cmd_CommandExecute(pAbc, Command)) {
        fprintf( stdout, "Cannot execute command \"%s\".\n", Command);
        return 1;
    }

    sprintf(Command, "%s %s", write_cmd, out_file);
    if (Cmd_CommandExecute(pAbc, Command)) {
        fprintf( stdout, "Cannot execute command \"%s\".\n", Command);
        return 1;
    }

#ifdef DEBUG
	sprintf(Command, "print_stats");
	if (Cmd_CommandExecute(pAbc, Command)) {
		fprintf( stdout, "Cannot execute command \"%s\".\n", Command);
		return 1;
	}
	sprintf(Command, "cec %s %s", in_file, out_file);	// 检验转换前后电路是否等价，非常耗时
	if (Cmd_CommandExecute(pAbc, Command)) {
		fprintf( stdout, "Cannot execute command \"%s\".\n", Command);
		return 1;
	}
#endif

    Abc_Stop();
    return 0;
}

int abc_map(char *in_file, char *out_file, char *lib) {  // 调用abc的map指令
    void *pAbc;
    char Command[1000];

    Abc_Start();
    pAbc = Abc_FrameGetGlobalFrame();

    sprintf(Command, "read_library abclib/%s", lib);
    if (Cmd_CommandExecute(pAbc, Command)) {
        fprintf( stdout, "Cannot execute command \"%s\".\n", Command);
        return 1;
    }

    sprintf(Command, "read %s", in_file);
    if (Cmd_CommandExecute(pAbc, Command)) {
        fprintf( stdout, "Cannot execute command \"%s\".\n", Command);
        return 1;
    }

    sprintf(Command, "fraig_store; balance; rewrite; rewrite -z; balance; rewrite -z; balance;");
    if (Cmd_CommandExecute(pAbc, Command)) {
        fprintf( stdout, "Cannot execute command \"%s\".\n", Command);
        return 1;
    }

    sprintf(Command,
            "fraig_store; balance; rewrite; refactor; balance; rewrite; rewrite -z; balance; refactor -z; rewrite -z; balance;");
    if (Cmd_CommandExecute(pAbc, Command)) {
        fprintf( stdout, "Cannot execute command \"%s\".\n", Command);
        return 1;
    }

    sprintf(Command, "fraig_store; fraig_restore;");
    if (Cmd_CommandExecute(pAbc, Command)) {
        fprintf( stdout, "Cannot execute command \"%s\".\n", Command);
        return 1;
    }

    sprintf(Command, "map -a");  // area-only mapping
    if (Cmd_CommandExecute(pAbc, Command)) {
        fprintf( stdout, "Cannot execute command \"%s\".\n", Command);
        return 1;
    }

    sprintf(Command, "write_blif %s", out_file);
    if (Cmd_CommandExecute(pAbc, Command)) {
        fprintf( stdout, "Cannot execute command \"%s\".\n", Command);
        return 1;
    }

    Abc_Stop();
    return 0;
}

int abc_lutpack(char *in_file, char *out_file, char *lib) {  // 调用abc的lutpack指令
    abc_simplify(in_file, (char*) "write_blif", (char*) "tmp.blif");

    void *pAbc;
    char Command[1000];

    Abc_Start();
    pAbc = Abc_FrameGetGlobalFrame();

    sprintf(Command, "read_lut abclib/%s", lib);
    if (Cmd_CommandExecute(pAbc, Command)) {
        fprintf( stdout, "Cannot execute command \"%s\".\n", Command);
        return 1;
    }

    sprintf(Command, "read %s", "tmp.blif");
    if (Cmd_CommandExecute(pAbc, Command)) {
        fprintf( stdout, "Cannot execute command \"%s\".\n", Command);
        return 1;
    }

    sprintf(Command, "lutpack");  // an area-oriented resynthesis engine for network mapped into K-LUTs
    if (Cmd_CommandExecute(pAbc, Command)) {
        fprintf( stdout, "Cannot execute command \"%s\".\n", Command);
        return 1;
    }

    sprintf(Command, "write_blif %s", out_file);
    if (Cmd_CommandExecute(pAbc, Command)) {
        fprintf( stdout, "Cannot execute command \"%s\".\n", Command);
        return 1;
    }

    Abc_Stop();

    remove("tmp.blif");

    return 0;
}

#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>
#include <iostream>
#include <filesystem>
#include <map>
#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <queue>
#include <windows.h>
using namespace std;

#undef ERROR
#undef NO_ERROR
#undef GetCurrentTime

typedef unsigned int uint;

enum WORK {
	NO_WORK,
	PRINT_HELP,
	RUN,
	EXIT_WITH_ERROR
};

enum ERROR{
	NO_ERROR,
	ARGUMENT_ERROR,
	FILE_ERROR,
	PASSWORD_ERROR,
	CONFIG_ERROR,
	UNKNOWN_ERROR,
};

//Debug Functions:
bool CheckDebugPassword(string a);
namespace Debug {
	void __DebugHandlerThread();
	string __CallFunc(string name, vector<string> args);
}
//--------

class Filter {
public:
	Filter() { name.clear(); }
	Filter(string _name) { name = _name; }
	string name, expression;
	vector<string> vars, thrds;
};

class __Config {
public:
	vector<string> filters_unique;
	map<string, uint> filters_index;
	vector<Filter> filters;
	bool HideConsole=false, QuietMode=false, __DEBUG_MODE__=false,WorkThrdLock=true,ProcThrdLock=true;
	string ConfigFileName;
};

class __Processes {
public:
	vector<vector<uint> > filters;
	
};

extern __Config Config;
extern __Processes Processes;
extern int ErrorCode;
extern thread *DbgThrd,*WorkThrd,*ProcThrd;
extern map<string, uint> pids;

extern uint AnalArgs(int argc, char** argv);
extern bool CheckExpression(string str);
extern bool CheckThread(string str);
extern map<string, map<string, string>> ReadIniFile(const string& filename); //ByChatGPT3.5
extern bool ReadConfig();
extern void PrintHelp();
extern void PrintLog(string moudle, string msgtype, string msg, FILE* fp = stdout);
extern string GetCurrentTime();
extern void WorkThread();
extern void ProcThread();
extern vector<vector<string>> ReadCSV(FILE* fp);
extern void ErrorHandler(int code);

extern char HelpText[];
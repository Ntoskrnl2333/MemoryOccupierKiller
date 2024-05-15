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
extern thread *DbgThrd,*WorkThrd,*ProcThrd,*ActiThrd;
extern map<string, uint> pids;

extern uint AnalArgs(int argc, char** argv);
extern bool CheckExpression(string str);
extern bool CheckThread(string str);
extern map<string, map<string, string>> ReadIniFile(const string& filename); //ByChatGPT3.5  Read a INI format file
extern bool ReadConfig(); //Read config file(Config.ConfigFileName)
extern void PrintHelp(); //Print help text (help text unfinished)
extern void PrintLog(string moudle, string msgtype, string msg, FILE* fp = stdout); //Print a log
extern string GetCurrentTime(); //%Y-%m-%d %H:%M:%S
extern void WorkThread(); //Main work thread
extern void ActivityThread(); //For ThrdNo$
extern void ProcThread(); //Update process information
extern vector<vector<string>> ReadCSV(FILE* fp); //Read a csv format data from fp
extern void ErrorHandler(int code); //Print an error log and exit the appication

extern char HelpText[];
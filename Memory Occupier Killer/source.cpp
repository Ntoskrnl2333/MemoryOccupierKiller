#include "head.h"

__Config Config;
__Processes Processes;
int ErrorCode = NO_ERROR;
thread* DbgThrd, * WorkThrd, *ProcThrd;
map<string, uint> pids;
queue<string> PrintQueue;

int main(int argc,char **argv) {
	uint work = AnalArgs(argc,argv);
	if (Config.__DEBUG_MODE__)
		DbgThrd=new thread(Debug::__DebugHandlerThread);
	bool flag = true;
	switch (work) {
	case NO_WORK:
		break;
	case PRINT_HELP:
		PrintHelp();
		break;
	case RUN:
		if (!ReadConfig())
			ErrorHandler(ErrorCode);
		WorkThrd = new thread(WorkThread);
		ProcThrd = new thread(ProcThread);
		WorkThrd->join();
		ProcThrd->join();
		break;
	case EXIT_WITH_ERROR:
		ErrorHandler(ErrorCode);
		break;
	}
	ErrorHandler(NO_ERROR);
	return 0;
}

/*
/hide
/config filename
/quiet
/__DEBUG__ password
/? /help
*/
uint AnalArgs(int argc, char** argv) {
	if (argc == 0)
		return PRINT_HELP;
	vector<string> args;
	for (int i = 1; i < argc; i++) {
		string a = argv[i];
		args.push_back(a);
	}
	if (filesystem::exists("mok.ini"))
		Config.ConfigFileName = "mok.ini"; // Default config file
	for (vector<string>::iterator it = args.begin(); it != args.end(); it++) {
		if (*it == "/help" || *it == "/?")
			return PRINT_HELP;
		else if (*it == "/__DEBUG__") {
			it++;
			if (it == args.end()) {
				ErrorCode = ARGUMENT_ERROR;
				return EXIT_WITH_ERROR;
			}
			if (CheckDebugPassword(*it))
				Config.__DEBUG_MODE__ = true;
			else {
				ErrorCode = PASSWORD_ERROR;
				return EXIT_WITH_ERROR;
			}
		}
		else if (*it == "/config") {
			it++;
			if (it == args.end()) {
				ErrorCode = ARGUMENT_ERROR;
				return EXIT_WITH_ERROR;
			}
			Config.ConfigFileName = *it;
			if (!filesystem::exists(Config.ConfigFileName)) {
				ErrorCode = FILE_ERROR;
				return EXIT_WITH_ERROR;
			}
			if (!filesystem::is_regular_file(Config.ConfigFileName)) {
				ErrorCode = FILE_ERROR;
				return EXIT_WITH_ERROR;
			}
		}
		else if (*it == "/hide")
			Config.HideConsole = true;
		else if (*it == "/quiet")
			Config.QuietMode = true;
		else
			return ARGUMENT_ERROR;
	}
	if (Config.ConfigFileName.empty()) {
		ErrorCode = ARGUMENT_ERROR;
		return EXIT_WITH_ERROR;
	}
	return RUN;
}

bool CheckExpression(string str){
	int deep = 0, stacksize = 0;
	for (int i = 0; i < str.size(); i++) {
		switch (str[i]) {
		case '(':
			deep++;
			if (deep > 1)
				return false;
			break;
		case ')':
			deep--;
			if (deep < 0)
				return false;
			stacksize++;
			break;
		case 'T': case 'F':
			stacksize++;
			break;
		case '0':case '1':case '2':case '3':case '4':
		case '5':case '6':case '7':case '8':case '9':
			if (deep != 1)
				return false;
			break;
		case '&':case '|':
			if (stacksize < 2 || deep!=0)
				return false;
			stacksize--;
		}
	}
	return true;
}

bool CheckThread(string str) {
	////////////////////////////////////////////////TODO!!////////////////////////////////////////////
	return true;
}

map<string, map<string, string>> ReadIniFile(const string& filename) {//ByChatGPT3.5
	PrintLog("ReadIniFile", "Info", "Start reading INI file.");
	map<string, map<string, string>> ini_contents;
	FILE* file = nullptr;
	if (fopen_s(&file, filename.c_str(), "r") != 0) {
		//cerr << "Failed to open file: " << filename << endl;
		//return ini_contents;

		ErrorCode = FILE_ERROR;
		return ini_contents;
	}

	char line[256];
	char current_section[256];
	char key[256];
	char value[256];

	memset(current_section, 0, sizeof(current_section));

	while (fgets(line, sizeof(line), file)) {
		// Remove leading and trailing whitespace
		char* pos = strchr(line, '\n');
		if (pos != nullptr)
			*pos = '\0';

		pos = strchr(line, '\r');
		if (pos != nullptr)
			*pos = '\0';

		char* start = line;
		while (*start && isspace(*start))
			start++;

		if (*start == '\0' || *start == ';') // Skip empty lines and comments
			continue;

		if (*start == '[' && line[strlen(line) - 1] == ']') { // Check if line is a section
			strcpy_s(current_section, sizeof(current_section), start + 1);
			current_section[strlen(current_section) - 1] = '\0';
			continue;
		}

		// Find key-value pair
		char* equal_sign = strchr(start, '=');
		if (equal_sign) {
			*equal_sign = '\0';
			char* key_end = equal_sign - 1;
			while (key_end > start && isspace(*key_end))
				*key_end-- = '\0';
			strcpy_s(key, sizeof(key), start);

			char* value_start = equal_sign + 1;
			while (*value_start && isspace(*value_start))
				value_start++;

			strcpy_s(value, sizeof(value), value_start);
			char* value_end = value + strlen(value) - 1;
			while (value_end > value && isspace(*value_end))
				*value_end-- = '\0';

			ini_contents[current_section][key] = value;
		}
	}

	if (file != nullptr)
		fclose(file);
	PrintLog("ReadIniFile", "Info", "Finish reading INI file.");
	return ini_contents; 
}

#define __EXIT_WITH_CONFIG_ERROR {ErrorCode = CONFIG_ERROR;return false;}
bool ReadConfig() {
	PrintLog("ReadConfig", "Info", "Start reading configuration file.");
	ErrorCode = NO_ERROR;
	map<string, map<string, string>> config = ReadIniFile(Config.ConfigFileName);
	if (ErrorCode == FILE_ERROR)
		return false;

	if (system("tasklist > output.tmp") != 0){
		filesystem::remove("output.tmp");
		ErrorHandler(UNKNOWN_ERROR);
	}
	filesystem::remove("output.tmp");

	if (config.find("FiltersInformation") == config.end()) __EXIT_WITH_CONFIG_ERROR //Get section FiltersInformation
	map<string, string> finfo=config.find("FiltersInformation")->second;
	
	if (finfo.find("NOF") == finfo.end())__EXIT_WITH_CONFIG_ERROR //Get the number of the filters
	string NOFstr = finfo.find("NOF")->second;
	
	int NOF;
	int res=sscanf_s(NOFstr.c_str(),"%d",&NOF);
	if(res!=1 || NOF<0) __EXIT_WITH_CONFIG_ERROR

	for (int i = 1; i <= NOF; i++) {// Get the names of the filters
		string key = "FilterNo";
		key += (char)(i + '0');
		if (finfo.find(key) == finfo.end()) __EXIT_WITH_CONFIG_ERROR
		string _name = finfo.find(key)->second;

		Config.filters.push_back(Filter(_name));
	}
	
	for (int i = 1; i <= NOF; i++) {// Each filter:
		if(config.find(Config.filters[i-1].name)==config.end()) __EXIT_WITH_CONFIG_ERROR
		map<string,string> filter=config.find(Config.filters[i-1].name)->second;
		
		if (filter.find("NOV") == filter.end()) __EXIT_WITH_CONFIG_ERROR//Get the number of variables
		string _NOV=filter.find("NOV")->second;
		int NOV;
		res=sscanf_s(_NOV.c_str(), "%d", &NOV);
		if(res!=1 || NOV<0) __EXIT_WITH_CONFIG_ERROR

			for (int j = 1; j <= NOV; j++) { //Each variable
				string key = "VarNo";
				key += (char)(j + '0');
				if (filter.find(key) == filter.end()) __EXIT_WITH_CONFIG_ERROR //Get the variable
					string var = filter.find(key)->second;

				string command = "tasklist /FI \" " + var + "\" > output.tmp";//Check the veriable
				if (system(command.c_str()) != 0) {
					PrintLog("ReadConfig", "Debug", command.c_str());
					filesystem::remove("output.tmp");
					ErrorCode = CONFIG_ERROR; return false;
				}
				filesystem::remove("output.tmp");

				Config.filters[i - 1].vars.push_back(var);

				if (Config.filters_index.find(var) == Config.filters_index.end()){
					Config.filters_index[var] = Config.filters_unique.size();
					Config.filters_unique.push_back(var);
				}
		}

		//Get the expression
		if (filter.find("Expression") == filter.end()) __EXIT_WITH_CONFIG_ERROR
		string expre = filter.find("Expression")->second;
		if (!CheckExpression(expre))__EXIT_WITH_CONFIG_ERROR //Check the expression


		//Get the number of the action threads
		if (filter.find("NOT") == filter.end()) __EXIT_WITH_CONFIG_ERROR
		string _NOT = filter.find("NOT")->second;
		int NOT;
		res = sscanf_s(_NOT.c_str(), "%d", &NOT);
		if(res!=1 || NOT<0) __EXIT_WITH_CONFIG_ERROR

		for (int j = 1; j <= NOT; j++) {//Each thread:
			string key = "ThrdNo";
			key += (char)(j + '0');
			if (filter.find(key) == filter.end()) __EXIT_WITH_CONFIG_ERROR
			string thrd = filter.find(key)->second;//Get thread
			if (!CheckThread(thrd)) __EXIT_WITH_CONFIG_ERROR //Check thread

			Config.filters[i-1].thrds.push_back(thrd);
		}
	}
	PrintLog("ReadConfig", "Info", "Finished reading configuration file.");
	return true;
}

void PrintHelp() {
	puts(HelpText);
}

string GetCurrentTime() {
	// Get the current time point
	auto now = chrono::system_clock::now();

	// Convert the current time to time_t type
	time_t currentTime = chrono::system_clock::to_time_t(now);

	// Convert time_t to local time structure using localtime function
	tm* localTime = localtime(&currentTime);

	// Get the milliseconds of the current time
	auto since_epoch = now.time_since_epoch();
	auto millis = chrono::duration_cast<chrono::milliseconds>(since_epoch);
	long long milliseconds = millis.count() % 1000;

	// Use stringstream to build the date-time string
	stringstream ss;
	ss << put_time(localTime, "%Y-%m-%d %H:%M:%S");
	ss << "." << setfill('0') << setw(3) << milliseconds;

	return ss.str();
}

void PrintLog(string moudle, string msgtype, string msg, FILE* fp) { //1234-12-12 12:34:56.789 [Moudle] [Error] Message
	static bool PrintLock = false;

	if (Config.QuietMode)
		return;
	string output = GetCurrentTime();

	if (!moudle.empty()){
		output += " [";
		output += moudle;
		output += "]";
	}
	if (!msgtype.empty()) {
		output += " [";
		output += msgtype;
		output += "]";
	}
	if (!msg.empty()) {
		output += " ";
		output += msg;
	}
	output += '\n';

	while (PrintLock);
	PrintLock = true;
	fprintf(fp,"%s",output.c_str());
	PrintLock = false;
}

void WorkThread() {
	PrintLog("WorkThread","Info","Work thread has been started.");
	while (Config.WorkThrdLock) {
		for (uint i = 0; i < Config.filters.size(); i++) {
			///////////////////////////////////////////TOUPDATE/////////////////////////////////////////////
			vector<uint> acceptpids;
			/*while (Processes.filters.size() != Config.filters_unique.size());
			acceptpids=Processes.filters[Config.filters_index.find(Config.filters[i].vars[0])->second];
			for (uint j = 1; j < Config.filters[i].vars.size(); j++) {
				while (Processes.filters.size() != Config.filters_unique.size());
				vector<uint> pids = Processes.filters[Config.filters_index.find(Config.filters[i].vars[j])->second];
				bool dellast=false;
				for (auto it=acceptpids.begin();it!=acceptpids.end(); it++) {
					if (dellast) {
						dellast = false;
						acceptpids.erase(it-1);
					}
					if (find(pids.begin(), pids.end(), *it) == pids.end())
						dellast = true;
				}
			}*/ //Ignore Expression version

			vector<vector<uint> > sta;
			vector<uint> True, False, pids, first, second;
			uint len;
			True.push_back(UINT_MAX);
			False.push_back(UINT_MAX - 1);
			for (uint j = 0; j < Config.filters[i].expression.size(); j++) {
				uint code, rtn;
				char tmp[1024];
				switch (Config.filters[i].expression[j]) {
				case 'T':
					sta.push_back(True);
					break;
				case 'F':
					sta.push_back(False);
					break;
				case '(':
					rtn = sscanf(Config.filters[i].expression.substr(j+1).c_str(),"%u",&code);
					if (rtn == 0 || code > Config.filters[i].vars.size()) {
						sprintf(tmp, "Expression is bad after checking! Expression: %s Index: %u", Config.filters[i].expression.c_str(), j);
						PrintLog("WorkThread", "Warning", tmp);
						acceptpids.clear();
						break;
					}
					while (Processes.filters[code].empty());
					sta.push_back(Processes.filters[code]);

					for (; Config.filters[i].expression[j]!=')'; j++);//Go to next part
					j++;
					break;
				case '&':
					first.clear();
					second.clear();
					first = sta.back();
					sta.pop_back();
					second = sta.back();
					sta.pop_back();

					if (second.empty() || first.empty())
						sta.push_back(vector<uint>());
					else if (first == True && second == True)
						sta.push_back(vector<uint>());
					else if (first == True)
						sta.push_back(second);
					else if (second == True)
						sta.push_back(first);
					else if (first == False || second == False)
						sta.push_back(vector<uint>());
					else {
						pids.clear();
						for (uint k : first)
							if (find(second.begin(), second.end(), k)!=second.end())
								pids.push_back(k);
						sta.push_back(pids);
					}
					break;
				case '|':
					first.clear();
					second.clear();
					first = sta.back();
					sta.pop_back();
					second = sta.back();
					sta.pop_back();

					if (second.empty() || first.empty())
						sta.push_back(vector<uint>());
					else if (first == True && second == True)
						sta.push_back(vector<uint>());
					else if (first == False || second == False)
						sta.push_back(vector<uint>());
					else if (first == True)
						sta.push_back(second);
					else if (second == True)
						sta.push_back(first);
					else {
						pids.clear();
						for (uint k : first)
							pids.push_back(k);
						for (uint k : second)
							pids.push_back(k);

						sort(pids.begin(), pids.end());
						len=unique(pids.begin(), pids.end())-pids.begin();

						while (pids.size() > len)
							pids.pop_back();

						sta.push_back(pids);
					}
					break;
				}
			}

			acceptpids = sta.back();

			for (uint j : acceptpids) {
				char cmd[1024];
				sprintf(cmd,"taskkill -f -pid %u > output.tmp",j);
				system(cmd);
				filesystem::remove("output.tmp");
				PrintLog("WorkThread","Debug",cmd);
			}
		}
		Sleep(2000);
	}
	PrintLog("WorkThread","Info","Work thread has been exited.");
	WorkThrd = NULL;
}

void ProcThread() {
	uint err = 0;
	PrintLog("ProcThread", "Info", "Processes thread has been started.");
	while (Config.ProcThrdLock) {
		PrintLog("ProcThread", "Info", "Reloading processes.");

		Processes.filters.clear();
		for (uint it=0; it < Config.filters_unique.size();it++) {
			string cmd = "tasklist /fo csv /nh /fi \"";
			cmd += Config.filters_unique[it];
			cmd += '\"';

			FILE* fp = _popen(cmd.c_str(), "r");
			vector<vector<string>> res = ReadCSV(fp);
			fclose(fp);

			if (res[0].size() == 1) {
				Processes.filters.push_back(vector<uint>());
				continue;
			}

			for (uint i = 0; i < res.size(); i++)
				for (uint j = 0; j < res[i].size(); j++)
					res[i][j] = res[i][j].substr(1, res[i][j].size() - 2);
			


			Processes.filters.push_back(vector<uint>());
			for (uint i = 0; i < res.size(); i++) {
				uint pid = 0;
				uint rtn = sscanf_s(res[i][1].c_str(), "%u", &pid);
				if (rtn != 1) {
					PrintLog("ProcThread","Warning","Tasklist hasn\'t given the right output!");
					Sleep(500);
				}
				Processes.filters.back().push_back(pid);
			}
		}

		PrintLog("ProcThread", "Info", "Reload processes completed.");
		Sleep(2000);
	}
	PrintLog("ProcThread", "Info", "Processes thread has been exited.");
	ProcThrd = NULL;
	if (err != 0)
		ErrorHandler(err);
}

vector<vector<string>> ReadCSV(FILE* fp) {//By ChatGPT 3.5
	vector<vector<string>> data;
	const int bufferSize = 65535; // Define buffer size

	char *buffer= new char[bufferSize];
	while (fgets(buffer, bufferSize, fp) != nullptr) { // Read line by line
		string line(buffer);
		vector<string> row;
		size_t pos = 0;
		string token;

		// Extract each field with comma as delimiter
		while ((pos = line.find(',')) != string::npos) {
			token = line.substr(0, pos);
			row.push_back(token);
			line.erase(0, pos + 1);
		}
		row.push_back(line); // Add last field

		data.push_back(row); // Add a row of data to data
	}

	delete[] buffer;

	return data;
}

void ErrorHandler(int code){
	switch (code) {
	case NO_ERROR:
		break;
	case ARGUMENT_ERROR:
		puts("FATAL: Argument error!");
		break;
	case FILE_ERROR:
		puts("FATAL: File error!");
		break;
	case PASSWORD_ERROR:
		puts("FATAL: Password is incorrect!");
		break;
	case CONFIG_ERROR:
		puts("FATAL: Wrong configuration file!");
		break;
	case UNKNOWN_ERROR:
	default:
		printf("FATAL: Unknown error!\nError code: %d (%x).\n",code,code);
		break;
	}
	if (WorkThrd != NULL) {
		Config.WorkThrdLock = false;
		WorkThrd->join();
		delete WorkThrd;
		WorkThrd = NULL;
	}
	if (DbgThrd != NULL) {
		Config.__DEBUG_MODE__ = false;
		DbgThrd->join();
		delete DbgThrd;
		DbgThrd = NULL;
	}
	exit(code);
}

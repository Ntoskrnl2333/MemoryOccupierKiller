/*
* N2Q8E9YFHQJO2E9FD0J8A2YEFQJ23PF90EWAIOF
*/

#include "head.h"

bool CheckDebugPassword(string a){
	return !strcmp(a.c_str(),"N2Q8E9YFHQJO2E9FD0J8A2YEFQJ23PF90EWAIOF");
}

namespace Debug{

/* Commands:
null 0x00
exit 0x01
getmem 0x02 0x1122334455667788
callfunc 0x03 0x60 0x00 0x65 0x64 0x00

end: 0x55AA
*/

void __DebugHandlerThread(){
	while (Config.__DEBUG_MODE__) {
		if (filesystem::exists("DEBUG_INPUT.bin")) {
			uint fsz = filesystem::file_size("DEBUG_INPUT.bin");
			char* buf=NULL;
			FILE* fp = NULL;
			char* ptr, a;
			unsigned long long int address = 0;
			vector<char> res;

			if (fsz < 3)
				goto LABEL_END_OF_IF;

			buf = new char[fsz];

			fp = fopen("DEBUG_INPUT.bin","rb");
			fread(buf, 1, fsz, fp);
			fclose(fp);

			if (buf[fsz - 2] != 0x55 || buf[fsz - 1] != 0xAA)
				goto LABEL_END_OF_IF;
			filesystem::remove("DEBUG_INPUT.bin");
			
			switch (buf[0]) {
			case 0x00:
				break;
			case 0x01:
				ErrorHandler(NO_ERROR);
				break;
			case 0x02:
				if (fsz < 3 + 8)
					goto LABEL_END_OF_IF;
				for (uint i = 9; i >= 1; i--) {
					address <<= 8;
					address += buf[i];
				}
				ptr = (char*)address;
				a = *ptr;
				res.push_back(a);
				break;
			case 0x03:
				vector<string> args;
				string tmp;
				for (uint i = 1; i < fsz - 2; i++)
					if (buf[i] == 0x00) {
						args.push_back(tmp);
						tmp.clear();
					}
					else
						tmp += buf[i];
				string name = args.front();
				args.erase(args.begin());
				__CallFunc(name, args);
				break;
			}

			if (res.empty())
				goto LABEL_END_OF_IF;

			fp = fopen("DEBUG_OUTPUT.bin", "wb");
			for (uint i = 0; i < res.size(); i++)
				fputc(res[i], fp);
			fclose(fp);

		LABEL_END_OF_IF:
			delete[] buf;
		}
	}
}

string __CallFunc(string name,vector<string> args){
	if (name == "AnalArgs") {
		int argc;
		sscanf(args[0].c_str(), "%d", &argc);
		char **argv=new char*[32];
		for (int i = 0; i < 32; i++)
			argv[i] = new char[1024];
		strcpy(argv[0], "mok.exe");
		for (int i = 1; i <= args.size(); i++)
			strcpy(argv[i-1],args[i].c_str());
		uint res = AnalArgs(argc,argv);
		char tmp[1024];
		sprintf(tmp,"%u",res);
		for(int i=0;i<32;i++)
			delete argv[i];
		delete[] argv;
		return tmp;
	}
	else if (name == "CheckExpression") {
		bool res=CheckExpression(args[0].c_str());
		return res ? "True" : "False";
	}
	else if (name == "CheckThread") {
		bool res = CheckThread(args[0].c_str());
		return res ? "True" : "False";
	}
}

}
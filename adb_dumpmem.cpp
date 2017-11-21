#include "stdio.h"
#include "colors.h"
#include "string.h"
#include <string>
#include <cstdlib>
#include <list>
#include "sys/ioctl.h"
#include <iostream>
#include <vector>
#include <sstream>
#include "Adb.h"

#define VERSION "1.0.1"

#define STR_LEN  255

using namespace std;

struct Sizes
{
	int NativeHeap;
	int DalvikHeap;
	int Unknown;
};


int getIntAt(string line, char reg, unsigned int posFromRight)
{

	istringstream f(line);
	string s;
	int ret = 0;
	vector<int> v;
	while (getline(f, s, reg))
	{
		if (s.size() > 0)
		{
			int i = atoi(s.c_str());
			v.push_back(i);
		}
	}
	if (v.size() > posFromRight)
	{
		ret = v.at(v.size() - posFromRight);
	}
	return ret;
}


int main(int argc, char* argv[])
{
	if (argc == 1)
	{
		printf("please enter valid process name or id\n");
		return 0;
	}
	if (argc == 2 && strncmp(argv[1], "--version", 20) == 0)
	{
		printf("ADB_MEMINFO %s\n", VERSION);
		return 0;
	}

	char szCommand[STR_LEN] = "";
	sprintf(szCommand, ADB" shell dumpsys meminfo %s", argv[1]);

	FILE* fp = NULL;
	char line[STR_LEN] = "";
	
	Sizes sInit;
	Sizes sCurrent;
	memset(&sInit, 0, sizeof(Sizes));
	memset(&sCurrent, 0, sizeof(Sizes));

	bool bFirstRun = true;
	int lineCount = 0;
	
	struct winsize termSize;
	string strNativeHeap = "Native Heap";
	string strDalvikHeap = "Dalvik Heap";
	string strUnknown = "Unknown";
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &termSize);
	
	list<string> lineList;
	CLEAR_SCREEN();
	while (1)
	{
		ioctl(STDOUT_FILENO, TIOCGWINSZ, &termSize);
		lineList.clear();
		fp = popen(szCommand, "r");
		memset(line, 0, STR_LEN);

		if (NULL != fp)
		{
			while (fgets(line, STR_LEN, fp))			//execute command and get output
			{
				lineList.push_back(string(line));
			}

			//CLEAR_SCREEN();
			for (int i = 0; i < lineCount+13; i++)
			{
				CLEAR_LINE();
			}
			lineCount = 0;
			colors::DrawLine(termSize.ws_col);
			printf(ANSI_TEXT_COLOR_GREEN "ADB DUMPSYS MEMINFO" ANSI_TEXT_COLOR_RESET "\n");

			while (!lineList.empty())					//printing output at once, to avoid flickering
			{
				string str = lineList.front();
				cout << str;
				lineList.pop_front();
				lineCount++;

				
				//Native Heap
				if (str.find(strNativeHeap) != string::npos)
				{
					if (bFirstRun) sInit.NativeHeap = getIntAt(str, ' ', 3);
					else sCurrent.NativeHeap = getIntAt(str, ' ', 3);
				}

				//Dalvik Heap
				if (str.find(strDalvikHeap) != string::npos)
				{
					if (bFirstRun) sInit.DalvikHeap = getIntAt(str, ' ', 3);
					else sCurrent.DalvikHeap = getIntAt(str, ' ', 3);
				}

				//Unknown
				if (str.find(strUnknown) != string::npos)
				{
					if (bFirstRun) sInit.Unknown = getIntAt(str, ' ', 4);
					else sCurrent.Unknown = getIntAt(str, ' ', 4);
				}
			}
			
			
			cout << "\n\n" << ANSI_TEXT_COLOR_GREEN << "MEMORY STATS" << ANSI_TEXT_COLOR_RESET << endl;

			printf("%20s%15s%15s%15s\n", " ", "Native Heap", "Dalvik Heap", "Unknown");
			printf("%20s%15s%15s%15s\n", " ", "----------", "----------", "----------");
			printf("%20s%15d%15d%15d\n", "Initial Values", sInit.NativeHeap, sInit.DalvikHeap, sInit.Unknown);
			printf("%20s%15d%15d%15d\n", "Current Values:", sCurrent.NativeHeap, sCurrent.DalvikHeap, sCurrent.Unknown);
			printf(ANSI_TEXT_COLOR_RED "\n%20s%15d%15d%15d\n" ANSI_TEXT_COLOR_RESET, "DIFFERENCE:", sCurrent.NativeHeap - sInit.NativeHeap, sCurrent.DalvikHeap - sInit.DalvikHeap, sCurrent.Unknown - sInit.Unknown);
 			

			colors::DrawLine(termSize.ws_col);
			
		
		}
		usleep(500 * 1000);
		bFirstRun = false;
	}
	
	

	return 0;
}
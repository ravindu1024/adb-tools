#include <iostream>
#include <cstdio>
#include <cstring>
#include <map>
#include <string>
#include <iomanip>
#include <cstdlib>
#include <list>

#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <algorithm>

#include "colors.h"

using namespace std;

#define		MAX_BUFFER_LENGTH	2000
#define		MIN_LINE_LEN		10

#define		PID_LEN				6
#define		TAG_LEN				20
#define		MSG_OFFSET			33

#define		ARG_TAG				"-t"
#define		ARG_LOGLEVEL		"-i"
#define		ARG_EXCLUDE			"-e"
#define		ARG_FINDWORDS		"-f"
#define		ARG_DEVICE			"-d"
#define		ARG_HELP			"--help"
#define		ARG_VERSION			"--version"


struct Message
{
	int level;
	int pid;
	string tag;
	string message;
};

struct CmdOptions
{
	int loglevel;
	string device;
	string tags;
	list<string> findwords;
	list<string> ignoreWords;
};

map<string, string> argMap;
map<std::string, int> mpTagList;
CmdOptions cmd;
int color = 1;



void ProcessMessageLine(const char* line, int length, Message* msg);
char GetLogLevel(int level);
void PrintOutput(Message* msg, int maxMsgLen);
void ProcessCmdOptions();
bool DoHighlight(const char* msg);
void SetLogColor(int level);
int GetTagColor(string tag);
void PrintVersion();
void PrintHelp();
bool IgnoreTag(const char* tag);






int main(int argc, char* argv[])
{
	if (argc == 2 && (strcmp(argv[1], ARG_VERSION) == 0))
	{
		PrintVersion();
		return 0;
	}
	if (argc == 2 && (strcmp(argv[1], ARG_HELP) == 0))
	{
		PrintHelp();
		return 0;
	}

	if (argc > 1)
	{
		//Process command line options
		string argType = "";

		for (int i = 1; i < argc; i++)
		{
			if (argv[i][0] == '-')
			{
				argType = string(argv[i], 2);
				if (!(argType == ARG_DEVICE || argType == ARG_TAG || argType == ARG_FINDWORDS || 
						argType == ARG_LOGLEVEL || argType == ARG_EXCLUDE))
				{
					//invalid argument
					PrintHelp();
					return 0;
				}
			}
			else
			{
				//cout << argType << endl;
				if (argMap.find(argType) == argMap.end())
				{
					//not found. new arg
					argMap.insert(make_pair(argType, string(argv[i])));
				}
				else
				{
					//found. append to string
					string str = " " + string(argv[i]);
					argMap.at(argType).append(str);
				}

				//specialization for findwords
				if (argType == ARG_FINDWORDS)
					cmd.findwords.push_back(string(argv[i]));

				//specialization for ignore words
				if (argType == ARG_EXCLUDE)
					cmd.ignoreWords.push_back(string(argv[i]));
			}
		}

		ProcessCmdOptions();

	}



	char szLine[MAX_BUFFER_LENGTH] = "";
	Message msg;
	struct winsize termSize;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &termSize);
	int maxMsgLen = termSize.ws_col - (PID_LEN + TAG_LEN + 8);

	if (maxMsgLen < 50)
	{
		cout << "terminal width not enough..." << endl;
		return 0;
	}

	string command = "adb logcat";
	if (cmd.device.length() > 0)
		command = "adb -s " + cmd.device + " logcat";
	if (cmd.tags.length() > 0)
		command += " -s " + cmd.tags;
	
	cout << "command: " << command << endl;

	//command = "./test.logcat"; //debug only
	FILE* fp = popen(command.c_str(), "r");
	if (NULL != fp)
	{
		while (fgets(szLine, MAX_BUFFER_LENGTH, fp))
		{
			ProcessMessageLine(szLine, sizeof(szLine), &msg);
			
			PrintOutput(&msg, maxMsgLen);
		}


		fclose(fp);
	}
	else
		printf("Command Error!\n");
	
	return 0;
}


void ProcessMessageLine(const char* line, int length, Message* msg)
{
	if (strnlen(line, length) < MIN_LINE_LEN)
	{
		msg->level = -1;
		return;
	}

	//process Log Level
	if (line[0] == 'V') msg->level = 0;
	else if (line[0] == 'I') msg->level = 1;
	else if (line[0] == 'D') msg->level = 2;
	else if (line[0] == 'W') msg->level = 3;
	else if (line[0] == 'E') msg->level = 4;
	else msg->level = -1;


	//do not use strtok on the original string. it changes it.

	
	const char* firstBracket = strchr(line, '(');
	const char* secondBracket = strchr(line, ')');
	const char* tagStart = line + 2;
	
	

	//Process TAG
	if (firstBracket != NULL && secondBracket != NULL && tagStart != NULL)
	{
		msg->tag = string(tagStart, (firstBracket - tagStart));
		if(msg->tag.length() > (TAG_LEN - 1)) 
			msg->tag.erase(TAG_LEN, std::string::npos);	//delimit to 20 chars
	}
	else
		msg->tag = "**unknown";


	//Process PID
	if (firstBracket != NULL && secondBracket != NULL)
	{
		msg->pid = atoi(string(firstBracket + 1, ((secondBracket - 1) - firstBracket)).c_str());
	}
	else
		msg->pid = -1;
	
	
	//Process Message
	if (secondBracket != NULL)
	{
		msg->message = string(secondBracket + 3);
	}
	
	
	
}

char GetLogLevel(int level)
{
	switch (level)
	{
		case 0: return 'V';
		case 1: return 'I';
		case 2: return 'D';
		case 3: return 'W';
		case 4: return 'E';
		default: return 'X';
	}
}

int GetLogLevelInt(char level)
{
	switch (level)
	{
	case 'v':
	case 'V': return 0;
	case 'i':
	case 'I': return 1;
	case 'd':
	case 'D': return 2;
	case 'w':
	case 'W': return 3;
	case 'e':
	case 'E': return 4;
	default: return -1;

	}
}

void PrintOutput(Message* msg, int maxMsgLen)
{
	if (msg->level >= cmd.loglevel && !IgnoreTag(msg->tag.c_str()))
	{
		printf(ANSI_BACK_COLOR_RESET ANSI_TEXT_COLOR_RESET);

		//Print PID
		printf(ANSI_BACK_COLOR_RESET ANSI_TEXT_COLOR_RESET);
		printf("%*i ", PID_LEN, msg->pid);						//white background, black text
		printf(ANSI_BACK_COLOR_RESET ANSI_TEXT_COLOR_RESET);

		//Print TAG
		printf(ANSI_TEXT_COLOR_RESET);		
		colors::SetBackColor(GetTagColor(msg->tag));
		printf("%*s ", TAG_LEN, msg->tag.c_str());
		printf(ANSI_BACK_COLOR_RESET ANSI_TEXT_COLOR_RESET " ");

		//Print Log Level
		SetLogColor(msg->level);
		printf(" %c ", GetLogLevel(msg->level));
		printf(ANSI_BACK_COLOR_RESET ANSI_TEXT_COLOR_RESET" ");

		
		msg->message.erase(msg->message.length() - 1);
		int msgLen = msg->message.length();
		bool bHighlight = false;

		if (!cmd.findwords.empty())
			bHighlight = DoHighlight(msg->message.c_str());
		else
			bHighlight = false;

		if (msgLen > maxMsgLen)
		{
			//Need to break message to multiple lines
			int num = msgLen / maxMsgLen;
			if (num*maxMsgLen < msgLen) num = num + 1;

			for (int i = 0; i < num; i++)
			{
				if (i == 0)
				{
					if (bHighlight)
						printf(ANSI_BACK_COLOR_WHITE ANSI_TEXT_COLOR_BLACK"%s" ANSI_BACK_COLOR_RESET ANSI_TEXT_COLOR_RESET"\n", msg->message.substr(0, maxMsgLen).c_str());
					else
						printf("%s\n", msg->message.substr(0, maxMsgLen).c_str());
				}
				else
				{
					if (bHighlight)
						printf("%*s"ANSI_BACK_COLOR_WHITE ANSI_TEXT_COLOR_BLACK"%s"ANSI_BACK_COLOR_RESET ANSI_TEXT_COLOR_RESET"\n", MSG_OFFSET, "", msg->message.substr(i*maxMsgLen, maxMsgLen).c_str());
					else
						printf("%*s%s\n", MSG_OFFSET, "", msg->message.substr(i*maxMsgLen, maxMsgLen).c_str());
				}
			}
		}
		else
		{
			if (bHighlight)
				printf(ANSI_BACK_COLOR_WHITE ANSI_TEXT_COLOR_BLACK"%s"ANSI_BACK_COLOR_RESET ANSI_TEXT_COLOR_RESET"\n", msg->message.c_str());
			else
				printf("%s\n", msg->message.c_str());
		}

		
	}
}


void ProcessCmdOptions()
{
	if(argMap.find(ARG_DEVICE) != argMap.end()) cmd.device = argMap.at(ARG_DEVICE);
	if (argMap.find(ARG_LOGLEVEL) != argMap.end())cmd.loglevel = GetLogLevelInt(argMap.at(ARG_LOGLEVEL).c_str()[0]);
	if (argMap.find(ARG_TAG) != argMap.end())cmd.tags = argMap.at(ARG_TAG);
	
	//findwords have already been processed
	//for (std::list<string>::iterator itr = cmd.findwords.begin(); itr != cmd.findwords.end(); itr++)
	//	cout << "find: " << *itr << endl;

	//ignore words have already been processed
	//for (std::list<string>::iterator itr = cmd.ignoreWords.begin(); itr != cmd.ignoreWords.end(); itr++)
	//	cout << "ignore: " << *itr << endl;
}


bool DoHighlight(const char* msg)
{
	for (std::list<string>::iterator itr = cmd.findwords.begin(); itr != cmd.findwords.end(); itr++)
	{
		if (NULL != (strstr(msg, (*itr).c_str())))
			return true;
	}
	return false;
}

bool IgnoreTag(const char* tag)
{
	bool bRet = false;
	bool bUseWildcard = false;

	//cout << "usewl: " << bUseWildcard << endl;
	if (tag != NULL)
	{
		for (std::list<string>::iterator itr = cmd.ignoreWords.begin(); itr != cmd.ignoreWords.end(); itr++)
		{
			if (itr->find('*') == itr->size()-1) bUseWildcard = true;

			if (bUseWildcard)
			{
				if (NULL != (strstr(tag, itr->substr(0, itr->size() - 1).c_str())))
					bRet = true;
			}
			else
			{
				if (strncmp(tag, itr->c_str(), TAG_LEN) == 0)
				{
					bRet = true;
				}
			}
		}
		
	}
	return bRet;
}


void SetLogColor(int level)
{
	printf(ANSI_TEXT_COLOR_BLACK);
	switch (level)
	{
	case 0: printf(ANSI_BACK_COLOR_WHITE);
		break;
	case 1: printf(ANSI_BACK_COLOR_GREEN);
		break;
	case 2: printf(ANSI_BACK_COLOR_BLUE);
		break;
	case 3: printf(ANSI_BACK_COLOR_YELLOW);
		break;
	case 4: printf(ANSI_BACK_COLOR_RED);
		break;
	}
}



int GetTagColor(string tag)
{
	
	if (mpTagList.find(tag) == mpTagList.end())
	{
		mpTagList.insert(std::make_pair(tag, color++));
		if (color == 8)
		{
			color = 1;
			return 7;
		}
		else return color;
	}
	else
	{
		//have tag. return its color
		return mpTagList[tag];
	}
		
}


void PrintVersion()
{
	cout << "ADB logcat version 1.2\n" << endl;
}

void PrintHelp()
{
	cout << "ADB logcat v1.2" << endl;
	cout << "Usage:" << endl;
	cout << ARG_TAG" : select tags" << endl;
	cout << ARG_DEVICE" : select device" << endl;
	cout << ARG_FINDWORDS" : highlight given words" << endl;
	cout << ARG_LOGLEVEL" : set minimum log level (D - debug, E - error, I - info, V - verbose, W - warning)" << endl;
	cout << ARG_EXCLUDE" : exclude tags by wildcard" << endl;
	cout << ARG_VERSION" : show program version" << endl;
	cout << ARG_HELP" : show this help section" << endl;
	cout << "Example usage: logcat -d DEVICE_NAME -t TAG1 TAG2 TAG3 -l D -f word1 word2 \"word3 word4\"" << endl;
	cout << "\t\tlogcat -e TAG1 TAG*\n" << endl;
}
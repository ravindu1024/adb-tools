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
using namespace colors;

int main(int argc, char* argv[])
{
	if (argc == 2 && strncmp(argv[1], "--version", 20) == 0)
	{
		printf("ADB_TOP %s\n", VERSION);
		return 0;
	}

	char szCommand[STR_LEN] = "";
	sprintf(szCommand, ADB" shell top -d 1");

	FILE* fp = NULL;
	char line[STR_LEN] = "";
	list<string> lineList;
	struct winsize termSize;
	int count = 0;
	int lineCount = 20;

	if (argc > 1)
	{
		lineCount = atoi(argv[1]);
	}

	ioctl(STDOUT_FILENO, TIOCGWINSZ, &termSize);
	fp = popen(szCommand, "r");
	memset(line, 0, STR_LEN);

	CLEAR_SCREEN();
	DrawLine(termSize.ws_col);
	printf("\n" ANSI_TEXT_COLOR_GREEN "ADB TOP" ANSI_TEXT_COLOR_RESET);

	if (NULL != fp)
	{
		while (fgets(line, STR_LEN, fp))			//execute command and get output
		{
			count++;
			if (string(line).find("PID PR") != string::npos)
			{
				count = 0;
				//CLEAR_SCREEN();
				for (int i = 0; i < lineCount + 3; i++)
				{
					CLEAR_LINE();
				}
				
				//ioctl(STDOUT_FILENO, TIOCGWINSZ, &termSize);
				//printf("test\n");
				DrawLine(termSize.ws_col);
				//printf("\n" ANSI_TEXT_COLOR_GREEN "ADB TOP" ANSI_TEXT_COLOR_RESET "\n\n");
			}
			if (count < lineCount + 1)
			{
				if (count == 0){
					cout << ANSI_TEXT_COLOR_BLACK ANSI_BACK_COLOR_WHITE << line << ANSI_BACK_COLOR_RESET ANSI_TEXT_COLOR_RESET;
				}
				else cout << line;
			}
			else if (count == lineCount+1)
			{
				DrawLine(termSize.ws_col);
			}

			
		}

		
	}
	usleep(500 * 1000);


	return 0;
}
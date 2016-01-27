#include <unistd.h>
#include <iostream>
#include <iomanip>

#define ANSI_TEXT_COLOR_BLACK   "\x1b[30m"
#define ANSI_TEXT_COLOR_RED     "\x1b[31m"
#define ANSI_TEXT_COLOR_GREEN   "\x1b[32m"
#define ANSI_TEXT_COLOR_YELLOW  "\x1b[33m"
#define ANSI_TEXT_COLOR_BLUE    "\x1b[34m"
#define ANSI_TEXT_COLOR_MAGENTA "\x1b[35m"
#define ANSI_TEXT_COLOR_CYAN    "\x1b[36m"
#define ANSI_TEXT_COLOR_WHITE   "\x1b[37m"
#define ANSI_TEXT_COLOR_RESET   "\x1b[39m"

#define ANSI_BACK_COLOR_BLACK     	"\x1b[40m"
#define ANSI_BACK_COLOR_RED     	"\x1b[41m"
#define ANSI_BACK_COLOR_GREEN     	"\x1b[42m"
#define ANSI_BACK_COLOR_YELLOW     	"\x1b[43m"
#define ANSI_BACK_COLOR_BLUE     	"\x1b[44m"
#define ANSI_BACK_COLOR_MAGENTA     "\x1b[45m"
#define ANSI_BACK_COLOR_CYAN     	"\x1b[46m"
#define ANSI_BACK_COLOR_WHITE     	"\x1b[47m"
#define ANSI_BACK_COLOR_RESET     	"\x1b[49m"


#define CLEAR_SCREEN() printf("\033[2J\033[1;1H");
#define CLEAR_LINE() printf("\033[A\033[2K");

namespace colors
{


	void DrawLine(int length)
	{
		using namespace std;
		cout << ANSI_TEXT_COLOR_GREEN << ANSI_BACK_COLOR_GREEN;
		cout << setfill('=') << setw(length) << "=";
		cout << ANSI_TEXT_COLOR_RESET << ANSI_BACK_COLOR_RESET << endl;
	}

	void SetTextColor(int color)
	{
		if (color < 1 || color > 7) return;
		else
		{
			//7 colors from 31 to 37
			color = color + 30;
			printf(ANSI_TEXT_COLOR_RESET);
			printf("\x1b[%im", color);
		}
	}

	void SetBackColor(int color)
	{
		if (color < 1 || color > 7) return;
		else
		{
			//7 colors from 31 to 37
			color = color + 40;
			printf(ANSI_BACK_COLOR_RESET);
			printf("\x1b[%im", color);
		}
	}

	void ResetColor()
	{
		printf(ANSI_BACK_COLOR_RESET ANSI_TEXT_COLOR_RESET);
	}

}
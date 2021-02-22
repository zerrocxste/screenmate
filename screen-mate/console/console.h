#include "../includes.h"

namespace console
{
	static bool is_open = false;
	inline void attach(const char* title)
	{
		if (is_open == false)
		{
			AllocConsole();
			freopen("conout$", "w", stdout);
			SetConsoleTitle(title);
			SetWindowPos(GetConsoleWindow(), 0, 10, 10, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
			is_open = true;
		}
	}
}
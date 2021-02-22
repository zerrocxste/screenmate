#include "../includes.h"

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	std::vector<HWND>* hwnds = (std::vector<HWND>*) lParam;

	if (IsWindowVisible(hwnd) && (!GetWindow(hwnd, GW_OWNER)))
	{
		char str[512] = { 0 };

		auto window_style = GetWindowLong(hwnd, GWL_STYLE);

		if (GetWindowText(hwnd, str, 511) && (bool)(window_style & WS_MINIMIZE) == false)
		{
			if (strcmp(str, "Program Manager") == NULL
				|| strcmp(str, "Microsoft Edge") == NULL
				|| strcmp(str, "Microsoft Text Input Application") == NULL)
				return 1;

			/*std::cout << str << std::endl;*/

			hwnds->push_back(hwnd);
		}
	}

	return 1;
}

namespace process_enumerator
{
	std::vector<HWND>v_process_enumerator;

	void process_enumerator_thread()
	{
		while (true)
		{
			std::vector<HWND> templates;
			EnumWindows(&EnumWindowsProc, (LPARAM)&templates);
			Sleep(10); //adjust this value for better speed of updating window information, but also adjust for better performance
			v_process_enumerator = templates;
			/*system("cls");*/
		}
	}
}


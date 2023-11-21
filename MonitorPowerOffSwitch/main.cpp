#include <iostream>
#include <windows.h>
#include <vector>
#include <lowlevelmonitorconfigurationapi.h>
#include <windowsx.h>

#pragma comment(lib, "Dxva2.lib")

#define KEY_DOWN(key) ((::GetAsyncKeyState(key) & 0x80000) ? 1 : 0)
#define KEY_UP(key)   ((::GetAsyncKeyState(key) & 0x80000) ? 0 : 1)

// VCP Code defined in VESA Monitor Control Command Set (MCCS) standard
const BYTE PowerMode = 0xD6;  
// Samsung only
// const BYTE PowerMode = 0xE1;  

const DWORD PowerOn = 0x01;
const DWORD PowerOff = 0x04;

// Monitor description struct
struct MonitorDesc
{
	HANDLE hdl;
	DWORD power;
};

// Monitor enumeration callback
BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
	std::vector<MonitorDesc>* pMonitors = reinterpret_cast<std::vector<MonitorDesc>*>(dwData);

	DWORD nMonitorCount;
	if (GetNumberOfPhysicalMonitorsFromHMONITOR(hMonitor, &nMonitorCount))
	{
		PHYSICAL_MONITOR* pMons = new PHYSICAL_MONITOR[nMonitorCount];

		if (GetPhysicalMonitorsFromHMONITOR(hMonitor, nMonitorCount, pMons))
		{
			for (DWORD i = 0; i < nMonitorCount; i++)
			{
				MonitorDesc desc;
				desc.hdl = pMons[i].hPhysicalMonitor;

				pMonitors->push_back(desc);
			}
		}
		delete[] pMons;
	}
	return TRUE;
}

// Switch monitor power
void MonitorSwitch(MonitorDesc& monitor, DWORD mode)
{
	if (monitor.power == mode)
	{
		return;
	}

	bool result = SetVCPFeature(monitor.hdl, PowerMode, mode);
	std::error_code ec(GetLastError(), std::system_category());

	if (ec)
	{
		std::cout << "message: " << ec.message() << std::endl;
	}

	monitor.power = mode;
}

void HelpMessage()
{
	std::cout << "MonitorPowerOffSwitch.exe - run background" << std::endl;
	std::cout << "MonitorPowerOffSwitch.exe -v or MonitorPowerOffSwitch.exe --verbose - print debug message" << std::endl;

	std::cout << "CTRL+ALT+L - power off current monitor" << std::endl;
	std::cout << "SHIFT - power on current monitor" << std::endl;
	std::cout << "CTRL+ALT+1 - switch current monitor to 1" << std::endl;
	std::cout << "CTRL+ALT+2 - switch current monitor to 2" << std::endl;
	std::cout << "CTRL+ALT+3 - switch current monitor to 3" << std::endl;
	std::cout << "CTRL+ALT+4 - switch current monitor to 4" << std::endl;
	std::cout << "WinKey+Z - switch power on or off monitor on mouse moving" << std::endl;
	std::cout << "for exit press CTRL+E" << std::endl;
}

int main(int argc, char* argv[])
{
	bool runBackground = true;

	if (argc == 1)
	{
		runBackground = true;
	}
	else if (argc == 2) {
		if (std::string(argv[1]) == "--verbose" || std::string(argv[1]) == "-v")
		{
			runBackground = false;
		}

		if (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h")
		{
			runBackground = false;
			HelpMessage();
			return 0;
		}
	}
	else {
		HelpMessage();
	}

	if (runBackground)
	{
		// Hiding the console
		HWND hWnd;
		AllocConsole();
		hWnd = FindWindowA("ConsoleWindowClass", NULL);
		ShowWindow(hWnd, 0);
	}

	std::vector<MonitorDesc> monitors;
	EnumDisplayMonitors(NULL, NULL, &MonitorEnumProc, reinterpret_cast<LPARAM>(&monitors));

	if (!runBackground)
	{
		std::cout << "monitors count: " << monitors.size() << std::endl;
	}

	// Init
	for (auto& monitor : monitors)
	{
		monitor.power = PowerOn;
	}

	int currentMonitorID = 0; //need start this exe in monitors[0] 
	// Here select the first one monitor as example
	MonitorDesc targetMonitor = monitors[currentMonitorID];
	bool targetMonitorChange = true;

	//Mouse position
	LONG zx = -1;
	LONG zy = -1;
	POINT ptB = { 0, 0 };

	bool debug_mouse_pos = false;

	while (1)
	{
		/*---------------------------------------------------------------*/
		/*-                          -                                  -*/
		/*-                          -                                  -*/
		/*-                          -                                  -*/
		/*-     monitors[0]          -             monitors[1]          -*/
		/*-                          -                                  -*/
		/*-                          -                                  -*/
		/*-                          -                                  -*/
		/*---------------------------------------------------------------*/
		/*                       {1919,1079}                             */

		LPPOINT xy = &ptB;   //Location variables
		GetCursorPos(xy);    //Gets the current mouse position        

		if (debug_mouse_pos)
		{
			//If the mouse moves, (i.e. the current coordinates change to print out the coordinates) print out the coordinates.
			if ((zx != xy->x) || (zy != xy->y))
			{
				//Here you need to test the edge of your monitor[0]
				//After Test, delete this and Hide the console by ShowWindow(hWnd, 0)
				printf("x=%d, y=%d\n", xy->x, xy->y);
			}
		}

		if (::GetAsyncKeyState('Z') == -32767)
		{
			if (KEY_DOWN(VK_LWIN))
			{
				//The coordinate in the lower right corner of my monitor is {1919,1079}
				if (xy->x > 1919 && currentMonitorID == 0)
				{
					currentMonitorID = 1;
					MonitorSwitch(monitors[1], PowerOn);
					MonitorSwitch(monitors[0], PowerOff);
				}
				else if (xy->x <= 1919 && currentMonitorID == 1)
				{
					currentMonitorID = 0;
					MonitorSwitch(monitors[0], PowerOn);
					MonitorSwitch(monitors[1], PowerOff);
				}
			}
		}

		if (::GetAsyncKeyState('L') == -32767)
		{
			if (KEY_DOWN(VK_CONTROL) && KEY_DOWN(VK_MENU))
			{
				// turn off
				if (targetMonitor.power == PowerOn)
				{
					MonitorSwitch(targetMonitor, PowerOff);
				}
			}
		}

		if (::GetAsyncKeyState(VK_SHIFT) == -32767)
		{
			if (KEY_DOWN(VK_SHIFT))
			{
				// turn on
				MonitorSwitch(targetMonitor, PowerOn);
			}
		}

		if (::GetAsyncKeyState('E') == -32767)
		{
			// turn on
			MonitorSwitch(targetMonitor, PowerOn);

			if (KEY_DOWN(VK_CONTROL))
			{
				return 0;
			}
		}

		if (::GetAsyncKeyState('1') == -32767)
		{
			if (KEY_DOWN(VK_CONTROL) && KEY_DOWN(VK_MENU))
			{
				if (monitors.size() > 0)
				{
					currentMonitorID = 0;
					targetMonitor = monitors[currentMonitorID];
					targetMonitorChange = true;
				}
			}
		}

		if (::GetAsyncKeyState('2') == -32767)
		{
			if (KEY_DOWN(VK_CONTROL) && KEY_DOWN(VK_MENU))
			{
				if (monitors.size() > 1)
				{
					currentMonitorID = 1;
					targetMonitor = monitors[currentMonitorID];
					targetMonitorChange = true;
				}
			}
		}

		if (::GetAsyncKeyState('3') == -32767)
		{
			if (KEY_DOWN(VK_CONTROL) && KEY_DOWN(VK_MENU))
			{
				if (monitors.size() > 2)
				{
					currentMonitorID = 2;
					targetMonitor = monitors[currentMonitorID];
					targetMonitorChange = true;
				}
			}
		}

		if (::GetAsyncKeyState('4') == -32767)
		{
			if (KEY_DOWN(VK_CONTROL) && KEY_DOWN(VK_MENU))
			{
				if (monitors.size() > 3)
				{
					currentMonitorID = 3;
					targetMonitor = monitors[currentMonitorID];
					targetMonitorChange = true;
				}
			}
		}

		if (targetMonitorChange)
		{
			std::cout << "targetMonitor.hdl: " << targetMonitor.hdl << std::endl;
			std::cout << "currentMonitorID: " << currentMonitorID << std::endl;
			targetMonitorChange = false;
		}

		// bug fix for 100% CPU load
		Sleep(1);
	}
}
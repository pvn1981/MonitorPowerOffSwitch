#include <iostream>
#include <windows.h>
#include <vector>
#include <lowlevelmonitorconfigurationapi.h>
#include <windowsx.h>

#pragma comment(lib, "Dxva2.lib")

#define KEY_DOWN(key) ((::GetAsyncKeyState(key) & 0x80000) ? 1 : 0)
#define KEY_UP(key)   ((::GetAsyncKeyState(key) & 0x80000) ? 0 : 1)

const BYTE PowerMode = 0xD6;  // VCP Code defined in VESA Monitor Control Command Set (MCCS) standard
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

	SetVCPFeature(monitor.hdl, PowerMode, mode);
	monitor.power = mode;
}

void HelpMessage()
{
	std::cout << "MonitorPowerOffSwitch.exe - run background" << std::endl;
	std::cout << "MonitorPowerOffSwitch.exe -v or MonitorPowerOffSwitch.exe --verbose - print debug message" << std::endl;
}

int main(int argc, char* argv[])
{
	bool runBackground = true;

	if (argc == 1)
	{
		runBackground = true;

	}
	else if (argc == 2) {
		runBackground = false;

		if (std::string(argv[1]) == "--verbose" || std::string(argv[1]) == "-v")
		{
			runBackground = false;
		}

		if (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h")
		{
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

	// Here select the first one monitor as example
	MonitorDesc targetMonitor = monitors[0];
	bool targetMonitorChange = true;

	if (!runBackground)
	{
		std::cout << "for exit press CTRL+ALT+L power off current monitor" << std::endl;
		std::cout << "for exit press CTRL+SHIFT - power on current monitor" << std::endl;
		std::cout << "for exit press CTRL+E" << std::endl;
	}

	while (1)
	{
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
					targetMonitor = monitors[0];
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
					targetMonitor = monitors[1];
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
					targetMonitor = monitors[2];
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
					targetMonitor = monitors[3];
					targetMonitorChange = true;
				}
			}
		}

		if (targetMonitorChange)
		{
			std::cout << "targetMonitor.hdl: " << targetMonitor.hdl << std::endl;
			targetMonitorChange = false;
		}

		// bug fix for 100% CPU load
		Sleep(1);
	}
}
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

int main()
{
	// Скрытие консоли 
	HWND hWnd;
	AllocConsole();
	hWnd = FindWindowA("ConsoleWindowClass", NULL);
	ShowWindow(hWnd, 0);

	std::vector<MonitorDesc> monitors;
	EnumDisplayMonitors(NULL, NULL, &MonitorEnumProc, reinterpret_cast<LPARAM>(&monitors));

	// Init
	for (auto& monitor : monitors)
	{
		monitor.power = PowerOn;
	}

	// Here select the first one monitor as example
	MonitorDesc targetMonitor = monitors[0];

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
				}
			}
		}

		// bug fix for 100% CPU load
		Sleep(1);
	}
}
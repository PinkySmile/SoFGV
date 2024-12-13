// From:Raymond Chen https://devblogs.microsoft.com/oldnewthing/20160627-00/?p=93755
// Compiles and runs with VS2022 Ent
#pragma comment(lib, "comctl32.lib")
#define STRICT
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <strsafe.h>
#include <stdio.h>
#include <stdlib.h>
#define HANDLE_WM_INPUT(hwnd, wParam, lParam, fn) ((fn)((hwnd), GET_RAWINPUT_CODE_WPARAM(wParam), (HRAWINPUT)(lParam)), 0)

HINSTANCE g_hinst;
HWND g_hwndChild;

void OnInput(HWND hwnd, WPARAM code, HRAWINPUT hRawInput)
{
	UINT dwSize;
	GetRawInputData(hRawInput, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
	RAWINPUT* input = (RAWINPUT*)malloc(dwSize);
	GetRawInputData(hRawInput, RID_INPUT, input, &dwSize, sizeof(RAWINPUTHEADER));
	if (input->header.dwType == RIM_TYPEKEYBOARD) {
		TCHAR prefix[80];
		prefix[0] = TEXT('\0');
		if (input->data.keyboard.Flags & RI_KEY_E0) {
			StringCchCat(prefix, ARRAYSIZE(prefix), TEXT("E0 "));
		}
		if (input->data.keyboard.Flags & RI_KEY_E1) {
			StringCchCat(prefix, ARRAYSIZE(prefix), TEXT("E1 "));
		}
		UINT name_size = 125;
		TCHAR name_buffer[125];
		name_buffer[0] = 0;
		GetRawInputDeviceInfo(input->header.hDevice, RIDI_DEVICENAME, name_buffer, &name_size);
		TCHAR buffer[512];
		StringCchPrintf(
			buffer, ARRAYSIZE(buffer),
			TEXT("%p, %s, msg=%04x, vk=%04x, scanCode=%s%02x, %s"),
			input->header.hDevice,
			name_buffer,
			input->data.keyboard.Message,
			input->data.keyboard.VKey,
			prefix,
			input->data.keyboard.MakeCode,
			(input->data.keyboard.Flags & RI_KEY_BREAK) ? TEXT("release") : TEXT("press")
		);
		ListBox_AddString(g_hwndChild, buffer);
	}
	DefRawInputProc(&input, 1, sizeof(RAWINPUTHEADER));
	free(input);
}

void OnSize(HWND hwnd, UINT state, int cx, int cy)
{
	if (g_hwndChild) {
		MoveWindow(g_hwndChild, 0, 0, cx, cy, TRUE);
	}
}

BOOL OnCreate(HWND hwnd, LPCREATESTRUCT lpcs)
{
	g_hwndChild = CreateWindow(TEXT("listbox"), NULL, LBS_HASSTRINGS | WS_CHILD | WS_VISIBLE | WS_VSCROLL, 0, 0, 0, 0, hwnd, NULL, g_hinst, 0);
	RAWINPUTDEVICE dev;
	dev.usUsagePage = 1;
	dev.usUsage = 6;
	dev.dwFlags = 0;
	dev.hwndTarget = hwnd;
	RegisterRawInputDevices(&dev, 1, sizeof(dev));
	return TRUE;
}

void OnDestroy(HWND hwnd)
{
	RAWINPUTDEVICE dev;
	dev.usUsagePage = 1;
	dev.usUsage = 6;
	dev.dwFlags = RIDEV_REMOVE;
	dev.hwndTarget = hwnd;
	RegisterRawInputDevices(&dev, 1, sizeof(dev));
	PostQuitMessage(0);
}

void OnPaint(HWND hwnd)
{
	PAINTSTRUCT ps;
	BeginPaint(hwnd, &ps);
	EndPaint(hwnd, &ps);
}

void OnPrintClient(HWND hwnd, HDC hdc)
{
	PAINTSTRUCT ps;
	ps.hdc = hdc;
	GetClientRect(hwnd, &ps.rcPaint);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uiMsg) {
	HANDLE_MSG(hwnd, WM_CREATE, OnCreate);
	HANDLE_MSG(hwnd, WM_SIZE, OnSize);
	HANDLE_MSG(hwnd, WM_DESTROY, OnDestroy);
	HANDLE_MSG(hwnd, WM_PAINT, OnPaint);
	HANDLE_MSG(hwnd, WM_INPUT, OnInput);
	case WM_PRINTCLIENT:
		OnPrintClient(hwnd, (HDC)wParam);
	return 0;
	}
	return DefWindowProc(hwnd, uiMsg, wParam, lParam);
}

BOOL InitApp(void)
{
	WNDCLASS wc;
	wc.style = 0;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = g_hinst;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = TEXT("GetRawInputData");
	if (!RegisterClass(&wc))
		return FALSE;
	return TRUE;
}

int WINAPI WinMain(HINSTANCE hinst, HINSTANCE hinstPrev, LPSTR lpCmdLine, int nShowCmd)
{
	MSG msg;
	HWND hwnd;
	g_hinst = hinst;
	if (!InitApp())
		return 0;
	if (SUCCEEDED(CoInitialize(NULL))) {/* In case we use COM */
		hwnd = CreateWindow(
			TEXT("GetRawInputData"), /* Class Name */
			TEXT("GetRawInputData"), /* Title */
			WS_OVERLAPPEDWINDOW, /* Style */
			CW_USEDEFAULT, CW_USEDEFAULT, /* Position */
			CW_USEDEFAULT, CW_USEDEFAULT, /* Size */
			NULL, /* Parent */
			NULL, /* No menu */
			hinst, /* Instance */
			0
		); /* No special parameters */
		ShowWindow(hwnd, nShowCmd);
		while (GetMessage(&msg, NULL, 0, 0)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		CoUninitialize();
	}
	return 0;
}
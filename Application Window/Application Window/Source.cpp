#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <clocale>
#include <iostream>
#include <iomanip>
#include <objbase.h>
#include <math.h>
#include <winnt.h>
#include <string>
#include <fstream>


#include ".\include\ioctl.h"
#include ".\include\ifc_ldev.h"
#include ".\include\create.h"
#include ".\include\pcicmd.h"
#include ".\include\791cmd.h"
#include ".\include\e2010cmd.h"
#include ".\include\e154cmd.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		case WM_CREATE:
		{
			HWND hButton = CreateWindow(L"BUTTON", L"OK!", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 300, 30, hWnd, reinterpret_cast<HMENU>(1337), nullptr, nullptr);
			HWND hButton1 = CreateWindow(L"BUTTON", L"OK!", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 60, 300, 30, hWnd, reinterpret_cast<HMENU>(0), nullptr, nullptr);
		}
		return 0;
		case WM_DESTROY:
		{
			PostQuitMessage(EXIT_SUCCESS);
		}
		return 0;

		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case 1337:
				MessageBox(hWnd, L"DO", L"NE", MB_ICONINFORMATION);
			}
		}
	}
	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

/*int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow) {
	MSG msg{};
	HWND hwnd{};
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
	wcex.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
	wcex.hInstance = hInstance;
	wcex.lpfnWndProc = WndProc;
	wcex.lpszClassName = L"MyAppClass";
	wcex.lpszMenuName = nullptr;
	wcex.style = CS_HREDRAW | CS_VREDRAW;

	if (!RegisterClassEx(&wcex))
		return EXIT_FAILURE;
	if (hwnd = CreateWindow(wcex.lpszClassName, L"Заголовок", WS_OVERLAPPEDWINDOW, 0, 0, 600, 600, nullptr, nullptr, wcex.hInstance, nullptr); hwnd == INVALID_HANDLE_VALUE)
		return EXIT_FAILURE;
	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
	return static_cast<int>(msg.wParam);
}

*/
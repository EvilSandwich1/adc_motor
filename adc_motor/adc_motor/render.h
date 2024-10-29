#pragma once
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#define WIN64

#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <numeric>
#include <stdio.h>
#include <conio.h>
#include <clocale>
#include <iostream>
#include <iomanip>
#include <windowsx.h>
#include <objbase.h>
#include <math.h>
#include <winnt.h>
#include <string>
#include <fstream>
#include <sstream>
#include <exception>
#include <array>
#include <vector>
#include <algorithm>

#include ".\include\imgui\imgui.h"
#include ".\include\imgui\imconfig.h"
#include ".\include\imgui\imgui_impl_dx12.h"
#include ".\include\imgui\imgui_impl_win32.h"
#include ".\include\implot\implot.h"
#include ".\include\implot\implot_internal.h"
#include ".\include\pffft\pffft.h"
#include <d3d12.h>
#include <dxgi1_4.h>
#include <tchar.h>

class render 
{

	public: struct FrameContext
	{
		ID3D12CommandAllocator* CommandAllocator;
		UINT64                  FenceValue;
	};
	struct ScrollingBuffer {
		int MaxSize;
		int Offset;
		ImVector<ImVec2> Data;
		ScrollingBuffer(int max_size = 2000) {
			MaxSize = max_size;
			Offset = 0;
			Data.reserve(MaxSize);
		}
		  void AddPoint(float x, float y) {
			  if (Data.size() < MaxSize)
				  Data.push_back(ImVec2(x, y));
			  else {
				  Data[Offset] = ImVec2(x, y);
				  Offset = (Offset + 1) % MaxSize;
			  }
		  }
		  void Erase() {
			  if (Data.size() > 0) {
				  Data.shrink(0);
				  Offset = 0;
			  }
		  }
	  };

	enum class CTL_ID {
		TEXTFIELD,
		PLACEHOLDER
	};
public:
	explicit render();
	enum class TIMER_ID {
		IDT_TIMER
	};
	void init_window_class();
	void settext(HWND hwnd, std::wstring str);
	~render();
	int current_window;
	int npoints_global;
	int samples_global;
	HWND m_hWnd{}, m_hWndButton{}, m_hWndEdit{};
	static LRESULT CALLBACK app_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK wnd_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	//void paint(std::vector<float>& data);
	void demo();
	void d3dctx();
	void scrolling(float data);
	void oscilloscope(std::vector<float> data, int samples);
	void fourier(std::vector<float>& data, int npoints);
	void create_controls();
	bool CreateDeviceD3D(HWND hWnd);
	void CleanupDeviceD3D();
	void CreateRenderTarget();
	void CleanupRenderTarget();
	void WaitForLastSubmittedFrame();
	FrameContext* WaitForNextFrameResources();
	void cleanup(LPCWSTR classname);
};
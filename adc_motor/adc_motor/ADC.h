#pragma once
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#define INITGUID
#define WIN64
#define _USE_MATH_DEFINES
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
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
#include <vector>
#include <numeric>

#include ".\include\ioctl.h"
#include ".\include\ifc_ldev.h"
#include ".\include\create.h"
#include ".\include\pcicmd.h"
#include ".\include\791cmd.h"
#include ".\include\e2010cmd.h"
#include ".\include\e154cmd.h"

class ADC 
{
	enum class CTL_ID {
		TEXTFIELD,
		PLACEHOLDER
	};
	enum class TIMER_ID {
		IDT_TIMER
	};
	public: 
		std::vector<float> buffer_long;
		std::vector<float> pffft_buffer;
		bool ready_swap;
		explicit ADC();
		//int run();
		~ADC();
		//void init_window_class();
		float frame();
		//float* frame();
		void* stream();
		std::vector<float>& data_proc();
		bool stream_setup();
		float data_avg(std::vector<float>& dataStream);
		//void pffft(int npoints, int window_id);
		bool init_e440();
		void disconnect();
	private:
		HINSTANCE CallCreateInstance(LPCWSTR name);
		//void create_controls();
		//void display();
		//UINT_PTR const IDT_TIMER;
};
#pragma once
#include <Windows.h>
#include <stdio.h>
#include <tchar.h>
#include <iostream>;
#include <fstream>
#include <conio.h>
#include <string>
#include <thread>
#include <future>
#include <chrono>
#include "ADC.h"
#define ASCII

using namespace std::literals;

class StpCoord {
public:
	float x;
	float y;
	float z;
};

class AlgCoord {
public:
	float begin_x;
	float step_x;
	float end_x;
	float begin_y;
	float step_y;
	float end_y;
	float begin_z;
	float step_z;
	float end_z;
};

class stepper {
public:
	explicit stepper();
	~stepper();
	bool initialize();
	std::string read();
	void write_cmd(std::string& outputData);
	void just(float speed);
	StpCoord get_current_coord();
	bool home();
	bool move(float val, std::string coord);
	bool algorithm(AlgCoord coord, ADC adc, int delay);
	bool algorithm_smart(AlgCoord coord, ADC adc, int delay);
	void test(AlgCoord coord);
	void close();
	bool global_just_flg;
};


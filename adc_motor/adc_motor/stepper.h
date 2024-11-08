#pragma once
#include <Windows.h>
#include <stdio.h>
#include <tchar.h>
#include <iostream>;
#include <conio.h>
#include <string>
#define ASCII

class StpCoord {
public:
	float x;
	float y;
	float z;
};

class stepper {
public:
	explicit stepper();
	~stepper();
	bool initialize();
	char* read();
	void write_cmd(char* outputData);
	StpCoord get_current_coord();
	void close();
};


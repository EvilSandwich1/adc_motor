#pragma once
#include <Windows.h>
#include <stdio.h>
#include <tchar.h>
#include <iostream>;
#include <conio.h>
#define ASCII

class STEPPER {
public:
	explicit STEPPER();
	void initialize();
	void read();
	void writeCmd(LPCVOID outputData);
};
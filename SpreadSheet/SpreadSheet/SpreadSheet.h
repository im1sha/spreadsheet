#pragma once

#ifndef UNICODE
#define UNICODE
#endif // !UNICODE

#include "GraphicDefines.h"
#include <string>

class SpreadSheet {
public:

	SpreadSheet(HWND hWnd, HINSTANCE hInstanse);

	~SpreadSheet();
	
	void update();
	void initialize(int x, int y);

	const int MAX_VALUE = 9;
	const int MIN_VALUE = 1;

private:
	int x_ = 0;
	int y_ = 0;

	HWND hWnd_ = nullptr;

	void drawSpreadSheet(int x, int y);
	void destoy();
};

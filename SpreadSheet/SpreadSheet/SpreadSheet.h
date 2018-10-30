#pragma once

#ifndef UNICODE
#define UNICODE
#endif // !UNICODE

#include "GraphicDefines.h"
#include <string>

class SpreadSheet {
public:

	SpreadSheet(HWND hWnd);

	~SpreadSheet();
	
	void update();
	void initialize(int rows, int columns);

	const int MAX_LINES = 9;
	const int MIN_LINES = 1;

private:
	int rows_ = 0;
	int columns_ = 0;

	HWND hWnd_ = nullptr; // associated HWND

	void draw(int rows, int columns);
	int * getTextHeights(int rows, int columns, int xStep, HDC wndDC, RECT clientRect, WCHAR ** strings);
	void paintTable(int rows, int columns, int xStep, int * ySteps, HDC wndDC, WCHAR ** strings);
	void destoy();
};

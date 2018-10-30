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
	
	const int MAX_LINES = 9;
	const int MIN_LINES = 1;

	void resize(LPARAM lParam);

	void hScroll(WPARAM wParam);

	void vScroll(WPARAM wParam);

	void update();

	void initialize(int rows, int columns, LPARAM lParam);

private:

	LONG charWidth_;
	LONG charHeight_;

	LONG minCellWidth_;
	LONG minCellHeight_;

	LONG minDisplayedHeight_;
	LONG minDisplayedWidth_;

	const LONG MIN_CHAR_IN_CELL_LINE = 2;
	const LONG MIN_LINES_IN_CELL = 1;
	
	int rows_ = 0;
	int columns_ = 0;

	bool isInitialized_ = false;

	HWND hWnd_ = nullptr; // associated HWND

	void draw(int rows, int columns);

	int * getTextHeights(int rows, int columns, int xStep, HDC wndDC, RECT clientRect, WCHAR ** strings);

	void paintTable(int rows, int columns, int xStep, int * ySteps, HDC wndDC, WCHAR ** strings);

	void destoy();
};

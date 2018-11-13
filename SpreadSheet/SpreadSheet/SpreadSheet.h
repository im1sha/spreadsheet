#pragma once

#ifndef UNICODE
#define UNICODE
#endif // !UNICODE

#include "GraphicDefines.h"
#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <math.h>

class SpreadSheet {
public:

	SpreadSheet(HWND hWnd);

	~SpreadSheet();
	
	static const int MAX_STRINGS = 81;
	const int MAX_LINES = 9;
	const int MIN_LINES = 1;

	void resize(LPARAM lParam);

	void hScroll(WPARAM wParam);
	void vScroll(WPARAM wParam);

	void update();
	void initialize(int rows, int columns, std::vector<std::wstring> strings, LPARAM lParam);
	
	bool areDimensionsSet();
	POINT getMinWidthAndHeight();

private:

	WCHAR** tableStrings_ = nullptr;
	//std::vector<std::vector<int> > charsInWords_;
	std::vector<std::vector<int> > wordsLenghts_;
	
	//LONG charWidth_ = 0;
	LONG charHeight_ = 0;

	LONG minColumnWidth_ = 0;

	LONG minDisplayedWidth_ = 320;
	LONG minDisplayedHeight_ = 240;

	const LONG MIN_CHAR_IN_CELL_LINE = 3;
	const LONG MIN_LINES_IN_CELL = 1;
	
	int rows_ = 0;
	int columns_ = 0;

	bool isInitialized_ = false;

	HWND hWnd_ = nullptr; // associated HWND
	HDC hDC_ = nullptr; // device context

	// font section
	const std::wstring DEFAULT_FONT = L"futuralightc";
	HGDIOBJ hPreviousFont_ = nullptr;
	HFONT hFont_ = nullptr;
	const int DEFAULT_FONT_SIZE = 20;

	HPEN hPen_ = nullptr;
	HPEN hOldPen_ = nullptr;
	int oldBackground_;

	int spaceWidth_;

	void draw(int rows, int columns);

	std::vector<int> getTextHeights(std::vector<std::vector<int> > lengths,  int lineWidth, int charHeight);

	void paintTable(int rows, int columns, int xStep, std::vector<int> ySteps, int totalWidth, std::vector<int> textHeights, HDC wndDC, WCHAR** strings);

	int getMaxLinesInRow(std::vector<std::vector<int>> lengths, int lineWidth);

	void freeTableContent();

	void processStrings(std::vector<std::wstring> strings);

	std::wstring deleteExtraDelimiters(const std::wstring s, wchar_t delimiter);

	std::vector<std::wstring> split(const std::wstring stringToSplit, wchar_t delimiter);

	WCHAR** toWcharArray(std::vector<std::wstring> strings);

};

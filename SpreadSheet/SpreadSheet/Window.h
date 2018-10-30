#pragma once

#ifndef UNICODE
#define UNICODE
#endif // !UNICODE

#include "GraphicDefines.h"
#include "SpreadSheet.h"
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <fstream>

class Window
{
public:
	Window(HINSTANCE hInstance, int nCmdShow);

	~Window();

	int messageLoop();

private:
	HWND hWnd_ = nullptr; // related HWND
	HINSTANCE hInstance_ = nullptr;

	// values which identify hWnd_ 
	const WCHAR DEFAULT_CLASS_NAME[20] = L"SpreadSheetClass";
	const WCHAR DEFAULT_WINDOW_NAME[15] = L"SpreadSheet";

	// windows placed on hWnd_
	HWND editRowshWnd_ = nullptr;
	HWND editColumnshWnd_ = nullptr;
	HWND buttonhWnd_ = nullptr;

	// child windows default positions 
	const RECT EDIT_ROWS_DEFAULT_POSITION = { 10, 10, 200, 20 };
	const RECT EDIT_COLUMNS_DEFAULT_POSITION = { 10, 50, 200, 20 };
	const RECT BUTTON_OK_DEFAULT_POSITION = { 10, 90, 50, 20 };

	const LONG DEFAULT_CHILD_STYLE = WS_BORDER | WS_CHILD | WS_VISIBLE;

	const int DEFAULT_BUTTON_NO = 1;
	const int MAX_TEXT_LENGTH = 20;

	std::vector<std::wstring> tableData_;

	SpreadSheet* spreadSheet_ = nullptr;

	HWND initialize(HINSTANCE hInstance, int nCmdShow);

	ATOM registerClass(WNDCLASSEX wndClassEx, HINSTANCE hInstance, WNDPROC wndProc);

	static LRESULT CALLBACK windowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	bool processShowSpreadsheetRequest(LPARAM lParam);

	bool processCreateChildItemsRequest();

	bool loadStringsFromFile();
};

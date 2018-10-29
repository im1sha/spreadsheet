#pragma once

#ifndef UNICODE
#define UNICODE
#endif // !UNICODE

#include "GraphicDefines.h"
#include "SpreadSheet.h"
#include <string>

class Window
{
public:
	Window(HINSTANCE hInstance, int nCmdShow);

	~Window();

	int messageLoop();

	const int DEFAULT_BUTTON_NO = 1;

private:
	HWND hWnd_ = nullptr;
	HWND textBoxXhWnd_ = nullptr;
	HWND textBoxYhWnd_ = nullptr;
	HWND buttonhWnd_ = nullptr;

	const int MAX_TEXT_LENGTH = 20;
	HINSTANCE hInstance_ = nullptr;

	SpreadSheet* spreadSheet_ = nullptr;

	const WCHAR defaultClassName_[20] = L"SpreadSheetClass";
	const WCHAR defaultWindowName_[15] = L"SpreadSheet";

	HWND initialize(HINSTANCE hInstance, int nCmdShow);

	ATOM registerClass(WNDCLASSEX wndClassEx, HINSTANCE hInstance, WNDPROC wndProc);

	static LRESULT CALLBACK windowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};

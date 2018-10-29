#pragma once

#include "SpreadSheet.h"

SpreadSheet::SpreadSheet(HWND hWnd, HINSTANCE hInstanse)
{
	this->hWnd_ = hWnd;
	//this->initialize(x_, y_);
}

SpreadSheet::~SpreadSheet()
{
	this->destoy();
}

void SpreadSheet::initialize(int x, int y)
{
	x_ = x;
	y_ = y;
	update();
}

void SpreadSheet::update()
{	
	if ((x_ != 0) && (y_ != 0))
	{		
		drawSpreadSheet(x_, y_);
	}
}

void SpreadSheet::destoy()
{

}

//{
//	x1 = x2 = LOWORD(lParam);
//	y1 = y2 = HIWORD(lParam);
//}

// dwStyle = (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);

void SpreadSheet::drawSpreadSheet(int x, int y)
{
	RECT clientRect;
	GetClientRect(hWnd_, &clientRect);
	HDC wndDC = GetDC(hWnd_);

	tagPAINTSTRUCT ps {
		ps.hdc = wndDC,
		ps.fErase = true,
		ps.rcPaint = clientRect
	};

	HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
	HPEN hOldPen = (HPEN)SelectObject(wndDC, hPen);
	

	BeginPaint(hWnd_, &ps);
	int oldBackgroung = SetBkMode(wndDC,TRANSPARENT);

	int xStep = (clientRect.right - clientRect.left) / x;
	int* textHeights = new int[y] { 0 };
	WCHAR str1[] = L"qwertyhrt qwertyhrt qwertyhrt "
		"qwertyhrt qwertyhrt qwertyhrt qwertyhrt qwertyhrt";
	WCHAR str2[] = L"ooo zzz yyy";


	RECT defaultCell = {
		0,
		0,
		xStep,
		(clientRect.right - clientRect.left) / x
	};
	for (size_t j = 0; j < y; j++)
	{
		for (size_t i = 0; i < x; i++)
		{			
			WCHAR* textToPrint = (j % 2 == 0) ? str1 : str2;
			RECT textBounds = defaultCell;
			int height = DrawText(wndDC, textToPrint, (int)wcslen(textToPrint), 
				&textBounds, DT_CALCRECT | DT_CENTER | DT_WORDBREAK);
			if (height > textHeights[j])
			{
				textHeights[j] = height;
			}
		}
	}

	int* ySteps = new int[y];
	int fullTextHeight = 0;
	for (size_t i = 0; i < y; i++)
	{
		fullTextHeight += textHeights[i];
	}

	for (size_t i = 0; i < y; i++)
	{
		ySteps[i] = textHeights[i] + (clientRect.bottom - clientRect.top - fullTextHeight) / y;
	}

	int currentBottom = 0;
	for (size_t j = 0; j < y; j++)
	{
		int yStep = ySteps[j];
		currentBottom += yStep;
		for (size_t i = 0; i < x; i++)
		{		
			WCHAR* textToPrint = (j % 2 == 0) ? str1 : str2;
						
			RECT cell = {
				(LONG)(xStep * i),
				(LONG)(currentBottom - ySteps[j]),
				(LONG)(xStep * (i + 1)),
				(LONG)(currentBottom)
			};

			RECT textBounds = cell;
			int height = DrawText(wndDC, textToPrint, (int)wcslen(textToPrint),
				&textBounds, DT_CALCRECT | DT_CENTER | DT_WORDBREAK);

			RECT textRect = cell;
			textRect.top = currentBottom - ((ySteps[j] + height) / 2);
						
			Rectangle(
				wndDC,
				cell.left,
				cell.top,			
				cell.right,
				cell.bottom
			);
			
			DrawText(wndDC, textToPrint, (int)wcslen(textToPrint),
				&textRect, DT_CENTER | DT_WORDBREAK);
		}
	}
	delete[] textHeights;

	SetBkMode(wndDC, oldBackgroung);
	EndPaint(hWnd_, &ps);

	DeleteObject(hPen);	
	SelectObject(wndDC, hOldPen);

	ReleaseDC(hWnd_, wndDC);
}


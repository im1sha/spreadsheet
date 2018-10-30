#pragma once

#include "SpreadSheet.h"

SpreadSheet::SpreadSheet(HWND hWnd)
{
	this->hWnd_ = hWnd;
}

SpreadSheet::~SpreadSheet()
{
	this->destoy();
}

void SpreadSheet::initialize(int rows, int columns)
{
	rows_ = rows;
	columns_ = columns;
}

void SpreadSheet::update()
{	
	if ((rows_ != 0) && (columns_ != 0))
	{		
		draw(rows_, columns_);
	}
}

void SpreadSheet::destoy()
{

}

void SpreadSheet::draw(int rows, int columns)
{
	// gathering client window's data
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
	int oldBackgroung = SetBkMode(wndDC, TRANSPARENT);

	// string processing
	WCHAR str1[] = L"qwertyhrt qwertyhrt qwertyhrt qwertyhrt qwertyhrt qwertyhrt qwertyhrt qwertyhrt";
	WCHAR str2[] = L"ooo zzz yyy";
	WCHAR** strings = (WCHAR **)calloc(2, sizeof(WCHAR *));
	strings[0] = str1;
	strings[1] = str2;

	// calculating draw parameters
	int xStep = (clientRect.right - clientRect.left) / columns;
	int* textHeights = getTextHeights(rows, columns, xStep, wndDC, clientRect, strings);
	int fullTextHeight = 0;
	for (size_t i = 0; i < rows; i++)
	{
		fullTextHeight += textHeights[i];		
	}	
	int* ySteps = new int[rows];
	for (size_t i = 0; i < rows; i++)
	{
		ySteps[i] = textHeights[i] + (clientRect.bottom - clientRect.top - fullTextHeight) / rows;
	}

	paintTable(rows, columns, xStep, ySteps, wndDC, strings);

	free(strings);
	delete[] textHeights;

	// restoring defaults
	SetBkMode(wndDC, oldBackgroung);
	EndPaint(hWnd_, &ps);
	DeleteObject(hPen);	
	SelectObject(wndDC, hOldPen);
	ReleaseDC(hWnd_, wndDC);
}

// Calculates max text height in row
int* SpreadSheet::getTextHeights(int rows, int columns, int xStep, HDC wndDC, RECT clientRect, WCHAR ** strings)
{
	int* textHeights = new int[rows] { };
	RECT defaultCell = {
		0,
		0,
		(clientRect.right - clientRect.left) / columns,
		xStep
	};
	for (size_t j = 0; j < rows; j++)
	{
		for (size_t i = 0; i < columns; i++)
		{
			WCHAR* textToPrint = (j % 2 == 0) ? strings[0] : strings[1];
			RECT textBounds = defaultCell;
			int height = DrawText(wndDC, textToPrint, (int)wcslen(textToPrint),
				&textBounds, DT_CALCRECT | DT_CENTER | DT_WORDBREAK);
			if (height > textHeights[j])
			{
				textHeights[j] = height;
			}
		}
	}
	return textHeights;
}

void SpreadSheet::paintTable(int rows, int columns, int xStep, int *ySteps, HDC wndDC, WCHAR ** strings)
{
	int currentBottom = 0;
	for (size_t j = 0; j < rows; j++)
	{
		int yStep = ySteps[j];
		currentBottom += yStep;
		for (size_t i = 0; i < columns; i++)
		{
			WCHAR* textToPrint = (j % 2 == 0) ? strings[0] : strings[1];

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
}
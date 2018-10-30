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

void SpreadSheet::destoy()
{

}

void SpreadSheet::initialize(int rows, int columns, LPARAM lParam)
{
	rows_ = rows;
	columns_ = columns;

	HDC hdc = GetDC(hWnd_);
	TEXTMETRIC tm;
	GetTextMetrics(hdc, &tm);

	charWidth_ = tm.tmAveCharWidth;
	charHeight_ = tm.tmHeight + tm.tmExternalLeading;
	minCellWidth_ = MIN_CHAR_IN_CELL_LINE * charWidth_;
	minCellHeight_ = MIN_LINES_IN_CELL * charHeight_;
	minDisplayedWidth_ = columns_ * minCellWidth_;
	minDisplayedHeight_ = rows_ * minCellHeight_;

	ReleaseDC(hWnd_, hdc);

	isInitialized_ = true;

	resize(lParam);
}

void SpreadSheet::update()
{	
	if (isInitialized_)
	{		
		draw(rows_, columns_);
	}
}

void SpreadSheet::resize(LPARAM lParam)
{
	if (isInitialized_)
	{
		SCROLLINFO si;
		WORD xClient = LOWORD(lParam);
		WORD yClient = HIWORD(lParam);

		// Set the scrolling ranges and page sizes
		si.cbSize = sizeof(si);
		si.fMask = SIF_RANGE | SIF_PAGE;
		si.nMin = 0;
		si.nMax = yClient / charHeight_;
		si.nPage = yClient / charHeight_;
		SetScrollInfo(hWnd_, SB_VERT, &si, TRUE);

		si.cbSize = sizeof(si);
		si.fMask = SIF_RANGE | SIF_PAGE;
		si.nMin = 0;
		si.nMax = xClient / charWidth_;
		si.nPage = xClient / charWidth_;
		SetScrollInfo(hWnd_, SB_HORZ, &si, TRUE);

		draw(rows_, columns_);
	}
}

// 
void SpreadSheet::draw(int rows, int columns)
{
	/*if ((clientRect.bottom - clientRect.top > minDisplayedHeight_) &&
		(clientRect.right - clientRect.left > minDisplayedWidth_)) {
	}*/

	// client window's data gathering
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


	//------------------------------------

	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_POS;
	GetScrollInfo(hWnd_, SB_VERT, &si);
	int yPos = si.nPos;
	// Get horizontal scroll bar position.
	GetScrollInfo(hWnd_, SB_HORZ, &si);
	int xPos = si.nPos;
	// Find painting limits.
	int FirstLine = max(0, yPos + ps.rcPaint.top / charHeight_);
	int LastLine = min(100, yPos + ps.rcPaint.bottom / charHeight_); // 100 - lines in table 

	//------------------------------------


	// string processing
	WCHAR str1[] = L"qwertyhrt qwertyhrt qwertyhrt qwertyhrt qwertyhrt qwertyhrt qwertyhrt qwertyhrt";
	WCHAR str2[] = L"ooo zzz yyy";
	WCHAR** strings = (WCHAR **)calloc(2, sizeof(WCHAR *));
	strings[0] = str1;
	strings[1] = str2;

	// draw parameters calculating
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

	// defaults restoring
	SetBkMode(wndDC, oldBackgroung);
	EndPaint(hWnd_, &ps);
	DeleteObject(hPen);	
	SelectObject(wndDC, hOldPen);
	ReleaseDC(hWnd_, wndDC);
}

void SpreadSheet::vScroll(WPARAM wParam)
{
	SCROLLINFO si;

	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL;
	GetScrollInfo(hWnd_, SB_VERT, &si);

	int yPos = si.nPos;
	switch (LOWORD(wParam))
	{
	// clicked the HOME keyboard key.
	case SB_TOP:
		si.nPos = si.nMin;
		break;
	// clicked the END keyboard key.
	case SB_BOTTOM:
		si.nPos = si.nMax;
		break;
	// clicked the top arrow.
	case SB_LINEUP:
		si.nPos -= 1;
		break;
	// clicked the bottom arrow.
	case SB_LINEDOWN:
		si.nPos += 1;
		break;
	// clicked the scroll bar shaft above the scroll box.
	case SB_PAGEUP:
		si.nPos -= si.nPage;
		break;
	// clicked the scroll bar shaft below the scroll box.
	case SB_PAGEDOWN:
		si.nPos += si.nPage;
		break;
	// dragged the scroll box.
	case SB_THUMBTRACK:
		si.nPos = si.nTrackPos;
		break;
	default:
		break;
	}

	si.fMask = SIF_POS;
	SetScrollInfo(hWnd_, SB_VERT, &si, TRUE);
	GetScrollInfo(hWnd_, SB_VERT, &si);

	if (si.nPos != yPos)
	{
		ScrollWindow(hWnd_, 0, charHeight_ * (yPos - si.nPos), NULL, NULL); 
		UpdateWindow(hWnd_);
	}

}

void SpreadSheet::hScroll(WPARAM wParam)
{
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL;

	GetScrollInfo(hWnd_, SB_HORZ, &si);
	int xPos = si.nPos;

	switch (LOWORD(wParam))
	{
	// clicked the left arrow.
	case SB_LINELEFT:
		si.nPos -= 1;
		break;
	// clicked the right arrow.
	case SB_LINERIGHT:
		si.nPos += 1;
		break;
	// clicked the scroll bar shaft left of the scroll box.
	case SB_PAGELEFT:
		si.nPos -= si.nPage;
		break;
	// clicked the scroll bar shaft right of the scroll box.
	case SB_PAGERIGHT:
		si.nPos += si.nPage;
		break;
	// dragged the scroll box.
	case SB_THUMBTRACK:
		si.nPos = si.nTrackPos;
		break;
	default:
		break;
	}

	si.fMask = SIF_POS;
	SetScrollInfo(hWnd_, SB_HORZ, &si, TRUE);
	GetScrollInfo(hWnd_, SB_HORZ, &si);

	// if the position has changed scroll the window.
	if (si.nPos != xPos)
	{
		ScrollWindow(hWnd_, charWidth_ * (xPos - si.nPos), 0, NULL, NULL); 
		UpdateWindow(hWnd_);
	}
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
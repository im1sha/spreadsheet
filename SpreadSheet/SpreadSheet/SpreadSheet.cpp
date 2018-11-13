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
	if (tableStrings_ != nullptr)
	{
		for (size_t i = 0; i < MAX_STRINGS; i++)
		{
			if (tableStrings_[i] != nullptr)
			{
				free(tableStrings_[i]);
			}
		}
		free(tableStrings_);
	}
}

void SpreadSheet::initialize(int rows, int columns, std::vector<std::wstring> strings, LPARAM lParam)
{
	rows_ = rows;
	columns_ = columns;

	HDC hdc = GetDC(hWnd_);
	TEXTMETRIC tm;
	GetTextMetrics(hdc, &tm);
	ReleaseDC(hWnd_, hdc);

	charWidth_ = ( tm.tmMaxCharWidth + tm.tmAveCharWidth) / 2;
	charHeight_ = tm.tmHeight + tm.tmExternalLeading;
	
	processStrings(strings);

	for (size_t i = 0; i < wordsLenghts_.size(); i++)
	{
		for (size_t j = 0; j < wordsLenghts_[i].size(); j++)
		{
			if (wordsLenghts_[i][j] > minColumnWidth_)
			{
				minColumnWidth_ = wordsLenghts_[i][j] + charWidth_;
			}
		}
	}

	isInitialized_ = true;

	//resize(lParam);
}

WCHAR** SpreadSheet::toWcharArray(std::vector<std::wstring> strings)
{
	if (strings.size() == 0)
	{
		return nullptr;
	}

	WCHAR ** result = (WCHAR **)calloc(SpreadSheet::MAX_STRINGS,
		sizeof(WCHAR *));

	size_t stringLenght;
	for (size_t i = 0; i < strings.size(); i++)
	{
		stringLenght = strings[i].size();
		result[i] = (WCHAR *)calloc(stringLenght + 1, sizeof(WCHAR));
		strings[i].copy(result[i], stringLenght, 0);
	}

	return result;
}

std::vector<std::wstring> SpreadSheet::split(const std::wstring stringToSplit, wchar_t delimiter)
{
	std::vector<std::wstring> elements;
	std::wstringstream ss(stringToSplit);
	std::wstring item;
	while (getline(ss, item, delimiter))
	{
		elements.push_back(item);
	}
	return elements;
}

std::wstring SpreadSheet::deleteExtraDelimiters(const std::wstring s, wchar_t delimiter)
{
	std::vector<std::wstring> elements;
	std::wstringstream ss(s);
	std::wstring item;
	while (getline(ss, item, delimiter))
	{
		if (item.size() != 0)
		{
			elements.push_back(item);
		}
	}
	std::wstring result = L"";

	if (elements.size() != 0)
	{
		for (size_t i = 0; i < elements.size() - 1; i++)
		{
			result += elements[i] + delimiter;
		}
		result += elements[elements.size() - 1];
	}

	return result;
}

bool SpreadSheet::areDimensionsSet()
{
	return isInitialized_;
}

POINT SpreadSheet::getMinWidthAndHeight()
{
	return { minDisplayedWidth_, minDisplayedHeight_ };
}

void SpreadSheet::processStrings(std::vector<std::wstring> strings)
{
	for (size_t i = 0; i < strings.size(); i++)
	{
		strings[i] = deleteExtraDelimiters(strings[i], ' ');
		std::vector<std::wstring> words = split(strings[i], ' ');

		std::vector<int> totalChars;
		std::vector<int> totalLenght;

		for (size_t i = 0; i < words.size(); i++)
		{
			int charsInWord = (int)words[i].size();
			totalChars.push_back(charsInWord);
			totalLenght.push_back(charsInWord * charWidth_);
		}

		charsInWords_.push_back(totalChars);
		wordsLenghts_.push_back(totalLenght);
	}
	tableStrings_ = toWcharArray(strings);
}

void SpreadSheet::update()
{	
	if (isInitialized_)
	{		
		draw(rows_, columns_);
	}
}

// ~
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
	// client window's data gathering
	RECT clientRect;
	GetClientRect(hWnd_, &clientRect);
	HDC wndDC = GetDC(hWnd_);
	tagPAINTSTRUCT ps {
		ps.hdc = wndDC,
		ps.fErase = true,
		ps.rcPaint = clientRect
	};
	HPEN hPen_ = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
	HPEN hOldPen = (HPEN)SelectObject(wndDC, hPen_);
	BeginPaint(hWnd_, &ps);
	int oldBackgroung = SetBkMode(wndDC, TRANSPARENT);


	//------------------------------------

	//SCROLLINFO si;
	//si.cbSize = sizeof(si);
	//si.fMask = SIF_POS;
	//GetScrollInfo(hWnd_, SB_VERT, &si);
	//int yPos = si.nPos;
	//// Get horizontal scroll bar position.
	//GetScrollInfo(hWnd_, SB_HORZ, &si);
	//int xPos = si.nPos;
	//// Find painting limits.
	//int FirstLine = max(0, yPos + ps.rcPaint.top / charHeight_);
	//int LastLine = min(100, yPos + ps.rcPaint.bottom / charHeight_); // 100 - lines in table 

	//------------------------------------


	// draw parameters calculating
	int averageColumnWidth = (clientRect.right - clientRect.left) / columns;
	int xStep = (averageColumnWidth < minColumnWidth_) ? minColumnWidth_ : averageColumnWidth;

	
	std::vector<int> textHeights = getTextHeights(wordsLenghts_, xStep, charHeight_);
	

	int fullTextHeight = 0;
	for (size_t i = 0; i < rows; i++)
	{
		fullTextHeight += textHeights[i];		
	}	

	std::vector<int> ySteps(rows);

	if (clientRect.bottom - clientRect.top > fullTextHeight)
	{
		for (size_t i = 0; i < rows; i++)
		{
			ySteps[i] = textHeights[i] + (clientRect.bottom - clientRect.top - fullTextHeight) / rows;
		}
		ySteps[ySteps.size() - 1] += (clientRect.bottom - clientRect.top - fullTextHeight) % rows;
	}
	else 
	{
		ySteps = textHeights;
	}

	paintTable(rows, columns, xStep, ySteps, clientRect.right - clientRect.left, textHeights, wndDC, tableStrings_);

	// defaults restoring
	SetBkMode(wndDC, oldBackgroung);
	EndPaint(hWnd_, &ps);
	DeleteObject(hPen_);	
	SelectObject(wndDC, hOldPen);
	ReleaseDC(hWnd_, wndDC);
}


// ~
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

// ~
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
std::vector<int> SpreadSheet::getTextHeights(std::vector<std::vector<int> > lengths, int lineWidth, int lineHeight)
{
	std::vector<int> textHeights;
	if (lengths.size() != 0)
	{
		for (size_t i = 0; i < rows_; i++)
		{
			std::vector<std::vector<int> >::const_iterator start = wordsLenghts_.begin() + i * columns_;
			std::vector<std::vector<int> >::const_iterator end = wordsLenghts_.begin() + (i + 1) * columns_;
			std::vector<std::vector<int> > rowValues(start, end);
			textHeights.push_back(getMaxLinesInRow(rowValues, lineWidth));
		}
		for (size_t i = 0; i < textHeights.size(); i++)
		{
			textHeights[i] *= lineHeight;
		}
	}
	
	return textHeights;
}

//
void SpreadSheet::paintTable(int rows, int columns, int xStep, std::vector<int> ySteps, int totalWidth, std::vector<int> textHeights, HDC wndDC, WCHAR** strings)
{
	int currentBottom = 0;
	
	for (size_t j = 0; j < rows; j++)
	{
		int yStep = ySteps[j];
		currentBottom +=  yStep;
		for (size_t i = 0; i < columns; i++)
		{
			WCHAR* textToPrint = strings[j * columns + i];

			RECT cell = {
				(LONG)(xStep * i),
				(LONG)(currentBottom - ySteps[j]),
				(LONG)(xStep * (i + 1)),
				(LONG)(currentBottom)
			};
	
			RECT textRect = cell;
			textRect.top = currentBottom - ((ySteps[j] + textHeights[j]) / 2);

			Rectangle(
				wndDC,
				cell.left,
				cell.top,
				cell.right + 1,
				cell.bottom + 1
			);

			DrawText(wndDC, textToPrint, (int)wcslen(textToPrint),
				&textRect, DT_CENTER | DT_WORDBREAK | DT_EDITCONTROL);
		}
	}

	// fill area to the right of the table
	Rectangle(
		wndDC,
		(LONG)(xStep * columns),
		(LONG) 0,
		(LONG)(xStep * columns + totalWidth % xStep + 1),
		(LONG)currentBottom + 1
	);
}

int SpreadSheet::getMaxLinesInRow(std::vector<std::vector<int> > lengths, int lineWidth)
{
	int maxLines = 1;
	for (size_t i = 0; i < lengths.size(); i++)
	{
		int currentLines = 1;
		int filledNow = 0;
		for (size_t j = 0; j < lengths[i].size(); j++)
		{
			filledNow += charWidth_ + lengths[i][j]; // space + word 
			if (filledNow > lineWidth)
			{
				currentLines += 1;
				filledNow = 0;
			}
		}

		if (currentLines > maxLines)
		{
			maxLines = currentLines;
		}
	}

	return maxLines;
}



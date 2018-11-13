#pragma once

#include "SpreadSheet.h"

SpreadSheet::SpreadSheet(HWND hWnd)
{
	this->hWnd_ = hWnd;
}

SpreadSheet::~SpreadSheet()
{
	this->deinitialize();
}

void SpreadSheet::deinitialize()
{
	// restoring previous graphical settings
	HDC wndDC = ::GetDC(hWnd_);

	if (hDC_ != nullptr && hPreviousFont_ != nullptr)
	{
		::SelectObject(hDC_, hPreviousFont_);
		if (hFont_ != nullptr)
		{
			::DeleteObject(hFont_);
		}
	}
	
	::SetBkMode(wndDC, oldBackground_);

	if (hOldPen_ != nullptr && wndDC != nullptr)
	{
		::SelectObject(wndDC, hOldPen_);
		if (hPen_ != nullptr)
		{
			::DeleteObject(hPen_);
		}
	}
	
	::ReleaseDC(hWnd_, wndDC);

	this->freeWcharStringArray(tableStrings_, (size_t) columns_ * rows_);
}

void SpreadSheet::initialize(int rows, int columns, std::vector<std::wstring> strings, LPARAM lParam)
{
	rows_ = rows;
	columns_ = columns;

	HDC winDC = ::GetDC(hWnd_);
	hPen_ = ::CreatePen(PS_SOLID, 1, RGB(110, 80, 190));
	hOldPen_ = (HPEN) ::SelectObject(winDC, hPen_);
	oldBackground_ = ::SetBkMode(winDC, TRANSPARENT);

	// set font
	hFont_ = ::CreateFont(DEFAULT_FONT_SIZE, NULL, NULL, NULL,
		FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_TT_ONLY_PRECIS,
		CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY,
		DEFAULT_PITCH | FF_SWISS, DEFAULT_FONT.c_str());

	hPreviousFont_ = ::SelectObject(winDC, hFont_);

	TEXTMETRIC tm = {};
	::GetTextMetrics(winDC, &tm);

	//charWidth_ = tm.tmAveCharWidth + sqrt(tm.tmMaxCharWidth);
	//charHeight_ = tm.tmHeight + tm.tmExternalLeading;

	tagSIZE spaceSize = {};
	GetTextExtentPoint32(winDC, L" ", 1, &spaceSize);
	spaceWidth_ = spaceSize.cx;

	::ReleaseDC(hWnd_, winDC);

	this->processStrings(strings); 

	// find minColumnWidth_ by length of word with max length
	for (size_t i = 0; i < wordsLenghts_.size(); i++)
	{
		for (size_t j = 0; j < wordsLenghts_[i].size(); j++)
		{
			if (wordsLenghts_[i][j] + spaceWidth_ > minColumnWidth_)
			{
				minColumnWidth_ = wordsLenghts_[i][j] + spaceWidth_;
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

	WCHAR ** result = (WCHAR **)::calloc(SpreadSheet::MAX_STRINGS,
		sizeof(WCHAR *));

	size_t stringLenght;
	for (size_t i = 0; i < strings.size(); i++)
	{
		stringLenght = strings[i].size();
		result[i] = (WCHAR *)::calloc(stringLenght + 1, sizeof(WCHAR));
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
	return { minColumnWidth_ * columns_, firstRowHeight_ };
}

void SpreadSheet::freeWcharStringArray(wchar_t** arrayToFree, size_t length) 
{
	if (arrayToFree != nullptr)
	{
		for (size_t j = 0; j < length; j++)
		{
			if (arrayToFree[j] != nullptr)
			{
				::free(arrayToFree[j]);
			}
		}
		::free(arrayToFree);
	}
}

// initializes wordsLenghts_ & tableStrings_ & charHeight_
void SpreadSheet::processStrings(std::vector<std::wstring> strings)
{
	HDC wndDC = ::GetDC(hWnd_);

	for (size_t i = 0; i < strings.size(); i++)
	{
		strings[i] = this->deleteExtraDelimiters(strings[i], ' ');
		std::vector<std::wstring> words = this->split(strings[i], ' ');
		wchar_t** wcharWords = this->toWcharArray(words);

		std::vector<int> cellLenghts;
		tagSIZE stringSize = {};
		
		for (size_t j = 0; j < words.size(); j++)
		{		
			::GetTextExtentPoint32(wndDC, wcharWords[j], (int) words[j].size(), &stringSize); // get word length 
			cellLenghts.push_back(stringSize.cx);
		}

		freeWcharStringArray(wcharWords, words.size());

		wordsLenghts_.push_back(cellLenghts);
	}
	tableStrings_ = this->toWcharArray(strings);

	tagSIZE stringSize = {};
	for (size_t i = 0; i < strings.size(); i++)
	{
		::GetTextExtentPoint32(wndDC, tableStrings_[i], (int)strings[i].size(), &stringSize); // get word length 
		if (stringSize.cy > lineHeight_)
		{
			lineHeight_ = stringSize.cy;
		}
	}

	::ReleaseDC(hWnd_, wndDC);
}

void SpreadSheet::update()
{	
	if (isInitialized_)
	{		
		this->draw(rows_, columns_);
	}
}

// draws table and fill it with content
void SpreadSheet::draw(int rows, int columns)
{
	// client window's data gathering
	HDC wndDC = ::GetDC(hWnd_);

	RECT clientRect;
	::GetClientRect(hWnd_, &clientRect);

	tagPAINTSTRUCT ps{
		ps.hdc = wndDC,
		ps.fErase = true,
		ps.rcPaint = clientRect
	};

	::BeginPaint(hWnd_, &ps);
	::SelectObject(wndDC, hPen_);
	::SelectObject(wndDC, hFont_);
	::SetBkMode(wndDC, TRANSPARENT);



	// draw parameters calculating
	int averageColumnWidth = (clientRect.right - clientRect.left) / columns;
	int xStep = (averageColumnWidth < minColumnWidth_) ? minColumnWidth_ : averageColumnWidth;

	std::vector<int> textHeights = this->getTextHeights(wordsLenghts_, xStep, lineHeight_);
	firstRowHeight_ = textHeights[0];
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

	this->paintTable(rows, columns, xStep, ySteps, clientRect.right - clientRect.left, textHeights, tableStrings_, wndDC);

	// draw ending
	::EndPaint(hWnd_, &ps);
	::ReleaseDC(hWnd_, wndDC);
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
void SpreadSheet::paintTable(int rows, int columns, int xStep, std::vector<int> ySteps, int totalWidth, std::vector<int> textHeights, WCHAR** strings, HDC wndDC)
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

			::DrawText(wndDC, textToPrint, (int) wcslen(textToPrint),
				&textRect, DT_CENTER | DT_WORDBREAK | DT_EDITCONTROL);
		}
	}

	// fill area to the right of the table
	::Rectangle(
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
			filledNow += spaceWidth_ + lengths[i][j]; // space + word length
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



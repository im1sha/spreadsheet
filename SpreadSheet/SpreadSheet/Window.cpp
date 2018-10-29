#include "Window.h"

Window::Window(HINSTANCE hInstance, int nCmdShow)
{
	this->hInstance_ = hInstance;
	HWND hWnd = initialize(hInstance, nCmdShow);
	this->hWnd_ = hWnd;
	this->spreadSheet_ = new SpreadSheet(hWnd, hInstance);
}

Window::~Window()
{
	if (spreadSheet_ != nullptr)
	{
		delete spreadSheet_;
	}
}

int Window::messageLoop()
{
	MSG msg;
	BOOL result;

	while (result = GetMessage(&msg, nullptr, 0, 0))
	{
		if (-1 == result)
		{
			// HANDLE THIS ERROR
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

HWND Window::initialize(HINSTANCE hInstance, int nCmdShow)
{
	WNDCLASSEX wndClassEx = { };
	registerClass(wndClassEx, hInstance, Window::windowProc);
	HWND hWnd = CreateWindow(defaultClassName_, defaultWindowName_, WS_OVERLAPPEDWINDOW | WS_VSCROLL,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, hInstance, 0);

	SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)this);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	return hWnd;
}

ATOM Window::registerClass(WNDCLASSEX wndClassEx, HINSTANCE hInstance, WNDPROC wndProc)
{
	wndClassEx.cbSize = sizeof(WNDCLASSEX);
	wndClassEx.style = CS_DBLCLKS;
	wndClassEx.lpfnWndProc = wndProc;
	wndClassEx.cbClsExtra = 0;
	wndClassEx.cbWndExtra = 0;
	wndClassEx.hInstance = hInstance;
	wndClassEx.hIcon = nullptr;
	wndClassEx.hCursor = LoadCursor(0, IDC_ARROW);
	wndClassEx.hbrBackground = CreateSolidBrush(DEFAULT_BACKGROUND_COLOR);
	wndClassEx.lpszMenuName = 0;
	wndClassEx.lpszClassName = defaultClassName_;
	wndClassEx.hIconSm = nullptr;
	return RegisterClassEx(&wndClassEx);
}

LRESULT CALLBACK Window::windowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{	
	Window* window = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
	SpreadSheet* s = nullptr;
	if (window != nullptr)
	{
		s = window->spreadSheet_;
	}
	
	switch (message)
	{
	case WM_LBUTTONDOWN:
	{		
		if (window == nullptr)
		{
			break;
		}
		if (window->textBoxXhWnd_ == nullptr &&
			window->textBoxYhWnd_ == nullptr && 
			window->buttonhWnd_ == nullptr
			)
		{
			window->textBoxXhWnd_ =
				CreateWindow(L"EDIT", L"",
					WS_BORDER | WS_CHILD | WS_VISIBLE,
					10, 10, 200, 20,
					hWnd, nullptr, nullptr, nullptr
				);
			window->textBoxYhWnd_ =
				CreateWindow(L"EDIT", L"",
					WS_BORDER | WS_CHILD | WS_VISIBLE,
					10, 50, 200, 20,
					hWnd, nullptr, nullptr, nullptr
				);
			window->buttonhWnd_ =
				CreateWindow(L"BUTTON", L"OK",
					WS_BORDER | WS_CHILD | WS_VISIBLE,
					10, 90, 50, 20,
					hWnd, (HMENU)window->DEFAULT_BUTTON_NO, nullptr, nullptr
				);
		}
			
		break;
	}
	case WM_COMMAND:
	{
		if (window == nullptr)
		{
			break;
		}
		if (LOWORD(wParam) == window->DEFAULT_BUTTON_NO)
		{
			WCHAR* inputX = (WCHAR *)calloc(window->MAX_TEXT_LENGTH, sizeof(WCHAR));
			WCHAR* inputY = (WCHAR *)calloc(window->MAX_TEXT_LENGTH, sizeof(WCHAR));

			GetWindowText(window->textBoxXhWnd_, 
				inputX, window->MAX_TEXT_LENGTH);
			GetWindowText(window->textBoxYhWnd_,
				inputY, window->MAX_TEXT_LENGTH);
	
			int xValue = _wtoi(inputX);
			int yValue = _wtoi(inputY);
			if (s != nullptr)
			{
				if (xValue < s->MIN_VALUE || xValue > s->MAX_VALUE ||
					yValue < s->MIN_VALUE || yValue > s->MAX_VALUE)
				{
					MessageBox(hWnd, L"incorrect values", L"Attention", MB_OK);
				}
				else
				{
					s->initialize(xValue, yValue);

					DestroyWindow(window->textBoxXhWnd_);
					DestroyWindow(window->textBoxYhWnd_);
					DestroyWindow(window->buttonhWnd_);

					PostMessage(window->hWnd_, WM_LOAD_SPREADSHEET, NULL, NULL);
				}
			}
		}
		break;
	}
	case WM_LOAD_SPREADSHEET:
	{
		if (s != nullptr)
		{
			s->update();
		}
		break;
	}
	case WM_SIZE:
	case WM_PAINT: 
	{	
		if (s != nullptr)
		{
			s->update();
		}
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	case WM_CLOSE:
	{
		PostQuitMessage(0);
		break;
	}
	default:
	{
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	}
	return 0;
}


//LRESULT CALLBACK Window::windowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
//{
//	HDC hDC;
//	PAINTSTRUCT ps;
//	TEXTMETRIC  tm;
//
//	static std::wstring text;
//	std::wstring symb;
//
//	static int cxChar, cyChar, cxClient, cyClient;
//	static int nCharPerLine, nClientLines;
//	static int xCaret = 0, yCaret = 0;
//	int curIndex;
//	int nLines;    // число "полных" строк текста
//	int nTailChar; // число символов в последней строке
//	int x, y, i;
//
//	switch (message) {
//
//	case WM_CREATE:
//		hDC = GetDC(hWnd);
//		SelectObject(hDC, GetStockObject(SYSTEM_FIXED_FONT));
//		GetTextMetrics(hDC, &tm);
//		cxChar = tm.tmAveCharWidth;
//		cyChar = tm.tmHeight;
//		ReleaseDC(hWnd, hDC);
//		break;
//
//	case WM_SIZE:
//		// получить размеры окна в пиксел€х
//		cxClient = LOWORD(lParam);
//		cyClient = HIWORD(lParam);
//		// вычислить размеры окна в символах (по 'x') и строках (по 'y')
//		nCharPerLine = max(1, cxClient / cxChar);
//		nClientLines = max(1, cyClient / cyChar);
//
//		if (hWnd == GetFocus())
//			SetCaretPos(xCaret * cxChar, yCaret * cyChar);
//		break;
//
//	case WM_SETFOCUS:
//		// создать и показать каретку
//		CreateCaret(hWnd, NULL, 0, cyChar);               // 1
//		SetCaretPos(xCaret * cxChar, yCaret * cyChar);
//		ShowCaret(hWnd);
//		break;
//
//	case WM_KILLFOCUS:
//		// спр€тать и уничтожить каретку
//		HideCaret(hWnd);
//		DestroyCaret();
//		break;
//
//	case WM_KEYDOWN:
//		nLines = text.size() / nCharPerLine;
//		nTailChar = text.size() % nCharPerLine;
//
//		switch (wParam) {
//		case VK_HOME:   xCaret = 0;
//			break;
//		case VK_END:    if (yCaret == nLines)
//			xCaret = nTailChar;
//						else
//			xCaret = nCharPerLine - 1;
//			break;
//		case VK_PRIOR:  yCaret = 0;
//			break;
//		case VK_NEXT:   yCaret = nLines;
//			xCaret = nTailChar;
//			break;
//		case VK_LEFT:   xCaret = max(xCaret - 1, 0);
//			break;
//		case VK_RIGHT:  xCaret = min(xCaret + 1, nCharPerLine - 1);
//			if ((yCaret == nLines) && (xCaret > nTailChar))
//				xCaret = nTailChar;
//			break;
//		case VK_UP:     yCaret = max(yCaret - 1, 0);
//			break;
//		case VK_DOWN:   yCaret = min(yCaret + 1, nLines);
//			if ((yCaret == nLines) && (xCaret > nTailChar))
//				xCaret = nTailChar;
//			break;
//		case VK_DELETE: curIndex = yCaret * nCharPerLine + xCaret;
//			if (curIndex < text.size()) {
//				text.erase(curIndex, 1);
//				InvalidateRect(hWnd, NULL, TRUE);
//			}
//			break;
//		}
//
//		SetCaretPos(xCaret * cxChar, yCaret * cyChar);
//		break;
//
//	case WM_CHAR:
//		switch (wParam) {
//		case '\b':              // символ 'backspace'
//			if (xCaret > 0) {
//				xCaret--;
//				SendMessage(hWnd, WM_KEYDOWN, VK_DELETE, 1);
//			}
//			break;
//
//		case '\t':              // символ 'tab'
//			do { SendMessage(hWnd, WM_CHAR, ' ', 1L); } while (xCaret % 8 != 0);
//			break;
//
//		case '\r': case '\n':   // возврат каретки или перевод строки
//
//			for (i = 0; i < nCharPerLine - xCaret; ++i)
//				text += ' ';
//			xCaret = 0;
//			if (++yCaret == nClientLines) {
//				MessageBox(hWnd, L"Ќет места в окне", L"Error", MB_OK);
//				yCaret--;
//			}
//			break;
//
//		default:                // любой другой символ
//
//			curIndex = yCaret * nCharPerLine + xCaret;
//			if (curIndex == text.size())
//				text += (char)wParam;
//			else {
//				symb = (char)wParam;
//				text.insert(curIndex, symb);
//			}
//
//			InvalidateRect(hWnd, NULL, TRUE);
//
//			if (++xCaret == nCharPerLine) {
//				xCaret = 0;
//				if (++yCaret == nClientLines) {
//					MessageBox(hWnd, L"Ќет места в окне", L"Error", MB_OK);
//					yCaret--;
//				}
//			}
//			break;
//		}
//
//		SetCaretPos(xCaret * cxChar, yCaret * cyChar);
//		break;
//
//	case WM_PAINT:
//		hDC = BeginPaint(hWnd, &ps);
//		SelectObject(hDC, GetStockObject(SYSTEM_FIXED_FONT));
//
//		if (text.size()) {
//			nLines = text.size() / nCharPerLine;
//			nTailChar = text.size() % nCharPerLine;
//			for (y = 0; y < nLines; ++y)
//				TextOut(hDC, 0, y*cyChar, text.c_str() + y * nCharPerLine, nCharPerLine);
//			TextOut(hDC, 0, y*cyChar, text.c_str() + y * nCharPerLine, nTailChar);
//		}
//
//		EndPaint(hWnd, &ps);
//		break;
//
//	case WM_DESTROY:
//		PostQuitMessage(0);
//		break;
//
//	default:
//		return DefWindowProc(hWnd, message, wParam, lParam);
//	}
//	return 0;
//}

#include "Window.h"

Window::Window(HINSTANCE hInstance, int nCmdShow)
{
	this->hInstance_ = hInstance;
	HWND hWnd = initialize(hInstance, nCmdShow);
	this->hWnd_ = hWnd;
	this->spreadSheet_ = new SpreadSheet(hWnd);
}

Window::~Window()
{
	// destroy child windows if they still exist
	if (editRowshWnd_ != nullptr)
	{
		DestroyWindow(editRowshWnd_);
	}
	if (editColumnshWnd_ != nullptr)
	{
		DestroyWindow(editColumnshWnd_);
	}
	if (buttonhWnd_ != nullptr)
	{
		DestroyWindow(buttonhWnd_);
	}	

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
			// TODO: HANDLE THIS ERROR
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
	HWND hWnd = CreateWindow(DEFAULT_CLASS_NAME, DEFAULT_WINDOW_NAME, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, hInstance, 0);

	// save a reference to the current Window instance 
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
	wndClassEx.lpszClassName = DEFAULT_CLASS_NAME;
	wndClassEx.hIconSm = nullptr;
	return RegisterClassEx(&wndClassEx);
}

LRESULT CALLBACK Window::windowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{	
	// retrieve a reference to the related Window class
	Window* window = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

	SpreadSheet* s = nullptr;
	if (window != nullptr)
	{
		s = window->spreadSheet_;
	}
	
	switch (message)
	{
	case WM_CREATE:
	{
		DefWindowProc(hWnd, message, wParam, lParam);
		PostMessage(hWnd, WM_CREATE_CHILD_ITEMS, 0, 0);
		break;
	}
	case WM_CREATE_CHILD_ITEMS:
	{		
		if (window != nullptr)
		{
			window->processCreateChildItemsRequest();
		}
		break;
	}
	case WM_COMMAND:
	{
		if (window != nullptr)
		{
			if (LOWORD(wParam) == window->DEFAULT_BUTTON_NO)
			{
				window->processShowSpreadsheetRequest();
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

bool Window::processShowSpreadsheetRequest() 
{
	bool result = false;

	SpreadSheet* s = this->spreadSheet_;
	HWND hWnd = this->hWnd_;

	WCHAR* rowsInput = (WCHAR*)calloc(this->MAX_TEXT_LENGTH, sizeof(WCHAR));
	WCHAR* columnsInput = (WCHAR*)calloc(this->MAX_TEXT_LENGTH, sizeof(WCHAR));

	// get input from window
	GetWindowText(this->editRowshWnd_, rowsInput, this->MAX_TEXT_LENGTH);
	GetWindowText(this->editColumnshWnd_, columnsInput, this->MAX_TEXT_LENGTH);

	int rows = _wtoi(rowsInput);
	int columns = _wtoi(columnsInput);
	if (s != nullptr)
	{
		bool wrongInput = (rows < s->MIN_LINES) || (rows > s->MAX_LINES) ||
			(columns < s->MIN_LINES) || (columns > s->MAX_LINES);
		if (wrongInput)
		{
			MessageBox(hWnd, L"Incorrect values", L"Attention", MB_OK);
			
		}
		else // create table if input is value
		{
			s->initialize(rows, columns);

			DestroyWindow(this->editRowshWnd_);
			DestroyWindow(this->editColumnshWnd_);
			DestroyWindow(this->buttonhWnd_);

			PostMessage(this->hWnd_, WM_LOAD_SPREADSHEET, 0, 0);
			result = true;
		}
	}

	return result;
}

bool Window::processCreateChildItemsRequest() 
{	
	bool result = 
		(this->editRowshWnd_ == nullptr) &&
		(this->editColumnshWnd_ == nullptr) &&
		(this->buttonhWnd_ == nullptr);

	if (result)
	{
		// creating child windows 
		this->editRowshWnd_ =
			CreateWindow(L"EDIT", L"",
				this->DEFAULT_CHILD_STYLE,
				this->EDIT_ROWS_DEFAULT_POSITION.left,
				this->EDIT_ROWS_DEFAULT_POSITION.top,
				this->EDIT_ROWS_DEFAULT_POSITION.right,
				this->EDIT_ROWS_DEFAULT_POSITION.bottom,
				this->hWnd_, nullptr, nullptr, nullptr
			);
		this->editColumnshWnd_ =
			CreateWindow(L"EDIT", L"",
				this->DEFAULT_CHILD_STYLE,
				this->EDIT_COLUMNS_DEFAULT_POSITION.left,
				this->EDIT_COLUMNS_DEFAULT_POSITION.top,
				this->EDIT_COLUMNS_DEFAULT_POSITION.right,
				this->EDIT_COLUMNS_DEFAULT_POSITION.bottom,
				this->hWnd_, nullptr, nullptr, nullptr
			);
		this->buttonhWnd_ =
			CreateWindow(L"BUTTON", L"OK",
				this->DEFAULT_CHILD_STYLE,
				this->BUTTON_OK_DEFAULT_POSITION.left,
				this->BUTTON_OK_DEFAULT_POSITION.top,
				this->BUTTON_OK_DEFAULT_POSITION.right,
				this->BUTTON_OK_DEFAULT_POSITION.bottom,
				this->hWnd_, (HMENU)(this->DEFAULT_BUTTON_NO), nullptr, nullptr
			);
	}

	return result;
}
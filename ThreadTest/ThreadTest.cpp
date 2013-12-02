// ThreadTest.cpp : Defines the entry point for the application.
//

#include "stdafx.h"

#include <memory>				// for std::shared_ptr
#include <map>					// for std::map
//#include <typeinfo>			// for typeid
//#include <algorithm>			// for std::for_each

#include "BaseLibrary.h"
#include "ThreadTest.h"

#define WM_WORKDONE WM_USER + 0

//

class Win32Controller : public Controller
{
public:
	Win32Controller(HWND hDlg_) : hDlg(hDlg_) 
	{
		conversion[START] = IDC_START;
		conversion[STOP] = IDC_STOP;
		conversion[QUIT] = IDC_QUIT;
	}
	virtual void UpdateControls()
	{
		std::for_each(std::begin(controls), std::end(controls), [&](std::pair<int, bool> control){ EnableWindow(GetDlgItem(hDlg, conversion[control.first]), control.second); });
	}
	virtual void OnFinish(Work * work)
	{
		PostMessage(hDlg, WM_WORKDONE, (WPARAM)typeid(*work).hash_code(), 0);
	}

	HWND hDlg;
	std::map<int, int> conversion;
};

//

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM /*lParam*/)
{
	static Win32Controller *s_win32Controller;

	switch (message)
	{
	case WM_INITDIALOG:
		{
			s_win32Controller = new Win32Controller(hDlg);
			
			s_win32Controller->EnableControl(IDC_START, false);
			s_win32Controller->EnableControl(IDC_STOP, false);
			s_win32Controller->EnableControl(IDC_QUIT, true);
			s_win32Controller->UpdateControls();

			Work *work = new InitWork(s_win32Controller);

			s_win32Controller->QueueWork(std::make_shared<InitWork>(InitWork(s_win32Controller)));
			return (INT_PTR)TRUE;
		}

	case WM_CLOSE:
		{
			s_win32Controller->QueueWork(std::make_shared<QuitWork>(QuitWork(s_win32Controller)));
			return (INT_PTR)TRUE;
		}

	case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case IDC_START:
				s_win32Controller->QueueWork(std::make_shared<StartWork>(StartWork(s_win32Controller)));
				break;
			case IDC_STOP:
				s_win32Controller->QueueWork(std::make_shared<StopWork>(StopWork(s_win32Controller)));
				break;
			case IDC_QUIT:
				s_win32Controller->QueueWork(std::make_shared<QuitWork>(QuitWork(s_win32Controller)));
				break;
			}
			return (INT_PTR)TRUE;
		}

	case WM_WORKDONE:
		{
			s_win32Controller->UpdateControls();
			if ((size_t)wParam == typeid(QuitWork).hash_code())
			{
				s_win32Controller->SetDone();
				EndDialog(hDlg, 0);
				PostQuitMessage(0);
				delete s_win32Controller;
			}
			return (INT_PTR)TRUE;
		}
	}
	return (INT_PTR)FALSE;
}

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR /*lpCmdLine*/, int /*nCmdShow*/)
{
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), NULL, About);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, NULL, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}

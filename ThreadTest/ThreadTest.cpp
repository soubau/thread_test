// ThreadTest.cpp : Defines the entry point for the application.
//

#include "stdafx.h"

#include <thread>				// for std::thread
#include <condition_variable>	// for std::condition_variable
#include <memory>				// for std::shared_ptr
#include <queue>				// for std::queue
#include <map>					// for std::map
//#include <typeinfo>			// for typeid
//#include <mutex>				// for std::mutex
//#include <algorithm>			// for std::for_each
//#include <functional>			// for std::function

#include "BaseLibrary.h"
#include "ThreadTest.h"

#define WM_WORKDONE WM_USER + 0

//

class Win32Controller : public Controller
{
public:
	Win32Controller(HWND hDlg_) : hDlg(hDlg_) {}

	void QueueWork(std::shared_ptr<Work> work, int id = 0)
	{
		work->OnQueued();
		Controller::QueueWork(work);
	}
	void EnableControl(int id, bool enabled) 
	{
		controls[id] = enabled;
	}
	void UpdateControls()
	{
		std::for_each(std::begin(controls), std::end(controls), [&](std::pair<int, bool> control){ EnableWindow(GetDlgItem(hDlg, control.first), control.second); });
	}

	HWND hDlg;
	std::map<int, bool> controls;
};

class Win32Work
{
public:
	Win32Work(Win32Controller * win32Controller_) : win32Controller(win32Controller_) {}
	virtual void OnFinish()
	{
		PostMessage(win32Controller->hDlg, WM_WORKDONE, (WPARAM)typeid(*this).hash_code(), 0);
	}
	
	Win32Controller *win32Controller;
};

class Win32InitWork : public InitWork, public Win32Work
{
public:
	Win32InitWork(Win32Controller * win32Controller_) : Win32Work(win32Controller_) {}
	virtual void OnQueued()
	{
		win32Controller->EnableControl(IDC_START, false);
		win32Controller->EnableControl(IDC_STOP, false);
		win32Controller->UpdateControls();
	}
	virtual void OnFinish()
	{
		win32Controller->EnableControl(IDC_START, true);
		Win32Work::OnFinish();
		InitWork::OnFinish();
	}
};

class Win32StartWork : public StartWork, public Win32Work
{
public:
	Win32StartWork(Win32Controller * win32Controller_) : Win32Work(win32Controller_) {}
	virtual void OnQueued()
	{
		win32Controller->EnableControl(IDC_START, false);
		win32Controller->UpdateControls();
	}
	virtual void OnFinish()
	{
		win32Controller->EnableControl(IDC_STOP, true);
		Win32Work::OnFinish();
		StartWork::OnFinish();
	}
};

class Win32StopWork : public StopWork, public Win32Work
{
public:
	Win32StopWork(Win32Controller * win32Controller_) : Win32Work(win32Controller_) {}
	virtual void OnQueued()
	{
		win32Controller->EnableControl(IDC_STOP, false);
		win32Controller->UpdateControls();
	}
	virtual void OnFinish()
	{
		win32Controller->EnableControl(IDC_START, true);
		Win32Work::OnFinish();
		StopWork::OnFinish();
	}
};

class Win32QuitWork : public QuitWork, public Win32Work
{
public:
	Win32QuitWork(Win32Controller * win32Controller_) : Win32Work(win32Controller_) {}
	virtual void OnQueued()
	{
		win32Controller->EnableControl(IDC_START, false);
		win32Controller->EnableControl(IDC_STOP, false);
		win32Controller->EnableControl(IDC_QUIT, false);
		win32Controller->UpdateControls();
	}
	virtual void OnFinish()
	{
		Win32Work::OnFinish();
		QuitWork::OnFinish();
	}
};

//

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM /*lParam*/)
{
	static Win32Controller *s_Controller;

	switch (message)
	{
	case WM_INITDIALOG:
		{
			s_Controller = new Win32Controller(hDlg);
			
			s_Controller->EnableControl(IDC_START, false);
			s_Controller->EnableControl(IDC_STOP, false);
			s_Controller->EnableControl(IDC_QUIT, true);
			s_Controller->UpdateControls();

			Work *work = new Win32InitWork(s_Controller);

			s_Controller->QueueWork(std::make_shared<Win32InitWork>(Win32InitWork(s_Controller)));
			return (INT_PTR)TRUE;
		}

	case WM_CLOSE:
		{
			s_Controller->QueueWork(std::make_shared<Win32QuitWork>(Win32QuitWork(s_Controller)), IDC_QUIT);
			return (INT_PTR)TRUE;
		}

	case WM_COMMAND:
		{
			if (LOWORD(wParam) == IDC_START)
			{
				s_Controller->QueueWork(std::make_shared<Win32StartWork>(Win32StartWork(s_Controller)), IDC_START);
			}
			else if (LOWORD(wParam) == IDC_STOP)
			{
				s_Controller->QueueWork(std::make_shared<Win32StopWork>(Win32StopWork(s_Controller)), IDC_STOP);
			}
			else if (LOWORD(wParam) == IDC_QUIT)
			{
				s_Controller->QueueWork(std::make_shared<Win32QuitWork>(Win32QuitWork(s_Controller)), IDC_QUIT);
			}
			return (INT_PTR)TRUE;
		}

	case WM_WORKDONE:
		{
			s_Controller->UpdateControls();
			if ((size_t)wParam == typeid(Win32QuitWork).hash_code())
			{
				s_Controller->SetDone();
				EndDialog(hDlg, 0);
				PostQuitMessage(0);
				delete s_Controller;
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

// CommitMonitor - simple checker for new commits in svn repositories

// Copyright (C) 2007 - Stefan Kueng

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
// CommitMonitor.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "CommitMonitor.h"
#include "HiddenWindow.h"
#include "MainDlg.h"
#include "CmdLineParser.h"
#include "DiffViewer.h"
#include "version.h"

#include "apr_general.h"
#include "svn_ra.h"

// Global Variables:
HINSTANCE hInst;								// current instance
HANDLE g_mutex = 0;

#define APPNAME_MUTEX _T("CommitMonitor_{3802F59C-BEBD-49b9-A345-F99CBA2FBD0D}")

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);

	::OleInitialize(NULL);

	// we need some of the common controls
	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = ICC_LINK_CLASS|ICC_LISTVIEW_CLASSES|ICC_PAGESCROLLER_CLASS
		|ICC_PROGRESS_CLASS|ICC_STANDARD_CLASSES|ICC_TAB_CLASSES|ICC_TREEVIEW_CLASSES
		|ICC_UPDOWN_CLASS|ICC_USEREX_CLASSES|ICC_WIN95_CLASSES;
	InitCommonControlsEx(&icex);

	int argc = 0;
	const char* const * argv = NULL;
	apr_app_initialize(&argc, &argv, NULL);
	setlocale(LC_ALL, ""); 

	// create a client name string to be used for all http/https connections
	char namestring[MAX_PATH] = {0};
	sprintf_s(namestring, MAX_PATH, "CommitMonitor-%d.%d.%d.%d", CM_VERMAJOR, CM_VERMINOR, CM_VERMICRO, CM_VERBUILD);
	svn_ra_set_client_namestring(namestring);

	// first create a hidden window which serves as our main window for receiving
	// the window messages, starts the monitoring thread and handles the icon
	// in the tray area.

	MSG msg;
	msg.wParam = FALSE;
	HACCEL hAccelTable;

	hInst = hInstance;

	INITCOMMONCONTROLSEX used = {
		sizeof(INITCOMMONCONTROLSEX),
		ICC_BAR_CLASSES
	};
	InitCommonControlsEx(&used);


	CCmdLineParser parser(lpCmdLine);
	if (parser.HasKey(_T("patchfile")))
	{
		hAccelTable = LoadAccelerators(hInst, MAKEINTRESOURCE(IDC_CMVIEWER));
		// in this case, we start another part of our application, not
		// the monitoring part.
		CDiffViewer viewer(hInst);
		if (parser.HasVal(_T("title")))
			viewer.SetTitle(parser.GetVal(_T("title")));
		if (viewer.RegisterAndCreateWindow())
		{
			if (viewer.LoadFile(parser.GetVal(_T("patchfile"))))
			{
				::ShowWindow(viewer.GetHWNDEdit(), SW_SHOW);
				::SetFocus(viewer.GetHWNDEdit());

				// Main message loop:
				while (GetMessage(&msg, NULL, 0, 0))
				{
					if (!TranslateAccelerator(viewer, hAccelTable, &msg))
					{
						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}
				}
			}
		}
	}
	else
	{
		//only one instance of this application part allowed
		g_mutex = ::CreateMutex(NULL, FALSE, APPNAME_MUTEX);
        
		if (g_mutex != NULL)
		{   
			if ((::GetLastError()==ERROR_ALREADY_EXISTS)&&(!parser.HasKey(_T("task"))))
			{
				//an instance of this app is already running
				HWND hWnd = FindWindow(ResString(hInst, IDS_APP_TITLE), NULL);		//try finding the running instance of this app
				if (hWnd)
				{
					UINT COMMITMONITOR_SHOWDLGMSG = RegisterWindowMessage(_T("CommitMonitor_ShowDlgMsg"));
					PostMessage(hWnd, COMMITMONITOR_SHOWDLGMSG ,0 ,0);				//open the window of the already running app
					SetForegroundWindow(hWnd);										//set the window to front
				}
				apr_terminate();
				return FALSE;
			}		
		}


		CHiddenWindow hiddenWindow(hInst);

		hAccelTable = LoadAccelerators(hInst, MAKEINTRESOURCE(IDC_COMMITMONITOR));

		if (hiddenWindow.RegisterAndCreateWindow())
		{
			if (parser.HasKey(_T("task")))
			{
				hiddenWindow.SetTask(true);
			}
			else if (!parser.HasKey(_T("hidden")))
			{
				hiddenWindow.ShowDialog();
			}
			// Main message loop:
			while (GetMessage(&msg, NULL, 0, 0))
			{
				if (!TranslateAccelerator(hiddenWindow, hAccelTable, &msg))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}
		}
		hiddenWindow.StopThread();
	}

	::OleUninitialize();
	apr_terminate();
	return (int) msg.wParam;
}



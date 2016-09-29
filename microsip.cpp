/* 
 * Copyright (C) 2011-2016 MicroSIP (http://www.microsip.org)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 */

// microsip.cpp : Defines the class behaviors for the application.
//
#include "stdafx.h"
#include "microsip.h"
#include "mainDlg.h"
#include "const.h"
#include "settings.h"

#include "Strsafe.h"

#include <afxinet.h>

#pragma comment(linker, "/SECTION:.shr,RWS")
#pragma data_seg(".shr")
HWND hGlobal = NULL;
#pragma data_seg()


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CmicrosipApp

BEGIN_MESSAGE_MAP(CmicrosipApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CmicrosipApp construction

CmicrosipApp::CmicrosipApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CmicrosipApp object

CmicrosipApp theApp;


// CmicrosipApp initialization

BOOL CmicrosipApp::InitInstance()
{
	bool AlreadyRunning;
	HANDLE hMutexOneInstance = ::CreateMutex( NULL, TRUE,
        _T("MICROSIP-088FA840-B10D-11D3-BC36-006067709674"));
    AlreadyRunning = (GetLastError() == ERROR_ALREADY_EXISTS);
    if (hMutexOneInstance != NULL) {
        ::ReleaseMutex(hMutexOneInstance);
    }
    if ( AlreadyRunning && hGlobal != NULL) {
		if ( lstrcmp(theApp.m_lpCmdLine, _T("/exit"))==0) {
			::SendMessage(hGlobal, WM_CLOSE, NULL, NULL);
		} else if ( lstrcmp(theApp.m_lpCmdLine, _T("/minimized"))==0) {
		} else if ( lstrcmp(theApp.m_lpCmdLine, _T("/hidden"))==0) {
		} else {
			::ShowWindow(hGlobal, SW_SHOW);
			::SetForegroundWindow(hGlobal);
			if (lstrlen(theApp.m_lpCmdLine)) {
				COPYDATASTRUCT cd;
				cd.dwData = 1;
				cd.lpData = theApp.m_lpCmdLine;
				cd.cbData = sizeof(TCHAR) * (lstrlen(theApp.m_lpCmdLine) + 1);
				::SendMessage(hGlobal, WM_COPYDATA, NULL, (LPARAM)&cd);
			}
		}
        return FALSE;
	} else {
		if ( lstrcmp(theApp.m_lpCmdLine, _T("/exit"))==0) {
			return FALSE;
		}
	}

	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	// Set this to include all the common control classes you want to use
	// in your application.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	InitCtrls.dwICC = ICC_LISTVIEW_CLASSES |
		ICC_LINK_CLASS | 
		ICC_BAR_CLASSES | 
		ICC_LINK_CLASS | 
		ICC_STANDARD_CLASSES | 
		ICC_TAB_CLASSES | 
		ICC_UPDOWN_CLASS;

	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	//AfxEnableControlContainer();

	CoInitializeEx(NULL, COINIT_MULTITHREADED);

	AfxInitRichEdit2();

	WNDCLASS wc;
	// Get the info for this class.
    // #32770 is the default class name for dialogs boxes.
	::GetClassInfo(AfxGetInstanceHandle(), L"#32770", &wc);
	wc.lpszClassName = _T(_GLOBAL_NAME);
	// Register this class so that MFC can use it.
	if (!::AfxRegisterClass(&wc)) return FALSE;

	CmainDlg *mainDlg = new CmainDlg;
	m_pMainWnd = mainDlg;
	hGlobal = m_pMainWnd->m_hWnd;

	//--
	LRESULT pResult;
	mainDlg->OnTcnSelchangeTab(NULL, &pResult);

	if (mainDlg->m_startMinimized) {
		m_pMainWnd->ShowWindow(SW_HIDE);
	}

	mainDlg->OnCreated();

	//--

	if (m_pMainWnd)
      return TRUE;
   else
	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

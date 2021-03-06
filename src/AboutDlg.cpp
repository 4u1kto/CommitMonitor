// CommitMonitor - simple checker for new commits in svn repositories

// Copyright (C) 2007, 2012-2013 - Stefan Kueng

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

#include "stdafx.h"
#include "resource.h"
#include "AboutDlg.h"
#include "Registry.h"
#include "version.h"
#include <string>
#include <Commdlg.h>


CAboutDlg::CAboutDlg(HWND hParent)
    : m_hHiddenWnd(NULL)
    , m_hParent(hParent)
{
}

CAboutDlg::~CAboutDlg(void)
{
}

LRESULT CAboutDlg::DlgFunc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            InitDialog(hwndDlg, IDI_COMMITMONITOR);
            // initialize the controls
            m_link.ConvertStaticToHyperlink(hwndDlg, IDC_WEBLINK, _T("http://stefanstools.sourceforge.net"));
            TCHAR verbuf[1024] = {0};
#ifdef _WIN64
            _stprintf_s(verbuf, _countof(verbuf), _T("CommitMonitor version %d.%d.%d.%d (64-bit)"), CM_VERMAJOR, CM_VERMINOR, CM_VERMICRO, CM_VERBUILD);
#else
            _stprintf_s(verbuf, _countof(verbuf), _T("CommitMonitor version %d.%d.%d.%d"), CM_VERMAJOR, CM_VERMINOR, CM_VERMICRO, CM_VERBUILD);
#endif
            SetDlgItemText(hwndDlg, IDC_VERSIONLABEL, verbuf);
        }
        return TRUE;
    case WM_COMMAND:
        return DoCommand(LOWORD(wParam));
    default:
        return FALSE;
    }
}

LRESULT CAboutDlg::DoCommand(int id)
{
    switch (id)
    {
    case IDOK:
        // fall through
    case IDCANCEL:
        EndDialog(*this, id);
        break;
    }
    return 1;
}

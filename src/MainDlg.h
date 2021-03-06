// CommitMonitor - simple checker for new commits in svn repositories

// Copyright (C) 2007-2015 - Stefan Kueng

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

#pragma once
#include <string>
#include "BaseDialog.h"
#include "ListCtrl.h"
#include "UrlInfo.h"
#include "AeroControls.h"
#include "StringUtils.h"

#define REPOBROWSER_CTRL_MIN_WIDTH 50
#define REPOBROWSER_CTRL_MIN_HEIGHT 40

#define DRAGMODE_NONE           0
#define DRAGMODE_HORIZONTAL     1
#define DRAGMODE_VERTICAL       2

#define TIMER_REFRESH           101
#define TIMER_LABEL             102
#define TIMER_FILTER            103


#define FILTER_ELAPSE 500


/**
 * main dialog.
 */
class CMainDlg : public CDialog
{
public:
    CMainDlg(HWND hParent);
    ~CMainDlg(void);

    void                    SetUrlInfos(CUrlInfos * pUrlInfos) {m_pURLInfos = pUrlInfos;}
    void                    SetUpdateAvailable(bool bUpdate) {m_bNewerVersionAvailable = bUpdate;}
    void                    SetLastSelectedProject(const std::wstring& proj) { m_lastSelectedProject = proj; }
    std::wstring            GetLastSelectedProject() { return m_lastSelectedProject; }

protected:
    LRESULT CALLBACK        DlgFunc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT                 DoCommand(int id);
    virtual bool            PreTranslateMessage(MSG* pMsg);

private:
    bool                    OnSetCursor(HWND hWnd, UINT nHitTest, UINT message);
    bool                    OnMouseMove(UINT nFlags, POINT point);
    bool                    OnLButtonDown(UINT nFlags, POINT point);
    bool                    OnLButtonUp(UINT nFlags, POINT point);
    void                    DrawXorBar(HDC hDC, LONG x1, LONG y1, LONG width, LONG height);
    void                    PositionChildWindows(POINT point, bool bHorz, bool bShowBar);
    void                    DoResize(int width, int height);
    bool                    CreateToolbar();

    void                    RefreshURLTree(bool bSelectUnread, const std::wstring& urltoselect);
    HTREEITEM               FindParentTreeNode(const std::wstring& url);
    HTREEITEM               FindTreeNode(const std::wstring& url, HTREEITEM hItem = TVI_ROOT);
    bool                    SelectNextWithUnread(HTREEITEM hItem = TVI_ROOT);
    void                    OnSelectTreeItem(LPNMTREEVIEW lpNMTreeView);
    void                    OnSelectListItem(LPNMLISTVIEW lpNMListView);
    LRESULT                 OnCustomDrawListItem(LPNMLVCUSTOMDRAW lpNMCustomDraw);
    LRESULT                 OnCustomDrawTreeItem(LPNMTVCUSTOMDRAW lpNMCustomDraw);
    void                    OnKeyDownListItem(LPNMLVKEYDOWN pnkd);
    void                    OnDblClickListItem(LPNMITEMACTIVATE lpnmitem);
    void                    TreeItemSelected(HWND hTreeControl, HTREEITEM hSelectedItem);
    void                    RemoveSelectedListItems();
    void                    MarkAllAsRead(HTREEITEM hItem, bool includingChildren);
    void                    RefreshAll(HTREEITEM hItem);
    void                    CheckNow(HTREEITEM hItem);
    void                    SetRemoveButtonState();
    bool                    ShowDiff(bool bUseTSVN);
    void                    SortItems(int col);
    void                    SaveWndPosition();
    void                    OnContextMenu(WPARAM wParam, LPARAM lParam);
    void                    InitAliases();

    /// window procedure of the sub classed tree view control
    static LRESULT CALLBACK TreeProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam);
    WNDPROC                 m_oldTreeWndProc;   ///< pointer to the original window proc of the tree view control
    static LRESULT CALLBACK FilterProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam);
    WNDPROC                 m_oldFilterWndProc; ///< pointer to the original window proc of the filter control
    static int CALLBACK     CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

private:
    HWND                    m_hTreeControl;
    HWND                    m_hListControl;
    HWND                    m_hLogMsgControl;
    HWND                    m_hFilterControl;
    HWND                    m_hCheckControl;
    HFONT                   m_font;

    CListCtrl               m_ListCtrl;

    HWND                    m_hParent;
    HWND                    m_hwndToolbar;
    HIMAGELIST              m_hToolbarImages;
    HIMAGELIST              m_hImgList;

    int                     m_nDragMode;
    LONG                    m_oldx, m_oldy;
    LONG                    m_topmarg;
    LONG                    m_xSliderPos;
    LONG                    m_ySliderPos;
    LONG                    m_bottommarg;

    HFONT                   m_boldFont;

    CUrlInfos *             m_pURLInfos;
    std::wstring            m_lastSelectedProject;

    bool                    m_bBlockListCtrlUI;
    bool                    m_bNewerVersionAvailable;
    bool                    m_refreshNeeded;
    size_t                  m_listviewUnfilteredCount;

    AeroControlBase         m_aerocontrols;

    // key=author, value=alias
    std::map<std::wstring, std::wstring, ci_lessW> m_aliases;
};

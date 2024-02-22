#ifndef DEFINE_CTABCTRLWND_H
#define DEFINE_CTABCTRLWND_H

#include <string>
#include <list>

class CTabCtrl
{
public:
	CTabCtrl()
	{
		mLONG_PTRPoc = 0;
	}
	virtual ~CTabCtrl()
	{

	}
	void AddWnd(HWND hWnd, std::string strTitle = _T(""))
	{
		mListTabData.push_back(std::move(TabData({ hWnd, strTitle })));
	}
	int SetCurSel(int nItem);
	void SetDoubleClickClose(bool bDoubleClickClose){_bDoubleClickClose = bDoubleClickClose;};
	void SetWndChangeTitle(HWND WndChangeTitle){mWndChangeTitle = WndChangeTitle;};
	void Attach(HWND hWnd)
	{
		if(mLONG_PTRPoc)
			SetWindowLongPtr(mWnd, DWL_DLGPROC, mLONG_PTRPoc);
		mWnd = hWnd;
		mLONG_PTRPoc = SetWindowLongPtr(mWnd, DWL_DLGPROC, (LONG_PTR)s_WindowProc);
		SetWindowLongPtr(mWnd, DWL_USER, (LONG_PTR)this);
	}
protected:
	struct TabData
	{
		HWND hWnd;
		std::string strTitle;
	};
	typedef std::list<TabData> ListTabData;
	ListTabData mListTabData;
	HWND mWndChangeTitle;
	HWND mWnd;
	bool _bDoubleClickClose;
	LONG_PTR mLONG_PTRPoc;
	void SetTabPos()
	{
		if (mListTabData.size()<= 0)
			return;
		RECT rcClient, rcTab;
		GetClientRect(GetParent(mWnd), &rcClient);
		SendMessageA(mWnd, TCM_GETITEMRECT, 0, (LPARAM)& rcTab);

		int i = 0;
		auto nSel = SendMessageA(mWnd, TCM_GETCURSEL, 0, NULL);
		for (auto iter=mListTabData.begin(); iter!= mListTabData.end();++iter)
		{
			auto pTabData = iter;
			if (nSel != i++)
			{
				ShowWindow(pTabData->hWnd, SW_HIDE);
			}
			else
			{
				ShowWindow(pTabData->hWnd, SW_SHOW);
				if (NULL != mWndChangeTitle)
					SetWindowTextA(mWndChangeTitle, pTabData->strTitle.c_str());
				MoveWindow(pTabData->hWnd, 0, rcTab.bottom, rcClient.right, rcClient.bottom - rcTab.bottom, TRUE);
			}
		}
	}
	LRESULT CALLBACK WindowProc(HWND hwnd,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam
	) {
		switch (uMsg)
		{
		case WM_SIZE:
			SetTimer(hwnd, 0, 50, NULL);
			break;
		case WM_TIMER:
			KillTimer(hwnd, wParam);
			SetTabPos();
			break;
		case WM_NOTIFY:
		{
			auto pNMHDR = (NMHDR*)lParam;
			switch (pNMHDR->code)
			{
			case TCN_SELCHANGE:
				SetTimer(hwnd, 0, 50, NULL);
				break;
			}
		}
		}
		return 0;
	}

	static LRESULT CALLBACK s_WindowProc(HWND hwnd,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam
	) {
		auto pCTabCtrl = (CTabCtrl*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		return pCTabCtrl->WindowProc(hwnd, uMsg, wParam, lParam);
	}

};


#endif
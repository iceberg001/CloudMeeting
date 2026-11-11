#ifndef __CNODEDEMOWND_H__
#define __CNODEDEMOWND_H__

#include "StdAfx.h"

class CNodeDemoWnd : public DuiLib::WindowImplBase
{
public:
    CNodeDemoWnd();

    DUI_DECLARE_MESSAGE_MAP()

public:
    DuiLib::CDuiString GetSkinFile();
    LPCTSTR GetWindowClassName() const;
    UINT GetClassStyle() const;
    void OnFinalMessage(HWND hWnd);

    DuiLib::CControlUI* CreateControl(LPCTSTR pstrClass);
    void Notify(TNotifyUI& msg);

private:
    void OnCloseBtn(TNotifyUI& msg);
};

#endif // __CNODEDEMOWND_H__
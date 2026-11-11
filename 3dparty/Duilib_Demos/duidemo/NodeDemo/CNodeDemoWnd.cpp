#include "StdAfx.h"
#include "CNodeDemoWnd.h"
#include "CNodeAddUI.h"
#include "CNodeResultUI.h"
#include "../../DuiLib/Core/CNodeUIManager.h"

using namespace DuiLib;

DUI_BEGIN_MESSAGE_MAP(CNodeDemoWnd, WindowImplBase)
    DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK, OnCloseBtn)
DUI_END_MESSAGE_MAP()

CNodeDemoWnd::CNodeDemoWnd()
{
}

CDuiString CNodeDemoWnd::GetSkinFile()
{
    return _T("XML_NODE_DEMO");
}

LPCTSTR CNodeDemoWnd::GetWindowClassName() const
{
    return _T("NodeDemoWnd");
}

UINT CNodeDemoWnd::GetClassStyle() const
{
    return CS_DBLCLKS;
}

void CNodeDemoWnd::OnFinalMessage(HWND hWnd)
{
    CNodeUIManager::GetInstance().Clear();
    __super::OnFinalMessage(hWnd);
    delete this;
}

CControlUI* CNodeDemoWnd::CreateControl(LPCTSTR pstrClass)
{
    if (_tcsicmp(pstrClass, _T("CNodeAddUI")) == 0)
    {
        return new CNodeAddUI;
    }
    if (_tcsicmp(pstrClass, _T("CNodeResultUI")) == 0)
    {
        return new CNodeResultUI;
    }
    return NULL;
}

void CNodeDemoWnd::Notify(TNotifyUI& msg)
{
    __super::Notify(msg);
}

void CNodeDemoWnd::OnCloseBtn(TNotifyUI& msg)
{
    if (msg.pSender && msg.pSender->GetName().CompareNoCase(_T("closebtn")) == 0)
    {
        Close();
    }
}
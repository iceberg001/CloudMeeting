#include "StdAfx.h"
#include "CNodeResultUI.h"
#include "CNodeAddUI.h"

using namespace DuiLib;

IMPLEMENT_DUICONTROL(CNodeResultUI)

CNodeResultUI::CNodeResultUI()
    : lblResult_(NULL)
    , editResult_(NULL)
{
    SetNodeName(_T("NodeResult"));

    CHorizontalLayoutUI* layout = new CHorizontalLayoutUI;
    layout->SetInset(CDuiRect(10, 10, 10, 10));
    layout->SetChildPadding(8);
    layout->SetBkColor(0xFFFFF3E0);
    layout->SetAttribute(_T("border"), _T("1"));
    layout->SetAttribute(_T("bordercolor"), _T("#FFFF9800"));

    lblResult_ = new CLabelUI;
    lblResult_->SetFixedWidth(60);
    lblResult_->SetFixedHeight(28);
    lblResult_->SetText(_T("½á¹û="));
    lblResult_->SetAttribute(_T("align"), _T("center"));
    layout->Add(lblResult_);

    editResult_ = new CEditUI;
    editResult_->SetName(_T("editResult"));
    editResult_->SetFixedWidth(120);
    editResult_->SetFixedHeight(28);
    editResult_->SetAttribute(_T("bkcolor"), _T("#FFFFFFFF"));
    editResult_->SetAttribute(_T("border"), _T("1"));
    editResult_->SetAttribute(_T("bordercolor"), _T("#FF999999"));
    editResult_->SetReadOnly(true);
    editResult_->SetText(_T("0"));
    layout->Add(editResult_);

    this->Add(layout);
}

LPCTSTR CNodeResultUI::GetClass() const
{
    return _T("CNodeResultUI");
}

LPVOID CNodeResultUI::GetInterface(LPCTSTR pstrName)
{
    if (_tcscmp(pstrName, _T("CNodeResultUI")) == 0) return static_cast<CNodeResultUI*>(this);
    return CNodeUIBase::GetInterface(pstrName);
}

void CNodeResultUI::OnNodeMessage(const CNodeMessage& msg)
{
    if (msg.msgType == NODE_MSG_ADD_RESULT)
    {
        editResult_->SetText(msg.msgData);
        if (GetManager())
        {
            RECT rc = editResult_->GetPos();
            GetManager()->Invalidate(rc);
        }
    }
}
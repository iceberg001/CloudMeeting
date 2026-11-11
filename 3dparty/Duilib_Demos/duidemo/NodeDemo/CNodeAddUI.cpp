#include "StdAfx.h"
#include "CNodeAddUI.h"

using namespace DuiLib;

IMPLEMENT_DUICONTROL(CNodeAddUI)

CNodeAddUI::CNodeAddUI()
    : editA_(NULL)
    , editB_(NULL)
    , btnCalc_(NULL)
    , lblPlus_(NULL)
{
    SetNodeName(_T("NodeAdd"));

    CHorizontalLayoutUI* layout = new CHorizontalLayoutUI;
    layout->SetInset(CDuiRect(10, 10, 10, 10));
    layout->SetChildPadding(8);
    layout->SetBkColor(0xFFE8F5E9);
    layout->SetAttribute(_T("border"), _T("1"));
    layout->SetAttribute(_T("bordercolor"), _T("#FF4CAF50"));

    editA_ = new CEditUI;
    editA_->SetName(_T("editA"));
    editA_->SetFixedWidth(80);
    editA_->SetFixedHeight(28);
    editA_->SetAttribute(_T("bkcolor"), _T("#FFFFFFFF"));
    editA_->SetAttribute(_T("border"), _T("1"));
    editA_->SetAttribute(_T("bordercolor"), _T("#FF999999"));
    editA_->SetNumberOnly(true);
    editA_->SetText(_T("0"));
    layout->Add(editA_);

    lblPlus_ = new CLabelUI;
    lblPlus_->SetFixedWidth(20);
    lblPlus_->SetFixedHeight(28);
    lblPlus_->SetText(_T("+"));
    lblPlus_->SetAttribute(_T("align"), _T("center"));
    layout->Add(lblPlus_);

    editB_ = new CEditUI;
    editB_->SetName(_T("editB"));
    editB_->SetFixedWidth(80);
    editB_->SetFixedHeight(28);
    editB_->SetAttribute(_T("bkcolor"), _T("#FFFFFFFF"));
    editB_->SetAttribute(_T("border"), _T("1"));
    editB_->SetAttribute(_T("bordercolor"), _T("#FF999999"));
    editB_->SetNumberOnly(true);
    editB_->SetText(_T("0"));
    layout->Add(editB_);

    btnCalc_ = new CButtonUI;
    btnCalc_->SetName(_T("btnCalc"));
    btnCalc_->SetFixedWidth(50);
    btnCalc_->SetFixedHeight(28);
    btnCalc_->SetText(_T("="));
    btnCalc_->SetAttribute(_T("bkcolor"), _T("#FF4CAF50"));
    btnCalc_->SetAttribute(_T("align"), _T("center"));
    btnCalc_->OnNotify += MakeDelegate(this, &CNodeAddUI::OnButtonNotify);
    layout->Add(btnCalc_);

    this->Add(layout);
}

LPCTSTR CNodeAddUI::GetClass() const
{
    return _T("CNodeAddUI");
}

LPVOID CNodeAddUI::GetInterface(LPCTSTR pstrName)
{
    if (_tcscmp(pstrName, _T("CNodeAddUI")) == 0) return static_cast<CNodeAddUI*>(this);
    return CNodeUIBase::GetInterface(pstrName);
}

bool CNodeAddUI::OnButtonNotify(void* param)
{
    TNotifyUI* msg = static_cast<TNotifyUI*>(param);
    if (msg == NULL) return false;
    if (msg->sType != DUI_MSGTYPE_CLICK) return false;

    int a = _ttoi(editA_->GetText().GetData());
    int b = _ttoi(editB_->GetText().GetData());
    int result = a + b;

    TCHAR buf[32] = {0};
    _stprintf(buf, _T("%d"), result);

    PostNodeMessage(CDuiString(NODE_MSG_ADD_RESULT),
                    CDuiString(buf),
                    0, 0, true);
    return true;
}
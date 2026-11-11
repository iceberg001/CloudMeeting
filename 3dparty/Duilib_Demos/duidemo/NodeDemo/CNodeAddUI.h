#ifndef __CNODEADDUI_H__
#define __CNODEADDUI_H__

#include "StdAfx.h"
#include "../../DuiLib/Core/CNodeUIBase.h"

#define NODE_MSG_ADD_RESULT  _T("add_result")

class CNodeAddUI : public DuiLib::CNodeUIBase
{
    DECLARE_DUICONTROL(CNodeAddUI)
public:
    CNodeAddUI();

    LPCTSTR GetClass() const;
    LPVOID  GetInterface(LPCTSTR pstrName);

private:
    bool OnButtonNotify(void* param);

    DuiLib::CEditUI*            editA_;
    DuiLib::CEditUI*            editB_;
    DuiLib::CButtonUI*          btnCalc_;
    DuiLib::CLabelUI*           lblPlus_;
};

#endif // __CNODEADDUI_H__
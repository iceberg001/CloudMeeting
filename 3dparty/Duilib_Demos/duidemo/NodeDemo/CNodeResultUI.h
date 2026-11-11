#ifndef __CNODERESULTUI_H__
#define __CNODERESULTUI_H__

#include "StdAfx.h"
#include "../../DuiLib/Core/CNodeUIBase.h"

class CNodeResultUI : public DuiLib::CNodeUIBase
{
    DECLARE_DUICONTROL(CNodeResultUI)
public:
    CNodeResultUI();

    LPCTSTR GetClass() const;
    LPVOID  GetInterface(LPCTSTR pstrName);

    bool WantNodeMessage() const { return true; }
    void OnNodeMessage(const DuiLib::CNodeMessage& msg);

private:
    DuiLib::CLabelUI*   lblResult_;
    DuiLib::CEditUI*    editResult_;
};

#endif // __CNODERESULTUI_H__
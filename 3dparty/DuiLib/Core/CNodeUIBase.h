#ifndef __CNODEUIBASE_H__
#define __CNODEUIBASE_H__

#include "UIContainer.h"
#include "CNodeMessage.h"

namespace DuiLib {

class CNodeUIManager;

class UILIB_API CNodeUIBase : public CContainerUI
{
    DECLARE_DUICONTROL(CNodeUIBase)
public:
    CNodeUIBase();
    virtual ~CNodeUIBase();

    LPCTSTR GetClass() const;
    LPVOID  GetInterface(LPCTSTR pstrName);

    void SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit = true);
    void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
    void DoEvent(TEventUI& event);

    // 便捷查找：在当前节点子树内按名字递归查找（不依赖窗口管理器，构造阶段可用）
    CControlUI* FindControl(LPCTSTR pstrName);

    void PostNodeMessage(const CDuiString& msgType,
                         const CDuiString& msgData = _T(""),
                         WPARAM             wParam = 0,
                         LPARAM             lParam = 0,
                         bool               bAsync = true);

    void PostNodeMessage(const CDuiString& msgType,
                         void*             data,
                         size_t            size,
                         WPARAM            wParam = 0,
                         LPARAM            lParam = 0,
                         bool              bAsync = true);

    virtual bool WantNodeMessage() const { return false; }
    virtual void OnNodeMessage(const CNodeMessage& msg) {}

    CDuiString GetNodeName() const { return nodeName_; }
    void       SetNodeName(const CDuiString& name) { nodeName_ = name; }

    bool IsRegisteredToManager() const { return bRegistered_; }

protected:
    CDuiString    nodeName_;
    bool          bRegistered_;
};

} // namespace DuiLib

#endif // __CNODEUIBASE_H__
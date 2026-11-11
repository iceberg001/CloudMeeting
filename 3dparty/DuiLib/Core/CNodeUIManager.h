#ifndef __CNODEUIMANAGER_H__
#define __CNODEUIMANAGER_H__

#include "Utils/Utils.h"
#include "CNodeMessage.h"
#include <vector>

namespace DuiLib {

class CNodeUIBase;

#define NODE_TIMER_ID  0x4E4F4445  // 'NODE' - node 消息派发定时器 ID

class UILIB_API CNodeUIManager
{
public:
    static CNodeUIManager& GetInstance();

    void RegisterNode(CNodeUIBase* node);
    void UnregisterNode(CNodeUIBase* node);

    void BroadcastMessage(const CNodeMessage& msg, bool bAsync = true);
    void ProcessMessageQueue();

    size_t      GetNodeCount() const;
    CNodeUIBase* FindNode(const CDuiString& name) const;

    UINT GetActiveTimerId() const { return activeTimerId_; }

    void Clear();

private:
    CNodeUIManager();
    ~CNodeUIManager();
    CNodeUIManager(const CNodeUIManager&) ;
    CNodeUIManager& operator=(const CNodeUIManager&) ;

    std::vector<CNodeUIBase*>   nodes_;
    std::vector<CNodeMessage>   messageQueue_;
    CNodeUIBase*                timerControl_;
    UINT                        activeTimerId_;
    bool                        bTimerActive_;
};

} // namespace DuiLib

#endif // __CNODEUIMANAGER_H__
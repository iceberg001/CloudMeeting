#include "StdAfx.h"
#include "CNodeUIManager.h"
#include "CNodeUIBase.h"
#include "UIManager.h"

namespace DuiLib {

CNodeUIManager& CNodeUIManager::GetInstance()
{
    static CNodeUIManager instance;
    return instance;
}

CNodeUIManager::CNodeUIManager()
    : timerControl_(NULL)
    , activeTimerId_(0)
    , bTimerActive_(false)
{
}

CNodeUIManager::~CNodeUIManager()
{
    nodes_.clear();
    messageQueue_.clear();
}

void CNodeUIManager::RegisterNode(CNodeUIBase* node)
{
    if (node == NULL) return;
    for (size_t i = 0; i < nodes_.size(); ++i)
    {
        if (nodes_[i] == node) return;
    }
    nodes_.push_back(node);
}

void CNodeUIManager::UnregisterNode(CNodeUIBase* node)
{
    if (node == NULL) return;
    for (std::vector<CNodeUIBase*>::iterator it = nodes_.begin(); it != nodes_.end(); ++it)
    {
        if (*it == node)
        {
            nodes_.erase(it);
            return;
        }
    }
}

void CNodeUIManager::BroadcastMessage(const CNodeMessage& msg, bool bAsync)
{
    if (!bAsync)
    {
        for (size_t i = 0; i < nodes_.size(); ++i)
        {
            if (nodes_[i] && nodes_[i]->WantNodeMessage())
            {
                nodes_[i]->OnNodeMessage(msg);
            }
        }
        return;
    }

    messageQueue_.push_back(msg);

    if (!bTimerActive_)
    {
        for (size_t i = 0; i < nodes_.size(); ++i)
        {
            CNodeUIBase* node = nodes_[i];
            if (node && node->GetManager())
            {
                node->GetManager()->SetTimer(node, NODE_TIMER_ID, 30);
                activeTimerId_ = NODE_TIMER_ID;
                timerControl_ = node;
                bTimerActive_ = true;
                break;
            }
        }
    }
}

void CNodeUIManager::ProcessMessageQueue()
{
    if (bTimerActive_ && timerControl_ && timerControl_->GetManager())
    {
        timerControl_->GetManager()->KillTimer(timerControl_, activeTimerId_);
        bTimerActive_ = false;
        activeTimerId_ = 0;
        timerControl_ = NULL;
    }

    if (messageQueue_.empty()) return;

    std::vector<CNodeMessage> queue;
    queue.swap(messageQueue_);

    for (size_t i = 0; i < queue.size(); ++i)
    {
        for (size_t j = 0; j < nodes_.size(); ++j)
        {
            if (nodes_[j] && nodes_[j]->WantNodeMessage())
            {
                nodes_[j]->OnNodeMessage(queue[i]);
            }
        }
    }
}

size_t CNodeUIManager::GetNodeCount() const
{
    return nodes_.size();
}

CNodeUIBase* CNodeUIManager::FindNode(const CDuiString& name) const
{
    for (size_t i = 0; i < nodes_.size(); ++i)
    {
        if (nodes_[i] && nodes_[i]->GetNodeName() == name)
        {
            return nodes_[i];
        }
    }
    return NULL;
}

void CNodeUIManager::Clear()
{
    nodes_.clear();
    messageQueue_.clear();
    bTimerActive_ = false;
    activeTimerId_ = 0;
    timerControl_ = NULL;
}

} // namespace DuiLib
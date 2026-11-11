#ifndef MSG_H
#define MSG_H

#include "netheader.h"
#include <queue>
#include <mutex>
#include <condition_variable>
#include <cstring>

struct MsgData
{
    char* ptr = nullptr;
    int len = 0;
    int targetfd = 0;
    MSG_TYPE msgType = IMG_SEND;
    uint32_t ip = 0;

    MsgData() {}

    MsgData(MSG_TYPE msg_type, char* msg, int length, int fd)
    {
        msgType = msg_type;
        ptr = msg;
        len = length;
        targetfd = fd;
    }

    void freeData()
    {
        if (ptr)
        {
            delete[] ptr;
            ptr = nullptr;
        }
    }
};

class SEND_QUEUE
{
private:
    mutable std::mutex lock_;
    std::condition_variable cond_;
    std::queue<MsgData> sendQueue_;
    int maxSize_ = QUEUE_MAXSIZE;

public:
    SEND_QUEUE() {}

    void setMaxSize(int size) { maxSize_ = size; }

    void pushMsg(MsgData msg)
    {
        std::unique_lock<std::mutex> lk(lock_);
        while (static_cast<int>(sendQueue_.size()) >= maxSize_)
        {
            cond_.wait(lk);
        }
        sendQueue_.push(msg);
        cond_.notify_one();
    }

    // 媒体帧专用：队列积压超过上限时直接丢帧，绝不阻塞生产者（采集线程）。
    // 否则对端收流慢时队列塞满，采集线程永久卡在 pushMsg，
    // 退出时 join 采集线程就死锁，表现为“窗口关了进程还在”。
    bool pushMediaMsg(MsgData msg, int mediaCap = 30)
    {
        {
            std::unique_lock<std::mutex> lk(lock_);
            if (static_cast<int>(sendQueue_.size()) >= mediaCap)
            {
                msg.freeData();
                return false; // 丢帧
            }
            sendQueue_.push(msg);
        }
        cond_.notify_one();
        return true;
    }

    // 音频专用：音频块很小且实时性高，被整段丢弃就听不到对方说话。
    // 队列满时丢弃最旧的一帧（通常是大块视频帧）腾出空间，保证音频总能进队。
    bool pushAudioMsg(MsgData msg)
    {
        {
            std::unique_lock<std::mutex> lk(lock_);
            if (static_cast<int>(sendQueue_.size()) >= maxSize_ && !sendQueue_.empty())
            {
                sendQueue_.front().freeData();
                sendQueue_.pop();
            }
            sendQueue_.push(msg);
        }
        cond_.notify_one();
        return true;
    }

    MsgData popMsg()
    {
        std::unique_lock<std::mutex> lk(lock_);
        while (sendQueue_.empty())
        {
            cond_.wait(lk);
        }
        MsgData msg = sendQueue_.front();
        sendQueue_.pop();
        cond_.notify_one();
        return msg;
    }

    bool tryPopMsg(MsgData& msg, int timeoutMs = 2000)
    {
        std::unique_lock<std::mutex> lk(lock_);
        if (sendQueue_.empty())
        {
            cond_.wait_for(lk, std::chrono::milliseconds(timeoutMs));
        }
        if (sendQueue_.empty()) return false;
        msg = sendQueue_.front();
        sendQueue_.pop();
        cond_.notify_one();
        return true;
    }

    void clear()
    {
        std::unique_lock<std::mutex> lk(lock_);
        while (!sendQueue_.empty())
        {
            sendQueue_.front().freeData();
            sendQueue_.pop();
        }
        cond_.notify_all();
    }

    bool empty() const
    {
        std::unique_lock<std::mutex> lk(lock_);
        return sendQueue_.empty();
    }

    int size() const
    {
        std::unique_lock<std::mutex> lk(lock_);
        return static_cast<int>(sendQueue_.size());
    }
};

#endif
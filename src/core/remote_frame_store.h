#ifndef REMOTE_FRAME_STORE_H
#define REMOTE_FRAME_STORE_H

#include <cstdint>
#include <map>
#include <mutex>
#include <vector>

// 按 IP 缓存每个远端参会者的最新视频帧（RGB32）。
// 网络线程写入，UI 线程读取，内部加锁。
class RemoteFrameStore
{
public:
    static RemoteFrameStore& Instance()
    {
        static RemoteFrameStore instance;
        return instance;
    }

    struct Frame
    {
        std::vector<uint8_t> data;  // RGB32 像素数据
        int width = 0;
        int height = 0;
    };

    // 写入/覆盖某 IP 的最新帧
    void Update(uint32_t ip, const uint8_t* data, int len, int width, int height)
    {
        if (!data || len <= 0 || width <= 0 || height <= 0) return;
        std::lock_guard<std::mutex> lk(lock_);
        Frame& f = frames_[ip];
        f.data.assign(data, data + len);
        f.width = width;
        f.height = height;
    }

    // 读取某 IP 的最新帧（拷贝出去）
    bool Get(uint32_t ip, Frame& out) const
    {
        std::lock_guard<std::mutex> lk(lock_);
        auto it = frames_.find(ip);
        if (it == frames_.end() || it->second.data.empty()) return false;
        out = it->second;
        return true;
    }

    void Remove(uint32_t ip)
    {
        std::lock_guard<std::mutex> lk(lock_);
        frames_.erase(ip);
    }

    void Clear()
    {
        std::lock_guard<std::mutex> lk(lock_);
        frames_.clear();
    }

private:
    mutable std::mutex lock_;
    std::map<uint32_t, Frame> frames_;
};

#endif

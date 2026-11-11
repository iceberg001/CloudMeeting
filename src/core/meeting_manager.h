#ifndef MEETING_MANAGER_H
#define MEETING_MANAGER_H

#include "server_logic.h"
#include "client_logic.h"
#include <memory>
#include <mutex>
#include <functional>
#include <string>
#include <set>
#include <chrono>
#include <cstdint>

class MeetingManager
{
public:
    enum class Role { NONE, SERVER, CLIENT };

    static MeetingManager& GetInstance();

    void StartAsServer(int port, int maxRooms);
    void StartAsClient(const std::string& serverIp, int port);
    void Stop();

    uint32_t CreateMeeting();
    bool JoinMeeting(uint32_t roomNo);
    void ExitMeeting();

    // 视频/音频/文本发送。视频帧自动在头部附加 8 字节宽高信息（网络序）。
    void SendVideoFrame(const uint8_t* data, int len, int width, int height);
    void SendAudioData(const uint8_t* data, int len);
    void SendTextMessage(const std::string& text);
    void SendCloseCamera();

    Role GetRole() const { return role_; }
    bool IsInMeeting() const { return inMeeting_; }
    uint32_t GetRoomNo() const { return roomNo_; }
    uint32_t GetSelfIp() const { return selfIp_; }
    const std::set<uint32_t>& GetPartnerIps() const { return partnerIps_; }
    // 服务器 IP（客户端模式下是所连接的服务器，服务器模式下是本机 IP）
    std::string GetServerIpStr() const { return TcpSocket::IpToString(serverIp_); }

    using MeetingCreatedCallback = std::function<void(uint32_t roomNo)>;
    using MeetingJoinedCallback = std::function<void(uint32_t roomNo)>;
    using PartnerJoinCallback = std::function<void(uint32_t ip)>;
    using PartnerExitCallback = std::function<void(uint32_t ip)>;
    using VideoFrameCallback = std::function<void(uint32_t ip, const uint8_t* data, int len, int width, int height)>;
    using AudioDataCallback = std::function<void(uint32_t ip, const uint8_t* data, int len)>;
    using TextMessageCallback = std::function<void(uint32_t ip, const std::string& text)>;
    using CameraClosedCallback = std::function<void(uint32_t ip)>;
    using ErrorCallback = std::function<void(const std::string&)>;

    void SetMeetingCreatedCallback(MeetingCreatedCallback cb) { meetingCreatedCb_ = cb; }
    void SetMeetingJoinedCallback(MeetingJoinedCallback cb) { meetingJoinedCb_ = cb; }
    void SetPartnerJoinCallback(PartnerJoinCallback cb) { partnerJoinCb_ = cb; }
    void SetPartnerExitCallback(PartnerExitCallback cb) { partnerExitCb_ = cb; }
    void SetVideoFrameCallback(VideoFrameCallback cb) { videoFrameCb_ = cb; }
    void SetAudioDataCallback(AudioDataCallback cb) { audioDataCb_ = cb; }
    void SetTextMessageCallback(TextMessageCallback cb) { textMsgCb_ = cb; }
    void SetCameraClosedCallback(CameraClosedCallback cb) { cameraClosedCb_ = cb; }
    void SetErrorCallback(ErrorCallback cb) { errorCb_ = cb; }

private:
    MeetingManager();
    ~MeetingManager();

    Role role_ = Role::NONE;
    bool inMeeting_ = false;
    uint32_t roomNo_ = 0;
    uint32_t selfIp_ = 0;
    uint32_t serverIp_ = 0;
    std::set<uint32_t> partnerIps_;

    // 视频发送节流（约 10fps）：RGB32 帧很大（1080p 桌面约 8MB），
    // 不限帧率会把发送队列塞满，音频被挤掉，表现为“看得到聊天听不到声音”
    std::chrono::steady_clock::time_point lastVideoFrameTime_{};

    std::unique_ptr<ServerLogic> serverLogic_;
    std::unique_ptr<ClientLogic> clientLogic_;

    // 保护 serverLogic_/clientLogic_ 的生命周期：
    // 采集线程调用 Send* 访问网络对象，退出时 Stop() 会销毁它们，
    // 不加锁会出现 use-after-free（窗口关闭后崩溃或残留）。
    // Stop 持锁 join 是安全的：Server/Client 内部线程不取这把锁。
    mutable std::mutex netMutex_;

    MeetingCreatedCallback meetingCreatedCb_;
    MeetingJoinedCallback meetingJoinedCb_;
    PartnerJoinCallback partnerJoinCb_;
    PartnerExitCallback partnerExitCb_;
    VideoFrameCallback videoFrameCb_;
    AudioDataCallback audioDataCb_;
    TextMessageCallback textMsgCb_;
    CameraClosedCallback cameraClosedCb_;
    ErrorCallback errorCb_;

    void OnClientMsg(const MsgData& msg);
};

#endif
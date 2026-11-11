#include "meeting_manager.h"
#include "net_utils.h"
#include "debug_log.h"
#include <cstring>
#include <vector>


MeetingManager& MeetingManager::GetInstance()
{
    static MeetingManager instance;
    return instance;
}

MeetingManager::MeetingManager()
{
    selfIp_ = TcpSocket::StringToIp(GetLocalIp());
}

MeetingManager::~MeetingManager()
{
    Stop();
}

void MeetingManager::StartAsServer(int port, int maxRooms)
{
    LOG_DEBUG("StartAsServer port=%d maxRooms=%d", port, maxRooms);
    if (role_ != Role::NONE) Stop();

    std::unique_lock<std::mutex> lk(netMutex_);
    role_ = Role::SERVER;
    serverIp_ = selfIp_;
    serverLogic_ = std::make_unique<ServerLogic>(port, maxRooms);

    serverLogic_->SetMeetingCreatedCallback([this](uint32_t roomNo) {
        LOG_DEBUG("MeetingCreatedCallback roomNo=%u", roomNo);
        roomNo_ = roomNo;
        inMeeting_ = true;
        partnerIps_.insert(selfIp_);
        if (meetingCreatedCb_) meetingCreatedCb_(roomNo);
    });

    // 本地房主接收通道：房间内其他人的视频/语音/聊天/上下线也送到房主 UI
    serverLogic_->SetOwnerInboundCallback([this](int msgType, uint32_t ip, const char* data, int len) {
        if (ip == selfIp_) return; // 跳过自己广播出去的消息
        MsgData msg;
        msg.msgType = static_cast<MSG_TYPE>(msgType);
        msg.ip = ip;
        msg.ptr = const_cast<char*>(data);
        msg.len = len;
        OnClientMsg(msg); // 不拥有 ptr，OnClientMsg 不负责释放
    });

    serverLogic_->Start();
    LOG_DEBUG("ServerLogic started, running=%d", serverLogic_->IsRunning() ? 1 : 0);
}

void MeetingManager::StartAsClient(const std::string& serverIp, int port)
{
    if (role_ != Role::NONE) Stop();

    std::unique_lock<std::mutex> lk(netMutex_);
    role_ = Role::CLIENT;
    serverIp_ = TcpSocket::StringToIp(serverIp);
    clientLogic_ = std::make_unique<ClientLogic>();

    clientLogic_->SetMsgCallback([this](const MsgData& msg) {
        OnClientMsg(msg);
    });

    clientLogic_->SetErrorCallback([this](const std::string& err) {
        if (errorCb_) errorCb_(err);
    });

    if (!clientLogic_->Connect(serverIp, port))
    {
        if (errorCb_) errorCb_("Failed to connect to server");
        role_ = Role::NONE;
        return;
    }

    // 用连接本端地址校正本机 IP（GetLocalIp 在多网卡机器上可能取错，
    // 比如返回虚拟网卡地址）。这是本机在到服务器链路上的真实 IP，
    // 与创建者广播帧所带的 IP 绝不会相同，避免参与者窗格错乱
    uint32_t localIp = TcpSocket::GetLocalAddrIp(clientLogic_->GetSocket());
    if (localIp != 0)
        selfIp_ = localIp;
}

void MeetingManager::Stop()
{
    std::unique_lock<std::mutex> lk(netMutex_);
    if (role_ == Role::SERVER && serverLogic_)
    {
        serverLogic_->Stop();
        serverLogic_.reset();
    }
    if (role_ == Role::CLIENT && clientLogic_)
    {
        clientLogic_->Disconnect();
        clientLogic_.reset();
    }

    role_ = Role::NONE;
    inMeeting_ = false;
    roomNo_ = 0;
    partnerIps_.clear();
}

uint32_t MeetingManager::CreateMeeting()
{
    LOG_DEBUG("CreateMeeting role=%d", (int)role_);
    std::unique_lock<std::mutex> lk(netMutex_);
    if (role_ == Role::SERVER && serverLogic_)
    {
        // 登记房主 IP：新成员加入时服务器要把它放进成员列表
        uint32_t roomNo = serverLogic_->CreateMeetingForOwner(selfIp_);
        LOG_DEBUG("CreateMeetingForOwner returned roomNo=%u", roomNo);
        return roomNo;
    }
    if (role_ == Role::CLIENT && clientLogic_)
    {
        clientLogic_->SendCreateMeeting();
    }
    return 0;
}

bool MeetingManager::JoinMeeting(uint32_t roomNo)
{
    std::unique_lock<std::mutex> lk(netMutex_);
    if (role_ == Role::CLIENT && clientLogic_)
    {
        clientLogic_->SendJoinMeeting(roomNo);
        return true;
    }
    return false;
}

void MeetingManager::ExitMeeting()
{
    {
        std::unique_lock<std::mutex> lk(netMutex_);
        if (role_ == Role::CLIENT && clientLogic_)
        {
            clientLogic_->SendExitMeeting();
        }
    }

    inMeeting_ = false;
    roomNo_ = 0;
    partnerIps_.clear();
}

// 大帧最近邻降采样（RGB32，宽高各减半）：1080p 单帧约 8MB，
// 不降采样会把发送队列/带宽塞死，音频被挤掉
static void DownsampleRgb32(const uint8_t* src, int width, int height, std::vector<uint8_t>& dst)
{
    int dw = width / 2;
    int dh = height / 2;
    if (dw <= 0 || dh <= 0) { dst.clear(); return; }
    dst.resize(static_cast<size_t>(dw) * dh * 4);
    for (int y = 0; y < dh; ++y)
    {
        const uint8_t* srow = src + static_cast<size_t>(y * 2) * width * 4;
        uint8_t* drow = dst.data() + static_cast<size_t>(y) * dw * 4;
        for (int x = 0; x < dw; ++x)
        {
            memcpy(drow + static_cast<size_t>(x) * 4, srow + static_cast<size_t>(x) * 8, 4);
        }
    }
}

void MeetingManager::SendVideoFrame(const uint8_t* data, int len, int width, int height)
{
    if (!data || len <= 0 || width <= 0 || height <= 0) return;

    // 节流：最多约 10fps，控制局域网带宽，避免队列拥塞把音频挤掉
    {
        auto now = std::chrono::steady_clock::now();
        if (now - lastVideoFrameTime_ < std::chrono::milliseconds(100))
            return;
        lastVideoFrameTime_ = now;
    }

    // 降采样：宽度超过 1280 的帧宽高各减半
    std::vector<uint8_t> down;
    const uint8_t* sdata = data;
    int sw = width, sh = height;
    if (width > 1280)
    {
        DownsampleRgb32(data, width, height, down);
        if (!down.empty())
        {
            sdata = down.data();
            sw = width / 2;
            sh = height / 2;
        }
    }
    int slen = sw * sh * 4;

    // 帧载荷 = 8字节头（宽、高，网络序） + RGB32 像素
    std::vector<uint8_t> payload(slen + 8);
    uint32_t nw = Htonl(static_cast<uint32_t>(sw));
    uint32_t nh = Htonl(static_cast<uint32_t>(sh));
    memcpy(payload.data(), &nw, sizeof(uint32_t));
    memcpy(payload.data() + 4, &nh, sizeof(uint32_t));
    memcpy(payload.data() + 8, sdata, slen);

    std::unique_lock<std::mutex> lk(netMutex_);
    if (role_ == Role::CLIENT && clientLogic_)
    {
        clientLogic_->SendVideoFrame(payload.data(), static_cast<int>(payload.size()));
    }
    else if (role_ == Role::SERVER && serverLogic_ && roomNo_ > 0)
    {
        serverLogic_->BroadcastMediaToRoom(roomNo_, selfIp_, IMG_RECV,
                                           payload.data(), static_cast<int>(payload.size()));
    }
}

void MeetingManager::SendAudioData(const uint8_t* data, int len)
{
    std::unique_lock<std::mutex> lk(netMutex_);
    if (role_ == Role::CLIENT && clientLogic_)
    {
        clientLogic_->SendAudioData(data, len);
    }
    else if (role_ == Role::SERVER && serverLogic_ && roomNo_ > 0)
    {
        serverLogic_->BroadcastMediaToRoom(roomNo_, selfIp_, AUDIO_RECV, data, len);
    }
}

void MeetingManager::SendTextMessage(const std::string& text)
{
    std::unique_lock<std::mutex> lk(netMutex_);
    if (role_ == Role::CLIENT && clientLogic_)
    {
        clientLogic_->SendTextMessage(text);
    }
    else if (role_ == Role::SERVER && serverLogic_ && roomNo_ > 0)
    {
        serverLogic_->BroadcastTextToRoom(roomNo_, selfIp_, text);
    }
}

void MeetingManager::SendCloseCamera()
{
    std::unique_lock<std::mutex> lk(netMutex_);
    if (role_ == Role::CLIENT && clientLogic_)
    {
        clientLogic_->SendCloseCamera();
    }
    else if (role_ == Role::SERVER && serverLogic_ && roomNo_ > 0)
    {
        serverLogic_->BroadcastCloseCameraToRoom(roomNo_, selfIp_);
    }
}

void MeetingManager::OnClientMsg(const MsgData& msgIn)
{
    MsgData msg = msgIn;
    // 客户端身份保护：服务器不会把客户端自己发的帧回发给自己，
    // 所以收到"IP == 本机 IP"的消息只可能是创建者自报 IP 出错
    // （GetLocalIp 取错网卡，比如与本机相同），统一用服务器 IP 代替，
    // 否则创建者的画面会渲染进"我"的窗格里来回切换
    if (role_ == Role::CLIENT && msg.ip != 0 &&
        msg.ip == selfIp_ && serverIp_ != 0)
    {
        msg.ip = serverIp_;
    }

    switch (msg.msgType)
    {
    case CREATE_MEETING_RESPONSE:
    {
        if (msg.ptr && msg.len >= sizeof(uint32_t))
        {
            uint32_t roomNo;
            memcpy(&roomNo, msg.ptr, sizeof(uint32_t));
            roomNo = Ntohl(roomNo);
            if (roomNo > 0)
            {
                roomNo_ = roomNo;
                inMeeting_ = true;
                partnerIps_.insert(selfIp_);
                if (meetingCreatedCb_) meetingCreatedCb_(roomNo);
            }
            else
            {
                if (errorCb_) errorCb_("No available rooms");
            }
        }
        break;
    }
    case JOIN_MEETING_RESPONSE:
    {
        if (msg.ptr && msg.len >= sizeof(uint32_t))
        {
            uint32_t result;
            memcpy(&result, msg.ptr, sizeof(uint32_t));
            result = Ntohl(result);
            if (result > 0 && result != 0xFFFFFFFF)
            {
                roomNo_ = result;
                inMeeting_ = true;
                partnerIps_.insert(selfIp_);
                if (meetingJoinedCb_) meetingJoinedCb_(result);
            }
            else if (result == 0)
            {
                if (errorCb_) errorCb_("Meeting does not exist");
            }
            else if (result == 0xFFFFFFFF)
            {
                if (errorCb_) errorCb_("Meeting is full");
            }
        }
        break;
    }
    case PARTNER_JOIN:
    {
        partnerIps_.insert(msg.ip);
        if (partnerJoinCb_) partnerJoinCb_(msg.ip);
        break;
    }
    case PARTNER_JOIN2:
    {
        if (msg.ptr && msg.len > 0)
        {
            int count = msg.len / sizeof(uint32_t);
            for (int i = 0; i < count; i++)
            {
                uint32_t ip;
                memcpy(&ip, msg.ptr + i * sizeof(uint32_t), sizeof(uint32_t));
                // 成员列表里的 IP 也做同样的身份保护
                if (role_ == Role::CLIENT && ip == selfIp_ && serverIp_ != 0)
                    ip = serverIp_;
                partnerIps_.insert(ip);
                if (partnerJoinCb_) partnerJoinCb_(ip);
            }
        }
        break;
    }
    case PARTNER_EXIT:
    {
        partnerIps_.erase(msg.ip);
        if (partnerExitCb_) partnerExitCb_(msg.ip);
        break;
    }
    case IMG_RECV:
    {
        // 载荷 = 8字节宽高头 + RGB32 像素
        if (msg.ptr && msg.len > 8 && videoFrameCb_)
        {
            uint32_t nw, nh;
            memcpy(&nw, msg.ptr, sizeof(uint32_t));
            memcpy(&nh, msg.ptr + 4, sizeof(uint32_t));
            int width = static_cast<int>(Ntohl(nw));
            int height = static_cast<int>(Ntohl(nh));
            if (width > 0 && height > 0)
            {
                videoFrameCb_(msg.ip,
                              reinterpret_cast<const uint8_t*>(msg.ptr) + 8,
                              msg.len - 8, width, height);
            }
        }
        break;
    }
    case AUDIO_RECV:
    {
        if (msg.ptr && msg.len > 0 && audioDataCb_)
        {
            audioDataCb_(msg.ip, reinterpret_cast<const uint8_t*>(msg.ptr), msg.len);
        }
        break;
    }
    case TEXT_RECV:
    {
        if (msg.ptr && msg.len > 0 && textMsgCb_)
        {
            std::string text(msg.ptr, msg.len);
            textMsgCb_(msg.ip, text);
        }
        break;
    }
    case CLOSE_CAMERA:
    {
        if (cameraClosedCb_) cameraClosedCb_(msg.ip);
        break;
    }
    default:
        break;
    }
}

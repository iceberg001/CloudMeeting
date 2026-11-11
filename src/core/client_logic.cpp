#include "client_logic.h"
#include "net_utils.h"
#include <cstring>
#include <cstdio>

#pragma comment(lib, "ws2_32.lib")

ClientLogic::ClientLogic()
{
    TcpSocket::InitWinsock();
}

ClientLogic::~ClientLogic()
{
    Disconnect();
    TcpSocket::CleanupWinsock();
}

bool ClientLogic::Connect(const std::string& serverIp, int port)
{
    if (connected_.load()) return true;

    serverIp_ = serverIp;
    port_ = port;

    sockFd_ = TcpSocket::Create();
    if (sockFd_ == INVALID_SOCKET) return false;

    TcpSocket::SetTcpNoDelay(sockFd_);

    if (!TcpSocket::Connect(sockFd_, serverIp_, port_))
    {
        TcpSocket::Close(sockFd_);
        sockFd_ = INVALID_SOCKET;
        return false;
    }

    connected_ = true;
    running_ = true;

    recvThread_ = std::thread(&ClientLogic::RecvLoop, this);
    sendThread_ = std::thread(&ClientLogic::SendLoop, this);

    return true;
}

void ClientLogic::Disconnect()
{
    if (!connected_.load()) return;

    connected_ = false;
    running_ = false;

    sendQueue_.clear();

    if (sockFd_ != INVALID_SOCKET)
    {
        TcpSocket::Close(sockFd_);
        sockFd_ = INVALID_SOCKET;
    }

    if (recvThread_.joinable())
        recvThread_.join();
    if (sendThread_.joinable())
        sendThread_.join();
}

void ClientLogic::SendCreateMeeting()
{
    SendMsg(CREATE_MEETING, nullptr, 0);
}

void ClientLogic::SendJoinMeeting(uint32_t roomNo)
{
    uint32_t netRoomNo = Htonl(roomNo);
    char* data = new char[sizeof(uint32_t)];
    memcpy(data, &netRoomNo, sizeof(uint32_t));
    SendMsg(JOIN_MEETING, data, sizeof(uint32_t));
}

void ClientLogic::SendExitMeeting()
{
    SendMsg(EXIT_MEETING, nullptr, 0);
}

void ClientLogic::SendVideoFrame(const uint8_t* data, int len)
{
    char* buf = new char[len];
    memcpy(buf, data, len);
    SendMsg(IMG_SEND, buf, len);
}

void ClientLogic::SendAudioData(const uint8_t* data, int len)
{
    char* buf = new char[len];
    memcpy(buf, data, len);
    SendMsg(AUDIO_SEND, buf, len);
}

void ClientLogic::SendTextMessage(const std::string& text)
{
    char* buf = new char[text.size()];
    memcpy(buf, text.c_str(), text.size());
    SendMsg(TEXT_SEND, buf, static_cast<int>(text.size()));
}

void ClientLogic::SendCloseCamera()
{
    SendMsg(CLOSE_CAMERA, nullptr, 0);
}

void ClientLogic::SendMsg(MSG_TYPE type, const char* data, int len)
{
    MsgData msg;
    msg.msgType = type;
    msg.len = len;
    if (data && len > 0)
    {
        msg.ptr = new char[len];
        memcpy(msg.ptr, data, len);
    }
    else
    {
        msg.ptr = nullptr;
    }
    // 媒体帧：队列积压时丢弃，绝不阻塞采集线程（否则退出时 join 死锁）。
    // 音频走优先通道：满时挤掉最旧的帧，保证小块音频不被整段丢弃。
    if (type == AUDIO_SEND)
        sendQueue_.pushAudioMsg(msg);
    else if (type == IMG_SEND)
        sendQueue_.pushMediaMsg(msg);
    else
        sendQueue_.pushMsg(msg);
}

void ClientLogic::SendLoop()
{
    while (running_.load())
    {
        MsgData msg;
        if (!sendQueue_.tryPopMsg(msg, 2000))
            continue;

        char* sendBuf = new char[msg.len + MSG_HEADER + 2];
        int len = 0;

        sendBuf[len++] = '$';
        uint16_t type = Htons(static_cast<uint16_t>(msg.msgType));
        memcpy(sendBuf + len, &type, sizeof(uint16_t));
        len += 2;

        uint32_t ip = 0;
        memcpy(sendBuf + len, &ip, sizeof(uint32_t));
        len += 4;

        uint32_t msgLen = Htonl(static_cast<uint32_t>(msg.len));
        memcpy(sendBuf + len, &msgLen, sizeof(uint32_t));
        len += 4;

        if (msg.ptr && msg.len > 0)
        {
            memcpy(sendBuf + len, msg.ptr, msg.len);
            len += msg.len;
        }

        sendBuf[len++] = '#';

        int ret = Writen(sockFd_, sendBuf, len);
        if (ret < 0 && errorCb_)
        {
            errorCb_("Send error");
        }

        delete[] sendBuf;
        msg.freeData();
    }
}

void ClientLogic::RecvLoop()
{
    while (running_.load())
    {
        char head[15] = {0};
        int ret = Readn(sockFd_, head, MSG_HEADER);
        if (ret <= 0 || ret < MSG_HEADER)
        {
            if (running_.load() && errorCb_)
            {
                errorCb_("Connection closed by remote");
            }
            connected_ = false;
            running_ = false;
            return;
        }

        if (head[0] != '$')
        {
            continue;
        }

        MSG_TYPE msgType;
        memcpy(&msgType, head + 1, 2);
        msgType = static_cast<MSG_TYPE>(Ntohs(static_cast<uint16_t>(msgType)));

        uint32_t ip;
        memcpy(&ip, head + 3, 4);

        uint32_t msgLen;
        memcpy(&msgLen, head + 7, 4);
        msgLen = Ntohl(msgLen);

        char* data = nullptr;
        if (msgLen > 0)
        {
            data = new char[msgLen];
            ret = Readn(sockFd_, data, msgLen);
            if (ret < static_cast<int>(msgLen))
            {
                delete[] data;
                if (running_.load() && errorCb_)
                {
                    errorCb_("Data incomplete");
                }
                connected_ = false;
                running_ = false;
                return;
            }
        }

        char tail;
        Readn(sockFd_, &tail, 1);
        if (tail != '#')
        {
            delete[] data;
            continue;
        }

        MsgData msg;
        msg.msgType = msgType;
        msg.ip = ip;
        msg.ptr = data;
        msg.len = msgLen;

        if (msgCallback_)
        {
            msgCallback_(msg);
        }

        msg.freeData();
    }
}
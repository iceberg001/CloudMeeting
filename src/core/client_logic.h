#ifndef CLIENT_LOGIC_H
#define CLIENT_LOGIC_H

#include "msg.h"
#include "tcp_socket.h"
#include <thread>
#include <mutex>
#include <atomic>
#include <functional>
#include <string>
#include <cstdint>

class ClientLogic
{
public:
    ClientLogic();
    ~ClientLogic();

    bool Connect(const std::string& serverIp, int port);
    void Disconnect();
    bool IsConnected() const { return connected_.load(); }
    SOCKET GetSocket() const { return sockFd_; }

    void SendCreateMeeting();
    void SendJoinMeeting(uint32_t roomNo);
    void SendExitMeeting();
    void SendVideoFrame(const uint8_t* data, int len);
    void SendAudioData(const uint8_t* data, int len);
    void SendTextMessage(const std::string& text);
    void SendCloseCamera();

    using MsgCallback = std::function<void(const MsgData&)>;
    void SetMsgCallback(MsgCallback cb) { msgCallback_ = cb; }

    using ErrorCallback = std::function<void(const std::string&)>;
    void SetErrorCallback(ErrorCallback cb) { errorCb_ = cb; }

private:
    std::string serverIp_;
    int port_ = 0;
    SOCKET sockFd_ = INVALID_SOCKET;
    std::atomic<bool> connected_{false};
    std::atomic<bool> running_{false};

    std::thread recvThread_;
    std::thread sendThread_;

    SEND_QUEUE sendQueue_;
    MsgCallback msgCallback_;
    ErrorCallback errorCb_;

    void RecvLoop();
    void SendLoop();
    void SendMsg(MSG_TYPE type, const char* data, int len);
    void ProcessRecvData(const char* head, int headLen);
};

#endif
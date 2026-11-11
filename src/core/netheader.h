#ifndef NETHEADER_H
#define NETHEADER_H

#include <cstdint>

enum MSG_TYPE
{
    IMG_SEND = 0,
    IMG_RECV,
    AUDIO_SEND,
    AUDIO_RECV,
    TEXT_SEND,
    TEXT_RECV,
    CREATE_MEETING,
    EXIT_MEETING,
    JOIN_MEETING,
    CLOSE_CAMERA,

    CREATE_MEETING_RESPONSE = 20,
    PARTNER_EXIT = 21,
    PARTNER_JOIN = 22,
    JOIN_MEETING_RESPONSE = 23,
    PARTNER_JOIN2 = 24,

    RemoteHostClosedError = 40,
    OtherNetError = 41
};

#define MSG_HEADER 11
#define QUEUE_MAXSIZE 1500
#define MB (1024 * 1024)

#endif
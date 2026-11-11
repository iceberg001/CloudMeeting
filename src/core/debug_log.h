#ifndef DEBUG_LOG_H
#define DEBUG_LOG_H

#include <cstdio>
#include <cstdarg>
#include <windows.h>

inline void DebugLog(const char* fmt, ...)
{
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    OutputDebugStringA("[CloudMeeting] ");
    OutputDebugStringA(buf);
    OutputDebugStringA("\n");

    char logPath[MAX_PATH] = {0};
    GetModuleFileNameA(nullptr, logPath, MAX_PATH);
    char* pSlash = strrchr(logPath, '\\');
    if (pSlash) *(pSlash + 1) = '\0';
    strcat_s(logPath, MAX_PATH, "cloudmeeting_debug.log");

    FILE* fp = nullptr;
    fopen_s(&fp, logPath, "a");
    if (fp)
    {
        // 带毫秒时间戳，便于分析退出/卡顿耗时
        SYSTEMTIME st;
        GetLocalTime(&st);
        fprintf(fp, "[CloudMeeting %02d:%02d:%02d.%03d] %s\n",
                st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, buf);
        fclose(fp);
    }
}

#define LOG_DEBUG(fmt, ...) DebugLog(fmt, ##__VA_ARGS__)

#endif
#ifndef __CNODEMESSAGE_H__
#define __CNODEMESSAGE_H__

#include "Utils/Utils.h"

namespace DuiLib {

class CNodeUIBase;

struct CNodeMessage
{
    CDuiString        msgType;
    CDuiString        msgData;
    void *            dataPtr_;
    size_t            dataSize_;
    CNodeUIBase *     sender_;
    WPARAM            wParam;
    LPARAM            lParam;

    CNodeMessage()
        : dataPtr_(NULL), dataSize_(0)
        , sender_(NULL), wParam(0), lParam(0) {}

    CNodeMessage(const CDuiString& type,
                 const CDuiString& data = _T(""),
                 CNodeUIBase*       sender = NULL,
                 WPARAM             w = 0,
                 LPARAM             l = 0)
        : msgType(type), msgData(data)
        , dataPtr_(NULL), dataSize_(0)
        , sender_(sender), wParam(w), lParam(l) {}

    CNodeMessage(const CDuiString& type,
                 void*              data,
                 size_t             size,
                 CNodeUIBase*       sender = NULL,
                 WPARAM             w = 0,
                 LPARAM             l = 0)
        : msgType(type)
        , dataPtr_(data), dataSize_(size)
        , sender_(sender), wParam(w), lParam(l) {}

    template <typename T>
    T * GetDataAs() const
    {
        return static_cast<T*>(dataPtr_);
    }
};

} // namespace DuiLib

#endif // __CNODEMESSAGE_H__
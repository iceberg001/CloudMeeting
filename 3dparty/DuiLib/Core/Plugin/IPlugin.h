#ifndef __IPLUGIN_H__
#define __IPLUGIN_H__

#include "Utils/Utils.h"

namespace DuiLib {

struct PluginInfo
{
    CDuiString sName;
    CDuiString sVersion;
    CDuiString sAuthor;
    CDuiString sDescription;
    CDuiString sComponents;
};

class IPlugin
{
public:
    virtual ~IPlugin() {}
    virtual bool OnLoad(class CPaintManagerUI* pm) = 0;
    virtual void OnUnload() = 0;
    virtual PluginInfo GetInfo() const = 0;
};

} // namespace DuiLib

typedef DuiLib::IPlugin* (*FN_CreatePlugin)();
typedef void (*FN_DestroyPlugin)(DuiLib::IPlugin*);

#define PLUGIN_EXPORT extern "C" __declspec(dllexport)

#endif // __IPLUGIN_H__
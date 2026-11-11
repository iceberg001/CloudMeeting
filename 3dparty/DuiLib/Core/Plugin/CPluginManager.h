#ifndef __CPLUGINMANAGER_H__
#define __CPLUGINMANAGER_H__

#include "Utils/Utils.h"
#include "IPlugin.h"
#include <vector>
#include <map>

namespace DuiLib {

struct LoadedPlugin
{
    HMODULE          hModule;
    IPlugin*         pPlugin;
    PluginInfo       info;
    CDuiString       sFilePath;
};

class UILIB_API CPluginManager
{
public:
    static CPluginManager& GetInstance();

    bool LoadPlugin(LPCTSTR pszDllPath, CPaintManagerUI* pm);
    void UnloadPlugin(LPCTSTR pszName);
    void UnloadAll();

    int GetPluginCount() const;
    const LoadedPlugin* GetPlugin(int index) const;
    const LoadedPlugin* FindPlugin(LPCTSTR pszName) const;

    CDuiString GetAllComponentNames() const;

    void RegisterDescription(LPCTSTR pszClassName, LPCTSTR pszDesc);
    LPCTSTR GetComponentDescription(LPCTSTR pszClassName) const;

private:
    CPluginManager() {}
    ~CPluginManager();
    CPluginManager(const CPluginManager&);
    CPluginManager& operator=(const CPluginManager&);

    std::vector<LoadedPlugin*> m_plugins;
    std::map<CDuiString, CDuiString> m_descriptions;
};

} // namespace DuiLib

#endif // __CPLUGINMANAGER_H__
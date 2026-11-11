#include "StdAfx.h"
#include "CPluginManager.h"
#include "UIManager.h"
#include "ControlFactory.h"

namespace DuiLib {

CPluginManager& CPluginManager::GetInstance()
{
    static CPluginManager instance;
    return instance;
}

CPluginManager::~CPluginManager()
{
    UnloadAll();
}

bool CPluginManager::LoadPlugin(LPCTSTR pszDllPath, CPaintManagerUI* pm)
{
    HMODULE hMod = ::LoadLibrary(pszDllPath);
    if (hMod == NULL) return false;

    FN_CreatePlugin pCreate = (FN_CreatePlugin)::GetProcAddress(hMod, "CreatePlugin");
    FN_DestroyPlugin pDestroy = (FN_DestroyPlugin)::GetProcAddress(hMod, "DestroyPlugin");
    if (pCreate == NULL || pDestroy == NULL)
    {
        ::FreeLibrary(hMod);
        return false;
    }

    IPlugin* pPlugin = pCreate();
    if (pPlugin == NULL)
    {
        ::FreeLibrary(hMod);
        return false;
    }

    if (!pPlugin->OnLoad(pm))
    {
        pDestroy(pPlugin);
        ::FreeLibrary(hMod);
        return false;
    }

    LoadedPlugin* pLoaded = new LoadedPlugin;
    pLoaded->hModule = hMod;
    pLoaded->pPlugin = pPlugin;
    pLoaded->info = pPlugin->GetInfo();
    pLoaded->sFilePath = pszDllPath;
    m_plugins.push_back(pLoaded);
    return true;
}

void CPluginManager::UnloadPlugin(LPCTSTR pszName)
{
    for (size_t i = 0; i < m_plugins.size(); ++i)
    {
        if (m_plugins[i]->info.sName.CompareNoCase(pszName) == 0)
        {
            LoadedPlugin* p = m_plugins[i];
            p->pPlugin->OnUnload();
            FN_DestroyPlugin pDestroy = (FN_DestroyPlugin)::GetProcAddress(p->hModule, "DestroyPlugin");
            if (pDestroy) pDestroy(p->pPlugin);
            ::FreeLibrary(p->hModule);
            delete p;
            m_plugins.erase(m_plugins.begin() + i);
            return;
        }
    }
}

void CPluginManager::UnloadAll()
{
    for (size_t i = 0; i < m_plugins.size(); ++i)
    {
        LoadedPlugin* p = m_plugins[i];
        p->pPlugin->OnUnload();
        FN_DestroyPlugin pDestroy = (FN_DestroyPlugin)::GetProcAddress(p->hModule, "DestroyPlugin");
        if (pDestroy) pDestroy(p->pPlugin);
        ::FreeLibrary(p->hModule);
        delete p;
    }
    m_plugins.clear();
}

int CPluginManager::GetPluginCount() const
{
    return (int)m_plugins.size();
}

const LoadedPlugin* CPluginManager::GetPlugin(int index) const
{
    if (index < 0 || index >= (int)m_plugins.size()) return NULL;
    return m_plugins[index];
}

const LoadedPlugin* CPluginManager::FindPlugin(LPCTSTR pszName) const
{
    for (size_t i = 0; i < m_plugins.size(); ++i)
    {
        if (m_plugins[i]->info.sName.CompareNoCase(pszName) == 0)
            return m_plugins[i];
    }
    return NULL;
}

CDuiString CPluginManager::GetAllComponentNames() const
{
    CDuiString result;
    for (size_t i = 0; i < m_plugins.size(); ++i)
    {
        if (!result.IsEmpty()) result += _T("|");
        result += m_plugins[i]->info.sComponents;
    }
    return result;
}

void CPluginManager::RegisterDescription(LPCTSTR pszClassName, LPCTSTR pszDesc)
{
    if (pszClassName && pszDesc)
        m_descriptions[pszClassName] = pszDesc;
}

LPCTSTR CPluginManager::GetComponentDescription(LPCTSTR pszClassName) const
{
    if (!pszClassName) return NULL;
    std::map<CDuiString, CDuiString>::const_iterator it = m_descriptions.find(pszClassName);
    if (it != m_descriptions.end())
        return it->second.GetData();
    return NULL;
}

} // namespace DuiLib
#ifndef __WNDSHADOW_COMPAT_H__
#define __WNDSHADOW_COMPAT_H__

#pragma once

namespace DuiLib
{

class CWndShadow
{
public:
    CWndShadow() : m_hWnd(NULL), m_bHasImage(false) {}
    ~CWndShadow() {}

    static bool Initialize(HINSTANCE /*hInstance*/)
    {
        return true;
    }

    void Create(HWND hWnd)
    {
        m_hWnd = hWnd;
        CStdPtrArray* pPMs = CPaintManagerUI::GetPaintManagers();
        if (pPMs) {
            for (int i = 0; i < pPMs->GetSize(); i++) {
                CPaintManagerUI* pPM = (CPaintManagerUI*)pPMs->GetAt(i);
                if (pPM && pPM->GetPaintWindow() == hWnd) {
                    CShadowUI* pShadow = pPM->GetShadow();
                    if (m_bHasImage) {
                        pShadow->SetImage(m_sImage);
                        pShadow->SetShadowCorner(m_rcCorner);
                    }
                    pShadow->ShowShadow(true);
                    break;
                }
            }
        }
    }

    void SetImage(LPCTSTR szImage, RECT rcCorner, RECT /*rcHole*/)
    {
        m_sImage = szImage;
        m_rcCorner = rcCorner;
        m_bHasImage = true;
        if (m_hWnd) {
            CStdPtrArray* pPMs = CPaintManagerUI::GetPaintManagers();
            if (pPMs) {
                for (int i = 0; i < pPMs->GetSize(); i++) {
                    CPaintManagerUI* pPM = (CPaintManagerUI*)pPMs->GetAt(i);
                    if (pPM && pPM->GetPaintWindow() == m_hWnd) {
                        CShadowUI* pShadow = pPM->GetShadow();
                        pShadow->SetImage(szImage);
                        pShadow->SetShadowCorner(rcCorner);
                        break;
                    }
                }
            }
        }
    }

private:
    HWND m_hWnd;
    bool m_bHasImage;
    CDuiString m_sImage;
    RECT m_rcCorner;
};

}

#endif

/*
 *                 Shell basics
 *
 * Copyright 1998 Marcus Meissner
 * Copyright 1998 Juergen Schmied (jsch)  *  <juergen.schmied@metronet.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include "precomp.h"

#include "shell32_version.h"

WINE_DEFAULT_DEBUG_CHANNEL(shell);

/**************************************************************************
 * Default ClassFactory types
 */
typedef HRESULT (CALLBACK *LPFNCREATEINSTANCE)(IUnknown* pUnkOuter, REFIID riid, LPVOID* ppvObject);
HRESULT IDefClF_fnConstructor(LPFNCREATEINSTANCE lpfnCI, PLONG pcRefDll, const IID *riidInst, IClassFactory **theFactory);


/**************************************************************************
 * Default ClassFactory Implementation
 *
 * SHCreateDefClassObject
 *
 * NOTES
 *  Helper function for dlls without their own classfactory.
 *  A generic classfactory is returned.
 *  When the CreateInstance of the cf is called the callback is executed.
 */

class IDefClFImpl :
    public CComObjectRootEx<CComMultiThreadModelNoCS>,
    public IClassFactory
{
private:
    CLSID                    *rclsid;
    LPFNCREATEINSTANCE        lpfnCI;
    const IID                *riidInst;
    LONG                    *pcRefDll;        /* pointer to refcounter in external dll (ugrrr...) */
public:
    IDefClFImpl();
    HRESULT Initialize(LPFNCREATEINSTANCE lpfnCI, PLONG pcRefDll, const IID *riidInstx);

    // IClassFactory
    virtual HRESULT WINAPI CreateInstance(IUnknown * pUnkOuter, REFIID riid, LPVOID *ppvObject);
    virtual HRESULT WINAPI LockServer(BOOL fLock);

BEGIN_COM_MAP(IDefClFImpl)
    COM_INTERFACE_ENTRY_IID(IID_IClassFactory, IClassFactory)
END_COM_MAP()
};

IDefClFImpl::IDefClFImpl()
{
    lpfnCI = NULL;
    riidInst = NULL;
    pcRefDll = NULL;
    rclsid = NULL;
}

HRESULT IDefClFImpl::Initialize(LPFNCREATEINSTANCE lpfnCIx, PLONG pcRefDllx, const IID *riidInstx)
{
    lpfnCI = lpfnCIx;
    riidInst = riidInstx;
    pcRefDll = pcRefDllx;

    if (pcRefDll)
        InterlockedIncrement(pcRefDll);

    TRACE("(%p)%s\n", this, shdebugstr_guid(riidInst));
    return S_OK;
}

/******************************************************************************
 * IDefClF_fnCreateInstance
 */
HRESULT WINAPI IDefClFImpl::CreateInstance(IUnknown * pUnkOuter, REFIID riid, LPVOID *ppvObject)
{
    TRACE("%p->(%p,%s,%p)\n", this, pUnkOuter, shdebugstr_guid(&riid), ppvObject);

    *ppvObject = NULL;

    if (riidInst == NULL || IsEqualCLSID(riid, *riidInst) || IsEqualCLSID(riid, IID_IUnknown))
    {
        return lpfnCI(pUnkOuter, riid, ppvObject);
    }

    ERR("unknown IID requested %s\n", shdebugstr_guid(&riid));
    return E_NOINTERFACE;
}

/******************************************************************************
 * IDefClF_fnLockServer
 */
HRESULT WINAPI IDefClFImpl::LockServer(BOOL fLock)
{
    TRACE("%p->(0x%x), not implemented\n", this, fLock);
    return E_NOTIMPL;
}

/**************************************************************************
 *  IDefClF_fnConstructor
 */

HRESULT IDefClF_fnConstructor(LPFNCREATEINSTANCE lpfnCI, PLONG pcRefDll, const IID *riidInst, IClassFactory **theFactory)
{
    return ShellObjectCreatorInit<IDefClFImpl>(lpfnCI, pcRefDll, riidInst, IID_IClassFactory, theFactory);
}

/******************************************************************************
 * SHCreateDefClassObject            [SHELL32.70]
 */
HRESULT WINAPI SHCreateDefClassObject(
    REFIID    riid,
    LPVOID*    ppv,
    LPFNCREATEINSTANCE lpfnCI,    /* [in] create instance callback entry */
    LPDWORD    pcRefDll,        /* [in/out] ref count of the dll */
    REFIID    riidInst)        /* [in] optional interface to the instance */
{
    IClassFactory                *pcf;
    HRESULT                        hResult;

    TRACE("%s %p %p %p %s\n", shdebugstr_guid(&riid), ppv, lpfnCI, pcRefDll, shdebugstr_guid(&riidInst));

    if (!IsEqualCLSID(riid, IID_IClassFactory))
        return E_NOINTERFACE;
    hResult = IDefClF_fnConstructor(lpfnCI, (PLONG)pcRefDll, &riidInst, &pcf);
    if (FAILED(hResult))
        return hResult;
    *ppv = pcf;
    return S_OK;
}

class CShell32Module : public CComModule
{
public:
};


BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_ShellFSFolder, CFSFolder)
    OBJECT_ENTRY(CLSID_MyComputer, CDrivesFolder)
    OBJECT_ENTRY(CLSID_ShellDesktop, CDesktopFolder)
    OBJECT_ENTRY(CLSID_ShellItem, CShellItem)
    OBJECT_ENTRY(CLSID_ShellLink, CShellLink)
    OBJECT_ENTRY(CLSID_DragDropHelper, CDropTargetHelper)
    OBJECT_ENTRY(CLSID_ControlPanel, CControlPanelFolder)
    OBJECT_ENTRY(CLSID_MyDocuments, CMyDocsFolder)
    OBJECT_ENTRY(CLSID_NetworkPlaces, CNetFolder)
    OBJECT_ENTRY(CLSID_FontsFolderShortcut, CFontsFolder)
    OBJECT_ENTRY(CLSID_Printers, CPrinterFolder)
    OBJECT_ENTRY(CLSID_AdminFolderShortcut, CAdminToolsFolder)
    OBJECT_ENTRY(CLSID_RecycleBin, CRecycleBin)
    OBJECT_ENTRY(CLSID_OpenWithMenu, COpenWithMenu)
    OBJECT_ENTRY(CLSID_NewMenu, CNewMenu)
    OBJECT_ENTRY(CLSID_StartMenu, CStartMenu)
    OBJECT_ENTRY(CLSID_MenuBandSite, CMenuBandSite)
    OBJECT_ENTRY(CLSID_MenuBand, CMenuBand)
    OBJECT_ENTRY(CLSID_MenuDeskBar, CMenuDeskBar)
    OBJECT_ENTRY(CLSID_ExeDropHandler, CExeDropHandler)
END_OBJECT_MAP()

CShell32Module                                gModule;


/***********************************************************************
 * DllGetVersion [SHELL32.@]
 *
 * Retrieves version information of the 'SHELL32.DLL'
 *
 * PARAMS
 *     pdvi [O] pointer to version information structure.
 *
 * RETURNS
 *     Success: S_OK
 *     Failure: E_INVALIDARG
 *
 * NOTES
 *     Returns version of a shell32.dll from IE4.01 SP1.
 */

STDAPI DllGetVersion(DLLVERSIONINFO *pdvi)
{
    /* FIXME: shouldn't these values come from the version resource? */
    if (pdvi->cbSize == sizeof(DLLVERSIONINFO) ||
        pdvi->cbSize == sizeof(DLLVERSIONINFO2))
    {
        pdvi->dwMajorVersion = WINE_FILEVERSION_MAJOR;
        pdvi->dwMinorVersion = WINE_FILEVERSION_MINOR;
        pdvi->dwBuildNumber = WINE_FILEVERSION_BUILD;
        pdvi->dwPlatformID = WINE_FILEVERSION_PLATFORMID;
        if (pdvi->cbSize == sizeof(DLLVERSIONINFO2))
        {
            DLLVERSIONINFO2 *pdvi2 = (DLLVERSIONINFO2 *)pdvi;

            pdvi2->dwFlags = 0;
            pdvi2->ullVersion = MAKEDLLVERULL(WINE_FILEVERSION_MAJOR,
                                              WINE_FILEVERSION_MINOR,
                                              WINE_FILEVERSION_BUILD,
                                              WINE_FILEVERSION_PLATFORMID);
        }
        TRACE("%u.%u.%u.%u\n",
              pdvi->dwMajorVersion, pdvi->dwMinorVersion,
              pdvi->dwBuildNumber, pdvi->dwPlatformID);
        return S_OK;
    }
    else
    {
        WARN("wrong DLLVERSIONINFO size from app\n");
        return E_INVALIDARG;
    }
}

/*************************************************************************
 * global variables of the shell32.dll
 * all are once per process
 *
 */
HINSTANCE    shell32_hInstance;

void *operator new (size_t, void *buf)
{
    return buf;
}

/*************************************************************************
 * SHELL32 DllMain
 *
 * NOTES
 *  calling oleinitialize here breaks sone apps.
 */
STDAPI_(BOOL) DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID fImpLoad)
{
    TRACE("%p 0x%x %p\n", hInstance, dwReason, fImpLoad);
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        /* HACK - the global constructors don't run, so I placement new them here */
        new (&gModule) CShell32Module;
        new (&_AtlWinModule) CAtlWinModule;
        new (&_AtlBaseModule) CAtlBaseModule;
        new (&_AtlComModule) CAtlComModule;

        shell32_hInstance = hInstance;
        gModule.Init(ObjectMap, hInstance, NULL);

        DisableThreadLibraryCalls (hInstance);

        /* get full path to this DLL for IExtractIconW_fnGetIconLocation() */
        GetModuleFileNameW(hInstance, swShell32Name, MAX_PATH);
        swShell32Name[MAX_PATH - 1] = '\0';

        /* Initialize comctl32 */
        INITCOMMONCONTROLSEX InitCtrls;
        InitCtrls.dwSize = sizeof(INITCOMMONCONTROLSEX);
        InitCtrls.dwICC = ICC_WIN95_CLASSES | ICC_DATE_CLASSES | ICC_USEREX_CLASSES;
        InitCommonControlsEx(&InitCtrls);

        SIC_Initialize();
        InitChangeNotifications();
        InitIconOverlays();
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        shell32_hInstance = NULL;
        SIC_Destroy();
        FreeChangeNotifications();
        gModule.Term();
    }
    return TRUE;
}

/***********************************************************************
 *              DllCanUnloadNow (SHELL32.@)
 */
STDAPI DllCanUnloadNow()
{
    return gModule.DllCanUnloadNow();
}

/*************************************************************************
 *              DllGetClassObject     [SHELL32.@]
 *              SHDllGetClassObject   [SHELL32.128]
 */
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppv)
{
    HRESULT                                hResult;

    TRACE("CLSID:%s,IID:%s\n", shdebugstr_guid(&rclsid), shdebugstr_guid(&riid));

    hResult = gModule.DllGetClassObject(rclsid, riid, ppv);
    TRACE("-- pointer to class factory: %p\n", *ppv);
    return hResult;
}

/***********************************************************************
 *              DllRegisterServer (SHELL32.@)
 */
STDAPI DllRegisterServer()
{
    HRESULT hr;

    hr = gModule.DllRegisterServer(FALSE);
    if (FAILED(hr))
        return hr;

    hr = gModule.UpdateRegistryFromResource(IDR_FOLDEROPTIONS, TRUE, NULL);
    if (FAILED(hr))
        return hr;

    hr = SHELL_RegisterShellFolders();
    if (FAILED(hr))
        return hr;

    return S_OK;
}

/***********************************************************************
 *              DllUnregisterServer (SHELL32.@)
 */
STDAPI DllUnregisterServer()
{
    HRESULT hr;

    hr = gModule.DllUnregisterServer(FALSE);
    if (FAILED(hr))
        return hr;

    hr = gModule.UpdateRegistryFromResource(IDR_FOLDEROPTIONS, FALSE, NULL);
    if (FAILED(hr))
        return hr;

    return S_OK;
}

/*************************************************************************
 * DllInstall         [SHELL32.@]
 *
 * PARAMETERS
 *
 *    BOOL bInstall - TRUE for install, FALSE for uninstall
 *    LPCWSTR pszCmdLine - command line (unused by shell32?)
 */

HRESULT WINAPI DllInstall(BOOL bInstall, LPCWSTR cmdline)
{
    FIXME("%s %s: stub\n", bInstall ? "TRUE":"FALSE", debugstr_w(cmdline));
    return S_OK;        /* indicate success */
}

#include <windows.h>
#include <commctrl.h>
#include <prsht.h>
#include <aclui.h>
#include <sddl.h>
#include <ntsecapi.h>
#include "resource.h"

ULONG DbgPrint(PCH Format,...);
#define DPRINT DbgPrint

#ifndef NT_SUCCESS
#define NT_SUCCESS(status) ((LONG)(status) >= 0)
#endif

#define EnableRedrawWindow(hwnd) \
    SendMessage((hwnd), WM_SETREDRAW, TRUE, 0)

#define DisableRedrawWindow(hwnd) \
    SendMessage((hwnd), WM_SETREDRAW, FALSE, 0)

extern HINSTANCE hDllInstance;

typedef struct _ACE_LISTITEM
{
    struct _ACE_LISTITEM *Next;
    SID_NAME_USE SidNameUse;
    WCHAR *DisplayString;
    WCHAR *AccountName;
    WCHAR *DomainName;
} ACE_LISTITEM, *PACE_LISTITEM;

typedef struct _SECURITY_PAGE
{
    HWND hWnd;
    HWND hBtnRemove;
    HWND hAceCheckList;

    /* Main ACE List */
    HWND hWndAceList;
    PACE_LISTITEM AceListHead;

    HIMAGELIST hiUsrs;

    LPSECURITYINFO psi;
    SI_OBJECT_INFO ObjectInfo;
} SECURITY_PAGE, *PSECURITY_PAGE;

/* MISC ***********************************************************************/

BOOL
OpenLSAPolicyHandle(IN WCHAR *SystemName,
                    IN ACCESS_MASK DesiredAccess,
                    OUT PLSA_HANDLE PolicyHandle);

DWORD
LoadAndFormatString(IN HINSTANCE hInstance,
                    IN UINT uID,
                    OUT LPWSTR *lpTarget,
                    ...);

LPARAM
ListViewGetSelectedItemData(IN HWND hwnd);

BOOL
ListViewSelectItem(IN HWND hwnd,
                   IN INT Index);

/* CHECKLIST CONTROL **********************************************************/

#define CIS_DISABLED    (0x2)
#define CIS_ENABLED     (0x0)
#define CIS_ALLOW       (0x1)
#define CIS_DENY        (0x0)

#define CLB_ALLOW       (0x1)
#define CLB_DENY        (0x0)

#define CIS_MASK (CIS_DISABLED | CIS_ALLOW | CIS_DENY)

#define CLM_ADDITEM     (WM_USER + 1)
#define CLM_DELITEM     (WM_USER + 2)
#define CLM_GETITEMCOUNT        (WM_USER + 3)
#define CLM_CLEAR       (WM_USER + 4)
#define CLM_SETCHECKBOXCOLUMN   (WM_USER + 5)
#define CLM_GETCHECKBOXCOLUMN   (WM_USER + 6)

BOOL
RegisterCheckListControl(HINSTANCE hInstance);

VOID
UnregisterCheckListControl(VOID);

/* EOF */

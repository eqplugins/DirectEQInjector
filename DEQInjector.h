//tray.h

#ifndef _TRAY_H_ //inclusion guard
#define _TRAY_H_
#define _WIN32_WINNT 0x0501
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <windowsx.h>
#include <shellapi.h>

#ifndef RC_INVOKED //variable definitions and function prototypes just confuse the resource compiler

void minimize(HWND hwnd);
void restore(HWND hwnd);

bool minimized = false;
const int TASKBARCREATED = RegisterWindowMessage("TaskbarCreated");

#endif // 8: #ifndef RC_INVOKED

#define IDI_ICON        0
#define IDC_MINIMIZE    1
#define IDI_TRAYICON    2
#define IDM_TRAYEXIT    3
#define IDM_TRAYHISTORY    4
#define IDM_TRAYABOUT   5
#define IDM_TRAYDONATE   6
#define IDM_TRAYHOMEPAGE   7
#define MSG_MINTRAYICON (WM_USER+0)


#endif // 1: #ifndef _TRAY_H_



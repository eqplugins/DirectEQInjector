#define _CRT_SECURE_NO_DEPRECATE 1
#include "DEQInjector.h"
#include <stdio.h>
#include "resource.h"
#include <TlHelp32.h>
#include "url.h"
#include "Wininet.h"
#include <set>
using namespace std;

set<DWORD> processes;
float dllversion = 0.0f;

bool AlreadyOpened(DWORD process) {
	if (processes.find(process)!= processes.end())
	{
		return true;  
	}
	else
	{
		processes.insert(process);
		return false;
	}
}

HANDLE GetProcessHandle(LPSTR szExeName)
{
	PROCESSENTRY32 Pc = { sizeof(PROCESSENTRY32) } ;
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);
	if(Process32First(hSnapshot, &Pc)){
		do{
			if(!strcmp(Pc.szExeFile, szExeName)) {
				if (!AlreadyOpened(Pc.th32ProcessID)) {
					CloseHandle(hSnapshot);
					return OpenProcess(PROCESS_ALL_ACCESS, TRUE, Pc.th32ProcessID);
				}
			}
		} while(Process32Next(hSnapshot, &Pc));
	}
	CloseHandle(hSnapshot);
	return NULL;
}

void EnableDebugPriv()
{
	HANDLE hToken;
	LUID sedebugnameValue;
	TOKEN_PRIVILEGES tkp;
	OutputDebugString("DEQ: Requesting debug priviliges.\n");
	// enable the SeDebugPrivilege
	if ( ! OpenProcessToken( GetCurrentProcess(),
		TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken ) )
		return;

	if ( ! LookupPrivilegeValue( NULL, SE_DEBUG_NAME, &sedebugnameValue ) )
	{
		OutputDebugString("DEQ: Had debug priviliges by default.\n");
		CloseHandle( hToken );
		return;
	}

	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Luid = sedebugnameValue;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	if (AdjustTokenPrivileges( hToken, FALSE, &tkp, sizeof tkp, NULL, NULL )) {
		OutputDebugString("DEQ: Got debug priviliges.\n");
	} else {
		OutputDebugString("DEQ: Couldn't get debug priviliges.\n");
	}
		
	CloseHandle( hToken );
}

void DllInject(HANDLE hProcess, LPSTR lpszDllPath)
{
	HMODULE hmKernel = GetModuleHandle("Kernel32");//heres the DLL
	if(hmKernel == NULL || hProcess == NULL) OutputDebugString("DEQ: Can't get proper handles!\n");
	int nPathLen = lstrlen(lpszDllPath) + 1;
	LPVOID lpvMem = VirtualAllocEx(hProcess, NULL, nPathLen, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
	WriteProcessMemory(hProcess, lpvMem, lpszDllPath, nPathLen, NULL);
	DWORD dwWaitResult, dwExitResult = 0;
	HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)GetProcAddress(hmKernel, "LoadLibraryA"), lpvMem, 0, NULL);
	if(hThread != NULL){
		OutputDebugString("DEQ: CreateRemoteThread SUCCESS!\n");
		dwWaitResult = WaitForSingleObject(hThread, INFINITE);
		GetExitCodeThread(hThread, &dwExitResult);
		CloseHandle(hThread);
		char temp[100];
		sprintf_s(temp,"EXITCODE=%d\n",dwExitResult);
		OutputDebugString(temp);
	} else {
		OutputDebugString("DEQ: CreateRemoteThread FAILED!\n");
	}
	VirtualFreeEx(hProcess, lpvMem, 0, MEM_RELEASE);
}

/*  Declare Windows procedure  */
LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);

/*  Make the class name into a global variable  */
char szClassName[ ] = "TrayMinimizerClass__";

int WINAPI WinMain (HINSTANCE hThisInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR lpszArgument,
                    int nFunsterStil)

{
	HWND hwnd;               /* This is the handle for our window */
	MSG messages;            /* Here messages to the application are saved */
	WNDCLASSEX wincl;        /* Data structure for the windowclass */

	/* The Window structure */
	wincl.hInstance = hThisInstance;
	wincl.lpszClassName = szClassName;
	wincl.lpfnWndProc = WindowProcedure;      /* This function is called by windows */
	wincl.style = CS_DBLCLKS;                 /* Catch double-clicks */
	wincl.cbSize = sizeof (WNDCLASSEX);

	/* Use default icon and mouse-pointer */
	wincl.hIcon = LoadIcon (NULL, IDI_APPLICATION);
	wincl.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
	wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
	wincl.lpszMenuName = NULL;                 /* No menu */
	wincl.cbClsExtra = 0;                      /* No extra bytes after the window class */
	wincl.cbWndExtra = 0;                      /* structure or the window instance */
	/* Use Windows's default color as the background of the window */
	wincl.hbrBackground = (HBRUSH) COLOR_BACKGROUND;

	/* Register the window class, and if it fails quit the program */
	if (!RegisterClassEx (&wincl))
		return 0;

	/* The class is registered, let's create the program*/
	hwnd = CreateWindowEx(
		0,                   /* Extended possibilites for variation */
		szClassName,         /* Classname */
		"DirectEQ",       /* Title Text */
		SW_SHOWDEFAULT , /* default window */
		CW_USEDEFAULT,       /* Windows decides the position */
		CW_USEDEFAULT,       /* where the window ends up on the screen */
		0,                 /* The programs width */
		0,                 /* and height in pixels */
		HWND_DESKTOP,        /* The window is a child-window to desktop */
		NULL,                /* No menu */
		hThisInstance,       /* Program Instance handler */
		NULL                 /* No Window Creation data */
		);

	/* Make the window visible on the screen */
	//ShowWindow (hwnd, nFunsterStil);
	//minimize(hwnd);
	//minimized = true;
	/* Run the message loop. It will run until GetMessage() returns 0 */
	HANDLE hProcess;
	char dir[256];
	EnableDebugPriv();
	HMODULE hDirectEQ = LoadLibrary("DirectEq.dll");
	if (hDirectEQ) {
		//OutputDebugString("SUCCESS\n");
		if (!GetProcAddress(hDirectEQ, "DEQGetVersion")) 
		{
			MessageBox(NULL,"DirectEQ - File DirectEQ.dll is corrupt.","DirectEQ",MB_OK);
			exit(1);
			//OutputDebugString("Failed!\r\n");
		} else {
			typedef float (*_DEQGetVersion)();
			_DEQGetVersion DEQGetVersion;
			DEQGetVersion = (_DEQGetVersion)GetProcAddress(hDirectEQ, "DEQGetVersion");
			dllversion = DEQGetVersion();
		}
		FreeLibrary(hDirectEQ);
	} else
	{
		MessageBox(NULL,"DirectEQ - File DirectEQ.dll is missing.","DirectEQ",MB_OK);
		exit(1);
		//OutputDebugString("FAILED\n");
	}

	DWORD dwType;

	if (!InternetGetConnectedState(&dwType,0))
	{
		// Don't attempt connection or it will bring up the dialog
	}
	else
	{
		HINTERNET hInternet;
		HINTERNET hSession;
		hInternet = InternetOpen( "DirectEQ", INTERNET_OPEN_TYPE_DIRECT, 0, 0, 0); 
		DWORD options = INTERNET_FLAG_NEED_FILE|INTERNET_FLAG_HYPERLINK|INTERNET_FLAG_RESYNCHRONIZE|INTERNET_FLAG_RELOAD;
		hSession = InternetOpenUrl(hInternet,"http://www.sitelonggone.com/odessa/directeq/deq.cfg", NULL, NULL, options, 0);

		//Attempt connection
		if (!hSession)
		{
			// Call failed
			DWORD err = GetLastError();
			if ((err == ERROR_INTERNET_NAME_NOT_RESOLVED) ||
				(err == ERROR_INTERNET_CANNOT_CONNECT) ||
				(err == ERROR_INTERNET_TIMEOUT))
			{
				// probably not connected...handle appropriately
			}
		} else {
			// We're connected!!!
			char buffer[10]={0};
			DWORD read;
			InternetReadFile(hSession,buffer,sizeof(buffer),&read);
			//OutputDebugString(buffer);
			if ((atof(buffer)-dllversion)>0.0001) {
					char temp[128];
					sprintf_s(temp,"New version of DirectEQ (%s) is available at http://www.sitelonggone.com/odessa/directeq",buffer);
					MessageBox(hwnd,temp,"DirectEQ",MB_OK);
			}
			InternetCloseHandle(hSession);
			InternetCloseHandle(hInternet);
		}
	}

	minimize(hwnd);
	PeekMessage( &messages, NULL, 0U, 0U, PM_REMOVE );
	while( messages.message!=WM_QUIT )
	{
		if(PeekMessage( &messages, NULL, 0U, 0U, PM_REMOVE ))
		{
			TranslateMessage( &messages );
			DispatchMessage( &messages );
		}
		else
		{
			hProcess = GetProcessHandle("eqgame.exe");
			if (hProcess){
				//OutputDebugString("INJECTING DirectEQ\n");
				GetCurrentDirectory(256,dir);				   
				DllInject(hProcess, strcat(dir,"\\DirectEq.dll"));
			}
			Sleep(10);
		}
	}

	/* The program return-value is 0 - The value that PostQuitMessage() gave */
	return messages.wParam;
}


/*  This function is called by the Windows function DispatchMessage()  */

LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{ 
    switch (message)                  /* handle the messages */
    {
        case MSG_MINTRAYICON:
        {

            if (lParam == WM_RBUTTONUP) //time to display a menu.
            {
                SetForegroundWindow(hwnd);
                HMENU myMenu = NULL;
                myMenu = CreatePopupMenu(); //create our menu. You'll want to error-check this, because if it fails the next few functions may produce segmentation faults, and your menu won't work.
				AppendMenu(myMenu, MF_STRING, IDM_TRAYDONATE, "Donate");
				AppendMenu(myMenu, MF_STRING, IDM_TRAYHOMEPAGE, "Homepage");
                AppendMenu(myMenu, MF_STRING, IDM_TRAYHISTORY, "History");
				AppendMenu(myMenu, MF_STRING, IDM_TRAYABOUT, "About");
                AppendMenu(myMenu, MF_STRING, IDM_TRAYEXIT, "Exit");

				POINT pt;
				GetCursorPos(&pt);
				UINT clicked = TrackPopupMenu(myMenu, TPM_RETURNCMD | TPM_NONOTIFY | TPM_LEFTBUTTON, pt.x,pt.y, 0, hwnd, NULL); //display the menu. you MUST #include <windowsx.h> to use those two macros.
                SendMessage(hwnd, WM_NULL, 0, 0); //send benign message to window to make sure the menu goes away.

				if (clicked == IDM_TRAYEXIT) {
					restore(hwnd);
					SendMessage(hwnd, WM_DESTROY, 0, 0);
				}
				else if (clicked == IDM_TRAYHISTORY) {
					char temp[4096]="Injection History:\n";
					set<DWORD>::iterator processes_iter = processes.begin();
					while(processes_iter != processes.end()) 
					{
						sprintf_s(temp,"%seqgame.exe - %d\n",temp,*processes_iter);
						++processes_iter; 
					} 
					MessageBox(NULL,temp,"DirectEQ",MB_OK);
				}
				else if (clicked == IDM_TRAYABOUT) {
					char temp[4096];
					sprintf_s(temp,"DirectEQ %1.3f\nBy Odessa (eqplugins@gmail.com)",dllversion);
					MessageBox(NULL,temp,"DirectEQ",MB_OK);

				}
				else if (clicked == IDM_TRAYDONATE) {
					CURL url;
					url.Open("");
				} else if (clicked == IDM_TRAYHOMEPAGE) {
					CURL url;
					url.Open("http://www.sitelonggone.com/odessa/directeq");
				}
            }
        }
            break;
        case WM_DESTROY:
            PostQuitMessage (0);       /* send a WM_QUIT to the message queue */
            break;
        default:                      /* for messages that we don't deal with */
            return DefWindowProc (hwnd, message, wParam, lParam);
    }

    return 0;
}

void minimize(HWND hwnd)
{
    NOTIFYICONDATA nid = { 0 };
    nid.cbSize = sizeof(NOTIFYICONDATA); //this helps the OS determine stuff. (I have no idea, but it is necessary.
    nid.hWnd = hwnd; //the hWnd and uID members allow the OS to uniquely identify your icon. One window (the hWnd) can have more than one icon, as long as they have unique uIDs.
    nid.uID = IDI_TRAYICON; //sorry, had forgotten this in my original example. but without, the function probably wouldn't work
    nid.uFlags = //some flags that determine the tray's behavior:
        NIF_ICON //we're adding an icon
        | NIF_MESSAGE //we want the tray to send a message to the window identified by hWnd when something happens to our icon (see uCallbackMesage member below).
        | NIF_TIP; //our icon has a tooltip.
    nid.uCallbackMessage = MSG_MINTRAYICON; //this message must be handled in hwnd's window procedure. more info below.
    nid.hIcon = (HICON)LoadImage( //load up the icon:
        GetModuleHandle(NULL), //get the HINSTANCE to this program
        MAKEINTRESOURCE(IDI_ICON1), //grab the icon out of our resource file
        IMAGE_ICON, //tells the versatile LoadImage function that we are loading an icon
        32, 32, //x and y values. we want a 16x16-pixel icon for the tray.
        0); //no flags necessary. these flags specify special behavior, such as loading the icon from a file instead of a resource. see source list below for MSDN docs on LoadImage.
	char temp[64];
	sprintf_s(temp,"DirectEQ %1.3f",dllversion);
    strcpy_s(nid.szTip, temp); //this string cannot be longer than 64 characters including the NULL terminator (which is added by default to string literals).
    //There are some more members of the NOTIFYICONDATA struct that are for advanced features we aren't using. See sources below for MSDN docs if you want to use balloon tips (only Win2000/XP).
    Shell_NotifyIcon(NIM_ADD, &nid);
    //ShowWindow(hwnd, SW_HIDE);
    minimized = true;
}

void restore(HWND hwnd)
{
    NOTIFYICONDATA nid = { 0 };
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.uID = IDI_TRAYICON;
    Shell_NotifyIcon(NIM_DELETE, &nid);
    //ShowWindow(hwnd, SW_SHOW);
  //  minimized = false;
}

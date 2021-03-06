// Cheese.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "Cheese.h"
#include <math.h>

#pragma data_seg(".SHARE")
HWND hWndMain = NULL;
#pragma data_seg()
#pragma comment(linker, "/section:.SHARE,rws")

HINSTANCE hInstance;
HHOOK hook;

// Forward declaration
static LRESULT CALLBACK msghook(int nCode, WPARAM wParam, LPARAM lParam);

/****************************************************************
*                               DllMain
* Inputs:
*       HINSTANCE hInst: Instance handle for the DLL
*       DWORD Reason: Reason for call
*       LPVOID reserved: ignored
* Result: BOOL
*       TRUE if successful
*       FALSE if there was an error (never returned)
* Effect:
*       Initializes the DLL.
****************************************************************/

BOOL APIENTRY DllMain(HINSTANCE hInst, DWORD Reason, LPVOID reserved)
{
	switch (Reason)
	{
	case DLL_PROCESS_ATTACH:
		// Save the instance handle because we need it to set the hook later
		hInstance = hInst;
		break;
	case DLL_PROCESS_DETACH:
		// If the server has not unhooked the hook, unhook it as we unload
		if (hWndMain != NULL)
			unHook(hWndMain);
		break;
	}
	return TRUE;
}

/****************************************************************
*                               setMyHook
* Inputs:
*       HWND hWnd: Window whose hook is to be set
* Result: BOOL
*       TRUE if the hook is properly set
*       FALSE if there was an error, such as the hook already
*             being set
* Effect:
*       Sets the hook for the specified window.
*       This sets a message-intercept hook (WH_GETMESSAGE)
*       If the setting is successful, the hWnd is set as the
*       server window.
****************************************************************/

__declspec(dllexport) BOOL setHook(HWND hWnd)
{
	if (hWndMain != NULL)
		return FALSE; //  already hooked
	hook = SetWindowsHookEx(
		WH_MOUSE,
		(HOOKPROC)msghook,
		hInstance,
		0);
	if (hook != NULL)
	{ // success
		hWndMain = hWnd;
		return TRUE;
	}
	return FALSE; // failed to set hook
}

/****************************************************************
*                             clearMyHook
* Inputs:
*       HWND hWnd: Window whose hook is to be cleared
* Result: BOOL
*       TRUE if the hook is properly unhooked
*       FALSE if you gave the wrong parameter
* Effect:
*       Removes the hook that has been set.
****************************************************************/
__declspec(dllexport) BOOL unHook(HWND hWnd)
{
	if (hWnd != hWndMain || hWnd == NULL)
		return FALSE;
	BOOL unhooked = UnhookWindowsHookEx(hook);
	if (unhooked) {
		hWndMain = NULL;
	}
	return unhooked;
}

/****************************************************************
*                              msghook
* Inputs:
*       int nCode: Code value
*       WPARAM wParam: parameter
*       LPARAM lParam: parameter
* Result: LRESULT
*
* Effect:
*       If the message is a mouse-move message, posts it back to
*       the server window with the mouse coordinates
* Notes:
*       This must be a CALLBACK function or it will not work!
****************************************************************/

static LRESULT CALLBACK msghook(int nCode, WPARAM wParam, LPARAM lParam)
{
	// If the value of nCode is < 0, just pass it on and return 0
	// this is required by the specification of hook handlers
	if (nCode < 0)
	{
		CallNextHookEx(hook, nCode, wParam, lParam);
		return 0;
	}

	// If it is a mouse-move message, either in the client area or
	// the non-client area, we want to notify the parent that it has
	// occurred. Note the use of PostMessage instead of SendMessage
	if (wParam == WM_MOUSEMOVE ||
		wParam == WM_NCMOUSEMOVE) {
		PostMessage(hWndMain,
			WH_MOUSE,
			0, 0);
	}

	// Pass the message on to the next hook
	return CallNextHookEx(hook, nCode, wParam, lParam);
}
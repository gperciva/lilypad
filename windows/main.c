/* -*-c-style:stroustrup-*-
 *  LilyPad
 *
 *  Copyright 2000 Mike McCormack <Mike_McCormack@looksmart.com.au>
 *  Copyright 1997,98 Marcel Baur <mbaur@g26.ethz.ch>
 *  Copyright 2002 Sylvain Petreolle <spetreolle@yahoo.fr>
 *  Copyright 2002 Andriy Palamarchuk
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
 * Foundation, Inc., 51 Franklin St, Suite 500, Boston, MA 02110-1335, USA
 *
 */

/* lilypond supports Windows 2000 and higher. */
#ifndef WINVER
#define WINVER 0x500
#endif

#include <windows.h>
#include <shlwapi.h>
#include <stdio.h>

#include "main.h"
#include "dialog.h"
#include "pmdpi.h"
#include "lilypad_res.h"

LILYPAD_GLOBALS Globals;
static ATOM aFINDMSGSTRING;

/***********************************************************************
 *
 *           SetFileName
 *
 *  Sets Global File Name.
 */
VOID SetFileName(__LPCWSTR szFileName)
{
    lstrcpy(Globals.szFileName, szFileName);
    Globals.szFileTitle[0] = 0;
    GetFileTitle(szFileName, Globals.szFileTitle, sizeof(Globals.szFileTitle));
}

/***********************************************************************
 *
 *           LILYPAD_InitFont
 *
 *  Initialize font for the edit window
 */
static VOID LILYPAD_InitFont()
{
    LOGFONT *lf = &Globals.lfFont;
    HDC hdc;
    TCHAR szBuff[MAX_STRING_LEN];

    if (LoadString(Globals.hInstance, STRING_FONT_SIZE,
		   szBuff, sizeof(szBuff)/sizeof(szBuff[0])))
        Globals.iPointSize = StrToInt(szBuff);
    else
        Globals.iPointSize = 10 * 10;  /* default font size is 10pt */

    hdc=GetDC(Globals.hMainWnd);
    Globals.wDPI = GetDeviceCaps(hdc, LOGPIXELSY);
    ReleaseDC(Globals.hMainWnd, hdc);

    lf->lfHeight        = -MulDiv(Globals.iPointSize,
				  Globals.wDPI,
				  72 * 10);  /* 72pt = 1inch */

    lf->lfWidth         = 0;
    lf->lfEscapement    = 0;
    lf->lfOrientation   = 0;

    if (LoadString(Globals.hInstance, STRING_FONT_WEIGHT,
		   szBuff, sizeof(szBuff)/sizeof(szBuff[0])))
        lf->lfWeight    = StrToInt(szBuff);
    else
        lf->lfWeight    = FW_NORMAL;

    if (LoadString(Globals.hInstance, STRING_FONT_ITALIC,
		   szBuff, sizeof(szBuff)/sizeof(szBuff[0])))
        lf->lfItalic    = StrToInt(szBuff);
    else
        lf->lfItalic    = FALSE;

    lf->lfUnderline     = FALSE;
    lf->lfStrikeOut     = FALSE;

    if (LoadString(Globals.hInstance, STRING_FONT_CHARSET,
		   szBuff, sizeof(szBuff)/sizeof(szBuff[0])))
        lf->lfCharSet   = StrToInt(szBuff);
    else
        lf->lfCharSet   = DEFAULT_CHARSET;

    lf->lfOutPrecision  = OUT_DEFAULT_PRECIS;
    lf->lfClipPrecision = CLIP_DEFAULT_PRECIS;
    lf->lfQuality       = DEFAULT_QUALITY;

    if (LoadString(Globals.hInstance, STRING_FONT_PITCHANDFAMILY,
		   szBuff, sizeof(szBuff)/sizeof(szBuff[0])))
    {
        int i;
        StrToIntEx(szBuff, STIF_SUPPORT_HEX, &i);
        lf->lfPitchAndFamily = i;
    }
    else
        lf->lfPitchAndFamily = FIXED_PITCH | FF_DONTCARE;

    if (!LoadString(Globals.hInstance, STRING_FONT_FACENAME, lf->lfFaceName,
		    sizeof(lf->lfFaceName)/sizeof(lf->lfFaceName[0])))
        lf->lfFaceName[0] = 0;

    Globals.hFont = CreateFontIndirect(lf);
    SendMessage(Globals.hEdit, WM_SETFONT, (WPARAM)Globals.hFont, (LPARAM)FALSE);
}

/***********************************************************************
 *
 *           LILYPAD_MenuCommand
 *
 *  All handling of main menu events
 */
static int LILYPAD_MenuCommand(WPARAM wParam)
{
    switch (wParam)
    {
    case CMD_NEW:               DIALOG_FileNew(); break;
    case CMD_OPEN:              DIALOG_FileOpen(); break;
    case CMD_SAVE:              DIALOG_FileSave(); break;
    case CMD_SAVE_AS:           DIALOG_FileSaveAs(); break;
    case CMD_PRINT:             DIALOG_FilePrint(); break;
    case CMD_PAGE_SETUP:        DIALOG_FilePageSetup(); break;
    case CMD_EXIT:              DIALOG_FileExit(); break;

    case CMD_UNDO:             DIALOG_EditUndo(); break;
    case CMD_CUT:              DIALOG_EditCut(); break;
    case CMD_COPY:             DIALOG_EditCopy(); break;
    case CMD_PASTE:            DIALOG_EditPaste(); break;
    case CMD_DELETE:           DIALOG_EditDelete(); break;
    case CMD_SELECT_ALL:       DIALOG_EditSelectAll(); break;
    case CMD_TIME_DATE:        DIALOG_EditTimeDate();break;

    case CMD_SEARCH:           DIALOG_Search(); break;
    case CMD_SEARCH_NEXT:      DIALOG_SearchNext(); break;

    case CMD_WRAP:             DIALOG_EditWrap(); break;
    case CMD_FONT:             DIALOG_SelectFont(); break;

    case CMD_HELP_ABOUT_LILYPAD: DIALOG_HelpAboutLilyPad(); break;

    default:
	break;
    }
   return 0;
}

/***********************************************************************
 * Data Initialization
 */
static VOID LILYPAD_InitData(VOID)
{
    __LPWSTR p = Globals.szFilter;
    static const __WCHAR ly_files[] = { '*','.','l','y',0 };
    static const __WCHAR all_files[] = { '*','.','*',0 };

    LoadString(Globals.hInstance, STRING_LILYPOND_FILES_LY, p, MAX_STRING_LEN);
    p += lstrlen(p) + 1;
    lstrcpy(p, ly_files);
    p += lstrlen(p) + 1;
    LoadString(Globals.hInstance, STRING_ALL_FILES, p, MAX_STRING_LEN);
    p += lstrlen(p) + 1;
    lstrcpy(p, all_files);
    p += lstrlen(p) + 1;
    *p = '\0';
    Globals.hDevMode = NULL;
    Globals.hDevNames = NULL;
    Globals.MarginFlags = 0;

    CheckMenuItem(GetMenu(Globals.hMainWnd), CMD_WRAP,
            MF_BYCOMMAND | (Globals.bWrapLongLines ? MF_CHECKED : MF_UNCHECKED));
}

/***********************************************************************
 * Enable/disable items on the menu based on control state
 */
static VOID LILYPAD_InitMenuPopup(HMENU menu, int index)
{
    int enable;

    EnableMenuItem(menu, CMD_UNDO,
        SendMessage(Globals.hEdit, EM_CANUNDO, 0, 0) ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(menu, CMD_PASTE,
        IsClipboardFormatAvailable(CF_TEXT) ? MF_ENABLED : MF_GRAYED);
    enable = SendMessage(Globals.hEdit, EM_GETSEL, 0, 0);
    enable = (HIWORD(enable) == LOWORD(enable)) ? MF_GRAYED : MF_ENABLED;
    EnableMenuItem(menu, CMD_CUT, enable);
    EnableMenuItem(menu, CMD_COPY, enable);
    EnableMenuItem(menu, CMD_DELETE, enable);

    EnableMenuItem(menu, CMD_SELECT_ALL,
        GetWindowTextLength(Globals.hEdit) ? MF_ENABLED : MF_GRAYED);
}

/***********************************************************************
 *
 *           LILYPAD_WndProc
 */
static LRESULT WINAPI LILYPAD_WndProc(HWND hWnd, UINT msg, WPARAM wParam,
                               LPARAM lParam)
{
    if (msg == aFINDMSGSTRING)
    {
	FINDREPLACE *fr = (FINDREPLACE *)lParam;

	if (fr->Flags & FR_DIALOGTERM)
	    Globals.hFindReplaceDlg = NULL;
	if (fr->Flags & FR_FINDNEXT)
	{
	    Globals.lastFind = *fr;
	    LILYPAD_DoFind(fr);
	}
	return 0;
    }

    switch (msg) {

    case WM_CREATE:
    {
        static const __WCHAR editW[] = { 'e','d','i','t',0 };
	DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL |
	                ES_AUTOVSCROLL | ES_MULTILINE | ES_NOHIDESEL;
        unsigned int uTabLength = TAB_LENGTH * 4;
        RECT rc;
        GetClientRect(hWnd, &rc);

	if (!Globals.bWrapLongLines) dwStyle |= WS_HSCROLL | ES_AUTOHSCROLL;

        Globals.hEdit = CreateWindowEx(WS_EX_CLIENTEDGE, editW, NULL, dwStyle,
                             0, 0, rc.right, rc.bottom, hWnd,
                             NULL, Globals.hInstance, NULL);
        LILYPAD_InitFont();
        SendMessage(Globals.hEdit, EM_SETTABSTOPS, 1, (LPARAM)&uTabLength);
        break;
    }

    case WM_COMMAND:
        LILYPAD_MenuCommand(LOWORD(wParam));
        break;

    case WM_DESTROYCLIPBOARD:
        /*MessageBox(Globals.hMainWnd, "Empty clipboard", "Debug", MB_ICONEXCLAMATION);*/
        break;

    case WM_CLOSE:
        if (DoCloseFile()) {
            DestroyWindow(hWnd);
        }
        break;

    case WM_QUERYENDSESSION:
        if (DoCloseFile()) {
            return 1;
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case WM_SIZE:
        SetWindowPos(Globals.hEdit, NULL, 0, 0, LOWORD(lParam), HIWORD(lParam),
                     SWP_NOOWNERZORDER | SWP_NOZORDER);
        break;

    case WM_SETFOCUS:
        SetFocus(Globals.hEdit);
        break;

    case WM_DROPFILES:
    {
        __WCHAR szFileName[MAX_PATH];
        HANDLE hDrop = (HANDLE) wParam;

        DragQueryFile(hDrop, 0, szFileName, SIZEOF(szFileName));
        DragFinish(hDrop);
        DoOpenFile(szFileName);
        break;
    }

    case WM_INITMENUPOPUP:
        LILYPAD_InitMenuPopup((HMENU)wParam, lParam);
        break;

    case WM_DPICHANGED:
        WmDpiChanged(hWnd, HIWORD(wParam), (LPRECT)lParam);
        break;

    case WM_NCCREATE:
        WmNcCreate(hWnd);
        return DefWindowProc(hWnd, msg, wParam, lParam);

    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}

static int AlertFileDoesNotExist(__LPCWSTR szFileName)
{
   int nResult;
   __WCHAR szMessage[MAX_STRING_LEN];
   __WCHAR szResource[MAX_STRING_LEN];

   LoadString(Globals.hInstance, STRING_DOESNOTEXIST, szResource, SIZEOF(szResource));
   wsprintf(szMessage, szResource, szFileName);

   LoadString(Globals.hInstance, STRING_ERROR, szResource, SIZEOF(szResource));

   nResult = MessageBox(Globals.hMainWnd, szMessage, szResource,
                        MB_ICONEXCLAMATION | MB_YESNO);

   return(nResult);
}

static void HandleCommandLine(__LPWSTR cmdline)
{
    __WCHAR delimiter;
    int opt_print=0;
    int column=0;
    int line=0;

    /* skip white space */
    while (*cmdline == ' ')
	cmdline++;

    /* skip executable name */
    delimiter = (*cmdline == '"' ? '"' : ' ');

    if (*cmdline == delimiter)
	cmdline++;

    while (*cmdline && *cmdline != delimiter)
	cmdline++;

    if (*cmdline == delimiter)
	cmdline++;

    while (*cmdline == ' ' || *cmdline == '-' || *cmdline == '+')
    {
        __WCHAR c = *cmdline++;

	if (c == '+')
	{
	    while (*cmdline >= '0' && *cmdline <= '9')
	    {
	        line *= 10;
		line += *((char *) cmdline) - '0';
		cmdline++;
	    }
	   if (*cmdline == ':')
	     {
		 cmdline++;
		while (*cmdline >= '0' && *cmdline <= '9')
		{
		    column *= 10;
		    column += *((char *) cmdline) - '0';
		    cmdline++;
		}
	     }
	}
	else if (c == '-')
	{
	    if (*cmdline == 'p' || *cmdline == 'P')
		opt_print = 1;
	    /* skipping unknown option */
	    cmdline++;
	 }
    }

    if (*cmdline)
    {
        /* file name is passed in the command line */
        __LPCWSTR file_name;
        BOOL file_exists;
        __WCHAR buf[MAX_PATH];

        if (cmdline[0] == '"')
        {
            cmdline++;
            cmdline[lstrlen(cmdline) - 1] = 0;
        }

        if (FileExists(cmdline))
        {
            file_exists = TRUE;
            file_name = cmdline;
        }
        else
        {
            static const __WCHAR lyW[] = { '.','l','y',0 };

            /* try to find file with ".ly" extension */
            if (!lstrcmp(lyW, cmdline + lstrlen(cmdline) - lstrlen(lyW)))
            {
                file_exists = FALSE;
                file_name = cmdline;
	    }
            else
            {
                lstrcpyn(buf, cmdline, MAX_PATH - lstrlen(lyW) - 1);
                lstrcat(buf, lyW);
                file_name = buf;
                file_exists = FileExists(buf);
            }
        }

        if (file_exists)
        {
            DoOpenFile(file_name);
            InvalidateRect(Globals.hMainWnd, NULL, FALSE);
            if (opt_print)
                DIALOG_FilePrint();
	    if (line || column)
		GotoLineColumn(line, column);
        }
        else
        {
            switch (AlertFileDoesNotExist(file_name)) {
            case IDYES:
                DoOpenFile(file_name);
                break;

            case IDNO:
                break;
            }
        }
     }
}

static LPTSTR LILYPAD_StrRStr(LPTSTR pszSource, LPTSTR pszLast, LPTSTR pszSrch)
{
    int len = lstrlen(pszSrch);
    pszLast--;
    while (pszLast >= pszSource)
    {
        if (StrCmpN(pszLast, pszSrch, len) == 0)
            return pszLast;
        pszLast--;
    }
    return NULL;
}

/***********************************************************************
 * The user activated the Find dialog
 */
void LILYPAD_DoFind(FINDREPLACE *fr)
{
    LPTSTR content;
    LPTSTR found;
    int len = lstrlen(fr->lpstrFindWhat);
    int fileLen;
    DWORD pos;

    fileLen = GetWindowTextLength(Globals.hEdit) + 1;
    content = HeapAlloc(GetProcessHeap(), 0, fileLen * sizeof(TCHAR));
    if (!content) return;
    GetWindowText(Globals.hEdit, content, fileLen);

    SendMessage(Globals.hEdit, EM_GETSEL, 0, (LPARAM)&pos);
    switch (fr->Flags & (FR_DOWN|FR_MATCHCASE))
    {
        case 0:
            found = StrRStrI(content, content+pos-len, fr->lpstrFindWhat);
            break;
        case FR_DOWN:
            found = StrStrI(content+pos, fr->lpstrFindWhat);
            break;
        case FR_MATCHCASE:
            found = LILYPAD_StrRStr(content, content+pos-len, fr->lpstrFindWhat);
            break;
        case FR_DOWN|FR_MATCHCASE:
            found = StrStr(content+pos, fr->lpstrFindWhat);
            break;
        default:    /* shouldn't happen */
            return;
    }
    HeapFree(GetProcessHeap(), 0, content);

    if (found == NULL)
    {
        DIALOG_StringMsgBox(Globals.hFindReplaceDlg, STRING_NOTFOUND, fr->lpstrFindWhat,
            MB_ICONINFORMATION|MB_OK);
        return;
    }

    SendMessage(Globals.hEdit, EM_SETSEL, found - content, found - content + len);
    SendMessage(Globals.hEdit, EM_SCROLLCARET, 0, 0);
}

/***********************************************************************
 *
 *           WinMain
 */
int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE prev, LPSTR cmdline, int show)
{
    MSG        msg;
    HACCEL      hAccel;
    WNDCLASSEX class;
    static const __WCHAR className[] = {'L','i','l','y','P','a','d',0};
    static const __WCHAR winName[]   = {'L','i','l','y','P','a','d',0};

    aFINDMSGSTRING = RegisterWindowMessage(FINDMSGSTRING);

    ZeroMemory(&Globals, sizeof(Globals));
    Globals.hInstance       = hInstance;
    Globals.bWrapLongLines  = TRUE;

    ZeroMemory(&class, sizeof(class));
    class.cbSize        = sizeof(class);
    class.lpfnWndProc   = LILYPAD_WndProc;
    class.hInstance     = Globals.hInstance;

    class.hIcon         = LoadIcon(Globals.hInstance, MAKEINTRESOURCE(IDI_LILYPAD));
    class.hCursor       = LoadCursor(0, IDC_ARROW);
    class.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    class.lpszMenuName  = MAKEINTRESOURCE(MAIN_MENU);
    class.lpszClassName = className;
    /* TODO: Create small 16x16 icon for taskbar
     * and upper-left corner of window */
    /* class.hIconSm    = LoadIcon(0, IDI_APPLICATION); */

       /* if system calls fail, there is no unicode support */
    if (!RegisterClassEx (&class))
      {
#ifdef UNICODE
	if (GetLastError () == ERROR_CALL_NOT_IMPLEMENTED)
	  ExitProcess (3);
#endif
       /* FIXME: what happens here? */
       return FALSE;
     }

    /* Setup windows */

    initialize_per_monitor_dpi ();
    Globals.hMainWnd =
        CreateWindow(className, winName, WS_OVERLAPPEDWINDOW,
                     CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
                     NULL, NULL, Globals.hInstance, NULL);
    if (!Globals.hMainWnd)
    {
        ShowLastError();
        ExitProcess(1);
    }

    LILYPAD_InitData();
    DIALOG_FileNew();

    ShowWindow(Globals.hMainWnd, show);
    UpdateWindow(Globals.hMainWnd);
    DragAcceptFiles(Globals.hMainWnd, TRUE);

    HandleCommandLine(GetCommandLine());

    hAccel = LoadAccelerators( hInstance, MAKEINTRESOURCE(ID_ACCEL) );

    while (GetMessage(&msg, 0, 0, 0))
    {
	if (!TranslateAccelerator(Globals.hMainWnd, hAccel, &msg) && !IsDialogMessage(Globals.hFindReplaceDlg, &msg))
	{
	    TranslateMessage(&msg);
	    DispatchMessage(&msg);
	}
    }
    uninitialize_per_monitor_dpi ();
    return msg.wParam;
}

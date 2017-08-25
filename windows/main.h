/*
 *  LilyPad (notepad.h)
 *
 *  Copyright 1997,98 Marcel Baur <mbaur@g26.ethz.ch>
 *  Copyright 2002 Sylvain Petreolle <spetreolle@yahoo.fr>
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
 */

#ifdef UNICODE
#define __WCHAR	    WCHAR
#define __LPCWSTR   LPCWSTR
#define __LPWSTR    LPWSTR
#else
#define	__WCHAR	    char
#define __LPCWSTR   LPCSTR
#define __LPWSTR    LPSTR
#endif

#define SIZEOF(a) sizeof(a)/sizeof((a)[0])

#include "lilypad_res.h"

#define MAX_STRING_LEN      255
#define TAB_LENGTH          8

typedef struct
{
  HANDLE  hInstance;
  HWND    hMainWnd;
  HWND    hFindReplaceDlg;
  HWND    hEdit;
  HFONT   hFont; /* Font used by the edit control */
  LOGFONT lfFont;
  int     iPointSize;
  WORD    wDPI;
  BOOL    bWrapLongLines;
  __WCHAR   szFindText[MAX_PATH];
  __WCHAR   szFileName[MAX_PATH];
  __WCHAR   szFileTitle[MAX_PATH];
  __WCHAR   szFilter[2 * MAX_STRING_LEN + 100];

  RECT    rtMargin;
  DWORD   MarginFlags;

  FINDREPLACE find;
  FINDREPLACE lastFind;
  HGLOBAL hDevMode; /* printer mode */
  HGLOBAL hDevNames; /* printer names */
} LILYPAD_GLOBALS;

extern LILYPAD_GLOBALS Globals;

VOID SetFileName(__LPCWSTR szFileName);
void LILYPAD_DoFind(FINDREPLACE *fr);

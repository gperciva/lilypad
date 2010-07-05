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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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

#include "notepad_res.h"

#define MAX_STRING_LEN      255

typedef struct
{
  HANDLE  hInstance;
  HWND    hMainWnd;
  HWND    hFindReplaceDlg;
  HWND    hEdit;
  HFONT   hFont; /* Font used by the edit control */
  LOGFONT lfFont;
  BOOL    bWrapLongLines;
  __WCHAR   szFindText[MAX_PATH];
  __WCHAR   szFileName[MAX_PATH];
  __WCHAR   szFileTitle[MAX_PATH];
  __WCHAR   szFilter[2 * MAX_STRING_LEN + 100];
  __WCHAR   szMarginTop[MAX_PATH];
  __WCHAR   szMarginBottom[MAX_PATH];
  __WCHAR   szMarginLeft[MAX_PATH];
  __WCHAR   szMarginRight[MAX_PATH];
  __WCHAR   szHeader[MAX_PATH];
  __WCHAR   szFooter[MAX_PATH];

  FINDREPLACE find;
  HGLOBAL hDevMode; /* printer mode */
  HGLOBAL hDevNames; /* printer names */
} LILYPAD_GLOBALS;

extern LILYPAD_GLOBALS Globals;

VOID SetFileName(__LPCWSTR szFileName);

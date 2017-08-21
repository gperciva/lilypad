/*
 *  LilyPad (pmdpi.c: per-monitor DPI aware)
 *
 *  Copyright (C) 2017 Masamichi Hosoda <trueroad@trueroad.jp>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 *
 */

#include <windows.h>

#include "main.h"
#include "pmdpi.h"

typedef BOOL (WINAPI *LPENABLENONCLIENTDPISCALING)(HWND);

static HMODULE hModDll = NULL;
static LPENABLENONCLIENTDPISCALING lpfnEnableNonClientDpiScaling = NULL;

VOID initialize_per_monitor_dpi (VOID)
{
    if( !hModDll )
        hModDll = LoadLibrary(TEXT("user32.dll"));
    if( hModDll && !lpfnEnableNonClientDpiScaling )
        lpfnEnableNonClientDpiScaling =
          (LPENABLENONCLIENTDPISCALING)
          GetProcAddress(hModDll, "EnableNonClientDpiScaling");
}

VOID uninitialize_per_monitor_dpi (VOID)
{
    lpfnEnableNonClientDpiScaling = NULL;
    if( hModDll ) {
        FreeLibrary(hModDll);
        hModDll = NULL;
    }
}

VOID WmDpiChanged(HWND hWnd, WORD wDPI, LPRECT lprc)
{
    HFONT currfont = Globals.hFont;
    LPLOGFONT lf = &Globals.lfFont;

    Globals.wDPI = wDPI;
    lf->lfHeight = -MulDiv(Globals.iPointSize,
                           Globals.wDPI,
                           72 * 10);  /* 72pt = 1inch */

    Globals.hFont=CreateFontIndirect( lf );
    SendMessage( Globals.hEdit, WM_SETFONT,
                 (WPARAM)Globals.hFont, (LPARAM)FALSE );
    if( currfont!=NULL )
        DeleteObject( currfont );

    SetWindowPos(hWnd, NULL, lprc->left, lprc->top,
                 lprc->right - lprc->left, lprc->bottom - lprc->top,
                 SWP_NOZORDER);
}

VOID WmNcCreate(HWND hWnd)
{
    if( lpfnEnableNonClientDpiScaling )
        lpfnEnableNonClientDpiScaling(hWnd);
}

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
typedef HANDLE (WINAPI *LPSETTHREADDPIAWARENESSCONTEXT)(HANDLE);

#ifndef DPI_AWARENESS_CONTEXT_SYSTEM_AWARE
#define DPI_AWARENESS_CONTEXT_SYSTEM_AWARE ((HANDLE)-2)
#endif
#ifndef DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE
#define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE ((HANDLE)-3)
#endif
#ifndef DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2
#define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 ((HANDLE)-4)
#endif

static HMODULE hModDll = NULL;
static LPENABLENONCLIENTDPISCALING lpfnEnableNonClientDpiScaling = NULL;
static LPSETTHREADDPIAWARENESSCONTEXT lpfnSetThreadDpiAwarenessContext = NULL;
static BOOL bDialogSwitchAware;

VOID initialize_per_monitor_dpi (VOID)
{
    HANDLE aware;

    if( !hModDll )
        hModDll = LoadLibrary(TEXT("user32.dll"));
    if( hModDll && !lpfnEnableNonClientDpiScaling )
        lpfnEnableNonClientDpiScaling =
          (LPENABLENONCLIENTDPISCALING)
          GetProcAddress(hModDll, "EnableNonClientDpiScaling");
    if( hModDll && !lpfnSetThreadDpiAwarenessContext )
        lpfnSetThreadDpiAwarenessContext =
          (LPSETTHREADDPIAWARENESSCONTEXT)
          GetProcAddress(hModDll, "SetThreadDpiAwarenessContext");

    if( lpfnSetThreadDpiAwarenessContext ) {
        aware = lpfnSetThreadDpiAwarenessContext
          (DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
        if( aware ) {
            /* Windows 10 Creators Update (1703) + */
            bDialogSwitchAware = FALSE;
        } else {
            /* Windows 10 Anniversary Update (1607) */
            bDialogSwitchAware = TRUE;
            set_per_monitor_dpi();
        }
    }
}

VOID uninitialize_per_monitor_dpi (VOID)
{
    lpfnSetThreadDpiAwarenessContext = NULL;
    lpfnEnableNonClientDpiScaling = NULL;
    if( hModDll ) {
        FreeLibrary(hModDll);
        hModDll = NULL;
    }
}

VOID unset_per_monitor_dpi (VOID)
{
    if( lpfnSetThreadDpiAwarenessContext && bDialogSwitchAware )
        lpfnSetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_SYSTEM_AWARE);
}

VOID set_per_monitor_dpi (VOID)
{
    if( lpfnSetThreadDpiAwarenessContext && bDialogSwitchAware )
        lpfnSetThreadDpiAwarenessContext
          (DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);
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

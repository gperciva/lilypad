/*
 *  LilyPad (pmdpi.h: per-monitor DPI aware)
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

#ifndef WM_DPICHANGED
#define WM_DPICHANGED 0x02e0
#endif

VOID initialize_per_monitor_dpi (VOID);
VOID uninitialize_per_monitor_dpi (VOID);

VOID WmDpiChanged(HWND hWnd, WORD wDPI, LPRECT lprc);
VOID WmNcCreate(HWND hWnd);

/* -*-c-style:stroustrup-*-
 *  LilyPad (dialog.c)
 *
 *  Copyright 1998,99 Marcel Baur <mbaur@g26.ethz.ch>
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
 */

#include <assert.h>
#include <stdio.h>
#include <windows.h>
#include <commdlg.h>
#include <shlwapi.h>

#include "main.h"
#include "dialog.h"
#include "convert.h"

static INT_PTR WINAPI DIALOG_AboutLilyPadDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

VOID ShowLastError(void)
{
    DWORD error = GetLastError();
    if (error != NO_ERROR)
    {
        __LPWSTR lpMsgBuf;
        __WCHAR szTitle[MAX_STRING_LEN];

        LoadString(Globals.hInstance, STRING_ERROR, szTitle, SIZEOF(szTitle));
        FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            NULL, error, 0,
            (LPTSTR) &lpMsgBuf, 0, NULL);
        MessageBox(NULL, lpMsgBuf, szTitle, MB_OK | MB_ICONERROR);
        LocalFree(lpMsgBuf);
    }
}

/**
 * Sets the caption of the main window according to Globals.szFileTitle:
 *    Untitled - LilyPad     if no file is open
 *    filename - LilyPad     if a file is given
 */
static void UpdateWindowCaption(void)
{
  __WCHAR szCaption[MAX_STRING_LEN];
  __WCHAR szLilyPad[MAX_STRING_LEN];
  static const __WCHAR hyphenW[] = { ' ','-',' ',0 };

  LoadString(Globals.hInstance, STRING_LILYPAD, szCaption, SIZEOF(szCaption));

  if (Globals.szFileTitle[0] != '\0')
    lstrcpy (szCaption, Globals.szFileTitle);
  else
    LoadString(Globals.hInstance, STRING_UNTITLED, szCaption, SIZEOF(szCaption));

  LoadString(Globals.hInstance, STRING_LILYPAD, szLilyPad, SIZEOF(szLilyPad));
  lstrcat(szCaption, hyphenW);
  lstrcat(szCaption, szLilyPad);

  SetWindowText(Globals.hMainWnd, szCaption);
}

int DIALOG_StringMsgBox(HWND hParent, int formatId, __LPCWSTR szString, DWORD dwFlags)
{
   __WCHAR szMessage[MAX_STRING_LEN];
   __WCHAR szResource[MAX_STRING_LEN];

   /* Load and format szMessage */
   LoadString(Globals.hInstance, formatId, szResource, SIZEOF(szResource));
   wnsprintf(szMessage, SIZEOF(szMessage), szResource, szString);

   /* Load szCaption */
   if ((dwFlags & MB_ICONMASK) == MB_ICONEXCLAMATION)
     LoadString(Globals.hInstance, STRING_ERROR, szResource, SIZEOF(szResource));
   else
     LoadString(Globals.hInstance, STRING_LILYPAD, szResource, SIZEOF(szResource));

   /* Display Modal Dialog */
   if (hParent == NULL)
     hParent = Globals.hMainWnd;
   return MessageBox(hParent, szMessage, szResource, dwFlags);
}

static void AlertFileNotFound(__LPCWSTR szFileName)
{
  DIALOG_StringMsgBox(0, STRING_NOTFOUND, szFileName, MB_ICONEXCLAMATION|MB_OK);
}

static int AlertFileNotSaved(__LPCWSTR szFileName)
{
   __WCHAR szUntitled[MAX_STRING_LEN];

   LoadString(Globals.hInstance, STRING_UNTITLED, szUntitled, SIZEOF(szUntitled));

   return DIALOG_StringMsgBox(0,
			      STRING_NOTSAVED,
			      szFileName[0] ? szFileName : szUntitled,
			      MB_ICONQUESTION|MB_YESNOCANCEL);
}

/**
 * Returns:
 *   TRUE  - if file exists
 *   FALSE - if file does not exist
 */
BOOL FileExists(__LPCWSTR szFilename)
{
   WIN32_FIND_DATA entry;
   HANDLE hFile;

   hFile = FindFirstFile(szFilename, &entry);
   FindClose(hFile);

   return (hFile != INVALID_HANDLE_VALUE);
}


static VOID DoSaveFile(VOID)
{
    HANDLE hFile;
    DWORD dwNumWrite;
    LPSTR pTemp;
    DWORD size;
#ifdef UNICODE
    LPWSTR pwTemp;
    DWORD wsize;
#endif

    hFile = CreateFile(Globals.szFileName, GENERIC_WRITE, FILE_SHARE_WRITE,
                       NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if(hFile == INVALID_HANDLE_VALUE)
    {
        ShowLastError();
        return;
    }

#ifdef UNICODE
    wsize = GetWindowTextLengthW(Globals.hEdit) + 1;
    pwTemp = HeapAlloc(GetProcessHeap(), 0, wsize * sizeof(WCHAR));
    if (!pwTemp)
    {
	CloseHandle(hFile);
        ShowLastError();
        return;
    }
    wsize = GetWindowTextW(Globals.hEdit, pwTemp, wsize) + 1;

    size = WideCharToMultiByte(CP_UTF8, 0, pwTemp, wsize ,
			       NULL, 0, NULL, NULL);
    pTemp = HeapAlloc(GetProcessHeap(), 0, size);
    if (!pTemp)
    {
	HeapFree(GetProcessHeap(), 0, pwTemp);
	CloseHandle(hFile);
        ShowLastError();
        return;
    }
    size = WideCharToMultiByte(CP_UTF8, 0, pwTemp, wsize,
			       pTemp, size, NULL, NULL) - 1;
#else
    size = GetWindowTextLengthA(Globals.hEdit) + 1;
    pTemp = HeapAlloc(GetProcessHeap(), 0, size);
    if (!pTemp)
    {
	CloseHandle(hFile);
        ShowLastError();
        return;
    }
    size = GetWindowTextA(Globals.hEdit, pTemp, size);
#endif

    if (!WriteFile(hFile, pTemp, size, &dwNumWrite, NULL))
        ShowLastError();
    else
        SendMessage(Globals.hEdit, EM_SETMODIFY, FALSE, 0);

    SetEndOfFile(hFile);
    CloseHandle(hFile);
#ifdef UNICODE
    HeapFree(GetProcessHeap(), 0, pwTemp);
#endif
    HeapFree(GetProcessHeap(), 0, pTemp);
}

/**
 * Returns:
 *   TRUE  - User agreed to close (both save/don't save)
 *   FALSE - User cancelled close by selecting "Cancel"
 */
BOOL DoCloseFile(void)
{
    int nResult;
    static const __WCHAR empty_strW[] = { 0 };

    if (SendMessage(Globals.hEdit, EM_GETMODIFY, 0, 0))
    {
        /* prompt user to save changes */
        nResult = AlertFileNotSaved(Globals.szFileName);
        switch (nResult) {
            case IDYES:     DIALOG_FileSave();
                            break;

            case IDNO:      break;

            case IDCANCEL:  return(FALSE);

            default:        return(FALSE);
        } /* switch */
    } /* if */

    SetFileName(empty_strW);

    UpdateWindowCaption();
    return(TRUE);
}

static LPTSTR newline_normalization(LPTSTR pIn, LPDWORD lpdwSize)
{
    int counter=0;
    int i;

    for ( i = 0; i < *lpdwSize; i++ )
    {
        switch (pIn[i])
        {
            case TEXT('\r'):
                if (pIn[i+1]==TEXT('\n'))
                    i++;
                else
                    counter++;
                break;
            case TEXT('\n'):
                counter++;
                break;
            case 0:
                *lpdwSize = i;
                break;
        }
    }
    if (counter)
    {
        LPTSTR pOut;
        DWORD dwNewSize = *lpdwSize + counter;

        pOut = HeapAlloc(GetProcessHeap(), 0,
			 (dwNewSize + 1) * sizeof(TCHAR) );
        if (pOut)
        {
            int i_in = 0;
            int i_out = 0;

            while ((i_in < *lpdwSize) && (i_out < dwNewSize))
            {
                TCHAR c=pIn[i_in];
                switch (c)
                {
                    case TEXT('\r'):
                        pOut[i_out++] = TEXT('\r');
                        pOut[i_out] = TEXT('\n');
                        if (pIn[i_in+1]==TEXT('\n'))
                            i_in++;
                        break;
                    case TEXT('\n'):
                        pOut[i_out++] = TEXT('\r');
                        pOut[i_out] = TEXT('\n');
                        break;
                    case 0:
                        pIn[i_out] = 0;
                        *lpdwSize = i_in;
                        dwNewSize = i_out;
                        break;
                    default:
                        pOut[i_out] = c;
                        break;
                }
                i_in++;
                i_out++;
            }

            pOut[dwNewSize] = 0;
            HeapFree(GetProcessHeap(), 0, pIn);
            *lpdwSize = dwNewSize;
            return pOut;
        }
    }
    return pIn;
}

void DoOpenFile(__LPCWSTR szFileName)
{
    static const __WCHAR dotlog[] = { '.','L','O','G',0 };
    HANDLE hFile;
    LPSTR pTemp;
    DWORD size;
    DWORD dwNumRead;
    __WCHAR log[5];
#ifdef UNICODE
    LPWSTR pwTemp;
    DWORD wsize;
    DWORD dwNumConverted;
#endif

    /* Close any files and prompt to save changes */
    if (!DoCloseFile())
	return;

    hFile = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ, NULL,
	OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if(hFile == INVALID_HANDLE_VALUE)
    {
        AlertFileNotFound(szFileName);
	return;
    }

    size = GetFileSize(hFile, NULL);
    if (size == INVALID_FILE_SIZE)
    {
	CloseHandle(hFile);
	ShowLastError();
	return;
    }

    pTemp = HeapAlloc(GetProcessHeap(), 0, size + 1);
    if (!pTemp)
    {
	CloseHandle(hFile);
	ShowLastError();
	return;
    }

    if (!ReadFile(hFile, pTemp, size, &dwNumRead, NULL))
    {
	CloseHandle(hFile);
	HeapFree(GetProcessHeap(), 0, pTemp);
	ShowLastError();
	return;
    }

    CloseHandle(hFile);

#ifdef UNICODE
    wsize = dwNumRead + 1;
    pwTemp = HeapAlloc(GetProcessHeap(), 0, wsize * sizeof(WCHAR));
    if (!pwTemp)
    {
	HeapFree(GetProcessHeap(), 0, pTemp);
	ShowLastError();
	return;
    }
    convert_to_utf16(pTemp, dwNumRead, pwTemp, wsize, &dwNumConverted);

    pwTemp[dwNumConverted] = 0;  /* null termination */
    pwTemp=newline_normalization(pwTemp, &dwNumConverted);
    if (*pwTemp == 0xFEFF)
      SetWindowText(Globals.hEdit, pwTemp + 1);  /* skip BOM */
    else
      SetWindowText(Globals.hEdit, pwTemp);

    HeapFree(GetProcessHeap(), 0, pwTemp);
#else
    pTemp[dwNumRead] = 0;  /* null termination */
    pTemp=newline_normalization(pTemp, &dwNumRead);
    SetWindowText(Globals.hEdit, pTemp);
#endif

    HeapFree(GetProcessHeap(), 0, pTemp);

    SendMessage(Globals.hEdit, EM_SETMODIFY, FALSE, 0);
    SendMessage(Globals.hEdit, EM_EMPTYUNDOBUFFER, 0, 0);
    SetFocus(Globals.hEdit);
    
    /*  If the file starts with .LOG, add a time/date at the end and set cursor after
     *  See http://support.microsoft.com/?kbid=260563
     */
    if (GetWindowText
	(Globals.hEdit, log, sizeof(log)/sizeof(log[0])) && !lstrcmp(log, dotlog))
    {
	static const __WCHAR lfW[] = { '\r','\n',0 };
	SendMessage(Globals.hEdit, EM_SETSEL, GetWindowTextLength(Globals.hEdit), -1);
	SendMessage(Globals.hEdit, EM_REPLACESEL, TRUE, (LPARAM)lfW);
	DIALOG_EditTimeDate();
	SendMessage(Globals.hEdit, EM_REPLACESEL, TRUE, (LPARAM)lfW);
    }

    SetFileName(szFileName);
    UpdateWindowCaption();
}

VOID DIALOG_FileNew(VOID)
{
    static const __WCHAR empty_strW[] = { 0 };

    /* Close any files and promt to save changes */
    if (DoCloseFile()) {
        SetWindowText(Globals.hEdit, empty_strW);
        SendMessage(Globals.hEdit, EM_EMPTYUNDOBUFFER, 0, 0);
        SetFocus(Globals.hEdit);
    }
}

VOID DIALOG_FileOpen(VOID)
{
    OPENFILENAME openfilename;
    __WCHAR szPath[MAX_PATH];
    __WCHAR szDir[MAX_PATH];
    static const __WCHAR szDefaultExt[] = { 'l','y',0 };
    static const __WCHAR ly_files[] = { '*','.','l','y',0 };

    ZeroMemory(&openfilename, sizeof(openfilename));

    GetCurrentDirectory(SIZEOF(szDir), szDir);
    lstrcpy(szPath, ly_files);

    openfilename.lStructSize       = sizeof(openfilename);
    openfilename.hwndOwner         = Globals.hMainWnd;
    openfilename.hInstance         = Globals.hInstance;
    openfilename.lpstrFilter       = Globals.szFilter;
    openfilename.lpstrFile         = szPath;
    openfilename.nMaxFile          = SIZEOF(szPath);
    openfilename.lpstrInitialDir   = szDir;
    openfilename.Flags             = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST |
        OFN_HIDEREADONLY;
    openfilename.lpstrDefExt       = szDefaultExt;


    if (GetOpenFileName(&openfilename))
	DoOpenFile(openfilename.lpstrFile);
}


VOID DIALOG_FileSave(VOID)
{
    if (Globals.szFileName[0] == '\0')
        DIALOG_FileSaveAs();
    else
        DoSaveFile();
}

VOID DIALOG_FileSaveAs(VOID)
{
    OPENFILENAME saveas;
    __WCHAR szPath[MAX_PATH];
    __WCHAR szDir[MAX_PATH];
    static const __WCHAR szDefaultExt[] = { 'l','y',0 };
    static const __WCHAR ly_files[] = { '*','.','l','y',0 };

    ZeroMemory(&saveas, sizeof(saveas));

    GetCurrentDirectory(SIZEOF(szDir), szDir);
    lstrcpy(szPath, ly_files);

    saveas.lStructSize       = sizeof(OPENFILENAME);
    saveas.hwndOwner         = Globals.hMainWnd;
    saveas.hInstance         = Globals.hInstance;
    saveas.lpstrFilter       = Globals.szFilter;
    saveas.lpstrFile         = szPath;
    saveas.nMaxFile          = SIZEOF(szPath);
    saveas.lpstrInitialDir   = szDir;
    saveas.Flags             = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT |
        OFN_HIDEREADONLY;
    saveas.lpstrDefExt       = szDefaultExt;

    if (GetSaveFileName(&saveas)) {
        SetFileName(szPath);
        UpdateWindowCaption();
        DoSaveFile();
    }
}

static VOID init_default_printer(VOID)
{
    if (!Globals.hDevMode && !Globals.hDevNames)
    {
        PAGESETUPDLG psd = { 0 };

        psd.lStructSize = sizeof(psd);
        psd.Flags = PSD_RETURNDEFAULT;
        if (PageSetupDlg(&psd))
        {
            Globals.hDevMode = psd.hDevMode;
            Globals.hDevNames = psd.hDevNames;

            Globals.rtMargin = psd.rtMargin;
            if (psd.Flags & PSD_INHUNDREDTHSOFMILLIMETERS)
            {
                Globals.MarginFlags = PSD_INHUNDREDTHSOFMILLIMETERS;
            }
            else if (psd.Flags & PSD_INTHOUSANDTHSOFINCHES)
            {
                Globals.MarginFlags = PSD_INTHOUSANDTHSOFINCHES;
            }
        }
    }
    if (!Globals.MarginFlags)
    {
        Globals.MarginFlags = PSD_INHUNDREDTHSOFMILLIMETERS;
        Globals.rtMargin.left = 2500;
        Globals.rtMargin.top = 2500;
        Globals.rtMargin.right = 2500;
        Globals.rtMargin.bottom = 2500;
    }
}

static VOID print_header(HDC hdc, RECT rcHdrArea, RECT rcHdrText, BOOL dopage)
{
    if (dopage)
    {
        TCHAR szUntitled[MAX_STRING_LEN];
        LPTSTR p;

        if (Globals.szFileTitle[0] != 0)
        {
            p = Globals.szFileTitle;
        }
        else
        {
            LoadString(Globals.hInstance, STRING_UNTITLED,
		       szUntitled, sizeof(szUntitled)/sizeof(szUntitled[0]));
            p = szUntitled;
        }
        /* Write a rectangle and header at the top of each page */
        Rectangle(hdc, rcHdrArea.left, rcHdrArea.top,
                  rcHdrArea.right, rcHdrArea.bottom);
        ExtTextOut(hdc, rcHdrText.left, rcHdrText.top, ETO_CLIPPED, &rcHdrText,
		   p, lstrlen(p), NULL);
    }
}

static LPTSTR print_main_text(HDC hdc, RECT rc, BOOL dopage, LPTSTR p)
{
    DRAWTEXTPARAMS dtps = { 0 };
    HDC hdcDraw;

    if(!dopage)
    {
        /* To skip this page, draw the text on dummy DC. */
        hdcDraw=CreateCompatibleDC(hdc);
        SelectObject(hdcDraw, GetCurrentObject(hdc, OBJ_FONT));
    }
    else
    {
        hdcDraw=hdc;
    }
    dtps.cbSize = sizeof(dtps);
    dtps.iTabLength = TAB_LENGTH;
    DrawTextEx(hdcDraw, p, -1, &rc, DT_EDITCONTROL | DT_NOPREFIX |
	       DT_EXPANDTABS | DT_TABSTOP |
	       ( Globals.bWrapLongLines ? DT_WORDBREAK : 0 ),
	       &dtps);

    if(!dopage)
    {
        DeleteDC(hdcDraw);
    }
#ifdef UNICODE
    p = p + dtps.uiLengthDrawn;
#else
    {
        /* A MBCS character is not 1 byte per 1 character. */
        unsigned int ui;
        for( ui = 0; ui < dtps.uiLengthDrawn; ui++)
            p = CharNext( p );
    }
#endif
    return p;
}

VOID DIALOG_FilePrint(VOID)
{
    DOCINFO di;
    PRINTDLGEX printer = { 0 };
    PRINTPAGERANGE ppr[16];
    SIZE szMetric;
    RECT rcMargin, rcHdrArea, rcHdrText, rcMain;
    int cWidthPels, cHeightPels;
    int pagecount, dopage, copycount;
    unsigned int i;
    LOGFONT lfHdrFont, lfMainFont;
    HFONT hHdrFont, hMainFont;
    HPEN hPen;
    DWORD size;
    __LPWSTR pTemp;
    static const TCHAR letterM[] = TEXT("M");
    
    /* initialize default printer */
    init_default_printer();

    /* Get Current Settings */
    printer.lStructSize           = sizeof(printer);
    printer.hwndOwner             = Globals.hMainWnd;
    printer.hDevMode              = Globals.hDevMode;
    printer.hDevNames             = Globals.hDevNames;
    
    /* Set some default flags */
    printer.Flags                 = PD_RETURNDC | PD_NOSELECTION |
      PD_NOCURRENTPAGE | PD_USEDEVMODECOPIESANDCOLLATE;

    printer.nPageRanges           = 0;
    printer.nMaxPageRanges        = sizeof(ppr) / sizeof(ppr[0]);
    printer.lpPageRanges          = ppr;

    printer.nMinPage              = 1;
    /* we really need to calculate number of pages to set nMaxPage */
    printer.nMaxPage              = -1;
    printer.nCopies               = 1;
    printer.nStartPage            = START_PAGE_GENERAL;

    if (PrintDlgEx(&printer) != S_OK ||
        printer.dwResultAction == PD_RESULT_CANCEL)
        return;

    Globals.hDevMode = printer.hDevMode;
    Globals.hDevNames = printer.hDevNames;

    if (printer.dwResultAction == PD_RESULT_APPLY)
    {
        if (printer.hDC)
            DeleteDC(printer.hDC);
        return;
    }

    assert(printer.dwResultAction == PD_RESULT_PRINT);
    assert(printer.hDC != 0);

    /* Map mode*/
    SetMapMode(printer.hDC, MM_TEXT);

    /* header font */
    lfHdrFont = Globals.lfFont;
    lfHdrFont.lfHeight = -MulDiv(Globals.iPointSize,
				 GetDeviceCaps(printer.hDC, LOGPIXELSY),
				 72 * 10);  /* 72pt = 1inch */
    hHdrFont = CreateFontIndirect(&lfHdrFont);

    /* main text font */
    lfMainFont = Globals.lfFont;
    lfMainFont.lfHeight = -MulDiv(Globals.iPointSize,
				  GetDeviceCaps(printer.hDC, LOGPIXELSY),
				  72 * 10);  /* 72pt = 1inch */
    hMainFont = CreateFontIndirect(&lfMainFont);

    /* pen */
    hPen = CreatePen(PS_INSIDEFRAME,
		     MulDiv(1, GetDeviceCaps(printer.hDC, LOGPIXELSY), 72),
		     RGB(0, 0, 0));  /* 1pt black insideframe pen */

    /* initialize DOCINFO */
    di.cbSize = sizeof(DOCINFO);
    di.lpszDocName = Globals.szFileTitle;
    di.lpszOutput = NULL;
    di.lpszDatatype = NULL;
    di.fwType = 0; 

    if (StartDoc(printer.hDC, &di) <= 0) return;
    
    /* Get the page dimensions in pixels. */
    cWidthPels = GetDeviceCaps(printer.hDC, HORZRES);
    cHeightPels = GetDeviceCaps(printer.hDC, VERTRES);

    /* Get the header font height for calculate page layout */
    SelectObject(printer.hDC, hHdrFont);
    GetTextExtentPoint32(printer.hDC, letterM, 1, &szMetric);

    /* Get Margin */
    rcMargin = Globals.rtMargin;
    if (Globals.MarginFlags & PSD_INHUNDREDTHSOFMILLIMETERS)
    {
        rcMargin.left = MulDiv(rcMargin.left,
                               GetDeviceCaps(printer.hDC, LOGPIXELSX),
                               2540);  /* 25.4mm = 1inch */
        rcMargin.top = MulDiv(rcMargin.top,
                              GetDeviceCaps(printer.hDC, LOGPIXELSX),
                              2540);  /* 25.4mm = 1inch */
        rcMargin.right = MulDiv(rcMargin.right,
                                GetDeviceCaps(printer.hDC, LOGPIXELSX),
                                2540);  /* 25.4mm = 1inch */
        rcMargin.bottom = MulDiv(rcMargin.bottom,
                                 GetDeviceCaps(printer.hDC, LOGPIXELSX),
                                 2540);  /* 25.4mm = 1inch */
    }
    else
    {
        rcMargin.left = MulDiv(rcMargin.left,
                               GetDeviceCaps(printer.hDC, LOGPIXELSX),
                               1000);
        rcMargin.top = MulDiv(rcMargin.top,
                              GetDeviceCaps(printer.hDC, LOGPIXELSX),
                              1000);
        rcMargin.right = MulDiv(rcMargin.right,
                                GetDeviceCaps(printer.hDC, LOGPIXELSX),
                                1000);
        rcMargin.bottom = MulDiv(rcMargin.bottom,
                                 GetDeviceCaps(printer.hDC, LOGPIXELSX),
                                 1000);
    }

    /* The RECT for the header area */
    rcHdrArea.left = rcMargin.left;
    rcHdrArea.top = rcMargin.top;
    rcHdrArea.right = cWidthPels-rcMargin.right;
    rcHdrArea.bottom = rcMargin.top+szMetric.cy*2;

    /* The RECT for the header text */
    rcHdrText.left = rcMargin.left + szMetric.cx*2;
    rcHdrText.top = rcMargin.top+szMetric.cy/2;
    rcHdrText.right = cWidthPels-rcMargin.right-szMetric.cx*2;
    rcHdrText.bottom = rcMargin.top+szMetric.cy*2;

    /* The RECT for the main text */
    rcMain.left = rcMargin.left;
    rcMain.top = rcMargin.top+szMetric.cy*4;
    rcMain.right = cWidthPels-rcMargin.right;
    rcMain.bottom = cHeightPels-rcMargin.bottom;

    /* pen */
    SelectObject(printer.hDC, hPen);

    /* Get the file text */
    size = GetWindowTextLength(Globals.hEdit) + 1;
    pTemp = HeapAlloc(GetProcessHeap(), 0, size * sizeof(__WCHAR));
    if (!pTemp)
    {
        ShowLastError();
        return;
    }
    size = GetWindowText(Globals.hEdit, pTemp, size);
    
    for (copycount=1; copycount <= printer.nCopies; copycount++) {
        LPTSTR p = pTemp;
        pagecount = 1;
        do {
            if ( printer.Flags & PD_PAGENUMS )
            {
                int i;

                dopage = 0;
                for ( i = 0; i < printer.nPageRanges; i++ )
                {
                    if(pagecount >= printer.lpPageRanges[i].nFromPage &&
		       pagecount <= printer.lpPageRanges[i].nToPage)
                        dopage = 1;
                }
            }
            else
                dopage = 1;
            
            if (dopage) {
                if (StartPage(printer.hDC) <= 0) {
                    static const __WCHAR failedW[] = { 'S','t','a','r','t','P','a','g','e',' ','f','a','i','l','e','d',0 };
                    static const __WCHAR errorW[] = { 'P','r','i','n','t',' ','E','r','r','o','r',0 };
                    MessageBox(Globals.hMainWnd, failedW, errorW, MB_ICONEXCLAMATION);
                    return;
                }
            }

            SelectObject(printer.hDC, hHdrFont);
            print_header(printer.hDC, rcHdrArea, rcHdrText, dopage);

            SelectObject(printer.hDC, hMainFont);
            p = print_main_text(printer.hDC, rcMain, dopage, p);
            
            if (dopage)
                EndPage(printer.hDC);
            pagecount++;
        } while ( *p != 0 );
    }

    EndDoc(printer.hDC);
    DeleteDC(printer.hDC);
    DeleteObject(hPen);
    DeleteObject(hMainFont);
    DeleteObject(hHdrFont);
    HeapFree(GetProcessHeap(), 0, pTemp);
}

VOID DIALOG_FileExit(VOID)
{
    PostMessage(Globals.hMainWnd, WM_CLOSE, 0, 0l);
}

VOID DIALOG_EditUndo(VOID)
{
    SendMessage(Globals.hEdit, EM_UNDO, 0, 0);
}

VOID DIALOG_EditCut(VOID)
{
    SendMessage(Globals.hEdit, WM_CUT, 0, 0);
}

VOID DIALOG_EditCopy(VOID)
{
    SendMessage(Globals.hEdit, WM_COPY, 0, 0);
}

VOID DIALOG_EditPaste(VOID)
{
    SendMessage(Globals.hEdit, WM_PASTE, 0, 0);
}

VOID DIALOG_EditDelete(VOID)
{
    SendMessage(Globals.hEdit, WM_CLEAR, 0, 0);
}

VOID DIALOG_EditSelectAll(VOID)
{
    SendMessage(Globals.hEdit, EM_SETSEL, 0, (LPARAM)-1);
}

VOID DIALOG_EditTimeDate(VOID)
{
    SYSTEMTIME   st;
    __WCHAR        szDate[MAX_STRING_LEN];
    static const __WCHAR spaceW[] = { ' ',0 };

    GetLocalTime(&st);

    GetTimeFormat(LOCALE_USER_DEFAULT, 0, &st, NULL, szDate, MAX_STRING_LEN);
    SendMessage(Globals.hEdit, EM_REPLACESEL, TRUE, (LPARAM)szDate);

    SendMessage(Globals.hEdit, EM_REPLACESEL, TRUE, (LPARAM)spaceW);

    GetDateFormat(LOCALE_USER_DEFAULT, DATE_LONGDATE, &st, NULL, szDate, MAX_STRING_LEN);
    SendMessage(Globals.hEdit, EM_REPLACESEL, TRUE, (LPARAM)szDate);
}

VOID DIALOG_EditWrap(VOID)
{
    BOOL modify = FALSE;
    static const __WCHAR editW[] = { 'e','d','i','t',0 };
    DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL |
                    ES_AUTOVSCROLL | ES_MULTILINE | ES_NOHIDESEL ;
    RECT rc;
    DWORD size;
    __LPWSTR pTemp;

    size = GetWindowTextLength(Globals.hEdit) + 1;
    pTemp = HeapAlloc(GetProcessHeap(), 0, size * sizeof(__WCHAR));
    if (!pTemp)
    {
        ShowLastError();
        return;
    }
    GetWindowText(Globals.hEdit, pTemp, size);
    modify = SendMessage(Globals.hEdit, EM_GETMODIFY, 0, 0);
    DestroyWindow(Globals.hEdit);
    GetClientRect(Globals.hMainWnd, &rc);
    if( Globals.bWrapLongLines ) dwStyle |= WS_HSCROLL | ES_AUTOHSCROLL;
    Globals.hEdit = CreateWindowEx(WS_EX_CLIENTEDGE, editW, NULL, dwStyle,
                         0, 0, rc.right, rc.bottom, Globals.hMainWnd,
                         NULL, Globals.hInstance, NULL);
    SendMessage(Globals.hEdit, WM_SETFONT, (WPARAM)Globals.hFont, (LPARAM)FALSE);
    SetWindowText(Globals.hEdit, pTemp);
    SendMessage(Globals.hEdit, EM_SETMODIFY, (WPARAM)modify, 0);
    SetFocus(Globals.hEdit);
    HeapFree(GetProcessHeap(), 0, pTemp);
    
    Globals.bWrapLongLines = !Globals.bWrapLongLines;
    CheckMenuItem(GetMenu(Globals.hMainWnd), CMD_WRAP,
        MF_BYCOMMAND | (Globals.bWrapLongLines ? MF_CHECKED : MF_UNCHECKED));
}

VOID DIALOG_SelectFont(VOID)
{
    CHOOSEFONT cf;
    LOGFONT lf=Globals.lfFont;

    ZeroMemory( &cf, sizeof(cf) );
    cf.lStructSize=sizeof(cf);
    cf.hwndOwner=Globals.hMainWnd;
    cf.lpLogFont=&lf;
    cf.iPointSize=Globals.iPointSize;
    cf.Flags=CF_SCREENFONTS | CF_SCALABLEONLY | CF_NOVERTFONTS |
      CF_INITTOLOGFONTSTRUCT;

    if( ChooseFont(&cf) )
    {
        HFONT currfont=Globals.hFont;

        Globals.iPointSize=cf.iPointSize;
        lf.lfHeight = -MulDiv(Globals.iPointSize,
                              Globals.wDPI,
                              72 * 10);  /* 72pt = 1inch */
        lf.lfWidth = 0;

        Globals.hFont=CreateFontIndirect( &lf );
        Globals.lfFont=lf;
        SendMessage( Globals.hEdit, WM_SETFONT, (WPARAM)Globals.hFont, (LPARAM)TRUE );
        if( currfont!=NULL )
            DeleteObject( currfont );
    }
}

VOID DIALOG_Search(VOID)
{
        ZeroMemory(&Globals.find, sizeof(Globals.find));
        Globals.find.lStructSize      = sizeof(Globals.find);
        Globals.find.hwndOwner        = Globals.hMainWnd;
        Globals.find.hInstance        = Globals.hInstance;
        Globals.find.lpstrFindWhat    = Globals.szFindText;
        Globals.find.wFindWhatLen     = SIZEOF(Globals.szFindText);
        Globals.find.Flags            = FR_DOWN|FR_HIDEWHOLEWORD;

        /* We only need to create the modal FindReplace dialog which will */
        /* notify us of incoming events using hMainWnd Window Messages    */

        Globals.hFindReplaceDlg = FindText(&Globals.find);
        assert(Globals.hFindReplaceDlg !=0);
}

VOID DIALOG_SearchNext(VOID)
{
    if (Globals.lastFind.lpstrFindWhat == NULL)
        DIALOG_Search();
    else                /* use the last find data */
        LILYPAD_DoFind(&Globals.lastFind);
}

VOID DIALOG_HelpAboutLilyPad(VOID)
{
  DialogBox (Globals.hInstance, MAKEINTRESOURCE(DIALOG_ABOUTLILYPAD),
	     Globals.hMainWnd, DIALOG_AboutLilyPadDlgProc);
}

static INT_PTR WINAPI DIALOG_AboutLilyPadDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg)
    {
    case WM_COMMAND:
      switch (wParam)
        {
        case IDOK:
          EndDialog (hDlg, IDOK);
          return TRUE;
        case IDCANCEL:
          EndDialog (hDlg, IDCANCEL);
          return TRUE;
	default:
	    break;
	}
      break;
    case WM_INITDIALOG:
      break;
    }

  return FALSE;
}

/***********************************************************************
 *
 *           DIALOG_FilePageSetup
 */
VOID DIALOG_FilePageSetup(void)
{
    PAGESETUPDLG psd = { 0 };

    psd.lStructSize = sizeof(psd);
    psd.hwndOwner = Globals.hMainWnd;
    psd.hDevMode = Globals.hDevMode;
    psd.hDevNames = Globals.hDevNames;
    psd.Flags = ( Globals.MarginFlags ?
                  ( Globals.MarginFlags | PSD_MARGINS ) : 0);
    psd.rtMargin = Globals.rtMargin;
    if (PageSetupDlg(&psd))
    {
        Globals.hDevMode = psd.hDevMode;
        Globals.hDevNames = psd.hDevNames;

        Globals.rtMargin = psd.rtMargin;
        if (psd.Flags & PSD_INHUNDREDTHSOFMILLIMETERS)
        {
            Globals.MarginFlags = PSD_INHUNDREDTHSOFMILLIMETERS;
        }
        else if (psd.Flags & PSD_INTHOUSANDTHSOFINCHES)
        {
            Globals.MarginFlags = PSD_INTHOUSANDTHSOFINCHES;
        }
    }
}

/**
 * Returns:
 *   TRUE  - Placement successful.
 *   FALSE - Placement outside of text.
 */
BOOL GotoLineColumn(int nLine, int nColumn)
{
    int nResult;
    int nLines;
    int nCharacters=0;
    int nIndex;

    /* Line count is zero based.  */
    if (nLine > 0)
        nLine--;

    /* The number of lines.  */
    nLines = SendMessage(Globals.hEdit, EM_GETLINECOUNT, 0, 0);

    if (nLine > nLines)
	return FALSE;

    /* The desired line's character index.  */
    nIndex = SendMessage(Globals.hEdit, EM_LINEINDEX, nLine, 0);

    /* The number of characters in the selected line.  */
    nCharacters = SendMessage(Globals.hEdit, EM_LINELENGTH, nIndex, 0);

    if (nColumn > nCharacters)
	nColumn = 0;

    /* select the text .. place cursor */
    SendMessage(Globals.hEdit, EM_SETSEL, nIndex + nColumn, nIndex + nColumn);
      
    return TRUE;
}

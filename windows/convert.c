/*
 *  LilyPad (convert.c: text codepage autodetect and converter)
 *
 *  Copyright (C) 2014 Masamichi Hosoda <trueroad@trueroad.jp>
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
#include <mlang.h>

#include "convert.h"

#ifdef UNICODE

const IID IID_IMultiLanguage2 = {0xdccfc164, 0x2b38, 0x11d2, {0xb7, 0xec, 0x00, 0xc0, 0x4f, 0x8f, 0x5d, 0x9a}};
const CLSID CLSID_CMultiLanguage = {0x275c23e2, 0x3747, 0x11d0, {0x9f, 0xea, 0x00, 0xaa, 0x00, 0x3f, 0x86, 0x46}};

static BOOL to_utf16_com(LPSTR buff, DWORD size,
			 LPWSTR wbuff, DWORD wsize, LPDWORD converted);
static BOOL to_utf16_api(LPSTR buff, DWORD size,
			 LPWSTR wbuff, DWORD wsize, LPDWORD converted);
static int detect_codepage(IMultiLanguage2 *mlang, LPSTR buff, DWORD size);
static BOOL convert_codepage(IMultiLanguage2 *mlang,
			     int codepage, LPSTR buff, DWORD size,
			     LPWSTR wbuff, DWORD wsize, LPDWORD converted);

BOOL convert_to_utf16(LPSTR buff, DWORD size,
		      LPWSTR wbuff, DWORD wsize, LPDWORD converted)
{
  if(SUCCEEDED(CoInitialize(NULL)))
    {
      BOOL retval;

      /* try COM interface codepage auto detection and conversion */
      retval = to_utf16_com(buff, size, wbuff, wsize, converted);
      CoUninitialize();

      if(retval)
	return TRUE;
    }

  /* if COM interface conversion failed, try API conversion */
  return to_utf16_api(buff, size, wbuff, wsize, converted);
}

static BOOL to_utf16_com(LPSTR buff, DWORD size,
			 LPWSTR wbuff, DWORD wsize, LPDWORD converted)
{
  HRESULT result;
  IMultiLanguage2 *mlang;
  int codepage;
  BOOL retval;

  result = CoCreateInstance(&CLSID_CMultiLanguage,
			    NULL,
			    CLSCTX_INPROC_SERVER,
			    &IID_IMultiLanguage2,
			    (void**)&mlang);
  if(FAILED(result))
    return FALSE;

  codepage = detect_codepage(mlang, buff, size);
  if(codepage < 0)
    codepage = 65001;  /* if codepage auto detection failed, use UTF-8 */

  retval = convert_codepage(mlang, codepage, buff, size,
			    wbuff, wsize, converted);

  mlang->lpVtbl->Release(mlang);
  return retval;
}

static int detect_codepage(IMultiLanguage2 *mlang, LPSTR buff, DWORD size)
{
  HRESULT result;
  DetectEncodingInfo encoding[1];
  INT size_tmp=size;
  INT scores = sizeof(encoding) / sizeof(encoding[0]);

  result = mlang->lpVtbl->DetectInputCodepage(mlang,
					      MLDETECTCP_NONE,
					      0,
					      buff,
					      &size_tmp,
					      encoding,
					      &scores);
  if(FAILED(result))
    return -1;
  return encoding[0].nCodePage;
}

static BOOL convert_codepage(IMultiLanguage2 *mlang,
			     int codepage, LPSTR buff, DWORD size,
			     LPWSTR wbuff, DWORD wsize, LPDWORD converted)
{
  HRESULT result;
  DWORD dwMode=0;
  UINT size_tmp=size;
  UINT wsize_tmp=wsize;

  result = mlang->lpVtbl->ConvertStringToUnicode(mlang,
						 &dwMode,
						 codepage,
						 buff,
						 &size_tmp,
						 wbuff,
						 &wsize_tmp);
  if(FAILED(result))
    return FALSE;
  if(converted)
    *converted = wsize_tmp;
  return TRUE;
}

static BOOL to_utf16_api(LPSTR buff, DWORD size,
			 LPWSTR wbuff, DWORD wsize, LPDWORD converted)
{
  int retval;

  retval=MultiByteToWideChar(CP_UTF8, 0, buff, size, wbuff, wsize);

  if(retval>0)
    {
      if(converted)
	*converted = retval;
      return TRUE;
    }
  return FALSE;
}

#endif /* UNICODE */

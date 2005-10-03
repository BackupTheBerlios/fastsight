/*
 * fastsight
 * 
 * Copyright (c) 2005 Paulo Matias
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <vfw.h>

#include "cfgmanager.h"
#include "capture.h"

static HWND capture_hwnd;
static unsigned char *capture_buffer;

static void capture_shutdown()
{
  capDriverDisconnect(capture_hwnd); 
  DestroyWindow(capture_hwnd);
}

/* vfw needs this boring stuff... */
static LRESULT PASCAL onFrame(HWND hWnd, LPVIDEOHDR lpVHdr)
{
  
  unsigned char *ps, *pd;
  int i;
  
  if(!lpVHdr)
    return (LRESULT)FALSE;
  if(!lpVHdr->lpData)
    return (LRESULT)FALSE;
  if(lpVHdr->dwBytesUsed > 320*240*3)
    return (LRESULT)TRUE;
  
  ps = (unsigned char *)(lpVHdr->lpData+320*239*3);
  pd = capture_buffer;
  
  for(i = 0; i < 320*240; i++)
  {
    pd[0] = ps[2];
    pd[1] = ps[1];
    pd[2] = ps[0];
    pd+=3;
    ps-=3;
  }
  
  return (LRESULT)TRUE;
  
}

int capture_init()
{
  
  int cap_id;
  char *s;
  HWND hwnd;
  WNDCLASS wndclass;
  CAPDRIVERCAPS pcdc;
  DWORD dwSize;
  BITMAPINFO *pbiInfo;
  
  if(!(s = cfgmanager_get_value("cap_id")))
    s = "0";
  cap_id = atoi(s);
  
  wndclass.style = 0;
  wndclass.lpfnWndProc = DefWindowProc;
  wndclass.cbClsExtra = 0;
  wndclass.cbWndExtra = 0;
  wndclass.hInstance = NULL;
  wndclass.hIcon = NULL;
  wndclass.hCursor = NULL;
  wndclass.hbrBackground = NULL;
  wndclass.lpszMenuName = NULL;
  wndclass.lpszClassName = "WSCLAS";
  
  RegisterClass(&wndclass);
  
  hwnd=CreateWindow("WSCLAS", "", 0, 0, 0, 1, 1, HWND_DESKTOP, NULL, /*hInst*/NULL, NULL);  
  capture_hwnd=capCreateCaptureWindow("Window", WS_CHILD, 0, 0, 160, 120, hwnd, 1);
  if(capture_hwnd==NULL)
    return 0;
  
  if(capDriverConnect(capture_hwnd, cap_id)==FALSE)
    return 0;
  
  capDriverGetCaps(capture_hwnd, &pcdc, sizeof(CAPDRIVERCAPS));
  if(!pcdc.fCaptureInitialized)
  {
    capture_shutdown();
    return 0;
  }
  
  atexit(capture_shutdown);
  
  dwSize=capGetVideoFormatSize(capture_hwnd);
  pbiInfo=(BITMAPINFO *)malloc(dwSize);
  
  capGetVideoFormat(capture_hwnd, pbiInfo, dwSize);
  
  pbiInfo->bmiHeader.biWidth = 320;
  pbiInfo->bmiHeader.biHeight = 240;
  pbiInfo->bmiHeader.biBitCount = 24;
  
  pbiInfo->bmiHeader.biSizeImage = 0;
  pbiInfo->bmiHeader.biCompression = BI_RGB;
  pbiInfo->bmiHeader.biClrUsed = 0;
  pbiInfo->bmiHeader.biClrImportant = 0;
  pbiInfo->bmiHeader.biPlanes = 1;
  
  pbiInfo->bmiColors->rgbBlue = 0;
  pbiInfo->bmiColors->rgbGreen = 0;
  pbiInfo->bmiColors->rgbRed = 0;
  pbiInfo->bmiColors->rgbReserved = 0;
  
  capSetVideoFormat(capture_hwnd, pbiInfo, dwSize);
  
  capture_buffer = (unsigned char *)malloc(320*240*3);
  memset(capture_buffer, 0, 320*240*3);
  
  capSetCallbackOnFrame(capture_hwnd, onFrame);
  
  capGrabFrameNoStop(capture_hwnd);
  
  return 1;
  
}

unsigned char *capture_frame()
{
  capGrabFrameNoStop(capture_hwnd);
  return capture_buffer;
}

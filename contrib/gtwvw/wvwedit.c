/*
* $Id: wvwedit.c 9468 2012-06-20 07:24:49Z andijahja $
 */
/*
 * wvwedit.C
 * Video subsystem for Win32 using GUI windows instead of Console
 * with multiple windows support
 *   Copyright 2004 Budyanto Dj. <budyanto@centrin.net.id>
 * gtwvw edit functions
 * GTWVW is initially created based on:
 *
 * =Id: gtwvt.c,v 1.60 2004/01/26 08:14:07 vouchcac Exp =
 *
 * Harbour Project source code:
 * Video subsystem for Win32 using GUI windows instead of Console
 *     Copyright 2003 Peter Rees <peter@rees.co.nz>
 *                    Rees Software & Systems Ltd
 * based on
 *   Bcc ConIO Video subsystem by
 *     Copyright 2002 Marek Paliwoda <paliwoda@inteia.pl>
 *     Copyright 2002 Przemys�aw Czerpak <druzus@polbox.com>
 *   Video subsystem for Win32 compilers
 *     Copyright 1999-2000 Paul Tucker <ptucker@sympatico.ca>
 *     Copyright 2002 Przemys�aw Czerpak <druzus@polbox.com>
 *
 * The following parts are Copyright of the individual authors.
 * www - http://www.harbour-project.org
 *
 *
 * Copyright 1999 David G. Holm <dholm@jsd-llc.com>
 *    hb_gt_wvw_Tone()
 *
 * See doc/license.txt for licensing terms.
 *
 * www - http://www.harbour-project.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option )
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.   If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307 USA (or visit the web site http://www.gnu.org/ ).
 *
 * As a special exception, the Harbour Project gives permission for
 * additional uses of the text contained in its release of Harbour.
 *
 * The exception is that, if you link the Harbour libraries with other
 * files to produce an executable, this does not by itself cause the
 * resulting executable to be covered by the GNU General Public License.
 * Your use of that executable is in no way restricted on account of
 * linking the Harbour library code into it.
 *
 * This exception does not however invalidate any other reasons why
 * the executable file might be covered by the GNU General Public License.
 *
 * This exception applies only to the code released by the Harbour
 * Project under the name Harbour.  If you copy code from other
 * Harbour Project or Free Software Foundation releases into a copy of
 * Harbour, as the General Public License permits, the exception does
 * not apply to the code that you add in this way.   To avoid misleading
 * anyone as to the status of such modified files, you must delete
 * this exception notice from them.
 *
 * If you write modifications of your own for Harbour, it is your choice
 * whether to permit this exception to apply to your modifications.
 * If you do not wish that, delete this exception notice.
 *
 */

#if defined( _MSC_VER ) && ( _MSC_VER >= 1400 )
   #if !defined( _CRT_SECURE_NO_WARNINGS )
      #define _CRT_SECURE_NO_WARNINGS
   #endif
#endif

#include "hbole.h"
#include <tchar.h>
#include <stdlib.h>

#define TTS_BALLOON             0x40 // added by MAG

#define WINVER 0x0500
#define _WIN32_WINNT 0x0500

  #ifndef _WIN32_IE
    #define _WIN32_IE 0x0400
#endif

#include "hbgtwvw.h"

#include <windows.h>
#include <commctrl.h>


/*WVW_EBcreate( [nWinNum], nTop, nLeft, nBottom, nRight, cText, bBlock, ;
 *                         lMultiline, nMoreStyle, nMaxChar, nReserved, aOffset)
 *create editbox for window nWinNum
 *nTop: row of top/left corner (in character unit)
 *nLeft: col of top/left corner (in character unit)
 *nBottom: row of bottom/right corner (in character unit)
 *nRight: col of bottom/right corner (in character unit)
 *cText: initial text to display, default = ""
 *       WARNING!! must be of "C" typed!
 *bBlock: codeblock to execute on these events:
 *         event=EN_SETFOCUS(...): editbox got focus
 *         event=EN_KILLFOCUS(...): editbox lose focus
 *         This codeblock will be evaluated with these parameters:
 *         nWinNum: window number
 *         nEBid  : editbox id
 *         nType  : event type (EN_SETFOCUS/EN_KILLFOCUS supported)
 *
 *
 *lMultiline: .f. :: single line editbox (default)
 *            .t. :: multi line editbox
 * mapped internally into two types of editbox:
 *         WVW_EB_SINGLELINE (1): single line editbox
 *         WVW_EB_MULTILINE (2): multi line editbox
 *         default is WVW_EB_SINGLELINE (1)
 *
 *nMoreStyle: more style that will be added to the predefined style
 *            some examples: ES_PASSWORD, ES_READONLY
 *
 *nMaxChar: (FUTURE FEATURE) maximum number of chars allowed
 *
 *nReserved: reserved for future use
 *
 *aOffset: array {y1,x1,y2,x2} of offsets to corner pixels, to adjust
 *         dimension of editbox.
 *         defaults: {-2,-2,+2,+2}
 *
 *returns control id of newly created editbox of windows nWinNum
 *returns 0 if failed
 *
 *example:
 */

HB_FUNC( WVW_EBCREATE)
{
   UINT usWinNum = WVW_WHICH_WINDOW;
   WIN_DATA * pWindowData = hb_gt_wvw_GetWindowsData( usWinNum );
   HWND hWndParent = pWindowData->hWnd;
   HWND hWndEB;
   POINT xy = { 0 };
   int   iTop, iLeft, iBottom, iRight;
   int   iOffTop, iOffLeft, iOffBottom, iOffRight;
   UINT  uiEBid;
   USHORT   usTop    = ( USHORT )hb_parni( 2 ),
            usLeft   = ( USHORT )hb_parni( 3 ),
            usBottom = ( USHORT )hb_parni( 4 ),
            usRight  = ( USHORT )hb_parni( 5 );
   LPTSTR   lpszText = (LPTSTR) hb_parcx( 6 );

   BOOL     bMultiline = (ISLOG(8) ? hb_parl(8) : FALSE);
   BYTE     bEBType = (BYTE) (bMultiline ? WVW_EB_MULTILINE : WVW_EB_SINGLELINE);

   DWORD    dwMoreStyle = (DWORD) (ISNUM(9) ? hb_parnl(9) : 0);

   USHORT   usMaxChar = (USHORT) (ISNUM(10) && hb_parni(10)>0 ? hb_parni(10) : 0);

   DWORD    dwStyle;
   WVW_DATA * pData=  hb_getWvwData( ) ;
   if (pWindowData->hEBfont==NULL)
   {
      pWindowData->hEBfont = CreateFontIndirect( &pData->s_lfEB );
      if (pWindowData->hEBfont==NULL)
      {
        hb_retnl(0);
        return;
      }
   }

   iOffTop    = !ISNIL( 12 ) ? hb_parni( 12,1 ) : 0;
   iOffLeft   = !ISNIL( 12 ) ? hb_parni( 12,2 ) : 0;
   iOffBottom = !ISNIL( 12 ) ? hb_parni( 12,3 ) : 0;
   iOffRight  = !ISNIL( 12 ) ? hb_parni( 12,4 ) : 0;

   if (hb_gt_wvw_GetMainCoordMode())
   {
     hb_wvw_HBFUNCPrologue(usWinNum, &usTop, &usLeft, &usBottom, &usRight);
   }

   xy      = hb_gt_wvwGetXYFromColRow( pWindowData, usLeft, usTop );
   iTop    = xy.y + iOffTop ;
   iLeft   = xy.x + iOffLeft;

   xy      = hb_gt_wvwGetXYFromColRow( pWindowData, usRight + 1, usBottom + 1 );

   xy.y   -= pWindowData->byLineSpacing;

   iBottom = xy.y - 1 + iOffBottom;
   iRight  = xy.x - 1 + iOffRight;

   uiEBid = LastControlId(usWinNum, WVW_CONTROL_EDITBOX);
   if (uiEBid==0)
   {
     uiEBid = WVW_ID_BASE_EDITBOX;
   }
   else
   {
     uiEBid++;
   }

   dwStyle = WS_BORDER | WS_GROUP | WS_TABSTOP | dwMoreStyle;

   if ((bEBType & WVW_EB_MULTILINE) == WVW_EB_MULTILINE)
   {
     dwStyle |= ES_AUTOVSCROLL | ES_MULTILINE |
                ES_WANTRETURN | WS_BORDER | WS_VSCROLL;
   }
   else
   {
     dwStyle |= ES_AUTOHSCROLL;
   }

   if ( pWindowData->CodePage == OEM_CHARSET )
   {
     dwStyle |= ES_OEMCONVERT;

   }

   hWndEB = CreateWindowEx(
       0L,
       "EDIT",
       (LPSTR) NULL,
       WS_CHILD | WS_VISIBLE | (DWORD) dwStyle,
       iLeft,
       iTop,
       iRight-iLeft+1,
       iBottom-iTop+1,
       hWndParent,
       (HMENU) uiEBid,
       (HINSTANCE) hb_hInstance,
       (LPVOID) NULL
   );

   if(hWndEB)
   {
     RECT rXB = { 0 }, rOffXB = { 0 };
     WNDPROC OldProc;
     //USHORT i;
     BOOL    bFromOEM = ( pWindowData->CodePage == OEM_CHARSET );

     if (bFromOEM)
     {
       ULONG ulLen = (ULONG) strlen(lpszText);
       LPTSTR lpszTextANSI = (LPTSTR) hb_xgrab(ulLen+1);
       OemToChar( lpszText, lpszTextANSI );
       lpszText = lpszTextANSI;
     }

     SendMessage(
       (HWND) hWndEB,
       WM_SETTEXT,
       0,
       (LPARAM) lpszText
     );

     if (bFromOEM)
     {
       hb_xfree(lpszText);
     }

     if (usMaxChar>0)
     {
       SendMessage(
         (HWND) hWndEB,
         EM_LIMITTEXT,
         (WPARAM) usMaxChar,
         (LPARAM) 0
       );
     }

     rXB.top = usTop;     rXB.left= usLeft;
     rXB.bottom=usBottom; rXB.right =usRight;
     rOffXB.top = iOffTop;     rOffXB.left= iOffLeft;
     rOffXB.bottom=iOffBottom; rOffXB.right =iOffRight;

     AddControlHandle(usWinNum, WVW_CONTROL_EDITBOX, hWndEB, uiEBid, (HB_ITEM *) hb_param( 7, HB_IT_BLOCK ), rXB, rOffXB, (byte) bEBType);

     OldProc = (WNDPROC) SetWindowLong (hWndEB,
                                        GWL_WNDPROC, (LONG) hb_gt_wvwEBProc) ;

     StoreControlProc(usWinNum, WVW_CONTROL_EDITBOX, hWndEB, OldProc);

     SendMessage( hWndEB, WM_SETFONT, (WPARAM) pWindowData->hEBfont, (LPARAM) TRUE);

     hb_retnl( (LONG) uiEBid );
   }
   else
   {

     hb_retnl( (LONG) 0 );
   }
}

/*WVW_EBdestroy( [nWinNum], nEBid )
 *destroy editbox nEBid for window nWinNum
 */
HB_FUNC( WVW_EBDESTROY)
{
   UINT usWinNum = WVW_WHICH_WINDOW;
   WIN_DATA * pWindowData = hb_gt_wvw_GetWindowsData( usWinNum );
   UINT uiEBid = (UINT) ( ISNIL( 2 ) ? 0  : hb_parni( 2 ) );
   CONTROL_DATA * pcd = pWindowData->pcdCtrlList;
   CONTROL_DATA * pcdPrev = (CONTROL_DATA *) NULL;

   while (pcd)
   {
     if (pcd->byCtrlClass == WVW_CONTROL_EDITBOX && pcd->uiCtrlid == uiEBid)
     {
       break;
     }

     pcdPrev = pcd;
     pcd = pcd->pNext;
   }
   if (pcd==NULL) { return; }

   DestroyWindow (pcd->hWndCtrl) ;

   if (pcdPrev==NULL)
   {
     pWindowData->pcdCtrlList = pcd->pNext;
   }
   else
   {
     pcdPrev->pNext = pcd->pNext;
   }

   if (pcd->phiCodeBlock)
   {
      hb_itemRelease( pcd->phiCodeBlock );

   }

   hb_xfree( pcd );
}

/*WVW_EBsetFocus( [nWinNum], nEditId )
 *set the focus to editbox nEditId in window nWinNum
 */
HB_FUNC( WVW_EBSETFOCUS )
{
  UINT usWinNum = WVW_WHICH_WINDOW;
  UINT   uiCtrlId = ISNIL(2) ? 0 : hb_parni(2);
  byte   bStyle;
  HWND   hWndEB = FindControlHandle(usWinNum, WVW_CONTROL_EDITBOX, uiCtrlId, &bStyle);

  if (hWndEB)
  {
    hb_retl( SetFocus(hWndEB) != NULL );
  }
  else
  {
    hb_retl(FALSE);
  }
}

/*WVW_EBisFocused( [nWinNum], nEditId )
 *returns .t. if the focus is on editbox nEditId in window nWinNum
 */
HB_FUNC( WVW_EBISFOCUSED )
{
  UINT usWinNum = WVW_WHICH_WINDOW;
  UINT   uiCtrlId = ISNIL(2) ? 0 : hb_parni(2);
  byte   bStyle;
  HWND   hWndEB = FindControlHandle(usWinNum, WVW_CONTROL_EDITBOX, uiCtrlId, &bStyle);

  hb_retl((HWND) GetFocus() == hWndEB);
}

/*WVW_EBenable( [nWinNum], nEditId, [lEnable] )
 *enable/disable editbox nEditId on window nWinNum
 *(lEnable defaults to .t., ie. enabling the editbox)
 *return previous state of the editbox (TRUE:enabled FALSE:disabled)
 *(if nEditId is invalid, this function returns FALSE too)
 */
HB_FUNC( WVW_EBENABLE )
{
  UINT usWinNum = WVW_WHICH_WINDOW;
  UINT   uiCtrlId = ISNIL(2) ? 0 : hb_parni(2);
  BOOL   bEnable  = ISNIL(3) ? TRUE : hb_parl(3);
  byte   bStyle;
  HWND   hWndEB = FindControlHandle(usWinNum, WVW_CONTROL_EDITBOX, uiCtrlId, &bStyle);
  WIN_DATA * pWindowData = hb_gt_wvw_GetWindowsData( usWinNum );  

  if (hWndEB)
  {
    hb_retl( EnableWindow(hWndEB, bEnable)==0 );

    if (!bEnable)
    {
       SetFocus( pWindowData->hWnd );
    }
  }
  else
  {
    hb_retl(FALSE);
  }
}

/*WVW_EBeditable( [nWinNum], nEditId, [lEditable] )
 *get/set editability attribute from editbox nEditId on window nWinNum
 *(if lEditable is not specified, no change to editability)
 *return previous state of the editbox (TRUE:editable FALSE:not editable)
 *(if nEditId is invalid, this function returns FALSE too)
 */
HB_FUNC( WVW_EBEDITABLE )
{
  UINT usWinNum = WVW_WHICH_WINDOW;
  UINT   uiCtrlId = ISNIL(2) ? 0 : hb_parni(2);
  BOOL   bEditable  = ISNIL(3) ? TRUE : hb_parl(3);
  byte   bStyle;
  HWND   hWndEB = FindControlHandle(usWinNum, WVW_CONTROL_EDITBOX, uiCtrlId, &bStyle);

  if (hWndEB)
  {
    DWORD dwStyle = (DWORD) GetWindowLong( hWndEB, GWL_STYLE );

    hb_retl( !((dwStyle & ES_READONLY)==ES_READONLY) );

    if (!ISNIL(3))
    {
      SendMessage(
                   (HWND) hWndEB,
                   EM_SETREADONLY,
                   (WPARAM) !bEditable,
                   (LPARAM) 0
                 );
    }
  }
  else
  {
    hb_retl(FALSE);
  }
}

/*WVW_EBsetcodeblock( [nWinNum], nEBid, bBlock )
 *assign (new) codeblock bBlock to editbox nEBid for window nWinNum
 *
 * return .t. if successful
 */
HB_FUNC( WVW_EBSETCODEBLOCK )
{
   UINT usWinNum = WVW_WHICH_WINDOW;

   UINT uiEBid = (UINT) ( ISNIL( 2 ) ? 0  : hb_parni( 2 ) );
   CONTROL_DATA * pcd = GetControlData(usWinNum, WVW_CONTROL_EDITBOX, NULL, uiEBid);
   WVW_DATA * pData=  hb_getWvwData( ) ;   
   PHB_ITEM phiCodeBlock = hb_param( 3, HB_IT_BLOCK );
   BOOL bOldSetting = pData->s_bRecurseCBlock;

   if (!phiCodeBlock || pcd==NULL || pcd->bBusy)
   {
     hb_retl( FALSE );
     return;
   }

   pData->s_bRecurseCBlock = FALSE;
   pcd->bBusy = TRUE;

   if (pcd->phiCodeBlock)
   {
      hb_itemRelease( pcd->phiCodeBlock );

   }

   pcd->phiCodeBlock = hb_itemNew( phiCodeBlock );

   pcd->bBusy = FALSE;
   pData->s_bRecurseCBlock = bOldSetting;

   hb_retl( TRUE );
}

/*WVW_EBSetFont([nWinNum], cFontFace, nHeight, nWidth, nWeight, nQUality,;
 *                             lItalic, lUnderline, lStrikeout
 *
 *this will initialize font for ALL editboxes in window nWinNum
 *(including ones created later on)
 *
 *TODO: ? should nHeight be ignored, and always forced to use standard char height?
 */
HB_FUNC( WVW_EBSETFONT )
{
   UINT usWinNum = WVW_WHICH_WINDOW;
   WIN_DATA * pWindowData = hb_gt_wvw_GetWindowsData( usWinNum );
   BOOL  retval = TRUE;
   WVW_DATA * pData=  hb_getWvwData( ) ;   

   pData->s_lfEB.lfHeight         = ISNIL( 3 ) ? pWindowData->fontHeight - 2 : hb_parnl( 3 );
   pData->s_lfEB.lfWidth          = ISNIL( 4 ) ? pData->s_lfEB.lfWidth : hb_parni( 4 );
   pData->s_lfEB.lfEscapement     = 0;
   pData->s_lfEB.lfOrientation    = 0;
   pData->s_lfEB.lfWeight         = ISNIL( 5 ) ? pData->s_lfEB.lfWeight : hb_parni( 5 );
   pData->s_lfEB.lfItalic         = ISNIL( 7 ) ? pData->s_lfEB.lfItalic    : ( BYTE )hb_parl( 7 );
   pData->s_lfEB.lfUnderline      = ISNIL( 8 ) ? pData->s_lfEB.lfUnderline : ( BYTE )hb_parl( 8 );
   pData->s_lfEB.lfStrikeOut      = ISNIL( 9 ) ? pData->s_lfEB.lfStrikeOut : ( BYTE )hb_parl( 9 );
   pData->s_lfEB.lfCharSet        = DEFAULT_CHARSET;

   pData->s_lfEB.lfQuality        = ISNIL( 6 ) ? pData->s_lfEB.lfQuality : ( BYTE )hb_parni( 6 );
   pData->s_lfEB.lfPitchAndFamily = FF_DONTCARE;
   if ( ISCHAR( 2 ) )
   {
      strcpy( pData->s_lfEB.lfFaceName, hb_parcx( 2 ) );
   }

   if (pWindowData->hEBfont)
   {
      HFONT hOldFont = pWindowData->hEBfont;
      HFONT hFont = CreateFontIndirect( &pData->s_lfEB );
      if (hFont)
      {
         CONTROL_DATA * pcd = pWindowData->pcdCtrlList;

         while (pcd)
         {
           if ((pcd->byCtrlClass == WVW_CONTROL_EDITBOX) &&
               ((HFONT) SendMessage( pcd->hWndCtrl, WM_GETFONT, (WPARAM) 0, (LPARAM) 0) == hOldFont)
              )
           {
              SendMessage( pcd->hWndCtrl, WM_SETFONT, (WPARAM) hFont, (LPARAM) TRUE);

           }

           pcd = pcd->pNext;
         }

         pWindowData->hEBfont = hFont;
         DeleteObject( (HFONT) hOldFont );

      }
      else
      {
         retval = FALSE;
      }
   }

   hb_retl( retval );

}

/*WVW_EBIsMultiline( [nWinNum], nEBid )
 *returns .t. if editbox nEBid in window nWinNum is multiline
 *otherwise .f.
 *Also returns .f. if nEBid not valid
 */
HB_FUNC( WVW_EBISMULTILINE )
{
   UINT usWinNum = WVW_WHICH_WINDOW;
   UINT uiEBid = hb_parni(2);
   CONTROL_DATA * pcd = GetControlData(usWinNum, WVW_CONTROL_EDITBOX, NULL, uiEBid);
   BOOL bMultiline;

   if (pcd==NULL)
   {
      hb_retl(FALSE);
      return;
   }

   bMultiline = ((pcd->bStyle & WVW_EB_MULTILINE) == WVW_EB_MULTILINE);
   hb_retl(bMultiline);
}

/*WVW_EBgettext( [nWinNum], nEBid,;
 *                          lSoftBreak )
 *returns current text from editbox nEBid in window nWinNum
 *lSoftBreak: Default is FALSE.
 *             insert soft line break character (CR+CR+LF) at wordwrap positions
 *             can be usefull to convert the text to MEMO format
 *             eg. converting editbox's softbreaks into memoline softbreak:
 *                cStr := wvw_ebgettext(NIL, nEBid, .t.)
 *                cStr := strtran(cStr, CR+CR+LF, chr(141)+LF)
 *
 *returns "" in case of error (eg. nEBid not valid)
 */
HB_FUNC( WVW_EBGETTEXT )
{
   UINT usWinNum = WVW_WHICH_WINDOW;
   UINT uiEBid = hb_parni(2);
   CONTROL_DATA * pcd = GetControlData(usWinNum, WVW_CONTROL_EDITBOX, NULL, uiEBid);
   BOOL bSoftBreak = (ISLOG(3) ? hb_parl(3) : FALSE);
   USHORT usLen;
   WIN_DATA * pWindowData = hb_gt_wvw_GetWindowsData( usWinNum );  
   LPTSTR   lpszTextANSI;
   BOOL     bToOEM = ( pWindowData->CodePage == OEM_CHARSET );

   if (pcd==NULL)
   {
      hb_retl(FALSE);
      return;
   }

   if (bSoftBreak)
   {
      SendMessage(
        (HWND) pcd->hWndCtrl,
        EM_FMTLINES,
        (WPARAM) TRUE,
        (LPARAM) 0
      );

   }

   usLen = ( USHORT )SendMessage( (HWND) pcd->hWndCtrl, WM_GETTEXTLENGTH, 0, 0 ) + 1 ;

   lpszTextANSI = ( LPTSTR ) hb_xgrab( usLen );

   SendMessage(
     (HWND) pcd->hWndCtrl,
     WM_GETTEXT,
     usLen,
     (LPARAM) lpszTextANSI
   );

   if (bToOEM)
   {
     ULONG ulLen = (ULONG) strlen(lpszTextANSI);
     LPTSTR lpszText = (LPTSTR) hb_xgrab(ulLen+1);
     CharToOem( lpszTextANSI, lpszText );
     hb_retc( lpszText );
     hb_xfree( lpszText );
   }
   else
   {
     hb_retc( lpszTextANSI );
   }

   hb_xfree( lpszTextANSI );
}

/*WVW_EBsettext( [nWinNum], nEBid, cText )
 *set current text of editbox nEBid in window nWinNum
 *returns .t. if successful, .f. in case of error (eg. nEBid not valid)
 */
HB_FUNC( WVW_EBSETTEXT )
{
   UINT usWinNum = WVW_WHICH_WINDOW;
   UINT uiEBid = hb_parni(2);
   WIN_DATA * pWindowData = hb_gt_wvw_GetWindowsData( usWinNum );     
   CONTROL_DATA * pcd = GetControlData(usWinNum, WVW_CONTROL_EDITBOX, NULL, uiEBid);
   BOOL bRetval;
   LPTSTR   lpszText = (LPTSTR) hb_parcx( 3 );
   BOOL     bFromOEM = ( pWindowData->CodePage == OEM_CHARSET );

   if (pcd==NULL)
   {
      hb_retl(FALSE);
      return;
   }

   if (bFromOEM)
   {
     ULONG ulLen = (ULONG) strlen(lpszText);
     LPTSTR lpszTextANSI = (LPTSTR) hb_xgrab(ulLen+1);
     OemToChar( lpszText, lpszTextANSI );
     lpszText = lpszTextANSI;
   }

   bRetval = SendMessage(
               (HWND) pcd->hWndCtrl,
               WM_SETTEXT,
               0,
               (LPARAM) lpszText
             );

   if (bFromOEM)
   {
     hb_xfree(lpszText);
   }

   hb_retl( bRetval );
}

/*WVW_EBgetsel( [nWinNum], nEBid, @nstart, @nend )
 *get selected text editbox nEBid in window nWinNum
 *the start selected text (0-based) is in nstart
 *the end selected text (0-based) is in nend
 *returns .t. if operation successful
 *returns .f. if not (eg. nEBid not valid)
 */
HB_FUNC( WVW_EBGETSEL )
{
   UINT usWinNum = WVW_WHICH_WINDOW;
   UINT uiEBid = hb_parni(2);
   CONTROL_DATA * pcd = GetControlData(usWinNum, WVW_CONTROL_EDITBOX, NULL, uiEBid);
   DWORD dwStart, dwEnd;

   if (pcd==NULL)
   {
      hb_retl(FALSE);
      return;
   }

   SendMessage( (HWND) pcd->hWndCtrl,
                EM_GETSEL,
                (WPARAM) &dwStart,
                (LPARAM) &dwEnd
              );

   if (ISBYREF(3)) hb_stornl( dwStart, 3 );
   if (ISBYREF(4)) hb_stornl( dwEnd, 4 );
   hb_retl(TRUE);
}

/*WVW_EBsetsel( [nWinNum], nEBid, nstart, nend )
 *set selected text editbox nEBid in window nWinNum
 *the start selected text (0-based) is in nstart
 *the end selected text (0-based) is in nend
 *notes: nstart may be > nend (flipped selection)
 *notes: to selet all text: WVW_EBsetsel(nwinnum, nebid, 0, -1)
 *returns .t. if operation successful
 *returns .f. if not (eg. nEBid not valid)
 */
HB_FUNC( WVW_EBSETSEL )
{
   UINT usWinNum = WVW_WHICH_WINDOW;
   UINT uiEBid = hb_parni(2);
   CONTROL_DATA * pcd = GetControlData(usWinNum, WVW_CONTROL_EDITBOX, NULL, uiEBid);
   DWORD dwStart = (DWORD) (ISNUM(3) ? hb_parnl(3) : 0);
   DWORD dwEnd   = (DWORD) (ISNUM(4) ? hb_parnl(4) : 0);

   if (pcd==NULL)
   {
      hb_retl(FALSE);
      return;
   }

   SendMessage( (HWND) pcd->hWndCtrl,
                EM_SETSEL,
                (WPARAM) dwStart,
                (LPARAM) dwEnd
              );
   hb_retl(TRUE);
}
//Static controls

HB_FUNC( WVW_STCREATE )
{
   UINT usWinNum = WVW_WHICH_WINDOW;
   WIN_DATA * pWindowData = hb_gt_wvw_GetWindowsData( usWinNum );
   WVW_DATA * pData =  hb_getWvwData( ) ;      
   HWND hWndParent = pWindowData->hWnd;
   HWND hWndCB;
   //RECT r;
//   HDC hDc;

   POINT xy = { 0 };
   int   iTop, iLeft, iBottom, iRight;
   int   iOffTop, iOffLeft, iOffBottom, iOffRight;
   UINT uiCBid;
   BOOL bBorder = hb_parnl( 7 ) ;
   ULONG ulExStyle = 0  | ( bBorder ? WS_EX_CLIENTEDGE:0 );

   USHORT   usWidth  = ( USHORT )hb_parni( 4 );
   USHORT   usTop    = ( USHORT )hb_parni( 2 ),
            usLeft   = ( USHORT )hb_parni( 3 ),
            usBottom = ISNUM(11) ? ( USHORT )hb_parni(11) : usTop,
            usRight  = ISNUM(12) ? ( USHORT )hb_parni( 12 ) : usLeft + usWidth - 1;
   //char * sText = hb_parc( 5 );


   int  iStyle =  ( bBorder ? WS_BORDER :0 ) ;
   int  iBox = ISNUM(10) ? hb_parni( 10 ) : 0;
   HFONT hFont = NULL;
   if (iBox > 0 )
      iStyle |= iBox;


   if (ISNUM(8))
      hFont = (HFONT) HB_PARHANDLE( 8 ) ;
   else
      if (pWindowData->hSTfont==NULL)
      {
         pWindowData->hSTfont = CreateFontIndirect( &pData->s_lfST );
        if (pWindowData->hSTfont==NULL)
         {
           hb_retnl(0);
           return;
         }
      }



   iOffTop    = !ISNIL( 6 ) ? hb_parni( 6,1 ) : 0;
   iOffLeft   = !ISNIL( 6 ) ? hb_parni( 6,2 ) : 0;

   iOffBottom = !ISNIL( 6 ) ? hb_parni( 6,3 ) : 0;
   iOffRight  = !ISNIL( 6 ) ? hb_parni( 6,4 ) : 0;

   if (hb_gt_wvw_GetMainCoordMode())
   {
     hb_wvw_HBFUNCPrologue(usWinNum, &usTop, &usLeft, &usBottom, &usRight);
   }

   xy      = hb_gt_wvwGetXYFromColRow( pWindowData, usLeft, usTop );
   iTop    = xy.y + iOffTop ;
   iLeft   = xy.x + iOffLeft;

   xy      = hb_gt_wvwGetXYFromColRow( pWindowData, usRight + 1, usBottom + 1 );

   xy.y   -= pWindowData->byLineSpacing;

   iBottom = xy.y - 1 + iOffBottom ;
   iRight  = xy.x - 1 + iOffRight;

   uiCBid = LastControlId( usWinNum, WVW_CONTROL_STATIC);
   if (uiCBid==0)
   {
     uiCBid = WVW_ID_BASE_STATIC;
   }
   else
   {
     uiCBid++;
   }

   hWndCB = CreateWindowEx(
       ulExStyle,
       "STATIC",
       (LPSTR) NULL,
       WS_CHILD | WS_VISIBLE | (DWORD) iStyle,
       iLeft,
       iTop,
       iRight-iLeft+1,
       iBottom-iTop+1,
       hWndParent,
       (HMENU) uiCBid,
       (HINSTANCE) hb_hInstance,
       (LPVOID) NULL
   );

   if(hWndCB)
   {
     RECT rXB = { 0 }, rOffXB = { 0 };

//     WNDPROC OldProc;

     rXB.top = usTop;     rXB.left= usLeft;
     rXB.bottom=usBottom; rXB.right =usRight;
     rOffXB.top = iOffTop;     rOffXB.left= iOffLeft;
     rOffXB.bottom=iOffBottom; rOffXB.right =iOffRight;


     if( ISCHAR(5))
     {
        SendMessage( hWndCB, WM_SETTEXT, 0, (LPARAM) hb_parc(5) );
     }
     if (hFont)
        SendMessage( hWndCB, WM_SETFONT, (WPARAM) hFont, (LPARAM) TRUE);
     else
        SendMessage( hWndCB, WM_SETFONT, (WPARAM) pWindowData->hSTfont, (LPARAM) TRUE);
     hb_retnl( (LONG) uiCBid );
     HB_STOREHANDLE( hWndCB ,9);
   }
   else
   {

     hb_retnl( (LONG) 0 );
   }
}


HB_FUNC( WVW_STSETTEXT )
{
//  byte   bStyle;
  HWND   hWndCB = (HWND) HB_PARHANDLE( 2 );

  if (hWndCB)
  {

   SetWindowText( (HWND) hWndCB, (LPCTSTR) hb_parc( 3 ) );
   hb_retl(1);
  }
  else
  {
    hb_retl(FALSE);
  }
}


HB_FUNC( WVW_STSETFONT )
{
   UINT usWinNum = WVW_WHICH_WINDOW;
   WIN_DATA * pWindowData = hb_gt_wvw_GetWindowsData( usWinNum );
   WVW_DATA * pData=  hb_getWvwData( ) ;   
   BOOL  retval = TRUE;

   pData->s_lfST.lfHeight         = ISNIL( 3 ) ? pWindowData->fontHeight - 2 : hb_parnl( 3 );
   pData->s_lfST.lfWidth          = ISNIL( 4 ) ? pData->s_lfST.lfWidth : hb_parni( 4 );
   pData->s_lfST.lfEscapement     = 0;
   pData->s_lfST.lfOrientation    = 0;
   pData->s_lfST.lfWeight         = ISNIL( 5 ) ? pData->s_lfST.lfWeight : hb_parni( 5 );
   pData->s_lfST.lfItalic         = ISNIL( 7 ) ? pData->s_lfST.lfItalic    : ( BYTE )hb_parl( 7 );
   pData->s_lfST.lfUnderline      = ISNIL( 8 ) ? pData->s_lfST.lfUnderline : ( BYTE )hb_parl( 8 );
   pData->s_lfST.lfStrikeOut      = ISNIL( 9 ) ? pData->s_lfST.lfStrikeOut : ( BYTE )hb_parl( 9 );
   pData->s_lfST.lfCharSet        = DEFAULT_CHARSET;

   pData->s_lfST.lfQuality        = ISNIL( 6 ) ? pData->s_lfST.lfQuality : ( BYTE )hb_parni( 6 );
   pData->s_lfST.lfPitchAndFamily = FF_DONTCARE;
   if ( ISCHAR( 2 ) )
   {
      strcpy( pData->s_lfST.lfFaceName, hb_parcx( 2 ) );
   }

   if (pWindowData->hSTfont)
   {
      HFONT hOldFont = pWindowData->hSTfont;
      HFONT hFont = CreateFontIndirect( &pData->s_lfST );
      if (hFont)
      {
         CONTROL_DATA * pcd = pWindowData->pcdCtrlList;

         while (pcd)
         {
           if ((pcd->byCtrlClass == WVW_CONTROL_STATIC) &&
               ((HFONT) SendMessage( pcd->hWndCtrl, WM_GETFONT, (WPARAM) 0, (LPARAM) 0) == hOldFont)
              )
           {
              SendMessage( pcd->hWndCtrl, WM_SETFONT, (WPARAM) hFont, (LPARAM) TRUE);

           }

           pcd = pcd->pNext;
         }

         pWindowData->hSTfont = hFont;
         DeleteObject( (HFONT) hOldFont );

      }
      else
      {
         retval = FALSE;
      }
   }

   hb_retl( retval );

}


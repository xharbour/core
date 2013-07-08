/*
 * $Id: wvtutils.c 9773 2012-10-19 08:29:51Z andijahja $
 */

/*
 * Harbour Project source code:
 * Video subsystem for Win32 using GUI windows instead of Console
 *
 *    Copyright 2007 Pritpal Bedi <pritpal@vouchcac.com>
 * based on:
 *
 *    Copyright 2003 Peter Rees <peter@rees.co.nz>
 *                    Rees Software & Systems Ltd
 * based on
 *   Bcc ConIO Video subsystem by
 *     Copyright 2002 Marek Paliwoda <paliwoda@inteia.pl>
 *     Copyright 2002 Przemyslaw Czerpak <druzus@polbox.com>
 *   Video subsystem for Win32 compilers
 *     Copyright 1999-2000 Paul Tucker <ptucker@sympatico.ca>
 *     Copyright 2002 Przemys�aw Czerpak <druzus@polbox.com>
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

//-------------------------------------------------------------------//

#define HB_OS_WIN_USED

//-------------------------------------------------------------------//

#include "gtwvg.h"

/* workaround for missing declaration in MinGW32 */
#if !defined(TTM_SETTITLE) && defined(TTM_SETTITLEA)
   #define TTM_SETTITLE TTM_SETTITLEA
#endif

#if defined(__MINGW32CE__)
/* ChooseColorW() problem is fixed in current devel MINGW32CE version but
 * people who use recent official release (0.50) needs it
 */
#undef ChooseColor
BOOL WINAPI ChooseColor( LPCHOOSECOLORW );
#endif /* __MINGW32CE__ */

//-------------------------------------------------------------------//

HB_EXTERN_BEGIN

extern HANDLE  hb_hInstance;

extern BOOL     wvt_Array2Rect(PHB_ITEM aRect, RECT *rc );
extern PHB_ITEM wvt_Rect2Array( RECT *rc  );
extern BOOL     wvt_Array2Point(PHB_ITEM aPoint, POINT *pt );
extern PHB_ITEM wvt_Point2Array( POINT *pt  );
extern BOOL     wvt_Array2Size(PHB_ITEM aSize, SIZE *siz );
extern PHB_ITEM wvt_Size2Array( SIZE *siz  );
extern void     wvt_Rect2ArrayEx( RECT *rc ,PHB_ITEM aRect );
extern void     wvt_Point2ArrayEx( POINT *pt  , PHB_ITEM aPoint);
extern void     wvt_Size2ArrayEx( SIZE *siz ,PHB_ITEM aSize );

HB_EXTERN_END

//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
//
//               Pritpal Bedi <pritpal@vouchcac.com>
//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//

HB_FUNC( WVT_UTILS )
{
   // Retained for legacy code.
}

//-------------------------------------------------------------------//
//
//     Wvt_ChooseFont( cFontName, nHeight, nWidth, nWeight, nQuality, ;
//                                    lItalic, lUnderline, lStrikeout )
//
HB_FUNC( WVT_CHOOSEFONT )
{
   PHB_GTWVT _s = hb_wvt_gtGetWVT();

   CHOOSEFONT  cf;// = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
   LOGFONT     lf;// = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
   LONG        PointSize = 0;

   if ( ! ISNIL( 2 ) )
   {
      PointSize = -MulDiv( ( LONG ) hb_parnl( 2 ), GetDeviceCaps( _s->hdc, LOGPIXELSY ), 72 ) ;
   }

   lf.lfHeight         = PointSize;
   lf.lfWidth          = ISNIL( 3 ) ? 0 : hb_parni( 3 );
   lf.lfWeight         = ISNIL( 4 ) ? 0 : hb_parni( 4 );
   lf.lfItalic         = ISNIL( 6 ) ? 0 : ( BYTE ) hb_parl( 6 );
   lf.lfUnderline      = ISNIL( 7 ) ? 0 : ( BYTE ) hb_parl( 7 );
   lf.lfStrikeOut      = ISNIL( 8 ) ? 0 : ( BYTE ) hb_parl( 8 );
   lf.lfCharSet        = DEFAULT_CHARSET;
   lf.lfQuality        = ISNIL( 5 ) ? DEFAULT_QUALITY : ( BYTE ) hb_parni( 5 );
   lf.lfPitchAndFamily = FF_DONTCARE;
   if ( ISCHAR( 1 ) )
   {
      HB_TCHAR_CPTO( lf.lfFaceName, hb_parc( 1 ), sizeof( lf.lfFaceName ) - 1 );
   }

   cf.lStructSize      = sizeof( CHOOSEFONT );
   cf.hwndOwner        = _s->hWnd;
   cf.hDC              = ( HDC ) NULL;
   cf.lpLogFont        = &lf;
   cf.iPointSize       = 0;
   cf.Flags            = CF_SCREENFONTS | CF_EFFECTS | CF_SHOWHELP | CF_INITTOLOGFONTSTRUCT ;
   cf.rgbColors        = RGB( 0,0,0 );
   cf.lCustData        = 0L;
   cf.lpfnHook         = ( LPCFHOOKPROC ) NULL;
   cf.lpTemplateName   = ( LPTSTR ) NULL;
   cf.hInstance        = ( HINSTANCE ) NULL;
   cf.lpszStyle        = ( LPTSTR ) NULL;
   cf.nFontType        = SCREEN_FONTTYPE;
   cf.nSizeMin         = 0;
   cf.nSizeMax         = 0;

   if ( ChooseFont( &cf ) )
   {
      char * szFaceName = HB_TCHAR_CONVFROM( lf.lfFaceName );

      PointSize = -MulDiv( lf.lfHeight, 72, GetDeviceCaps( _s->hdc, LOGPIXELSY ) ) ;

      hb_reta( 8 );
      hb_storc(  szFaceName        , -1, 1 );
      hb_stornl( ( LONG ) PointSize, -1, 2 );
      hb_storni( lf.lfWidth        , -1, 3 );
      hb_storni( lf.lfWeight       , -1, 4 );
      hb_storni( lf.lfQuality      , -1, 5 );
      hb_storl(  lf.lfItalic       , -1, 6 );
      hb_storl(  lf.lfUnderline    , -1, 7 );
      hb_storl(  lf.lfStrikeOut    , -1, 8 );

      HB_TCHAR_FREE( szFaceName );
   }
   else
   {
      hb_reta( 8 );

      hb_storc(  ""        , -1, 1 );
      hb_stornl( ( LONG ) 0, -1, 2 );
      hb_storni( 0         , -1, 3 );
      hb_storni( 0         , -1, 4 );
      hb_storni( 0         , -1, 5 );
      hb_storl(  0         , -1, 6 );
      hb_storl(  0         , -1, 7 );
      hb_storl(  0         , -1, 8 );
   }

   return ;
}

//-------------------------------------------------------------------//
//
//    Wvt_ChooseColor( nRGBInit, aRGB16, nFlags ) => nRGBSelected
//
HB_FUNC( WVT_CHOOSECOLOR )
{
   PHB_GTWVT _s = hb_wvt_gtGetWVT();

   CHOOSECOLOR cc ;
   COLORREF    crCustClr[ 16 ] ;
   int         i ;

   for( i = 0 ; i < 16 ; i++ )
   {
     crCustClr[ i ] = ( ISARRAY( 2 ) ? ( COLORREF ) hb_parnl( 2, i+1 ) : GetSysColor( COLOR_BTNFACE ) ) ;
   }

   cc.lStructSize   = sizeof( CHOOSECOLOR ) ;
   cc.hwndOwner     = _s->hWnd ;
   cc.rgbResult     = ISNIL( 1 ) ?  0 : ( COLORREF ) hb_parnl( 1 ) ;
   cc.lpCustColors  = crCustClr ;
   cc.Flags         = ( WORD ) ( ISNIL( 3 ) ? CC_ANYCOLOR | CC_RGBINIT | CC_FULLOPEN : hb_parnl( 3 ) );

   if ( ChooseColor( &cc ) )
   {
      hb_retnl( cc.rgbResult ) ;
   }
   else
   {
      hb_retnl( -1 );
   }
}

//-------------------------------------------------------------------//
//
//  Wvt_MessageBox( cMessage, cTitle, nIcon, hWnd )
//
HB_FUNC( WVT_MESSAGEBOX )
{
   PHB_GTWVT _s = hb_wvt_gtGetWVT();

   LPTSTR title = HB_TCHAR_CONVTO( hb_parc( 1 ) );
   LPTSTR msg = HB_TCHAR_CONVTO( hb_parc( 2 ) );
   hb_retni( MessageBox( _s->hWnd, title, msg, ISNIL( 3 ) ? MB_OK : hb_parni( 3 ) ) ) ;
   HB_TCHAR_FREE( title );
   HB_TCHAR_FREE( msg );
}

//-------------------------------------------------------------------//
//#if _WIN32_IE > 0x400
//-------------------------------------------------------------------//
//
//                              Tooltips
//
//-------------------------------------------------------------------//

HB_FUNC( WVT_SETTOOLTIPACTIVE )
{
   PHB_GTWVT _s = hb_wvt_gtGetWVT();

   BOOL bActive = _s->bToolTipActive;

   if ( ! ISNIL( 1 ) )
   {
      _s->bToolTipActive = hb_parl( 1 );
   }

   hb_retl( bActive );
}

//-------------------------------------------------------------------//
//
//   Wvt_SetToolTip( nTop, nLeft, nBottom, nRight, cToolText )
//
HB_FUNC( WVT_SETTOOLTIP )
{
   PHB_GTWVT _s = hb_wvt_gtGetWVT();

   TOOLINFO ti;
   POINT    xy = { 0,0 };
   int      iTop, iLeft, iBottom, iRight;

   if ( ! _s->bToolTipActive )
   {
      return;
   }

   memset( &ti, 0, sizeof( ti ) );
   ti.cbSize    = sizeof( TOOLINFO );
   ti.hwnd      = _s->hWnd;
   ti.uId       = 100000;

   if ( SendMessage( _s->hWndTT, TTM_GETTOOLINFO, 0, ( LPARAM ) &ti ) )
   {
      LPTSTR text = HB_TCHAR_CONVTO( hb_parcx( 5 ) );

      xy      = hb_wvt_gtGetXYFromColRow( ( USHORT ) hb_parni( 2 ), ( USHORT ) hb_parni( 1 ) );
      iTop    = xy.y;
      iLeft   = xy.x;

      xy      = hb_wvt_gtGetXYFromColRow( ( USHORT ) hb_parni( 4 )+1, ( USHORT ) ( hb_parni( 3 )+1 ) );
      iBottom = xy.y - 1;
      iRight  = xy.x - 1;

      ti.lpszText = text;

      ti.rect.left   = iLeft;
      ti.rect.top    = iTop;
      ti.rect.right  = iRight;
      ti.rect.bottom = iBottom;

      SendMessage( _s->hWndTT, TTM_SETTOOLINFO, 0, ( LPARAM ) &ti );

      HB_TCHAR_FREE( text );
   }
}

//-------------------------------------------------------------------//

HB_FUNC( WVT_SETTOOLTIPTEXT )
{
   PHB_GTWVT _s = hb_wvt_gtGetWVT();

   TOOLINFO ti;

   ti.cbSize = sizeof( TOOLINFO );
   ti.hwnd   = _s->hWnd;
   ti.uId    = 100000;

   if ( SendMessage( _s->hWndTT, TTM_GETTOOLINFO, 0, ( LPARAM ) &ti ) )
   {
      LPTSTR text = HB_TCHAR_CONVTO( hb_parcx( 1 ) );
      ti.lpszText = text;
      SendMessage( _s->hWndTT, TTM_UPDATETIPTEXT, 0, ( LPARAM ) &ti );
      HB_TCHAR_FREE( text );
   }
}

//-------------------------------------------------------------------//

HB_FUNC( WVT_SETTOOLTIPMARGIN )
{
#if !defined( __WINCE__ )
   PHB_GTWVT _s = hb_wvt_gtGetWVT();

   RECT rc = { 0,0,0,0 };

   rc.left   = hb_parni( 2 );
   rc.top    = hb_parni( 1 );
   rc.right  = hb_parni( 4 );
   rc.bottom = hb_parni( 3 );

   SendMessage( _s->hWndTT, TTM_SETMARGIN, 0, ( LPARAM ) &rc );
#endif
}

//-------------------------------------------------------------------//

HB_FUNC( WVT_SETTOOLTIPWIDTH )
{
#if !defined( __WINCE__ )
   PHB_GTWVT _s = hb_wvt_gtGetWVT();

   int iTipWidth = SendMessage( _s->hWndTT, TTM_GETMAXTIPWIDTH, 0, 0 );

   if ( ISNUM( 1 ) )
   {
      SendMessage( _s->hWndTT, TTM_SETMAXTIPWIDTH, 0, ( LPARAM ) ( int ) hb_parni( 1 ) );
   }

   hb_retni( iTipWidth );
#endif
}

//-------------------------------------------------------------------//

HB_FUNC( WVT_SETTOOLTIPBKCOLOR )
{
#if !defined( __WINCE__ )
   PHB_GTWVT _s = hb_wvt_gtGetWVT();

   COLORREF cr = SendMessage( _s->hWndTT, TTM_GETTIPBKCOLOR, 0, 0 );

   if ( ISNUM( 1 ) )
   {
      SendMessage( _s->hWndTT, TTM_SETTIPBKCOLOR, ( WPARAM ) ( COLORREF ) hb_parnl( 1 ), 0 );
   }
   hb_retnl( ( COLORREF ) cr );
#endif
}

//-------------------------------------------------------------------//

HB_FUNC( WVT_SETTOOLTIPTEXTCOLOR )
{
#if !defined( __WINCE__ )
   PHB_GTWVT _s = hb_wvt_gtGetWVT();

   COLORREF cr = SendMessage( _s->hWndTT, TTM_GETTIPTEXTCOLOR, 0, 0 );

   if ( ISNUM( 1 ) )
   {
      SendMessage( _s->hWndTT, TTM_SETTIPTEXTCOLOR, ( WPARAM ) ( COLORREF ) hb_parnl( 1 ), 0 );
   }

   hb_retnl( ( COLORREF ) cr );
#endif
}

//-------------------------------------------------------------------//
#if _WIN32_IE > 0x400

HB_FUNC( WVT_SETTOOLTIPTITLE )
{
#if !defined( __WINCE__ )
   PHB_GTWVT _s = hb_wvt_gtGetWVT();

   int iIcon;

   if ( ! ISNIL( 2 ) )
   {
      iIcon = ISNIL( 1 ) ? 0 : hb_parni( 1 );
      if ( iIcon > 3 )
      {
         iIcon = 0 ;
      }
      SendMessage( _s->hWndTT, TTM_SETTITLE, ( WPARAM ) iIcon, ( LPARAM ) hb_parc( 2 ) );
   }
#endif
}

#endif
//-------------------------------------------------------------------//

HB_FUNC( WVT_GETTOOLTIPWIDTH )
{
#if !defined( __WINCE__ )
   PHB_GTWVT _s = hb_wvt_gtGetWVT();

   hb_retni( SendMessage( _s->hWndTT, TTM_GETMAXTIPWIDTH, 0, 0 ) );
#endif
}

//-------------------------------------------------------------------//

HB_FUNC( WVT_GETTOOLTIPBKCOLOR )
{
#if !defined( __WINCE__ )
   PHB_GTWVT _s = hb_wvt_gtGetWVT();

   hb_retnl( ( COLORREF ) SendMessage( _s->hWndTT, TTM_GETTIPBKCOLOR, 0, 0 ) );
#endif
}

//-------------------------------------------------------------------//

HB_FUNC( WVT_GETTOOLTIPTEXTCOLOR )
{
#if !defined( __WINCE__ )
   PHB_GTWVT _s = hb_wvt_gtGetWVT();

   hb_retnl( ( COLORREF ) SendMessage( _s->hWndTT, TTM_GETTIPTEXTCOLOR, 0, 0 ) );
#endif
}

//-------------------------------------------------------------------//
//#endif
//-------------------------------------------------------------------//

HB_FUNC( WVT_SETGUI )
{
   PHB_GTWVT _s = hb_wvt_gtGetWVT();

   BOOL bGui = _s->bGui;

   if ( ISLOG( 1 ) )
   {
      _s->bGui = hb_parl( 1 );
   }

   hb_retl( bGui );
}

//-------------------------------------------------------------------//

HB_FUNC( WVT_SETMOUSEPOS )
{
   PHB_GTWVT _s = hb_wvt_gtGetWVT();

   POINT xy = { 0,0 };

   xy = hb_wvt_gtGetXYFromColRow( ( USHORT ) hb_parni( 2 ), ( USHORT ) hb_parni( 1 ) );

   if ( ClientToScreen( _s->hWnd, &xy ) )
   {
      hb_retl( SetCursorPos( xy.x, xy.y + ( _s->PTEXTSIZE.y / 2 ) ) );
   }
   else
   {
      hb_retl( FALSE );
   }
}

//-------------------------------------------------------------------//

HB_FUNC( WVT_GETPAINTRECT )
{
   PHB_GTWVT _s = hb_wvt_gtGetWVT();

   PHB_ITEM info = hb_itemArrayNew( 4 );

   hb_arraySetNI( info, 1, _s->rowStart );
   hb_arraySetNI( info, 2, _s->colStart );
   hb_arraySetNI( info, 3, _s->rowStop  );
   hb_arraySetNI( info, 4, _s->colStop  );

   hb_itemReturnRelease( info );
}

//-------------------------------------------------------------------//

HB_FUNC( WVT_SETPOINTER )
{
   PHB_GTWVT _s = hb_wvt_gtGetWVT();

   int     iCursor = hb_parni( 1 );
   HCURSOR hCursor;

   switch ( iCursor )
   {
   case 1:
      hCursor = LoadCursor( NULL, IDC_ARROW    );
      break;

   case 2:
      hCursor = LoadCursor( NULL, IDC_IBEAM    );
      break;

   case 3:
      hCursor = LoadCursor( NULL, IDC_WAIT     );
      break;

   case 4:
      hCursor = LoadCursor( NULL, IDC_CROSS    );
      break;

   case 5:
      hCursor = LoadCursor( NULL, IDC_UPARROW  );
      break;

   case 6:
      hCursor = LoadCursor( NULL, IDC_SIZE     );
      break;

   case 7:
      hCursor = LoadCursor( NULL, IDC_ICON     );
      break;

   case 8:
      hCursor = LoadCursor( NULL, IDC_SIZENWSE );
      break;

   case 9:
      hCursor = LoadCursor( NULL, IDC_SIZENESW );
      break;

   case 10:
      hCursor = LoadCursor( NULL, IDC_SIZEWE   );
      break;

   case 11:
      hCursor = LoadCursor( NULL, IDC_SIZENS   );
      break;

   case 12:
      hCursor = LoadCursor( NULL, IDC_SIZEALL  );
      break;

   case 13:
      hCursor = LoadCursor( NULL, IDC_NO       );
      break;

   case 14:
      hCursor = LoadCursor( NULL, IDC_HAND     );
      break;

   case 15:
      hCursor = LoadCursor( NULL, IDC_APPSTARTING );
      break;

   case 16:
      hCursor = LoadCursor( NULL, IDC_HELP     );
      break;

   default:
      hCursor = LoadCursor( NULL, IDC_ARROW    );
      break;
   }

   SetClassLong( _s->hWnd, GCL_HCURSOR, ( DWORD ) hCursor );
}

//-------------------------------------------------------------------//

HB_FUNC( WVT_SETMOUSEMOVE )
{
   PHB_GTWVT _s = hb_wvt_gtGetWVT();

   BOOL bMouseMove = _s->MouseMove;

   if( ISLOG( 1 ) )
     _s->MouseMove = hb_parl( 1 );

   hb_retl( bMouseMove );
}

//-------------------------------------------------------------------//

HB_FUNC( WVT_GETXYFROMROWCOL )
{
   PHB_ITEM info = hb_itemArrayNew( 2 );
   POINT    xy = { 0,0 };

   xy = hb_wvt_gtGetXYFromColRow( ( USHORT ) hb_parni( 2 ), ( USHORT ) hb_parni( 1 ) );

   hb_arraySetNL( info, 1, xy.x );
   hb_arraySetNL( info, 2, xy.y );

   hb_itemReturnRelease( info );
}

//-------------------------------------------------------------------//

HB_FUNC( WVT_GETFONTINFO )
{
   PHB_GTWVT _s = hb_wvt_gtGetWVT();

   PHB_ITEM info = hb_itemArrayNew( 7 );

   hb_arraySetC(  info, 1, _s->fontFace    );
   hb_arraySetNL( info, 2, _s->fontHeight  );
   hb_arraySetNL( info, 3, _s->fontWidth   );
   hb_arraySetNL( info, 4, _s->fontWeight  );
   hb_arraySetNL( info, 5, _s->fontQuality );
   hb_arraySetNL( info, 6, _s->PTEXTSIZE.y );
   hb_arraySetNL( info, 7, _s->PTEXTSIZE.x );

   hb_itemReturnRelease( info );
}

//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
//
//                 Peter Rees <peter@rees.co.nz>
//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//

HB_FUNC( WVT_SETMENU )
{
   PHB_GTWVT _s = hb_wvt_gtGetWVT();

   RECT wi = { 0, 0, 0, 0 };
   RECT ci = { 0, 0, 0, 0 };
   RECT rc = { 0, 0, 0, 0 };
   USHORT height, width;

   SetMenu( _s->hWnd, ( HMENU ) hb_parni( 1 ) ) ;

   GetWindowRect( _s->hWnd, &wi );
   GetClientRect( _s->hWnd, &ci );

   height = ( USHORT ) ( _s->PTEXTSIZE.y * _s->ROWS );
   width  = ( USHORT ) ( _s->PTEXTSIZE.x * _s->COLS );

   width  += ( USHORT ) ( wi.right - wi.left - ci.right );
   height += ( USHORT ) ( wi.bottom - wi.top - ci.bottom );

   if( _s->CentreWindow && SystemParametersInfo( SPI_GETWORKAREA, 0, &rc, 0 ) )
   {
      wi.left = rc.left + ( ( rc.right - rc.left - width  ) / 2 );
      wi.top  = rc.top  + ( ( rc.bottom - rc.top - height ) / 2 );
   }
   SetWindowPos( _s->hWnd, NULL, wi.left, wi.top, width, height, SWP_NOZORDER );
}

//-------------------------------------------------------------------//

HB_FUNC( WVT_SETPOPUPMENU )
{
   PHB_GTWVT _s = hb_wvt_gtGetWVT();

   HMENU hPopup = _s->hPopup ;

   _s->hPopup = ( HMENU ) hb_parnl( 1 );
   if ( hPopup )
   {
      hb_retnl( ( LONG ) hPopup );
   }
}

//-------------------------------------------------------------------//

HB_FUNC( WVT_CREATEMENU )
{
  hb_retnl( ( LONG ) CreateMenu() ) ;
}

//-------------------------------------------------------------------//

HB_FUNC( WVT_CREATEPOPUPMENU )
{
  hb_retnl( ( LONG ) CreatePopupMenu() ) ;
}

//-------------------------------------------------------------------//

HB_FUNC( WVT_APPENDMENU )
{
  char    ucBuf[ 256 ];
  int     i,iLen ;
  LPCTSTR lpszCaption;

  if ( ISCHAR( 4 ) )
  {
    iLen = hb_parclen( 4 );
    if ( iLen > 0 && iLen < 256 )   // Translate '~' to '&'
    {
      //lpszCaption = hb_parc( 4 ) ;
      lpszCaption = HB_TCHAR_CONVTO( hb_parc( 4 ) );
      for ( i = 0; i < iLen; i++ )
      {
        ucBuf[ i ] = ( *lpszCaption == '~' ) ? '&' : *lpszCaption ;
        lpszCaption++;
      }
      ucBuf[ iLen ]= '\0';
      //lpszCaption = ucBuf ;
      lpszCaption = HB_TCHAR_CONVTO( ucBuf );
    }
    else
    {
       //lpszCaption = hb_parc( 4 ) ;
       lpszCaption = HB_TCHAR_CONVTO( hb_parc( 4 ) );
    }
  }
  else
  {
    lpszCaption = ( LPCTSTR ) hb_parni( 4 ) ; // It is a SEPARATOR or Submenu
  }

  hb_retl( AppendMenu( ( HMENU ) hb_parnl( 1 ), ( UINT ) hb_parni( 2 ), ( HB_PTRDIFF ) hb_parni( 3 ), ( LPCTSTR ) lpszCaption ) ) ;
}

//-------------------------------------------------------------------//

HB_FUNC( WVT_DELETEMENU )
{
  hb_retl( DeleteMenu( ( HMENU ) hb_parnl( 1 ), ( UINT ) hb_parni( 2 ), ( UINT ) hb_parni( 3 ) ) );
}

//-------------------------------------------------------------------//

HB_FUNC( WVT_DESTROYMENU )
{
  hb_retl( DestroyMenu( ( HMENU ) hb_parnl( 1 ) ) );
}

//-------------------------------------------------------------------//

HB_FUNC( WVT_ENABLEMENUITEM )
{
   hb_retni( EnableMenuItem( ( HMENU ) hb_parnl( 1 ), ( UINT ) hb_parni( 2 ), ( UINT ) hb_parni( 3 ) ) );
}

//-------------------------------------------------------------------//

HB_FUNC( WVT_GETLASTMENUEVENT )
{
   PHB_GTWVT _s = hb_wvt_gtGetWVT();

   hb_retni( _s->LastMenuEvent );
}

//-------------------------------------------------------------------//

HB_FUNC( WVT_SETLASTMENUEVENT )
{
   PHB_GTWVT _s = hb_wvt_gtGetWVT();

   int iEvent = _s->LastMenuEvent;
   if ( ISNUM( 1 ) )
      _s->LastMenuEvent = hb_parni( 1 );

   hb_retni( iEvent );
}

//-------------------------------------------------------------------//

HB_FUNC( WVT_SETMENUKEYEVENT )
{
   PHB_GTWVT _s = hb_wvt_gtGetWVT();

   int iOldEvent = _s->MenuKeyEvent;

   if( ISNUM( 1 ) )
     _s->MenuKeyEvent = hb_parni( 1 );

   hb_retni( iOldEvent ) ;
}

//-------------------------------------------------------------------//

HB_FUNC( WVT_DRAWMENUBAR )
{
   PHB_GTWVT _s = hb_wvt_gtGetWVT();

   DrawMenuBar( _s->hWnd ) ;
}

//-------------------------------------------------------------------//

HB_FUNC( WVT_ENABLESHORTCUTS )
{
   PHB_GTWVT _s = hb_wvt_gtGetWVT();

   BOOL bWas = _s->EnableShortCuts;

   if( ISLOG( 1 ) )
      _s->EnableShortCuts = hb_parl( 1 );

   hb_retl( bWas );
}

//-------------------------------------------------------------------//

HB_FUNC( WVT_INVALIDATERECT )
{
   PHB_GTWVT _s = hb_wvt_gtGetWVT();

   RECT  rc = { 0,0,0,0 };
   POINT xy = { 0,0 };

   xy           = hb_wvt_gtGetXYFromColRow( ( USHORT ) hb_parni( 2 ), ( USHORT ) hb_parni( 1 ) );
   rc.top       = xy.y;
   rc.left      = xy.x;
   xy           = hb_wvt_gtGetXYFromColRow( ( USHORT ) hb_parni( 4 )+1, ( USHORT ) ( hb_parni( 3 )+1 ) );
   rc.bottom    = xy.y - 1;
   rc.right     = xy.x - 1;

   InvalidateRect( _s->hWnd, &rc, TRUE );
}

//-------------------------------------------------------------------//

HB_FUNC( WVT_ISLBUTTONPRESSED )
{
   hb_retl( GetKeyState( VK_LBUTTON ) & 0x8000 );
}

//-------------------------------------------------------------------//

HB_FUNC( WVT_CLIENTTOSCREEN )
{
   PHB_GTWVT _s = hb_wvt_gtGetWVT();

   PHB_ITEM info = hb_itemArrayNew( 2 );
   POINT    xy = { 0,0 };

   xy = hb_wvt_gtGetXYFromColRow( ( USHORT ) hb_parni( 2 ), ( USHORT ) hb_parni( 1 ) );

   ClientToScreen( _s->hWnd, &xy );

   hb_arraySetNL( info, 1, xy.x );
   hb_arraySetNL( info, 2, xy.y );

   hb_itemReturnRelease( info );
}

//-------------------------------------------------------------------//

HB_FUNC( WVT_GETCURSORPOS )
{
   POINT    xy = { 0,0 };
   PHB_ITEM info = hb_itemArrayNew( 2 );

   GetCursorPos( &xy );

   hb_arraySetNI( info, 1, xy.x );
   hb_arraySetNI( info, 2, xy.y );

   hb_itemReturnRelease( info );
}

//-------------------------------------------------------------------//

HB_FUNC( WVT_TRACKPOPUPMENU )
{
   PHB_GTWVT _s = hb_wvt_gtGetWVT();

   POINT xy = { 0,0 };

   GetCursorPos( &xy );

   hb_retnl( TrackPopupMenu( ( HMENU ) hb_parnl( 1 ) ,
                     TPM_CENTERALIGN | TPM_RETURNCMD ,
                                                xy.x ,
                                                xy.y ,
                                                   0 ,
                                            _s->hWnd ,
                                                NULL ) );
}

//-------------------------------------------------------------------//

HB_FUNC( WVT_GETMENU )
{
   PHB_GTWVT _s = hb_wvt_gtGetWVT();

   hb_retnl( ( ULONG ) GetMenu( _s->hWnd ) );
}

//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
//
//                             Dialogs
//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//

HB_FUNC( WVT_CREATEDIALOGDYNAMIC )
{
   PHB_GTWVT _s = hb_wvt_gtGetWVT();

   PHB_ITEM pFirst = hb_param( 3,HB_IT_ANY );
   PHB_ITEM pFunc  = NULL ;
   PHB_DYNS pExecSym;
   HWND     hDlg   = 0;
   int      iType  = 0;
   int      iIndex;
   int      iResource = hb_parni( 4 );

   /* check if we still have room for a new dialog */
   for ( iIndex = 0; iIndex < WVT_DLGML_MAX; iIndex++ )
   {
      if ( _s->hDlgModeless[ iIndex ] == NULL )
      {
         break;
      }
   }

   if ( iIndex >= WVT_DLGML_MAX )
   {
      /* no more room */
      hb_retnl( (ULONG) NULL );
      return;
   }

   if ( HB_IS_BLOCK( pFirst ) )
   {
      /* pFunc is pointing to stored code block (later) */
      pFunc = hb_itemNew( pFirst );
      iType = 2;
   }
   else if( hb_itemType( pFirst ) == HB_IT_STRING )
   {
      #ifdef __XHARBOUR__
      hb_dynsymLock();
      #endif
      pExecSym = hb_dynsymFindName( hb_itemGetCPtr( pFirst ) );
      #ifdef __XHARBOUR__
      hb_dynsymUnlock();
      #endif
      if ( pExecSym )
      {
         pFunc = ( PHB_ITEM ) pExecSym;
      }
      iType = 1;
   }

   {
      if ( ISNUM( 3 ) )
      {
         LPTSTR lpTemplate = HB_TCHAR_CONVTO( hb_parc( 1 ) );
         hDlg = CreateDialogIndirect( ( HINSTANCE     ) hb_hInstance,
                                      ( LPDLGTEMPLATE ) lpTemplate,
                                                        hb_parl( 2 ) ? _s->hWnd : NULL,
                                      ( DLGPROC       ) hb_parnl( 3 ) );
         HB_TCHAR_FREE( lpTemplate );
      }
      else
      {
         switch ( iResource )
         {
            case 0:
            {
               LPTSTR lpTemplate = HB_TCHAR_CONVTO( hb_parc( 1 ) );
               hDlg = CreateDialog( ( HINSTANCE ) hb_hInstance,
                                                  lpTemplate,
                                                  hb_parl( 2 ) ? _s->hWnd : NULL,
                                      ( DLGPROC ) hb_wvt_gtDlgProcMLess );
               HB_TCHAR_FREE( lpTemplate );
            }
            break;

            case 1:
            {
               hDlg = CreateDialog( ( HINSTANCE ) hb_hInstance,
                                    MAKEINTRESOURCE( ( WORD ) hb_parni( 1 ) ),
                                    hb_parl( 2 ) ? _s->hWnd : NULL,
                                    ( DLGPROC ) hb_wvt_gtDlgProcMLess );
            }
            break;

            case 2:
            {
               /* hb_parc( 1 ) is already unicode compliant, so no conversion */
               hDlg = CreateDialogIndirect( ( HINSTANCE     ) hb_hInstance,
                                            ( LPDLGTEMPLATE ) hb_parc( 1 ),
                                            hb_parl( 2 ) ? _s->hWnd : NULL,
                                            ( DLGPROC ) hb_wvt_gtDlgProcMLess );
            }
            break;
         }
      }

      if ( hDlg )
      {
         _s->hDlgModeless[ iIndex ] = hDlg;

         if ( pFunc )
         {
            /* if codeblock, store the codeblock and lock it there */
            if (HB_IS_BLOCK( pFirst ))
            {
               _s->pcbFunc[ iIndex ] = pFunc;
            }

            _s->pFunc[ iIndex ] = pFunc;
            _s->iType[ iIndex ] = iType;
         }
         else
         {
            _s->pFunc[ iIndex ] = NULL;
            _s->iType[ iIndex ] = 0;
         }
         SendMessage( hDlg, WM_INITDIALOG, 0, 0 );
      }
      else
      {
         /* if codeblock item created earlier, release it */
         if (iType==2 && pFunc)
         {
            hb_itemRelease( pFunc );
         }
         _s->hDlgModeless[ iIndex ] = NULL;
      }
   }

   hb_retnl( ( ULONG ) hDlg );
}

//-------------------------------------------------------------------//

HB_FUNC( WVT_CREATEDIALOGMODAL )
{
   PHB_GTWVT _s = hb_wvt_gtGetWVT();

   PHB_ITEM pFirst    = hb_param( 3,HB_IT_ANY );
   PHB_ITEM pFunc     = NULL ;
   PHB_DYNS pExecSym;
   int      iIndex;
   int      iResource = hb_parni( 4 );
   int      iResult   = 0;
   HWND     hParent   = ISNIL( 5 ) ? _s->hWnd : ( HWND ) hb_parnl( 5 );

   /* check if we still have room for a new dialog */
   for ( iIndex = 0; iIndex < WVT_DLGMD_MAX; iIndex++ )
   {
      if ( _s->hDlgModal[ iIndex ] == NULL )
      {
         break;
      }
   }

   if ( iIndex >= WVT_DLGMD_MAX )
   {
      /* no more room */
      hb_retni( ( int ) NULL );
      return;
   }

   if ( HB_IS_BLOCK( pFirst ) )
   {
      /* pFunc is pointing to stored code block (later) */

      _s->pcbFuncModal[ iIndex ] = hb_itemNew( pFirst );

      pFunc = _s->pcbFuncModal[ iIndex ];
      _s->pFuncModal[ iIndex ] = pFunc;
      _s->iTypeModal[ iIndex ] = 2;
   }
   else if( hb_itemType( pFirst ) == HB_IT_STRING )
   {
      #ifdef __XHARBOUR__
      hb_dynsymLock();
      #endif
      pExecSym = hb_dynsymFindName( hb_itemGetCPtr( pFirst ) );
      #ifdef __XHARBOUR__
      hb_dynsymUnlock();
      #endif
      if ( pExecSym )
      {
         pFunc = ( PHB_ITEM ) pExecSym;
      }
      _s->pFuncModal[ iIndex ] = pFunc;
      _s->iTypeModal[ iIndex ] = 1;
   }

   switch ( iResource )
   {
      case 0:
      {
         LPTSTR lpTemplate = HB_TCHAR_CONVTO( hb_parc( 1 ) );
         iResult = DialogBoxParam( ( HINSTANCE     ) hb_hInstance,
                                                     lpTemplate,
                                                     hParent,
                                         ( DLGPROC ) hb_wvt_gtDlgProcModal,
                                ( LPARAM ) ( DWORD ) iIndex+1 );
         HB_TCHAR_FREE( lpTemplate );
      }
      break;

      case 1:
      {
         iResult = DialogBoxParam( ( HINSTANCE     ) hb_hInstance,
                           MAKEINTRESOURCE( ( WORD ) hb_parni( 1 ) ),
                                                     hParent,
                                         ( DLGPROC ) hb_wvt_gtDlgProcModal,
                                ( LPARAM ) ( DWORD ) iIndex+1 );
      }
      break;

      case 2:
      {
         /* hb_parc( 1 ) is already unicode compliant, so no conversion */
         iResult = DialogBoxIndirectParam( ( HINSTANCE     ) hb_hInstance,
                                           ( LPDLGTEMPLATE ) hb_parc( 1 ),
                                                             hParent,
                                                 ( DLGPROC ) hb_wvt_gtDlgProcModal,
                                        ( LPARAM ) ( DWORD ) iIndex+1 );
      }
      break;
   }

   hb_retni( iResult );
}

//-------------------------------------------------------------------//

HB_FUNC( WVT__MAKEDLGTEMPLATE )
{
   WORD  *p, *pdlgtemplate ;
   WORD  nItems = ( WORD ) hb_parni( 1, 4 ) ;
   int   i, nchar ;
   DWORD lStyle ;

   // Parameters: 12 arrays
   // 1 for DLG template
   // 11 for item properties

   // 64k allow to build up to 255 items on the dialog
   //
   pdlgtemplate = p = ( PWORD ) LocalAlloc( LPTR, 65534 )  ;

   //---------------

    lStyle = hb_parnl(1,3) ;

    // start to fill in the dlgtemplate information.  addressing by WORDs

    *p++ = 1                        ; // version
    *p++ = 0xFFFF                   ; // signature
    *p++ = LOWORD ( hb_parnl(1,1) ) ; // Help Id
    *p++ = HIWORD ( hb_parnl(1,1) ) ;

    *p++ = LOWORD ( hb_parnl(1,2) ) ; // ext. style
    *p++ = HIWORD ( hb_parnl(1,2) ) ;

    *p++ = LOWORD (lStyle)          ;
    *p++ = HIWORD (lStyle)          ;

    *p++ = (WORD)   nItems          ;  // NumberOfItems
    *p++ = (short)  hb_parni(1,5)   ;  // x
    *p++ = (short)  hb_parni(1,6)   ;  // y
    *p++ = (short)  hb_parni(1,7)   ;  // cx
    *p++ = (short)  hb_parni(1,8)   ;  // cy
    *p++ = (short)  0               ;  // Menu (ignored for now.)
    *p++ = (short)  0x00            ;  // Class also ignored

    if ( hb_parinfa( 1,11 ) == HB_IT_STRING )
    {
       nchar = nCopyAnsiToWideChar( p, (LPSTR) hb_parc( 1,11 ) ) ;
       p += nchar   ;
    }
    else
    {
      *p++ =0 ;
    }
    // add in the wPointSize and szFontName here iff the DS_SETFONT bit on

    if ( ( lStyle & DS_SETFONT ) )
    {
      *p++ = (short) hb_parni(1,12) ;
      *p++ = (short) hb_parni(1,13) ;
      *p++ = (short) hb_parni(1,14) ;

      nchar = nCopyAnsiToWideChar( p, (LPSTR) hb_parc( 1,15 ) ) ;
      p += nchar ;
    } ;

    //---------------
    // Now, for the items

   for ( i = 1 ; i <= nItems ; i++ ) {
      // make sure each item starts on a DWORD boundary
      p = lpwAlign (p) ;

      *p++ = LOWORD ( hb_parnl(2,i) ) ;    // help id
      *p++ = HIWORD ( hb_parnl(2,i) ) ;

      *p++ = LOWORD ( hb_parnl(3,i) ) ; // ext. style
      *p++ = HIWORD ( hb_parnl(3,i) ) ;

      *p++ = LOWORD ( hb_parnl(4,i) ) ; // style
      *p++ = HIWORD ( hb_parnl(4,i) ) ;

      *p++ = (short)  hb_parni(5,i)   ;  // x
      *p++ = (short)  hb_parni(6,i)   ;  // y
      *p++ = (short)  hb_parni(7,i)   ;  // cx
      *p++ = (short)  hb_parni(8,i)   ;  // cy

      *p++ = LOWORD ( hb_parnl(9,i) ) ;  // id
      *p++ = HIWORD ( hb_parnl(9,i) ) ;  // id   // 0;

      if ( hb_parinfa( 10,i ) == HB_IT_STRING )
      {
         nchar = nCopyAnsiToWideChar( p, (LPSTR) hb_parc( 10,i ) ) ; // class
         p += nchar ;
         }
      else
         {
         *p++ = 0xFFFF ;
         *p++ = (WORD) hb_parni(10,i) ;
         }

      if ( hb_parinfa( 11,i ) == HB_IT_STRING )
         {
         nchar = nCopyAnsiToWideChar( p, (LPSTR) hb_parc( 11,i ) ) ;  // text
         p += nchar ;
         }
      else
         {
         *p++ = 0xFFFF ;
         *p++ = (WORD) hb_parni(11,i) ;
         }

      *p++ = 0x00 ;  // extras ( in array 12 )
    } ;

    p = lpwAlign( p )  ;

    hb_retclen( ( LPSTR ) pdlgtemplate, ( ( ULONG ) p - ( ULONG ) pdlgtemplate ) ) ;

    LocalFree( LocalHandle( pdlgtemplate ) ) ;
}

//-------------------------------------------------------------------//
//
//  Helper routine.  Take an input pointer, return closest
//  pointer that is aligned on a DWORD (4 byte) boundary.
//
HB_EXPORT LPWORD lpwAlign( LPWORD lpIn )
{
   ULONG ul;
   ul = ( ULONG ) lpIn;
   ul += 3;
   ul >>=2;
   ul <<=2;
  return ( LPWORD ) ul;
}

//-----------------------------------------------------------------------------

HB_EXPORT int nCopyAnsiToWideChar( LPWORD lpWCStr, LPSTR lpAnsiIn )
{
   int nChar = 0;

   do
   {
      *lpWCStr++ = ( WORD ) *lpAnsiIn;
      nChar++;
   }
   while ( *lpAnsiIn++ );

   return nChar;
}

//-------------------------------------------------------------------//

HB_FUNC( WVT_LBADDSTRING )
{
   LPTSTR text = HB_TCHAR_CONVTO( hb_parc( 3 ) );
   SendMessage( GetDlgItem( ( HWND ) hb_parnl( 1 ), hb_parni( 2 ) ), LB_ADDSTRING, 0, ( LPARAM )( LPSTR ) text );
   HB_TCHAR_FREE( text );
}

//-------------------------------------------------------------------//

HB_FUNC( WVT_LBGETCOUNT )
{
   hb_retnl( SendMessage( GetDlgItem( ( HWND ) hb_parnl( 1 ), hb_parni( 2 ) ), LB_GETCOUNT, 0, 0 ) );
}

//-------------------------------------------------------------------//

HB_FUNC( WVT_LBDELETESTRING )
{
   SendMessage( GetDlgItem( ( HWND ) hb_parnl( 1 ), hb_parni( 2 ) ), LB_DELETESTRING, hb_parni( 3 ), 0 );
}

//-------------------------------------------------------------------//

HB_FUNC( WVT_LBSETCURSEL )
{
   SendMessage( GetDlgItem( ( HWND ) hb_parnl( 1 ), hb_parni( 2 ) ), LB_SETCURSEL, hb_parni( 3 ), 0 );
}

//-------------------------------------------------------------------//

HB_FUNC( WVT_CBADDSTRING )
{
   LPTSTR text = HB_TCHAR_CONVTO( hb_parc( 3 ) );
   SendMessage( GetDlgItem( ( HWND ) hb_parnl( 1 ), hb_parni( 2 ) ), CB_ADDSTRING, 0, ( LPARAM )( LPSTR ) text );
   HB_TCHAR_FREE( text );
}

//-------------------------------------------------------------------//

HB_FUNC( WVT_CBSETCURSEL )
{
   SendMessage( GetDlgItem( ( HWND ) hb_parnl( 1 ), hb_parni( 2 ) ), CB_SETCURSEL, hb_parni( 3 ), 0 );
}

//-------------------------------------------------------------------//
//
//   Wvt_DlgSetIcon( hDlg, ncIcon )
//
HB_FUNC( WVT_DLGSETICON )
{
   HICON hIcon;

   if ( ISNUM( 2 ) )
   {
      hIcon = LoadIcon( ( HINSTANCE ) hb_hInstance, MAKEINTRESOURCE( hb_parni( 2 ) ) );
   }
   else
   {
      LPTSTR icon = HB_TCHAR_CONVTO( hb_parc( 2 ) );
      hIcon = ( HICON ) LoadImage( ( HINSTANCE ) NULL, icon, IMAGE_ICON, 0, 0, LR_LOADFROMFILE );
      HB_TCHAR_FREE( icon );
   }

   if ( hIcon )
   {
      SendMessage( ( HWND ) hb_parnl( 1 ), WM_SETICON, ICON_SMALL, ( LPARAM ) hIcon ); // Set Title Bar ICON
      SendMessage( ( HWND ) hb_parnl( 1 ), WM_SETICON, ICON_BIG,   ( LPARAM ) hIcon ); // Set Task List Icon
   }

   if ( hIcon )
   {
      hb_retnl( ( ULONG ) hIcon );
   }
}

//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
//
//             Direct WinApi Functions - Prefixed WIN_*()
//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//

HB_FUNC( WIN_SENDMESSAGE )
{
   LPTSTR cText = NULL;

   if( ISBYREF( 4 ) )
   {
      cText = HB_TCHAR_CONVTO( hb_parc( 4 ) );
   }

   hb_retnl( ( ULONG ) SendMessage( ( HWND ) hb_parnl( 1 ),
                                    ( UINT ) hb_parni( 2 ),
                                    ( ISNIL( 3 ) ? 0 : ( WPARAM ) hb_parnl( 3 ) ),
                                    ( ISNIL( 4 ) ? 0 : ( cText ? ( LPARAM ) ( LPSTR ) cText :
                                       ( ISCHAR( 4 ) ? ( LPARAM )( LPSTR ) hb_parc( 4 ) :
                                           ( LPARAM ) hb_parnl( 4 ) ) ) ) )
           );

   if( cText )
   {
      char * szText = HB_TCHAR_CONVFROM( cText );
      hb_storc( szText, 4 );
      HB_TCHAR_FREE( szText );
      HB_TCHAR_FREE( cText );
   }
}

//-------------------------------------------------------------------//

HB_FUNC( WIN_SENDDLGITEMMESSAGE )
{
   PHB_ITEM pText = hb_param( 5, HB_IT_STRING );
   char     *cText = NULL;
   int      iLen = 0;

   if( pText )
   {
      iLen  = hb_itemGetCLen( pText );
      cText = (char*) hb_xgrab( iLen+1 );
      hb_xmemcpy( cText, hb_itemGetCPtr( pText ), iLen+1 );
   }

   hb_retnl( (LONG) SendDlgItemMessage( (HWND) hb_parnl( 1 ) ,
                                        (int)  hb_parni( 2 ) ,
                                        (UINT) hb_parni( 3 ) ,
                                        (ISNIL(4) ? 0 : (WPARAM) hb_parnl( 4 ))   ,
                                        (cText ? (LPARAM) cText : (LPARAM) hb_parnl( 5 ))
                                      ) );

   if( cText )
   {
      if( ISBYREF( 5 ) )
      {
         hb_storclen( cText, iLen, 5 ) ;
      }
      hb_xfree( cText );
   }
}

//-------------------------------------------------------------------//
//
//  WIN_SetTimer( hWnd, nIdentifier, nTimeOut )
//
HB_FUNC( WIN_SETTIMER )
{
   hb_retl( SetTimer( ( HWND ) hb_parnl( 1 ), hb_parni( 2 ), hb_parni( 3 ), NULL ) );
}

//-------------------------------------------------------------------//

HB_FUNC( WIN_SETFOCUS )
{
   SetFocus( ( HWND ) hb_parnl( 1 ) );
}

//-------------------------------------------------------------------//

HB_FUNC( WIN_SETTEXTCOLOR )
{
   hb_retnl( ( ULONG ) SetTextColor( ( HDC ) hb_parnl( 1 ), ( COLORREF ) hb_parnl( 2 ) ) );
}

//-------------------------------------------------------------------//

HB_FUNC( WIN_SETBKCOLOR )
{
   hb_retnl( ( ULONG ) SetBkColor( ( HDC ) hb_parnl( 1 ), ( COLORREF ) hb_parnl( 2 ) ) );
}

//-------------------------------------------------------------------//

HB_FUNC( WIN_SETBKMODE )
{
   hb_retni( ( int ) SetBkMode( ( HDC ) hb_parnl( 1 ), hb_parni( 2 ) ) );
}

//-------------------------------------------------------------------//

HB_FUNC( WIN_GETSTOCKOBJECT )
{
   hb_retnl( ( ULONG ) GetStockObject( hb_parnl( 1 ) ) );
}

//-------------------------------------------------------------------//

HB_FUNC( WIN_DELETEOBJECT )
{
   hb_retl( DeleteObject( ( HGDIOBJ ) hb_parnl( 1 ) ) );
}

//-------------------------------------------------------------------//

HB_FUNC( WIN_SELECTOBJECT )
{
   hb_retnl( ( ULONG ) SelectObject( ( HDC ) hb_parnl( 1 ), ( HGDIOBJ ) hb_parnl( 2 ) ) );
}

//-------------------------------------------------------------------//

HB_FUNC( WIN_LOWORD )
{
   hb_retnl( LOWORD( hb_parnl( 1 ) ) );
}

//-------------------------------------------------------------------//

HB_FUNC( WIN_HIWORD )
{
   hb_retnl( HIWORD( hb_parnl( 1 ) ) );
}

//-------------------------------------------------------------------//

HB_FUNC( WIN_MULDIV )
{
   hb_retni( MulDiv( hb_parni( 1 ), hb_parni( 2 ), hb_parni( 3 ) ) );
}

//-------------------------------------------------------------------//

HB_FUNC( WIN_GETDIALOGBASEUNITS )
{
   hb_retnl( ( LONG ) GetDialogBaseUnits() ) ;
}

//-------------------------------------------------------------------//

HB_FUNC( WIN_SETMENU )
{
   SetMenu( ( HWND ) hb_parnl( 1 ), ( HMENU ) hb_parni( 2 ) ) ;
}

//-------------------------------------------------------------------//

HB_FUNC( WIN_SETDLGITEMTEXT )
{
   LPTSTR lpBuffer = HB_TCHAR_CONVTO( hb_parc( 3 ) );
   SetDlgItemText( ( HWND ) hb_parnl( 1 ), hb_parni( 2 ), lpBuffer );
   HB_TCHAR_FREE( lpBuffer );
}

//-------------------------------------------------------------------//

HB_FUNC( WIN_GETDLGITEMTEXT )
{
   USHORT iLen = ( USHORT ) SendMessage( GetDlgItem( ( HWND ) hb_parnl( 1 ), hb_parni( 2 ) ), WM_GETTEXTLENGTH, 0, 0 ) + 1 ;
   LPTSTR cText = ( LPTSTR ) hb_xgrab( iLen * sizeof( TCHAR ) );
   char * szText;
   USHORT iResult;

   iResult = ( USHORT ) GetDlgItemText( ( HWND ) hb_parnl( 1 ),   // handle of dialog box
                             hb_parni( 2 ),            // identifier of control
                             cText,                    // address of buffer for text
                             iLen                      // maximum size of string
                            );

   cText[ iResult ] = '\0';
   szText = HB_TCHAR_CONVFROM( cText );
   hb_retc( szText );
   HB_TCHAR_FREE( szText );
   hb_xfree( cText );
}

//-------------------------------------------------------------------//

HB_FUNC( WIN_CHECKDLGBUTTON )
{
   hb_retl( CheckDlgButton( ( HWND ) hb_parnl( 1 ), hb_parni( 2 ),
                            ( UINT )( ISNUM( 3 ) ? hb_parni( 3 ) : hb_parl( 3 ) ) ) );
}

//-------------------------------------------------------------------//

HB_FUNC( WIN_ISDLGBUTTONCHECKED )
{
   hb_retni( IsDlgButtonChecked( ( HWND ) hb_parnl( 1 ), hb_parni( 2 ) ) ) ;
}

//-------------------------------------------------------------------//

HB_FUNC( WIN_CHECKRADIOBUTTON )
{
    hb_retl( CheckRadioButton( ( HWND ) hb_parnl( 1 ),   // handle of dialog box
                                        hb_parni( 2 ),   // identifier of first radio button in group
                                        hb_parni( 3 ),   // identifier of last radio button in group
                                        hb_parni( 4 )    // identifier of radio button to select
                              ) );
}

//-------------------------------------------------------------------//

HB_FUNC( WIN_GETDLGITEM )
{
   hb_retnl( ( ULONG ) GetDlgItem( ( HWND ) hb_parnl( 1 ), hb_parni( 2 ) ) );
}

//-------------------------------------------------------------------//

HB_FUNC( WIN_MESSAGEBOX )
{
   LPTSTR lpBuffer = HB_TCHAR_CONVTO( hb_parc( 2 ) );
   LPTSTR lpBuffer2 = HB_TCHAR_CONVTO( hb_parc( 3 ) );

   hb_retni( MessageBox( ( HWND ) hb_parnl( 1 ), lpBuffer, lpBuffer2, ISNIL( 4 ) ? MB_OK : hb_parni( 4 ) ) ) ;

   HB_TCHAR_FREE( lpBuffer );
   HB_TCHAR_FREE( lpBuffer2 );
}

//-------------------------------------------------------------------//

HB_FUNC( WIN_INVALIDATERECT )
{
   InvalidateRect( ( HWND ) hb_parnl( 1 ), NULL, TRUE );
}

//-------------------------------------------------------------------//
//
//  Win_LoadIcon( ncIcon )
//
HB_FUNC( WIN_LOADICON )
{
   HICON hIcon;

   if ( ISNUM( 1 ) )
   {
      hIcon = LoadIcon( ( HINSTANCE ) hb_hInstance, MAKEINTRESOURCE( hb_parni( 1 ) ) );
   }
   else
   {
      LPTSTR lpBuffer = HB_TCHAR_CONVTO( hb_parc( 1 ) );
      hIcon = ( HICON ) LoadImage( ( HINSTANCE ) NULL, lpBuffer, IMAGE_ICON, 0, 0, LR_LOADFROMFILE );
      HB_TCHAR_FREE( lpBuffer );
   }

   hb_retnl( ( ULONG ) hIcon ) ;
}

//-------------------------------------------------------------------//
//
//  Win_LoadImage( ncImage, nSource ) -> hImage
//    nSource == 0 ResourceIdByNumber
//    nSource == 1 ResourceIdByName
//    nSource == 2 ImageFromDiskFile
//
HB_FUNC( WIN_LOADIMAGE )
{
   HBITMAP hImage = 0;
   LPTSTR  lpBuffer = HB_TCHAR_CONVTO( hb_parc( 1 ) );
   int     iSource = hb_parni( 2 );

   switch ( iSource )
   {
      case 0:
         hImage = LoadBitmap( ( HINSTANCE ) hb_hInstance, MAKEINTRESOURCE( hb_parni( 1 ) ) );
         break;

      case 1:
         hImage = LoadBitmap( ( HINSTANCE ) hb_hInstance, lpBuffer );
         break;

      case 2:
         hImage = ( HBITMAP ) LoadImage( ( HINSTANCE ) NULL, lpBuffer, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE );
         break;
   }

   HB_TCHAR_FREE( lpBuffer );
   hb_retnl( ( ULONG ) hImage ) ;
}

//-------------------------------------------------------------------//

HB_FUNC( WIN_GETCLIENTRECT )
{
   RECT     rc = { 0,0,0,0 };
   PHB_ITEM info = hb_itemArrayNew( 4 );

   GetClientRect( ( HWND ) hb_parnl( 1 ), &rc );

   hb_arraySetNI( info, 1, rc.left   );
   hb_arraySetNI( info, 2, rc.top    );
   hb_arraySetNI( info, 3, rc.right  );
   hb_arraySetNI( info, 4, rc.bottom );

   hb_itemReturnRelease( info );
}

//-------------------------------------------------------------------//
//
//    Win_DrawImage( hdc, nLeft, nTop, nWidth, nHeight, cImage ) in Pixels
//
HB_FUNC( WIN_DRAWIMAGE )
{
   hb_retl( hb_wvt_DrawImage( ( HDC ) hb_parni( 1 ), hb_parni( 2 ), hb_parni( 3 ),
                                   hb_parni( 4 ), hb_parni( 5 ), hb_parc( 6 ) ) );
}

//-------------------------------------------------------------------//

HB_FUNC( WIN_GETDC )
{
   hb_retnl( ( ULONG ) GetDC( ( HWND ) hb_parnl( 1 )  ) );
}

//-------------------------------------------------------------------//

HB_FUNC( WIN_RELEASEDC )
{
   hb_retl( ReleaseDC( ( HWND ) hb_parnl( 1 ), ( HDC ) hb_parnl( 2 ) ) );
}

//-------------------------------------------------------------------//

HB_FUNC( WIN_RECTANGLE )
{
   Rectangle( ( HDC ) hb_parnl( 1 ), hb_parni( 2 ), hb_parni( 3 ), hb_parni( 4 ), hb_parni( 5 ) );
}

//-------------------------------------------------------------------//

HB_FUNC( WIN_CREATEBRUSH )
{
   LOGBRUSH lb = { 0,0,0 };

   lb.lbStyle = hb_parni( 1 );
   lb.lbColor = ISNIL( 2 ) ? RGB( 0,0,0 ) : ( COLORREF ) hb_parnl( 2 ) ;
   lb.lbHatch = ISNIL( 3 ) ? 0 : hb_parni( 3 );

   hb_retnl( ( ULONG ) CreateBrushIndirect( &lb ) );
}

//-------------------------------------------------------------------//
//
//   Win_DrawText( hDC, cText, aRect, nFormat )
//
HB_FUNC( WIN_DRAWTEXT )
{
   RECT rc = { 0,0,0,0 };
   LPTSTR lpBuffer = HB_TCHAR_CONVTO( hb_parc( 2 ) );

   rc.left   = hb_parni( 3,1 );
   rc.top    = hb_parni( 3,2 );
   rc.right  = hb_parni( 3,3 );
   rc.bottom = hb_parni( 3,4 );

   hb_retl( DrawText( ( HDC ) hb_parnl( 1 ), lpBuffer, lstrlen( lpBuffer ), &rc, hb_parni( 4 ) ) );
   HB_TCHAR_FREE( lpBuffer );
}

//-------------------------------------------------------------------//

HB_FUNC( WIN_GETWINDOWRECT )
{
   RECT rc;
   PHB_ITEM info = hb_itemArrayNew( 4 );

   GetWindowRect( ( HWND ) hb_parnl( 1 ), &rc );

   hb_arraySetNI( info, 1, rc.left   );
   hb_arraySetNI( info, 2, rc.top    );
   hb_arraySetNI( info, 3, rc.right  );
   hb_arraySetNI( info, 4, rc.bottom );

   hb_itemReturnRelease( info );
}

//-------------------------------------------------------------------//

HB_FUNC( WIN_MOVEWINDOW )
{
   MoveWindow( (HWND) hb_parnl( 1 ), hb_parnl( 2 ), hb_parnl( 3 ), hb_parnl( 4 ), hb_parnl( 5 ), hb_parl( 6 ) );
}

//-------------------------------------------------------------------//

HB_FUNC( WIN_SETPARENT )
{
   hb_retnl( ( long ) SetParent(  (HWND) hb_parnl( 1 ), (HWND) hb_parnl( 2 ) ) );
}

//-------------------------------------------------------------------//

HB_FUNC( WIN_SETWINDOWLONG )
{
   hb_retnl( SetWindowLong( ( HWND ) hb_parnl( 1 ), hb_parni( 2 ), hb_parnl( 3 ) ) );
}

//-------------------------------------------------------------------//

HB_FUNC( WIN_ISWINDOW )
{
   hb_retl( IsWindow( (HWND) hb_parnl( 1 ) ) );
}

//-------------------------------------------------------------------//

HB_FUNC( WVT_GETFONTHANDLE )
{
   PHB_GTWVT _s = hb_wvt_gtGetWVT();

   HFONT hFont = 0;
   int   iSlot = hb_parni( 1 ) - 1;

   if ( iSlot >= 0 && iSlot < WVT_PICTURES_MAX )
      hFont = _s->hUserFonts[ iSlot ];

   hb_retnl( ( ULONG ) hFont );
}

//-------------------------------------------------------------------//

HB_FUNC( WIN_CLIENTTOSCREEN )
{
   POINT    Point ;
   PHB_ITEM pArray = hb_param( 2 , HB_IT_ARRAY );

   if ( wvt_Array2Point( pArray ,&Point ) )
   {
      if ( ClientToScreen( (HWND) hb_parnl( 1 ), &Point ) )
      {
          wvt_Point2ArrayEx( &Point, pArray );
          hb_retl( TRUE ) ;
      }
      else
      {
         hb_retl( FALSE ) ;
      }
   }
   else
   {
      hb_retl( FALSE ) ;
   }
}

//----------------------------------------------------------------------//

HB_FUNC( WIN_SCREENTOCLIENT )
{
   POINT    Point ;
   PHB_ITEM pArray = hb_param( 2 , HB_IT_ARRAY );

   if ( wvt_Array2Point( pArray, &Point ) )
   {
      if( ScreenToClient( (HWND) hb_parnl( 1 ), &Point ) > 0 )
      {
          wvt_Point2ArrayEx( &Point, pArray );
          hb_retl( TRUE ) ;
      }
      else
      {
         hb_retl( FALSE ) ;
      }
   }
   else
   {
      hb_retl( FALSE ) ;
   }
}

//----------------------------------------------------------------------//
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//
//
//                     Utility Functions - Not API
//
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//

BOOL wvt_Array2Rect(PHB_ITEM aRect, RECT *rc )
{
   if (HB_IS_ARRAY(aRect) && hb_arrayLen(aRect) == 4) {
      rc->left   = hb_arrayGetNL(aRect,1);
      rc->top    = hb_arrayGetNL(aRect,2);
      rc->right  = hb_arrayGetNL(aRect,3);
      rc->bottom = hb_arrayGetNL(aRect,4);
      return TRUE ;
   }
   return FALSE;
}

//----------------------------------------------------------------------//

PHB_ITEM wvt_Rect2Array( RECT *rc  )
{
   PHB_ITEM aRect   = hb_itemArrayNew( 4 );
   PHB_ITEM element = hb_itemNew( NULL );

   hb_arraySet(aRect, 1, hb_itemPutNL( element, rc->left   ));
   hb_arraySet(aRect, 2, hb_itemPutNL( element, rc->top    ));
   hb_arraySet(aRect, 3, hb_itemPutNL( element, rc->right  ));
   hb_arraySet(aRect, 4, hb_itemPutNL( element, rc->bottom ));
   hb_itemRelease(element);
   return aRect;
}

//----------------------------------------------------------------------//

BOOL wvt_Array2Point(PHB_ITEM aPoint, POINT *pt )
{
   if (HB_IS_ARRAY(aPoint) && hb_arrayLen(aPoint) == 2) {
      pt->x = hb_arrayGetNL(aPoint,1);
      pt->y = hb_arrayGetNL(aPoint,2);
      return TRUE ;
   }
   return FALSE;
}

//----------------------------------------------------------------------//

PHB_ITEM wvt_Point2Array( POINT *pt  )
{
   PHB_ITEM aPoint = hb_itemArrayNew(2);
   PHB_ITEM element = hb_itemNew(NULL);

   hb_arraySet(aPoint, 1, hb_itemPutNL(element, pt->x));
   hb_arraySet(aPoint, 2, hb_itemPutNL(element, pt->y));
   hb_itemRelease(element);
   return aPoint;
}

//----------------------------------------------------------------------//

BOOL wvt_Array2Size(PHB_ITEM aSize, SIZE *siz )
{
   if (HB_IS_ARRAY(aSize) && hb_arrayLen(aSize) == 2) {
      siz->cx = hb_arrayGetNL(aSize,1);
      siz->cy = hb_arrayGetNL(aSize,2);
      return TRUE ;
   }
   return FALSE;
}

//----------------------------------------------------------------------//

PHB_ITEM wvt_Size2Array( SIZE *siz  )
{
   PHB_ITEM aSize   = hb_itemArrayNew(2);
   PHB_ITEM element = hb_itemNew(NULL);

   hb_arraySet(aSize, 1, hb_itemPutNL(element, siz->cx));
   hb_arraySet(aSize, 2, hb_itemPutNL(element, siz->cy));
   hb_itemRelease(element);
   return aSize;
}

//----------------------------------------------------------------------//

void  wvt_Rect2ArrayEx( RECT *rc ,PHB_ITEM aRect )
{
   PHB_ITEM element = hb_itemNew(NULL);

   hb_arraySet(aRect, 1, hb_itemPutNL(element, rc->left));
   hb_arraySet(aRect, 2, hb_itemPutNL(element, rc->top));
   hb_arraySet(aRect, 3, hb_itemPutNL(element, rc->right));
   hb_arraySet(aRect, 4, hb_itemPutNL(element, rc->bottom));
   hb_itemRelease(element);
}

//----------------------------------------------------------------------//

void wvt_Point2ArrayEx( POINT *pt, PHB_ITEM aPoint)
{

   PHB_ITEM element = hb_itemNew(NULL);

   hb_arraySet(aPoint, 1, hb_itemPutNL(element, pt->x));
   hb_arraySet(aPoint, 2, hb_itemPutNL(element, pt->y));
   hb_itemRelease(element);
}

//----------------------------------------------------------------------//

void wvt_Size2ArrayEx( SIZE *siz, PHB_ITEM aSize )
{
   PHB_ITEM element = hb_itemNew(NULL);

   hb_arraySet(aSize, 1, hb_itemPutNL(element, siz->cx));
   hb_arraySet(aSize, 2, hb_itemPutNL(element, siz->cy));
   hb_itemRelease(element);
}

//----------------------------------------------------------------------//

#define HB_PARTSTR( n )  ( ISCHAR( n ) ? HB_TCHAR_CONVTO( hb_parc(n) ) : NULL )
#define HB_PARTFREE( p ) do { if( p ) HB_TCHAR_FREE( p ); } while( 0 )

HB_FUNC( WVT__GETOPENFILENAME )
{
   OPENFILENAME ofn;
   LPTSTR lpFileName, lpstrTitle, lpstrFilter, lpstrInitialDir, lpstrDefExt;
   int size = hb_parclen( 2 );

   size += size ? 1 : 1024;
   lpFileName = ( LPTSTR ) hb_xgrab( size * sizeof( TCHAR ) );
   HB_TCHAR_CPTO( lpFileName, hb_parcx( 2 ), size - 1 );
   lpstrTitle      = HB_PARTSTR( 3 );
   lpstrFilter     = HB_PARTSTR( 4 );
   lpstrInitialDir = HB_PARTSTR( 6 );
   lpstrDefExt     = HB_PARTSTR( 7 );

   ZeroMemory( &ofn, sizeof( ofn ) );

   ofn.hInstance        = GetModuleHandle( NULL )  ;
   ofn.lStructSize      = sizeof( ofn );
   ofn.hwndOwner        = ISNIL(1) ? GetActiveWindow() : (HWND) hb_parnl( 1 ) ;
   ofn.lpstrTitle       = lpstrTitle;
   ofn.lpstrFilter      = lpstrFilter;
   ofn.Flags            = ISNIL(5) ? OFN_SHOWHELP|OFN_NOCHANGEDIR : hb_parnl( 5 ) ;
   ofn.lpstrInitialDir  = lpstrInitialDir;
   ofn.lpstrDefExt      = lpstrDefExt;
   ofn.nFilterIndex     = ISNIL(8) ? 0 : (int) hb_parni( 8 );
   ofn.lpstrFile        = lpFileName;
   ofn.nMaxFile         = size;

   if( GetOpenFileName( &ofn ) )
   {
      char * szFileName = HB_TCHAR_CONVFROM( lpFileName );
      hb_stornl( ofn.nFilterIndex, 8 );
      hb_storclen( szFileName, size, 2 ) ;
      hb_retc( szFileName );
      HB_TCHAR_FREE( szFileName );
   }
   else
   {
      hb_retc( NULL );
   }
   hb_xfree( lpFileName );
   HB_PARTFREE( lpstrTitle );
   HB_PARTFREE( lpstrFilter );
   HB_PARTFREE( lpstrInitialDir );
   HB_PARTFREE( lpstrDefExt );
}

//----------------------------------------------------------------------//

HB_FUNC( WVT__GETSAVEFILENAME )
{
   OPENFILENAME ofn;
   LPTSTR lpstrTitle, lpstrFilter, lpstrInitialDir, lpstrDefExt;
   TCHAR lpFileName[MAX_PATH + 1];

   HB_TCHAR_CPTO( lpFileName, hb_parcx( 2 ), MAX_PATH );
   lpstrTitle      = HB_PARTSTR( 3 );
   lpstrFilter     = HB_PARTSTR( 4 );
   lpstrInitialDir = HB_PARTSTR( 6 );
   lpstrDefExt     = HB_PARTSTR( 7 );

   ZeroMemory( &ofn, sizeof( ofn ) );

   ofn.hInstance       = GetModuleHandle( NULL );
   ofn.lStructSize     = sizeof( ofn );
   ofn.hwndOwner       = ISNIL   (1) ? GetActiveWindow() : (HWND) hb_parnl( 1 );
   ofn.lpstrTitle      = lpstrTitle;
   ofn.lpstrFilter     = lpstrFilter;
   ofn.Flags           = (ISNIL  (5) ? OFN_FILEMUSTEXIST|OFN_EXPLORER|OFN_NOCHANGEDIR : hb_parnl( 5 ) );
   ofn.lpstrInitialDir = lpstrInitialDir;
   ofn.lpstrDefExt     = lpstrDefExt;
   ofn.nFilterIndex    = hb_parni(8);
   ofn.lpstrFile       = lpFileName;
   ofn.nMaxFile        = MAX_PATH;

   if( GetSaveFileName( &ofn ) )
   {
      char * szFileName = HB_TCHAR_CONVFROM( lpFileName );
      hb_stornl( ofn.nFilterIndex, 8 );
      hb_retc( szFileName );
      HB_TCHAR_FREE( szFileName );
   }
   else
   {
      hb_retc( NULL );
   }

   HB_PARTFREE( lpstrTitle );
   HB_PARTFREE( lpstrFilter );
   HB_PARTFREE( lpstrInitialDir );
   HB_PARTFREE( lpstrDefExt );
}
//----------------------------------------------------------------------//

HB_FUNC( WIN_AND )
{
   hb_retnl( hb_parnl(1) & hb_parnl(2) ) ;
}

//----------------------------------------------------------------------//

HB_FUNC( WIN_OR )
{
   hb_retnl( hb_parnl(1) | hb_parnl(2) ) ;
}

//----------------------------------------------------------------------//

HB_FUNC( WIN_NOT )
{
   hb_retnl( ~( hb_parnl(1) ) ) ;
}

//----------------------------------------------------------------------//

HB_FUNC( WIN_TRACKPOPUPMENU )
{
   HMENU hMenu  = ( HMENU ) hb_parnl( 1 );
   UINT  uFlags = ISNIL( 2 ) ? TPM_CENTERALIGN | TPM_RETURNCMD : hb_parnl( 2 );
   HWND  hWnd   = ISNIL( 3 ) ? GetActiveWindow() : ( HWND ) hb_parnl( 3 );

   POINT xy = { 0,0 };

   GetCursorPos( &xy );

   hb_retnl( TrackPopupMenu( hMenu, uFlags, xy.x, xy.y, 0, hWnd, NULL ) );
}

//-------------------------------------------------------------------//

HB_FUNC( WIN_CHOOSECOLOR )
{
   CHOOSECOLOR cc ;
   COLORREF    crCustClr[ 16 ] ;
   int         i ;

   for( i = 0 ; i < 16 ; i++ )
   {
     crCustClr[ i ] = ( ISARRAY( 2 ) ? ( COLORREF ) hb_parnl( 2, i+1 ) : GetSysColor( COLOR_BTNFACE ) ) ;
   }

   cc.lStructSize   = sizeof( CHOOSECOLOR ) ;
   cc.hwndOwner     = ISNIL( 4 ) ? NULL : (HWND) hb_parnl( 4 );
   cc.rgbResult     = ISNIL( 1 ) ?  0 : ( COLORREF ) hb_parnl( 1 ) ;
   cc.lpCustColors  = crCustClr ;
   cc.Flags         = ( WORD ) ( ISNIL( 3 ) ? CC_ANYCOLOR | CC_RGBINIT | CC_FULLOPEN : hb_parnl( 3 ) );

   if ( ChooseColor( &cc ) )
   {
      hb_retnl( cc.rgbResult ) ;
   }
   else
   {
      hb_retnl( -1 );
   }
}

//-------------------------------------------------------------------//

HB_FUNC( WIN_FINDWINDOW )
{
   HWND hwnd = FindWindow( NULL, hb_parc( 1 ) );
   if ( hwnd )
   {
      hb_retnl( (LONG) hwnd );
   }
   else
   {
      hb_retnl( -1 );
   }
}

//----------------------------------------------------------------------//

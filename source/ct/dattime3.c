/*
 * $Id: dattime3.c 9784 2012-10-23 21:26:49Z andijahja $
 */

/*
 * Harbour Project source code:
 *   CT3 Date & Time functions:
 *       WAITPERIOD(), TIMEVALID(), SETTIME(), SETDATE()
 *
 * Copyright 2007 Przemyslaw Czerpak <druzus / at / priv.onet.pl>
 * www - http://www.harbour-project.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307 USA (or visit the web site http://www.gnu.org/).
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
 * not apply to the code that you add in this way.  To avoid misleading
 * anyone as to the status of such modified files, you must delete
 * this exception notice from them.
 *
 * If you write modifications of your own for Harbour, it is your choice
 * whether to permit this exception to apply to your modifications.
 * If you do not wish that, delete this exception notice.
 *
 */

#define HB_OS_WIN_USED

#include "hbapi.h"
#include "hbdate.h"
#if defined( HB_OS_WIN )
#   include <windows.h>
#   include <winbase.h>
#elif defined( HB_OS_UNIX )
/* stime exists only in SVr4, SVID, X/OPEN and Linux */
#   ifndef _SVID_SOURCE
#      define _SVID_SOURCE
#   endif
#endif
#include <time.h>

/* TODO: make it MT safe */
static double  s_dTimeSet     = 0;
static double  s_dTimeCounter = 0;

HB_FUNC( WAITPERIOD )
{
   double d = hb_dateSeconds();

   if( hb_pcount() > 0 )
   {
      s_dTimeSet     = d;
      s_dTimeCounter = d + hb_parnd( 1 ) / 100.0;
   }

   if( d < s_dTimeSet )
      d += 86400.0;

   hb_retl( d < s_dTimeCounter );
}

static BOOL _hb_timeValid( const char * szTime, HB_SIZE ulLen, int * piDecode )
{
   BOOL fValid = FALSE;

   /* NOTE: In CA-Clipper Tools, timevalid() is limited to HH:MM:SS:hh (hundredth).
            In xHarbour, timevalid() is extended to HH:MM:SS:ttt (thousandth).
    */
   if( ulLen == 2 || ulLen == 5 || ulLen == 8 || ulLen == 11 || ulLen == 12 )
   {
      static int  s_iMax[] = { 23, 59, 59, 99 };

      int         i, iVal;
      ULONG       ul;

      if( ulLen == 12 )
         s_iMax[ 3 ] = 999;

      fValid = TRUE;
      for( ul = 0; fValid && ul < ulLen; ++ul )
      {
         fValid = ul < 11 && ul % 3 == 2 ? szTime[ ul ] == ':' :
                  ( szTime[ ul ] >= '0' && szTime[ ul ] <= '9' );
      }
      for( ul = 0, i = 0; fValid && ul < ulLen; ul += 3, ++i )
      {
         iVal     = 10 * ( szTime[ ul ] - '0' ) + ( szTime[ ul + 1 ] - '0' );
         fValid   = iVal <= s_iMax[ i ];
         if( piDecode )
            piDecode[ i ] = iVal;
      }
   }

   return fValid;
}

HB_FUNC( TIMEVALID )
{
   hb_retl( _hb_timeValid( hb_parc( 1 ), hb_parclen( 1 ), NULL ) );
}

HB_FUNC( SETTIME )
{
   BOOL  fResult = FALSE;
   int   iTime[ 4 ];

   iTime[ 0 ] = iTime[ 1 ] = iTime[ 2 ] = iTime[ 3 ] = 0;
   if( _hb_timeValid( hb_parc( 1 ), hb_parclen( 1 ), iTime ) )
   {
#if defined( HB_OS_WIN )
      SYSTEMTIME st;
      GetLocalTime( &st );
      st.wHour          = ( WORD ) iTime[ 0 ];
      st.wMinute        = ( WORD ) iTime[ 1 ];
      st.wSecond        = ( WORD ) iTime[ 2 ];
      st.wMilliseconds  = ( WORD ) iTime[ 3 ] * 10;
      fResult           = SetLocalTime( &st );
#elif defined( HB_OS_UNIX ) && ! defined( __WATCOMC__ )
/* stime exists only in SVr4, SVID, X/OPEN and Linux */
      ULONG    lNewTime;
      time_t   tm;

      lNewTime = iTime[ 0 ] * 3600 + iTime[ 1 ] * 60 + iTime[ 2 ];
      tm       = time( NULL );
      tm       += lNewTime - ( tm % 86400 );
      fResult  = stime( &tm ) == 0;
#endif
   }

   hb_retl( fResult );
}

HB_FUNC( SETDATE )
{
   BOOL  fResult  = FALSE;
   long  lDate    = hb_pardl( 1 );

   if( lDate )
   {
      int iYear, iMonth, iDay;

      hb_dateDecode( lDate, &iYear, &iMonth, &iDay );
      if( iYear >= 1970 )
      {
#if defined( HB_OS_WIN )
         SYSTEMTIME st;
         GetLocalTime( &st );
         st.wYear       = ( WORD ) iYear;
         st.wMonth      = ( WORD ) iMonth;
         st.wDay        = ( WORD ) iDay;
         st.wDayOfWeek  = ( WORD ) hb_dateJulianDOW( lDate );
         fResult        = SetLocalTime( &st );
#elif defined( HB_OS_UNIX ) && ! defined( __WATCOMC__ )
/* stime exists only in SVr4, SVID, X/OPEN and Linux */
         long     lNewDate;
         time_t   tm;

         lNewDate = lDate - hb_dateEncode( 1970, 1, 1 );
         tm       = time( NULL );
         tm       = lNewDate * 86400 + ( tm % 86400 );
         fResult  = stime( &tm ) == 0;
#endif
      }
   }

   hb_retl( fResult );
}

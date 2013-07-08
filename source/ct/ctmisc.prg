/*
 * $Id: ctmisc.prg 9736 2012-10-10 12:21:17Z andijahja $
 */

/*
 * Harbour Project source code:
 * Misc CA-Tools functions
 *
 * Copyright 1999-2001 Viktor Szakats <viktor.szakats@syenar.hu>
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

#include "color.ch"
#include "common.ch"
#include "setcurs.ch"

MEMVAR GetList

FUNCTION CENTER( c, n, p, lMode )

   LOCAL cRet

   DEFAULT n TO MaxCol() + 1 - Col() * 2
   DEFAULT c TO ""
   DEFAULT lMode TO .F.
   cRet := PadC( AllTrim( c ), n, p )

   RETURN IF( lMode, cRet, RTrim( cRet ) )

FUNCTION CSETCURS( l )

   IF PCount() == 0
      RETURN SetCursor() != SC_NONE
   ENDIF

   RETURN SetCursor( iif( l, SC_NORMAL, SC_NONE ) ) != SC_NONE

FUNCTION CSETKEY( n )

   RETURN SetKey( n )

FUNCTION CSETCENT( nCentury )

   IF nCentury == NIL
      RETURN __SetCentury()
   ELSE
      RETURN __SetCentury( nCentury )
   ENDIF

   RETURN NIL

FUNCTION LTOC( l )

   RETURN iif( l, "T", "F" )

FUNCTION RESTGETS( aGetList )

   GetList := aGetList

   RETURN .T.

FUNCTION SAVEGETS()

   LOCAL aGetList := GetList

   GetList := {}

   RETURN aGetList

FUNCTION DOSPARAM

   LOCAL cRet := ""
   LOCAL nCount := hb_argc(), i

   FOR i := 1 TO nCount
      cRet += if( i == 1, "", " " ) + hb_argv( i )
   NEXT

   RETURN cRet

FUNCTION SetRC( nRow, nCol )

   SetPos( nRow, nCol )

   RETURN ""

#ifndef __PLATFORM__Windows

FUNCTION EXENAME()

   RETURN hb_argv( 0 )

#endif

#ifdef __PLATFORM__Windows
#pragma BEGINDUMP

#include "hbapi.h"
#include "windows.h"
HB_FUNC( EXENAME )
{
   char szBuffer[ MAX_PATH + 1 ] = {0} ;

   // GetModuleFileName( ISNIL(1) ? GetModuleHandle( NULL ) : (HMODULE) hb_parnl( 1 ), szBuffer ,MAX_PATH );
   GetModuleFileName( ISNIL(1) ? GetModuleHandle( NULL ) : (HMODULE) hb_parns( 1 ), szBuffer ,MAX_PATH );

   hb_retc( szBuffer );
}

#pragma enddump

#endif

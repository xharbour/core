/*
 * $Id: accept.c 9759 2012-10-15 14:02:59Z andijahja $
 */

/*
 * Harbour Project source code:
 * ACCEPT command related functions
 *
 * Copyright 1999 Eddie Runia <eddie@runia.com>
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

/*
 * The following parts are Copyright of the individual authors.
 * www - http://www.harbour-project.org
 *
 * Copyright 1999-2001 Viktor Szakats <viktor.szakats@syenar.hu>
 *    __ACCEPTSTR()
 *
 * See doc/license.txt for licensing terms.
 *
 */

#include "hbapi.h"
#include "hbvm.h"
#include "hbapigt.h"
#include "inkey.ch"

HB_FUNC_EXTERN( QOUT );

#define ACCEPT_BUFFER_LEN 256 /* length of input buffer for ACCEPT command */

#ifdef HB_C52_UNDOC

static char s_szAcceptResult[ ACCEPT_BUFFER_LEN ] = { '\0' };

HB_FUNC( __ACCEPTSTR )
{
   hb_retc( s_szAcceptResult );
}

#endif

HB_FUNC( __ACCEPT )
{
   char  szAcceptResult[ ACCEPT_BUFFER_LEN ];
   ULONG ulLen;
   int   input;

   /* cPrompt(s) passed ? */
   if( hb_pcount() >= 1 )
      HB_FUNC_EXEC( QOUT );

   ulLen                = 0;
   input                = 0;

   szAcceptResult[ 0 ]  = '\0';

   while( input != K_ENTER && hb_vmRequestQuery() == 0 )
   {
      /* Wait forever, for keyboard events only */
      input = hb_inkey( TRUE, 0.0, INKEY_KEYBOARD );
      switch( input )
      {
         case K_BS:
         case K_LEFT:
            if( ulLen > 0 )
            {
               hb_conOutAlt( "\x8", sizeof( char ) ); /* Erase it from the screen. */
               ulLen--;                               /* Adjust input count to get rid of last character */
            }
            break;

         default:
            if( ulLen < ( ACCEPT_BUFFER_LEN - 1 ) && input >= 32 && input <= 255 )
            {
               szAcceptResult[ ulLen ] = ( char ) input;                   /* Accept the input */
               hb_conOutAlt( &szAcceptResult[ ulLen ], sizeof( char ) );   /* Then display it */
               ulLen++;                                                    /* Then adjust the input count */
            }
      }
   }

   szAcceptResult[ ulLen ] = '\0';

#ifdef HB_C52_UNDOC
   hb_strncpy( s_szAcceptResult, szAcceptResult, sizeof( s_szAcceptResult ) - 1 );
#endif

   hb_retc( szAcceptResult );
}


/*
 * $Id: is.c 9741 2012-10-11 23:10:30Z andijahja $
 */

/*
 * Harbour Project source code:
 * IS*() string functions
 *
 * Copyright 1999 Antonio Linares <alinares@fivetech.com>
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

#include <ctype.h>

#include "hbapi.h"

#ifndef HB_CDP_SUPPORT_OFF
#include "hbapicdp.h"
#endif

/* determines if first char of string is letter */

HB_FUNC( ISALPHA )
{
   const char * szString = hb_parc( 1 );

   if( szString != NULL )
   {
      if( HB_ISALPHA( ( BYTE ) *szString ) )
         hb_retl( TRUE );
#ifndef HB_CDP_SUPPORT_OFF
      else if( ( hb_cdppage() )->nChars && szString[ 0 ] &&
               ( strchr( ( hb_cdppage() )->CharsUpper, *szString ) || strchr( ( hb_cdppage() )->CharsLower, *szString ) ) )
         hb_retl( TRUE );
#endif
      else
         hb_retl( FALSE );
   }
   else
      hb_retl( FALSE );
}

/* determines if first char of string is digit */

HB_FUNC( ISDIGIT )
{
   const char * szString = hb_parc( 1 );

   if( szString != NULL )
      hb_retl( HB_ISDIGIT( ( BYTE ) *szString ) );
   else
      hb_retl( FALSE );
}

/* determines if first char of string is upper-case */

HB_FUNC( ISUPPER )
{
   const char * szString = hb_parc( 1 );

   if( szString != NULL )
   {
      if( HB_ISUPPER( ( BYTE ) *szString ) )
         hb_retl( TRUE );
#ifndef HB_CDP_SUPPORT_OFF
      else if( ( hb_cdppage() )->nChars && szString[ 0 ] && strchr( ( hb_cdppage() )->CharsUpper, *szString ) )
         hb_retl( TRUE );
#endif
      else
         hb_retl( FALSE );
   }
   else
      hb_retl( FALSE );
}

/* determines if first char of string is lower-case */

HB_FUNC( ISLOWER )
{
   const char * szString = hb_parc( 1 );

   if( szString != NULL )
   {
      if( HB_ISLOWER( ( BYTE ) *szString ) )
         hb_retl( TRUE );
#ifndef HB_CDP_SUPPORT_OFF
      else if( ( hb_cdppage() )->nChars && szString[ 0 ] && strchr( ( hb_cdppage() )->CharsLower, *szString ) )
         hb_retl( TRUE );
#endif
      else
         hb_retl( FALSE );
   }
   else
      hb_retl( FALSE );
}

/* determines if first char of a string is an alphanumeric character */

HB_FUNC( ISALNUM )
{
   const char * szString = hb_parc( 1 );

   if( szString != NULL )
   {
      hb_retl( HB_ISALNUM( ( BYTE ) *szString ) );
   }
   else
      hb_retl( FALSE );
}

/* determines if first char of a string is a control character;
   that is, if it is in the range 0 - 31 or 127 (0x00 - 0x1f or 0x7f).
 */

HB_FUNC( ISCNTRL )
{
   const char * szString = hb_parc( 1 );

   if( szString != NULL )
   {
      hb_retl( iscntrl( ( BYTE ) *szString ) );
   }
   else
      hb_retl( FALSE );
}

/* determines if first char of a string is a printable character.
   The space character (' ') is not considered a printable character.
 */

HB_FUNC( ISGRAPH )
{
   const char * szString = hb_parc( 1 );

   if( szString != NULL )
   {
      hb_retl( isgraph( ( BYTE ) *szString ) );
   }
   else
      hb_retl( FALSE );
}

/* determines if first char of a string is a printable character.
   Printable characters have an ASCII value between 32 - 126, (0x20 - 0x7e),
   a space and the tilde, inclusive.
 */

HB_FUNC( ISPRINT )
{
   const char * szString = hb_parc( 1 );

   if( szString != NULL )
   {
      hb_retl( isprint( ( BYTE ) *szString ) );
   }
   else
      hb_retl( FALSE );
}

/* determines if first char of a string is a punctuation character.
   A punctuation character is one that is not alphabetic, not numeric,
   not a control character, and not a white space.
 */

HB_FUNC( ISPUNCT )
{
   const char * szString = hb_parc( 1 );

   if( szString != NULL )
   {
      hb_retl( ispunct( ( BYTE ) *szString ) );
   }
   else
      hb_retl( FALSE );
}

/* determines if first char of a string is a white�space character;
   that is, a horizontal tab, a new�line, a vertical tab, a form�feed,
   a carriage�return or a space.
 */

HB_FUNC( ISSPACE )
{
   const char * szString = hb_parc( 1 );

   if( szString != NULL )
   {
      hb_retl( isspace( ( BYTE ) *szString ) );
   }
   else
      hb_retl( FALSE );
}

/* determines if first char of a string is a hexadecimal digit
   ('A' � 'F', 'a' � 'f', or '0' �'9').
 */

HB_FUNC( ISXDIGIT )
{
   const char * szString = hb_parc( 1 );

   if( szString != NULL )
   {
      hb_retl( isxdigit( ( BYTE ) *szString ) );
   }
   else
      hb_retl( FALSE );
}

/* determines if first char of a string is a member of the 7-bit ASCII
   character set; that is, if: 0  <=  c  <= 127
 */

HB_FUNC( ISASCII )
{
   const char * szString = hb_parc( 1 );

   if( szString != NULL )
   {
#if defined( __XCC__ )
      hb_retl( (unsigned)(( BYTE ) *szString) < 0x80 );
#else
      hb_retl( isascii( ( BYTE ) *szString ) );
#endif
   }
   else
      hb_retl( FALSE );
}

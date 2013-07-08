/*
 * $Id: trim.c 9724 2012-10-02 10:36:35Z andijahja $
 */

/*
 * Harbour Project source code:
 * *TRIM() functions
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
#include "hbapi.h"
#include "hbapiitm.h"
#include "hbapierr.h"

/* trims from the left, and returns a new pointer to szText */
/* also returns the new length in lLen */
char * hb_strLTrim( const char * szText, HB_SIZE * ulLen )
{
   HB_TRACE( HB_TR_DEBUG, ( "hb_strLTrim(%s, %p)", szText, ulLen ) );

   while( *ulLen && HB_ISSPACE( *szText ) )
   {
      szText++;
      ( *ulLen )--;
   }

   return ( char * ) szText;
}

/* return length of szText ignoring trailing white space (or true spaces) */
HB_SIZE hb_strRTrimLen( const char * szText, HB_SIZE ulLen, BOOL bAnySpace )
{
   HB_TRACE( HB_TR_DEBUG, ( "hb_strRTrimLen(%s, %lu. %d)", szText, ulLen, ( int ) bAnySpace ) );

   if( bAnySpace )
   {
      while( ulLen && HB_ISSPACE( szText[ ulLen - 1 ] ) )
         ulLen--;
   }
   else
   {
      while( ulLen && szText[ ulLen - 1 ] == ' ' )
         ulLen--;
   }

   return ulLen;
}

/* trims leading spaces from a string */

HB_FUNC( LTRIM )
{
   PHB_ITEM pText = hb_param( 1, HB_IT_STRING );

   if( pText )
   {
      HB_SIZE  ulLen, ulSrc;
      char *   szText;

      ulLen    = ulSrc = pText->item.asString.length;
      szText   = hb_strLTrim( pText->item.asString.value, &ulLen );

      if( ulLen == ulSrc )
      {
         hb_itemReturn( pText );
      }
      else
      {
         hb_retclen( szText, ulLen );
      }
   }
   else
   {
      hb_errRT_BASE_SubstR( EG_ARG, 1101, NULL, "LTRIM", 1, hb_paramError( 1 ) );
   }
}

/* trims trailing spaces from a string */

/* NOTE: The second parameter is a Harbour extension. */

HB_FUNC( RTRIM )
{
   PHB_ITEM pText = hb_param( 1, HB_IT_STRING );

   if( pText )
   {
      HB_SIZE  ulLen, ulSrc;
      char *   szText = pText->item.asString.value;

      ulSrc = pText->item.asString.length;
#ifdef HB_EXTENSION
      ulLen = hb_strRTrimLen( szText, ulSrc, ISLOG( 2 ) && hb_parl( 2 ) );
#else
      ulLen = hb_strRTrimLen( szText, ulSrc, FALSE );
#endif

      if( ulLen == ulSrc )
      {
         hb_itemReturn( pText );
      }
      else
      {
         hb_retclen( szText, ulLen );
      }
   }
   else
      /* NOTE: "TRIM" is right here [vszakats] */
      hb_errRT_BASE_SubstR( EG_ARG, 1100, NULL, "TRIM", 1, hb_paramError( 1 ) );
}

/* synonymn for RTRIM */
HB_FUNC( TRIM )
{
   HB_FUNC_EXEC( RTRIM );
}

/* trims leading and trailing spaces from a string */

/* NOTE: The second parameter is a Harbour extension. */

HB_FUNC( ALLTRIM )
{
   PHB_ITEM pText = hb_param( 1, HB_IT_STRING );

   if( pText )
   {
      HB_SIZE  ulLen, ulSrc;
      char *   szText = pText->item.asString.value;

      ulSrc    = pText->item.asString.length;
#ifdef HB_EXTENSION
      ulLen    = hb_strRTrimLen( szText, ulSrc, ISLOG( 2 ) && hb_parl( 2 ) );
#else
      ulLen    = hb_strRTrimLen( szText, ulSrc, FALSE );
#endif
      szText   = hb_strLTrim( szText, &ulLen );

      if( ulLen == ulSrc )
      {
         hb_itemReturn( pText );
      }
      else
      {
         hb_retclen( szText, ulLen );
      }
   }
   else
#ifdef HB_COMPAT_C53
      hb_errRT_BASE_SubstR( EG_ARG, 2022, NULL, "ALLTRIM", 1, hb_paramError( 1 ) );  /* NOTE: This appeared in CA-Cl*pper 5.3 [vszakats] */
#else
      hb_retc( "" );
#endif
}

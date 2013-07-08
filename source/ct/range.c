/*
 * $Id: range.c 9784 2012-10-23 21:26:49Z andijahja $
 */

/*
 * Harbour Project source code:
 *   RANGEREM() and RANGEREPL() CT3 string functions
 *
 * Copyright 2001 IntTec GmbH, Neunlindenstr 32, 79106 Freiburg, Germany
 *        Author: Martin Vogel <vogel@inttec.de>
 *
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

#include "ct.h"

HB_FUNC( RANGEREM )
{
   if( ( ( hb_parclen( 1 ) > 0 ) || ISNUM( 1 ) ) &&
       ( ( hb_parclen( 2 ) > 0 ) || ISNUM( 2 ) ) &&
       ISCHAR( 3 ) )
   {

      char *            pcString = ( char * ) hb_parc( 3 );
      size_t            sStrLen  = ( size_t ) hb_parclen( 3 );
      char *            pcRet;
      unsigned char *   pc;
      unsigned char     ucChar1, ucChar2;
      size_t            sRetIndex;
      int               iMode, iBool;

      if( ISNUM( 1 ) )
      {
         ucChar1 = ( unsigned char ) ( hb_parnl( 1 ) % 256 );
      }
      else
      {
         ucChar1 = *( ( unsigned char * ) hb_parc( 1 ) );
      }

      if( ISNUM( 2 ) )
      {
         ucChar2 = ( unsigned char ) ( hb_parnl( 2 ) % 256 );
      }
      else
      {
         ucChar2 = *( ( unsigned char * ) hb_parc( 2 ) );
      }

      iMode       = ( ucChar2 < ucChar1 );

      pcRet       = ( char * ) hb_xgrab( sStrLen + 1 );
      sRetIndex   = 0;
      for( pc = ( unsigned char * ) pcString; pc < ( unsigned char * ) pcString + sStrLen; pc++ )
      {
         iBool = ( ( *pc ) >= ucChar1 );
         if( iMode )
         {
            iBool |= ( ( *pc ) <= ucChar2 );
         }
         else
         {
            iBool &= ( ( *pc ) <= ucChar2 );
         }

         if( ! iBool )
         {
            *( pcRet + sRetIndex ) = *pc;
            sRetIndex++;
         }
      }

      hb_retclen( pcRet, sRetIndex );
      hb_xfree( pcRet );

   }
   else /* ((hb_parclen (1) > 0) || ISNUM (1)) &&
           ((hb_parclen (2) > 0) || ISNUM (2)) &&
           ISCHAR (3)) */
   {
      PHB_ITEM pSubst         = NULL;
      int      iArgErrorMode  = ct_getargerrormode();
      if( iArgErrorMode != CT_ARGERR_IGNORE )
      {
         pSubst = ct_error_subst( ( USHORT ) iArgErrorMode, EG_ARG, CT_ERROR_RANGEREM,
                                  NULL, "RANGEREM", 0, EF_CANSUBSTITUTE, 3,
                                  hb_paramError( 1 ), hb_paramError( 2 ),
                                  hb_paramError( 3 ) );
      }

      if( pSubst != NULL )
      {
         hb_itemRelease( hb_itemReturnForward( pSubst ) );
      }
      else
      {
         if( ISCHAR( 3 ) )
         {
            hb_retclen( hb_parc( 3 ), hb_parclen( 3 ) );
         }
         else
         {
            hb_retc( "" );
         }
      }
   }
}

HB_FUNC( RANGEREPL )
{
   int iNoRef = ct_getref() && ISBYREF( 3 );

   if( ( ( hb_parclen( 1 ) > 0 ) || ISNUM( 1 ) ) &&
       ( ( hb_parclen( 2 ) > 0 ) || ISNUM( 2 ) ) &&
       ISCHAR( 3 ) &&
       ( ( hb_parclen( 4 ) > 0 ) || ISNUM( 4 ) ) )
   {

      char *            pcString = ( char * ) hb_parc( 3 );
      size_t            sStrLen  = ( size_t ) hb_parclen( 3 );
      char *            pcRet;
      unsigned char *   pc;
      unsigned char     ucChar1, ucChar2, ucReplace;
      size_t            sRetIndex;
      int               iMode, iBool;

      if( ISNUM( 1 ) )
      {
         ucChar1 = ( unsigned char ) ( hb_parnl( 1 ) % 256 );
      }
      else
      {
         ucChar1 = *( ( unsigned char * ) hb_parc( 1 ) );
      }

      if( ISNUM( 2 ) )
      {
         ucChar2 = ( unsigned char ) ( hb_parnl( 2 ) % 256 );
      }
      else
      {
         ucChar2 = *( ( unsigned char * ) hb_parc( 2 ) );
      }

      if( ISNUM( 4 ) )
      {
         ucReplace = ( unsigned char ) ( hb_parnl( 4 ) % 256 );
      }
      else
      {
         ucReplace = *( ( unsigned char * ) hb_parc( 4 ) );
      }

      iMode       = ( ucChar2 < ucChar1 );

      pcRet       = ( char * ) hb_xgrab( sStrLen + 1 );
      sRetIndex   = 0;
      for( pc = ( unsigned char * ) pcString; pc < ( unsigned char * ) pcString + sStrLen; pc++ )
      {
         iBool = ( ( *pc ) >= ucChar1 );
         if( iMode )
         {
            iBool |= ( ( *pc ) <= ucChar2 );
         }
         else
         {
            iBool &= ( ( *pc ) <= ucChar2 );
         }

         if( iBool )
         {
            *( pcRet + sRetIndex ) = ucReplace;
            sRetIndex++;
         }
         else
         {
            *( pcRet + sRetIndex ) = *pc;
            sRetIndex++;
         }
      }

      if( ISBYREF( 3 ) )
      {
         hb_storclen( pcRet, sStrLen, 3 );
      }

      if( iNoRef )
      {
         /* Contrary to the official documentation, RANGREPL() returns NIL instead of .F.
          * in this situation. If the string is not passed by reference, it returns the
          * string regardless of iNoRef. */
         hb_ret();
      }
      else
      {
         hb_retclen( pcRet, sStrLen );
      }

      hb_xfree( pcRet );

   }
   else /* ((hb_parclen (1) > 0) || ISNUM (1)) &&
           ((hb_parclen (2) > 0) || ISNUM (2)) &&
           ISCHAR (3) &&
           ((hb_parclen (4) > 0))) */
   {
      PHB_ITEM pSubst         = NULL;
      int      iArgErrorMode  = ct_getargerrormode();
      if( iArgErrorMode != CT_ARGERR_IGNORE )
      {
         pSubst = ct_error_subst( ( USHORT ) iArgErrorMode, EG_ARG, CT_ERROR_RANGEREPL,
                                  NULL, "RANGEREPL", 0, EF_CANSUBSTITUTE, 4,
                                  hb_paramError( 1 ), hb_paramError( 2 ),
                                  hb_paramError( 3 ), hb_paramError( 4 ) );
      }

      if( pSubst != NULL )
      {
         hb_itemRelease( hb_itemReturnForward( pSubst ) );
      }
      else
      {
         if( iNoRef )
         {
            hb_ret();
         }
         else
         {
            if( ISCHAR( 3 ) )
            {
               hb_retclen( hb_parc( 3 ), hb_parclen( 3 ) );
            }
            else
            {
               hb_retc( "" );
            }
         }
      }
   }
}


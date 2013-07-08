/*
 * $Id: wordrepl.c 9722 2012-10-02 06:13:24Z andijahja $
 */

/*
 * Harbour Project source code:
 *   CT3 string function WORDREPL()
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

HB_FUNC( WORDREPL )
{
   int      iNoRet;
   int      iMultiPass;

   HB_SIZE  sSearchLen, sReplaceLen;

   /* suppressing return value ? */
   iNoRet      = ct_getref() && ISBYREF( 2 );
   iMultiPass  = ct_getatmupa();

   /* param check */
   if( ( ( sSearchLen = hb_parclen( 1 ) ) / 2 > 0 ) &&
       ( ISCHAR( 2 ) ) &&
       ( ( sReplaceLen = hb_parclen( 3 ) ) / 2 > 0 ) )
   {
      /* get parameters */
      const char *   pcSearch    = hb_parc( 1 );
      const char *   pcString    = hb_parc( 2 );
      HB_SIZE        sStrLen     = hb_parclen( 2 );
      const char *   pcReplace   = hb_parc( 3 );
      int            iMode;
      char *         pcRet;
      HB_SIZE        sIndex;

      if( ISLOG( 4 ) )
      {
         iMode = hb_parl( 4 );
      }
      else
      {
         iMode = 0;
      }

      pcRet = ( char * ) hb_xgrab( sStrLen + 1 );
      hb_xmemcpy( pcRet, pcString, (size_t) sStrLen );

      for( sIndex = 0; sIndex < ( sSearchLen & 0xFFFFFFFE ); sIndex += 2 )
      {
         HB_SIZE      sMatchStrLen;
         const char * pc;
         HB_SIZE      sReplIndex = sIndex;

         if( sReplIndex > ( sReplaceLen & 0xFFFFFFFE ) )
         {
            sReplIndex = ( sReplaceLen & 0xFFFFFFFE );
         }

         pc = pcString;

         while( ( pc = ct_at_exact_forward( pc, sStrLen - ( pc - pcString ),
                                            pcSearch + sIndex, 2,
                                            &sMatchStrLen ) ) != NULL )
         {
            if( iMode )
            {
               /* always replace */
               *( pcRet + ( pc - pcString ) )      = *( pcReplace + sReplIndex );
               *( pcRet + ( pc - pcString ) + 1 )  = *( pcReplace + sReplIndex + 1 );

               if( iMultiPass )
               {
                  pc++;
               }
               else
               {
                  pc += 2;
               }
            }
            else
            {
               /* replace only if pc is an even position */
               if( ( ( pc - pcString ) % 2 ) == 0 )
               {
                  *( pcRet + ( pc - pcString ) )      = *( pcReplace + sReplIndex );
                  *( pcRet + ( pc - pcString ) + 1 )  = *( pcReplace + sReplIndex + 1 );
                  /* parse pcString in steps of two characters */
                  pc                                  += 2;
               }
               else
               {
                  /* we are on an odd position, so add only 1 to pc */
                  pc++;
               }
            }
         }
      }

      /* return string */
      if( ISBYREF( 2 ) )
      {
         hb_storclen( pcRet, sStrLen, 2 );
      }

      if( iNoRet )
      {
         hb_retl( 0 );
      }
      else
      {
         hb_retclen( pcRet, sStrLen );
      }

      hb_xfree( pcRet );

   }
   else /* ((sSearchLen = (size_t)hb_parclen (1))/2 > 0) &&
           (ISCHAR (2)) &&
           ((sReplaceLen = (size_t)hb_parclen (3))/2 > 0))  */
   {
      PHB_ITEM pSubst         = NULL;
      int      iArgErrorMode  = ct_getargerrormode();

      if( iArgErrorMode != CT_ARGERR_IGNORE )
      {
         pSubst = ct_error_subst( ( USHORT ) iArgErrorMode, EG_ARG, CT_ERROR_WORDREPL,
                                  NULL, "WORDREPL", 0, EF_CANSUBSTITUTE, 4,
                                  hb_paramError( 1 ), hb_paramError( 2 ),
                                  hb_paramError( 3 ), hb_paramError( 4 ) );
      }

      if( pSubst != NULL )
      {
         hb_itemRelease( hb_itemReturnForward( pSubst ) );
      }
      else
      {
         if( iNoRet )
         {
            hb_retl( 0 );
         }
         else
         {
            if( ISCHAR( 2 ) )
            {
               hb_retclen( hb_parc( 2 ), hb_parclen( 2 ) );
            }
            else
            {
               hb_retc( "" );
            }
         }
      }
   }

   return;
}


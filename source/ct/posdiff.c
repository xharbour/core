/*
 * $Id: posdiff.c 9784 2012-10-23 21:26:49Z andijahja $
 */

/*
 * Harbour Project source code:
 *   POSDIFF() and POSEQUAL() CT3 string functions
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

HB_FUNC( POSDIFF )
{
   if( ISCHAR( 1 ) && ISCHAR( 2 ) )
   {
      const char *  pcString1   = hb_parc( 1 );
      HB_SIZE       sStrLen1    = hb_parclen( 1 );
      const char *  pcString2   = hb_parc( 2 );
      HB_SIZE       sStrLen2    = hb_parclen( 2 );
      const char *  pc1, * pc2;
      HB_SIZE       sIgnore;

      if( ISNUM( 3 ) )
         sIgnore = hb_parnl( 3 );
      else
         sIgnore = 0;

      if( ( sIgnore > sStrLen1 ) || ( sIgnore > sStrLen2 ) )
      {
         int iArgErrorMode = ct_getargerrormode();
         if( iArgErrorMode != CT_ARGERR_IGNORE )
         {
            ct_error( ( USHORT ) iArgErrorMode, EG_ARG, CT_ERROR_POSDIFF,
                      NULL, "POSDIFF", 0, EF_CANDEFAULT, 3,
                      hb_paramError( 1 ), hb_paramError( 2 ),
                      hb_paramError( 3 ) );
         }

         hb_retnl( 0 );
         return;
      }

      pc1   = pcString1 + sIgnore;
      pc2   = pcString2 + sIgnore;

      while( ( pc1 < pcString1 + sStrLen1 ) && ( pc2 < pcString2 + sStrLen2 ) )
      {
         if( *pc1 != *pc2 )
         {
            hb_retnl( ( LONG ) ( pc1 - pcString1 ) + 1 );
            return;
         }
         pc1++;
         pc2++;
      }

      if( sStrLen1 != sStrLen2 )
      {
         hb_retnl( ( LONG ) ( sStrLen1 < sStrLen2 ? sStrLen1 : sStrLen2 ) + 1 );
      }
      else
      {
         hb_retnl( 0 );
      }

      return;

   }
   else /* (ISCHAR (1) && ISCHAR (2)) */
   {
      PHB_ITEM pSubst         = NULL;
      int      iArgErrorMode  = ct_getargerrormode();
      if( iArgErrorMode != CT_ARGERR_IGNORE )
      {
         pSubst = ct_error_subst( ( USHORT ) iArgErrorMode, EG_ARG, CT_ERROR_POSDIFF,
                                  NULL, "POSDIFF", 0, EF_CANSUBSTITUTE, 3,
                                  hb_paramError( 1 ), hb_paramError( 2 ), hb_paramError( 3 ) );
      }

      if( pSubst != NULL )
      {
         hb_itemRelease( hb_itemReturnForward( pSubst ) );
      }
      else
      {
         if( ISCHAR( 1 ) || ISCHAR( 2 ) )
            hb_retnl( 1 );
         else
            hb_retnl( 0 );
      }
   }
}

HB_FUNC( POSEQUAL )
{
   if( ISCHAR( 1 ) && ISCHAR( 2 ) )
   {
      const char * pcString1   = hb_parc( 1 );
      HB_SIZE      sStrLen1    = hb_parclen( 1 );
      const char * pcString2   = hb_parc( 2 );
      HB_SIZE      sStrLen2    = hb_parclen( 2 );
      const char * pc1, * pc2;
      HB_SIZE      sIgnore, sCompare, sCompareCnt, sRet = 0;

      if( ISNUM( 4 ) )
         sIgnore = hb_parnl( 4 );
      else
         sIgnore = 0;

      if( ISNUM( 3 ) )
         sCompare = hb_parnl( 3 );
      else
         sCompare = ( sStrLen1 < sStrLen2 ? sStrLen1 : sStrLen2 ) - sIgnore;

      if( ( sCompare == 0 ) || ( sIgnore > sStrLen1 ) || ( sIgnore > sStrLen2 ) )
      {
         int iArgErrorMode = ct_getargerrormode();

         if( iArgErrorMode != CT_ARGERR_IGNORE )
         {
            ct_error( ( USHORT ) iArgErrorMode, EG_ARG, CT_ERROR_POSEQUAL,
                      NULL, "POSEQUAL", 0, EF_CANDEFAULT, 4,
                      hb_paramError( 1 ), hb_paramError( 2 ),
                      hb_paramError( 3 ), hb_paramError( 4 ) );
         }

         hb_retnl( 0 );
         return;
      }

      if( ( sStrLen1 < ( sCompare + sIgnore ) ) || ( sStrLen2 < ( sCompare + sIgnore ) ) )
      {
         int iArgErrorMode = ct_getargerrormode();

         if( iArgErrorMode != CT_ARGERR_IGNORE )
         {
            ct_error( ( USHORT ) iArgErrorMode, EG_ARG, CT_ERROR_POSEQUAL,
                      NULL, "POSEQUAL", 0, EF_CANDEFAULT, 4,
                      hb_paramError( 1 ), hb_paramError( 2 ),
                      hb_paramError( 3 ), hb_paramError( 4 ) );
         }

         hb_retnl( 0 );
         return;
      }

      pc1         = pcString1 + sIgnore;
      pc2         = pcString2 + sIgnore;
      sCompareCnt = 0;

      while( pc1 < pcString1 + sStrLen1 )
      {
         if( *pc1 == *pc2 )
         {
            /* save possible return value */
            if( sCompareCnt == 0 )
               sRet = pc1 - pcString1 + 1;

            sCompareCnt++;
            if( sCompareCnt == sCompare )
            {
               hb_retnl( ( LONG ) sRet );
               return;
            }
         }
         else
         {
            /* reset compare counter */
            sCompareCnt = 0;
         }
         pc1++;
         pc2++;
      }

      hb_retnl( 0 );

   }
   else /* (ISCHAR (1) && ISCHAR (2)) */
   {
      PHB_ITEM pSubst         = NULL;
      int      iArgErrorMode  = ct_getargerrormode();
      if( iArgErrorMode != CT_ARGERR_IGNORE )
      {
         pSubst = ct_error_subst( ( USHORT ) iArgErrorMode, EG_ARG, CT_ERROR_POSEQUAL,
                                  NULL, "POSEQUAL", 0, EF_CANSUBSTITUTE, 4,
                                  hb_paramError( 1 ), hb_paramError( 2 ),
                                  hb_paramError( 3 ), hb_paramError( 4 ) );
      }

      if( pSubst != NULL )
      {
         hb_itemRelease( hb_itemReturnForward( pSubst ) );
      }
      else
      {
         hb_retnl( 0 );
      }
   }
}


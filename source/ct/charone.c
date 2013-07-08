/*
 * $Id: charone.c 9692 2012-09-16 11:25:59Z andijahja $
 */

/*
 * Harbour Project source code:
 *   CT3 string functions
 *     - CHARONE()
 *     - WORDONE()
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

/* defines */
#define DO_CHARONE_CHARONE 0
#define DO_CHARONE_WORDONE 1

/* helper function for the *one functions */
static void do_charone( int iSwitch )
{
   const char *   pcString;
   size_t         sStrLen;
   const char *   pcDeleteSet;
   size_t         sDeleteSetLen;

   /* param check */
   if( ISCHAR( 1 ) )
   {
      if( ISCHAR( 2 ) )
      {
         pcString       = hb_parc( 2 );
         sStrLen        = ( size_t ) hb_parclen( 2 );
         pcDeleteSet    = hb_parc( 1 );
         sDeleteSetLen  = ( size_t ) hb_parclen( 1 );
      }
      else
      {
         pcString       = hb_parc( 1 );
         sStrLen        = ( size_t ) hb_parclen( 1 );
         pcDeleteSet    = NULL;
         sDeleteSetLen  = 0;
      }

      switch( iSwitch )
      {
         case DO_CHARONE_CHARONE:
            if( sStrLen > 1 )
            {
               const char *   pcSub;
               char *         pcRet;
               size_t         sRetStrLen  = 0;
               char           cCurrent    = *pcString;

               pcRet                   = ( char * ) hb_xgrab( sStrLen );

               /* copy first char */
               pcRet[ sRetStrLen++ ]   = cCurrent;

               for( pcSub = pcString + 1; pcSub < pcString + sStrLen; pcSub++ )
               {
                  if( *pcSub != cCurrent )
                  {
                     cCurrent                = *pcSub;
                     pcRet[ sRetStrLen++ ]   = cCurrent;
                  }
                  else if( pcDeleteSet != NULL
                           && ! ct_at_exact_forward( pcDeleteSet, sDeleteSetLen,
                                                     pcSub, 1, NULL ) )
                  {
                     pcRet[ sRetStrLen++ ] = cCurrent;
                  }
               }

               hb_retclen( pcRet, sRetStrLen );
               hb_xfree( pcRet );
            }
            else  /* if (sStrLen > 1) */
            {
               /* algorithm does nothing to 1-char-strings */
               hb_retclen( pcString, sStrLen );
            }
            break;

         case DO_CHARONE_WORDONE:
            if( sStrLen > 3 && sDeleteSetLen >= 2 )
            {
               const char *   pcSub;
               char *         pcRet;
               size_t         sRetStrLen  = 0;
               char           cCurrent1   = pcString[ 0 ];
               char           cCurrent2   = pcString[ 1 ];

               pcRet                   = ( char * ) hb_xgrab( sStrLen );

               /* copy first double char */
               pcRet[ sRetStrLen++ ]   = cCurrent1;
               pcRet[ sRetStrLen++ ]   = cCurrent2;

               for( pcSub = pcString + 2; pcSub < pcString + sStrLen - 1; pcSub += 2 )
               {
                  if( ! ( pcSub[ 0 ] == cCurrent1 && pcSub[ 1 ] == cCurrent2 ) )
                  {
                     cCurrent1               = pcSub[ 0 ];
                     cCurrent2               = pcSub[ 1 ];
                     pcRet[ sRetStrLen++ ]   = cCurrent1;
                     pcRet[ sRetStrLen++ ]   = cCurrent2;
                  }
                  else if( pcDeleteSet != NULL )
                  {
                     const char *   pc       = NULL;
                     const char *   pStart   = pcDeleteSet;
                     size_t         sLen     = sDeleteSetLen;

                     while( sLen >= 2
                            && ( pc = ct_at_exact_forward( pStart, sLen,
                                                           pcSub, 2, NULL ) ) != 0
                            && ( pc - pcDeleteSet ) % 2 == 1 )
                     {
                        pStart   = pc + 1;
                        sLen     = sDeleteSetLen - ( pStart - pcDeleteSet );
                     }

                     if( pc == NULL )
                     {
                        pcRet[ sRetStrLen++ ]   = cCurrent1;
                        pcRet[ sRetStrLen++ ]   = cCurrent2;
                     }
                  }
               }

               /* copy last character if string len is odd */
               if( sStrLen % 2 == 1 )
               {
                  pcRet[ sRetStrLen++ ] = pcString[ sStrLen - 1 ];
               }
               hb_retclen( pcRet, sRetStrLen );
               hb_xfree( pcRet );
            }
            else  /* if (sStrLen > 3) */
            {
               /* algorithm does nothing to 3-char-strings */
               hb_retclen( pcString, sStrLen );
            }
            break;
      }  /* switch (iSwitch) */
   }
   else  /* if (ISCHAR (1)) */
   {
      PHB_ITEM pSubst         = NULL;
      int      iArgErrorMode  = ct_getargerrormode();

      if( iArgErrorMode != CT_ARGERR_IGNORE )
      {
         pSubst = ct_error_subst( ( USHORT ) iArgErrorMode, EG_ARG,
                                  ( iSwitch == DO_CHARONE_CHARONE ? CT_ERROR_CHARONE : CT_ERROR_WORDONE ),
                                  NULL,
                                  ( iSwitch == DO_CHARONE_CHARONE ? "CHARONE" : "WORDONE" ),
                                  0, EF_CANSUBSTITUTE, 2,
                                  hb_paramError( 1 ), hb_paramError( 2 ) );
      }

      if( pSubst != NULL )
      {
         hb_itemRelease( hb_itemReturnForward( pSubst ) );
      }
      else
      {
         hb_retc( "" );
      }
   }
}

HB_FUNC( CHARONE )
{
   do_charone( DO_CHARONE_CHARONE );
}

HB_FUNC( WORDONE )
{
   do_charone( DO_CHARONE_WORDONE );
}

/*
 * $Id: strtran.c 9724 2012-10-02 10:36:35Z andijahja $
 */

/*
 * Harbour Project source code:
 * STRTRAN function
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
#include "hbfast.h"
#include "hbapierr.h"

/* TOFIX: Check for string overflow, Clipper can crash if the resulting
          string is too large. Example:
          StrTran( "...", ".", Replicate( "A", 32000 ) ) [vszakats] */

/* replaces lots of characters in a string */
/* TOFIX: Will not work with a search string of > 64 KB on some platforms */
HB_FUNC( STRTRAN )
{
   PHB_ITEM pText = hb_param( 1, HB_IT_STRING );

   if( pText )
   {
      PHB_ITEM pSeek = hb_param( 2, HB_IT_STRING );

      if( pSeek )
      {
         char *   szText   = pText->item.asString.value;
         HB_SIZE  ulText   = pText->item.asString.length;
         HB_SIZE  ulSeek   = pSeek->item.asString.length;

         if( ulSeek && ulSeek <= ulText )
         {
            char *   szSeek = pSeek->item.asString.value;
            char *   szReplace;
            HB_SIZE  ulStart;

            ulStart = ( hb_param( 4, HB_IT_NUMERIC ) ? hb_parnl( 4 ) : 1 );

            if( ! ulStart )
            {
               /* Clipper seems to work this way */
               hb_retc( "" );
            }
            else if( ulStart > 0 )
            {
               PHB_ITEM pReplace = hb_param( 3, HB_IT_STRING );
               HB_SIZE  ulReplace;
               HB_SIZE  ulCount;
               BOOL     bAll;

               if( pReplace )
               {
                  szReplace   = pReplace->item.asString.value;
                  ulReplace   = pReplace->item.asString.length;
               }
               else
               {
                  szReplace   = ""; /* shouldn't matter that we don't allocate */
                  ulReplace   = 0;
               }

               if( ISNUM( 5 ) )
               {
                  ulCount  = hb_parnl( 5 );
                  bAll     = FALSE;
               }
               else
               {
                  ulCount  = 0;
                  bAll     = TRUE;
               }

               if( bAll || ulCount > 0 )
               {
                  HB_SIZE  ulFound     = 0;
                  LONG     lReplaced   = 0;
                  HB_SIZE  i           = 0;
                  HB_SIZE  ulLength    = ulText;

                  while( i < ulText - ulSeek + 1 )
                  {
                     if( ( bAll || lReplaced < ( LONG ) ulCount ) && memcmp( szText + i, szSeek, ( size_t ) ulSeek ) == 0 )
                     {
                        ulFound++;

                        if( ulFound >= ulStart )
                        {
                           lReplaced++;
                           ulLength = ulLength - ulSeek + ulReplace;
                           i        += ulSeek;
                           continue;
                        }
                     }

                     i++;
                  }

                  if( ulFound )
                  {
                     char *   szResult = ( char * ) hb_xgrab( ulLength + 1 );
                     char *   szPtr    = szResult;

                     ulFound  = 0;
                     i        = 0;

                     while( i < ulText - ulSeek + 1 )
                     {
                        if( lReplaced && memcmp( szText + i, szSeek, ( size_t ) ulSeek ) == 0 )
                        {
                           ulFound++;

                           if( ulFound >= ulStart )
                           {
                              lReplaced--;
                              HB_MEMCPY( szPtr, szReplace, ( size_t ) ulReplace );
                              szPtr += ulReplace;
                              i     += ulSeek;
                              continue;
                           }
                        }

                        *szPtr = szText[ i ];
                        szPtr++;
                        i++;
                     }

                     while( i < ulText )
                     {
                        *szPtr = szText[ i ];
                        szPtr++;
                        i++;
                     }

                     hb_retclenAdopt( szResult, ulLength );
                  }
                  else
                  {
                     hb_retclen( szText, ulText );
                  }
               }
               else
               {
                  hb_retclen( szText, ulText );
               }
            }
            else
            {
               hb_retclen( szText, ulText );
            }
         }
         else
         {
            hb_retclen( szText, ulText );
         }
      }
      else
      {
         hb_errRT_BASE_SubstR( EG_ARG, 1126, NULL, "STRTRAN", 3, hb_paramError( 1 ), hb_paramError( 2 ), hb_paramError( 3 ) ); /* NOTE: Undocumented but existing Clipper Run-time error [vszakats] */
      }
   }
   else
   {
      hb_errRT_BASE_SubstR( EG_ARG, 1126, NULL, "STRTRAN", 3, hb_paramError( 1 ), hb_paramError( 2 ), hb_paramError( 3 ) ); /* NOTE: Undocumented but existing Clipper Run-time error [vszakats] */
   }
}

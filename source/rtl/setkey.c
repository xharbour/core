/*
 * $Id: setkey.c 9759 2012-10-15 14:02:59Z andijahja $
 */

/*
 * Harbour Project source code:
 * SETKEY() and related functions
 *
 * Copyright 1999 A White <awhite@user.rose.com>
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
   Either way you have to clean up the memory on exit. The best way to
   do this is to add a hb_setkeyInit() and hb_setkeyExit() function
   and call them from CONSOLE.C Init/Exit functions.
 */

#include "hbvm.h"
#include "hbapi.h"
#include "hbfast.h"
#include "hbapiitm.h"
#include "hbapigt.h"

typedef struct HB_SETKEY_
{
   SHORT iKeyCode;
   PHB_ITEM pAction;
   PHB_ITEM pIsActive;
   struct HB_SETKEY_ * next;
} HB_SETKEY, * PHB_SETKEY;

static PHB_SETKEY s_sk_list;

void hb_setkeyInit( void )
{
   s_sk_list = NULL;
}

void hb_setkeyExit( void )
{
   while( s_sk_list )
   {
      PHB_SETKEY sk_list_tmp;

      hb_itemRelease( s_sk_list->pAction );

      if( s_sk_list->pIsActive )
      {
         hb_itemRelease( s_sk_list->pIsActive );
      }

      sk_list_tmp = s_sk_list->next;
      hb_xfree( ( void * ) s_sk_list );
      s_sk_list   = sk_list_tmp;
   }

   s_sk_list = NULL;
}

static PHB_SETKEY sk_findkey( SHORT iKeyCode, PHB_SETKEY * sk_list_end )
{
   PHB_SETKEY sk_list_tmp;

   *sk_list_end = NULL;

   for( sk_list_tmp = s_sk_list; sk_list_tmp && sk_list_tmp->iKeyCode != iKeyCode; sk_list_tmp = sk_list_tmp->next )
   {
      *sk_list_end = sk_list_tmp;
   }

   return sk_list_tmp;
}

static void sk_add( BOOL bReturn, SHORT iKeyCode, PHB_ITEM pAction, PHB_ITEM pIsActive )
{
   if( iKeyCode )
   {
      PHB_SETKEY sk_list_tmp, sk_list_end;

      // Verify to allow only codeblock in pIsActive
      pIsActive   = ( pIsActive && HB_IS_BLOCK( pIsActive ) ) ? pIsActive : NULL;

      sk_list_tmp = sk_findkey( iKeyCode, &sk_list_end );
      if( sk_list_tmp == NULL )
      {
         if( pAction )
         {
            sk_list_tmp             = ( PHB_SETKEY ) hb_xgrab( sizeof( HB_SETKEY ) );
            sk_list_tmp->next       = NULL;
            sk_list_tmp->iKeyCode   = iKeyCode;
            sk_list_tmp->pAction    = hb_itemNew( pAction );
            sk_list_tmp->pIsActive  = pIsActive ? hb_itemNew( pIsActive ) : NULL;

            if( sk_list_end == NULL )
            {
               s_sk_list = sk_list_tmp;
            }
            else
            {
               sk_list_end->next = sk_list_tmp;
            }
         }
      }
      else
      {
         /* Return the previous value */

         if( bReturn )
            hb_itemReturn( sk_list_tmp->pAction );

         /* Free the previous values */

         hb_itemRelease( sk_list_tmp->pAction );
         if( sk_list_tmp->pIsActive )
         {
            hb_itemRelease( sk_list_tmp->pIsActive );
         }
         /* Set the new values or free the entry */

         if( pAction )
         {
            sk_list_tmp->pAction    = hb_itemNew( pAction );
            sk_list_tmp->pIsActive  = pIsActive ? hb_itemNew( pIsActive ) : NULL;
         }
         else
         {
            /* if this is true, then the key found is the first key in the list */
            if( sk_list_end == NULL )
            {
               sk_list_tmp = s_sk_list->next;
               hb_xfree( s_sk_list );
               s_sk_list   = sk_list_tmp;
            }
            else
            {
               sk_list_end->next = sk_list_tmp->next;
               hb_xfree( sk_list_tmp );
            }
         }
      }
   }
}

HB_FUNC( SETKEY )
{
   PHB_ITEM pKeyCode = hb_param( 1, HB_IT_NUMERIC );

   if( pKeyCode )
   {
      /* Get a SETKEY value */

      if( hb_pcount() == 1 )
      {
         PHB_SETKEY sk_list_tmp, sk_list_end;

         /* sk_list_end is not used in this context */
         sk_list_tmp = sk_findkey( ( SHORT ) hb_itemGetNI( pKeyCode ), &sk_list_end );

         if( sk_list_tmp )
         {
#if defined( HB_EXTENSION )
            PHB_ITEM pIsActiveResults = sk_list_tmp->pIsActive ? hb_vmEvalBlockV( sk_list_tmp->pIsActive, 1, pKeyCode ) : NULL;

            if( pIsActiveResults && HB_IS_LOGICAL( pIsActiveResults ) && ! hb_itemGetL( pIsActiveResults ) )
            {
               hb_ret();
            }
            else
#endif
            hb_itemReturn( sk_list_tmp->pAction );
         }
      }
      else
      {
         /* Set a SETKEY value */

         sk_add( TRUE, ( SHORT ) hb_itemGetNI( pKeyCode ),
                 hb_param( 2, HB_IT_BLOCK ),
#if defined( HB_EXTENSION )
                 hb_param( 3, HB_IT_BLOCK ) );
#else
                 NULL );
#endif
      }
   }
}

#if defined( HB_EXTENSION )

/* Sets the same block for an array of keycodes */

HB_FUNC( HB_SETKEYARRAY )
{
   PHB_ITEM pKeyCodeArray  = hb_param( 1, HB_IT_ARRAY );
   PHB_ITEM pAction        = hb_param( 2, HB_IT_BLOCK );

   if( pKeyCodeArray && pAction )
   {
      PHB_ITEM pIsActive   = hb_param( 3, HB_IT_BLOCK );
      HB_SIZE  nLen        = hb_arrayLen( pKeyCodeArray );
      ULONG    nPos;

      for( nPos = 1; nPos <= nLen; nPos++ )
         sk_add( FALSE, ( SHORT ) hb_arrayGetNI( pKeyCodeArray, nPos ), pAction, pIsActive );
   }
}

HB_FUNC( HB_SETKEYGET )
{
   PHB_ITEM pKeyCode = hb_param( 1, HB_IT_NUMERIC );

   if( pKeyCode )
   {
      PHB_SETKEY sk_list_tmp, sk_list_end;

      /* sk_list_end is not used in this context */
      sk_list_tmp = sk_findkey( ( SHORT ) hb_itemGetNI( pKeyCode ), &sk_list_end );

      if( sk_list_tmp )
      {
         PHB_ITEM pIsActive = hb_param( 2, HB_IT_ANY );

         hb_itemReturn( sk_list_tmp->pAction );

         if( pIsActive )
         {
            hb_itemCopy( pIsActive, sk_list_tmp->pIsActive );
         }
      }
   }
}

HB_FUNC( HB_SETKEYSAVE )
{
   PHB_ITEM    pKeys, pParam;
   PHB_SETKEY  sk_list_tmp;
   HB_SIZE     itemcount, nitem;

   /* build an multi-dimensional array from existing hot-keys, and return it */

   /* count the number of items in the list */
   for( itemcount = 0, sk_list_tmp = s_sk_list;
        sk_list_tmp;
        itemcount++, sk_list_tmp = sk_list_tmp->next )
      ;

   pKeys = hb_itemArrayNew( itemcount );

   for( nitem = 1, sk_list_tmp = s_sk_list;
        nitem <= itemcount;
        nitem++, sk_list_tmp = sk_list_tmp->next )
   {
      PHB_ITEM pKeyElements, pTmp;

      pKeyElements   = hb_itemArrayNew( 3 );

      pTmp           = hb_itemPutNI( NULL, sk_list_tmp->iKeyCode );
      hb_arraySet( pKeyElements, 1, pTmp );
      hb_itemRelease( pTmp );

      pTmp = hb_itemNew( sk_list_tmp->pAction );
      hb_arraySet( pKeyElements, 2, pTmp );
      hb_itemRelease( pTmp );

      if( sk_list_tmp->pIsActive )
      {
         pTmp = hb_itemNew( sk_list_tmp->pIsActive );
         hb_arraySet( pKeyElements, 3, pTmp );
         hb_itemRelease( pTmp );
      }

      hb_arraySet( pKeys, nitem, pKeyElements );
      hb_itemRelease( pKeyElements );
   }

   hb_itemRelease( hb_itemReturn( pKeys ) );

   pParam = hb_param( 1, HB_IT_ANY );
   if( pParam )
   {
      hb_setkeyExit(); /* destroy the internal list */

      if( HB_IS_ARRAY( pParam ) )
      {
         itemcount = hb_arrayLen( pParam );

         for( nitem = 1; nitem <= itemcount; nitem++ )
         {
            PHB_ITEM itmKeyElements = hb_arrayGetItemPtr( pParam, nitem );

            sk_add( FALSE, ( SHORT ) hb_arrayGetNI( itmKeyElements, 1 ),
                    hb_arrayGetItemPtr( itmKeyElements, 2 ),
                    hb_arrayGetItemPtr( itmKeyElements, 3 ) );
         }
      }
   }
}

HB_FUNC( HB_SETKEYCHECK )
{
   PHB_ITEM pKeyCode    = hb_param( 1, HB_IT_NUMERIC );
   BOOL     bIsKeySet   = FALSE;

   if( pKeyCode )
   {
      PHB_SETKEY sk_list_tmp, sk_list_end;

      /* sk_list_end is not used in this context */
      sk_list_tmp = sk_findkey( ( SHORT ) hb_itemGetNI( pKeyCode ), &sk_list_end );

      if( sk_list_tmp )
      {
         PHB_ITEM pIsActiveResults = sk_list_tmp->pIsActive ? hb_vmEvalBlockV( sk_list_tmp->pIsActive, 1, pKeyCode ) : NULL;

         if( pIsActiveResults == NULL || ! HB_IS_LOGICAL( pIsActiveResults ) || hb_itemGetL( pIsActiveResults ) )
         {
            bIsKeySet = TRUE;

            switch( hb_pcount() )
            {
               case 1:  hb_vmEvalBlockV( sk_list_tmp->pAction, 1, pKeyCode ); break;
               case 2:  hb_vmEvalBlockV( sk_list_tmp->pAction, 2, hb_param( 2, HB_IT_ANY ), pKeyCode ); break;
               case 3:  hb_vmEvalBlockV( sk_list_tmp->pAction, 3, hb_param( 2, HB_IT_ANY ), hb_param( 3, HB_IT_ANY ), pKeyCode ); break;
               default: hb_vmEvalBlockV( sk_list_tmp->pAction, 4, hb_param( 2, HB_IT_ANY ), hb_param( 3, HB_IT_ANY ), hb_param( 4, HB_IT_ANY ), pKeyCode ); break;
            }
         }
      }
   }

   hb_retl( bIsKeySet );
}

#endif

/*
 * $Id: inkeyapi.c 9724 2012-10-02 10:36:35Z andijahja $
 */

/*
 * Harbour Project source code:
 * Inkey GT API
 *
 * Copyright 2007 Przemyslaw Czerpak <druzus / at / priv.onet.pl>
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

#include "hbgtcore.h"

int  hb_inkey( BOOL fWait, double dSeconds, int iEventMask )
{
   PHB_GT pGT;

   HB_TRACE( HB_TR_DEBUG, ( "hb_inkey(%d, %f, %d)", ( int ) fWait, dSeconds, iEventMask ) );

   pGT = hb_gt_Base();
   return pGT ? HB_GTSELF_INKEYGET( pGT, fWait, dSeconds, iEventMask ) : 0;
}

void hb_inkeyPut( int iKey )
{
   PHB_GT pGT;

   HB_TRACE( HB_TR_DEBUG, ( "hb_inkeyPut(%d)", iKey ) );

   pGT = hb_gt_Base();
   if( pGT )
      HB_GTSELF_INKEYPUT( pGT, iKey );
}

int  hb_inkeyLast( int iEventMask )
{
   PHB_GT pGT;

   HB_TRACE( HB_TR_DEBUG, ( "hb_inkeyLast(%d)", iEventMask ) );

   pGT = hb_gt_Base();
   return pGT ? HB_GTSELF_INKEYLAST( pGT, iEventMask ) : 0;
}

int  hb_inkeyNext( int iEventMask )
{
   PHB_GT pGT;

   HB_TRACE( HB_TR_DEBUG, ( "hb_inkeyNext(%d)", iEventMask ) );

   pGT = hb_gt_Base();
   return pGT ? HB_GTSELF_INKEYNEXT( pGT, iEventMask ) : 0;
}

void hb_inkeyPoll( void )
{
   PHB_GT pGT;

   HB_TRACE( HB_TR_DEBUG, ( "hb_inkeyPoll()" ) );

   pGT = hb_gt_Base();
   if( pGT )
      HB_GTSELF_INKEYPOLL( pGT );
}

int  hb_inkeySetLast( int iKey )
{
   PHB_GT pGT;

   HB_TRACE( HB_TR_DEBUG, ( "hb_inkeySetLast(%d)", iKey ) );

   pGT = hb_gt_Base();
   return pGT ? HB_GTSELF_INKEYSETLAST( pGT, iKey ) : 0;
}

void hb_inkeySetText( const char * szText, HB_SIZE ulLen )
{
   PHB_GT pGT;

   HB_TRACE( HB_TR_DEBUG, ( "hb_inkeySetText(%s,%lu)", szText, ulLen ) );

   pGT = hb_gt_Base();
   if( pGT )
      HB_GTSELF_INKEYSETTEXT( pGT, szText, ulLen );
}

void hb_inkeyReset( void )
{
   PHB_GT pGT;

   HB_TRACE( HB_TR_DEBUG, ( "hb_inkeyReset()" ) );

   pGT = hb_gt_Base();
   if( pGT )
      HB_GTSELF_INKEYRESET( pGT );
}

void hb_inkeySetCancelKeys( int iCancelKey, int iCancelKeyEx )
{
   HB_TRACE( HB_TR_DEBUG, ( "hb_inkeySetCancelKeys(%d,%d)", iCancelKey, iCancelKeyEx ) );

/*
   s_InkeyAltC = iCancelKey;
   s_InkeyAltCEx = iCancelKeyEx;
 */
   HB_SYMBOL_UNUSED( iCancelKey );
   HB_SYMBOL_UNUSED( iCancelKeyEx );
}

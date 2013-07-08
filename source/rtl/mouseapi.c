/*
 * $Id: mouseapi.c 9724 2012-10-02 10:36:35Z andijahja $
 */

/*
 * Harbour Project source code:
 * Mouse API
 *
 * Copyright 1999-2001 Viktor Szakats <viktor.szakats@syenar.hu>
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
 * Copyright 1999 Jose Lalin <dezac@corevia.com>
 *    API proposal
 *
 * See doc/license.txt for licensing terms.
 *
 */

#include "hbgtcore.h"

/* NOTE: Mouse initialization is called directly from low level GT driver
 * because it possible that mouse subsystem can depend on the terminal
 * (for example, mouse subsystem cannot be initialized before ncurses
 * driver is initialized).
 */
/* C callable interface */

BOOL hb_mouseIsPresent( void )
{
   PHB_GT pGT;

   HB_TRACE( HB_TR_DEBUG, ( "hb_mouseIsPresent()" ) );

   pGT = hb_gt_Base();
   return pGT ? HB_GTSELF_MOUSEISPRESENT( pGT ) : FALSE;
}

BOOL hb_mouseGetCursor( void )
{
   PHB_GT pGT;

   HB_TRACE( HB_TR_DEBUG, ( "hb_mouseGetCursor()" ) );

   pGT = hb_gt_Base();
   return pGT ? HB_GTSELF_MOUSEGETCURSOR( pGT ) : FALSE;
}

void hb_mouseSetCursor( BOOL fVisible )
{
   PHB_GT pGT;

   HB_TRACE( HB_TR_DEBUG, ( "hb_mouseSetCursor(%d)", ( int ) fVisible ) );

   pGT = hb_gt_Base();
   if( pGT )
      HB_GTSELF_MOUSESETCURSOR( pGT, fVisible );
}

int hb_mouseCol( void )
{
   PHB_GT pGT;

   HB_TRACE( HB_TR_DEBUG, ( "hb_mouseCol()" ) );

   pGT = hb_gt_Base();
   return pGT ? HB_GTSELF_MOUSECOL( pGT ) : 0;
}

int hb_mouseRow( void )
{
   PHB_GT pGT;

   HB_TRACE( HB_TR_DEBUG, ( "hb_mouseRow()" ) );

   pGT = hb_gt_Base();
   return pGT ? HB_GTSELF_MOUSEROW( pGT ) : 0;
}

void hb_mouseGetPos( int * piRow, int * piCol )
{
   PHB_GT pGT;

   HB_TRACE( HB_TR_DEBUG, ( "hb_mouseSetPos(%p, %p)", piRow, piCol ) );

   pGT = hb_gt_Base();
   if( pGT )
      HB_GTSELF_MOUSEGETPOS( pGT, piRow, piCol );
}

void hb_mouseSetPos( int iRow, int iCol )
{
   PHB_GT pGT;

   HB_TRACE( HB_TR_DEBUG, ( "hb_mouseSetPos(%d, %d)", iRow, iCol ) );

   pGT = hb_gt_Base();
   if( pGT )
      HB_GTSELF_MOUSESETPOS( pGT, iRow, iCol );
}

void hb_mouseSetBounds( int iTop, int iLeft, int iBottom, int iRight )
{
   PHB_GT pGT;

   HB_TRACE( HB_TR_DEBUG, ( "hb_mouseSetBounds(%d, %d, %d, %d)", iTop, iLeft, iBottom, iRight ) );

   pGT = hb_gt_Base();
   if( pGT )
      HB_GTSELF_MOUSESETBOUNDS( pGT, iTop, iLeft, iBottom, iRight );
}

void hb_mouseGetBounds( int * piTop, int * piLeft, int * piBottom, int * piRight )
{
   PHB_GT pGT;

   HB_TRACE( HB_TR_DEBUG, ( "hb_mouseSetBounds(%p, %p, %p, %p)", piTop, piLeft, piBottom, piRight ) );

   pGT = hb_gt_Base();
   if( pGT )
      HB_GTSELF_MOUSEGETBOUNDS( pGT, piTop, piLeft, piBottom, piRight );
}

int hb_mouseStorageSize( void )
{
   PHB_GT pGT;

   HB_TRACE( HB_TR_DEBUG, ( "hb_mouseStorageSize()" ) );

   pGT = hb_gt_Base();
   return pGT ? HB_GTSELF_MOUSESTORAGESIZE( pGT ) : 0;
}

void hb_mouseSaveState( BYTE * pBuffer )
{
   PHB_GT pGT;

   HB_TRACE( HB_TR_DEBUG, ( "hb_mouseSaveState(%p)", pBuffer ) );

   pGT = hb_gt_Base();
   if( pGT )
      HB_GTSELF_MOUSESAVESTATE( pGT, pBuffer );
}

void hb_mouseRestoreState( BYTE * pBuffer )
{
   PHB_GT pGT;

   HB_TRACE( HB_TR_DEBUG, ( "hb_mouseRestoreState(%p)", pBuffer ) );

   pGT = hb_gt_Base();
   if( pGT )
      HB_GTSELF_MOUSERESTORESTATE( pGT, pBuffer );
}

int hb_mouseGetDoubleClickSpeed( void )
{
   PHB_GT pGT;

   HB_TRACE( HB_TR_DEBUG, ( "hb_mouseGetDoubleClickSpeed()" ) );

   pGT = hb_gt_Base();
   return pGT ? HB_GTSELF_MOUSEGETDOUBLECLICKSPEED( pGT ) : 0;
}

void hb_mouseSetDoubleClickSpeed( int iSpeed )
{
   PHB_GT pGT;

   HB_TRACE( HB_TR_DEBUG, ( "hb_mouseSetDoubleClickSpeed(%d)", iSpeed ) );

   pGT = hb_gt_Base();
   if( pGT )
      HB_GTSELF_MOUSESETDOUBLECLICKSPEED( pGT, iSpeed );
}

int hb_mouseCountButton( void )
{
   PHB_GT pGT;

   HB_TRACE( HB_TR_DEBUG, ( "hb_mouseCountButton()" ) );

   pGT = hb_gt_Base();
   return pGT ? HB_GTSELF_MOUSECOUNTBUTTON( pGT ) : 0;
}

BOOL hb_mouseButtonState( int iButton )
{
   PHB_GT pGT;

   HB_TRACE( HB_TR_DEBUG, ( "hb_mouseButtonState(%d)", iButton ) );

   pGT = hb_gt_Base();
   return pGT ? HB_GTSELF_MOUSEBUTTONSTATE( pGT, iButton ) : FALSE;
}

BOOL hb_mouseButtonPressed( int iButton, int * piRow, int * piCol )
{
   PHB_GT pGT;

   HB_TRACE( HB_TR_DEBUG, ( "hb_mouseButtonPressed(%d,%p,%p)", iButton, piRow, piCol ) );

   pGT = hb_gt_Base();
   return pGT ? HB_GTSELF_MOUSEBUTTONPRESSED( pGT, iButton, piRow, piCol ) : FALSE;
}

BOOL hb_mouseButtonReleased( int iButton, int * piRow, int * piCol )
{
   PHB_GT pGT;

   HB_TRACE( HB_TR_DEBUG, ( "hb_mouseButtonReleased(%d,%p,%p)", iButton, piRow, piCol ) );

   pGT = hb_gt_Base();
   return pGT ? HB_GTSELF_MOUSEBUTTONRELEASED( pGT, iButton, piRow, piCol ) : FALSE;
}

int hb_mouseReadKey( int iEventMask )
{
   PHB_GT pGT;

   HB_TRACE( HB_TR_DEBUG, ( "hb_mouseReadKey(%d)", iEventMask ) );

   pGT = hb_gt_Base();
   return pGT ? HB_GTSELF_MOUSEREADKEY( pGT, iEventMask ) : 0;
}

/*
 * $Id: bkgtsks.c 9834 2012-11-22 01:28:25Z andijahja $
 */

/*
 * Harbour Project source code:
 * The background tasks - an extension of idle state
 *
 * Copyright 2003-2008 Francesco Saverio Giudice <info@fsgiudice.com>
 * www - http://www.xharbour.org - http://www.harbour-project.org
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
 * See doc/license.txt for licensing terms.
 *
 */

#define HB_OS_WIN_USED

#define HB_THREAD_OPTIMIZE_STACK
#ifndef _HB_API_INTERNAL_
   #define _HB_API_INTERNAL_
#endif
#include "hbapi.h"
#include "hbapiitm.h"
#include "hbfast.h"
#include "hbstack.h"
#include "hbset.h"
#include "hbvm.h"
#include "error.ch"

#if defined( HB_OS_UNIX )
#if defined( HB_OS_DARWIN )
   #include <unistd.h>    /* We need usleep() in Darwin */
#else
   #include <time.h>
#endif
#endif

#ifndef HB_THREAD_SUPPORT

/* Task ID */
static ULONG s_ulBackgroundID                            = 0;

/* list of background tasks
 * A pointer into an array of pointers to items with a codeblock
 */
static PHB_BACKGROUNDTASK *   s_pBackgroundTasks      = NULL;

/* flag to prevent recursive calls of hb_backgroundRun() */
static BOOL                      s_bIamBackground        = FALSE;

/* current task to be executed */
static USHORT                    s_uiBackgroundTask      = 0;

/* number of tasks in the list */
static USHORT                    s_uiBackgroundMaxTask   = 0;
#else

#if defined( HB_VM_ALL )
HB_EXTERN_BEGIN
extern HB_STACK *        _TlsGetValue( void );
HB_EXTERN_END
   #define TlsGetValue( x ) _TlsGetValue()
#endif

#define s_ulBackgroundID      HB_VM_STACK.ulBackgroundID
#define s_pBackgroundTasks    ( HB_VM_STACK.pBackgroundTasks )
#define s_bIamBackground      HB_VM_STACK.bIamBackground
#define s_uiBackgroundTask    HB_VM_STACK.uiBackgroundTask
#define s_uiBackgroundMaxTask HB_VM_STACK.uiBackgroundMaxTask

#endif

/* ------------------------  C  LEVEL ------------------------------ */

ULONG hb_backgroundAddFunc( PHB_ITEM pBlock, int nMillisec, BOOL bActive )
{
   HB_THREAD_STUB
   /* store a copy of passed codeblock */
   PHB_BACKGROUNDTASK pBkgTask = ( PHB_BACKGROUNDTASK ) hb_xgrab( sizeof( HB_BACKGROUNDTASK ) );

   pBkgTask->pTask      = hb_itemNew( pBlock );
   pBkgTask->dSeconds   = hb_seconds();
   pBkgTask->millisec   = nMillisec;
   pBkgTask->bActive    = bActive;

   if( ! s_pBackgroundTasks )
   {
      pBkgTask->ulTaskID   = s_ulBackgroundID = 1;
      s_pBackgroundTasks   = ( PHB_BACKGROUNDTASK * ) hb_xgrab( sizeof( HB_BACKGROUNDTASK ) );
   }
   else
   {
      pBkgTask->ulTaskID = ++s_ulBackgroundID;
      if( pBkgTask->ulTaskID == 0 )  /* the counter reach maximum value */
      {
         /* find unique task ID and set the counter for next */
         int iTask = 0;
         while( iTask < s_uiBackgroundMaxTask )
         {
            if( s_pBackgroundTasks[ iTask ]->ulTaskID == pBkgTask->ulTaskID )
            {
               pBkgTask->ulTaskID++;
               /* This list is unsorted so we have to scan from the begining again */
               iTask = 0;
            }
            else
            {
               iTask++;
               if( s_ulBackgroundID < pBkgTask->ulTaskID )
                  s_ulBackgroundID = pBkgTask->ulTaskID;
            }
         }
      }
      s_pBackgroundTasks = ( PHB_BACKGROUNDTASK * ) hb_xrealloc( s_pBackgroundTasks, sizeof( HB_BACKGROUNDTASK ) * ( s_uiBackgroundMaxTask ) );
   }
   s_pBackgroundTasks[ s_uiBackgroundMaxTask ] = pBkgTask;
   ++s_uiBackgroundMaxTask;

   /* TraceLog( NULL, "BackGroung ID %u\n",  s_ulBackgroundID ); */

   return pBkgTask->ulTaskID;

}

/* RUN all tasks defined in background state but only if SET BACKGROUND TASKS is ON*/
void hb_backgroundRun( void )
{
   HB_THREAD_STUB
   PHB_BACKGROUNDTASK pBkgTask;

   if( ! s_bIamBackground && hb_setGetBackgroundTasks() )
   {
      s_bIamBackground = TRUE;

      if( s_uiBackgroundTask < s_uiBackgroundMaxTask )
      {
         double dCurrSeconds = hb_seconds();
         pBkgTask = ( PHB_BACKGROUNDTASK ) s_pBackgroundTasks[ s_uiBackgroundTask ];

         /* check if hb_seconds() is lower than pBkgTask->dSeconds, if so midnight is reached */
         if( ! ( pBkgTask->dSeconds ) || dCurrSeconds < pBkgTask->dSeconds )
            pBkgTask->dSeconds = dCurrSeconds;

         /* Check if a task can run */
         if( pBkgTask->bActive &&
             ( pBkgTask->millisec == 0 ||
               ( ( ( hb_seconds() - pBkgTask->dSeconds ) * 1000 ) >= pBkgTask->millisec )
             )
             )
         {
            hb_itemRelease( hb_itemDo( pBkgTask->pTask, 0 ) );
            pBkgTask->dSeconds = hb_seconds();
         }
         ++s_uiBackgroundTask;
      }
      else
      {
         if( s_uiBackgroundMaxTask &&
             s_uiBackgroundTask == s_uiBackgroundMaxTask )
            s_uiBackgroundTask = 0;
      }
      s_bIamBackground = FALSE;
   }
}

/* RUN all tasks also if SET BACKGROUND TASKS is OFF */
void hb_backgroundRunForced( void )
{
   BOOL     bOldSet  = hb_setGetBackgroundTasks();
   PHB_ITEM pItem    = hb_itemPutL( NULL, TRUE );

   hb_setSetItem( HB_SET_BACKGROUNDTASKS, pItem );

   hb_backgroundRun();

   hb_itemPutL( pItem, bOldSet );
   hb_setSetItem( HB_SET_BACKGROUNDTASKS, pItem );
   hb_itemRelease( pItem );
}

/* RUN only one tasks, intentionally no check if bacground are active is done */
void hb_backgroundRunSingle( ULONG ulID )
{
   HB_THREAD_STUB
   PHB_BACKGROUNDTASK pBkgTask;

   if( ! s_bIamBackground )
   {
      s_bIamBackground  = TRUE;

      pBkgTask          = hb_backgroundFind( ulID );

      if( pBkgTask )
         hb_itemRelease( hb_itemDo( pBkgTask->pTask, 0 ) );

      s_bIamBackground = FALSE;
   }
}

/* reset background counter to 0 */
void hb_backgroundReset( void )
{
   HB_THREAD_STUB

   if( s_uiBackgroundTask == s_uiBackgroundMaxTask )
      s_uiBackgroundTask = 0;
}

/* close all active background tasks on program exit */
void hb_backgroundShutDown( void )
{
   HB_THREAD_STUB

   if( s_pBackgroundTasks )
   {
      do
      {
         PHB_BACKGROUNDTASK pBkgTask = s_pBackgroundTasks[ --s_uiBackgroundMaxTask ];

         hb_itemRelease( pBkgTask->pTask );
         pBkgTask->pTask   = NULL;
         hb_xfree( pBkgTask );
      }
      while( s_uiBackgroundMaxTask );
      hb_xfree( s_pBackgroundTasks );

      s_pBackgroundTasks = NULL;
   }
}

/* caller have to free return ITEM by hb_itemRelease() if it's not NULL */
PHB_ITEM hb_backgroundDelFunc( ULONG ulID )
{
   HB_THREAD_STUB
   SHORT                iTask;
   PHB_BACKGROUNDTASK   pBkgTask;
   PHB_ITEM             pItem    = NULL;
   BOOL                 bOldSet  = hb_setGetBackgroundTasks();
   PHB_ITEM             pSet     = hb_itemPutL( NULL, FALSE );

   hb_setSetItem( HB_SET_BACKGROUNDTASKS, pSet );

   iTask = 0;
   while( iTask < s_uiBackgroundMaxTask )
   {
      pBkgTask = s_pBackgroundTasks[ iTask ];

      if( ulID == pBkgTask->ulTaskID )
      {
         pItem = pBkgTask->pTask;
         hb_xfree( pBkgTask );

         if( --s_uiBackgroundMaxTask )
         {
            if( iTask != s_uiBackgroundMaxTask )
               memmove( &s_pBackgroundTasks[ iTask ], &s_pBackgroundTasks[ iTask + 1 ],
                        sizeof( HB_BACKGROUNDTASK ) * ( s_uiBackgroundMaxTask - iTask ) );

            s_pBackgroundTasks = ( PHB_BACKGROUNDTASK * ) hb_xrealloc( s_pBackgroundTasks, sizeof( HB_BACKGROUNDTASK ) * s_uiBackgroundMaxTask );
         }
         else
         {
            hb_xfree( s_pBackgroundTasks );
            s_pBackgroundTasks = NULL;
         }
         /* Pitem has now a valid value */
         break;
      }
      ++iTask;
   }

   hb_itemPutL( pSet, bOldSet );
   hb_setSetItem( HB_SET_BACKGROUNDTASKS, pSet );
   hb_itemRelease( pSet );

   return pItem;
}

/* Find a task */
PHB_BACKGROUNDTASK hb_backgroundFind( ULONG ulID )
{
   HB_THREAD_STUB
   SHORT                iTask;
   PHB_BACKGROUNDTASK   pBkgTask;

   iTask = 0;
   while( iTask < s_uiBackgroundMaxTask )
   {
      pBkgTask = s_pBackgroundTasks[ iTask ];

      if( ulID == pBkgTask->ulTaskID )
         return pBkgTask;

      ++iTask;
   }
   return NULL;

}

/* Set task as active */
BOOL hb_backgroundActive( ULONG ulID, BOOL bActive )
{
   PHB_BACKGROUNDTASK   pBkgTask;
   BOOL                 bOldState = FALSE;

   pBkgTask = hb_backgroundFind( ulID );

   if( pBkgTask )
   {
      bOldState         = pBkgTask->bActive;
      pBkgTask->bActive = bActive;
   }
   return bOldState;

}

/* Set task time */
int hb_backgroundTime( ULONG ulID, int nMillisec )
{
   PHB_BACKGROUNDTASK   pBkgTask;
   int                  nOldState = 0;

   pBkgTask = hb_backgroundFind( ulID );

   if( pBkgTask )
   {
      nOldState            = pBkgTask->millisec;
      pBkgTask->millisec   = nMillisec;
   }
   return nOldState;

}

/* ------------------------ PRG LEVEL ------------------------------ */

/* forces to run Background functions */
HB_FUNC( HB_BACKGROUNDRUN )
{
   HB_THREAD_STUB
   if( s_pBackgroundTasks )
   {
      if( hb_parinfo( 1 ) & HB_IT_NUMERIC )
      {
         ULONG ulID = hb_parnl( 1 );   /* TODO: access to pointers from harbour code */
         hb_backgroundRunSingle( ulID );
      }
      else
         hb_backgroundRun();
   }
}

/* forces to run Background functions */
HB_FUNC( HB_BACKGROUNDRUNFORCED )
{
   /* HB_THREAD_STUB */
   if( hb_parinfo( 1 ) & HB_IT_NUMERIC )
   {
      ULONG ulID = hb_parnl( 1 );
      hb_backgroundRunSingle( ulID );
   }
   else
      hb_backgroundRunForced();
}

/* call from user code to reset Background state */
HB_FUNC( HB_BACKGROUNDRESET )
{
   hb_backgroundReset();
}


/* add a new background task and return its handle */
HB_FUNC( HB_BACKGROUNDADD )
{
   HB_THREAD_STUB_API

   PHB_ITEM pBlock      = hb_param( 1, HB_IT_ANY );
   PHB_ITEM pMillisec   = hb_param( 2, HB_IT_NUMERIC );
   PHB_ITEM pActive     = hb_param( 3, HB_IT_LOGICAL );

   if( HB_IS_BLOCK( pBlock ) || HB_IS_ARRAY( pBlock ) )
      hb_retnl( ( LONG ) hb_backgroundAddFunc( pBlock,
                                               ( pMillisec == NULL ? 0 : hb_itemGetNI( pMillisec ) ),
                                               ( pActive == NULL ? TRUE : hb_itemGetL( pActive ) )
                                               ) );
   else
      hb_retnl( -1 );    /* error - a codeblock is required */
}

/* Delete a task with given handle and return a codeblock with this task */
HB_FUNC( HB_BACKGROUNDDEL )
{
   HB_THREAD_STUB
   PHB_ITEM pItem = NULL;

   if( s_pBackgroundTasks && ( hb_parinfo( 1 ) & HB_IT_NUMERIC ) )
   {
      ULONG ulID = hb_parnl( 1 );   /* TODO: access to pointers from harbour code */
      pItem = hb_backgroundDelFunc( ulID );
   }

   if( pItem == NULL )
      hb_ret();    /* return NIL */
   else
   {
      hb_itemReturn( pItem ); /* return a codeblock */
      hb_itemRelease( pItem );
   }
}

/* Set a task as active or not */
HB_FUNC( HB_BACKGROUNDACTIVE )
{
   HB_THREAD_STUB
   BOOL bOldActive = FALSE;

   if( s_pBackgroundTasks && ( hb_parinfo( 1 ) & HB_IT_NUMERIC ) )
   {
      ULONG ulID     = hb_parnl( 1 );  /* TODO: access to pointers from harbour code */
      BOOL  bActive  = TRUE;

      if( hb_parinfo( 2 ) & HB_IT_LOGICAL )
         bActive = hb_parl( 2 );

      bOldActive = hb_backgroundActive( ulID, bActive );
   }

   hb_retl( bOldActive ); /* return old active value */

}

/* Set milliseconds after which a task will be executed */
HB_FUNC( HB_BACKGROUNDTIME )
{
   HB_THREAD_STUB
   int nOldMillisec = 0;

   if( s_pBackgroundTasks && ( hb_parinfo( 1 ) & HB_IT_NUMERIC ) )
   {
      ULONG ulID        = hb_parnl( 1 ); /* TODO: access to pointers from harbour code */
      int   nMillisec   = 1000;

      if( hb_parinfo( 2 ) & HB_IT_NUMERIC )
         nMillisec = hb_parni( 2 );

      nOldMillisec = hb_backgroundTime( ulID, nMillisec );
   }

   hb_retni( nOldMillisec ); /* return old millisecond value */
}

/*
 * $Id: dbgentry.c 9829 2012-11-19 17:15:34Z andijahja $
 */

/*
 * xHarbour Project source code:
 * Debugger entry routine
 *
 * Copyright 2005 Phil Krylov <phil a t newstar.rinet.ru>
 * www - http://www.xharbour.org
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
 * As a special exception, xHarbour license gives permission for
 * additional uses of the text contained in its release of xHarbour.
 *
 * The exception is that, if you link the xHarbour libraries with other
 * files to produce an executable, this does not by itself cause the
 * resulting executable to be covered by the GNU General Public License.
 * Your use of that executable is in no way restricted on account of
 * linking the xHarbour library code into it.
 *
 * This exception does not however invalidate any other reasons why
 * the executable file might be covered by the GNU General Public License.
 *
 * This exception applies only to the code released with this xHarbour
 * explicit exception.  If you add/copy code from other sources,
 * as the General Public License permits, the above exception does
 * not apply to the code that you add in this way.  To avoid misleading
 * anyone as to the status of such modified files, you must delete
 * this exception notice from them.
 *
 * If you write modifications of your own for xHarbour, it is your choice
 * whether to permit this exception to apply to your modifications.
 * If you do not wish that, delete this exception notice.
 *
 */

#include <ctype.h>

#include "hbapidbg.h"
#include "hbapierr.h"
#include "hbapiitm.h"
#include "hbapirdd.h"
#include "hbstack.h"
#include "hbvm.h"

#include "hbdebug.ch"
#include "hbfast.h"
#include "hbmacro.ch"
#include "classes.h"

#if defined( HB_OS_UNIX )
#define FILENAME_EQUAL( s1, s2 )       ( ! strcmp( ( s1 ), ( s2 ) ) )
#else
#define FILENAME_EQUAL( s1, s2 )       ( ! hb_stricmp( ( s1 ), ( s2 ) ) )
#endif

#define ALLOC  hb_xgrab
#define FREE   hb_xfree
#define STRDUP hb_strdup
#define STRNDUP( dest, source, len )   ( dest = hb_strndup( ( source ), ( len ) ) )

#define ARRAY_ADD( type, array, length ) \
   ( ( ++length == 1 ) ? ( array = ( type * ) hb_xgrab( sizeof( type ) ) ) : \
     ( ( array = ( type * ) hb_xrealloc( array, sizeof( type ) * length ) ) + \
       length - 1 ) )

#define ARRAY_DEL( type, array, length, index ) \
   do { \
      if( ! --length ) \
         hb_xfree( array ); \
      else if( index < length ) \
         memmove( array + index, array + index + 1, sizeof( type ) * ( length - index ) ); \
   } while( 0 )

typedef struct
{
   char *             szModule;
   int                nLine;
   char *             szFunction;
} HB_BREAKPOINT;

typedef struct
{
   int                nIndex;
   PHB_ITEM           xValue;
} HB_TRACEPOINT;

typedef struct
{
   char *             szName;
   char               cType;
   int                nFrame;
   int                nIndex;
} HB_VARINFO;

typedef struct
{
   char *             szExpr;
   PHB_ITEM           pBlock;
   int                nVars;
   char **            aVars;
   HB_VARINFO *       aScopes;
} HB_WATCHPOINT;

typedef struct
{
   char *             szModule;
   char *             szFunction;
   int                nLine;
   int                nProcLevel;
   int                nLocals;
   HB_VARINFO *       aLocals;
   int                nStatics;
   HB_VARINFO *       aStatics;
} HB_CALLSTACKINFO;

typedef struct
{
   char *             szModule;
   int                nStatics;
   HB_VARINFO *       aStatics;
   int                nGlobals;
   HB_VARINFO *       aGlobals;
   int                nExternGlobals;
   HB_VARINFO *       aExternGlobals;
} HB_MODULEINFO;

typedef struct
{
   BOOL               bQuit;
   BOOL               bGo;
   BOOL               bInside;
   int                nBreakPoints;
   HB_BREAKPOINT *    aBreak;
   int                nTracePoints;
   HB_TRACEPOINT *    aTrace;
   int                nWatchPoints;
   HB_WATCHPOINT *    aWatch;
   BOOL               bTraceOver;
   int                nTraceLevel;
   BOOL               bNextRoutine;
   BOOL               bCodeBlock;
   BOOL               bToCursor;
   int                nToCursorLine;
   char *             szToCursorModule;
   int                nProcLevel;
   int                nCallStackLen;
   HB_CALLSTACKINFO * aCallStack;
   int                nModules;
   HB_MODULEINFO *    aModules;
   BOOL               bCBTrace;
   BOOL               ( * pFunInvoke )( void );
   BOOL               bInitGlobals;
   BOOL               bInitStatics;
   BOOL               bInitLines;
   PHB_ITEM           pStopLines;
} HB_DEBUGINFO_;

static HB_DEBUGINFO_    s_Info = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* bCBTrace */ TRUE, 0, 0, 0, 0, 0 };
static HB_DEBUGINFO_ *  s_pInfo = &s_Info;

static PHB_ITEM hb_dbgActivateBreakArray( HB_DEBUGINFO_ * info );
static PHB_ITEM hb_dbgActivateModuleArray( HB_DEBUGINFO_ * info );
static PHB_ITEM hb_dbgActivateVarArray( int nVars, HB_VARINFO * aVars );
static void     hb_dbgAddLocal( HB_DEBUGINFO_ * info, char * szName, int nIndex, int nFrame );
static void     hb_dbgAddModule( HB_DEBUGINFO_ * info, char * szName );
static void     hb_dbgAddStack( HB_DEBUGINFO_ * info, char * szName, int nProcLevel );
static void     hb_dbgAddStatic( HB_DEBUGINFO_ * info, char * szName, int nIndex, int nFrame );
static void     hb_dbgAddStopLines( HB_DEBUGINFO_ * info, PHB_ITEM pItem );
static void     hb_dbgAddVar( int * nVars, HB_VARINFO ** aVars, char * szName, char cType, int nIndex, int nFrame );
static void     hb_dbgEndProc( HB_DEBUGINFO_ * info );
static PHB_ITEM hb_dbgEval( HB_DEBUGINFO_ * info, HB_WATCHPOINT * watch );
static PHB_ITEM hb_dbgEvalMacro( char * szExpr, PHB_ITEM pItem );
static PHB_ITEM hb_dbgEvalMakeBlock( HB_WATCHPOINT * watch );
static PHB_ITEM hb_dbgEvalResolve( HB_DEBUGINFO_ * info, HB_WATCHPOINT * watch );
static BOOL     hb_dbgIsAltD( void );
static BOOL     hb_dbgIsBreakPoint( HB_DEBUGINFO_ * info, char * szModule, int nLine );
static BOOL     hb_dbgEqual( PHB_ITEM pItem1, PHB_ITEM pItem2 );
static void     hb_dbgQuit( HB_DEBUGINFO_ * info );
static PHB_ITEM hb_dbgVarGet( HB_VARINFO * scope );
static void     hb_dbgVarSet( HB_VARINFO * scope, PHB_ITEM xNewValue );

static void hb_dbgActivate( HB_DEBUGINFO_ * info )
{
   PHB_DYNS pDynSym = hb_dynsymFind( "__DBGENTRY" );

   if( pDynSym && hb_dynsymIsFunction( pDynSym ) )
   {
      int      i;
      PHB_ITEM aCallStack = hb_itemArrayNew( info->nCallStackLen );
      PHB_ITEM aModules;
      PHB_ITEM aBreak;

      for( i = 0; i < info->nCallStackLen; i++ )
      {
         HB_CALLSTACKINFO *   pEntry   = &info->aCallStack[ i ];
         PHB_ITEM             aEntry   = hb_itemArrayNew( 6 );
         PHB_ITEM             pItem;

         hb_arraySetC( aEntry, 1, pEntry->szModule );
         hb_arraySetC( aEntry, 2, pEntry->szFunction );
         hb_arraySetNL( aEntry, 3, pEntry->nLine );
         hb_arraySetNL( aEntry, 4, pEntry->nProcLevel );

         pItem = hb_dbgActivateVarArray( pEntry->nLocals, pEntry->aLocals );
         hb_arraySet( aEntry, 5, pItem );
         hb_itemRelease( pItem );

         pItem = hb_dbgActivateVarArray( pEntry->nStatics, pEntry->aStatics );
         hb_arraySet( aEntry, 6, pItem );
         hb_itemRelease( pItem );

         hb_arraySet( aCallStack, info->nCallStackLen - i, aEntry );
         hb_itemRelease( aEntry );
      }

      aModules = hb_dbgActivateModuleArray( info );
      aBreak   = hb_dbgActivateBreakArray( info );

      hb_vmPushDynSym( pDynSym );
      hb_vmPushNil();
      hb_vmPushLong( HB_DBG_ACTIVATE );
      hb_vmPushPointer( info );
      hb_vmPushLong( info->nProcLevel );
      hb_vmPush( aCallStack );
      hb_vmPush( aModules );
      hb_vmPush( aBreak );

      hb_itemRelease( aCallStack );
      hb_itemRelease( aModules );
      hb_itemRelease( aBreak );

      hb_vmDo( 6 );
   }
}

static PHB_ITEM hb_dbgActivateBreakArray( HB_DEBUGINFO_ * info )
{
   int      i;
   PHB_ITEM pArray = hb_itemArrayNew( info->nBreakPoints );

   for( i = 0; i < info->nBreakPoints; i++ )
   {
      PHB_ITEM pBreak = hb_itemArrayNew( 3 );

      if( ! info->aBreak[ i ].szFunction )
      {
         hb_arraySetNI( pBreak, 1, info->aBreak[ i ].nLine );
         hb_arraySetC( pBreak, 2, info->aBreak[ i ].szModule );
      }
      else
         hb_arraySetC( pBreak, 3, info->aBreak[ i ].szFunction );

      hb_arraySet( pArray, i + 1, pBreak );
      hb_itemRelease( pBreak );
   }
   return pArray;
}

static PHB_ITEM hb_dbgActivateModuleArray( HB_DEBUGINFO_ * info )
{
   int      i;
   PHB_ITEM pArray = hb_itemArrayNew( info->nModules );

   for( i = 0; i < info->nModules; i++ )
   {
      PHB_ITEM pModule = hb_itemArrayNew( 4 );
      PHB_ITEM item;

      hb_arraySetC( pModule, 1, info->aModules[ i ].szModule );

      item = hb_dbgActivateVarArray( info->aModules[ i ].nStatics,
                                     info->aModules[ i ].aStatics );
      hb_arraySet( pModule, 2, item );
      hb_itemRelease( item );

      item = hb_dbgActivateVarArray( info->aModules[ i ].nGlobals,
                                     info->aModules[ i ].aGlobals );
      hb_arraySet( pModule, 3, item );
      hb_itemRelease( item );

      item = hb_dbgActivateVarArray( info->aModules[ i ].nExternGlobals,
                                     info->aModules[ i ].aExternGlobals );
      hb_arraySet( pModule, 4, item );
      hb_itemRelease( item );

      hb_arraySet( pArray, i + 1, pModule );
      hb_itemRelease( pModule );
   }
   return pArray;
}

static PHB_ITEM hb_dbgActivateVarArray( int nVars, HB_VARINFO * aVars )
{
   int      i;
   PHB_ITEM pArray = hb_itemArrayNew( nVars );

   for( i = 0; i < nVars; i++ )
   {
      PHB_ITEM aVar = hb_itemArrayNew( 4 );

      hb_arraySetC( aVar, 1, aVars[ i ].szName );
      hb_arraySetNL( aVar, 2, aVars[ i ].nIndex );
      hb_arraySetCL( aVar, 3, &aVars[ i ].cType, 1 );
      hb_arraySetNL( aVar, 4, aVars[ i ].nFrame );

      hb_arraySet( pArray, i + 1, aVar );
      hb_itemRelease( aVar );
   }
   return pArray;
}

void hb_dbgEntry( int nMode, int nLine, char * szName, int nIndex, int nFrame )
{
   int               i;
   HB_COUNTER        nProcLevel;
   char              szProcName[ HB_SYMBOL_NAME_LEN + HB_SYMBOL_NAME_LEN + 5 ];
   HB_DEBUGINFO_ *   info = s_pInfo;

   if( ( info->bInside || info->bQuit ) && nMode != HB_DBG_VMQUIT )
      return;

   switch( nMode )
   {
      case HB_DBG_MODULENAME:
         HB_TRACE( HB_TR_DEBUG, ( "MODULENAME %s", szName ) );

         if( szName[ strlen( szName ) - 1 ] == ':' )
            return;

         hb_procinfo( 0, szProcName, NULL, NULL );

         if( ! strncmp( szProcName, "(_INITSTATICS", 13 ) )
            info->bInitStatics = TRUE;
         else if( ! strncmp( szProcName, "(_INITGLOBALS", 13 ) )
            info->bInitGlobals = TRUE;
         else if( ! strncmp( szProcName, "(_INITLINES", 11 ) )
            info->bInitLines = TRUE;

         if( info->bInitStatics || info->bInitGlobals )
            hb_dbgAddModule( info, szName );
         else if( ! strncmp( szProcName, "(b)", 3 ) )
            info->bCodeBlock = TRUE;
         else if( info->bNextRoutine )
            info->bNextRoutine = FALSE;

         hb_dbgAddStack( info, szName, hb_dbg_ProcLevel() );
         for( i = 0; i < info->nBreakPoints; i++ )
         {
            if( info->aBreak[ i ].szFunction
                && ! strcmp( info->aBreak[ i ].szFunction, szProcName ) )
            {
               hb_dbg_InvokeDebug( TRUE );
               break;
            }
         }
         return;

      case HB_DBG_LOCALNAME:
         HB_TRACE( HB_TR_DEBUG, ( "LOCALNAME %s index %d", szName, nIndex ) );

         hb_dbgAddLocal( info, szName, nIndex, hb_dbg_ProcLevel() );
         return;

      case HB_DBG_STATICNAME:
         HB_TRACE( HB_TR_DEBUG, ( "STATICNAME %s index %d frame %d", szName, nIndex, nFrame ) );

         hb_dbgAddStatic( info, szName, nIndex, nFrame );
         return;

      case HB_DBG_SHOWLINE:
      {
         HB_CALLSTACKINFO *   pTop = &info->aCallStack[ info->nCallStackLen - 1 ];
         BOOL                 bOldClsScope;

         HB_TRACE( HB_TR_DEBUG, ( "SHOWLINE %d", nLine ) );

         nProcLevel     = hb_dbg_ProcLevel();

         /* Check if we've hit a tracepoint */
         bOldClsScope   = hb_clsSetScope( FALSE );
         for( i = 0; i < info->nTracePoints; i++ )
         {
            HB_TRACEPOINT *   tp = &info->aTrace[ i ];
            PHB_ITEM          xValue;

            xValue = hb_dbgEval( info, &info->aWatch[ tp->nIndex ] );
            if( ! xValue )
               xValue = hb_itemNew( NULL );

            if( HB_ITEM_TYPE( xValue ) != HB_ITEM_TYPE( tp->xValue ) ||
                ! hb_dbgEqual( xValue, tp->xValue ) )
            {
               hb_itemCopy( tp->xValue, xValue );
               hb_itemRelease( xValue );

               pTop->nLine       = nLine;
               info->nProcLevel  = nProcLevel - ( hb_dbgIsAltD() ? 2 : 0 );
               info->bTraceOver  = FALSE;
               info->bCodeBlock  = FALSE;
               info->bGo         = FALSE;
               if( info->bToCursor )
               {
                  info->bToCursor = FALSE;
                  FREE( info->szToCursorModule );
               }
               info->bNextRoutine = FALSE;

               hb_dbgActivate( info );
               return;
            }
            hb_itemRelease( xValue );
         }
         hb_clsSetScope( bOldClsScope );

         if( hb_dbgIsBreakPoint( info, pTop->szModule, nLine )
             || hb_dbg_InvokeDebug( FALSE )
             || ( info->pFunInvoke && info->pFunInvoke() ) )
         {
            info->bTraceOver = FALSE;
            if( info->bToCursor )
            {
               info->bToCursor = FALSE;
               FREE( info->szToCursorModule );
            }
            info->bNextRoutine   = FALSE;
            info->bGo            = FALSE;
         }

         /* Check if we must skip every level above info->nTraceLevel */
         if( info->bTraceOver )
         {
            if( info->nTraceLevel < info->nCallStackLen )
               return;
            info->bTraceOver = FALSE;
         }

         /* Check if we're skipping to a specific line of source */
         if( info->bToCursor )
         {
            if( nLine == info->nToCursorLine
                && FILENAME_EQUAL( pTop->szModule, info->szToCursorModule ) )
            {
               FREE( info->szToCursorModule );
               info->bToCursor = FALSE;
            }
            else
               return;
         }

         /* Check if'we skipping to the end of current routine */
         if( info->bNextRoutine )
            return;

         if( info->bCodeBlock )
         {
            info->bCodeBlock = FALSE;
            if( ! info->bCBTrace )
               return;
         }

         pTop->nLine = nLine;
         if( ! info->bGo )
         {
            info->nProcLevel = nProcLevel - ( hb_dbgIsAltD() ? 2 : 0 );
            hb_dbgActivate( info );
         }
         return;
      }

      case HB_DBG_ENDPROC:
         if( info->bQuit )
            return;

         HB_TRACE( HB_TR_DEBUG, ( "ENDPROC %d", nLine ) );

         if( info->bInitLines )
         {
            hb_dbgAddStopLines( info, hb_stackReturnItem() );
         }
         info->bCodeBlock     = FALSE;
         info->bInitStatics   = FALSE;
         info->bInitGlobals   = FALSE;
         info->bInitLines     = FALSE;
         hb_dbgEndProc( info );
         return;

      case HB_DBG_VMQUIT:
         if( info )
            hb_dbgQuit( info );

         return;
   }
}

static char * hb_dbgStripModuleName( char * szName )
{
   char * ptr;

   if( ( ptr = strrchr( szName, '/' ) ) != NULL )
      szName = ptr + 1;

   if( ( ptr = strrchr( szName, '\\' ) ) != NULL )
      szName = ptr + 1;

   return szName;
}

void hb_dbgAddBreak( void * handle, const char * cModule, int nLine, const char * szFunction )
{
   HB_DEBUGINFO_ *   info = ( HB_DEBUGINFO_ * ) handle;
   HB_BREAKPOINT *   pBreak;

   pBreak            = ARRAY_ADD( HB_BREAKPOINT, info->aBreak, info->nBreakPoints );
   pBreak->szModule  = STRDUP( cModule );
   pBreak->nLine     = nLine;

   if( szFunction )
      pBreak->szFunction = STRDUP( szFunction );
   else
      pBreak->szFunction = NULL;
}

static void hb_dbgAddLocal( HB_DEBUGINFO_ * info, char * szName, int nIndex, int nFrame )
{
   if( info->bInitGlobals )
   {
      HB_MODULEINFO * module = &info->aModules[ info->nModules - 1 ];

      hb_dbgAddVar( &module->nGlobals, &module->aGlobals, szName, 'G', nIndex, ( int ) hb_dbg_vmVarGCount() );
   }
   else
   {
      HB_CALLSTACKINFO * top = &info->aCallStack[ info->nCallStackLen - 1 ];

      hb_dbgAddVar( &top->nLocals, &top->aLocals, szName, 'L', nIndex, nFrame );
   }
}

static void hb_dbgAddModule( HB_DEBUGINFO_ * info, char * szName )
{
   char *   szModuleName;
   char *   szFuncName;
   int      iLen;

   szName      = hb_dbgStripModuleName( szName );
   szFuncName  = strrchr( szName, ':' );
   iLen        = szFuncName ? ( int ) ( szFuncName - szName ) : ( int ) strlen( szName );
   STRNDUP( szModuleName, szName, iLen );

   if( ! info->nModules || strcmp( info->aModules[ info->nModules - 1 ].szModule, szModuleName ) )
   {
      HB_MODULEINFO * pModule;

      pModule                 = ARRAY_ADD( HB_MODULEINFO, info->aModules, info->nModules );
      pModule->szModule       = szModuleName;
      pModule->nStatics       = 0;
      pModule->nGlobals       = 0;
      pModule->nExternGlobals = 0;
   }
   else
      FREE( szModuleName );
}


static void hb_dbgAddStack( HB_DEBUGINFO_ * info, char * szName, int nProcLevel )
{
   char                 szBuff[ HB_SYMBOL_NAME_LEN + HB_SYMBOL_NAME_LEN + 5 ];
   HB_CALLSTACKINFO *   top;
   char *               szFunction = strrchr( szName, ':' );

   if( szFunction )
      szFunction++;

   top = ARRAY_ADD( HB_CALLSTACKINFO, info->aCallStack, info->nCallStackLen );
   if( info->bCodeBlock )
   {
      HB_MEMCPY( szBuff, "(b)", 3 );
      hb_strncpy( szBuff + 3, szFunction, sizeof( szBuff ) - 4 );
      top->szFunction = STRDUP( szBuff );
   }
   else
   {
      if( szFunction )
         top->szFunction = STRDUP( szFunction );
      else
      {
         /* We're in an (_INITSTATICSnnnnn) pseudo-function */
         hb_procinfo( 0, szBuff, NULL, NULL );
         top->szFunction = STRDUP( szBuff );
      }
   }

   szName = hb_dbgStripModuleName( szName );

   if( szFunction )
      STRNDUP( top->szModule, szName, szFunction - szName - 1 );
   else
      top->szModule = STRDUP( szName );

   top->nProcLevel   = nProcLevel;
   top->nLine        = 0;
   top->nLocals      = 0;
   top->nStatics     = 0;
}


static void hb_dbgAddStatic( HB_DEBUGINFO_ * info, char * szName, int nIndex, int nFrame )
{
   if( info->bInitGlobals )
   {
      HB_MODULEINFO * module = &info->aModules[ info->nModules - 1 ];

      hb_dbgAddVar( &module->nExternGlobals, &module->aExternGlobals, szName,
                    'G', nIndex, ( int ) hb_dbg_vmVarGCount() );
   }
   else if( info->bInitStatics )
   {
      HB_MODULEINFO * module = &info->aModules[ info->nModules - 1 ];

      hb_dbgAddVar( &module->nStatics, &module->aStatics, szName, 'S', nIndex, nFrame );
   }
   else
   {
      HB_CALLSTACKINFO * top = &info->aCallStack[ info->nCallStackLen - 1 ];

      hb_dbgAddVar( &top->nStatics, &top->aStatics, szName, 'S', nIndex, nFrame );
   }
}

static void hb_dbgAddStopLines( HB_DEBUGINFO_ * info, PHB_ITEM pItem )
{
   int   i;
   int   nLinesLen;

   if( ! info->pStopLines )
      info->pStopLines = hb_itemNew( pItem );
   else
   {
      int   j;
      int   nItemLen  = ( int ) hb_itemSize( pItem );

      nLinesLen = ( int ) hb_itemSize( info->pStopLines );

      for( i = 1; i <= nItemLen; i++ )
      {
         PHB_ITEM pEntry   = hb_arrayGetItemPtr( pItem, i );
         char *   szModule = hb_arrayGetCPtr( pEntry, 1 );
         BOOL     bFound   = FALSE;

         szModule = hb_dbgStripModuleName( szModule );
         for( j = 1; j <= nLinesLen; j++ )
         {
            PHB_ITEM pLines = hb_arrayGetItemPtr( info->pStopLines, j );

            if( ! strcmp( hb_arrayGetCPtr( pLines, 1 ), szModule ) )
            {
               /* Merge stopline info */
               int      nOrigMin    = hb_arrayGetNL( pLines, 2 );
               int      nNewMin     = hb_arrayGetNL( pEntry, 2 );
               int      nOrigLen    = ( int ) hb_arrayGetCLen( pLines, 3 );
               int      nNewLen     = ( int ) hb_arrayGetCLen( pEntry, 3 );
               int      nMin        = HB_MIN( nNewMin, nOrigMin );
               int      nMax        = HB_MAX( nNewMin + nNewLen * 8, nOrigMin + nOrigLen * 8 );
               char *   pOrigBuffer = hb_arrayGetCPtr( pLines, 3 );
               char *   pNewBuffer  = hb_arrayGetCPtr( pEntry, 3 );
               int      nLen        = ( nMax + 1 - nMin + 7 ) / 8 + 1;
               int      k;
               char *   pBuffer     = ( char * ) hb_xgrab( nLen );

               hb_xmemset( pBuffer, 0, nLen );

               memmove( &( pBuffer[ ( nNewMin - nMin ) / 8 ] ), pNewBuffer, nNewLen );

               for( k = 0; k < nOrigLen; k++ )
               {
                  pBuffer[ nOrigMin / 8 + k - nMin / 8 ] |= pOrigBuffer[ k ];
               }

               hb_arraySetNL( pLines, 2, nMin );

               if( ! hb_arraySetCPtr( pLines, 3, pBuffer, nLen - 1 ) )
                  hb_xfree( pBuffer );

               bFound = TRUE;
               break;
            }
         }

         if( ! bFound )
            hb_arrayAddForward( info->pStopLines, pEntry );
      }
   }

   nLinesLen = ( int ) hb_itemSize( info->pStopLines );

   for( i = 1; i <= nLinesLen; i++ )
   {
      PHB_ITEM pEntry   = hb_arrayGetItemPtr( info->pStopLines, i );
      char *   szModule = hb_arrayGetCPtr( pEntry, 1 );

      if( szModule )
      {
         char * szName = hb_dbgStripModuleName( szModule );

         if( szName != szModule )
            hb_arraySetC( pEntry, 1, szName );
      }
   }
}

static void hb_dbgAddVar( int * nVars, HB_VARINFO ** aVars, char * szName, char cType, int nIndex, int nFrame )
{
   HB_VARINFO * var;

   var         = ARRAY_ADD( HB_VARINFO, *aVars, *nVars );
   var->szName = szName;
   var->cType  = cType;
   var->nIndex = nIndex;
   var->nFrame = nFrame;
}

void hb_dbgAddWatch( void * handle, const char * szExpr, BOOL bTrace )
{
   HB_DEBUGINFO_ *   info = ( HB_DEBUGINFO_ * ) handle;
   HB_WATCHPOINT *   pWatch;

   pWatch         = ARRAY_ADD( HB_WATCHPOINT, info->aWatch, info->nWatchPoints );
   pWatch->szExpr = STRDUP( szExpr );
   pWatch->pBlock = NULL;
   pWatch->nVars  = 0;

   if( bTrace )
   {
      HB_TRACEPOINT * pTrace = ARRAY_ADD( HB_TRACEPOINT, info->aTrace, info->nTracePoints );

      pTrace->nIndex = info->nWatchPoints - 1;
      pTrace->xValue = hb_dbgEval( info, pWatch );
   }
}

static void hb_dbgClearWatch( HB_WATCHPOINT * pWatch )
{
   FREE( pWatch->szExpr );

   if( pWatch->pBlock )
      hb_itemRelease( pWatch->pBlock );

   if( pWatch->nVars )
   {
      int i;

      for( i = 0; i < pWatch->nVars; i++ )
      {
         FREE( pWatch->aVars[ i ] );
      }
      FREE( pWatch->aVars );
   }
}

void hb_dbgDelBreak( void * handle, int nBreak )
{
   HB_DEBUGINFO_ *   info     = ( HB_DEBUGINFO_ * ) handle;
   HB_BREAKPOINT *   pBreak   = &info->aBreak[ nBreak ];

   FREE( pBreak->szModule );

   if( pBreak->szFunction )
      FREE( pBreak->szFunction );

   ARRAY_DEL( HB_BREAKPOINT, info->aBreak, info->nBreakPoints, nBreak );
}

void hb_dbgDelWatch( void * handle, int nWatch )
{
   HB_DEBUGINFO_ *   info     = ( HB_DEBUGINFO_ * ) handle;
   HB_WATCHPOINT *   pWatch   = &info->aWatch[ nWatch ];
   int               i;

   hb_dbgClearWatch( pWatch );
   ARRAY_DEL( HB_WATCHPOINT, info->aWatch, info->nWatchPoints, nWatch );

   for( i = 0; i < info->nTracePoints; i++ )
   {
      HB_TRACEPOINT * pTrace = &info->aTrace[ i ];

      if( pTrace->nIndex == nWatch )
      {
         if( pTrace->xValue )
            hb_itemRelease( pTrace->xValue );

         ARRAY_DEL( HB_TRACEPOINT, info->aTrace, info->nTracePoints, i );
         i--;
      }
      else if( pTrace->nIndex > nWatch )
         pTrace->nIndex--;
   }
}

static void hb_dbgEndProc( HB_DEBUGINFO_ * info )
{
   HB_CALLSTACKINFO * top;

   if( ! info->nCallStackLen )
      return;

   top = &info->aCallStack[ --info->nCallStackLen ];

   FREE( top->szFunction );
   FREE( top->szModule );

   if( top->nLocals )
      FREE( top->aLocals );

   if( top->nStatics )
      FREE( top->aStatics );

   if( ! info->nCallStackLen )
   {
      FREE( info->aCallStack );
      info->aCallStack = NULL;
   }
}

static BOOL hb_dbgEqual( PHB_ITEM pItem1, PHB_ITEM pItem2 )
{
   if( HB_ITEM_TYPE( pItem1 ) != HB_ITEM_TYPE( pItem2 ) )
      return FALSE;

   if( HB_IS_NIL( pItem1 ) )
      return HB_IS_NIL( pItem2 );

   if( HB_IS_LOGICAL( pItem1 ) )
      return hb_itemGetL( pItem1 ) == hb_itemGetL( pItem2 );

   if( HB_IS_POINTER( pItem1 ) )
      return hb_itemGetPtr( pItem1 ) == hb_itemGetPtr( pItem2 );

   if( HB_IS_STRING( pItem1 ) )
      return ! hb_itemStrCmp( pItem1, pItem2, TRUE );

   if( HB_IS_NUMINT( pItem1 ) )
      return hb_itemGetNInt( pItem1 ) == hb_itemGetNInt( pItem2 );

   if( HB_IS_NUMERIC( pItem1 ) )
      return hb_itemGetND( pItem1 ) == hb_itemGetND( pItem2 );

   if( HB_IS_ARRAY( pItem1 ) )
      return hb_arrayId( pItem1 ) == hb_arrayId( pItem2 );

   if( HB_IS_HASH( pItem1 ) )
      return hb_hashId( pItem1 ) == hb_hashId( pItem2 );

   return FALSE;
}

static PHB_ITEM hb_dbgEval( HB_DEBUGINFO_ * info, HB_WATCHPOINT * watch )
{
   PHB_ITEM xResult = NULL;

   HB_TRACE( HB_TR_DEBUG, ( "expr %s", watch->szExpr ) );

   /* Check if we have a cached pBlock */
   if( ! watch->pBlock )
      watch->pBlock = hb_dbgEvalMakeBlock( watch );

   if( watch->pBlock )
   {
      PHB_ITEM aVars    = hb_dbgEvalResolve( info, watch );
      PHB_ITEM aNewVars = hb_itemArrayNew( watch->nVars );
      int      i;

      hb_arrayCopy( aVars, aNewVars, NULL, NULL, NULL );

      info->bInside  = TRUE;
      xResult        = hb_itemDo( watch->pBlock, 1, aNewVars );
      info->bInside  = FALSE;

      for( i = 0; i < watch->nVars; i++ )
      {
         PHB_ITEM xOldValue   = hb_itemArrayGet( aVars, i + 1 );
         PHB_ITEM xNewValue   = hb_itemArrayGet( aNewVars, i + 1 );

         if( ! hb_dbgEqual( xOldValue, xNewValue ) )
            hb_dbgVarSet( &watch->aScopes[ i ], xNewValue );

         hb_itemRelease( xOldValue );
         hb_itemRelease( xNewValue );
      }

      hb_itemRelease( aVars );
      hb_itemRelease( aNewVars );

      for( i = 0; i < watch->nVars; i++ )
      {
         if( watch->aScopes[ i ].cType == 'M' )
            FREE( watch->aScopes[ i ].szName );
      }

      if( watch->nVars )
         FREE( watch->aScopes );
   }
   return xResult;
}

static PHB_ITEM hb_dbgEvalMacro( char * szExpr, PHB_ITEM pItem )
{
   PHB_ITEM       pStr;
   const char *   type;

   pStr  = hb_itemPutC( NULL, szExpr );
   type  = hb_macroGetType( pStr, HB_SM_RT_MACRO );

   hb_itemRelease( pStr );

   if( ( ! strcmp( type, "U" ) || ! strcmp( type, "UE" ) ) )
      return NULL;

   hb_vmPushString( szExpr, strlen( szExpr ) );
   hb_macroGetValue( hb_stackItemFromTop( -1 ), 0, HB_SM_RT_MACRO );
   hb_itemForwardValue( pItem, hb_stackItemFromTop( -1 ) );
   hb_stackPop();

   return pItem;
}

#define IS_IDENT_START( c )   ( HB_ISALPHA( ( UCHAR ) ( c ) ) || ( c ) == '_' )
#define IS_IDENT_CHAR( c )    ( IS_IDENT_START( ( c ) ) || HB_ISDIGIT( ( UCHAR ) ( c ) ) )

static int hb_dbgEvalSubstituteVar( HB_WATCHPOINT * watch, char * szWord, int nStart, int nLen )
{
   int      j;
   char *   t;

   for( j = 0; j < watch->nVars; j++ )
   {
      if( ! strcmp( szWord, watch->aVars[ j ] ) )
         break;
   }

   if( j == watch->nVars )
      *ARRAY_ADD( char *, watch->aVars, watch->nVars ) = szWord;
   else
      FREE( szWord );

   t                 = ( char * ) ALLOC( strlen( watch->szExpr ) - nLen + 9 + 1 );
   memmove( t, watch->szExpr, nStart );
   memmove( t + nStart, "__dbg[", 6 );
   t[ nStart + 6 ]   = '0' + ( char ) ( ( j + 1 ) / 10 );
   t[ nStart + 7 ]   = '0' + ( char ) ( ( j + 1 ) % 10 );
   t[ nStart + 8 ]   = ']';
   hb_strncpy( t + nStart + 9, watch->szExpr + nStart + nLen, strlen( watch->szExpr ) - nLen - nStart );
   FREE( watch->szExpr );
   watch->szExpr     = t;
   return nStart + 9;
}

static PHB_ITEM hb_dbgEvalMakeBlock( HB_WATCHPOINT * watch )
{
   int      i        = 0;
   PHB_ITEM pBlock;
   BOOL     bAfterId = FALSE;
   char *   s;
   HB_SIZE  buffsize;

   watch->nVars = 0;
   while( watch->szExpr[ i ] )
   {
      char c = watch->szExpr[ i ];

      if( IS_IDENT_START( c ) )
      {
         int      nStart   = i, nLen;
         int      j        = i;
         char *   szWord;

         while( c && IS_IDENT_CHAR( c ) )
         {
            j++;
            c = watch->szExpr[ j ];
         }
         nLen  = j - i;
         STRNDUP( szWord, watch->szExpr + i, nLen );
         i     = j;
         if( c )
         {
            while( watch->szExpr[ i ] && watch->szExpr[ i ] == ' ' )
            {
               i++;
            }
            if( watch->szExpr[ i ] == '(' )
            {
               FREE( szWord );
               continue;
            }
            if( watch->szExpr[ i ] == '-' && watch->szExpr[ i + 1 ] == '>' )
            {
               i += 2;
               while( ( c = watch->szExpr[ i ] ) != '\0' && IS_IDENT_CHAR( c ) )
               {
                  i++;
               }
               FREE( szWord );
               continue;
            }
         }
         hb_strupr( szWord );
         i        = hb_dbgEvalSubstituteVar( watch, szWord, nStart, nLen );
         bAfterId = TRUE;
         continue;
      }
      if( c == '.' )
      {
         if( watch->szExpr[ i + 1 ]
             && strchr( "TtFf", watch->szExpr[ i + 1 ] )
             && watch->szExpr[ i + 2 ] == '.' )
            i += 3;
         else if( ! hb_strnicmp( watch->szExpr + i + 1, "OR.", 3 ) )
            i += 4;
         else if( ! hb_strnicmp( watch->szExpr + i + 1, "AND.", 4 )
                  || ! hb_strnicmp( watch->szExpr + i + 1, "NOT.", 4 ) )
            i += 5;
         else
            i++;

         bAfterId = FALSE;
         continue;
      }
      if( c == ':'
          || ( c == '-' && watch->szExpr[ i + 1 ] == '>'
               && IS_IDENT_START( watch->szExpr[ i + 2 ] ) ) )
      {
         if( c == ':' && watch->szExpr[ i + 1 ] == ':' )
         {
            i        = hb_dbgEvalSubstituteVar( watch, hb_strdup( "SELF" ), i, 1 );
            bAfterId = TRUE;
            continue;
         }

         if( c == '-' )
            i++;

         i++;

         while( watch->szExpr[ i ] && IS_IDENT_CHAR( watch->szExpr[ i ] ) )
         {
            i++;
         }

         bAfterId = TRUE;
         continue;
      }

      if( strchr( " !#$=<>(+-*/%^|,{&", c ) )
      {
         i++;
         bAfterId = FALSE;
         continue;
      }

      if( c == '\'' || c == '\"' )
      {
         i++;
         while( watch->szExpr[ i ] && watch->szExpr[ i ] != c )
         {
            i++;
         }

         if( watch->szExpr[ i ] )
            i++;

         bAfterId = TRUE;
         continue;
      }
      if( c == '[' )
      {
         i++;

         if( bAfterId )
            bAfterId = FALSE;
         else
         {
            while( watch->szExpr[ i ] && watch->szExpr[ i ] != ']' )
            {
               i++;
            }

            if( watch->szExpr[ i ] )
               i++;

            bAfterId = TRUE;
         }
         continue;
      }
      i++;
   }

   buffsize = 8 + strlen( watch->szExpr ) + 1;

   s        = ( char * ) ALLOC( buffsize + 1 );
   hb_strncpy( s, "{|__dbg|", buffsize );
   hb_strncat( s, watch->szExpr, buffsize );
   hb_strncat( s, "}", buffsize );
   pBlock = hb_itemNew( NULL );

   if( ! hb_dbgEvalMacro( s, pBlock ) )
   {
      hb_itemRelease( pBlock );
      pBlock = NULL;
   }
   FREE( s );

   return pBlock;
}

static PHB_ITEM hb_dbgEvalResolve( HB_DEBUGINFO_ * info, HB_WATCHPOINT * watch )
{
   int                  i;
   HB_CALLSTACKINFO *   top      = &info->aCallStack[ info->nCallStackLen - 1 ];
   PHB_ITEM             aVars    = hb_itemArrayNew( watch->nVars );
   HB_VARINFO *         scopes;
   HB_MODULEINFO *      module   = NULL;
   HB_COUNTER           nProcLevel;

   if( ! watch->nVars )
      return aVars;

   scopes      = ( HB_VARINFO * ) ALLOC( watch->nVars * sizeof( HB_VARINFO ) );
   nProcLevel  = hb_dbg_ProcLevel();

   for( i = 0; i < info->nModules; i++ )
   {
      if( ! strcmp( info->aModules[ i ].szModule, top->szModule ) )
      {
         module = &info->aModules[ i ];
         break;
      }
   }

   for( i = 0; i < watch->nVars; i++ )
   {
      char *         name = watch->aVars[ i ];
      HB_VARINFO *   var;
      int            j;
      PHB_ITEM       pItem;

      for( j = 0; j < top->nLocals; j++ )
      {
         var = &top->aLocals[ j ];
         if( ! strcmp( name, var->szName ) )
         {
            scopes[ i ].cType    = 'L';
            scopes[ i ].nFrame   = nProcLevel - var->nFrame;
            scopes[ i ].nIndex   = var->nIndex;
            hb_itemArrayPut( aVars, i + 1, hb_dbgVarGet( &scopes[ i ] ) );
            break;
         }
      }

      if( j < top->nLocals )
         continue;

      for( j = 0; j < top->nStatics; j++ )
      {
         var = &top->aStatics[ j ];
         if( ! strcmp( name, var->szName ) )
         {
            scopes[ i ].cType    = 'S';
            scopes[ i ].nFrame   = var->nFrame;
            scopes[ i ].nIndex   = var->nIndex;
            hb_itemArrayPut( aVars, i + 1, hb_dbgVarGet( &scopes[ i ] ) );
            break;
         }
      }

      if( j < top->nStatics )
         continue;

      if( module )
      {
         for( j = 0; j < module->nStatics; j++ )
         {
            var = &module->aStatics[ j ];
            if( ! strcmp( name, var->szName ) )
            {
               scopes[ i ].cType    = 'S';
               scopes[ i ].nFrame   = var->nFrame;
               scopes[ i ].nIndex   = var->nIndex;
               hb_itemArrayPut( aVars, i + 1, hb_dbgVarGet( &scopes[ i ] ) );
               break;
            }
         }

         if( j < module->nStatics )
            continue;

         for( j = 0; j < module->nGlobals; j++ )
         {
            var = &module->aGlobals[ j ];
            if( ! strcmp( name, var->szName ) )
            {
               scopes[ i ].cType    = 'G';
               scopes[ i ].nFrame   = var->nFrame;
               scopes[ i ].nIndex   = var->nIndex;
               hb_itemArrayPut( aVars, i + 1, hb_dbgVarGet( &scopes[ i ] ) );
               break;
            }
         }

         if( j < module->nGlobals )
            continue;

         for( j = 0; j < module->nExternGlobals; j++ )
         {
            var = &module->aExternGlobals[ j ];
            if( ! strcmp( name, var->szName ) )
            {
               scopes[ i ].cType    = 'G';
               scopes[ i ].nFrame   = var->nFrame;
               scopes[ i ].nIndex   = var->nIndex;
               hb_itemArrayPut( aVars, i + 1, hb_dbgVarGet( &scopes[ i ] ) );
               break;
            }
         }

         if( j < module->nExternGlobals )
            continue;
      }

      scopes[ i ].cType    = 'M';
      scopes[ i ].szName   = STRDUP( name );
      pItem                = hb_dbgVarGet( &scopes[ i ] );

      if( pItem )
         hb_itemArrayPut( aVars, i + 1, pItem );

      if( scopes[ i ].cType == 'F' )
         hb_itemRelease( pItem );
   }
   watch->aScopes = scopes;
   return aVars;
}

PHB_ITEM hb_dbgGetExpressionValue( void * handle, const char * expression )
{
   HB_DEBUGINFO_ *   info = ( HB_DEBUGINFO_ * ) handle;
   PHB_ITEM          result;
   HB_WATCHPOINT     point;

   point.szExpr   = STRDUP( expression );
   point.pBlock   = NULL;
   point.nVars    = 0;

   result         = hb_dbgEval( info, &point );

   hb_dbgClearWatch( &point );

   return result;
}

PHB_ITEM hb_dbgGetSourceFiles( void * handle )
{
   HB_DEBUGINFO_ *   info     = ( HB_DEBUGINFO_ * ) handle;
   int               nModules = ( int ) hb_itemSize( info->pStopLines );
   PHB_ITEM          ret      = hb_itemArrayNew( nModules );
   int               i;

   for( i = 1; i <= nModules; i++ )
   {
      hb_arraySet( ret, i, hb_arrayGetItemPtr( hb_arrayGetItemPtr( info->pStopLines, i ), 1 ) );
   }
   return ret;
}

PHB_ITEM hb_dbgGetWatchValue( void * handle, int nWatch )
{
   HB_DEBUGINFO_ * info = ( HB_DEBUGINFO_ * ) handle;

   return hb_dbgEval( info, &( info->aWatch[ nWatch ] ) );
}

static BOOL hb_dbgIsAltD( void )
{
   char szName[ HB_SYMBOL_NAME_LEN + HB_SYMBOL_NAME_LEN + 5 ];

   hb_procinfo( 1, szName, NULL, NULL );
   return ! strcmp( szName, "ALTD" );
}

static BOOL hb_dbgIsBreakPoint( HB_DEBUGINFO_ * info, char * szModule, int nLine )
{
   int i;

   for( i = 0; i < info->nBreakPoints; i++ )
   {
      HB_BREAKPOINT * point = &info->aBreak[ i ];

      if( point->nLine == nLine
          && FILENAME_EQUAL( szModule, point->szModule ) )
         return TRUE;
   }
   return FALSE;
}

BOOL hb_dbgIsValidStopLine( void * handle, const char * szModule, int nLine )
{
   HB_DEBUGINFO_ *   info     = ( HB_DEBUGINFO_ * ) handle;
   int               nModules = ( int ) hb_itemSize( info->pStopLines );
   int               i;

   for( i = 1; i <= nModules; i++ )
   {
      PHB_ITEM pEntry = hb_arrayGetItemPtr( info->pStopLines, i );

      if( FILENAME_EQUAL( hb_arrayGetCPtr( pEntry, 1 ), szModule ) )
      {
         int   nMin  = hb_arrayGetNL( pEntry, 2 );
         int   nOfs  = nLine - nMin;

         if( nLine < nMin || ( ULONG ) ( nOfs / 8 ) > hb_arrayGetCLen( pEntry, 3 ) )
            return FALSE;

         return ( hb_arrayGetCPtr( pEntry, 3 )[ nOfs / 8 ] & ( 1 << ( nOfs % 8 ) ) ) != 0;
      }
   }
   return FALSE;
}

static void hb_dbgQuit( HB_DEBUGINFO_ * info )
{
   while( info->nWatchPoints )
   {
      hb_dbgDelWatch( info, info->nWatchPoints - 1 );
   }

   while( info->nBreakPoints )
   {
      hb_dbgDelBreak( info, info->nBreakPoints - 1 );
   }

   while( info->nCallStackLen )
   {
      hb_dbgEndProc( info );
   }

   if( info->pStopLines )
      hb_itemRelease( info->pStopLines );

   while( info->nModules )
   {
      int               nModules = info->nModules - 1;
      HB_MODULEINFO *   module   = &info->aModules[ nModules ];

      if( module->nStatics )
         FREE( module->aStatics );

      if( module->nGlobals )
         FREE( module->aGlobals );

      if( module->szModule )
         FREE( module->szModule );

      ARRAY_DEL( HB_MODULEINFO, info->aModules, info->nModules, nModules );
   }

   if( info->bToCursor )
   {
      info->bToCursor = FALSE;
      FREE( info->szToCursorModule );
   }
}

void hb_dbgSetCBTrace( void * handle, BOOL bCBTrace )
{
   HB_DEBUGINFO_ * info = ( HB_DEBUGINFO_ * ) handle;

   info->bCBTrace = bCBTrace;
}

void hb_dbgSetGo( void * handle )
{
   HB_DEBUGINFO_ * info = ( HB_DEBUGINFO_ * ) handle;

   info->bGo = TRUE;
}

void hb_dbgSetInvoke( void * handle, BOOL ( * pFunInvoke )( void ) )
{
   HB_DEBUGINFO_ * info = ( HB_DEBUGINFO_ * ) handle;

   info->pFunInvoke = pFunInvoke;
}

void hb_dbgSetNextRoutine( void * handle )
{
   HB_DEBUGINFO_ * info = ( HB_DEBUGINFO_ * ) handle;

   info->bNextRoutine = TRUE;
}

void hb_dbgSetQuit( void * handle )
{
   HB_DEBUGINFO_ * info = ( HB_DEBUGINFO_ * ) handle;

   info->bQuit = TRUE;
}

void hb_dbgSetToCursor( void * handle, const char * szModule, int nLine )
{
   HB_DEBUGINFO_ * info = ( HB_DEBUGINFO_ * ) handle;

   info->bToCursor         = TRUE;
   info->szToCursorModule  = STRDUP( szModule );
   info->nToCursorLine     = nLine;
}

void hb_dbgSetTrace( void * handle )
{
   HB_DEBUGINFO_ * info = ( HB_DEBUGINFO_ * ) handle;

   info->bTraceOver  = TRUE;
   info->nTraceLevel = info->nCallStackLen;
}

void hb_dbgSetWatch( void * handle, int nWatch, const char * szExpr, BOOL bTrace )
{
   HB_DEBUGINFO_ *   info     = ( HB_DEBUGINFO_ * ) handle;
   HB_WATCHPOINT *   pWatch   = &info->aWatch[ nWatch ];
   int               i;

   hb_dbgClearWatch( pWatch );
   pWatch->szExpr = STRDUP( szExpr );
   pWatch->pBlock = NULL;

   for( i = 0; i < info->nTracePoints; i++ )
   {
      HB_TRACEPOINT * pTrace = &info->aTrace[ i ];

      if( pTrace->nIndex == nWatch )
      {
         if( pTrace->xValue )
            hb_itemRelease( pTrace->xValue );

         ARRAY_DEL( HB_TRACEPOINT, info->aTrace, info->nTracePoints, i );
         break;
      }
   }

   if( bTrace )
   {
      HB_TRACEPOINT * pTrace = ARRAY_ADD( HB_TRACEPOINT, info->aTrace, info->nTracePoints );

      pTrace->nIndex = nWatch;
      pTrace->xValue = hb_dbgEval( info, pWatch );
   }
}

static PHB_ITEM hb_dbgVarGet( HB_VARINFO * scope )
{
   switch( scope->cType )
   {
      case 'G':
         return hb_dbg_vmVarGGet( scope->nFrame, scope->nIndex );
      case 'L':
         return hb_dbg_vmVarLGet( scope->nFrame, scope->nIndex );
      case 'S':
         return hb_dbg_vmVarSGet( scope->nFrame, scope->nIndex );
      case 'M':
      {
         PHB_DYNS pDyn;

         pDyn = hb_dynsymFind( scope->szName );
         if( pDyn != NULL )
         {
            PHB_ITEM pItem = hb_memvarGetValueBySym( pDyn );
            if( ! pItem )
            {
               pItem = hb_itemNew( NULL );

               if( hb_rddFieldGet( pItem, hb_dynsymSymbol( pDyn ) ) == SUCCESS )
                  scope->cType = 'F';
               else
               {
                  hb_itemRelease( pItem );
                  pItem = NULL;
               }
            }
            return pItem;
         }
      }
   }
   return NULL;
}

static void hb_dbgVarSet( HB_VARINFO * scope, PHB_ITEM xNewValue )
{
   switch( scope->cType )
   {
      case 'G':
      case 'L':
      case 'S':
         hb_itemCopy( hb_dbgVarGet( scope ), xNewValue );
         break;
      case 'M':
      {
         PHB_DYNS pDynSym = hb_dynsymFind( "__MVPUT" );

         if( pDynSym && hb_dynsymIsFunction( pDynSym ) )
         {
            hb_vmPushDynSym( pDynSym );
            hb_vmPushNil();
            hb_vmPushString( scope->szName, strlen( scope->szName ) );
            hb_vmPush( xNewValue );
            hb_vmDo( 2 );
         }
         break;
      }
   }
}

/*
 * .prg functions
 */
HB_FUNC( HB_DBG_SETENTRY )
{
   hb_dbg_SetEntry( hb_dbgEntry );
}

HB_FUNC( HB_DBG_SETGO )
{
   void * ptr = hb_parptr( 1 );

   if( ptr )
      hb_dbgSetGo( ptr );
}

HB_FUNC( HB_DBG_SETTRACE )
{
   void * ptr = hb_parptr( 1 );

   if( ptr )
      hb_dbgSetTrace( ptr );
}

HB_FUNC( HB_DBG_SETCBTRACE )
{
   void * ptr = hb_parptr( 1 );

   if( ptr )
      hb_dbgSetCBTrace( ptr, hb_parl( 2 ) );
}

HB_FUNC( HB_DBG_SETNEXTROUTINE )
{
   void * ptr = hb_parptr( 1 );

   if( ptr )
      hb_dbgSetNextRoutine( ptr );
}

HB_FUNC( HB_DBG_SETQUIT )
{
   void * ptr = hb_parptr( 1 );

   if( ptr )
      hb_dbgSetQuit( ptr );
}

HB_FUNC( HB_DBG_SETTOCURSOR )
{
   void * ptr = hb_parptr( 1 );

   if( ptr )
      hb_dbgSetToCursor( ptr, hb_parc( 2 ), hb_parni( 3 ) );
}

HB_FUNC( HB_DBG_GETEXPRVALUE )
{
   void * ptr = hb_parptr( 1 );

   if( ptr )
   {
      PHB_ITEM pItem;

      if( ISCHAR( 2 ) )
         pItem = hb_dbgGetExpressionValue( hb_parptr( 1 ), hb_parc( 2 ) );
      else
         pItem = hb_dbgGetWatchValue( hb_parptr( 1 ), hb_parni( 2 ) - 1 );

      if( pItem )
      {
         hb_storl( TRUE, 3 );
         hb_itemRelease( hb_itemReturn( pItem ) );
      }
      else
         hb_storl( FALSE, 3 );
   }
}

HB_FUNC( HB_DBG_GETSOURCEFILES )
{
   void * ptr = hb_parptr( 1 );

   if( ptr )
      hb_itemRelease( hb_itemReturn( hb_dbgGetSourceFiles( ptr ) ) );
}

HB_FUNC( HB_DBG_ISVALIDSTOPLINE )
{
   void * ptr = hb_parptr( 1 );

   if( ptr )
      hb_retl( hb_dbgIsValidStopLine( ptr, hb_parc( 2 ), hb_parni( 3 ) ) );
}

HB_FUNC( HB_DBG_ADDBREAK )
{
   void * ptr = hb_parptr( 1 );

   if( ptr )
      hb_dbgAddBreak( ptr, hb_parc( 2 ), hb_parni( 3 ), NULL );
}

HB_FUNC( HB_DBG_DELBREAK )
{
   void * ptr = hb_parptr( 1 );

   if( ptr )
      hb_dbgDelBreak( ptr, hb_parni( 2 ) );
}

HB_FUNC( HB_DBG_ADDWATCH )
{
   void * ptr = hb_parptr( 1 );

   if( ptr )
      hb_dbgAddWatch( ptr, hb_parc( 2 ), hb_parl( 3 ) );
}

HB_FUNC( HB_DBG_DELWATCH )
{
   void * ptr = hb_parptr( 1 );

   if( ptr )
      hb_dbgDelWatch( ptr, hb_parni( 2 ) );
}

HB_FUNC( HB_DBG_SETWATCH )
{
   void * ptr = hb_parptr( 1 );

   if( ptr )
      hb_dbgSetWatch( ptr, hb_parni( 2 ), hb_parc( 3 ), hb_parl( 4 ) );
}

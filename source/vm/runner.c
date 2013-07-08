/*
 * $Id: runner.c 9829 2012-11-19 17:15:34Z andijahja $
 */

/*
 * Harbour Project source code:
 * Harbour Portable Object (.hrb) file runner
 *
 * Copyright 1999 Eddie Runia <eddie@runia.com>
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
 * The following functions are added Feb 01,2002 by
 *       Alexander Kresin <alex@belacy.belgorod.su>
 *
 *  __HRBLOAD()
 *  __HRBDO()
 *  __HRBUNLOAD()
 *  __HRBGETFU()
 *  __HRBDOFU()
 */

#include "hbapi.h"
#include "hbstack.h"
#include "hbapiitm.h"
#include "hbapierr.h"
#include "hbapifs.h"
#include "hbvm.h"
#include "hbpcode.h"
#include "hbset.h"
#include "hb_io.h"
#include "classes.h"
/* TODO: Fill the error codes with valid ones (instead of 9999) */

typedef struct
{
   char * szName;                               /* Name of the function     */
   PHB_PCODEFUNC pCodeFunc;                     /* Dynamic function info    */
   BYTE * pCode;                                /* P-code                   */
} HB_DYNF, * PHB_DYNF;

typedef struct
{
   ULONG ulSymbols;                             /* Number of symbols        */
   ULONG ulFuncs;                               /* Number of functions      */
   BOOL fInit;                                  /* should be INIT functions executed */
   BOOL fExit;                                  /* should be EXIT functions executed */
   LONG lSymStart;                              /* Startup Symbol           */
   PHB_SYMB pSymRead;                           /* Symbols read             */
   PHB_DYNF pDynFunc;                           /* Functions read           */
   PSYMBOLS pModuleSymbols;
   HB_DYNS ModuleFakeDyn;
   char * pNamespaces;
} HRB_BODY, * PHRB_BODY;

static const BYTE szHead[] = { 192, 'H', 'R', 'B' };

#define SYM_NOLINK      0              /* symbol does not have to be linked */
#define SYM_FUNC        1              /* function defined in this module   */
#define SYM_EXTERN      2              /* function defined in other module  */
#define SYM_NOT_FOUND   0xFFFFFFFFUL   /* Symbol not found.                 */

HB_EXTERN_BEGIN
PHRB_BODY hb_hrbLoad( const char * szHrbBody, ULONG ulBodySize, char * szHrb );
PHRB_BODY hb_hrbLoadFromFile( const char * szHrb );
void hb_hrbDo( PHRB_BODY pHrbBody, int argc, char * argv[] );
void hb_hrbUnLoad( PHRB_BODY pHrbBody );
HB_EXTERN_END

static int hb_hrbReadHead( const char * szBody, ULONG ulBodySize, ULONG * pulBodyOffset )
{
   const char * pVersion;

   HB_TRACE( HB_TR_DEBUG, ( "hb_hrbReadHead(%p,%lu,%p)", szBody, ulBodySize, pulBodyOffset ) );

   if( ulBodySize < 6 || memcmp( szHead, szBody, 4 ) )
      return 0;

   pVersion       = szBody + 4;
   *pulBodyOffset += 6;

   return HB_PCODE_MKSHORT( pVersion );
}

static BOOL hb_hrbReadValue( const char * szBody, ULONG ulBodySize, ULONG * pulBodyOffset, ULONG * pulValue )
{
   HB_TRACE( HB_TR_DEBUG, ( "hb_hrbReadValue(%p,%lu,%p,%p)", szBody, ulBodySize, pulBodyOffset, pulValue ) );

   if( *pulBodyOffset + 4 < ulBodySize )
   {
      *pulValue      = HB_PCODE_MKLONG( szBody + *pulBodyOffset );
      *pulBodyOffset += 4;

      if( *pulValue <= 0x00FFFFFFUL )
         return TRUE;
   }

   return FALSE;
}

/* ReadId
   Read the next (zero terminated) identifier */
static char * hb_hrbReadId( const char * szBody, ULONG ulBodySize, ULONG * ulBodyOffset )
{
   const char * szIdx;

   HB_TRACE( HB_TR_DEBUG, ( "hb_hrbReadId(%p,%lu,%p)", szBody, ulBodySize, ulBodyOffset ) );

   szIdx = &szBody[ *ulBodyOffset ];

   do
   {
      if( *ulBodyOffset > ulBodySize )
      {
         hb_errRT_BASE( EG_CORRUPTION, 9999, NULL, "__HRBLOAD()->hb_hrbReadId", 0 );
         szIdx = "";
         break;
      }
   }
   while( szBody[ ( *ulBodyOffset )++ ] );

   return hb_strdup( szIdx );
}

static ULONG hb_hrbFindSymbol( const char * szName, PHB_DYNF pDynFunc, ULONG ulLoaded )
{
   ULONG ulRet;

   HB_TRACE( HB_TR_DEBUG, ( "hb_hrbFindSymbol(%s, %p, %lu)", szName, pDynFunc, ulLoaded ) );

   for( ulRet = 0; ulRet < ulLoaded; ++ulRet )
   {
      if( ! strcmp( szName, pDynFunc[ ulRet ].szName ) )
         return ulRet;
   }

   return SYM_NOT_FOUND;
}

static char * hb_hrbReadNamespaces( const char * szBody, ULONG ulBodySize, ULONG * ulBodyOffset )
{
   char * pNamespaces;

   if( szBody[ ( *ulBodyOffset ) ] == '\0' )
   {
      pNamespaces = NULL;
      ( *ulBodyOffset )++;
   }
   else
   {
      ULONG ulStart = *ulBodyOffset, ulLen;

      do
      {
         do
         {
            if( *ulBodyOffset > ulBodySize )
            {
               hb_errRT_BASE( EG_CORRUPTION, 9999, NULL, "__HRBLOAD()->hb_hrbReadNamespaces()", 0 );
               break;
            }
         }
         while( szBody[ ( *ulBodyOffset )++ ] );
      }
      while( szBody[ ( *ulBodyOffset )++ ] );

      ulLen       = ( *ulBodyOffset ) - ulStart;
      pNamespaces = ( char * ) hb_xgrab( ulLen );
      hb_xmemcpy( pNamespaces, szBody + ulStart, ulLen );
   }

   return pNamespaces;
}

static void hb_hrbFreeSymbols( PHB_SYMB pSymbols, ULONG ulSymbols )
{
   ULONG ul;

   for( ul = 0; ul < ulSymbols; ul++ )
   {
      if( pSymbols[ ul ].szName )
         hb_xfree( ( void * ) pSymbols[ ul ].szName );
   }
   hb_xfree( pSymbols );
}

static void hb_hrbInitStatic( PHRB_BODY pHrbBody )
{
   if( ! pHrbBody->fInit && ! pHrbBody->fExit )
   {
      ULONG ul;

      pHrbBody->fInit = TRUE;
      /* Initialize static variables first */
      for( ul = 0; ul < pHrbBody->ulSymbols; ul++ )    /* Check _INITSTATICS functions */
      {
         if( ( pHrbBody->pSymRead[ ul ].scope.value & HB_FS_INITEXIT ) == HB_FS_INITEXIT )
         {
            /* call (_INITSTATICS) function. This function assigns
             * literal values to static variables only. There is no need
             * to pass any parameters to this function because they
             * cannot be used to initialize static variable.
             */

            /* changed to call VM execution instead of direct function address call
             * pHrbBody->pSymRead[ ul ].value.pFunPtr();
             * [MLombardo]
             */

            hb_vmPushSymbol( &( pHrbBody->pSymRead[ ul ] ) );
            hb_vmPushNil();
            hb_vmDo( 0 );
         }
      }
   }
}

static void hb_hrbInit( PHRB_BODY pHrbBody, int argc, char * argv[] )
{
   if( pHrbBody->fInit )
   {
      ULONG ul;
      int   i;

      pHrbBody->fInit   = FALSE;
      pHrbBody->fExit   = TRUE;

      for( ul = 0; ul < pHrbBody->ulSymbols; ul++ )    /* Check INIT functions */
      {
         if( ( pHrbBody->pSymRead[ ul ].scope.value & HB_FS_INITEXIT ) == HB_FS_INIT )
         {
            hb_vmPushSymbol( pHrbBody->pSymRead + ul );
            hb_vmPushNil();
            for( i = 0; i < argc; i++ ) /* Push other cmdline params*/
               hb_vmPushString( argv[ i ], strlen( argv[ i ] ) );

            hb_vmDo( ( USHORT ) argc ); /* Run init function        */
         }
      }
   }
}

static void hb_hrbExit( PHRB_BODY pHrbBody )
{
   if( pHrbBody->fExit )
   {
      ULONG ul;

      pHrbBody->fExit   = FALSE;
      pHrbBody->fInit   = TRUE;

      for( ul = 0; ul < pHrbBody->ulSymbols; ul++ )
      {
         if( ( pHrbBody->pSymRead[ ul ].scope.value & HB_FS_INITEXIT ) == HB_FS_EXIT )
         {
            hb_vmPushSymbol( pHrbBody->pSymRead + ul );
            hb_vmPushNil();
            hb_vmDo( 0 );
         }
      }
   }
}

void hb_hrbUnLoad( PHRB_BODY pHrbBody )
{
   ULONG ul;

   hb_hrbExit( pHrbBody );

   if( pHrbBody->pModuleSymbols )
      hb_clsDeactiveClass( pHrbBody->pModuleSymbols );

   if( pHrbBody->pNamespaces )
      hb_xfree( pHrbBody->pNamespaces );

   if( pHrbBody->pModuleSymbols )
      hb_vmFreeSymbols( pHrbBody->pModuleSymbols );

   if( pHrbBody->pDynFunc )
   {
      for( ul = 0; ul < pHrbBody->ulFuncs; ul++ )
      {
         PHB_DYNS pDyn;

         if( pHrbBody->pDynFunc[ ul ].szName &&
             pHrbBody->pDynFunc[ ul ].pCodeFunc )
         {
            pDyn = hb_dynsymFind( pHrbBody->pDynFunc[ ul ].szName );
            if( pDyn && pDyn->pSymbol->value.pCodeFunc ==
                pHrbBody->pDynFunc[ ul ].pCodeFunc )
               pDyn->pSymbol->value.pCodeFunc = NULL;
         }

         if( pHrbBody->pDynFunc[ ul ].pCodeFunc )
            hb_xfree( pHrbBody->pDynFunc[ ul ].pCodeFunc );

         if( pHrbBody->pDynFunc[ ul ].pCode )
            hb_xfree( pHrbBody->pDynFunc[ ul ].pCode );

         if( pHrbBody->pDynFunc[ ul ].szName )
            hb_xfree( pHrbBody->pDynFunc[ ul ].szName );
      }

      hb_xfree( pHrbBody->pDynFunc );
   }

   hb_xfree( pHrbBody );
}

PHRB_BODY hb_hrbLoad( const char * szHrbBody, ULONG ulBodySize, char * szHrb )
{
   PHRB_BODY pHrbBody = NULL;

   if( szHrbBody )
   {
      ULONG    ulBodyOffset = 0;
      ULONG    ulSize;                             /* Size of function */
      ULONG    ul, ulPos;

      PHB_SYMB pSymRead;                           /* Symbols read     */
      PHB_DYNF pDynFunc;                           /* Functions read   */
      PHB_DYNS pDynSym;

      int      iVersion = hb_hrbReadHead( szHrbBody, ulBodySize, &ulBodyOffset );

      if( iVersion == 0 )
      {
         hb_errRT_BASE( EG_CORRUPTION, 9999, NULL, "__HRBLOAD()->hb_hrbLoad()", 0 );
         return NULL;
      }
      if( iVersion != HB_HRB_VER )
      {
         hb_errRT_BASE( EG_CORRUPTION, 9999, NULL, "Incompatible HRB version", 0 );
         return NULL;
      }

      pHrbBody                   = ( PHRB_BODY ) hb_xgrab( sizeof( HRB_BODY ) );

      pHrbBody->fInit            = FALSE;
      pHrbBody->fExit            = FALSE;
      pHrbBody->lSymStart        = -1;
      pHrbBody->ulFuncs          = 0;
      pHrbBody->pSymRead         = NULL;
      pHrbBody->pDynFunc         = NULL;
      pHrbBody->pModuleSymbols   = NULL;
      pHrbBody->pNamespaces      = hb_hrbReadNamespaces( szHrbBody, ( ULONG ) ulBodySize, &ulBodyOffset );;

      if( ! hb_hrbReadValue( szHrbBody, ulBodySize, &ulBodyOffset, &pHrbBody->ulSymbols ) ||
          pHrbBody->ulSymbols == 0 )
      {
         hb_hrbUnLoad( pHrbBody );
         hb_errRT_BASE( EG_CORRUPTION, 9999, NULL, "__HRBLOAD", 0 );
         return NULL;
      }
      memset( &( pHrbBody->ModuleFakeDyn ), 0, sizeof( HB_DYNS ) );

      pSymRead = ( PHB_SYMB ) hb_xgrab( pHrbBody->ulSymbols * sizeof( HB_SYMB ) );

      for( ul = 0; ul < pHrbBody->ulSymbols; ul++ )  /* Read symbols in .hrb */
      {
         ULONG ulValue;

         pSymRead[ ul ].szName = hb_hrbReadId( szHrbBody, ulBodySize, &ulBodyOffset );
         if( pSymRead[ ul ].szName == NULL ||
             ! hb_hrbReadValue( szHrbBody, ulBodySize, &ulBodyOffset, &ulValue ) )
            break;
         pSymRead[ ul ].scope.value       = ( HB_SYMBOLSCOPE ) ulValue;
         pSymRead[ ul ].value.pCodeFunc   = ( PHB_PCODEFUNC ) ( HB_PTRDIFF ) szHrbBody[ ulBodyOffset++ ];

         if( ( pSymRead[ ul ].scope.value & HB_FS_PUBLIC ) == 0 )
            pSymRead[ ul ].pDynSym = &( pHrbBody->ModuleFakeDyn );
         else
            pSymRead[ ul ].pDynSym = NULL;

         if( pHrbBody->lSymStart == -1 &&
             ( pSymRead[ ul ].scope.value & HB_FS_FIRST ) != 0 &&
             ( pSymRead[ ul ].scope.value & HB_FS_INITEXIT ) == 0 )
            pHrbBody->lSymStart = ul;
      }

      if( ul < pHrbBody->ulSymbols ||
          /* Read number of functions */
          ! hb_hrbReadValue( szHrbBody, ulBodySize, &ulBodyOffset, &pHrbBody->ulFuncs ) )
      {
         hb_hrbFreeSymbols( pSymRead, ul );
         hb_hrbUnLoad( pHrbBody );
         hb_errRT_BASE( EG_CORRUPTION, 9999, NULL, "__HRBLOAD", 0 );
         return NULL;
      }

      pHrbBody->pSymRead = pSymRead;

      if( pHrbBody->ulFuncs )
      {
         pDynFunc             = ( PHB_DYNF ) hb_xgrab( pHrbBody->ulFuncs * sizeof( HB_DYNF ) );
         memset( pDynFunc, 0, pHrbBody->ulFuncs * sizeof( HB_DYNF ) );
         pHrbBody->pDynFunc   = pDynFunc;

         for( ul = 0; ul < pHrbBody->ulFuncs; ul++ )
         {
            /* Read name of function */
            pDynFunc[ ul ].szName = hb_hrbReadId( szHrbBody, ulBodySize, &ulBodyOffset );
            if( pDynFunc[ ul ].szName == NULL )
               break;

            /* Read size of function */
            if( ! hb_hrbReadValue( szHrbBody, ulBodySize, &ulBodyOffset, &ulSize ) ||
                ulBodyOffset + ulSize > ulBodySize )
               break;

            /* Copy function body */
            pDynFunc[ ul ].pCode                = ( BYTE * ) hb_xgrab( ulSize );
            HB_MEMCPY( ( char * ) pDynFunc[ ul ].pCode, szHrbBody + ulBodyOffset, ulSize );
            ulBodyOffset                        += ulSize;

            pDynFunc[ ul ].pCodeFunc            = ( PHB_PCODEFUNC ) hb_xgrab( sizeof( HB_PCODEFUNC ) );
            pDynFunc[ ul ].pCodeFunc->pCode     = pDynFunc[ ul ].pCode;
            pDynFunc[ ul ].pCodeFunc->pSymbols  = pSymRead;
         }

         if( ul < pHrbBody->ulFuncs )
         {
            hb_hrbFreeSymbols( pSymRead, pHrbBody->ulSymbols );
            hb_hrbUnLoad( pHrbBody );
            hb_errRT_BASE( EG_CORRUPTION, 9999, NULL, "__HRBLOAD", 0 );
            return NULL;
         }
      }

      /* End of PCODE loading, now linking */

      for( ul = 0; ul < pHrbBody->ulSymbols; ul++ )
      {
#if defined( __XCC__ )
         if( pSymRead[ ul ].value.pCodeFunc == ( PHB_PCODEFUNC ) SYM_FUNC )
#else
         if( pSymRead[ ul ].value.pCodeFunc == ( PHB_PCODEFUNC ) ( HB_LONG ) SYM_FUNC )
#endif
         {
            ulPos = hb_hrbFindSymbol( pSymRead[ ul ].szName, pHrbBody->pDynFunc, pHrbBody->ulFuncs );

            if( ulPos == SYM_NOT_FOUND )
            {
#if defined( __XCC__ )
               pSymRead[ ul ].value.pCodeFunc = ( PHB_PCODEFUNC ) SYM_EXTERN;
#else
               pSymRead[ ul ].value.pCodeFunc = ( PHB_PCODEFUNC ) ( HB_LONG ) SYM_EXTERN;
#endif
            }
            else
            {
               pSymRead[ ul ].value.pCodeFunc   = ( PHB_PCODEFUNC ) pHrbBody->pDynFunc[ ulPos ].pCodeFunc;
               pSymRead[ ul ].scope.value       |= HB_FS_PCODEFUNC | HB_FS_LOCAL;
            }
         }

         /* External function */
#if defined( __XCC__ )
         if( pSymRead[ ul ].value.pCodeFunc == ( PHB_PCODEFUNC ) SYM_EXTERN )
#else
         if( pSymRead[ ul ].value.pCodeFunc == ( PHB_PCODEFUNC ) ( HB_LONG ) SYM_EXTERN )
#endif
         {
            pDynSym = hb_dynsymFind( pSymRead[ ul ].szName );

            if( pDynSym )
            {
               pSymRead[ ul ].value.pFunPtr = pDynSym->pSymbol->value.pFunPtr;

               if( pDynSym->pSymbol->scope.value & HB_FS_PCODEFUNC )
                  pSymRead[ ul ].scope.value |= HB_FS_PCODEFUNC;
            }
            else
            {
               char szName[ HB_SYMBOL_NAME_LEN + 1 ];

               hb_strncpy( szName, pSymRead[ ul ].szName, HB_SYMBOL_NAME_LEN );
               hb_hrbFreeSymbols( pSymRead, pHrbBody->ulSymbols );
               hb_hrbUnLoad( pHrbBody );
               hb_errRT_BASE( EG_ARG, 9999, "Unknown or unregistered symbol", szName, 0 );
               return NULL;
            }
         }
      }

      pHrbBody->pModuleSymbols               = hb_vmRegisterSymbols( pHrbBody->pSymRead,
                                                                     ( USHORT ) pHrbBody->ulSymbols, szHrb ? szHrb : ( char * ) "PCODE_HRB.hrb", TRUE, FALSE, NULL );
      pHrbBody->pModuleSymbols->pNamespaces  = pHrbBody->pNamespaces;
      pHrbBody->ModuleFakeDyn.pModuleSymbols = pHrbBody->pModuleSymbols;

      if( pHrbBody->pModuleSymbols->pSymbolTable != pSymRead )
      {
         /*
          * Old unused symbol table has been recycled - free the one
          * we allocated and disactivate static initialization [druzus]
          */
         pHrbBody->pSymRead   = pHrbBody->pModuleSymbols->pSymbolTable;
         hb_hrbFreeSymbols( pSymRead, pHrbBody->ulSymbols );

         pHrbBody->fInit      = TRUE;
      }
      else
      {
         /* mark symbol table as dynamically allocated so HVM will free it on exit */
         pHrbBody->pModuleSymbols->fAllocated = TRUE;

         /* initialize static variables */
         hb_hrbInitStatic( pHrbBody );
      }

      /* TraceLog( NULL, "Runner registered module: %s\n", pHrbBody->pModuleSymbols->szModuleName );
       */
   }

   return pHrbBody;
}

PHRB_BODY hb_hrbLoadFromFile( const char * szHrb )
{
   char        szFileName[ HB_PATH_MAX ];
   PHRB_BODY   pHrbBody = NULL;
   PHB_FNAME   pFileName;
   HB_FHANDLE  hFile;

   /* Create full filename */

   pFileName = hb_fsFNameSplit( szHrb );

   if( hb_setGetDefExtension() && pFileName->szExtension == NULL )
      pFileName->szExtension = ".hrb";

   hb_fsFNameMerge( szFileName, pFileName );
   hb_xfree( pFileName );

   /* Open as binary */

   do
   {
      hFile = hb_fsOpen( szFileName, FO_READ );
   }
   while( hFile == FS_ERROR &&
          hb_errRT_BASE_Ext1( EG_OPEN, 9999, NULL, szFileName, hb_fsError(),
                              EF_CANDEFAULT | EF_CANRETRY,
                              1, hb_paramError( 1 ) ) == E_RETRY );

   if( hFile != FS_ERROR )
   {
      ULONG ulBodySize = hb_fsSeek( hFile, 0, FS_END );

      if( ulBodySize )
      {
         BYTE * pbyBuffer;

         pbyBuffer               = ( BYTE * ) hb_xgrab( ulBodySize + sizeof( char ) + 1 );
         hb_fsSeek( hFile, 0, FS_SET );
         hb_fsReadLarge( hFile, pbyBuffer, ulBodySize );
         pbyBuffer[ ulBodySize ] = '\0';

         pHrbBody                = hb_hrbLoad( ( char * ) pbyBuffer, ( ULONG ) ulBodySize, szFileName );
         hb_xfree( pbyBuffer );
      }
      hb_fsClose( hFile );
   }

   return pHrbBody;
}

void hb_hrbDo( PHRB_BODY pHrbBody, int argc, char * argv[] )
{
   PHB_ITEM pRetVal = NULL;
   int      i;

   hb_hrbInit( pHrbBody, argc, argv );

   /* May not have a startup symbol, if first symbol was an INIT Symbol (was executed already).*/
   if( pHrbBody->lSymStart >= 0 )
   {
      hb_vmPushSymbol( &pHrbBody->pSymRead[ pHrbBody->lSymStart ] );
      hb_vmPushNil();

      for( i = 0; i < ( hb_pcount() - 1 ); i++ )
         hb_vmPush( hb_param( i + 2, HB_IT_ANY ) );  /* Push other cmdline params*/

      hb_vmDo( ( USHORT ) ( hb_pcount() - 1 ) );                    /* Run the thing !!!        */

      pRetVal = hb_itemNew( NULL );
      hb_itemCopy( pRetVal, hb_stackReturnItem() );
   }

   hb_hrbExit( pHrbBody );

   if( pRetVal )
      hb_itemReturnRelease( pRetVal );
}

/*
 * __HRBRUN( <cFile> [, xParam1 [, xParamN ] ] ) -> return value.
 *
 * This program will get the data from the .hrb file and run the p-code
 * contained in it.
 *
 * In due time it should also be able to collect the data from the
 * binary/executable itself
 */
HB_FUNC( __HRBRUN )
{
   ULONG ulLen = ( ULONG ) hb_parclen( 1 );

   if( ulLen > 0 )
   {
      const char *   fileOrBody = hb_parc( 1 );
      PHRB_BODY      pHrbBody;

      if( ulLen > 4 && memcmp( szHead, fileOrBody, 4 ) == 0 )
         pHrbBody = hb_hrbLoad( fileOrBody, ulLen, NULL );
      else
         pHrbBody = hb_hrbLoadFromFile( fileOrBody );

      if( pHrbBody )
      {
         int      argc  = hb_pcount(), i;
         char **  argv  = NULL;

         if( argc > 1 )
         {
            argv = ( char ** ) hb_xgrab( sizeof( char * ) * ( argc - 1 ) );

            for( i = 0; i < argc - 1; i++ )
               argv[ i ] = ( char * ) hb_parcx( i + 2 );
         }

         hb_hrbDo( pHrbBody, argc - 1, argv );

         if( argv )
            hb_xfree( argv );

         hb_hrbUnLoad( pHrbBody );
         hb_retl( TRUE );
      }
      else
         hb_retl( FALSE );
   }
   else
      hb_errRT_BASE( EG_ARG, 9999, NULL, "__HRBRUN", 0 );
}

HB_FUNC( __HRBLOAD )
{
   ULONG ulLen = ( ULONG ) hb_parclen( 1 );

   if( ulLen > 0 )
   {
      const char *   fileOrBody = hb_parc( 1 );
      PHRB_BODY      pHrbBody;

      /* If parameter string */
      if( ulLen > 4 && memcmp( szHead, fileOrBody, 4 ) == 0 )
         pHrbBody = hb_hrbLoad( fileOrBody, ulLen, NULL );
      else
         pHrbBody = hb_hrbLoadFromFile( fileOrBody );

      if( pHrbBody )
      {
         int      argc  = hb_pcount();
         char **  argv  = NULL;
         int      i;

         if( argc > 1 )
         {
            argv = ( char ** ) hb_xgrab( sizeof( char * ) * ( argc - 1 ) );

            for( i = 0; i < argc - 1; i++ )
               argv[ i ] = ( char * ) hb_parcx( i + 2 );
         }

         hb_hrbInit( pHrbBody, argc - 1, argv );

         if( argv )
            hb_xfree( argv );
      }
      hb_retptr( ( void * ) pHrbBody );
   }
   else
      hb_errRT_BASE( EG_ARG, 9998, NULL, "__HRBLOAD", 0 );
}

HB_FUNC( __HRBDO )
{
   PHRB_BODY pHrbBody = ( PHRB_BODY ) hb_parptr( 1 );

   if( pHrbBody )
   {
      int      argc  = hb_pcount();
      char **  argv  = NULL;
      int      i;

      if( argc > 1 )
      {
         argv = ( char ** ) hb_xgrab( sizeof( char * ) * ( argc - 1 ) );

         for( i = 0; i < argc - 1; i++ )
            argv[ i ] = ( char * ) hb_parcx( i + 2 );
      }

      hb_hrbDo( pHrbBody, argc - 1, argv );

      if( argv )
         hb_xfree( argv );
   }
   else
      hb_errRT_BASE( EG_ARG, 9999, NULL, "__HRBDO", 0 );
}

HB_FUNC( __HRBUNLOAD )
{
   PHRB_BODY pHrbBody = ( PHRB_BODY ) hb_parptr( 1 );

   if( pHrbBody )
      hb_hrbUnLoad( pHrbBody );
   else
      hb_errRT_BASE( EG_ARG, 9999, NULL, "__HRBUNLOAD", 0 );
}

HB_FUNC( __HRBGETFU )
{
   PHRB_BODY pHrbBody = ( PHRB_BODY ) hb_parptr( 1 );

   if( pHrbBody && hb_parclen( 2 ) > 0 )
   {
      char *   szName   = hb_strupr( hb_strdup( hb_parc( 2 ) ) );
      ULONG    ulPos    = 0;

      while( ulPos < pHrbBody->ulSymbols )
      {
         if( ! strcmp( szName, pHrbBody->pSymRead[ ulPos ].szName ) )
            break;

         ulPos++;
      }
      hb_xfree( szName );

      if( ulPos < pHrbBody->ulSymbols )
         hb_retptr( ( void * ) ( pHrbBody->pSymRead + ulPos ) );
      else
         hb_retptr( NULL );
   }
   else
      hb_errRT_BASE( EG_ARG, 9999, NULL, "__HRBGETFU", 0 );
}

HB_FUNC( __HRBDOFU )
{
   PHB_SYMB pSym = ( PHB_SYMB ) hb_parptr( 1 );

   if( pSym )
   {
      int   argc = hb_pcount();
      int   i;

      hb_vmPushSymbol( pSym );
      hb_vmPushNil();

      for( i = 2; i <= argc; i++ )  /* Push other  params  */
         hb_vmPush( hb_stackItemFromBase( i ) );

      hb_vmDo( ( USHORT ) ( argc - 1 ) );          /* Run function        */
   }
   else
      hb_errRT_BASE( EG_ARG, 9999, NULL, "__HRBDOFU", 0 );
}

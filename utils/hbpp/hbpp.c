/*
 * $Id: hbpp.c 9845 2012-12-01 03:29:25Z andijahja $
 */

/*
 * Harbour Project source code:
 * Preprocessor standalone main module
 *
 * Copyright 1999 Alexander S.Kresin <alex@belacy.belgorod.su>
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

/* Pacify MSVS2005 and above */
#if defined(_MSC_VER) && (_MSC_VER>=1400)
   #define _CRT_SECURE_NO_WARNINGS
   #define _CRT_SECURE_NO_DEPRECATE
#endif

/*
 * Avoid tracing in preprocessor/compiler.
 */
#if ! defined(HB_TRACE_UTILS)
   #if defined(HB_TRACE_LEVEL)
      #undef HB_TRACE_LEVEL
   #endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "hbppdef.h"
#include "hbcomp.h"
#include "hbset.h"
#include "pragma.h"

extern int hb_pp_ParseDefine( char * );

static void AddSearchPath( char * szPath, HB_PATHNAMES * * pSearchList );
static void OutTable( DEFINES * endDefine, COMMANDS * endCommand, COMMANDS *endTranslate );
static BOOL hb_pp_fopen( char * szFileName );

static char s_szLine[ HB_PP_STR_SIZE ];
static int  s_iWarnings = 0;
static char * hb_buffer;

HB_PATHNAMES * hb_comp_pIncludePath = NULL;
PHB_FNAME      hb_comp_pFileName = NULL;
FILES          hb_comp_files;
int            hb_comp_iLine = 1; /* currently parsed file line number */

/* These are need for the PP #pragma support */
BOOL hb_comp_bPPO = FALSE;                      /* flag indicating, is ppo output needed */
BOOL hb_comp_bStartProc = TRUE;                 /* holds if we need to create the starting procedure */
BOOL hb_comp_bLineNumbers = TRUE;               /* holds if we need pcodes with line numbers */
BOOL hb_comp_bShortCuts = TRUE;                 /* .and. & .or. expressions shortcuts */
int  hb_comp_iWarnings = 0;                     /* enable parse warnings */
BOOL hb_comp_bAutoMemvarAssume = FALSE;         /* holds if undeclared variables are automatically assumed MEMVAR (-a)*/
BOOL hb_comp_bForceMemvars = FALSE;             /* holds if memvars are assumed when accesing undeclared variable (-v)*/
BOOL hb_comp_bDebugInfo = FALSE;                /* holds if generate debugger required info */
int  hb_comp_iExitLevel = HB_EXITLEVEL_DEFAULT; /* holds if there was any warning during the compilation process */
FILE *hb_comp_yyppo = NULL;
FILE *hb_comp_PPTrace = NULL;

int  hb_comp_iLinePRG;
int  hb_comp_iLineINLINE = 0;

int  hb_comp_iHidden = 0;

int main( int argc, char * argv[] )
{
  FILE * handl_o;
  char szFileName[ HB_PATH_MAX ];
  char szPpoName[ HB_PATH_MAX ];
  int iArg = 1;
  BOOL bOutTable = FALSE;
  BOOL bOutNew = FALSE;
  DEFINES *stdef;
  COMMANDS *stcmd, *sttra;

  HB_TRACE(HB_TR_DEBUG, ("main(%d, %p)", argc, argv));

  printf( "xHarbour Preprocessor %d.%d.%d\n",
     HB_VER_MAJOR, HB_VER_MINOR, HB_VER_REVISION );
  printf( "Copyright 1999-2013, http://www.xharbour.org\n" );

  hb_pp_Table();
  stdef = hb_pp_topDefine;
  stcmd = hb_pp_topCommand;
  sttra = hb_pp_topTranslate;

  hb_pp_Init();

  while( iArg < argc )
  {
      if( HB_ISOPTSEP(argv[ iArg ][ 0 ]))
      {
          switch( argv[ iArg ][ 1 ] )
          {
            case 'd':
            case 'D':   /* defines a #define from the command line */
            {
                 char *szDefText = hb_strdup( argv[iArg] + 2 ), *pAssign, *sDefLine;
                 unsigned int i = 0;

                 while( i < strlen( szDefText ) && ! HB_ISOPTSEP( szDefText[ i ] ) )
                    i++;

                 szDefText[ i ] = '\0';
                 if( szDefText )
                 {
                    if( ( pAssign = strchr( szDefText, '=' ) ) == NULL )
                    {
                       hb_pp_AddDefine( szDefText, 0, FALSE );
                    }
                    else
                    {
                       UINT uiLen;
                       
                       szDefText[ pAssign - szDefText ] = '\0';

                       //hb_pp_AddDefine( szDefText,  pAssign + 1, FALSE );
                       uiLen = strlen( szDefText ) + 1 + strlen( pAssign + 1 ) + 1;
                       sDefLine = (char*) hb_xgrab( uiLen );
                       hb_snprintf( sDefLine, uiLen, "%s %s", szDefText, pAssign + 1 );
                       hb_pp_ParseDefine( sDefLine );
                       hb_xfree( sDefLine );
                    }
                 }

                 hb_xfree( szDefText );
            }
            break;

            case 'i':
            case 'I':
              AddSearchPath( argv[ iArg ]+2, &hb_comp_pIncludePath );
              break;

            case 'o':
            case 'O':
              bOutTable = TRUE;
              break;

            case 'n':
            case 'N':
              bOutNew = TRUE;
              break;

            case 'w':
            case 'W':
              s_iWarnings = 1;
              if( argv[ iArg ][ 2 ] )
              {  /*there is -w<0,1,2,3> probably */
                  s_iWarnings = argv[ iArg ][ 2 ] - '0';
                  if( s_iWarnings < 0 || s_iWarnings > 3 )
                    printf( "\nInvalid command line option: %s\n", argv[ iArg ] );
              }
              break;

            default:
              printf( "\nInvalid command line option: %s\n", &argv[ iArg ][ 1 ] );
              break;
          }
      }
      else
      {
          hb_comp_pFileName = hb_fsFNameSplit( argv[ iArg ] );
      }

      iArg++;
  }

  if( hb_comp_pFileName )
  {
      if( ! hb_comp_pFileName->szExtension )
        hb_comp_pFileName->szExtension =".prg";

      hb_fsFNameMerge( szFileName, hb_comp_pFileName );

      if( !hb_pp_fopen( szFileName ) )
        {
          printf("\nCan't open %s\n", szFileName );
          return 1;
        }

      printf( "\nParsing file %s\n", szFileName );
  }
  else
  {
      printf( "\nSyntax:  %s <file[.prg]> [options]"
              "\n"
              "\nOptions:  /d<id>[=<val>]   #define <id>"
              "\n          /i<path>         add #include file search path"
              "\n          /o               creates hbpp.out with all tables"
              "\n          /n               with those only, which defined in your file"
              "\n          /w               enable warnings"
              "\n"
              , argv[ 0 ] );

      if( bOutTable )
      {
         OutTable( NULL, NULL, NULL );
      }

      return 1;
  }

  hb_comp_pFileName->szExtension = ".ppo";
  hb_fsFNameMerge( szPpoName, hb_comp_pFileName );

  if( ( handl_o = fopen( szPpoName, "wt" ) ) == NULL )
    {
      printf("\nCan't open %s\n", szPpoName );
      return 1;
    }

  {
    char * szInclude = getenv( "INCLUDE" );

    if( szInclude )
      {
        char * pPath;
        char * pDelim;

        pPath = szInclude = hb_strdup( szInclude );
        while( ( pDelim = strchr( pPath, HB_OS_PATH_LIST_SEP_CHR ) ) != NULL )
          {
            *pDelim = '\0';
            AddSearchPath( pPath, &hb_comp_pIncludePath );
            pPath = pDelim + 1;
          }
        AddSearchPath( pPath, &hb_comp_pIncludePath );
      }
  }

  hb_buffer = ( char* ) hb_xgrab( HB_PP_STR_SIZE );
  while( hb_pp_Internal( handl_o,hb_buffer ) > 0 )
  {
  }
  fclose( hb_comp_files.pLast->handle );
  hb_xfree( hb_comp_files.pLast->pBuffer );
  hb_xfree( hb_comp_files.pLast );
  hb_xfree( hb_buffer );
  fclose( handl_o );

  if( bOutTable )
  {
     OutTable( NULL, NULL, NULL );
  }
  else if( bOutNew )
  {
     OutTable( stdef, stcmd, sttra );
  }

  printf( "\nOk" );

  return 0;
}

static void OutTable( DEFINES * endDefine, COMMANDS * endCommand, COMMANDS * endTranslate )
{
  FILE *handl_o;
  int ipos, len_mpatt = 0, len_value;
  int num;
  DEFINES * stdef1 = hb_pp_topDefine, * stdef2 = NULL, * stdef3;
  COMMANDS * stcmd1 = hb_pp_topCommand, * stcmd2 = NULL, * stcmd3;

  HB_TRACE(HB_TR_DEBUG, ("OutTable(%p, %p)", endDefine, endCommand));

  while( stdef1 != endDefine )
  {
     stdef3 = stdef1->last;
     stdef1->last = stdef2;
     stdef2 = stdef1;
     stdef1 = stdef3;
  }

  while( stcmd1 != endCommand )
  {
     stcmd3 = stcmd1->last;
     stcmd1->last = stcmd2;
     stcmd2 = stcmd1;
     stcmd1 = stcmd3;
  }

  if( ( handl_o = fopen( "hbpp.out", "wt" ) ) == NULL )
  {
      printf( "\nCan't open hbpp.out\n" );
      return;
  }

  num = 1;
  while( stdef2 != NULL )
  {
      fprintf( handl_o, "\n   static DEFINES sD___%i = ", num );
      fprintf( handl_o, "{\"%s\",", stdef2->name );

      if( stdef2->pars )
      {
        fprintf( handl_o, "\"%s\",", stdef2->pars );
      }
      else
      {
        fprintf( handl_o, "NULL," );
      }

      fprintf( handl_o, "%d,", stdef2->npars );

      if( stdef2->value )
      {
        fprintf( handl_o, "\"%s\"", stdef2->value );
      }
      else
      {
        fprintf( handl_o, "NULL" );
      }

      if( num == 1 )
      {
        fprintf( handl_o, ", NULL };" );
      }
      else
      {
        fprintf( handl_o, ", &sD___%i };", num - 1 );
      }

      stdef2 = stdef2->last;
      num++;
  }

  fprintf( handl_o, "\n   DEFINES * hb_pp_topDefine = " );

  if( num == 1 )
  {
    fprintf( handl_o, "NULL;" );
  }
  else
  {
    fprintf( handl_o, " = &sD___%i;\n", num - 1 );
  }

  num = 1;
  while( stcmd2 != NULL )
  {
      fprintf( handl_o, "\n   static COMMANDS sC___%i = ", num );
      fprintf( handl_o, "{%d,\"%s\",", stcmd2->com_or_xcom, stcmd2->name );

      if( stcmd2->mpatt != NULL )
      {
          len_mpatt = hb_pp_strocpy( s_szLine, stcmd2->mpatt );
          while( ( ipos = (int) hb_strAt( "\1", 1, s_szLine, len_mpatt ) ) > 0 )
          {
              hb_pp_Stuff( "\\1", s_szLine + ipos - 1, 2, 1, len_mpatt );
              len_mpatt++;
          }

          while( ( ipos = (int) hb_strAt( "\16", 1, s_szLine, len_mpatt ) ) > 0 )
          {
              hb_pp_Stuff( "\\16", s_szLine + ipos - 1, 3, 1, len_mpatt );
              len_mpatt += 2;
          }

          while( ( ipos = (int) hb_strAt( "\17", 1, s_szLine, len_mpatt ) ) > 0 )
          {
              hb_pp_Stuff( "\\17", s_szLine + ipos - 1, 3, 1, len_mpatt );
              len_mpatt += 2;
          }

          fprintf( handl_o, "\"%s\",", s_szLine );
      }
      else
      {
         fprintf( handl_o, "NULL," );
      }

      if( stcmd2->value != NULL )
      {
          len_value = hb_pp_strocpy( s_szLine, stcmd2->value );
          while( ( ipos = (int) hb_strAt( "\1", 1, s_szLine, len_value ) ) > 0 )
          {
              hb_pp_Stuff( "\\1", s_szLine + ipos - 1, 2, 1, len_value );
              len_value++;
          }

          while( ( ipos = (int) hb_strAt( "\16", 1, s_szLine, len_value ) ) > 0 )
          {
              hb_pp_Stuff( "\\16", s_szLine + ipos - 1, 3, 1, len_value );
              len_value += 2;
          }

          while( ( ipos = (int) hb_strAt( "\17", 1, s_szLine, len_value ) ) > 0 )
          {
              hb_pp_Stuff( "\\17", s_szLine + ipos - 1, 3, 1, len_value );
              len_value += 2;
          }

          if( len_mpatt + len_value > 80 )
          {
            fprintf( handl_o, "\n       " );
          }

          fprintf( handl_o, "\"%s\"", s_szLine );
      }
      else
      {
         fprintf( handl_o, "NULL" );
      }

      if( num == 1 )
      {
        fprintf( handl_o, ",NULL };" );
      }
      else
      {
        fprintf( handl_o, ",&sC___%i };", num - 1 );
      }

      stcmd2 = stcmd2->last;
      num++;
  }

  fprintf( handl_o, "\n   COMMANDS * hb_pp_topCommand = " );

  if( num == 1 )
  {
    fprintf( handl_o, "NULL;" );
  }
  else
  {
    fprintf( handl_o, " = &sC___%i;\n", num - 1 );
  }

  stcmd1 = hb_pp_topTranslate;
  stcmd2 = NULL;
  while( stcmd1 != endTranslate )
  {
      stcmd3 = stcmd1->last;
      stcmd1->last = stcmd2;
      stcmd2 = stcmd1;
      stcmd1 = stcmd3;
  }

  num = 1;
  while( stcmd2 != NULL )
  {
      fprintf( handl_o, "\n   static COMMANDS sT___%i = ", num );
      fprintf( handl_o, "{%d,\"%s\",", stcmd2->com_or_xcom, stcmd2->name );

      if( stcmd2->mpatt != NULL )
      {
          len_mpatt = hb_pp_strocpy( s_szLine, stcmd2->mpatt );

          while( ( ipos = (int) hb_strAt( "\1", 1, s_szLine, len_mpatt ) ) > 0 )
          {
              hb_pp_Stuff( "\\1", s_szLine + ipos - 1, 2, 1, len_mpatt );
              len_mpatt++;
          }

          while( ( ipos = (int) hb_strAt( "\16", 1, s_szLine, len_mpatt ) ) > 0 )
          {
              hb_pp_Stuff( "\\16", s_szLine + ipos - 1, 3, 1, len_mpatt );
              len_mpatt += 2;
          }

          while( ( ipos = (int) hb_strAt( "\17", 1, s_szLine, len_mpatt ) ) > 0 )
          {
              hb_pp_Stuff( "\\17", s_szLine + ipos - 1, 3, 1, len_mpatt );
              len_mpatt += 2;
          }

          fprintf( handl_o, "\"%s\",", s_szLine );
      }
      else
      {
        fprintf( handl_o, "NULL," );
      }

      if( stcmd2->value != NULL )
      {
          len_value = hb_pp_strocpy( s_szLine, stcmd2->value );

          while( ( ipos = (int) hb_strAt( "\1", 1, s_szLine, len_value ) ) > 0 )
          {
              hb_pp_Stuff( "\\1", s_szLine + ipos - 1, 2, 1, len_value );
              len_value++;
          }

          while( ( ipos = (int) hb_strAt( "\16", 1, s_szLine, len_value ) ) > 0 )
          {
              hb_pp_Stuff( "\\16", s_szLine + ipos - 1, 3, 1, len_value );
              len_value += 2;
          }

          while( ( ipos = (int) hb_strAt( "\17", 1, s_szLine, len_value ) ) > 0 )
          {
              hb_pp_Stuff( "\\17", s_szLine + ipos - 1, 3, 1, len_value );
              len_value += 2;
          }

          if( len_mpatt + len_value > 80 )
          {
            fprintf( handl_o, "\n       " );
          }

          fprintf( handl_o, "\"%s\"", s_szLine );
      }
      else
      {
         fprintf( handl_o, "NULL" );
      }

      if( num == 1 )
      {
        fprintf( handl_o, ",NULL };" );
      }
      else
      {
        fprintf( handl_o, ",&sT___%i };", num - 1 );
      }

      stcmd2 = stcmd2->last;
      num++;
  }

  fprintf( handl_o, "\n   COMMANDS * hb_pp_topTranslate = " );

  if( num == 1 )
  {
    fprintf( handl_o, "NULL;" );
  }
  else
  {
    fprintf( handl_o, "&sT___%i;", num - 1 );
  }

  fclose( handl_o );
}

/*
 * Function that adds specified path to the list of pathnames to search
 */
static void AddSearchPath( char * szPath, HB_PATHNAMES * * pSearchList )
{
  HB_PATHNAMES * pPath = *pSearchList;

  HB_TRACE(HB_TR_DEBUG, ("AddSearchPath(%s, %p)", szPath, pSearchList));

  if( pPath )
    {
      while( pPath->pNext )
        pPath = pPath->pNext;
      pPath->pNext = ( HB_PATHNAMES * ) hb_xgrab( sizeof( HB_PATHNAMES ) );
      pPath = pPath->pNext;
    }
  else
    {
      *pSearchList = pPath = ( HB_PATHNAMES * ) hb_xgrab( sizeof( HB_PATHNAMES ) );
    }
  pPath->pNext  = NULL;
  pPath->szPath = szPath;
}

void hb_compGenError( const char * _szErrors[], char cPrefix, int iError, const char * szError1, const char * szError2 )
{
  HB_TRACE(HB_TR_DEBUG, ("hb_compGenError(%p, %c, %d, %s, %s)", _szErrors, cPrefix, iError, szError1, szError2));

  printf( "\r(%i) ", hb_comp_iLine );
  printf( "Error %c%04i  ", cPrefix, iError );
  printf( _szErrors[ iError - 1 ], szError1, szError2 );
  printf( "\n\n" );

  /*
  exit( EXIT_FAILURE );
  */
}

void hb_compGenWarning( const char* _szWarnings[], char cPrefix, int iWarning, const char * szWarning1, const char * szWarning2)
{
  HB_TRACE(HB_TR_DEBUG, ("hb_compGenWarning(%p, %c, %d, %s, %s)", _szWarnings, cPrefix, iWarning, szWarning1, szWarning2));

  if( s_iWarnings )
    {
      const char *szText = _szWarnings[ iWarning - 1 ];

      if( ( ( int ) (szText[ 0 ] - '0') ) <= s_iWarnings )
        {
          printf( "\r(%i) ", hb_comp_iLine );
          printf( "Warning %c%04i  ", cPrefix, iWarning );
          printf( szText + 1, szWarning1, szWarning2 );
          printf( "\n" );
        }
    }
}

#ifndef hb_xgrab
void * hb_xgrab( HB_SIZE ulSize )         /* allocates fixed memory, exits on failure */
{
  void * pMem = malloc( (size_t) ulSize );

  HB_TRACE(HB_TR_DEBUG, ("hb_xgrab(%lu)", ulSize));

  if( ! pMem )
    hb_compGenError( hb_pp_szErrors, 'P', HB_PP_ERR_MEMALLOC, NULL, NULL );

  return pMem;
}
#endif

#ifndef hb_xrealloc
void * hb_xrealloc( void * pMem, HB_SIZE ulSize )       /* reallocates memory */
{
  void * pResult = realloc( pMem, (size_t) ulSize );

  HB_TRACE(HB_TR_DEBUG, ("hb_xrealloc(%p, %lu)", pMem, ulSize));

  if( ! pResult )
    hb_compGenError( hb_pp_szErrors, 'P', HB_PP_ERR_MEMREALLOC, NULL, NULL );

  return pResult;
}
#endif

#ifndef hb_xfree
void hb_xfree( void * pMem )            /* frees fixed memory */
{
  HB_TRACE(HB_TR_DEBUG, ("hb_xfree(%p)", pMem));

  if( pMem )
    free( pMem );
  else
    hb_compGenError( hb_pp_szErrors, 'P', HB_PP_ERR_MEMFREE, NULL, NULL );
}
#endif

static BOOL hb_pp_fopen( char * szFileName )
{
   PFILE pFile;
   FILE * handl_i = fopen( szFileName, "r" );

   if( !handl_i )
      return FALSE;

   pFile = ( PFILE ) hb_xgrab( sizeof( _FILE ) );
   pFile->handle = handl_i;
   pFile->pBuffer = hb_xgrab( HB_PP_BUFF_SIZE );
   pFile->iBuffer = pFile->lenBuffer = 10;
   pFile->szFileName = szFileName;
   pFile->pPrev = NULL;

   hb_comp_files.pLast = pFile;
   hb_comp_files.iFiles = 1;

   return TRUE;
}

PINLINE   hb_compInlineAdd( char * szFunName )
{
   HB_SYMBOL_UNUSED( szFunName );
   return NULL;
}

/* Needed for trace */
const char * hb_fsNameConv( const char * szFileName, char ** pszFree ) { if( pszFree ) * pszFree = NULL; return szFileName; }
int hb_setGetDirSeparator( void ) { return HB_OS_PATH_DELIM_CHR; }

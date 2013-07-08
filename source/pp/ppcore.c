/*
 * $Id: ppcore.c 9908 2013-01-13 05:23:04Z andijahja $
 */

/*
 * Harbour Project source code:
 *    Clipper compatible preprocessor
 *
 * Copyright 2006 Przemyslaw Czerpak <druzus / at / priv.onet.pl>
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

/* #define HB_PP_MULTILINE_STRING */
/* #define HB_C52_STRICT */
/* #define HB_PP_NO_LINEINFO_TOKEN */
/* #define HB_PP_STRICT_LINEINFO_TOKEN */

#if defined( __POCC__ )
   #pragma warn(push)
   #pragma warn(disable:2154)
#endif

#define _HB_PP_INTERNAL

#if defined( HB_PP_STRICT_LINEINFO_TOKEN ) && \
   defined( HB_PP_NO_LINEINFO_TOKEN )
#  undef HB_PP_NO_LINEINFO_TOKEN
#endif

#include "hbpp.h"
#include "hbver.h"
#include "hbdate.h"

#if defined( __HB_COMPILER__ )
   #include "hbcomp.h"
   #undef HB_PP_WARN_OVERLOADED_REDEF
   #undef HB_PP_ERR_EMPTY_OPTIONAL
   #undef HB_PP_ERR_UNCLOSED_OPTIONAL
   #undef HB_PP_ERR_DIRECTIVE_ENDIF
   #undef HB_PP_ERR_DIRECTIVE_ELSE
   #undef HB_PP_ERR_EXPLICIT
   #undef HB_PP_ERR_CANNOT_OPEN_RULES
#endif

HB_EXTERN_BEGIN
extern void hb_compSetDeferredFlagOn( void );
extern void hb_compSetCOutput( int iOutput );
HB_EXTERN_END

#ifdef __XHARBOUR__
#ifndef  __PPGEN__
#include "hbverbld.h"
#endif
#endif

#define HB_PP_WARN_DEFINE_REDEF           1           /* C1005 */

#ifdef __XHARBOUR__
#ifndef HB_C52_STRICT
   #define HB_PP_WARN_OVERLOADED_REDEF    2           /* Non Clipper */
#endif
#endif

#define HB_PP_ERR_ILLEGAL_CHAR            1           /* C2004 */
#define HB_PP_ERR_STRING_TERMINATOR       2           /* C2007 */
#define HB_PP_ERR_MISSING_ENDTEXT         3           /* C2033 */
#define HB_PP_ERR_DEFINE_SYNTAX           4           /* C2055 */
#define HB_PP_ERR_LABEL_MISSING_IN_DEFINE 5           /* C2057 */
#define HB_PP_ERR_PARE_MISSING_IN_DEFINE  6           /* C2058 */
#define HB_PP_ERR_MISSING_PATTERN_SEP     7           /* C2059 */
#define HB_PP_ERR_UNKNOWN_RESULT_MARKER   8           /* C2060 */
#define HB_PP_ERR_WRONG_LABEL             9           /* C2061 */
#define HB_PP_ERR_BAD_MATCH_MARKER        10          /* C2062 */
#define HB_PP_ERR_EMPTY_OPTIONAL          11          /* C2065 */
#define HB_PP_ERR_UNCLOSED_OPTIONAL       12          /* C2066 */
#define HB_PP_ERR_DIRECTIVE_IFDEF         13          /* C2068 */
#define HB_PP_ERR_DIRECTIVE_ENDIF         14          /* C2069 */
#define HB_PP_ERR_DIRECTIVE_ELSE          15          /* C2070 */
#define HB_PP_ERR_DIRECTIVE_UNDEF         16          /* C2071 */
#define HB_PP_ERR_AMBIGUOUS_MATCH_PATTERN 17          /* C2072 */
#define HB_PP_ERR_NESTED_OPTIONAL         18          /* C2073 */
#define HB_PP_ERR_EXPLICIT                19          /* C2074 */
#define HB_PP_ERR_CYCLIC_DEFINE           20          /* C2078 */
#define HB_PP_ERR_CYCLIC_TRANSLATE        21          /* C2079 */
#define HB_PP_ERR_CYCLIC_COMMAND          22          /* C2080 */
#define HB_PP_ERR_UNTERMINATED_COMMENT    23          /* C2083 */
#define HB_PP_ERR_PRAGMA                  24          /* C20?? */
#define HB_PP_ERR_DIRECTIVE_IF            25          /* C20?? */
#define HB_PP_ERR_CANNOT_OPEN_INPUT       26          /* C30?? */
#define HB_PP_ERR_FILE_TOO_LONG           27          /* C30?? */
#define HB_PP_ERR_CANNOT_CREATE_FILE      28          /* C3006 */
#define HB_PP_ERR_CANNOT_OPEN_FILE        29          /* C3007 */
#define HB_PP_ERR_WRONG_FILE_NAME         30          /* C3008 */
#define HB_PP_ERR_NESTED_INCLUDES         31          /* C3009 */
#define HB_PP_ERR_INVALID_DIRECTIVE       32          /* C3010 */
#define HB_PP_ERR_CANNOT_OPEN_RULES       33          /* C3011 */

#ifdef __XHARBOUR__
#define HB_PP_ERR_DELETE_SYNTAX           33          /* Non Clipper */
#define HB_PP_ERR_TOO_EXCESSIVE_PUSH      34          /* Non Clipper */
#endif

/* warning messages */
static const char * hb_pp_szWarnings[] =
{
   "1Redefinition or duplicate definition of #define %s"                /* C1005 */
#ifdef __XHARBOUR__
   , "1Overloaded #define %s"                                           /* Non Clipper */
#endif
};

/* error messages */
static const char * hb_pp_szErrors[] =
{
   "Illegal character: '\\x%s'",                                     /* C2004 */
   "Unterminated string: '%s'",                                      /* C2007 */
   "Missing ENDTEXT",                                                /* C2033 */
   "Syntax error in #define",                                        /* C2055 */
   "Label missing in #define",                                       /* C2057 */
   "Comma or right parenthesis missing in #define",                  /* C2058 */
   "Missing => in #translate/#command",                              /* C2059 */
   "Unknown result marker in #translate/#command: '%s'",             /* C2060 */
   "Label error in #translate/#command: '%s'",                       /* C2061 */
   "Bad match marker '%s' in #translate/#command",                   /* C2062 */
   "Empty optional clause in #translate/#command",                   /* C2065 */
   "Unclosed optional clause in #translate/#command",                /* C2066 */
   "Error in #ifdef",                                                /* C2068 */
   "#endif does not match #endif",                                   /* C2069 */
   "#else does not match #ifdef",                                    /* C2070 */
   "Error in #undef",                                                /* C2071 */
   "Ambiguous match pattern in #translate/#command",                 /* C2072 */
   "Result pattern contains nested clauses in #translate/#command",  /* C2073 */
   "#error: '%s'",                                                   /* C2074 */
   "Circularity detected in #define: '%s'",                          /* C2078 */
   "Circularity detected in #translate: '%s'",                       /* C2079 */
   "Circularity detected in #command: '%s'",                         /* C2080 */
   "Unterminated /* */ comment",                                     /* C2083 */

   "Error in #pragma",                                               /* C20?? */
   "Error in #if expression",                                        /* C20?? */

   "Cannot open input file: %s'",                                    /* C30?? */

   "File %s is too long",                                            /* C30?? */

   "Can't create preprocessed output file",                          /* C3006 */
   "Can't open #include file: '%s'",                                 /* C3007 */
   "Bad filename in #include",                                       /* C3008 */
   "Too many nested #includes",                                      /* C3009 */
   "Invalid name follows #",                                         /* C3010 */
   "Can't open standard rule file: '%s'"                             /* C3011 */
#ifdef __XHARBOUR__
   , "Syntax error in #untranslate/#uncommand: '%s'",                /* Non Clipper */
   "Too many nested #pragma push: '%s'"                              /* Non Clipper */
#endif
};


static const HB_PP_OPERATOR s_operators[] =
{
   { ".NOT.", 5, "!",     HB_PP_TOKEN_NOT | HB_PP_TOKEN_STATIC       },
   { ".AND.", 5, ".AND.", HB_PP_TOKEN_AND | HB_PP_TOKEN_STATIC       },
   { ".OR.",  4, ".OR.",  HB_PP_TOKEN_OR | HB_PP_TOKEN_STATIC        },
#ifndef HB_C52_STRICT
   { "...",   3, "...",   HB_PP_TOKEN_EPSILON | HB_PP_TOKEN_STATIC   },
#endif
   { "**=",   3, "^=",    HB_PP_TOKEN_EXPEQ | HB_PP_TOKEN_STATIC     },
   { "**",    2, "^",     HB_PP_TOKEN_POWER | HB_PP_TOKEN_STATIC     },
   { "++",    2, "++",    HB_PP_TOKEN_INC | HB_PP_TOKEN_STATIC       },
   { "--",    2, "--",    HB_PP_TOKEN_DEC | HB_PP_TOKEN_STATIC       },
   { "==",    2, "==",    HB_PP_TOKEN_EQUAL | HB_PP_TOKEN_STATIC     },
   { ":=",    2, ":=",    HB_PP_TOKEN_ASSIGN | HB_PP_TOKEN_STATIC    },
   { "+=",    2, "+=",    HB_PP_TOKEN_PLUSEQ | HB_PP_TOKEN_STATIC    },
   { "-=",    2, "-=",    HB_PP_TOKEN_MINUSEQ | HB_PP_TOKEN_STATIC   },
   { "*=",    2, "*=",    HB_PP_TOKEN_MULTEQ | HB_PP_TOKEN_STATIC    },
   { "/=",    2, "/=",    HB_PP_TOKEN_DIVEQ | HB_PP_TOKEN_STATIC     },
   { "%=",    2, "%=",    HB_PP_TOKEN_MODEQ | HB_PP_TOKEN_STATIC     },
   { "^=",    2, "^=",    HB_PP_TOKEN_EXPEQ | HB_PP_TOKEN_STATIC     },
   { "<=",    2, "<=",    HB_PP_TOKEN_LE | HB_PP_TOKEN_STATIC        },
   { ">=",    2, ">=",    HB_PP_TOKEN_GE | HB_PP_TOKEN_STATIC        },
   { "!=",    2, "<>",    HB_PP_TOKEN_NE | HB_PP_TOKEN_STATIC        },
   { "<>",    2, "<>",    HB_PP_TOKEN_NE | HB_PP_TOKEN_STATIC        },
   { "->",    2, "->",    HB_PP_TOKEN_ALIAS | HB_PP_TOKEN_STATIC     },
#ifdef __XHARBOUR__
   { "<<",    2, "<<",    HB_PP_TOKEN_SHIFTL | HB_PP_TOKEN_STATIC    },
   { ">>",    2, ">>",    HB_PP_TOKEN_SHIFTR | HB_PP_TOKEN_STATIC    },
   { "^^",    2, "^^",    HB_PP_TOKEN_BITXOR | HB_PP_TOKEN_STATIC    },
#endif
   { "@",     1, "@",     HB_PP_TOKEN_REFERENCE | HB_PP_TOKEN_STATIC },
   { "(",     1, "(",     HB_PP_TOKEN_LEFT_PB | HB_PP_TOKEN_STATIC   },
   { ")",     1, ")",     HB_PP_TOKEN_RIGHT_PB | HB_PP_TOKEN_STATIC  },
   { "[",     1, "[",     HB_PP_TOKEN_LEFT_SB | HB_PP_TOKEN_STATIC   },
   { "]",     1, "]",     HB_PP_TOKEN_RIGHT_SB | HB_PP_TOKEN_STATIC  },
   { "{",     1, "{",     HB_PP_TOKEN_LEFT_CB | HB_PP_TOKEN_STATIC   },
   { "}",     1, "}",     HB_PP_TOKEN_RIGHT_CB | HB_PP_TOKEN_STATIC  },
   { ",",     1, ",",     HB_PP_TOKEN_COMMA | HB_PP_TOKEN_STATIC     },
   { "\\",    1, "\\",    HB_PP_TOKEN_BACKSLASH | HB_PP_TOKEN_STATIC },
   { "|",     1, "|",     HB_PP_TOKEN_PIPE | HB_PP_TOKEN_STATIC      },
   { ".",     1, ".",     HB_PP_TOKEN_DOT | HB_PP_TOKEN_STATIC       },
   { "&",     1, "&",     HB_PP_TOKEN_AMPERSAND | HB_PP_TOKEN_STATIC },
   { ":",     1, ":",     HB_PP_TOKEN_SEND | HB_PP_TOKEN_STATIC      },
   { "!",     1, "!",     HB_PP_TOKEN_NOT | HB_PP_TOKEN_STATIC       },
   { "=",     1, "=",     HB_PP_TOKEN_EQ | HB_PP_TOKEN_STATIC        },
   { "<",     1, "<",     HB_PP_TOKEN_LT | HB_PP_TOKEN_STATIC        },
   { ">",     1, ">",     HB_PP_TOKEN_GT | HB_PP_TOKEN_STATIC        },
   { "#",     1, "#",     HB_PP_TOKEN_HASH | HB_PP_TOKEN_STATIC      },
   { "$",     1, "$",     HB_PP_TOKEN_IN | HB_PP_TOKEN_STATIC        },
   { "+",     1, "+",     HB_PP_TOKEN_PLUS | HB_PP_TOKEN_STATIC      },
   { "-",     1, "-",     HB_PP_TOKEN_MINUS | HB_PP_TOKEN_STATIC     },
   { "*",     1, "*",     HB_PP_TOKEN_MULT | HB_PP_TOKEN_STATIC      },
   { "/",     1, "/",     HB_PP_TOKEN_DIV | HB_PP_TOKEN_STATIC       },
   { "%",     1, "%",     HB_PP_TOKEN_MOD | HB_PP_TOKEN_STATIC       },
   { "^",     1, "^",     HB_PP_TOKEN_POWER | HB_PP_TOKEN_STATIC     }
/* unused: ? ~ " ' ` */
/* not accesible: " ' `  */
/* illegal in Clipper: ~ */
};

static void hb_pp_disp( PHB_PP_STATE pState, const char * szMessage )
{
   if( ! pState->pDispFunc )
   {
      printf( "%s", szMessage );
      fflush( stdout );
   }
   else
      ( pState->pDispFunc )( pState->cargo, szMessage );
}

static void hb_pp_error( PHB_PP_STATE pState, char type, int iError, const char * szParam )
{
   const char ** szMsgTable = type == 'W' ? hb_pp_szWarnings : hb_pp_szErrors;

   if( pState->pErrorFunc )
   {
      ( pState->pErrorFunc )( pState->cargo, szMsgTable, type, iError, szParam, NULL );
   }
   else
   {
      char  line[ 16 ];
      char  msg[ 200 ];
      char  buffer[ 256 ];

      if( pState->pFile )
         hb_snprintf( line, sizeof( line ), "(%d) ", pState->pFile->iCurrentLine );
      else
         line[ 0 ] = '\0';
      hb_snprintf( msg, sizeof( msg ), szMsgTable[ iError - 1 ], szParam );
      hb_snprintf( buffer, sizeof( buffer ), "%s%s: %s\n", line,
                   type == 'F' ? "Fatal" : type == 'W' ? "Warning" : "Error", msg );
      hb_pp_disp( pState, buffer );
   }
   if( type != 'W' )
   {
      pState->fError = TRUE;
      pState->iErrors++;
   }
}

static void hb_pp_operatorsFree( PHB_PP_OPERATOR pOperators, int iOperators )
{
   PHB_PP_OPERATOR pOperator = pOperators;

   while( --iOperators >= 0 )
   {
      hb_xfree( pOperator->name );
      hb_xfree( pOperator->value );
      ++pOperator;
   }
   hb_xfree( pOperators );
}

static PHB_PP_OPERATOR hb_pp_operatorFind( PHB_PP_STATE pState,
                                           char * buffer, HB_SIZE ulLen )
{
   PHB_PP_OPERATOR   pOperator   = pState->pOperators;
   int               i           = pState->iOperators;

   while( --i >= 0 )
   {
      if( pOperator->len <= ulLen &&
          hb_strnicmp( pOperator->name, buffer, pOperator->len ) == 0 )
         return pOperator;

      ++pOperator;
   }

   pOperator   = ( PHB_PP_OPERATOR ) s_operators;
   i           = sizeof( s_operators ) / sizeof( HB_PP_OPERATOR );

   do
   {
      if( pOperator->len <= ulLen &&
          hb_strnicmp( pOperator->name, buffer, pOperator->len ) == 0 )
         return pOperator;

      ++pOperator;
   }
   while( --i > 0 );

   return NULL;
}

#define HB_MEMBUF_DEFAULT_SIZE 256

static PHB_MEM_BUFFER hb_membufNew( void )
{
   PHB_MEM_BUFFER pBuffer = ( PHB_MEM_BUFFER ) hb_xgrab( sizeof( HB_MEM_BUFFER ) );

   pBuffer->ulLen       = 0;
   pBuffer->ulAllocated = HB_MEMBUF_DEFAULT_SIZE;
   pBuffer->pBufPtr     = ( char * ) hb_xgrab( pBuffer->ulAllocated );

   return pBuffer;
}

static void hb_membufFree( PHB_MEM_BUFFER pBuffer )
{
   hb_xfree( pBuffer->pBufPtr );
   hb_xfree( pBuffer );
}

static void hb_membufFlush( PHB_MEM_BUFFER pBuffer )
{
   pBuffer->ulLen = 0;
}

#ifdef HB_PP_MULTILINE_STRING
static void hb_membufRemove( PHB_MEM_BUFFER pBuffer, ULONG ulLeft )
{
   if( ulLeft < pBuffer->ulLen )
      pBuffer->ulLen = ulLeft;
}
#endif

static HB_SIZE hb_membufLen( PHB_MEM_BUFFER pBuffer )
{
   return pBuffer->ulLen;
}

static char * hb_membufPtr( PHB_MEM_BUFFER pBuffer )
{
   return pBuffer->pBufPtr;
}

static void hb_membufAddCh( PHB_MEM_BUFFER pBuffer, char ch )
{
   if( pBuffer->ulLen == pBuffer->ulAllocated )
   {
      pBuffer->ulAllocated <<= 1;
      pBuffer->pBufPtr     = ( char * ) hb_xrealloc( pBuffer->pBufPtr, pBuffer->ulAllocated );
   }
   pBuffer->pBufPtr[ pBuffer->ulLen++ ] = ch;
}

static void hb_membufAddData( PHB_MEM_BUFFER pBuffer, char * data, HB_SIZE ulLen )
{
   if( pBuffer->ulLen + ulLen > pBuffer->ulAllocated )
   {
      do
      {
         pBuffer->ulAllocated <<= 1;
      }
      while( pBuffer->ulLen + ulLen > pBuffer->ulAllocated );
      pBuffer->pBufPtr = ( char * ) hb_xrealloc( pBuffer->pBufPtr, pBuffer->ulAllocated );
   }

   HB_MEMCPY( &pBuffer->pBufPtr[ pBuffer->ulLen ], data, ( size_t ) ulLen );
   pBuffer->ulLen += ulLen;
}

static void hb_membufAddStr( PHB_MEM_BUFFER pBuffer, char * szText )
{
   hb_membufAddData( pBuffer, szText, strlen( szText ) );
}

static void hb_pp_tokenFree( PHB_PP_TOKEN pToken )
{
   if( HB_PP_TOKEN_ALLOC( pToken->type ) )
      hb_xfree( pToken->value );
   if( HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_MMARKER_RESTRICT ||
       HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_MMARKER_OPTIONAL ||
       HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_RMARKER_OPTIONAL )
   {
      while( pToken->pMTokens )
      {
         PHB_PP_TOKEN pMTokens = pToken->pMTokens;
         pToken->pMTokens = pMTokens->pNext;
         hb_pp_tokenFree( pMTokens );
      }
   }
   hb_xfree( pToken );
}

static void hb_pp_tokenListFree( PHB_PP_TOKEN * pTokenPtr )
{
   if( *pTokenPtr && ! HB_PP_TOKEN_ISPREDEF( *pTokenPtr ) )
   {
      do
      {
         PHB_PP_TOKEN pToken = *pTokenPtr;
         *pTokenPtr = pToken->pNext;
         hb_pp_tokenFree( pToken );
      }
      while( *pTokenPtr );
   }
}

static void hb_pp_tokenListFreeCmd( PHB_PP_TOKEN * pTokenPtr, PHB_PP_STATE pState )
{
   BOOL fStop = FALSE, fLine = FALSE;

   while( *pTokenPtr && ! fStop )
   {
      PHB_PP_TOKEN pToken = *pTokenPtr;
      *pTokenPtr  = pToken->pNext;
      fStop       = HB_PP_TOKEN_ISEOC( pToken );
      fLine       = HB_PP_TOKEN_ISEOL( pToken ) && ( pToken->type & HB_PP_TOKEN_LINECOUNTER );
      hb_pp_tokenFree( pToken );
   }

   if( fLine )
      pState->pFile->iCurrentLine++;
}

static int hb_pp_tokenMoveCommand( PHB_PP_TOKEN * pDestPtr, PHB_PP_TOKEN * pSrcPtr )
{
   PHB_PP_TOKEN   pToken;
   int            iLines = 0;

   while( *pSrcPtr )
   {
      pToken      = *pSrcPtr;
      *pSrcPtr    = pToken->pNext;
      *pDestPtr   = pToken;
      pDestPtr    = &pToken->pNext;
      if( HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_EOL )
         ++iLines;
      if( HB_PP_TOKEN_ISEOC( pToken ) )
      {
         break;
      }
   }
   *pDestPtr = NULL;

   return iLines;
}

static PHB_PP_TOKEN hb_pp_tokenResultEnd( PHB_PP_TOKEN * pTokenPtr, BOOL fDirect )
{
   PHB_PP_TOKEN pNext = NULL;

#ifdef HB_C52_STRICT
   HB_SYMBOL_UNUSED( fDirect );
#endif

   while( *pTokenPtr )
   {
      if( HB_PP_TOKEN_ISEOP( *pTokenPtr, fDirect ) )
      {
         pNext       = *pTokenPtr;
         *pTokenPtr  = NULL;
         break;
      }
      pTokenPtr = &( *pTokenPtr )->pNext;
   }

   return pNext;
}

static PHB_PP_TOKEN hb_pp_tokenNew( const char * value, HB_SIZE ulLen,
                                    int iSpaces, USHORT type )
{
   PHB_PP_TOKEN pToken = ( PHB_PP_TOKEN ) hb_xgrab( sizeof( HB_PP_TOKEN ) );

   /* TODO: error on overflow: ulLen >= USHRT_MAX */

   if( HB_PP_TOKEN_ALLOC( type ) )
   {
      if( ulLen <= 1 )
      {
         pToken->value  = ( char * ) hb_szAscii[ ulLen ? ( UCHAR ) value[ 0 ] : 0 ];
         type           |= HB_PP_TOKEN_STATIC;
      }
      else
      {
         pToken->value           = ( char * ) hb_xgrab( ulLen + 1 );
         HB_MEMCPY( pToken->value, value, ( size_t ) ulLen );
         pToken->value[ ulLen ]  = '\0';
      }
   }
   else
      pToken->value = ( char * ) value;

   pToken->len       = ( USHORT ) ulLen;
   pToken->spaces    = ( USHORT ) iSpaces;
   pToken->type      = type;
   pToken->index     = 0;
   pToken->pNext     = NULL;
   pToken->pMTokens  = NULL;

   return pToken;
}

static void hb_pp_tokenSetValue( PHB_PP_TOKEN pToken,
                                 const char * value, HB_SIZE ulLen )
{
   if( HB_PP_TOKEN_ALLOC( pToken->type ) )
      hb_xfree( pToken->value );
   if( ulLen <= 1 )
   {
      pToken->value  = ( char * ) hb_szAscii[ ulLen ? ( UCHAR ) value[ 0 ] : 0 ];
      pToken->type   |= HB_PP_TOKEN_STATIC;
   }
   else
   {
      pToken->type            &= ~HB_PP_TOKEN_STATIC;
      pToken->value           = ( char * ) hb_xgrab( ulLen + 1 );
      HB_MEMCPY( pToken->value, value, ( size_t ) ulLen );
      pToken->value[ ulLen ]  = '\0';
   }
   pToken->len = ( USHORT ) ulLen;
}

static PHB_PP_TOKEN hb_pp_tokenClone( PHB_PP_TOKEN pSource )
{
   PHB_PP_TOKEN pDest = ( PHB_PP_TOKEN ) hb_xgrab( sizeof( HB_PP_TOKEN ) );

   HB_MEMCPY( pDest, pSource, sizeof( HB_PP_TOKEN ) );
   if( HB_PP_TOKEN_ALLOC( pDest->type ) )
   {
      pDest->value               = ( char * ) hb_xgrab( pDest->len + 1 );
      HB_MEMCPY( pDest->value, pSource->value, pDest->len );
      pDest->value[ pDest->len ] = '\0';
   }
   pDest->pNext = NULL;

   return pDest;
}

static void hb_pp_tokenAdd( PHB_PP_TOKEN ** pTokenPtr,
                            const char * value, HB_SIZE ulLen,
                            int iSpaces, USHORT type )
{
   PHB_PP_TOKEN pToken = hb_pp_tokenNew( value, ulLen, iSpaces, type );

   **pTokenPtr = pToken;
   *pTokenPtr  = &pToken->pNext;
}

static void hb_pp_tokenAddCmdSep( PHB_PP_STATE pState )
{
   hb_pp_tokenAdd( &pState->pNextTokenPtr, ";", 1, pState->iSpacesNL, HB_PP_TOKEN_EOC | HB_PP_TOKEN_STATIC );
   pState->pFile->iTokens++;
   pState->fNewStatement   = TRUE;
   pState->fCanNextLine    = FALSE;
   if( pState->iBlockState )
   {
      if( pState->iBlockState == 5 )
         pState->iNestedBlock++;
      pState->iBlockState = 0;
   }
}

static void hb_pp_tokenAddNext( PHB_PP_STATE pState, const char * value, HB_SIZE ulLen,
                                USHORT type )
{
   if( pState->fCanNextLine )
      hb_pp_tokenAddCmdSep( pState );

   if( ! pState->fDirective )
   {
      if( pState->iNestedBlock && pState->fNewStatement &&
          HB_PP_TOKEN_TYPE( type ) == HB_PP_TOKEN_RIGHT_ECB )
      {
         pState->iBlockState = 0;
         pState->iNestedBlock--;
      }
      else if( pState->iLastType == HB_PP_TOKEN_LEFT_ECB &&
               HB_PP_TOKEN_TYPE( type ) == HB_PP_TOKEN_PIPE )
      {
         pState->iBlockState = 1;
      }
      else if( pState->iBlockState )
      {
         if( ( pState->iBlockState == 1 || pState->iBlockState == 2 ||
               pState->iBlockState == 4 ) &&
             HB_PP_TOKEN_TYPE( type ) == HB_PP_TOKEN_PIPE )
            pState->iBlockState = 5;
         else if( pState->iBlockState == 1 &&
                  HB_PP_TOKEN_TYPE( type ) == HB_PP_TOKEN_KEYWORD )
            pState->iBlockState = 2;
         else if( pState->iBlockState == 1 &&
                  HB_PP_TOKEN_TYPE( type ) == HB_PP_TOKEN_EPSILON )
            pState->iBlockState = 4;
         else if( pState->iBlockState == 2 &&
                  HB_PP_TOKEN_TYPE( type ) == HB_PP_TOKEN_COMMA )
            pState->iBlockState = 1;
         else
            pState->iBlockState = 0;
      }

      if( pState->fNewStatement && ulLen == 1 && *value == '#' )
      {
         pState->fDirective   = TRUE;
         value                = "#";
         type                 = HB_PP_TOKEN_DIRECTIVE | HB_PP_TOKEN_STATIC;
      }
   }

#ifndef HB_C52_STRICT
   if( pState->iSpacesMin != 0 && pState->iSpaces == 0 &&
       HB_PP_TOKEN_TYPE( type ) == HB_PP_TOKEN_KEYWORD )
      pState->iSpaces = pState->iSpacesMin;
#endif
   hb_pp_tokenAdd( &pState->pNextTokenPtr, value, ulLen, pState->iSpaces, type );
   pState->pFile->iTokens++;
   pState->fNewStatement   = FALSE;

   pState->iSpaces         = pState->iSpacesMin = 0;
   pState->iLastType       = HB_PP_TOKEN_TYPE( type );

   if( pState->iInLineState != HB_PP_INLINE_OFF )
   {
      if( pState->iInLineState == HB_PP_INLINE_START &&
          pState->iLastType == HB_PP_TOKEN_LEFT_PB )
      {
         pState->iInLineState    = HB_PP_INLINE_PARAM;
         pState->iInLineBraces   = 1;
      }
      else if( pState->iInLineState == HB_PP_INLINE_PARAM )
      {
         if( pState->iLastType == HB_PP_TOKEN_LEFT_PB )
            pState->iInLineBraces++;
         else if( pState->iLastType == HB_PP_TOKEN_RIGHT_PB )
         {
            if( --pState->iInLineBraces == 0 )
               pState->iInLineState = HB_PP_INLINE_BODY;
         }
      }
      else
         pState->iInLineState = HB_PP_INLINE_OFF;
   }
}

static void hb_pp_tokenAddStreamFunc( PHB_PP_STATE pState, PHB_PP_TOKEN pToken,
                                      const char * value, HB_SIZE ulLen )
{
   while( pToken )
   {
      if( HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_RMARKER_STRDUMP )
      {
         if( value )
         {
            hb_pp_tokenAdd( &pState->pNextTokenPtr, value, ulLen, pToken->spaces, HB_PP_TOKEN_STRING );
            pState->pFile->iTokens++;
         }
      }
      else
      {
         *pState->pNextTokenPtr  = hb_pp_tokenClone( pToken );
         pState->pNextTokenPtr   = &( *pState->pNextTokenPtr )->pNext;
         pState->pFile->iTokens++;
      }
      pToken = pToken->pNext;
   }
   pState->fNewStatement = TRUE;
}

static void hb_pp_readLine( PHB_PP_STATE pState )
{
   int ch, iLine = 0;

   while( TRUE )
   {
      if( pState->pFile->pLineBuf )
      {
         if( pState->pFile->ulLineBufLen )
         {
            ch = ( UCHAR ) pState->pFile->pLineBuf[ 0 ];
            pState->pFile->pLineBuf++;
            pState->pFile->ulLineBufLen--;
         }
         else
            break;
      }
      else
         ch = fgetc( pState->pFile->file_in );

      if( ch == EOF )
      {
         pState->pFile->fEof = TRUE;
         break;
      }

      iLine = 1;
      /* In Clipper ^Z works like \n */
      if( ch == '\n' || ch == '\x1a' )
      {
         break;
      }
      /* Clipper strips \r characters even from quoted strings */
      else if( ch != '\r' )
      {
         hb_membufAddCh( pState->pBuffer, ( char ) ch );
      }
   }
   pState->iLineTot  += iLine;
   iLine             = ++pState->pFile->iCurrentLine / 100;
   if( ! pState->fQuiet &&
       iLine != pState->pFile->iLastDisp )
   {
      char szLine[ 12 ];

      pState->pFile->iLastDisp = iLine;
      hb_snprintf( szLine, sizeof( szLine ), "\r%i00\r", iLine );
      hb_pp_disp( pState, szLine );
   }
}

static BOOL hb_pp_canQuote( BOOL fQuote, char * pBuffer, HB_SIZE ulLen,
                            HB_SIZE ul, HB_SIZE * pulAt )
{
   char cQuote = 0;

   /*
    * TODO: this is Clipper compatible but it breaks valid code so we may
    *       think about changing this condition in the future.
    */
   while( ul < ulLen )
   {
      if( pBuffer[ ul ] == ']' )
      {
         if( cQuote && ! fQuote )
         {
            HB_SIZE u = ul + 1;
            cQuote = 0;
            while( u < ulLen )
            {
               if( cQuote )
               {
                  if( pBuffer[ u ] == cQuote )
                     cQuote = 0;
               }
               else if( pBuffer[ u ] == '`' )
                  cQuote = '\'';
               else if( pBuffer[ u ] == '\'' || pBuffer[ u ] == '"' )
                  cQuote = pBuffer[ u ];
               else if( pBuffer[ u ] == '[' )
                  hb_pp_canQuote( TRUE, pBuffer, ulLen, u + 1, &u );
               ++u;
            }
            fQuote = cQuote == 0;
         }
         if( fQuote )
            *pulAt = ul;
         return fQuote;
      }
      else if( ! fQuote )
      {
         if( cQuote )
         {
            if( pBuffer[ ul ] == cQuote )
               cQuote = 0;
         }
         else if( pBuffer[ ul ] == '`' )
            cQuote = '\'';
         else if( pBuffer[ ul ] == '\'' || pBuffer[ ul ] == '"' )
            cQuote = pBuffer[ ul ];
      }
      ++ul;
   }
   return FALSE;
}

static BOOL hb_pp_hasCommand( char * pBuffer, HB_SIZE ulLen, HB_SIZE * pulAt, int iCmds, ... )
{
   HB_SIZE  ul = 0, u;
   char *   cmd;
   va_list  va;
   int      i;

   va_start( va, iCmds );
   for( i = 0; i < iCmds && ul < ulLen; ++i )
   {
      cmd   = va_arg( va, char * );
      u     = strlen( cmd );
      while( ul < ulLen && HB_PP_ISBLANK( pBuffer[ ul ] ) )
         ++ul;
      if( ul + u > ulLen || hb_strnicmp( cmd, pBuffer + ul, u ) != 0 )
         break;
      ul += u;
      if( ul < ulLen && ( HB_PP_ISNEXTIDCHAR( cmd[ u - 1 ] ) ||
                          HB_PP_ISTEXTCHAR( cmd[ u - 1 ] ) ) &&
          ( HB_PP_ISNEXTIDCHAR( pBuffer[ ul ] ) ||
            HB_PP_ISTEXTCHAR( pBuffer[ ul ] ) ) )
         break;
   }
   va_end( va );

   if( i == iCmds )
   {
      while( ul < ulLen && HB_PP_ISBLANK( pBuffer[ ul ] ) )
         ++ul;
      if( ul == ulLen || pBuffer[ ul ] == ';' )
      {
         *pulAt = ul;
         return TRUE;
      }
   }
   return FALSE;
}

static void hb_pp_dumpEnd( PHB_PP_STATE pState )
{
   pState->iStreamDump = HB_PP_STREAM_OFF;
   if( pState->iCondCompile )
   {
      hb_membufFlush( pState->pDumpBuffer );
   }
   else if( pState->pDumpFunc )
   {
      ( pState->pDumpFunc )( pState->cargo,
                             hb_membufPtr( pState->pDumpBuffer ),
                             ( ULONG ) hb_membufLen( pState->pDumpBuffer ),
                             pState->iDumpLine );

      /* I do not like it - dump data should be separated from
         preprocessed .prg code. What is inside DUMP area and
         how it will be interpreted depends on backend not on
         PP itself */
      if( pState->fWritePreprocesed )
      {
         //int iLines = 0;
         char *   pBuffer;
         HB_SIZE  ulLen;

         if( pState->pFile->fGenLineInfo )
         {
            fprintf( pState->file_out, "#line %d", pState->iDumpLine );
            if( pState->pFile->szFileName )
            {
               fprintf( pState->file_out, " \"%s\"", pState->pFile->szFileName );
            }
            fputc( '\n', pState->file_out );
            pState->pFile->fGenLineInfo = FALSE;
         }
         else if( pState->pFile->iLastLine < pState->iDumpLine )
         {
            int i = pState->pFile->iLastLine;

            do
            {
               fputc( '\n', pState->file_out );
            }
            while( ++i < pState->iDumpLine );
         }
         pBuffer  = hb_membufPtr( pState->pDumpBuffer );
         ulLen    = hb_membufLen( pState->pDumpBuffer );
         fputs( "#pragma BEGINDUMP\n", pState->file_out );
         fwrite( pBuffer, sizeof( char ), ( size_t ) ulLen, pState->file_out );
         fputs( "#pragma ENDDUMP\n", pState->file_out );

         //while( ulLen-- )
         //{
         //   if( *pBuffer++ == '\n' )
         //       ++iLines ;
         //}
         //pState->pFile->iLastLine = pState->iDumpLine + iLines + 2;
      }
      hb_membufFlush( pState->pDumpBuffer );
   }
}

static void hb_pp_getLine( PHB_PP_STATE pState )
{
   PHB_PP_TOKEN * pInLinePtr;
   char *         pBuffer, ch;
   HB_SIZE        ulLen, ul;
   BOOL           fDump    = FALSE;
   int            iLines   = 0;

   pInLinePtr              = NULL;
   hb_pp_tokenListFree( &pState->pFile->pTokenList );
   pState->pNextTokenPtr   = &pState->pFile->pTokenList;
   pState->pFile->iTokens  = pState->iSpaces = pState->iSpacesMin = 0;
   pState->fCanNextLine    = pState->fDirective = FALSE;
   pState->fNewStatement   = TRUE;
   pState->iLastType       = HB_PP_TOKEN_NUL;
   pState->iInLineState    = HB_PP_INLINE_OFF;
   pState->iInLineBraces   = 0;
   pState->iBlockState     = pState->iNestedBlock = 0;

   do
   {
      hb_membufFlush( pState->pBuffer );
      hb_pp_readLine( pState );
      pBuffer  = hb_membufPtr( pState->pBuffer );
      ulLen    = hb_membufLen( pState->pBuffer );
      if( pState->fCanNextLine )
      {
         pState->iSpaces      = pState->iSpacesNL;
         /*
          * set minimum number of leading spaces to 1 to avoid problems
          * with automatic word concatenation which is not Clipper compatible
          */
         pState->iSpacesMin   = 1;
         pState->fCanNextLine = FALSE;
         /* Clipper left only last leading blank character from
            concatenated lines */
         if( ulLen > 1 && HB_PP_ISBLANK( pBuffer[ 0 ] ) )
         {
            while( ulLen > 1 && HB_PP_ISBLANK( pBuffer[ 1 ] ) )
            {
               --ulLen;
               ++pBuffer;
            }
         }
      }
      else if( pState->iStreamDump && ulLen == 0 )
      {
         pBuffer[ 0 ]   = '\0';
         fDump          = TRUE;
      }
      ul = 0;
      while( ul < ulLen || fDump )
      {
         ch = pBuffer[ 0 ];
         if( pState->iStreamDump )
         {
            fDump = FALSE;
            if( pState->iStreamDump == HB_PP_STREAM_COMMENT )
            {
               if( ulLen > 0 )
               {
                  ++ul;
                  if( ulLen > 1 && ch == '*' && pBuffer[ 1 ] == '/' )
                  {
                     pState->iStreamDump  = HB_PP_STREAM_OFF;
                     /* Clipper clear number of leading spaces when multiline
                        comment ends */
                     pState->iSpaces      = 0;
                     /*
                      * but we cannot make the same because we have automatic
                      * word concatenation which is not Clipper compatible and
                      * will break code like:
                      */
#if 0
                     //   if /**/lVar; endif
#endif
                     pState->iSpacesMin = 1;
                     ++ul;
                  }
               }
            }
            else if( pState->iStreamDump == HB_PP_STREAM_INLINE_C )
            {
               if( ulLen > 0 )
               {
                  ++ul;
                  switch( pState->iInLineState )
                  {
                     case HB_PP_INLINE_QUOTE1:
                        if( ch == '\'' )
                           pState->iInLineState = HB_PP_INLINE_OFF;
                        else if( ch == '\\' && ulLen > 1 )
                           ++ul;
                        break;

                     case HB_PP_INLINE_QUOTE2:
                        if( ch == '"' )
                           pState->iInLineState = HB_PP_INLINE_OFF;
                        else if( ch == '\\' && ulLen > 1 )
                           ++ul;
                        break;

                     case HB_PP_INLINE_COMMENT:
                        if( ulLen > 1 && ch == '*' && pBuffer[ 1 ] == '/' )
                        {
                           pState->iInLineState = HB_PP_INLINE_OFF;
                           ++ul;
                        }
                        break;

                     default:
                        if( ch == '\'' )
                           pState->iInLineState = HB_PP_INLINE_QUOTE1;
                        else if( ch == '"' )
                           pState->iInLineState = HB_PP_INLINE_QUOTE2;
                        else if( ch == '{' )
                           ++pState->iInLineBraces;
                        else if( ch == '}' )
                        {
                           if( --pState->iInLineBraces == 0 )
                              pState->iStreamDump = HB_PP_STREAM_OFF;
                        }
                        else if( ulLen > 1 )
                        {
                           if( ch == '/' && pBuffer[ 1 ] == '*' )
                           {
                              pState->iInLineState = HB_PP_INLINE_COMMENT;
                              ++ul;
                           }
                           else if( ch == '/' && pBuffer[ 1 ] == '/' )
                              ulLen = ul = 0;
                        }
                  }
               }
               if( ul )
                  hb_membufAddData( pState->pStreamBuffer, pBuffer, ul );

               if( ulLen == ul || pState->iStreamDump == HB_PP_STREAM_OFF )
               {
                  hb_membufAddCh( pState->pStreamBuffer, '\n' );
                  if( pState->iStreamDump == HB_PP_STREAM_OFF )
                  {
                     if( pState->iCondCompile )
                     {
                        ;
                     }
                     else if( pState->pInLineFunc )
                     {
                        char szFunc[ 24 ];
                        hb_snprintf( szFunc, sizeof( szFunc ), "HB_INLINE_%03d", ++pState->iInLineCount );
                        if( pInLinePtr && *pInLinePtr )
                           hb_pp_tokenSetValue( *pInLinePtr, szFunc, strlen( szFunc ) );
                        pState->pInLineFunc( pState->cargo, szFunc,
                                             hb_membufPtr( pState->pStreamBuffer ),
                                             ( ULONG ) hb_membufLen( pState->pStreamBuffer ),
                                             pState->iDumpLine );
                     }
                     else
                     {
                        hb_pp_tokenAddNext( pState,
                                            hb_membufPtr( pState->pStreamBuffer ),
                                            hb_membufLen( pState->pStreamBuffer ),
                                            HB_PP_TOKEN_TEXT );
                     }
                     hb_membufFlush( pState->pStreamBuffer );
                  }
               }
            }
            else if( pState->iStreamDump == HB_PP_STREAM_DUMP_C )
            {
               if( hb_pp_hasCommand( pBuffer, ulLen, &ul, 3, "#", "pragma", "enddump" ) )
               {
                  hb_pp_dumpEnd( pState );
               }
               else
               {
                  ul = ulLen;
                  hb_membufAddData( pState->pDumpBuffer, pBuffer, ul );
                  hb_membufAddCh( pState->pDumpBuffer, '\n' );
               }
            }
            else if( hb_pp_hasCommand( pBuffer, ulLen, &ul, 1, "ENDTEXT" ) ||
                     hb_pp_hasCommand( pBuffer, ulLen, &ul, 3, "#", "pragma", "__endtext" ) )
            {
               if( pState->iStreamDump == HB_PP_STREAM_CLIPPER )
               {
                  if( pState->pFuncEnd )
                     hb_pp_tokenAddStreamFunc( pState, pState->pFuncEnd, NULL, 0 );
               }
               else
               {
                  /* HB_PP_STREAM_PRG, HB_PP_STREAM_C */
                  hb_pp_tokenAddStreamFunc( pState, pState->pFuncOut,
                                            hb_membufPtr( pState->pStreamBuffer ),
                                            hb_membufLen( pState->pStreamBuffer ) );
                  if( pState->pFuncEnd )
                  {
                     if( pState->pFuncOut )
                        hb_pp_tokenAddCmdSep( pState );
                     hb_pp_tokenAddStreamFunc( pState, pState->pFuncEnd,
                                               hb_membufPtr( pState->pStreamBuffer ),
                                               hb_membufLen( pState->pStreamBuffer ) );
                  }
                  hb_membufFlush( pState->pStreamBuffer );
               }
               hb_pp_tokenListFree( &pState->pFuncOut );
               hb_pp_tokenListFree( &pState->pFuncEnd );
               pState->iStreamDump = HB_PP_STREAM_OFF;
            }
            else if( pState->iStreamDump == HB_PP_STREAM_CLIPPER )
            {
               ul = ulLen;
               hb_pp_tokenAddStreamFunc( pState, pState->pFuncOut, pBuffer, ul );
            }
            else /* HB_PP_STREAM_PRG, HB_PP_STREAM_C */
            {
               ul = ulLen;
               if( pState->iStreamDump == HB_PP_STREAM_C )
                  hb_strRemEscSeq( pBuffer, &ul );
               hb_membufAddData( pState->pStreamBuffer, pBuffer, ul );
               hb_membufAddCh( pState->pStreamBuffer, '\n' );
               ul = ulLen; /* hb_strRemEscSeq() above could change ul */
            }
         }
#ifndef HB_C52_STRICT
         else if( ( ( ch == 'e' || ch == 'E' ) && ulLen > 1 &&
                    pBuffer[ 1 ] == '"' ) || ( ch == '"' && pState->fEscStr ) )
         {
            HB_SIZE ulStrip, u;

            if( ch != '"' )
               ++ul;
            while( ++ul < ulLen && pBuffer[ ul ] != '"' )
            {
               if( pBuffer[ ul ] == '\\' )
               {
                  if( ++ul == ulLen )
                     break;
               }
            }
#ifdef HB_PP_MULTILINE_STRING
            while( ul == ulLen )
            {
               u = 1;
               while( ul > u && pBuffer[ ul - u ] == ' ' )
                  ++u;
               if( ul >= u && pBuffer[ ul - u ] == ';' )
               {
                  ul       -= u;
                  ulLen    -= u;
                  u        = hb_membufLen( pState->pBuffer ) - u;
                  hb_membufRemove( pState->pBuffer, u );
                  hb_pp_readLine( pState );
                  ulLen    += hb_membufLen( pState->pBuffer ) - u;
                  pBuffer  = hb_membufPtr( pState->pBuffer ) + u - ul;
                  --ul;
                  while( ++ul < ulLen && pBuffer[ ul ] != '"' )
                  {
                     if( pBuffer[ ul ] == '\\' )
                     {
                        if( ++ul == ulLen )
                           break;
                     }
                  }
               }
               else
                  break;
            }
#endif
            u        = ch != '"' ? 2 : 1;
            ulStrip  = ul - u;
            hb_strRemEscSeq( pBuffer + u, &ulStrip );
            hb_pp_tokenAddNext( pState, pBuffer + u, ulStrip,
                                HB_PP_TOKEN_STRING );
            if( ul == ulLen )
            {
               HB_SIZE ulSkip = pBuffer - hb_membufPtr( pState->pBuffer );
               hb_membufAddCh( pState->pBuffer, '\0' );
               pBuffer = hb_membufPtr( pState->pBuffer ) + ulSkip;
               hb_pp_error( pState, 'E', HB_PP_ERR_STRING_TERMINATOR, pBuffer + u - 1 );
            }
            else
               ++ul;
         }
#endif
         else if( ch == '"' || ch == '\'' || ch == '`' )
         {
            if( ch == '`' )
               ch = '\'';
            while( ++ul < ulLen && pBuffer[ ul ] != ch )
               ;
#ifdef HB_PP_MULTILINE_STRING
            while( ul == ulLen )
            {
               ULONG u = 1;
               while( ul > u && pBuffer[ ul - u ] == ' ' )
                  ++u;
               if( ul >= u && pBuffer[ ul - u ] == ';' )
               {
                  ul       -= u;
                  ulLen    -= u;
                  u        = hb_membufLen( pState->pBuffer ) - u;
                  hb_membufRemove( pState->pBuffer, u );
                  hb_pp_readLine( pState );
                  ulLen    += hb_membufLen( pState->pBuffer ) - u;
                  pBuffer  = hb_membufPtr( pState->pBuffer ) + u - ul;
                  --ul;
                  while( ++ul < ulLen && pBuffer[ ul ] != ch )
                     ;
               }
               else
               {
                  ul = ulLen;
                  break;
               }
            }
#endif
            hb_pp_tokenAddNext( pState, pBuffer + 1, ul - 1,
                                HB_PP_TOKEN_STRING );

            if( ul == ulLen )
            {
               HB_SIZE ulSkip = pBuffer - hb_membufPtr( pState->pBuffer ) + 1;
               hb_membufAddCh( pState->pBuffer, '\0' );
               pBuffer = hb_membufPtr( pState->pBuffer ) + ulSkip;
               hb_pp_error( pState, 'E', HB_PP_ERR_STRING_TERMINATOR, pBuffer );
            }
            else
               ++ul;
         }
         else if( ch == '[' && ! pState->fDirective &&
                  hb_pp_canQuote( pState->fCanNextLine ||
                                  HB_PP_TOKEN_CANQUOTE( pState->iLastType ),
                                  pBuffer, ulLen, 1, &ul ) )
         {
            hb_pp_tokenAddNext( pState, pBuffer + 1, ul - 1, HB_PP_TOKEN_STRING );
            ++ul;
         }
         else if( ( ch == '/' || ch == '&' ) && ulLen > 1 && pBuffer[ 1 ] == ch )
         {
            /* strip the rest of line with // or && comment */
            ul = ulLen;
         }
         else if( ch == '*' && pState->pFile->iTokens == 0 )
         {
            /* strip the rest of line with // or && comment */
            ul = ulLen;
         }
         else if( ch == '/' && ulLen > 1 && pBuffer[ 1 ] == '*' )
         {
#ifdef HB_C52_STRICT
            /* In Clipper multiline comments used after ';' flushes
               the EOC token what causes that ';' is always command
               separator and cannot be used as line concatenator just
               before multiline comments */
            if( pState->fCanNextLine )
               hb_pp_tokenAddCmdSep( pState );
#endif
            pState->iStreamDump  = HB_PP_STREAM_COMMENT;
            ul                   += 2;
         }
         else if( ch == ' ' || ch == '\t' )
         {
            do
            {
               if( pBuffer[ ul ] == ' ' )
                  pState->iSpaces++;
               else if( pBuffer[ ul ] == '\t' )
                  pState->iSpaces += 4;
               else
                  break;
            }
            while( ++ul < ulLen );
         }
         else if( ch == ';' )
         {
            if( pState->fCanNextLine )
               hb_pp_tokenAddCmdSep( pState );
            pState->fCanNextLine = TRUE;
            pState->iSpacesNL    = pState->iSpaces;
            pState->iSpaces      = 0;
            ++ul;
         }
         else if( HB_PP_ISFIRSTIDCHAR( ch ) )
         {
            while( ++ul < ulLen && HB_PP_ISNEXTIDCHAR( pBuffer[ ul ] ) )
               ;

            /*
             * In Clipper note can be used only as 1-st token and after
             * statement separator ';' it does not work like a single line
             * comment.
             */
#ifdef HB_C52_STRICT
            if( pState->pFile->iTokens == 0 &&
#else
            if( pState->fNewStatement &&
#endif
                ul == 4 && hb_strnicmp( "NOTE", pBuffer, 4 ) == 0 )
            {
               /* strip the rest of line */
               ul = ulLen;
            }
            else
            {
               if( ul < ulLen && pBuffer[ ul ] == '&' )
               {
                  /*
                   * [<keyword>][&<keyword>[.[<nextidchars>]]]+ is a single
                   * token in Clipper and this fact is important in later
                   * preprocessing so we have to replicate it
                   */
                  while( ulLen - ul > 1 && pBuffer[ ul ] == '&' &&
                         HB_PP_ISFIRSTIDCHAR( pBuffer[ ul + 1 ] ) )
                  {
                     while( ++ul < ulLen && HB_PP_ISNEXTIDCHAR( pBuffer[ ul ] ) )
                        ;
                     if( ul < ulLen && pBuffer[ ul ] == '.' )
                        while( ++ul < ulLen && HB_PP_ISNEXTIDCHAR( pBuffer[ ul ] ) )
                           ;
                  }
                  if( ul < ulLen && pBuffer[ ul ] == '&' )
                     ++ul;
                  hb_pp_tokenAddNext( pState, pBuffer, ul, HB_PP_TOKEN_MACROTEXT );
               }
               else if( pState->pInLineFunc &&
                        pState->iInLineState == HB_PP_INLINE_OFF &&
                        ul == 9 && hb_strnicmp( "hb_inline", pBuffer, 9 ) == 0 )
               {
                  if( pState->fCanNextLine )
                     hb_pp_tokenAddCmdSep( pState );
                  pInLinePtr              = pState->pNextTokenPtr;
                  hb_pp_tokenAddNext( pState, pBuffer, ul, HB_PP_TOKEN_KEYWORD );
                  pState->iInLineState    = HB_PP_INLINE_START;
                  pState->iInLineBraces   = 0;
               }
               else
                  hb_pp_tokenAddNext( pState, pBuffer, ul, HB_PP_TOKEN_KEYWORD );
            }
         }
         /* This is Clipper incompatible token - such characters are illegal
            and error message generated, to replicate this behavior is enough
            to change HB_PP_ISILLEGAL() macro */
         else if( HB_PP_ISTEXTCHAR( ch ) )
         {
            while( ++ul < ulLen && HB_PP_ISTEXTCHAR( pBuffer[ ul ] ) )
               ;

            hb_pp_tokenAddNext( pState, pBuffer, ul, HB_PP_TOKEN_TEXT );
         }
         else if( HB_PP_ISILLEGAL( ch ) )
         {
            char szCh[ 3 ];

            hb_pp_tokenAddNext( pState, pBuffer, ++ul, HB_PP_TOKEN_NUL );
            hb_snprintf( szCh, sizeof( szCh ), "%02x", ch & 0xff );
            hb_pp_error( pState, 'E', HB_PP_ERR_ILLEGAL_CHAR, szCh );
         }
         else if( HB_PP_ISDIGIT( ch ) )
         {
            if( ulLen >= 3 && pBuffer[ 0 ] == '0' &&
                ( pBuffer[ 1 ] == 'x' || pBuffer[ 1 ] == 'X' ) &&
                HB_PP_ISHEX( pBuffer[ 2 ] ) )
            {
               ul = 2;
               while( ++ul < ulLen && HB_PP_ISHEX( pBuffer[ ul ] ) )
                  ;

               /* (LEX: mark token as hex?) */
               hb_pp_tokenAddNext( pState, pBuffer, ul, HB_PP_TOKEN_NUMBER );
            }
            else if( ulLen >= 3 && pBuffer[ 0 ] == '0' &&
                     ( pBuffer[ 1 ] == 'd' || pBuffer[ 1 ] == 'D' ) &&
                     HB_PP_ISDIGIT( pBuffer[ 2 ] ) )
            {
               ul = 2;
               while( ++ul < ulLen && HB_PP_ISDIGIT( pBuffer[ ul ] ) )
                  ;

               hb_pp_tokenAddNext( pState, pBuffer, ul, HB_PP_TOKEN_DATE );
            }
            else
            {
               while( ++ul < ulLen && HB_PP_ISDIGIT( pBuffer[ ul ] ) )
                  ;
               if( ulLen - ul > 1 && pBuffer[ ul ] == '.' &&
                   HB_PP_ISDIGIT( pBuffer[ ul + 1 ] ) )
               {
                  ++ul;
                  while( ++ul < ulLen && HB_PP_ISDIGIT( pBuffer[ ul ] ) )
                     ;
               }
               hb_pp_tokenAddNext( pState, pBuffer, ul, HB_PP_TOKEN_NUMBER );
            }
         }
         else if( ch == '.' && ulLen > 1 && HB_PP_ISDIGIT( pBuffer[ 1 ] ) )
         {
            while( ++ul < ulLen && HB_PP_ISDIGIT( pBuffer[ ul ] ) )
               ;

            hb_pp_tokenAddNext( pState, pBuffer, ul, HB_PP_TOKEN_NUMBER );
         }
         else if( ch == '.' && ulLen >= 3 && pBuffer[ 2 ] == '.' &&
                  ( HB_PP_ISTRUE( pBuffer[ 1 ] ) || HB_PP_ISFALSE( pBuffer[ 1 ] ) ) )
         {
            const char * value = HB_PP_ISTRUE( pBuffer[ 1 ] ) ? ".T." : ".F.";

            ul = 3;
            hb_pp_tokenAddNext( pState, value, ul, HB_PP_TOKEN_LOGICAL | HB_PP_TOKEN_STATIC );
         }
         else if( ch == '&' && ulLen > 1 && HB_PP_ISFIRSTIDCHAR( pBuffer[ 1 ] ) )
         {
            int iParts = 0;
            /*
             * [<keyword>][&<keyword>[.[<nextidchars>]]]+ is a single token in Clipper
             * and this fact is important in later preprocessing so we have
             * to replicate it
             */
            while( ulLen - ul > 1 && pBuffer[ ul ] == '&' &&
                   HB_PP_ISFIRSTIDCHAR( pBuffer[ ul + 1 ] ) )
            {
               ++iParts;
               while( ++ul < ulLen && HB_PP_ISNEXTIDCHAR( pBuffer[ ul ] ) )
                  ;
               if( ul < ulLen && pBuffer[ ul ] == '.' )
                  while( ++ul < ulLen && HB_PP_ISNEXTIDCHAR( pBuffer[ ul ] ) )
                     ++iParts;
            }
            if( ul < ulLen && pBuffer[ ul ] == '&' )
            {
               ++iParts;
               ++ul;
            }
            hb_pp_tokenAddNext( pState, pBuffer, ul, iParts == 1 ?
                                HB_PP_TOKEN_MACROVAR : HB_PP_TOKEN_MACROTEXT );
         }
         else if( ch == '{' && ! pState->fCanNextLine &&
                  ( pState->iInLineState == HB_PP_INLINE_BODY ||
                    pState->iInLineState == HB_PP_INLINE_START ) )
         {
            if( pState->iInLineState == HB_PP_INLINE_START )
            {
               hb_pp_tokenAddNext( pState, "(", 1, HB_PP_TOKEN_LEFT_PB | HB_PP_TOKEN_STATIC );
               hb_pp_tokenAddNext( pState, ")", 1, HB_PP_TOKEN_RIGHT_PB | HB_PP_TOKEN_STATIC );
            }
            pState->iInLineState = HB_PP_INLINE_OFF;
            pState->iStreamDump  = HB_PP_STREAM_INLINE_C;
            pState->iDumpLine    = pState->pFile->iCurrentLine - 1;
            if( pState->pStreamBuffer )
               hb_membufFlush( pState->pStreamBuffer );
            else
               pState->pStreamBuffer = hb_membufNew();
         }
         else
         {
            PHB_PP_OPERATOR pOperator = hb_pp_operatorFind( pState, pBuffer, ulLen );

            if( pOperator )
            {
               hb_pp_tokenAddNext( pState, pOperator->value,
                                   strlen( pOperator->value ),
                                   pOperator->type );
               ul = pOperator->len;
            }
            else
            {
               hb_pp_tokenAddNext( pState, pBuffer, ++ul, HB_PP_TOKEN_OTHER );
            }
         }
         pBuffer  += ul;
         ulLen    -= ul;
         ul       = 0;
      }

      if( ! pState->fCanNextLine &&
          ( pState->iNestedBlock || pState->iBlockState == 5 ) )
      {
         iLines++;
         hb_pp_tokenAdd( &pState->pNextTokenPtr, "\n", 1, 0, HB_PP_TOKEN_EOL | HB_PP_TOKEN_STATIC | HB_PP_TOKEN_LINECOUNTER );
         pState->fDirective      = FALSE;
         pState->iSpaces         = pState->iSpacesMin = 0;
         pState->pFile->iTokens++;
         pState->fNewStatement   = TRUE;
         if( pState->iBlockState )
         {
            if( pState->iBlockState == 5 )
               pState->iNestedBlock++;
            pState->iBlockState = 0;
         }
      }
   }
   while( ( pState->pFile->pLineBuf ? pState->pFile->ulLineBufLen != 0 :
            ! pState->pFile->fEof ) &&
          ( pState->fCanNextLine || pState->iNestedBlock ||
            ( pState->iStreamDump && pState->iStreamDump != HB_PP_STREAM_CLIPPER ) ) );

   if( pState->iStreamDump )
   {
      if( pState->iStreamDump == HB_PP_STREAM_COMMENT )
         hb_pp_error( pState, 'E', HB_PP_ERR_UNTERMINATED_COMMENT, NULL );
      else if( pState->iStreamDump == HB_PP_STREAM_DUMP_C )
         hb_pp_dumpEnd( pState );
      else if( pState->pFile->pLineBuf ? ! pState->pFile->ulLineBufLen :
               pState->pFile->fEof )
         hb_pp_error( pState, 'E', HB_PP_ERR_MISSING_ENDTEXT, NULL );
   }

   if( pState->pFile->iTokens != 0 )
   {
      hb_pp_tokenAdd( &pState->pNextTokenPtr, "\n", 1, 0, HB_PP_TOKEN_EOL | HB_PP_TOKEN_STATIC );
      pState->pFile->iTokens++;
   }
   pState->pFile->iCurrentLine -= iLines;
}

static int hb_pp_tokenStr( PHB_PP_TOKEN pToken, PHB_MEM_BUFFER pBuffer,
                           BOOL fSpaces, BOOL fQuote, int ltype )
{
   int iLines = 0, iSpace = fSpaces ? pToken->spaces : 0;

   /* This is workaround for stringify token list and later decoding by FLEX
      which breaks Clipper compatible code */
   if( iSpace == 0 && fQuote && ltype &&
       ltype >= HB_PP_TOKEN_ASSIGN && ltype != HB_PP_TOKEN_EQ &&
       HB_PP_TOKEN_TYPE( pToken->type ) >= HB_PP_TOKEN_ASSIGN &&
       HB_PP_TOKEN_TYPE( pToken->type ) != HB_PP_TOKEN_EQ )
      iSpace = 1;
#ifdef __XHARBOUR__
   /* This is a workaround for when [...] is reverted into a literal string
      after it's been tokenized like array, f.e.
         @ 0,0 SAY [---] */
   else if( iSpace == 0xFFFF )
      iSpace = 0;
#endif

   if( iSpace > 0 )
   {
      do
      {
         hb_membufAddCh( pBuffer, ' ' );
      }
      while( --iSpace );
   }

   if( HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_STRING )
   {
      int   iq = 7, i;
      char  ch;

      for( i = 0; iq && i < pToken->len; ++i )
      {
         switch( pToken->value[ i ] )
         {
            case '"':
               iq &= ~1;
               break;
            case '\'':
               iq &= ~2;
               break;
            case ']':
               iq &= ~4;
               break;
            case '\n':
            case '\r':
            case '\0':
               iq = 0;
               break;
         }
      }
      if( iq == 0 && fQuote )
      {
         /* generate string with 'e' prefix before opening '"' and quote
            control characters inside, f.e.:
               e"line1\nline2"
          */

         hb_membufAddCh( pBuffer, 'e' );
         hb_membufAddCh( pBuffer, '"' );
         for( i = 0; i < pToken->len; ++i )
         {
            ch = pToken->value[ i ];
            switch( ch )
            {
               case '\r':
                  iq = ch = 'r';
                  break;
               case '\n':
                  iq = ch = 'n';
                  break;
               case '\t':
                  iq = ch = 't';
                  break;
               case '\b':
                  iq = ch = 'b';
                  break;
               case '\0':
                  iq = ch = '0';
                  break;
               case '"':
               case '\\':
                  iq = 1;
                  break;
               default:
                  iq = 0;
                  break;
            }
            if( iq )
               hb_membufAddCh( pBuffer, '\\' );
            hb_membufAddCh( pBuffer, ch );
         }
         hb_membufAddCh( pBuffer, '"' );
      }
      else
      {
         if( iq & 1 )
            ch = '"';
         else if( iq & 2 )
            ch = '\'';
         else
            ch = '[';

         hb_membufAddCh( pBuffer, ch );
         hb_membufAddData( pBuffer, pToken->value, pToken->len );
         hb_membufAddCh( pBuffer, ch == '[' ? ']' : ch );
      }
   }
   else
   {
      if( HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_EOL )
         ++iLines;
      hb_membufAddData( pBuffer, pToken->value, pToken->len );
   }

   return iLines;
}

static BOOL hb_pp_tokenValueCmp( PHB_PP_TOKEN pToken, char * szValue, USHORT mode )
{
   if( pToken->len )
   {
      if( HB_PP_CMP_MODE( mode ) == HB_PP_CMP_CASE )
         return memcmp( szValue, pToken->value, pToken->len ) == 0;
      if( HB_PP_CMP_MODE( mode ) == HB_PP_CMP_DBASE && pToken->len >= 4 &&
          ( HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_KEYWORD ||
            HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_STRING ||
            HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_TEXT ) )
         return hb_strnicmp( szValue, pToken->value, pToken->len < 4 ? 4 : pToken->len ) == 0;
      else
         return hb_stricmp( szValue, pToken->value ) == 0;
   }
   return FALSE;
}

static BOOL hb_pp_tokenEqual( PHB_PP_TOKEN pToken, PHB_PP_TOKEN pMatch,
                              USHORT mode )
{
   return pToken == pMatch ||
          ( mode != HB_PP_CMP_ADDR &&
            HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_TYPE( pMatch->type ) &&
            ( pToken->len == pMatch->len ||
             ( HB_PP_CMP_MODE( mode ) == HB_PP_CMP_DBASE && pMatch->len > 4 &&
               pToken->len >= 4 && pMatch->len > pToken->len ) ) &&
            hb_pp_tokenValueCmp( pToken, pMatch->value, mode ) );
}

static void hb_pp_patternClearResults( PHB_PP_RULE pRule )
{
   PHB_PP_MARKER  pMarker  = pRule->pMarkers;
   int            i        = pRule->markers;

   while( --i >= 0 )
   {
      pMarker->matches = 0;
      while( pMarker->pResult )
      {
         PHB_PP_RESULT pResult = pMarker->pResult;
         pMarker->pResult = pResult->pNext;
         hb_xfree( pResult );
      }
      ++pMarker;
   }
   pRule->pNextExpr = NULL;
}

static BOOL hb_pp_patternAddResult( PHB_PP_RULE pRule, USHORT marker,
                                    PHB_PP_TOKEN pFirst, PHB_PP_TOKEN pNext )
{
   PHB_PP_MARKER pMarker = &pRule->pMarkers[ marker - 1 ];

   if( pMarker->matches == 0 || pMarker->canrepeat )
   {
      PHB_PP_RESULT * pResultPtr,
                      pResult = ( PHB_PP_RESULT ) hb_xgrab( sizeof( HB_PP_RESULT ) );
      pMarker->matches++;
      pResult->pFirstToken = pFirst;
      pResult->pNextExpr   = pNext;
      pResult->pNext       = NULL;
      pResultPtr           = &pMarker->pResult;
      while( *pResultPtr )
         pResultPtr = &( *pResultPtr )->pNext;
      *pResultPtr          = pResult;
      return TRUE;
   }

   return FALSE;
}

static PHB_PP_RULE hb_pp_ruleNew( PHB_PP_TOKEN pMatch, PHB_PP_TOKEN pResult,
                                  USHORT mode, USHORT markers,
                                  PHB_PP_MARKER pMarkers )
{
   PHB_PP_RULE pRule = ( PHB_PP_RULE ) hb_xgrab( sizeof( HB_PP_RULE ) );

   pRule->pPrev      = NULL;
   pRule->mode       = mode;
   pRule->pMatch     = pMatch;
   pRule->pResult    = pResult;
   pRule->markers    = markers;
   pRule->pMarkers   = pMarkers;
   pRule->pNextExpr  = NULL;

   return pRule;
}

static void hb_pp_ruleFree( PHB_PP_RULE pRule )
{
   hb_pp_tokenListFree( &pRule->pMatch );
   hb_pp_tokenListFree( &pRule->pResult );
   hb_pp_patternClearResults( pRule );
   if( pRule->pMarkers )
      hb_xfree( pRule->pMarkers );
   hb_xfree( pRule );
}

static void hb_pp_ruleListFree( PHB_PP_RULE * pRulePtr )
{
   PHB_PP_RULE pRule;

   while( *pRulePtr )
   {
      pRule       = *pRulePtr;
      *pRulePtr   = pRule->pPrev;
      hb_pp_ruleFree( pRule );
   }
}

static void hb_pp_ruleListNonStdFree( PHB_PP_RULE * pRulePtr )
{
   PHB_PP_RULE pRule;

   while( *pRulePtr )
   {
      pRule = *pRulePtr;
      if( ( pRule->mode & HB_PP_STD_RULE ) != 0 )
      {
         pRulePtr = &pRule->pPrev;
      }
      else
      {
         *pRulePtr = pRule->pPrev;
         hb_pp_ruleFree( pRule );
      }
   }
}

static void hb_pp_ruleListSetStd( PHB_PP_RULE pRule )
{
   while( pRule )
   {
      pRule->mode |= HB_PP_STD_RULE;
      pRule       = pRule->pPrev;
   }
}

static void hb_pp_ruleSetId( PHB_PP_STATE pState, PHB_PP_TOKEN pMatch, BYTE id )
{
   if( HB_PP_TOKEN_ISMATCH( pMatch ) )
   {
      int i;
      for( i = 0; i < HB_PP_HASHID_MAX; ++i )
         pState->pMap[ i ] |= id;
   }
   else
      pState->pMap[ HB_PP_HASHID( pMatch ) ] |= id;
}

static void hb_pp_ruleListSetId( PHB_PP_STATE pState, PHB_PP_RULE pRule, BYTE id )
{
   while( pRule )
   {
      hb_pp_ruleSetId( pState, pRule->pMatch, id );
      if( HB_PP_TOKEN_ISMATCH( pRule->pMatch ) )
         break;
      pRule = pRule->pPrev;
   }
}

static PHB_PP_RULE hb_pp_defineFind( PHB_PP_STATE pState, PHB_PP_TOKEN pToken, PHB_PP_RULE pFirstRule )
{
   PHB_PP_RULE pRule = pFirstRule == NULL ? pState->pDefinitions : pFirstRule;

   /* TODO% create binary tree or hash table - the #define keyword token has
            to be unique so it's not necessary to keep the stack list,
            it will increase the speed when there is a lot of #define values */

   while( pRule && ! hb_pp_tokenEqual( pToken, pRule->pMatch, HB_PP_CMP_CASE ) )
      pRule = pRule->pPrev;

   return pRule;
}

static void hb_pp_defineAdd( PHB_PP_STATE pState, USHORT mode,
                             USHORT markers, PHB_PP_MARKER pMarkers,
                             PHB_PP_TOKEN pMatch, PHB_PP_TOKEN pResult )
{
   PHB_PP_RULE pRule = hb_pp_defineFind( pState, pMatch, NULL );

#ifdef HB_C52_STRICT
   if( pRule )
#else
   if( pRule && pMarkers == NULL )
#endif
   {
      hb_pp_tokenListFree( &pRule->pMatch );
      hb_pp_tokenListFree( &pRule->pResult );
      hb_pp_patternClearResults( pRule );
      if( pRule->pMarkers )
         hb_xfree( pRule->pMarkers );
      pRule->pMatch     = pMatch;
      pRule->pResult    = pResult;
      pRule->pMarkers   = pMarkers;
      pRule->markers    = markers;
      pRule->mode       = mode;
      hb_pp_error( pState, 'W', HB_PP_WARN_DEFINE_REDEF, pMatch->value );
   }
#ifndef HB_C52_STRICT
   else if( pRule )
   {
      pRule                = hb_pp_ruleNew( pMatch, pResult, mode, markers, pMarkers );
      pRule->pPrev         = pState->pDefinitions;
      pState->pDefinitions = pRule;
      pState->iDefinitions++;
      hb_pp_error( pState, 'W', HB_PP_WARN_OVERLOADED_REDEF, pMatch->value );
   }
#endif
   else
   {
      pRule                = hb_pp_ruleNew( pMatch, pResult, mode, markers, pMarkers );
      pRule->pPrev         = pState->pDefinitions;
      pState->pDefinitions = pRule;
      pState->iDefinitions++;
   }
   hb_pp_ruleSetId( pState, pMatch, HB_PP_DEFINE );
}

static void hb_pp_defineDel( PHB_PP_STATE pState, PHB_PP_TOKEN pToken )
{
   PHB_PP_RULE * pRulePtr = &pState->pDefinitions, pRule;

   while( *pRulePtr )
   {
      pRule = *pRulePtr;
      if( hb_pp_tokenEqual( pToken, pRule->pMatch, HB_PP_CMP_CASE ) )
      {
         *pRulePtr = pRule->pPrev;
         hb_pp_ruleFree( pRule );
         pState->iDefinitions--;
         return;
      }
      pRulePtr = &pRule->pPrev;
   }
}

static PHB_PP_FILE hb_pp_FileNew( PHB_PP_STATE pState, char * szFileName,
                                  BOOL fSysFile, BOOL * pfNested, FILE * file_in,
                                  BOOL fSearchPath, PHB_PP_OPEN_FUNC pOpenFunc )
{
   char        szFileNameBuf[ HB_PATH_MAX ];
   PHB_PP_FILE pFile;

   if( ! file_in )
   {
      if( pOpenFunc )
      {
         file_in     = ( pOpenFunc ) ( pState->cargo, szFileName, fSysFile,
                                       pfNested, szFileNameBuf );
         szFileName  = szFileNameBuf;
      }
      else
      {
         PHB_FNAME   pFileName   = hb_fsFNameSplit( szFileName );
         BOOL        fNested     = FALSE;

         pFileName->szName       = szFileName;
         pFileName->szExtension  = NULL;
         if( ! fSysFile )
         {
            if( ! pFileName->szPath || ! pFileName->szPath[ 0 ] )
            {
               char * szFirstFName = NULL;
               pFile = pState->pFile;
               while( pFile )
               {
                  if( pFile->szFileName )
                     szFirstFName = pFile->szFileName;
                  pFile = pFile->pPrev;
               }
               if( szFirstFName )
               {
                  PHB_FNAME pFirstFName = hb_fsFNameSplit( szFirstFName );
                  pFileName->szPath = pFirstFName->szPath;
                  hb_fsFNameMerge( szFileNameBuf, pFileName );
                  szFileName        = szFileNameBuf;
                  hb_xfree( pFirstFName );
               }
            }

            file_in  = hb_fopen( szFileName, "r" );
            fNested  = file_in == NULL && hb_fsMaxFilesError();
         }

         if( ! file_in )
         {
            if( fNested )
            {
               if( pfNested )
                  *pfNested = TRUE;
            }
            else if( pState->pIncludePath && fSearchPath )
            {
               HB_PATHNAMES * pPath = pState->pIncludePath;

               while( pPath && ! file_in )
               {
                  pFileName->szPath = pPath->szPath;
                  hb_fsFNameMerge( szFileNameBuf, pFileName );
                  file_in           = hb_fopen( szFileNameBuf, "r" );
                  pPath             = pPath->pNext;
               }
            }
         }
         hb_xfree( pFileName );
      }

      if( ! file_in )
         return NULL;
   }

   pFile             = ( PHB_PP_FILE ) hb_xgrab( sizeof( HB_PP_FILE ) );
   memset( pFile, '\0', sizeof( HB_PP_FILE ) );

   pFile->szFileName = hb_strdup( szFileName );
   pFile->file_in    = file_in;
   pFile->iLastLine  = 1;

   return pFile;
}

static PHB_PP_FILE hb_pp_FileBufNew( const char * pLineBuf, HB_SIZE ulLineBufLen )
{
   PHB_PP_FILE pFile;

   pFile                = ( PHB_PP_FILE ) hb_xgrab( sizeof( HB_PP_FILE ) );
   memset( pFile, '\0', sizeof( HB_PP_FILE ) );

   pFile->pLineBuf      = pLineBuf;
   pFile->ulLineBufLen  = ulLineBufLen;
   pFile->iLastLine     = 1;

   return pFile;
}

static void hb_pp_FileFree( PHB_PP_STATE pState, PHB_PP_FILE pFile,
                            PHB_PP_CLOSE_FUNC pCloseFunc )
{
   if( pFile->file_in )
   {
      if( pCloseFunc )
         ( pCloseFunc ) ( pState->cargo, pFile->file_in );
      else
         fclose( pFile->file_in );
   }

   if( pFile->szFileName )
      hb_xfree( pFile->szFileName );

   hb_pp_tokenListFree( &pFile->pTokenList );
   hb_xfree( pFile );
}

static void hb_pp_InFileFree( PHB_PP_STATE pState )
{
   while( pState->pFile )
   {
      PHB_PP_FILE pFile = pState->pFile;
      pState->pFile = pFile->pPrev;
      hb_pp_FileFree( pState, pFile, pState->pCloseFunc );
   }
   pState->iFiles = 0;
}

static void hb_pp_OutFileFree( PHB_PP_STATE pState )
{
   if( pState->file_out )
   {
      fclose( pState->file_out );
      pState->file_out = NULL;
   }
   if( pState->szOutFileName )
   {
      hb_xfree( pState->szOutFileName );
      pState->szOutFileName = NULL;
   }
   pState->fWritePreprocesed = FALSE;
}

static void hb_pp_TraceFileFree( PHB_PP_STATE pState )
{
   if( pState->file_trace )
   {
      fclose( pState->file_trace );
      pState->file_trace = NULL;
   }
   if( pState->szTraceFileName )
   {
      hb_xfree( pState->szTraceFileName );
      pState->szTraceFileName = NULL;
   }
   pState->fWriteTrace = FALSE;
}

static PHB_PP_STATE hb_pp_stateNew( void )
{
   PHB_PP_STATE pState = ( PHB_PP_STATE ) hb_xgrab( sizeof( HB_PP_STATE ) );

   memset( pState, '\0', sizeof( HB_PP_STATE ) );

   /* create new line buffer */
   pState->pBuffer      = hb_membufNew();

   /* set default maximum number of translations */
   pState->iMaxCycles   = HB_PP_MAX_CYCLES;

   return pState;
}

static void hb_pp_stateFree( PHB_PP_STATE pState )
{
   hb_pp_InFileFree( pState );
   hb_pp_OutFileFree( pState );
   hb_pp_TraceFileFree( pState );

   if( pState->pIncludePath )
      hb_fsFreeSearchPath( pState->pIncludePath );

   if( pState->iOperators > 0 )
      hb_pp_operatorsFree( pState->pOperators, pState->iOperators );

   hb_pp_ruleListFree( &pState->pDefinitions );
   hb_pp_ruleListFree( &pState->pTranslations );
   hb_pp_ruleListFree( &pState->pCommands );

   hb_pp_tokenListFree( &pState->pTokenOut );

   hb_membufFree( pState->pBuffer );
   if( pState->pDumpBuffer )
      hb_membufFree( pState->pDumpBuffer );
   if( pState->pOutputBuffer )
      hb_membufFree( pState->pOutputBuffer );
   if( pState->pStreamBuffer )
      hb_membufFree( pState->pStreamBuffer );

   if( pState->pCondStack )
      hb_xfree( pState->pCondStack );

   hb_pp_tokenListFree( &pState->pFuncOut );
   hb_pp_tokenListFree( &pState->pFuncEnd );

   hb_xfree( pState );
}

static PHB_PP_TOKEN hb_pp_streamFuncGet( PHB_PP_TOKEN pToken, PHB_PP_TOKEN * pFuncPtr )
{
   hb_pp_tokenListFree( pFuncPtr );

   if( pToken && HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_PIPE &&
       ! HB_PP_TOKEN_ISEOC( pToken->pNext ) )
   {
      PHB_PP_TOKEN * pStartPtr, * pEndPtr, pStart, pNext;
      pStartPtr = pEndPtr = &pToken->pNext;
      while( ! HB_PP_TOKEN_ISEOC( *pEndPtr ) &&
             HB_PP_TOKEN_TYPE( ( *pEndPtr )->type ) != HB_PP_TOKEN_PIPE )
         pEndPtr = &( *pEndPtr )->pNext;

      pToken      = *pEndPtr;
      *pEndPtr    = NULL;
      *pFuncPtr   = pStart = *pStartPtr;
      *pStartPtr  = pToken;
      /* replace %s with HB_PP_RMARKER_STRDUMP marker */
      while( pStart && pStart->pNext )
      {
         pNext = pStart->pNext;
         if( HB_PP_TOKEN_TYPE( pStart->type ) == HB_PP_TOKEN_MOD &&
             HB_PP_TOKEN_TYPE( pNext->type ) == HB_PP_TOKEN_KEYWORD &&
             pNext->len == 1 && pNext->value[ 0 ] == 's' )
         {
            HB_PP_TOKEN_SETTYPE( pStart, HB_PP_RMARKER_STRDUMP );
            pStart->pNext  = pNext->pNext;
            hb_pp_tokenFree( pNext );
            pNext          = pStart->pNext;
         }
         pStart = pNext;
      }
   }
   return pToken;
}

/* #pragma {__text,__stream,__cstream}|functionOut|functionEnd|functionStart */
static BOOL hb_pp_pragmaStream( PHB_PP_STATE pState, PHB_PP_TOKEN pToken )
{
   BOOL fError = FALSE;

   pToken   = hb_pp_streamFuncGet( pToken, &pState->pFuncOut );
   pToken   = hb_pp_streamFuncGet( pToken, &pState->pFuncEnd );
   if( pToken && HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_PIPE )
   {
      hb_pp_tokenSetValue( pToken, ";", 1 );
      HB_PP_TOKEN_SETTYPE( pToken, HB_PP_TOKEN_EOC );
   }

   return fError;
}

#define MAX_STREAM_SIZE 0xFFF0

static void hb_pp_pragmaStreamFile( PHB_PP_STATE pState, char * szFileName )
{
   PHB_PP_FILE pFile = hb_pp_FileNew( pState, szFileName, FALSE, NULL, NULL,
                                      TRUE, pState->pOpenFunc );

   if( pFile )
   {
      char *   pBuffer = ( char * ) hb_xgrab( MAX_STREAM_SIZE + 1 );
      HB_SIZE  ulSize;

      if( ! pState->pStreamBuffer )
         pState->pStreamBuffer = hb_membufNew();

      ulSize = fread( pBuffer, sizeof( char ), MAX_STREAM_SIZE + 1, pFile->file_in );
      hb_pp_FileFree( pState, pFile, pState->pCloseFunc );
      if( ulSize <= MAX_STREAM_SIZE )
      {
         if( pState->iStreamDump == HB_PP_STREAM_C )
            hb_strRemEscSeq( pBuffer, &ulSize );
         hb_membufAddData( pState->pStreamBuffer, pBuffer, ulSize );
         if( pState->pFuncOut )
            hb_pp_tokenAddStreamFunc( pState, pState->pFuncOut,
                                      hb_membufPtr( pState->pStreamBuffer ),
                                      hb_membufLen( pState->pStreamBuffer ) );
         if( pState->pFuncEnd )
         {
            if( pState->pFuncOut )
               hb_pp_tokenAddCmdSep( pState );
            hb_pp_tokenAddStreamFunc( pState, pState->pFuncEnd,
                                      hb_membufPtr( pState->pStreamBuffer ),
                                      hb_membufLen( pState->pStreamBuffer ) );
         }
         hb_membufFlush( pState->pStreamBuffer );
      }
      else
         hb_pp_error( pState, 'F', HB_PP_ERR_FILE_TOO_LONG, szFileName );
      hb_xfree( pBuffer );
   }
   else
      hb_pp_error( pState, 'F', HB_PP_ERR_CANNOT_OPEN_FILE, szFileName );

   hb_pp_tokenListFree( &pState->pFuncOut );
   hb_pp_tokenListFree( &pState->pFuncEnd );
}

static BOOL hb_pp_pragmaOperatorNew( PHB_PP_STATE pState, PHB_PP_TOKEN pToken )
{
   BOOL fError = TRUE;

   if( ! HB_PP_TOKEN_ISEOC( pToken ) && HB_PP_TOKEN_CANJOIN( pToken->type ) )
   {
      HB_SIZE ulLen;

      hb_membufFlush( pState->pBuffer );
      do
      {
         hb_membufAddData( pState->pBuffer, pToken->value, pToken->len );
         pToken = pToken->pNext;
      }
      while( ! HB_PP_TOKEN_ISEOC( pToken ) && pToken->spaces == 0 );
      ulLen = hb_membufLen( pState->pBuffer );
      if( ! HB_PP_TOKEN_ISEOC( pToken ) )
      {
         do
         {
            hb_membufAddData( pState->pBuffer, pToken->value, pToken->len );
            pToken = pToken->pNext;
         }
         while( ! HB_PP_TOKEN_ISEOC( pToken ) && pToken->spaces == 0 );
      }
      if( HB_PP_TOKEN_ISEOC( pToken ) && ulLen > 0 )
      {
         PHB_PP_OPERATOR   pOperator;
         char *            pBuffer  = hb_membufPtr( pState->pBuffer ), * pDstBuffer;
         HB_SIZE           ulDstLen = hb_membufLen( pState->pBuffer ) - ulLen;

         if( ulDstLen )
            pDstBuffer = pBuffer + ulLen;
         else
         {
            pDstBuffer  = pBuffer;
            ulDstLen    = ulLen;
         }
         if( pState->iOperators )
            pState->pOperators = ( PHB_PP_OPERATOR ) hb_xrealloc(
               pState->pOperators,
               sizeof( HB_PP_OPERATOR ) * ( pState->iOperators + 1 ) );
         else
            pState->pOperators = ( PHB_PP_OPERATOR ) hb_xgrab(
               sizeof( HB_PP_OPERATOR ) * ( pState->iOperators + 1 ) );
         pOperator         = &pState->pOperators[ pState->iOperators++ ];
         pOperator->name   = hb_strndup( pBuffer, ulLen );
         pOperator->len    = ulLen;
         pOperator->value  = hb_strndup( pDstBuffer, ulDstLen );
         pOperator->type   = HB_PP_TOKEN_OTHER;
         fError            = FALSE;
      }
   }
   return fError;
}

#define HB_PP_PRAGMA_PUSH  1
#define HB_PP_PRAGMA_POP   2

static BOOL hb_pp_setCompilerSwitch( PHB_PP_STATE pState, char * szSwitch,
                                     int iValue )
{
   BOOL fError = TRUE;

   switch( szSwitch[ 0 ] )
   {
      case 'p':
      case 'P':
         if( szSwitch[ 1 ] == '\0' )
         {
            pState->fWritePreprocesed  = pState->file_out != NULL && iValue != 0;
            fError                     = FALSE;
         }
         else if( ( szSwitch[ 1 ] == 't' || szSwitch[ 1 ] == 'T' ) &&
                  szSwitch[ 2 ] == '\0' )
         {
            pState->fWriteTrace  = pState->file_trace != NULL && iValue != 0;
            fError               = FALSE;
         }
         break;

      case 'q':
      case 'Q':
         if( szSwitch[ 1 ] == '\0' )
         {
            pState->fQuiet = iValue != 0;
            fError         = FALSE;
         }
         break;
   }

   if( pState->pSwitchFunc )
   {
      if( pState->iPushPop == HB_PP_PRAGMA_PUSH )
      {
         int   iPrevious   = 0;
         char  cIndex[ 1 ] = { '\0' };

         if( ( fError = ( pState->pSwitchFunc )( pState->cargo, szSwitch, iValue, &iPrevious, cIndex ) ) == FALSE )
         {
            int iPushed = pState->aaSwitchState[ ( unsigned char ) cIndex[ 0 ] ][ 0 ] + 1;

            if( iPushed < HB_PP_MAX_SWITCH_STATES )
            {
               pState->aaSwitchState[ ( unsigned char ) cIndex[ 0 ] ][ iPushed ] = iPrevious;
               pState->aaSwitchState[ ( unsigned char ) cIndex[ 0 ] ][ 0 ]       = iPushed;
            }
            else
            {
               hb_pp_error( pState, 'E', HB_PP_ERR_TOO_EXCESSIVE_PUSH, szSwitch );
            }
         }
      }
      else
      {
         fError = ( pState->pSwitchFunc )( pState->cargo, szSwitch, iValue, NULL, NULL );
      }
   }

   return fError;
}

static BOOL hb_pp_pragmaPop( PHB_PP_STATE pState, int * piValue, char cIndex[ 1 ] )
{
   int iPushed = pState->aaSwitchState[ ( unsigned char ) cIndex[ 0 ] ][ 0 ];

   if( iPushed > 0 && iPushed <= HB_PP_MAX_SWITCH_STATES )
   {
      *piValue                                                    = pState->aaSwitchState[ ( unsigned char ) cIndex[ 0 ] ][ iPushed ];
      pState->aaSwitchState[ ( unsigned char ) cIndex[ 0 ] ][ 0 ] = --iPushed;

      return TRUE;
   }

   return FALSE;
}

static PHB_PP_TOKEN hb_pp_pragmaGetLogical( PHB_PP_STATE pState, PHB_PP_TOKEN pToken, BOOL * pfValue, char cIndex[ 1 ] )
{
   PHB_PP_TOKEN pValue = NULL;

   if( pState->iPushPop == HB_PP_PRAGMA_POP )
   {
      if( HB_PP_TOKEN_ISEOC( pToken ) && hb_pp_pragmaPop( pState, ( int * ) pfValue, cIndex ) )
      {
         return pValue = pToken->pNext;
      }

      return NULL;
   }

   if( pToken && pToken->pNext &&
       HB_PP_TOKEN_TYPE( pToken->pNext->type ) == HB_PP_TOKEN_KEYWORD )
   {
      if( ( HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_EQ &&
            HB_PP_TOKEN_ISEOC( pToken->pNext->pNext ) ) ||
          ( pToken->pNext->pNext &&
            HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_LEFT_PB &&
            HB_PP_TOKEN_TYPE( pToken->pNext->pNext->type ) == HB_PP_TOKEN_RIGHT_PB &&
            HB_PP_TOKEN_ISEOC( pToken->pNext->pNext->pNext ) ) )
      {
         pValue = pToken->pNext;
         if( hb_stricmp( pValue->value, "ON" ) == 0 )
            *pfValue = TRUE;
         else if( hb_stricmp( pValue->value, "OFF" ) == 0 )
            *pfValue = FALSE;
         else
            pValue = NULL;
      }
   }
   return pValue;
}

static PHB_PP_TOKEN hb_pp_pragmaGetInt( PHB_PP_STATE pState, PHB_PP_TOKEN pToken, int * piValue, char cIndex[ 1 ] )
{
   PHB_PP_TOKEN pValue = NULL;

   if( pState->iPushPop == HB_PP_PRAGMA_POP )
   {
      if( HB_PP_TOKEN_ISEOC( pToken ) && hb_pp_pragmaPop( pState, ( int * ) piValue, cIndex ) )
      {
         return pToken;
      }

      return NULL;
   }

   if( pToken && pToken->pNext &&
       HB_PP_TOKEN_TYPE( pToken->pNext->type ) == HB_PP_TOKEN_NUMBER )
   {
      if( ( HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_EQ &&
            HB_PP_TOKEN_ISEOC( pToken->pNext->pNext ) ) ||
          ( pToken->pNext->pNext &&
            HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_LEFT_PB &&
            HB_PP_TOKEN_TYPE( pToken->pNext->pNext->type ) == HB_PP_TOKEN_RIGHT_PB &&
            HB_PP_TOKEN_ISEOC( pToken->pNext->pNext->pNext ) ) )
      {
         pValue   = pToken->pNext;
         *piValue = atoi( pValue->value );
      }
   }
   return pValue;
}

static PHB_PP_TOKEN hb_pp_pragmaGetSwitch( PHB_PP_STATE pState, PHB_PP_TOKEN pToken, int * piValue )
{
   PHB_PP_TOKEN pValue = NULL;

   if( pState->iPushPop == HB_PP_PRAGMA_POP )
   {
      if( pToken && HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_KEYWORD && pToken->len == 1 && HB_PP_TOKEN_ISEOC( pToken->pNext ) )
      {
         if( hb_pp_pragmaPop( pState, piValue, pToken->value ) )
         {
            return pToken;
         }
      }

      return NULL;
   }
   if( pToken && HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_KEYWORD )
   {
      BOOL fNum = pToken->len > 1 && HB_PP_ISDIGIT( pToken->value[ pToken->len - 1 ] );

      if( HB_PP_TOKEN_ISEOC( pToken->pNext ) )
      {
         if( fNum )
         {
            pValue   = pToken;
            *piValue = pValue->value[ pToken->len - 1 ] - '0';
         }
      }
      else if( HB_PP_TOKEN_ISEOC( pToken->pNext->pNext ) && ! fNum )
      {
         if( HB_PP_TOKEN_TYPE( pToken->pNext->type ) == HB_PP_TOKEN_MINUS )
         {
            pValue   = pToken;
            *piValue = 0;
         }
         else if( HB_PP_TOKEN_TYPE( pToken->pNext->type ) == HB_PP_TOKEN_PLUS )
         {
            pValue   = pToken;
            *piValue = 1;
         }
         else if( HB_PP_TOKEN_TYPE( pToken->pNext->type ) == HB_PP_TOKEN_NUMBER )
         {
            pValue   = pToken;
            *piValue = atoi( pValue->pNext->value );
         }
      }
   }
   return pValue;
}

static void hb_pp_pragmaNew( PHB_PP_STATE pState, PHB_PP_TOKEN pToken )
{
   PHB_PP_TOKEN   pValue   = NULL;
   BOOL           fError   = FALSE, fValue = FALSE;
   int            iValue   = 0;

   pState->iPushPop = 0;
   if( pToken && HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_KEYWORD )
   {
      if( hb_pp_tokenValueCmp( pToken, "push", HB_PP_CMP_DBASE ) )
      {
         pState->iPushPop  = HB_PP_PRAGMA_PUSH;
         pToken            = pToken->pNext;
      }
      else if( hb_pp_tokenValueCmp( pToken, "pop", HB_PP_CMP_DBASE ) )
      {
         pState->iPushPop  = HB_PP_PRAGMA_POP;
         pToken            = pToken->pNext;
      }
   }

   if( ! pToken )
      fError = TRUE;
   else if( pToken->len == 1 && HB_ISOPTSEP( pToken->value[ 0 ] ) )
   {
      if( ! pState->iCondCompile )
      {
         pToken   = pToken->pNext;
         pValue   = hb_pp_pragmaGetSwitch( pState, pToken, &iValue );
         if( pValue )
            fError = hb_pp_setCompilerSwitch( pState, pValue->value, iValue );
         else
            fError = TRUE;
      }
   }
   else if( HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_KEYWORD )
   {
      if( hb_pp_tokenValueCmp( pToken, "begindump", HB_PP_CMP_DBASE ) )
      {
         pState->iStreamDump  = HB_PP_STREAM_DUMP_C;
         pState->iDumpLine    = pState->pFile->iCurrentLine;
         if( ! pState->pDumpBuffer )
            pState->pDumpBuffer = hb_membufNew();
      }
      else if( hb_pp_tokenValueCmp( pToken, "enddump", HB_PP_CMP_DBASE ) )
      {
         pState->iStreamDump = HB_PP_STREAM_OFF;
      }
      else if( hb_pp_tokenValueCmp( pToken, "__text", HB_PP_CMP_DBASE ) )
      {
         fError = hb_pp_pragmaStream( pState, pToken->pNext );
         if( ! fError )
            pState->iStreamDump = HB_PP_STREAM_CLIPPER;
      }
      else if( hb_pp_tokenValueCmp( pToken, "__stream", HB_PP_CMP_DBASE ) )
      {
         fError = hb_pp_pragmaStream( pState, pToken->pNext );
         if( ! fError )
         {
            pState->iStreamDump = HB_PP_STREAM_PRG;
            if( ! pState->pStreamBuffer )
               pState->pStreamBuffer = hb_membufNew();
         }
      }
      else if( hb_pp_tokenValueCmp( pToken, "__cstream", HB_PP_CMP_DBASE ) )
      {
         fError = hb_pp_pragmaStream( pState, pToken->pNext );
         if( ! fError )
         {
            pState->iStreamDump = HB_PP_STREAM_C;
            if( ! pState->pStreamBuffer )
               pState->pStreamBuffer = hb_membufNew();
         }
      }
      else if( hb_pp_tokenValueCmp( pToken, "__streaminclude", HB_PP_CMP_DBASE ) )
      {
         if( pToken->pNext && HB_PP_TOKEN_TYPE( pToken->pNext->type ) == HB_PP_TOKEN_STRING )
         {
            fError = hb_pp_pragmaStream( pState, pToken->pNext->pNext );
            if( ! fError )
            {
               pState->iStreamDump  = HB_PP_STREAM_PRG;
               hb_pp_pragmaStreamFile( pState, pToken->pNext->value );
               pState->iStreamDump  = HB_PP_STREAM_OFF;
            }
         }
         else
            fError = TRUE;
      }
      else if( hb_pp_tokenValueCmp( pToken, "__cstreaminclude", HB_PP_CMP_DBASE ) )
      {
         if( pToken->pNext && HB_PP_TOKEN_TYPE( pToken->pNext->type ) == HB_PP_TOKEN_STRING )
         {
            fError = hb_pp_pragmaStream( pState, pToken->pNext->pNext );
            if( ! fError )
            {
               pState->iStreamDump  = HB_PP_STREAM_C;
               hb_pp_pragmaStreamFile( pState, pToken->pNext->value );
               pState->iStreamDump  = HB_PP_STREAM_OFF;
            }
         }
         else
            fError = TRUE;
      }
      else if( hb_pp_tokenValueCmp( pToken, "__endtext", HB_PP_CMP_DBASE ) )
      {
         pState->iStreamDump = HB_PP_STREAM_OFF;
      }
      else if( pState->iCondCompile )
      {
         /* conditional compilation - other preprocessing and output disabled */
      }
      else if( hb_pp_tokenValueCmp( pToken, "AUTOMEMVAR", HB_PP_CMP_DBASE ) )
      {
         pValue = hb_pp_pragmaGetLogical( pState, pToken->pNext, &fValue, "a" );
         if( pValue )
            fError = hb_pp_setCompilerSwitch( pState, "a", ( int ) fValue );
         else
            fError = TRUE;
      }
      else if( hb_pp_tokenValueCmp( pToken, "DEBUGINFO", HB_PP_CMP_DBASE ) )
      {
         pValue = hb_pp_pragmaGetLogical( pState, pToken->pNext, &fValue, "b" );
         if( pValue )
            fError = hb_pp_setCompilerSwitch( pState, "b", ( int ) fValue );
         else
            fError = TRUE;
      }
      else if( hb_pp_tokenValueCmp( pToken, "DYNAMICMEMVAR", HB_PP_CMP_DBASE ) )
      {
         pValue = hb_pp_pragmaGetLogical( pState, pToken->pNext, &fValue, "v" );
         if( pValue )
            fError = hb_pp_setCompilerSwitch( pState, "v", ( int ) fValue );
         else
            fError = TRUE;
      }
      else if( hb_pp_tokenValueCmp( pToken, "ENABLEWARNINGS", HB_PP_CMP_DBASE ) )
      {
         pValue = hb_pp_pragmaGetLogical( pState, pToken->pNext, &fValue, "w" );
         if( pValue )
            fError = hb_pp_setCompilerSwitch( pState, "w", fValue ? 1 : 0 );
         else
            fError = TRUE;
      }
      else if( hb_pp_tokenValueCmp( pToken, "EXITSEVERITY", HB_PP_CMP_DBASE ) )
      {
         pValue = hb_pp_pragmaGetInt( pState, pToken->pNext, &iValue, "e" );
         if( pValue )
            fError = hb_pp_setCompilerSwitch( pState, "es", iValue );
         else
            fError = TRUE;
      }
      else if( hb_pp_tokenValueCmp( pToken, "LINENUMBER", HB_PP_CMP_DBASE ) )
      {
         pValue = hb_pp_pragmaGetLogical( pState, pToken->pNext, &fValue, "l" );
         if( pValue )
            fError = hb_pp_setCompilerSwitch( pState, "l", fValue );
         else
            fError = TRUE;
      }
      else if( hb_pp_tokenValueCmp( pToken, "NOSTARTPROC", HB_PP_CMP_DBASE ) )
      {
         pValue = hb_pp_pragmaGetLogical( pState, pToken->pNext, &fValue, "n" );
         if( pValue )
            fError = hb_pp_setCompilerSwitch( pState, "n", fValue );
         else
            fError = TRUE;
      }
      else if( hb_pp_tokenValueCmp( pToken, "OPERATOR", HB_PP_CMP_DBASE ) )
      {
         fError = hb_pp_pragmaOperatorNew( pState, pToken->pNext );
      }
      else if( hb_pp_tokenValueCmp( pToken, "PREPROCESSING", HB_PP_CMP_DBASE ) )
      {
         pValue = hb_pp_pragmaGetLogical( pState, pToken->pNext, &fValue, "p" );
         if( pValue )
            fError = hb_pp_setCompilerSwitch( pState, "p", fValue );
         else
            fError = TRUE;
      }
      else if( hb_pp_tokenValueCmp( pToken, "SHORTCUT", HB_PP_CMP_DBASE ) )
      {
         pValue = hb_pp_pragmaGetLogical( pState, pToken->pNext, &fValue, "z" );
         if( pValue )
            fError = hb_pp_setCompilerSwitch( pState, "z", fValue );
         else
            fError = TRUE;
      }
      else if( hb_pp_tokenValueCmp( pToken, "DEFERRED", HB_PP_CMP_DBASE ) )
      {
         hb_compSetDeferredFlagOn();
      }
      else if( hb_pp_tokenValueCmp( pToken, "GC0", HB_PP_CMP_DBASE ) )
      {
         hb_compSetCOutput( 0 );
      }
      else if( hb_pp_tokenValueCmp( pToken, "GC1", HB_PP_CMP_DBASE ) )
      {
         hb_compSetCOutput( 1 );
      }
      else if( hb_pp_tokenValueCmp( pToken, "GC2", HB_PP_CMP_DBASE ) )
      {
         hb_compSetCOutput( 2 );
      }
      else if( hb_pp_tokenValueCmp( pToken, "GC3", HB_PP_CMP_DBASE ) )
      {
         hb_compSetCOutput( 3 );
      }
      else if( hb_pp_tokenValueCmp( pToken, "GC4", HB_PP_CMP_DBASE ) )
      {
         hb_compSetCOutput( 4 );
      }
      else if( hb_pp_tokenValueCmp( pToken, "CPP", HB_PP_CMP_DBASE ) )
      {
         hb_compSetCOutput( 5 );
      }
      else if( hb_pp_tokenValueCmp( pToken, "W0", HB_PP_CMP_DBASE ) )
      {
         hb_compSetCOutput( 6 );
      }
      else if( hb_pp_tokenValueCmp( pToken, "W1", HB_PP_CMP_DBASE ) )
      {
         hb_compSetCOutput( 7 );
      }
      else if( hb_pp_tokenValueCmp( pToken, "W2", HB_PP_CMP_DBASE ) )
      {
         hb_compSetCOutput( 8 );
      }
      else if( hb_pp_tokenValueCmp( pToken, "W3", HB_PP_CMP_DBASE ) )
      {
         hb_compSetCOutput( 9 );
      }
      else if( hb_pp_tokenValueCmp( pToken, "RECURSELEVEL", HB_PP_CMP_DBASE ) )
      {
         pValue   = hb_pp_pragmaGetInt( pState, pToken->pNext, &pState->iMaxCycles, "r" );
         fError   = pValue == NULL;
      }
      /* xHarbour extension */
      else if( hb_pp_tokenValueCmp( pToken, "TEXTHIDDEN", HB_PP_CMP_DBASE ) )
      {
         pValue = hb_pp_pragmaGetInt( pState, pToken->pNext, &pState->iHideStrings, "h" );
         if( pValue )
            fError = hb_pp_setCompilerSwitch( pState, pToken->value, pState->iHideStrings );
         else
            fError = TRUE;
      }
      else if( hb_pp_tokenValueCmp( pToken, "TRACE", HB_PP_CMP_DBASE ) )
      {
         pValue = hb_pp_pragmaGetLogical( pState, pToken->pNext, &fValue, "t" );
         if( pValue )
            fError = hb_pp_setCompilerSwitch( pState, "pt", fValue );
         else
            fError = TRUE;
      }
      else if( hb_pp_tokenValueCmp( pToken, "TRACEPRAGMAS", HB_PP_CMP_DBASE ) )
      {
         pValue   = hb_pp_pragmaGetLogical( pState, pToken->pNext, &pState->fTracePragmas, "g" );
         fError   = pValue == NULL;
      }
      else if( hb_pp_tokenValueCmp( pToken, "WARNINGLEVEL", HB_PP_CMP_DBASE ) )
      {
         pValue = hb_pp_pragmaGetInt( pState, pToken->pNext, &iValue, "w" );
         if( pValue )
            fError = hb_pp_setCompilerSwitch( pState, "w", iValue );
         else
            fError = TRUE;
      }
      else
         fError = TRUE;
   }
   else
      fError = TRUE;

   if( pState->iCondCompile )
   {
      ;
   }
   else if( fError )
   {
      hb_pp_error( pState, 'E', HB_PP_ERR_PRAGMA, NULL );
   }
   else if( pState->fTracePragmas || pState->fWriteTrace )
   {
      char szLine[ 12 ];

      hb_snprintf( szLine, sizeof( szLine ), "%d", pState->pFile->iCurrentLine );
      hb_membufFlush( pState->pBuffer );
      hb_membufAddCh( pState->pBuffer, '(' );
      hb_membufAddStr( pState->pBuffer, szLine );
      hb_membufAddStr( pState->pBuffer, ") #pragma " );

      if( pState->iPushPop == 0 )
      {
         hb_membufAddStr( pState->pBuffer, pToken->value );

         if( pValue && pValue != pToken )
         {
            hb_membufAddStr( pState->pBuffer, " set to '" );
            hb_membufAddStr( pState->pBuffer, pValue->value );
            hb_membufAddCh( pState->pBuffer, '\'' );
         }
      }
      else if( pState->iPushPop == HB_PP_PRAGMA_PUSH )
      {
         if( pValue && pValue != pToken )
         {
            if( pToken->len == 1 || HB_ISOPTSEP( pToken->value[ 0 ] ) )
            {
               hb_membufAddStr( pState->pBuffer, pToken->value );
               hb_membufAddStr( pState->pBuffer, " pushed '" );
               hb_membufAddStr( pState->pBuffer, pToken->pNext->value );

               if( ! HB_PP_TOKEN_ISEOC( pToken->pNext->pNext ) )
               {
                  hb_membufAddStr( pState->pBuffer, "' value '" );
                  hb_membufAddStr( pState->pBuffer, pToken->pNext->pNext->value );
               }
            }
            else
            {
               hb_membufAddStr( pState->pBuffer, pToken->value );
               hb_membufAddStr( pState->pBuffer, "' pushed '" );
               hb_membufAddStr( pState->pBuffer, pToken->pNext->pNext->value );
            }

            hb_membufAddCh( pState->pBuffer, '\'' );
         }
      }
      else if( pState->iPushPop == HB_PP_PRAGMA_POP )
      {
         if( pValue && pValue != pToken )
         {
            hb_membufAddStr( pState->pBuffer, " poped '" );
            hb_membufAddStr( pState->pBuffer, pToken->pNext->value );
            hb_membufAddCh( pState->pBuffer, '\'' );
         }
      }
      hb_membufAddCh( pState->pBuffer, '\n' );
      if( pState->fWriteTrace )
      {
         fwrite( hb_membufPtr( pState->pBuffer ), sizeof( char ),
                 ( size_t ) hb_membufLen( pState->pBuffer ), pState->file_trace );
      }
      if( pState->fTracePragmas )
      {
         hb_membufAddCh( pState->pBuffer, '\0' );
         hb_pp_disp( pState, hb_membufPtr( pState->pBuffer ) );
      }
   }
}

static void hb_pp_defineNew( PHB_PP_STATE pState, PHB_PP_TOKEN pToken, BOOL fDirect )
{
   PHB_PP_TOKEN pMatch = pToken ? pToken->pNext : NULL;

   if( ! pMatch || HB_PP_TOKEN_TYPE( pMatch->type ) != HB_PP_TOKEN_KEYWORD )
   {
      hb_pp_error( pState, 'E', HB_PP_ERR_DEFINE_SYNTAX, NULL );
   }
   else
   {
      PHB_PP_TOKEN   pResult, pLast = pMatch->pNext, pParam;
      PHB_PP_MARKER  pMarkers = NULL;
      USHORT         usPCount = 0, usParam;

      /* pseudo function? */
      if( pLast && HB_PP_TOKEN_TYPE( pLast->type ) == HB_PP_TOKEN_LEFT_PB &&
          pLast->spaces == 0 )
      {
         USHORT type = HB_PP_TOKEN_KEYWORD;
         while( TRUE )
         {
            pLast = pLast->pNext;
            if( pLast && ( usPCount == 0 || type == HB_PP_TOKEN_COMMA ) &&
                HB_PP_TOKEN_TYPE( pLast->type ) == HB_PP_TOKEN_RIGHT_PB )
               break;
            if( ! pLast || type != HB_PP_TOKEN_TYPE( pLast->type ) )
            {
               if( type == HB_PP_TOKEN_KEYWORD )
                  hb_pp_error( pState, 'E', HB_PP_ERR_LABEL_MISSING_IN_DEFINE, NULL );
               else
                  hb_pp_error( pState, 'E', HB_PP_ERR_PARE_MISSING_IN_DEFINE, NULL );
               return;
            }
            else if( type == HB_PP_TOKEN_KEYWORD )
            {
               ++usPCount;
               type = HB_PP_TOKEN_COMMA;
            }
            else
               type = HB_PP_TOKEN_KEYWORD;
         }
      }
      else  /* simple keyword define */
         pLast = pMatch;
      pResult        = pLast->pNext;
      pLast->pNext   = NULL;
      pToken->pNext  = hb_pp_tokenResultEnd( &pResult, fDirect );
      if( usPCount )
      {
         usPCount = 0;
         pParam   = pMatch->pNext->pNext;
         while( HB_PP_TOKEN_TYPE( pParam->type ) == HB_PP_TOKEN_KEYWORD )
         {
            usParam  = 0;
            /* Check if it's not repeated ID */
            pLast    = pMatch->pNext->pNext;
            while( pLast != pParam && ! hb_pp_tokenEqual( pParam, pLast, HB_PP_CMP_CASE ) )
            {
               pLast = pLast->pNext;
            }
            if( pLast == pParam )
            {
               pLast = pResult;
               /* replace parameter tokens in result pattern with regular
                  result markers */
               while( pLast )
               {
                  if( hb_pp_tokenEqual( pParam, pLast, HB_PP_CMP_CASE ) )
                  {
                     HB_PP_TOKEN_SETTYPE( pLast, HB_PP_RMARKER_REGULAR );
                     if( usParam == 0 )
                        usParam = ++usPCount;
                     pLast->index = usParam;
                  }
                  pLast = pLast->pNext;
               }
            }
            HB_PP_TOKEN_SETTYPE( pParam, HB_PP_MMARKER_REGULAR );
            pParam->index  = usParam;
            pParam         = pParam->pNext;
            if( HB_PP_TOKEN_TYPE( pParam->type ) == HB_PP_TOKEN_COMMA )
               pParam = pParam->pNext;
         }
         if( usPCount )
         {
            /* create regular match and result markers from parameters */
            pMarkers = ( PHB_PP_MARKER ) hb_xgrab( usPCount * sizeof( HB_PP_MARKER ) );
            memset( pMarkers, '\0', usPCount * sizeof( HB_PP_MARKER ) );
         }
      }
      hb_pp_defineAdd( pState, HB_PP_CMP_CASE, usPCount, pMarkers, pMatch, pResult );
   }
}

static BOOL hb_pp_tokenUnQuotedGet( PHB_PP_TOKEN ** pTokenPtr, BOOL * pfQuoted,
                                    BOOL fFree )
{
   PHB_PP_TOKEN pToken = **pTokenPtr;

   *pfQuoted = FALSE;
   if( pToken )
   {
      if( fFree )
      {
         **pTokenPtr = pToken->pNext;
         hb_pp_tokenFree( pToken );
      }
      else
      {
         *pTokenPtr = &pToken->pNext;
      }
      pToken = **pTokenPtr;
      if( pToken )
      {
         if( HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_BACKSLASH )
         {
            *pfQuoted   = TRUE;
            if( pToken->pNext )
               pToken->pNext->spaces = pToken->spaces;
            **pTokenPtr = pToken->pNext;
            hb_pp_tokenFree( pToken );
            pToken      = **pTokenPtr;
         }
      }
   }

   return pToken != NULL;
}

static BOOL hb_pp_matchMarkerNew( PHB_PP_TOKEN * pTokenPtr,
                                  PHB_PP_MARKERLST * pMarkerListPtr )
{
   USHORT         type        = HB_PP_TOKEN_NUL;
   PHB_PP_TOKEN   pMarkerId   = NULL, pMTokens = NULL;
   BOOL           fQuoted;

   /* At start pTokenPtr points to '<' token */

   if( hb_pp_tokenUnQuotedGet( &pTokenPtr, &fQuoted, TRUE ) && ! fQuoted )
   {
      if( HB_PP_TOKEN_TYPE( ( *pTokenPtr )->type ) == HB_PP_TOKEN_KEYWORD )
      {
         pMarkerId = *pTokenPtr;
         if( hb_pp_tokenUnQuotedGet( &pTokenPtr, &fQuoted, FALSE ) && ! fQuoted )
         {
            if( HB_PP_TOKEN_TYPE( ( *pTokenPtr )->type ) == HB_PP_TOKEN_GT )
               type = HB_PP_MMARKER_REGULAR;
            else if( HB_PP_TOKEN_TYPE( ( *pTokenPtr )->type ) == HB_PP_TOKEN_COMMA )
            {
               int i = 3;
               do
               {
                  if( ! hb_pp_tokenUnQuotedGet( &pTokenPtr, &fQuoted, TRUE ) || fQuoted )
                     break;
                  if( i == 3 && HB_PP_TOKEN_TYPE( ( *pTokenPtr )->type ) == HB_PP_TOKEN_EPSILON )
                  {
                     i = 0;
                     break;
                  }
                  if( HB_PP_TOKEN_TYPE( ( *pTokenPtr )->type ) != HB_PP_TOKEN_DOT )
                     break;
               }
               while( --i > 0 );
               if( i == 0 && hb_pp_tokenUnQuotedGet( &pTokenPtr, &fQuoted, TRUE ) &&
                   ! fQuoted && HB_PP_TOKEN_TYPE( ( *pTokenPtr )->type ) == HB_PP_TOKEN_GT )
                  type = HB_PP_MMARKER_LIST;
            }
            else if( HB_PP_TOKEN_TYPE( ( *pTokenPtr )->type ) == HB_PP_TOKEN_SEND )
            {
               if( hb_pp_tokenUnQuotedGet( &pTokenPtr, &fQuoted, TRUE ) )
               {
                  PHB_PP_TOKEN pLast = NULL;
                  do
                  {
                     if( HB_PP_TOKEN_TYPE( ( *pTokenPtr )->type ) == HB_PP_TOKEN_GT && ! fQuoted )
                     {
                        if( pLast )
                        {
                           pMTokens          = pMarkerId->pNext;
                           pMarkerId->pNext  = *pTokenPtr;
                           pTokenPtr         = &pMarkerId->pNext;
                           pLast->pNext      = NULL;
                        }
                        type = HB_PP_MMARKER_RESTRICT;
                        break;
                     }
                     pLast = *pTokenPtr;
                  }
                  while( hb_pp_tokenUnQuotedGet( &pTokenPtr, &fQuoted, FALSE ) );
               }
            }
         }
      }
      else if( HB_PP_TOKEN_TYPE( ( *pTokenPtr )->type ) == HB_PP_TOKEN_MULT )
      {
         if( hb_pp_tokenUnQuotedGet( &pTokenPtr, &fQuoted, TRUE ) && ! fQuoted &&
             HB_PP_TOKEN_TYPE( ( *pTokenPtr )->type ) == HB_PP_TOKEN_KEYWORD )
         {
            pMarkerId = *pTokenPtr;
            if( hb_pp_tokenUnQuotedGet( &pTokenPtr, &fQuoted, FALSE ) && ! fQuoted &&
                HB_PP_TOKEN_TYPE( ( *pTokenPtr )->type ) == HB_PP_TOKEN_MULT &&
                hb_pp_tokenUnQuotedGet( &pTokenPtr, &fQuoted, TRUE ) && ! fQuoted &&
                HB_PP_TOKEN_TYPE( ( *pTokenPtr )->type ) == HB_PP_TOKEN_GT )
               type = HB_PP_MMARKER_WILD;
         }
      }
      else if( HB_PP_TOKEN_TYPE( ( *pTokenPtr )->type ) == HB_PP_TOKEN_LEFT_PB )
      {
         if( hb_pp_tokenUnQuotedGet( &pTokenPtr, &fQuoted, TRUE ) && ! fQuoted &&
             HB_PP_TOKEN_TYPE( ( *pTokenPtr )->type ) == HB_PP_TOKEN_KEYWORD )
         {
            pMarkerId = *pTokenPtr;
            if( hb_pp_tokenUnQuotedGet( &pTokenPtr, &fQuoted, FALSE ) && ! fQuoted &&
                HB_PP_TOKEN_TYPE( ( *pTokenPtr )->type ) == HB_PP_TOKEN_RIGHT_PB &&
                hb_pp_tokenUnQuotedGet( &pTokenPtr, &fQuoted, TRUE ) && ! fQuoted &&
                HB_PP_TOKEN_TYPE( ( *pTokenPtr )->type ) == HB_PP_TOKEN_GT )
               type = HB_PP_MMARKER_EXTEXP;
         }
      }
      else if( HB_PP_TOKEN_TYPE( ( *pTokenPtr )->type ) == HB_PP_TOKEN_NOT )
      {
         if( hb_pp_tokenUnQuotedGet( &pTokenPtr, &fQuoted, TRUE ) && ! fQuoted &&
             HB_PP_TOKEN_TYPE( ( *pTokenPtr )->type ) == HB_PP_TOKEN_KEYWORD )
         {
            pMarkerId = *pTokenPtr;
            if( hb_pp_tokenUnQuotedGet( &pTokenPtr, &fQuoted, FALSE ) && ! fQuoted &&
                HB_PP_TOKEN_TYPE( ( *pTokenPtr )->type ) == HB_PP_TOKEN_NOT &&
                hb_pp_tokenUnQuotedGet( &pTokenPtr, &fQuoted, TRUE ) && ! fQuoted &&
                HB_PP_TOKEN_TYPE( ( *pTokenPtr )->type ) == HB_PP_TOKEN_GT )
               type = HB_PP_MMARKER_NAME;
         }
      }
   }

   if( type != HB_PP_TOKEN_NUL )
   {
      PHB_PP_MARKERLST  pMrkLst = *pMarkerListPtr, pMrkPrev = NULL;
      PHB_PP_MARKERPTR  pMrkPtr;

      while( pMrkLst && ! hb_pp_tokenEqual( pMrkLst->pMatchMarkers->pToken,
                                            pMarkerId, HB_PP_CMP_CASE ) )
      {
         pMrkPrev = pMrkLst;
         pMrkLst  = pMrkLst->pNext;
      }
      if( ! pMrkLst )
      {
         pMrkLst = ( PHB_PP_MARKERLST ) hb_xgrab( sizeof( HB_PP_MARKERLST ) );
         if( pMrkPrev )
            pMrkPrev->pNext = pMrkLst;
         else
            *pMarkerListPtr = pMrkLst;
         pMrkLst->pNext          = NULL;
         pMrkLst->pMatchMarkers  = NULL;
         pMrkLst->canrepeat      = TRUE;
         pMrkLst->index          = 0;
      }
      pMrkPtr                 = ( PHB_PP_MARKERPTR ) hb_xgrab( sizeof( HB_PP_MARKERPTR ) );
      pMrkPtr->pNext          = pMrkLst->pMatchMarkers;
      pMrkLst->pMatchMarkers  = pMrkPtr;
      pMrkPtr->pToken         = pMarkerId;
      pMrkPtr->pMTokens       = pMTokens;
      pMrkPtr->type           = type;
      /* mark non restricted markers for later detection two consecutive
         optional match markers */
      if( type != HB_PP_MMARKER_RESTRICT )
         pMarkerId->type |= HB_PP_TOKEN_MATCHMARKER;
      /* free the trailing '>' marker token */
      pMTokens    = *pTokenPtr;
      *pTokenPtr  = pMTokens->pNext;
      hb_pp_tokenFree( pMTokens );
      return TRUE;
   }
   return FALSE;
}

static BOOL hb_pp_matchHasKeywords( PHB_PP_TOKEN pToken )
{
   /* Now we are strictly Clipper compatible here though the nested
      optional markers which have keywords on deeper levels are not
      recognized. Exactly the same makes Clipper PP */
   while( HB_PP_TOKEN_ISMATCH( pToken ) )
      pToken = pToken->pNext;
   return pToken != NULL;
}

static BOOL hb_pp_matchPatternNew( PHB_PP_STATE pState, PHB_PP_TOKEN * pTokenPtr,
                                   PHB_PP_MARKERLST * pMarkerListPtr,
                                   PHB_PP_TOKEN ** pOptional )
{
   PHB_PP_TOKEN * pLastPtr = NULL;
   BOOL           fQuoted  = FALSE;

   if( HB_PP_TOKEN_TYPE( ( *pTokenPtr )->type ) == HB_PP_TOKEN_BACKSLASH )
   {
      PHB_PP_TOKEN pToken = *pTokenPtr;
      *pTokenPtr  = pToken->pNext;
      hb_pp_tokenFree( pToken );
      fQuoted     = TRUE;
   }

   do
   {
      /*
         Here we test for escaped multichar tokens such as:
         \<<
         which should really be processed as:
         \< <
         so we break the multi-char into multiple tokens.
       */
      if( fQuoted )
      {
         if( HB_PP_TOKEN_TYPE( ( *pTokenPtr )->type ) == HB_PP_TOKEN_SHIFTL )
         {
            PHB_PP_TOKEN   pEscaped = hb_pp_tokenNew( "<", 1, ( *pTokenPtr )->spaces, HB_PP_TOKEN_LT | HB_PP_TOKEN_STATIC );
            PHB_PP_TOKEN   pLT      = hb_pp_tokenNew( "<", 1, 0, HB_PP_TOKEN_LT | HB_PP_TOKEN_STATIC );

            pEscaped->pNext   = pLT;
            pLT->pNext        = ( *pTokenPtr )->pNext;

            hb_pp_tokenFree( *pTokenPtr );
            *pTokenPtr        = pEscaped;
         }
         /* Should not be needed, even <list:> matcher does NOT support [ESCAPED] operators!
            else if( HB_PP_TOKEN_TYPE( ( * pTokenPtr )->type ) == HB_PP_TOKEN_SHIFTR )
            {
            PHB_PP_TOKEN pEscaped = hb_pp_tokenNew( ">", 1, ( * pTokenPtr )->spaces, HB_PP_TOKEN_GT | HB_PP_TOKEN_STATIC );
            PHB_PP_TOKEN pGT      = hb_pp_tokenNew( ">", 1, 0,                       HB_PP_TOKEN_GT | HB_PP_TOKEN_STATIC );

            pEscaped->pNext = pGT;
            pGT->pNext      = ( * pTokenPtr )->pNext;

            hb_pp_tokenFree( * pTokenPtr );
          * pTokenPtr = pEscaped;
            }
          */
      }

      if( ! fQuoted )
      {
         if( HB_PP_TOKEN_TYPE( ( *pTokenPtr )->type ) == HB_PP_TOKEN_LT )
         {
            if( ! hb_pp_matchMarkerNew( pTokenPtr, pMarkerListPtr ) )
            {
               hb_pp_error( pState, 'E', HB_PP_ERR_BAD_MATCH_MARKER, ( *pTokenPtr )->value );
               return FALSE;
            }
            /* now pTokenPtr points to marker keyword, all other tokens
               have been stripped */
         }
         else if( HB_PP_TOKEN_TYPE( ( *pTokenPtr )->type ) == HB_PP_TOKEN_RIGHT_SB )
         {
            if( pOptional )
            {
               *pOptional = pTokenPtr;
               return TRUE;
            }
         }
         else if( HB_PP_TOKEN_TYPE( ( *pTokenPtr )->type ) == HB_PP_TOKEN_LEFT_SB )
         {
            PHB_PP_TOKEN * pStopOptPtr = NULL;
            if( ! ( *pTokenPtr )->pNext )
            {
               /* assign pOptional only to force error below */
               pOptional = &pTokenPtr;
               break;
            }
            else if( ! hb_pp_matchPatternNew( pState, &( *pTokenPtr )->pNext,
                                              pMarkerListPtr, &pStopOptPtr ) )
               return FALSE;
            else if( *pStopOptPtr == ( *pTokenPtr )->pNext )
            {
               hb_pp_error( pState, 'E', HB_PP_ERR_EMPTY_OPTIONAL, NULL );
               return FALSE;
            }
            else
            {
               PHB_PP_TOKEN pToken, pOptTok = ( *pTokenPtr )->pNext;
               pToken                  = *pStopOptPtr;
               *pStopOptPtr            = NULL;
               ( *pTokenPtr )->pNext   = pToken->pNext;
               hb_pp_tokenFree( pToken );
               /* create new optional match marker */
               HB_PP_TOKEN_SETTYPE( *pTokenPtr, HB_PP_MMARKER_OPTIONAL );
               if( ( *pTokenPtr )->spaces > 1 )
                  ( *pTokenPtr )->spaces = 1;
               ( *pTokenPtr )->type       |= HB_PP_TOKEN_MATCHMARKER;
               ( *pTokenPtr )->pMTokens   = pOptTok;
               if( pLastPtr && ! hb_pp_matchHasKeywords( *pLastPtr ) )
               {
                  if( ! hb_pp_matchHasKeywords( pOptTok ) )
                  {
                     hb_pp_error( pState, 'E', HB_PP_ERR_AMBIGUOUS_MATCH_PATTERN, NULL );
                     return FALSE;
                  }
                  /* replace the order for these optional tokens to keep
                     the ones with keywords 1-st */
                  ( *pTokenPtr )->pMTokens   = *pLastPtr;
                  *pLastPtr                  = pOptTok;
               }
               pLastPtr = &( *pTokenPtr )->pMTokens;
               /* to skip resetting pLastPtr below */
               continue;
            }
         }
      }
      pLastPtr = NULL;
   }
   while( hb_pp_tokenUnQuotedGet( &pTokenPtr, &fQuoted, FALSE ) );

   if( pOptional )
   {
      hb_pp_error( pState, 'E', HB_PP_ERR_UNCLOSED_OPTIONAL, NULL );
      return FALSE;
   }

   return TRUE;
}

static BOOL hb_pp_resultMarkerNew( PHB_PP_STATE pState,
                                   PHB_PP_TOKEN * pTokenPtr,
                                   PHB_PP_MARKERLST * pMarkerListPtr,
                                   BOOL fDump, BOOL fOptional,
                                   USHORT * pusPCount, USHORT spaces )
{
   USHORT         type        = HB_PP_TOKEN_NUL, rtype;
   PHB_PP_TOKEN   pMarkerId   = NULL, pToken;
   BOOL           fQuoted;

   /* At start pTokenPtr points to '<' token */
   if( hb_pp_tokenUnQuotedGet( &pTokenPtr, &fQuoted, TRUE ) && ! fQuoted )
   {
      rtype = HB_PP_TOKEN_TYPE( ( *pTokenPtr )->type );
      if( rtype == HB_PP_TOKEN_KEYWORD || rtype == HB_PP_TOKEN_STRING )
      {
         pMarkerId = *pTokenPtr;
         if( hb_pp_tokenUnQuotedGet( &pTokenPtr, &fQuoted, FALSE ) && ! fQuoted &&
             HB_PP_TOKEN_TYPE( ( *pTokenPtr )->type ) == HB_PP_TOKEN_GT )
         {
            if( rtype == HB_PP_TOKEN_STRING )
            {
               type = HB_PP_RMARKER_STRSTD;
               HB_PP_TOKEN_SETTYPE( pMarkerId, HB_PP_TOKEN_KEYWORD );
            }
            else
               type = fDump ? HB_PP_RMARKER_STRDUMP : HB_PP_RMARKER_REGULAR;
         }
      }
      else if( rtype == HB_PP_TOKEN_LEFT_PB )
      {
         if( hb_pp_tokenUnQuotedGet( &pTokenPtr, &fQuoted, TRUE ) && ! fQuoted &&
             HB_PP_TOKEN_TYPE( ( *pTokenPtr )->type ) == HB_PP_TOKEN_KEYWORD )
         {
            pMarkerId = *pTokenPtr;
            if( hb_pp_tokenUnQuotedGet( &pTokenPtr, &fQuoted, FALSE ) && ! fQuoted &&
                HB_PP_TOKEN_TYPE( ( *pTokenPtr )->type ) == HB_PP_TOKEN_RIGHT_PB &&
                hb_pp_tokenUnQuotedGet( &pTokenPtr, &fQuoted, TRUE ) && ! fQuoted &&
                HB_PP_TOKEN_TYPE( ( *pTokenPtr )->type ) == HB_PP_TOKEN_GT )
               type = HB_PP_RMARKER_STRSMART;
         }
      }
      else if( rtype == HB_PP_TOKEN_LEFT_CB )
      {
         if( hb_pp_tokenUnQuotedGet( &pTokenPtr, &fQuoted, TRUE ) && ! fQuoted &&
             HB_PP_TOKEN_TYPE( ( *pTokenPtr )->type ) == HB_PP_TOKEN_KEYWORD )
         {
            pMarkerId = *pTokenPtr;
            if( hb_pp_tokenUnQuotedGet( &pTokenPtr, &fQuoted, FALSE ) && ! fQuoted &&
                HB_PP_TOKEN_TYPE( ( *pTokenPtr )->type ) == HB_PP_TOKEN_RIGHT_CB &&
                hb_pp_tokenUnQuotedGet( &pTokenPtr, &fQuoted, TRUE ) && ! fQuoted &&
                HB_PP_TOKEN_TYPE( ( *pTokenPtr )->type ) == HB_PP_TOKEN_GT )
               type = HB_PP_RMARKER_BLOCK;
         }
      }
      else if( rtype == HB_PP_TOKEN_DOT )
      {
         if( hb_pp_tokenUnQuotedGet( &pTokenPtr, &fQuoted, TRUE ) && ! fQuoted &&
             HB_PP_TOKEN_TYPE( ( *pTokenPtr )->type ) == HB_PP_TOKEN_KEYWORD )
         {
            pMarkerId = *pTokenPtr;
            if( hb_pp_tokenUnQuotedGet( &pTokenPtr, &fQuoted, FALSE ) && ! fQuoted &&
                HB_PP_TOKEN_TYPE( ( *pTokenPtr )->type ) == HB_PP_TOKEN_DOT &&
                hb_pp_tokenUnQuotedGet( &pTokenPtr, &fQuoted, TRUE ) && ! fQuoted &&
                HB_PP_TOKEN_TYPE( ( *pTokenPtr )->type ) == HB_PP_TOKEN_GT )
               type = HB_PP_RMARKER_LOGICAL;
         }
      }
      else if( rtype == HB_PP_TOKEN_MINUS )
      {
         if( hb_pp_tokenUnQuotedGet( &pTokenPtr, &fQuoted, TRUE ) && ! fQuoted &&
             HB_PP_TOKEN_TYPE( ( *pTokenPtr )->type ) == HB_PP_TOKEN_KEYWORD )
         {
            pMarkerId = *pTokenPtr;
            if( hb_pp_tokenUnQuotedGet( &pTokenPtr, &fQuoted, FALSE ) && ! fQuoted )
            {
               /* <-id-> was bad choice for marker type because -> is single
                  ALIAS token so we have to add workaround for it now */
               if( HB_PP_TOKEN_TYPE( ( *pTokenPtr )->type ) == HB_PP_TOKEN_ALIAS ||
                   ( HB_PP_TOKEN_TYPE( ( *pTokenPtr )->type ) == HB_PP_TOKEN_MINUS &&
                     hb_pp_tokenUnQuotedGet( &pTokenPtr, &fQuoted, TRUE ) && ! fQuoted &&
                     HB_PP_TOKEN_TYPE( ( *pTokenPtr )->type ) == HB_PP_TOKEN_GT ) )
                  type = HB_PP_RMARKER_NUL;
            }
         }
      }
   }

   if( type == HB_PP_TOKEN_NUL )
   {
      hb_pp_error( pState, 'E', HB_PP_ERR_WRONG_LABEL, "" );
   }
   else
   {
      PHB_PP_MARKERLST pMrkLst = *pMarkerListPtr;

      while( pMrkLst && ! hb_pp_tokenEqual( pMrkLst->pMatchMarkers->pToken,
                                            pMarkerId, HB_PP_CMP_CASE ) )
      {
         pMrkLst = pMrkLst->pNext;
      }

      if( ! pMrkLst )
      {
         hb_pp_error( pState, 'E', HB_PP_ERR_UNKNOWN_RESULT_MARKER, pMarkerId->value );
      }
      else
      {
         if( ! pMrkLst->index )
            pMrkLst->index = ++( *pusPCount );
         if( ! fOptional )
            pMrkLst->canrepeat = FALSE;
         HB_PP_TOKEN_SETTYPE( pMarkerId, type );
         pMarkerId->index  = pMrkLst->index;
         pMarkerId->spaces = spaces;
         /* free the trailing '>' marker token */
         pToken            = *pTokenPtr;
         *pTokenPtr        = pToken->pNext;
         hb_pp_tokenFree( pToken );
         return TRUE;
      }
   }
   return FALSE;
}

static BOOL hb_pp_patternCompare( PHB_PP_TOKEN pToken1, PHB_PP_TOKEN pToken2, USHORT mode )
{
   while( pToken1 && pToken2 )
   {
      if( ! hb_pp_tokenEqual( pToken1, pToken2, mode ) )
         break;
      if( HB_PP_TOKEN_TYPE( pToken1->type ) == HB_PP_MMARKER_RESTRICT ||
          HB_PP_TOKEN_TYPE( pToken1->type ) == HB_PP_MMARKER_OPTIONAL ||
          HB_PP_TOKEN_TYPE( pToken1->type ) == HB_PP_RMARKER_OPTIONAL )
      {
         if( ! hb_pp_patternCompare( pToken1->pMTokens, pToken2->pMTokens, mode ) )
            break;
      }
      pToken1  = pToken1->pNext;
      pToken2  = pToken2->pNext;
   }
   return ! pToken1 && ! pToken2;
}

static void hb_pp_directiveDel( PHB_PP_STATE pState, PHB_PP_TOKEN pMatch,
                                USHORT markers, PHB_PP_MARKER pMarkers,
                                USHORT mode, BOOL fCommand )
{
   PHB_PP_RULE pRule, * pRulePtr = fCommand ? &pState->pCommands :
                                   &pState->pTranslations;

   while( *pRulePtr )
   {
      pRule = *pRulePtr;

      if( mode & HB_PP_CMP_KEY )
      {
         if( hb_pp_tokenEqual( pRule->pMatch, pMatch, mode ) )
            break;
      }
      else if( markers == 0 || pRule->markers == markers )
      {
         USHORT u;
         for( u = 0; u < markers; ++u )
         {
            if( pRule->pMarkers[ u ].canrepeat != pMarkers[ u ].canrepeat )
               break;
         }
         if( u == markers && hb_pp_patternCompare( pRule->pMatch, pMatch, mode ) )
         {
            *pRulePtr = pRule->pPrev;
            hb_pp_ruleFree( pRule );
            if( fCommand )
               pState->iCommands--;
            else
               pState->iTranslations--;
            return;
         }
      }
      pRulePtr = &pRule->pPrev;
   }
}

static void hb_pp_directiveNew( PHB_PP_STATE pState, PHB_PP_TOKEN pToken,
                                USHORT mode, BOOL fCommand, BOOL fDirect,
                                BOOL fDelete )
{
   PHB_PP_TOKEN   pResult, pMatch, pStart, pLast;
   BOOL           fValid = FALSE;

#ifdef HB_C52_STRICT
   HB_SYMBOL_UNUSED( fDirect );
#endif

   pMatch = pResult = pLast = NULL;
   if( pToken->pNext )
   {
      pStart = pToken->pNext;
      while( ! HB_PP_TOKEN_ISEOP( pStart, fDirect ) )
      {
         if( pMatch )
         {
            /* Clipper PP makes sth like that for result pattern of
             #[x]translate and #[x]command */
            if( pStart->spaces > 1 )
               pStart->spaces = 1;
         }
         else if( pStart->pNext &&
                  HB_PP_TOKEN_TYPE( pStart->type ) == HB_PP_TOKEN_EQ &&
                  HB_PP_TOKEN_TYPE( pStart->pNext->type ) == HB_PP_TOKEN_GT )
         {
            if( fDelete )
               hb_pp_error( pState, 'E', HB_PP_ERR_DELETE_SYNTAX, pToken->pNext->value );
            else
               fValid = TRUE;

            if( ! pLast )
               break;

            pLast->pNext   = NULL;
            pMatch         = pToken->pNext;
            pToken->pNext  = pStart;
            pToken         = pStart = pStart->pNext;
         }
         else if( fDelete && HB_PP_TOKEN_ISEOP( pStart->pNext, fDirect ) )
         {
            fValid   = TRUE;
            pMatch   = pToken->pNext;

            if( pStart->pNext )
            {
               pToken->pNext = pStart->pNext->pNext;
               hb_pp_tokenFree( pStart->pNext );
            }
            else
               pToken->pNext = NULL;

            pStart->pNext  = NULL;
            pLast          = pToken;
            break;
         }
         pLast    = pStart;
         pStart   = pStart->pNext;
      }
      if( pMatch && pLast != pToken )
      {
         pLast->pNext   = NULL;
         pResult        = pToken->pNext;
         pToken->pNext  = pStart;
      }
   }

   if( ! fValid )
   {
      hb_pp_error( pState, 'E', HB_PP_ERR_MISSING_PATTERN_SEP, NULL );
   }
   else if( pMatch ) /* isn't dummy directive? */
   {
      PHB_PP_MARKERLST  pMarkerList = NULL, pMrkLst;
      PHB_PP_MARKERPTR  pMrkPtr;
      PHB_PP_MARKER     pMarkers    = NULL;
      USHORT            usPCount    = 0;

      fValid = hb_pp_matchPatternNew( pState, &pMatch, &pMarkerList, NULL );
      if( fValid )
      {
         if( pResult )
         {
            PHB_PP_TOKEN * pTokenPtr, * pDumpPtr = NULL, * pOptStart = NULL;
            BOOL           fQuoted = FALSE;

            if( HB_PP_TOKEN_TYPE( pResult->type ) == HB_PP_TOKEN_BACKSLASH )
            {
               fQuoted  = TRUE;
               pLast    = pResult;
               pResult  = pResult->pNext;
               hb_pp_tokenFree( pLast );
            }
            pTokenPtr = &pResult;
            do
            {
               /*
                  Here we test for escaped multichar tokens such as:
                  \<<
                  which should really be processed as:
                  \< <
                  so we break the multi-char into multiple tokens.
                */
               if( fQuoted )
               {
                  if( HB_PP_TOKEN_TYPE( ( *pTokenPtr )->type ) == HB_PP_TOKEN_SHIFTL )
                  {
                     PHB_PP_TOKEN   pEscaped = hb_pp_tokenNew( "<", 1, ( *pTokenPtr )->spaces, HB_PP_TOKEN_LT | HB_PP_TOKEN_STATIC );
                     PHB_PP_TOKEN   pLT      = hb_pp_tokenNew( "<", 1, 0, HB_PP_TOKEN_LT | HB_PP_TOKEN_STATIC );

                     pEscaped->pNext   = pLT;
                     pLT->pNext        = ( *pTokenPtr )->pNext;

                     hb_pp_tokenFree( *pTokenPtr );
                     *pTokenPtr        = pEscaped;
                  }
                  /* Should not be needed for RESULT END of Marker!
                     else if( HB_PP_TOKEN_TYPE( ( * pTokenPtr )->type ) == HB_PP_TOKEN_SHIFTR )
                     {
                     PHB_PP_TOKEN pEscaped = hb_pp_tokenNew( ">", 1, ( * pTokenPtr )->spaces, HB_PP_TOKEN_GT | HB_PP_TOKEN_STATIC );
                     PHB_PP_TOKEN pGT      = hb_pp_tokenNew( ">", 1, 0,                       HB_PP_TOKEN_GT | HB_PP_TOKEN_STATIC );

                     pEscaped->pNext = pGT;
                     pGT->pNext      = ( * pTokenPtr )->pNext;

                     hb_pp_tokenFree( * pTokenPtr );
                   * pTokenPtr = pEscaped;
                     }
                   */
               }

               if( ! fQuoted )
               {
                  if( HB_PP_TOKEN_TYPE( ( *pTokenPtr )->type ) == HB_PP_TOKEN_HASH )
                  {
                     pDumpPtr = pTokenPtr;
                     /* to skip pDumpPtr reseting below */
                     continue;
                  }
                  else if( HB_PP_TOKEN_TYPE( ( *pTokenPtr )->type ) == HB_PP_TOKEN_LT )
                  {
                     USHORT spaces = ( *pTokenPtr )->spaces;
                     /* Free the string dump token: '#'. Clipper PP always
                        does it without checking type of next marker */
                     if( pDumpPtr )
                     {
                        pLast       = *pDumpPtr;
                        spaces      = pLast->spaces;
                        *pDumpPtr   = pLast->pNext;
                        hb_pp_tokenFree( pLast );
                        pTokenPtr   = pDumpPtr;
                     }

                     if( ! hb_pp_resultMarkerNew( pState, pTokenPtr, &pMarkerList,
                                                  pDumpPtr != NULL, pOptStart != NULL,
                                                  &usPCount, spaces ) )
                     {
                        fValid = FALSE;
                        break;
                     }
                     /* now pTokenPtr points to marker keyword, all other tokens
                        have been stripped */
                  }
                  else if( HB_PP_TOKEN_TYPE( ( *pTokenPtr )->type ) == HB_PP_TOKEN_LEFT_SB )
                  {
                     if( pOptStart )
                     {
                        fValid = FALSE;
                        hb_pp_error( pState, 'E', HB_PP_ERR_NESTED_OPTIONAL, NULL );
                        break;
                     }
                     pOptStart = pTokenPtr;
                  }
                  else if( HB_PP_TOKEN_TYPE( ( *pTokenPtr )->type ) == HB_PP_TOKEN_RIGHT_SB && pOptStart )
                  {
                     pLast                      = *pTokenPtr;
                     *pTokenPtr                 = NULL;
                     ( *pOptStart )->pMTokens   = ( *pOptStart )->pNext;
                     ( *pOptStart )->pNext      = pLast->pNext;
                     HB_PP_TOKEN_SETTYPE( *pOptStart, HB_PP_RMARKER_OPTIONAL );
#ifndef HB_C52_STRICT
                     /* This is not Clipper compatible but we have word
                        concatenation and without this modification we
                        will introduce very serious bug */
                     if( ( *pOptStart )->pMTokens &&
                         ( *pOptStart )->pMTokens->spaces == 0 &&
                         ( *pOptStart )->spaces > 0 &&
                         HB_PP_TOKEN_TYPE( ( *pOptStart )->pMTokens->type ) !=
                         HB_PP_TOKEN_COMMA )
                        ( *pOptStart )->pMTokens->spaces = 1;
#endif
                     pTokenPtr   = pOptStart;
                     pOptStart   = NULL;
                     hb_pp_tokenFree( pLast );
                  }
               }
               /* reset pDumpPtr */
               pDumpPtr = NULL;
            }
            while( hb_pp_tokenUnQuotedGet( &pTokenPtr, &fQuoted, FALSE ) );

            if( fValid && pOptStart )
            {
               fValid = FALSE;
               hb_pp_error( pState, 'E', HB_PP_ERR_UNKNOWN_RESULT_MARKER, NULL );
            }
         }
      }

      if( fValid && usPCount )
      {
         /* create regular match and result markers from parameters */
         pMarkers = ( PHB_PP_MARKER ) hb_xgrab( usPCount * sizeof( HB_PP_MARKER ) );
         memset( pMarkers, '\0', usPCount * sizeof( HB_PP_MARKER ) );
      }

      /* free marker index list */
      while( pMarkerList )
      {
         pMrkLst = pMarkerList;
         while( pMrkLst->pMatchMarkers )
         {
            pMrkPtr                 = pMrkLst->pMatchMarkers;
            pMrkLst->pMatchMarkers  = pMrkPtr->pNext;
            /* set match token type and parameters */
            if( pMarkers && pMrkLst->index )
            {
               pMarkers[ pMrkLst->index - 1 ].canrepeat  = pMrkLst->canrepeat;
               pMrkPtr->pToken->index                    = pMrkLst->index;
            }
            pMrkPtr->pToken->pMTokens = pMrkPtr->pMTokens;
            HB_PP_TOKEN_SETTYPE( pMrkPtr->pToken, pMrkPtr->type );
            hb_xfree( pMrkPtr );
         }
         pMarkerList = pMarkerList->pNext;
         hb_xfree( pMrkLst );
      }

      if( fValid )
      {
         if( fDelete )
         {
            hb_pp_directiveDel( pState, pMatch, usPCount, pMarkers, mode, fCommand );
            if( pMarkers )
               hb_xfree( pMarkers );
         }
         else
         {
            PHB_PP_RULE pRule;
            pRule = hb_pp_ruleNew( pMatch, pResult, mode, usPCount, pMarkers );
            if( fCommand )
            {
               pRule->pPrev      = pState->pCommands;
               pState->pCommands = pRule;
               pState->iCommands++;
               hb_pp_ruleSetId( pState, pMatch, HB_PP_COMMAND );
            }
            else
            {
               pRule->pPrev            = pState->pTranslations;
               pState->pTranslations   = pRule;
               pState->iTranslations++;
               hb_pp_ruleSetId( pState, pMatch, HB_PP_TRANSLATE );
            }
            pMatch = pResult = NULL;
         }
      }
   }
   hb_pp_tokenListFree( &pMatch );
   hb_pp_tokenListFree( &pResult );
}

static BOOL hb_pp_tokenStartExtBlock( PHB_PP_TOKEN * pTokenPtr )
{
   PHB_PP_TOKEN pToken = *pTokenPtr;

   if( pToken && HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_LEFT_ECB &&
       pToken->pNext && HB_PP_TOKEN_TYPE( pToken->pNext->type ) == HB_PP_TOKEN_PIPE )
   {
      USHORT prevtype = HB_PP_TOKEN_COMMA;
      pToken = pToken->pNext->pNext;
      while( pToken )
      {
         USHORT type = HB_PP_TOKEN_TYPE( pToken->type );
         if( ( ( type == HB_PP_TOKEN_KEYWORD || type == HB_PP_TOKEN_EPSILON ) &&
               prevtype == HB_PP_TOKEN_COMMA ) ||
             ( type == HB_PP_TOKEN_COMMA && prevtype == HB_PP_TOKEN_KEYWORD ) )
         {
            prevtype = type;
            pToken   = pToken->pNext;
         }
         else
            break;
      }
      if( pToken && pToken->pNext && HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_PIPE &&
          HB_PP_TOKEN_ISEOC( pToken->pNext ) )
      {
         *pTokenPtr = pToken->pNext;
         return TRUE;
      }
   }
   return FALSE;
}

static BOOL hb_pp_tokenStopExtBlock( PHB_PP_TOKEN * pTokenPtr )
{
   PHB_PP_TOKEN pToken = *pTokenPtr;

   if( HB_PP_TOKEN_ISEOC( pToken ) && pToken->pNext &&
       HB_PP_TOKEN_TYPE( pToken->pNext->type ) == HB_PP_TOKEN_RIGHT_ECB )
   {
      *pTokenPtr = pToken->pNext->pNext;
      return TRUE;
   }
   return FALSE;
}

static BOOL hb_pp_tokenSkipExp( PHB_PP_TOKEN * pTokenPtr, PHB_PP_TOKEN pStop,
                                USHORT mode, BOOL * pfStop )
{
   USHORT         curtype, prevtype = 0, lbrtype = 0, rbrtype = 0;
   PHB_PP_TOKEN   pToken   = *pTokenPtr, pPrev;
   int            iBraces  = 0;
   BOOL           fMatch;

   if( pfStop )
      *pfStop = FALSE;

   while( TRUE )
   {
      pPrev = pToken;
      if( hb_pp_tokenStartExtBlock( &pToken ) )
      {
         int iExtBlock = 1;
         while( pToken )
         {
            if( hb_pp_tokenStartExtBlock( &pToken ) )
               iExtBlock++;
            else if( hb_pp_tokenStopExtBlock( &pToken ) )
            {
               if( --iExtBlock == 0 )
                  break;
            }
            else
               pToken = pToken->pNext;
         }
         if( iExtBlock )
            pToken = pPrev;
      }

      if( HB_PP_TOKEN_ISEOC( pToken ) &&
          ( mode != HB_PP_CMP_ADDR || pToken == pStop ) )
      {
         if( pfStop )
            *pfStop = TRUE;
         break;
      }
      curtype = HB_PP_TOKEN_TYPE( pToken->type );
      if( iBraces )
      {
         if( curtype == lbrtype )
            ++iBraces;
         else if( curtype == rbrtype )
            --iBraces;
      }
      else if( mode == HB_PP_CMP_ADDR && pToken == pStop )
      {
         if( pfStop )
            *pfStop = TRUE;
         break;
      }
      else if( curtype == HB_PP_TOKEN_COMMA )
      {
         if( pfStop )
         {
            if( mode != HB_PP_CMP_ADDR && HB_PP_TOKEN_NEEDRIGHT( prevtype ) )
               *pfStop = TRUE;
            else
               pToken = pToken->pNext;
         }
         break;
      }
      else if( mode != HB_PP_CMP_ADDR &&
               ( HB_PP_TOKEN_CLOSE_BR( curtype ) ||
                 ( ! HB_PP_TOKEN_CANJOIN( curtype ) &&
                   ! HB_PP_TOKEN_CANJOIN( prevtype ) ) ||
                 ( HB_PP_TOKEN_NEEDRIGHT( prevtype ) &&
                   ! HB_PP_TOKEN_ISEXPTOKEN( pToken ) ) ||
                 ( pStop && hb_pp_tokenEqual( pToken, pStop, mode ) ) ) )
      {
         if( pfStop )
            *pfStop = TRUE;
         break;
      }
      else if( HB_PP_TOKEN_OPEN_BR( curtype ) )
      {
         lbrtype  = curtype;
         rbrtype  = ( curtype == HB_PP_TOKEN_LEFT_PB ? HB_PP_TOKEN_RIGHT_PB :
                      ( curtype == HB_PP_TOKEN_LEFT_SB ? HB_PP_TOKEN_RIGHT_SB :
                       HB_PP_TOKEN_RIGHT_CB ) );
         ++iBraces;
      }
      if( ! HB_PP_TOKEN_ISNEUTRAL( curtype ) )
         prevtype = curtype;
      pToken = pToken->pNext;
   }

   fMatch      = pToken != *pTokenPtr;
   *pTokenPtr  = pToken;

   return fMatch;
}

static BOOL hb_pp_tokenCanStartExp( PHB_PP_TOKEN pToken )
{
   if( ! HB_PP_TOKEN_NEEDLEFT( pToken ) && ! HB_PP_TOKEN_ISEOC( pToken ) )
   {
      if( HB_PP_TOKEN_TYPE( pToken->type ) != HB_PP_TOKEN_LEFT_SB )
         return TRUE;
      else
      {
         PHB_PP_TOKEN pEoc = NULL;

         pToken = pToken->pNext;
         while( ! HB_PP_TOKEN_ISEOL( pToken ) )
         {
            if( HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_RIGHT_SB )
            {
               if( pEoc )
               {
                  do
                  {
                     if( HB_PP_TOKEN_TYPE( pEoc->type ) == HB_PP_TOKEN_EOC )
                        HB_PP_TOKEN_SETTYPE( pEoc, HB_PP_TOKEN_TEXT );
                     pEoc = pEoc->pNext;
                  }
                  while( pEoc != pToken );
               }
               return TRUE;
            }
            if( ! pEoc && HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_EOC )
               pEoc = pToken;

            /* This is a workaround for when [...] is reverted into a literal string
               after it's been tokenized like array, f.e.
                  @ 0,0 SAY [---] */
            if( pToken->spaces == 0 )
               pToken->spaces = 0xFFFF;

            pToken = pToken->pNext;
         }
      }
   }
   return FALSE;
}

static BOOL hb_pp_tokenMatch( PHB_PP_TOKEN pMatch, PHB_PP_TOKEN * pTokenPtr,
                              PHB_PP_TOKEN pStop, USHORT mode )
{
   BOOL     fMatch = FALSE;
   USHORT   type;

   type = HB_PP_TOKEN_TYPE( pMatch->type );
   if( type == HB_PP_MMARKER_REGULAR )
   {
      if( hb_pp_tokenCanStartExp( *pTokenPtr ) )
      {
         if( ! pStop )
            pStop = pMatch->pNext;
         fMatch = hb_pp_tokenSkipExp( pTokenPtr, pStop, mode, NULL );
      }
   }
   else if( type == HB_PP_MMARKER_LIST )
   {
      if( hb_pp_tokenCanStartExp( *pTokenPtr ) )
      {
         BOOL fStop = FALSE;
         if( ! pStop )
            pStop = pMatch->pNext;
         do
         {
            if( ! hb_pp_tokenSkipExp( pTokenPtr, pStop, mode, &fStop ) )
               break;
            fMatch = TRUE;
         }
         while( ! fStop );
      }
   }
   else if( type == HB_PP_MMARKER_RESTRICT )
   {
      PHB_PP_TOKEN pRestrict = pMatch->pMTokens, pToken = *pTokenPtr;

      /*
       * Here we are strictly Clipper compatible. Clipper accepts dummy
       * restrict marker which starts from comma, <id: ,[ sth,...]>
       * which always match empty expression. The same effect can be
       * reached by giving ,, in the world list on other positions.
       */
      while( pRestrict )
      {
         if( HB_PP_TOKEN_TYPE( pRestrict->type ) == HB_PP_TOKEN_COMMA )
         {
            *pTokenPtr  = pToken;
            fMatch      = TRUE;
            break;
         }
         else if( HB_PP_TOKEN_TYPE( pRestrict->type ) == HB_PP_TOKEN_AMPERSAND &&
                  ( ! pRestrict->pNext ||
                    HB_PP_TOKEN_TYPE( pRestrict->pNext->type ) == HB_PP_TOKEN_COMMA ) &&
                  ( HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_MACROVAR ||
                    HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_MACROTEXT ||
                    ( HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_AMPERSAND &&
                      pToken->pNext &&
                      HB_PP_TOKEN_TYPE( pToken->pNext->type ) == HB_PP_TOKEN_LEFT_PB ) ) )
         {
            if( HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_MACROVAR ||
                HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_MACROTEXT )
            {
               *pTokenPtr = pToken->pNext;
            }
            else
            {
               int iBraces = 1;
               pToken = pToken->pNext->pNext;
               while( iBraces > 0 && ! HB_PP_TOKEN_ISEOC( pToken ) )
               {
                  if( HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_LEFT_PB )
                     ++iBraces;
                  else if( HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_RIGHT_PB )
                     --iBraces;
                  pToken = pToken->pNext;
               }
               *pTokenPtr = pToken;
            }
            fMatch = TRUE;
            break;
         }
         else if( ! HB_PP_TOKEN_ISEOC( pToken ) &&
                  hb_pp_tokenEqual( pToken, pRestrict, mode ) )
         {
            pToken      = pToken->pNext;
            pRestrict   = pRestrict->pNext;
            if( ! pRestrict )
            {
               *pTokenPtr  = pToken;
               fMatch      = TRUE;
               break;
            }
         }
         else
         {
            pToken = *pTokenPtr;
            do
            {
               type        = HB_PP_TOKEN_TYPE( pRestrict->type );
               pRestrict   = pRestrict->pNext;
            }
            while( pRestrict && type != HB_PP_TOKEN_COMMA );
         }
      }
   }
   else if( type == HB_PP_MMARKER_WILD )
   {
      /* TODO? now we are strictly Clipper compatible, but we may
         want to add some additional stop markers in the future here
         to support wild match markers also as not the last expression */
      if( ! HB_PP_TOKEN_ISEOS( *pTokenPtr ) )
      {
         fMatch = TRUE;
         do
         {
            *pTokenPtr = ( *pTokenPtr )->pNext;

            if( HB_PP_TOKEN_TYPE( ( *pTokenPtr )->type ) == HB_PP_TOKEN_EOC )
            {
               HB_PP_TOKEN_SETTYPE( *pTokenPtr, HB_PP_TOKEN_TEXT );
            }
         }
         while( ! HB_PP_TOKEN_ISEOS( *pTokenPtr ) );
      }
   }
   else if( type == HB_PP_MMARKER_EXTEXP )
   {
      if( HB_PP_TOKEN_TYPE( ( *pTokenPtr )->type ) != HB_PP_TOKEN_RIGHT_PB &&
          HB_PP_TOKEN_TYPE( ( *pTokenPtr )->type ) != HB_PP_TOKEN_RIGHT_SB &&
          HB_PP_TOKEN_TYPE( ( *pTokenPtr )->type ) != HB_PP_TOKEN_COMMA &&
          hb_pp_tokenCanStartExp( *pTokenPtr ) )
      {
         if( HB_PP_TOKEN_TYPE( ( *pTokenPtr )->type ) == HB_PP_TOKEN_LEFT_PB )
         {
            if( ! pStop )
               pStop = pMatch->pNext;
            fMatch = hb_pp_tokenSkipExp( pTokenPtr, pStop, mode, NULL );
         }
         else
         {
            do
            {
               *pTokenPtr = ( *pTokenPtr )->pNext;
            }
            while( ! HB_PP_TOKEN_ISEOC( *pTokenPtr ) &&
                   ( *pTokenPtr )->spaces == 0 &&
                   HB_PP_TOKEN_TYPE( ( *pTokenPtr )->type ) != HB_PP_TOKEN_COMMA );

            fMatch = TRUE;
         }
      }
   }
   else if( type == HB_PP_MMARKER_NAME )
   {
      if( HB_PP_TOKEN_TYPE( ( *pTokenPtr )->type ) == HB_PP_TOKEN_KEYWORD )
      {
         *pTokenPtr  = ( *pTokenPtr )->pNext;
         fMatch      = TRUE;
      }
   }
   else if( hb_pp_tokenEqual( *pTokenPtr, pMatch, mode ) )
   {
      *pTokenPtr  = ( *pTokenPtr )->pNext;
      fMatch      = TRUE;
   }

   return fMatch;
}

static BOOL hb_pp_patternMatch( PHB_PP_TOKEN pMatch, PHB_PP_TOKEN * pTokenPtr,
                                PHB_PP_TOKEN pStop,
                                USHORT mode, PHB_PP_RULE pRule )
{
   PHB_PP_TOKEN   pToken      = *pTokenPtr;
   PHB_PP_TOKEN   pFirst;
   BOOL           fOverflow   = FALSE;

   while( pMatch && ! HB_PP_TOKEN_ISEOS( pToken ) )
   {
      if( HB_PP_TOKEN_TYPE( pMatch->type ) == HB_PP_MMARKER_OPTIONAL )
      {
         PHB_PP_TOKEN pOptional = pMatch, pLast, pNewStop = pMatch->pNext;

         while( pNewStop && HB_PP_TOKEN_TYPE( pNewStop->type ) == HB_PP_MMARKER_OPTIONAL )
            pNewStop = pNewStop->pNext;

         do
         {
            pLast    = pOptional;
            pFirst   = pToken;
            if( hb_pp_patternMatch( pOptional->pMTokens, &pToken, pNewStop, mode, NULL ) &&
                pFirst != pToken )
            {
               if( pRule && ! hb_pp_patternMatch( pOptional->pMTokens, &pFirst, pNewStop, mode, pRule ) )
               {
                  fOverflow = TRUE;
                  break;
               }
               pOptional = pMatch;
            }
            else
               pOptional = pOptional->pNext;
         }
         while( pOptional && HB_PP_TOKEN_TYPE( pOptional->type ) == HB_PP_MMARKER_OPTIONAL &&
                ! HB_PP_TOKEN_ISEOS( pToken ) );
         pMatch = pLast;
      }
      else
      {
         pFirst = pToken;
         if( hb_pp_tokenMatch( pMatch, &pToken, pStop, mode ) )
         {
            if( pRule && pMatch->index && pFirst != pToken )
            {
               if( ! hb_pp_patternAddResult( pRule, pMatch->index, pFirst, pToken ) )
               {
                  fOverflow = TRUE;
                  break;
               }
            }
         }
         else
            break;
      }

      pMatch = pMatch->pNext;
   }

   if( ! fOverflow )
   {
      while( pMatch && HB_PP_TOKEN_TYPE( pMatch->type ) == HB_PP_MMARKER_OPTIONAL )
         pMatch = pMatch->pNext;
      if( pMatch == NULL )
      {
         *pTokenPtr = pToken;
         if( pRule )
            pRule->pNextExpr = pToken;
         return TRUE;
      }
   }
   return FALSE;
}

static BOOL hb_pp_patternCmp( PHB_PP_RULE pRule, PHB_PP_TOKEN pToken,
                              BOOL fCommand )
{
   PHB_PP_TOKEN pFirst = pToken;

   if( hb_pp_patternMatch( pRule->pMatch, &pToken, NULL,
                           HB_PP_CMP_MODE( pRule->mode ), NULL ) )
   {
      if( ! fCommand || HB_PP_TOKEN_ISEOC( pToken ) )
      {
         if( hb_pp_patternMatch( pRule->pMatch, &pFirst, NULL,
                                 HB_PP_CMP_MODE( pRule->mode ), pRule ) )
            return TRUE;
         else
            hb_pp_patternClearResults( pRule );
      }
   }
   return FALSE;
}

static PHB_PP_RESULT hb_pp_matchResultGet( PHB_PP_RULE pRule, USHORT usMatch,
                                           USHORT usIndex )
{
   PHB_PP_MARKER  pMarker = &pRule->pMarkers[ usIndex - 1 ];
   PHB_PP_RESULT  pMarkerResult;

   /* Clipper PP does not check status of match marker but only how many
      different values were assigned to match pattern */
   if( pMarker->matches == 1 )
      pMarkerResult = pMarker->pResult;
   else if( usMatch < pMarker->matches )
   {
      pMarkerResult = pMarker->pResult;
      while( usMatch-- )
         pMarkerResult = pMarkerResult->pNext;
   }
   else
      pMarkerResult = NULL;

   return pMarkerResult;
}

static PHB_PP_TOKEN * hb_pp_matchResultLstAdd( PHB_PP_STATE pState,
                                               USHORT spaces, USHORT type,
                                               PHB_PP_TOKEN * pResultPtr,
                                               PHB_PP_TOKEN pToken,
                                               PHB_PP_TOKEN pStop )
{
   PHB_PP_TOKEN   pNext;
   BOOL           fFirst = TRUE, fStop = FALSE;

   while( TRUE )
   {
      pNext = pToken;
      if( hb_pp_tokenSkipExp( &pNext, pStop, HB_PP_CMP_ADDR, &fStop ) &&
          ( fStop ? pToken : pToken->pNext ) != pNext )
      {
         /* Check for '&' token followed by single keyword or '('
            token and do not stringify such expressions but
            clone them */
         if( type == HB_PP_RMARKER_BLOCK )
         {
            BOOL fBlock = HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_LEFT_CB &&
                          pToken->pNext &&
                          ( fStop ? pToken->pNext : pToken->pNext->pNext ) != pNext &&
                          HB_PP_TOKEN_TYPE( pToken->pNext->type ) == HB_PP_TOKEN_PIPE;

#ifdef __XHARBOUR__
            if( ! fBlock )
            {
               fBlock = HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_LEFT_ECB &&
                        pToken->pNext &&
                        ( fStop ? pToken->pNext : pToken->pNext->pNext ) != pNext &&
                        HB_PP_TOKEN_TYPE( pToken->pNext->type ) == HB_PP_TOKEN_PIPE;
            }
#endif

            if( ! fBlock )
            {
               hb_pp_tokenAdd( &pResultPtr, "{", 1, fFirst ? spaces : 1,
                               HB_PP_TOKEN_LEFT_CB | HB_PP_TOKEN_STATIC );
               hb_pp_tokenAdd( &pResultPtr, "|", 1, 0, HB_PP_TOKEN_PIPE | HB_PP_TOKEN_STATIC );
               hb_pp_tokenAdd( &pResultPtr, "|", 1, 0, HB_PP_TOKEN_PIPE | HB_PP_TOKEN_STATIC );
               fFirst = FALSE;
            }
            do
            {
               *pResultPtr = hb_pp_tokenClone( pToken );
               if( fFirst )
               {
                  ( *pResultPtr )->spaces = spaces;
                  fFirst                  = FALSE;
               }
               pResultPtr  = &( *pResultPtr )->pNext;
               pToken      = pToken->pNext;
            }
            while( ( fStop ? pToken : pToken->pNext ) != pNext );
            if( ! fBlock )
               hb_pp_tokenAdd( &pResultPtr, "}", 1, 0, HB_PP_TOKEN_RIGHT_CB | HB_PP_TOKEN_STATIC );
         }
         else if( ( HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_MACROVAR ||
                    HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_MACROTEXT ) &&
                  ( fStop ? pToken->pNext : pToken->pNext->pNext ) == pNext )
         {
            if( HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_MACROVAR )
            {
               hb_pp_tokenAdd( &pResultPtr, pToken->value + 1, pToken->len -
                               ( pToken->value[ pToken->len - 1 ] == '.' ? 2 : 1 ),
                               fFirst ? spaces : pToken->spaces,
                               HB_PP_TOKEN_KEYWORD );
            }
            else
            {
               hb_membufFlush( pState->pBuffer );
               hb_pp_tokenStr( pToken, pState->pBuffer, FALSE, FALSE, 0 );
               hb_pp_tokenAdd( &pResultPtr,
                               hb_membufPtr( pState->pBuffer ),
                               hb_membufLen( pState->pBuffer ),
                               fFirst ? spaces : pToken->spaces,
                               HB_PP_TOKEN_STRING );
            }
            pToken   = pToken->pNext;
            fFirst   = FALSE;
         }
         else if( ( type == HB_PP_RMARKER_STRSMART &&
                    ( HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_STRING ||
                      HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_LEFT_PB ) ) ||
                  ( HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_AMPERSAND &&
                    pToken->pNext &&
                    ( fStop ? pToken->pNext : pToken->pNext->pNext ) != pNext &&
                    HB_PP_TOKEN_TYPE( pToken->pNext->type ) == HB_PP_TOKEN_LEFT_PB ) )
         {
            if( HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_AMPERSAND )
               pToken = pToken->pNext;
            do
            {
               *pResultPtr = hb_pp_tokenClone( pToken );
               if( fFirst )
               {
                  ( *pResultPtr )->spaces = spaces;
                  fFirst                  = FALSE;
               }
               pResultPtr  = &( *pResultPtr )->pNext;
               pToken      = pToken->pNext;
            }
            while( ( fStop ? pToken : pToken->pNext ) != pNext );
         }
         else
         {
            /* leading spaces calculation in Clipper is broken when
               separate tokens are stringified, it can be quite
               easy checked that it will interact with translation
               done just before - spaces are partially inherited.
               It means that Clipper PP does not clear some static
               buffers where holds this information.
               I decided to keep original internal spacing except the
               first token */
            BOOL fSpaces = FALSE;
            if( ! fFirst )
               spaces = pToken->spaces;
            hb_membufFlush( pState->pBuffer );
            do
            {
               hb_pp_tokenStr( pToken, pState->pBuffer, fSpaces, FALSE, 0 );
               fSpaces  = TRUE;
               pToken   = pToken->pNext;
            }
            while( ( fStop ? pToken : pToken->pNext ) != pNext );
            hb_pp_tokenAdd( &pResultPtr,
                            hb_membufPtr( pState->pBuffer ),
                            hb_membufLen( pState->pBuffer ),
                            spaces, HB_PP_TOKEN_STRING );
            fFirst = FALSE;
         }
      }
      if( fStop )
         break;
      /* clone comma token */
      *pResultPtr = hb_pp_tokenClone( pToken );
      if( fFirst )
      {
         ( *pResultPtr )->spaces = spaces;
         fFirst                  = FALSE;
      }
      pResultPtr  = &( *pResultPtr )->pNext;
      pToken      = pNext;
   }

   return pResultPtr;
}

static PHB_PP_TOKEN * hb_pp_matchResultAdd( PHB_PP_STATE pState,
                                            PHB_PP_RULE pRule, PHB_PP_TOKEN * pResultPtr,
                                            PHB_PP_TOKEN pMatch, USHORT usMatch )
{
   PHB_PP_RESULT  pMarkerResult = hb_pp_matchResultGet( pRule, usMatch, pMatch->index );
   PHB_PP_TOKEN   pToken, pStop;
   BOOL           fSpaces, fFirst;

   if( HB_PP_TOKEN_TYPE( pMatch->type ) == HB_PP_RMARKER_REGULAR )
   {
      if( pMarkerResult )
      {
         pToken   = pMarkerResult->pFirstToken;
         pStop    = pMarkerResult->pNextExpr;
         fFirst   = TRUE;
         if( pToken != pStop )
         {
            do
            {
               *pResultPtr = hb_pp_tokenClone( pToken );
               if( fFirst )
               {
                  ( *pResultPtr )->spaces = pMatch->spaces;
                  fFirst                  = FALSE;
               }
               pResultPtr  = &( *pResultPtr )->pNext;
               pToken      = pToken->pNext;
            }
            while( pToken != pStop );
         }
      }
   }
   else if( HB_PP_TOKEN_TYPE( pMatch->type ) == HB_PP_RMARKER_STRDUMP )
   {
      hb_membufFlush( pState->pBuffer );
      if( pMarkerResult )
      {
         pToken   = pMarkerResult->pFirstToken;
         pStop    = pMarkerResult->pNextExpr;
         if( pToken != pStop )
         {
            fSpaces = FALSE;
            do
            {
               hb_pp_tokenStr( pToken, pState->pBuffer, fSpaces, FALSE, 0 );
               fSpaces  = TRUE;
               pToken   = pToken->pNext;
            }
            while( pToken != pStop );
         }
      }
      hb_pp_tokenAdd( &pResultPtr, hb_membufPtr( pState->pBuffer ),
                      hb_membufLen( pState->pBuffer ),
                      pMatch->spaces, HB_PP_TOKEN_STRING );
   }
   else if( HB_PP_TOKEN_TYPE( pMatch->type ) == HB_PP_RMARKER_STRSTD ||
            HB_PP_TOKEN_TYPE( pMatch->type ) == HB_PP_RMARKER_STRSMART ||
            HB_PP_TOKEN_TYPE( pMatch->type ) == HB_PP_RMARKER_BLOCK )
   {
      if( pMarkerResult )
      {
         pToken   = pMarkerResult->pFirstToken;
         pStop    = pMarkerResult->pNextExpr;
         /* We have to divide the expression to comma separated ones */
         if( pToken != pStop )
         {
            pResultPtr = hb_pp_matchResultLstAdd( pState, pMatch->spaces,
                                                  HB_PP_TOKEN_TYPE( pMatch->type ), pResultPtr, pToken, pStop );
         }
      }
   }
   else if( HB_PP_TOKEN_TYPE( pMatch->type ) == HB_PP_RMARKER_LOGICAL )
   {
      /* Clipper documentation is wrong and Clipper PP only checks
         if such pattern was assigned not is non empty */
      hb_pp_tokenAdd( &pResultPtr, pMarkerResult ? ".T." : ".F.", 3,
                      pMatch->spaces, HB_PP_TOKEN_LOGICAL | HB_PP_TOKEN_STATIC );
   }
   else if( HB_PP_TOKEN_TYPE( pMatch->type ) == HB_PP_RMARKER_NUL )
   {
      /* nothing to stuff */
   }
   else
   {
      /* TODO? internal error? */
   }

   return pResultPtr;
}

static PHB_PP_TOKEN *  hb_pp_patternStuff( PHB_PP_STATE pState,
                                           PHB_PP_RULE pRule, USHORT usMatch,
                                           PHB_PP_TOKEN pResultPattern,
                                           PHB_PP_TOKEN * pResultPtr )
{
   while( pResultPattern )
   {
      if( pResultPattern->index )
      {
         pResultPtr = hb_pp_matchResultAdd( pState, pRule, pResultPtr, pResultPattern, usMatch );
      }
      else if( HB_PP_TOKEN_TYPE( pResultPattern->type ) == HB_PP_RMARKER_OPTIONAL )
      {
         USHORT         usMaxMatch  = 0, matches;
         PHB_PP_TOKEN   pToken      = pResultPattern->pMTokens;
         while( pToken )
         {
            if( pToken->index )
            {
               matches = pRule->pMarkers[ pToken->index - 1 ].matches;
               if( matches > usMaxMatch )
                  usMaxMatch = matches;
            }
            pToken = pToken->pNext;
         }
         for( matches = 0; matches < usMaxMatch; ++matches )
         {
            pResultPtr = hb_pp_patternStuff( pState, pRule, matches,
                                             pResultPattern->pMTokens,
                                             pResultPtr );
         }
      }
      else
      {
         *pResultPtr = hb_pp_tokenClone( pResultPattern );
         pResultPtr  = &( *pResultPtr )->pNext;
      }
      pResultPattern = pResultPattern->pNext;
   }

   return pResultPtr;
}

static char * hb_pp_tokenListStr( PHB_PP_TOKEN pToken, PHB_PP_TOKEN pStop,
                                  BOOL fStop, PHB_MEM_BUFFER pBuffer,
                                  BOOL fQuote, BOOL fEol )
{
   USHORT   ltype    = HB_PP_TOKEN_NUL;
   BOOL     fSpaces  = FALSE;

   hb_membufFlush( pBuffer );
   while( pToken && ( fStop ? pToken != pStop : ! HB_PP_TOKEN_ISEOC( pToken ) ) )
   {
      hb_pp_tokenStr( pToken, pBuffer, fSpaces, fQuote, ltype );
      ltype    = HB_PP_TOKEN_TYPE( pToken->type );
      fSpaces  = TRUE;
      pToken   = pToken->pNext;
   }
   if( fEol )
      hb_membufAddCh( pBuffer, '\n' );
   hb_membufAddCh( pBuffer, '\0' );

   return hb_membufPtr( pBuffer );
}

static void hb_pp_patternReplace( PHB_PP_STATE pState, PHB_PP_RULE pRule,
                                  PHB_PP_TOKEN * pTokenPtr, char * szType )
{
   PHB_PP_TOKEN pFinalResult = NULL, * pResultPtr, pToken, pSource;

   pResultPtr  = hb_pp_patternStuff( pState, pRule, 0, pRule->pResult, &pFinalResult );

   /* store original matched token pointer */
   pSource     = *pTokenPtr;

   /* Copy number of leading spaces from the first matched token
      to the first result token */
   if( pFinalResult && pSource )
      pFinalResult->spaces = pSource->spaces;

   /* Write trace information */
   if( pState->fWriteTrace )
   {
      fprintf( pState->file_trace, "%s(%d) >%s<\n",
               pState->pFile && pState->pFile->szFileName ? pState->pFile->szFileName : "",
               pState->pFile ? pState->pFile->iCurrentLine : 0,
               /* the source string */
               hb_pp_tokenListStr( pSource, pRule->pNextExpr, TRUE,
                                   pState->pBuffer, TRUE, FALSE ) );
      fprintf( pState->file_trace, "#%s%s >%s<\n",
               pRule->mode == HB_PP_CMP_STD ? "x" : "", szType,
               /* the result string */
               hb_pp_tokenListStr( pFinalResult, *pResultPtr, TRUE,
                                   pState->pBuffer, TRUE, FALSE ) );
   }

   /* Replace matched tokens with result pattern */
   *pResultPtr = pRule->pNextExpr;
   *pTokenPtr  = pFinalResult;

   /* Free the matched tokens */
   while( pSource != pRule->pNextExpr )
   {
      pToken   = pSource;
      pSource  = pSource->pNext;
      hb_pp_tokenFree( pToken );
   }

   hb_pp_patternClearResults( pRule );
}

static void hb_pp_processCondDefined( PHB_PP_STATE pState, PHB_PP_TOKEN pToken )
{
   PHB_PP_TOKEN pNext;

   while( ! HB_PP_TOKEN_ISEOS( pToken ) )
   {
      pNext = pToken->pNext;
      if( HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_KEYWORD &&
          hb_pp_tokenValueCmp( pToken, "defined", HB_PP_CMP_CASE ) &&
          pNext && HB_PP_TOKEN_TYPE( pNext->type ) == HB_PP_TOKEN_LEFT_PB &&
          pNext->pNext && HB_PP_TOKEN_TYPE( pNext->pNext->type ) == HB_PP_TOKEN_KEYWORD &&
          pNext->pNext->pNext && HB_PP_TOKEN_TYPE( pNext->pNext->pNext->type ) == HB_PP_TOKEN_RIGHT_PB )
      {
         BOOL fDefined = hb_pp_defineFind( pState, pNext->pNext, NULL ) != NULL;

         hb_pp_tokenSetValue( pToken, fDefined ? "1" : "0", 1 );
         HB_PP_TOKEN_SETTYPE( pToken, HB_PP_TOKEN_NUMBER );
         pToken->pNext              = pNext->pNext->pNext->pNext;
         pNext->pNext->pNext->pNext = NULL;
         hb_pp_tokenListFree( &pNext );
      }
      pToken = pToken->pNext;
   }
}

static BOOL hb_pp_processDefine( PHB_PP_STATE pState, PHB_PP_TOKEN * pFirstPtr )
{
   PHB_PP_TOKEN * pPrevPtr;
   BOOL           fSubst   = FALSE, fRepeat;
   int            iCycle   = 0;
   PHB_PP_RULE    pRule;

   do
   {
      pPrevPtr = NULL;
      fRepeat  = FALSE;
      while( ! HB_PP_TOKEN_ISEOS( *pFirstPtr ) )
      {
         if( HB_PP_TOKEN_TYPE( ( *pFirstPtr )->type ) == HB_PP_TOKEN_KEYWORD &&
             ( pState->pMap[ HB_PP_HASHID( *pFirstPtr ) ] & HB_PP_DEFINE ) )
         {
            pRule = NULL;

           #ifndef HB_C52_STRICT
 overloadDefine:
           #endif

            pRule = hb_pp_defineFind( pState, *pFirstPtr, pRule );

            if( pRule )
            {
               if( hb_pp_patternCmp( pRule, *pFirstPtr, FALSE ) )
               {
                  hb_pp_patternReplace( pState, pRule, pFirstPtr, "define" );
                  fSubst = fRepeat = TRUE;
                  if( ++pState->iCycle > pState->iMaxCycles ||
                      ++iCycle > HB_PP_MAX_REPATS + pState->iDefinitions )
                  {
                     pState->iCycle = pState->iMaxCycles + 1;
                     hb_pp_error( pState, 'E', HB_PP_ERR_CYCLIC_DEFINE, pRule->pMatch->value );
                     return TRUE;
                  }
                  continue;
               }

               #ifndef HB_C52_STRICT
               pRule = pRule->pPrev;
               if( pRule )
                  goto overloadDefine;
               #endif

               if( ! pPrevPtr )
                  pPrevPtr = pFirstPtr;
            }
         }
         iCycle      = 0;
         pFirstPtr   = &( *pFirstPtr )->pNext;
      }
      pFirstPtr = pPrevPtr;
   }
   while( pFirstPtr && fRepeat );

   return fSubst;
}

static BOOL hb_pp_processTranslate( PHB_PP_STATE pState, PHB_PP_TOKEN * pFirstPtr )
{
   PHB_PP_TOKEN * pTokenPtr;
   BOOL           fSubst   = FALSE, fRepeat;
   int            iCycle   = 0;

   do
   {
      pTokenPtr   = pFirstPtr;
      fRepeat     = FALSE;
      while( ! HB_PP_TOKEN_ISEOS( *pTokenPtr ) )
      {
         if( pState->pMap[ HB_PP_HASHID( *pTokenPtr ) ] & HB_PP_TRANSLATE )
         {
            PHB_PP_RULE pRule = pState->pTranslations;
            while( pRule )
            {
               if( hb_pp_patternCmp( pRule, *pTokenPtr, FALSE ) )
               {
                  hb_pp_patternReplace( pState, pRule, pTokenPtr, "translate" );
                  fSubst = fRepeat = TRUE;
                  if( ++pState->iCycle > pState->iMaxCycles ||
                      ++iCycle > HB_PP_MAX_REPATS + pState->iTranslations )
                  {
                     pState->iCycle = pState->iMaxCycles + 1;
                     hb_pp_error( pState, 'E', HB_PP_ERR_CYCLIC_TRANSLATE, pRule->pMatch->value );
                     return TRUE;
                  }
                  pRule = pState->pTranslations;
                  continue;
               }
               pRule = pRule->pPrev;
            }
         }
         iCycle      = 0;
         pTokenPtr   = &( *pTokenPtr )->pNext;
      }
   }
   while( fRepeat );

   return fSubst;
}

static BOOL hb_pp_processCommand( PHB_PP_STATE pState, PHB_PP_TOKEN * pFirstPtr )
{
   PHB_PP_RULE pRule;
   BOOL        fSubst   = FALSE, fRepeat = TRUE;
   int         iCycle   = 0;

   while( fRepeat && ! HB_PP_TOKEN_ISEOC( *pFirstPtr ) &&
          pState->pMap[ HB_PP_HASHID( *pFirstPtr ) ] & HB_PP_COMMAND )
   {
      fRepeat  = FALSE;
      pRule    = pState->pCommands;
      while( pRule )
      {
         if( hb_pp_patternCmp( pRule, *pFirstPtr, TRUE ) )
         {
            hb_pp_patternReplace( pState, pRule, pFirstPtr, "command" );
            fSubst = fRepeat = TRUE;
            if( ++pState->iCycle > pState->iMaxCycles ||
                ++iCycle > HB_PP_MAX_REPATS + pState->iCommands )
            {
               pState->iCycle = pState->iMaxCycles + 1;
               hb_pp_error( pState, 'E', HB_PP_ERR_CYCLIC_COMMAND, pRule->pMatch->value );
               return TRUE;
            }
            break;
         }
         pRule = pRule->pPrev;
      }
   }

   /* This is strictly compatible with Clipper PP which internally supports
         text <!linefunc!>,<!endfunc!>
      as stream begin directive */
   if( ! HB_PP_TOKEN_ISEOC( *pFirstPtr ) &&
       hb_pp_tokenValueCmp( *pFirstPtr, "TEXT", HB_PP_CMP_DBASE ) )
   {
      PHB_PP_TOKEN pToken = ( *pFirstPtr )->pNext, * pFuncPtr;

      if( pToken &&
          HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_KEYWORD &&
          pToken->pNext &&
          HB_PP_TOKEN_TYPE( pToken->pNext->type ) == HB_PP_TOKEN_COMMA &&
          pToken->pNext->pNext &&
          HB_PP_TOKEN_TYPE( pToken->pNext->pNext->type ) == HB_PP_TOKEN_KEYWORD &&
          HB_PP_TOKEN_ISEOC( pToken->pNext->pNext->pNext ) )
      {
         hb_pp_tokenListFree( &pState->pFuncOut );
         hb_pp_tokenListFree( &pState->pFuncEnd );

         pFuncPtr = &pState->pFuncOut;
         hb_pp_tokenAdd( &pFuncPtr, pToken->value, pToken->len, 0, HB_PP_TOKEN_KEYWORD );
         hb_pp_tokenAdd( &pFuncPtr, "(", 1, 0, HB_PP_TOKEN_LEFT_PB | HB_PP_TOKEN_STATIC );
         hb_pp_tokenAdd( &pFuncPtr, "%", 1, 1, HB_PP_RMARKER_STRDUMP | HB_PP_TOKEN_STATIC );
         hb_pp_tokenAdd( &pFuncPtr, ")", 1, 1, HB_PP_TOKEN_RIGHT_PB | HB_PP_TOKEN_STATIC );

         pToken   = pToken->pNext->pNext;
         pFuncPtr = &pState->pFuncEnd;
         hb_pp_tokenAdd( &pFuncPtr, pToken->value, pToken->len, 0, HB_PP_TOKEN_KEYWORD );
         hb_pp_tokenAdd( &pFuncPtr, "(", 1, 0, HB_PP_TOKEN_LEFT_PB | HB_PP_TOKEN_STATIC );
         hb_pp_tokenAdd( &pFuncPtr, ")", 1, 1, HB_PP_TOKEN_RIGHT_PB | HB_PP_TOKEN_STATIC );
         pState->iStreamDump  = HB_PP_STREAM_CLIPPER;
         hb_pp_tokenListFreeCmd( pFirstPtr, pState );
         fSubst               = TRUE;
      }
   }

   return fSubst;
}

static BOOL hb_pp_concatenateKeywords( PHB_PP_STATE pState, PHB_PP_TOKEN * pFirstPtr )
{
   PHB_PP_TOKEN   pToken   = *pFirstPtr, pNext;
   BOOL           fChanged = FALSE;

   while( pToken && pToken->pNext )
   {
      pNext = pToken->pNext;
      if( HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_KEYWORD &&
          pNext->spaces == 0 &&
          HB_PP_TOKEN_TYPE( pNext->type ) == HB_PP_TOKEN_KEYWORD )
      {
         hb_membufFlush( pState->pBuffer );
         hb_membufAddData( pState->pBuffer, pToken->value, pToken->len );
         hb_membufAddData( pState->pBuffer, pNext->value, pNext->len );

         /* Write trace information */
         if( pState->fWriteTrace )
         {
            fprintf( pState->file_trace, "%s(%d) >%s %s<\n(concatenate) >%s%s<\n",
                     pState->pFile && pState->pFile->szFileName ? pState->pFile->szFileName : "",
                     pState->pFile ? pState->pFile->iCurrentLine : 0,
                     pToken->value, pNext->value,
                     pToken->value, pNext->value );
         }

         hb_pp_tokenSetValue( pToken, hb_membufPtr( pState->pBuffer ),
                              hb_membufLen( pState->pBuffer ) );
         pToken->pNext  = pNext->pNext;
         hb_pp_tokenFree( pNext );
         fChanged       = TRUE;
      }
      else
         pToken = pNext;
   }

   return fChanged;
}

static PHB_PP_TOKEN hb_pp_calcPrecedence( PHB_PP_TOKEN pToken,
                                          int * piNextOper, int * piNextPrec )
{
   PHB_PP_TOKEN pNext = pToken->pNext;

   *piNextOper = HB_PP_TOKEN_TYPE( pToken->type );
   switch( *piNextOper )
   {
      /* not */
      case HB_PP_TOKEN_NOT:
         *piNextPrec = HB_PP_PREC_NOT;
         break;

      /* relational */
      case HB_PP_TOKEN_EQUAL:
      case HB_PP_TOKEN_HASH:
      case HB_PP_TOKEN_NE:
      case HB_PP_TOKEN_LE:
      case HB_PP_TOKEN_GE:
      case HB_PP_TOKEN_LT:
      case HB_PP_TOKEN_GT:
         *piNextPrec = HB_PP_PREC_REL;
         break;

      /* logical */
      case HB_PP_TOKEN_AND:
      case HB_PP_TOKEN_OR:
         *piNextPrec = HB_PP_PREC_LOG;
         break;

      /* bit */
      case HB_PP_TOKEN_PIPE:
         *piNextPrec = HB_PP_PREC_BIT;
         if( pNext && HB_PP_TOKEN_TYPE( pNext->type ) == HB_PP_TOKEN_PIPE &&
             pNext->spaces == 0 )
         {
            *piNextOper = HB_PP_TOKEN_OR;
            pNext       = pNext->pNext;
         }
         break;
      case HB_PP_TOKEN_AMPERSAND:
         *piNextPrec = HB_PP_PREC_BIT;
         /* It will not work because && will be stripped as comment */
         if( pNext && HB_PP_TOKEN_TYPE( pNext->type ) == HB_PP_TOKEN_AMPERSAND &&
             pNext->spaces == 0 )
         {
            *piNextOper = HB_PP_TOKEN_AND;
            pNext       = pNext->pNext;
         }
         break;
      case HB_PP_TOKEN_POWER:
         *piNextPrec = HB_PP_PREC_BIT;
         break;

      case HB_PP_TOKEN_BITXOR:
      case HB_PP_TOKEN_SHIFTL:
      case HB_PP_TOKEN_SHIFTR:
         *piNextPrec = HB_PP_PREC_BIT;
         break;

      /* math plus/minus */
      case HB_PP_TOKEN_PLUS:
      case HB_PP_TOKEN_MINUS:
         *piNextPrec = HB_PP_PREC_PLUS;
         break;

      /* math mult/div/mode */
      case HB_PP_TOKEN_MULT:
      case HB_PP_TOKEN_DIV:
      case HB_PP_TOKEN_MOD:
         *piNextPrec = HB_PP_PREC_MULT;
         break;

      default:
         *piNextPrec = HB_PP_PREC_NUL;
         break;
   }

   return pNext;
}

static HB_LONG hb_pp_calcOperation( HB_LONG lValueLeft, HB_LONG lValueRight,
                                    int iOperation )
{
   switch( iOperation )
   {
      case HB_PP_TOKEN_EQUAL:
         lValueLeft = ( lValueLeft == lValueRight ) ? 1 : 0;
         break;
      case HB_PP_TOKEN_HASH:
      case HB_PP_TOKEN_NE:
         lValueLeft  = ( lValueLeft != lValueRight ) ? 1 : 0;
         break;
      case HB_PP_TOKEN_LE:
         lValueLeft  = ( lValueLeft <= lValueRight ) ? 1 : 0;
         break;
      case HB_PP_TOKEN_GE:
         lValueLeft  = ( lValueLeft >= lValueRight ) ? 1 : 0;
         break;
      case HB_PP_TOKEN_LT:
         lValueLeft  = ( lValueLeft < lValueRight ) ? 1 : 0;
         break;
      case HB_PP_TOKEN_GT:
         lValueLeft  = ( lValueLeft > lValueRight ) ? 1 : 0;
         break;

      case HB_PP_TOKEN_AND:
         lValueLeft  = ( lValueLeft && lValueRight ) ? 1 : 0;
         break;
      case HB_PP_TOKEN_OR:
         lValueLeft  = ( lValueLeft || lValueRight ) ? 1 : 0;
         break;

      case HB_PP_TOKEN_PIPE:
         lValueLeft  |= lValueRight;
         break;
      case HB_PP_TOKEN_AMPERSAND:
         lValueLeft  &= lValueRight;
         break;
      case HB_PP_TOKEN_POWER:
      case HB_PP_TOKEN_BITXOR:
         lValueLeft  ^= lValueRight;
         break;
      case HB_PP_TOKEN_SHIFTL:
         lValueLeft  <<= lValueRight;
         break;
      case HB_PP_TOKEN_SHIFTR:
         lValueLeft  >>= lValueRight;
         break;

      case HB_PP_TOKEN_PLUS:
         lValueLeft  += lValueRight;
         break;
      case HB_PP_TOKEN_MINUS:
         lValueLeft  -= lValueRight;
         break;
      case HB_PP_TOKEN_MULT:
         lValueLeft  *= lValueRight;
         break;
      case HB_PP_TOKEN_DIV:
         lValueLeft  /= lValueRight;
         break;
      case HB_PP_TOKEN_MOD:
         lValueLeft  %= lValueRight;
         break;
   }

   return lValueLeft;
}

static PHB_PP_TOKEN hb_pp_calcValue( PHB_PP_TOKEN pToken, int iPrecedense,
                                     HB_LONG * plValue, BOOL * pfError )
{
   if( HB_PP_TOKEN_ISEOC( pToken ) )
      *pfError = TRUE;
   else if( HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_MINUS )
   {
      pToken   = hb_pp_calcValue( pToken->pNext, HB_PP_PREC_NEG, plValue, pfError );
      *plValue = -*plValue;
   }
   else if( HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_PLUS )
   {
      pToken = hb_pp_calcValue( pToken->pNext, iPrecedense, plValue, pfError );
   }
   else if( HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_NOT )
   {
      pToken   = hb_pp_calcValue( pToken->pNext, HB_PP_PREC_NOT, plValue, pfError );
      *plValue = *plValue ? 0 : 1;
   }
   else if( HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_LEFT_PB )
   {
      *pfError = TRUE;
      pToken   = hb_pp_calcValue( pToken->pNext, HB_PP_PREC_NUL, plValue, pfError );
      if( ! *pfError && ! HB_PP_TOKEN_ISEOC( pToken ) &&
          HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_RIGHT_PB )
         pToken = pToken->pNext;
      else
         *pfError = TRUE;
   }
   else if( HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_RIGHT_PB )
   {
      return pToken;
   }
   else if( HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_NUMBER )
   {
      int iOverflow;
      *plValue = hb_strValInt( pToken->value, &iOverflow );
      if( iOverflow )
         *pfError = TRUE;
      else
      {
         *pfError = FALSE;
         pToken   = pToken->pNext;
      }
   }
   else if( HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_LOGICAL )
   {
      *plValue = HB_PP_ISTRUE( pToken->value[ 1 ] ) ? 1 : 0;
      *pfError = FALSE;
      pToken   = pToken->pNext;
   }
   else
      *pfError = TRUE;

   while( ! ( *pfError || HB_PP_TOKEN_ISEOC( pToken ) ||
              HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_RIGHT_PB ) )
   {
      int            iNextOper, iNextPrec;
      PHB_PP_TOKEN   pNext;
      pNext = hb_pp_calcPrecedence( pToken, &iNextOper, &iNextPrec );
      if( iNextPrec < HB_PP_PREC_LOG )
         *pfError = TRUE;
      else if( iNextPrec > iPrecedense )
      {
         HB_LONG lValue = 0;
         *pfError = TRUE;
         pToken   = hb_pp_calcValue( pNext, iNextPrec, &lValue, pfError );
         if( ! *pfError )
         {
            *plValue = hb_pp_calcOperation( *plValue, lValue, iNextOper );
         }
      }
      else
         break;
   }

   return pToken;
}

static HB_LONG hb_pp_calculateValue( PHB_PP_STATE pState, PHB_PP_TOKEN pToken )
{
   BOOL     fError   = TRUE;
   HB_LONG  lValue   = 0;

   pToken = hb_pp_calcValue( pToken, HB_PP_PREC_NUL, &lValue, &fError );
   if( ! HB_PP_TOKEN_ISEOC( pToken ) )
      fError = TRUE;

   if( fError )
   {
      hb_pp_error( pState, 'E', HB_PP_ERR_DIRECTIVE_IF, NULL );
      lValue = 0;
   }

   return lValue;
}

static void hb_pp_conditionPush( PHB_PP_STATE pState, BOOL fCond )
{
   if( pState->iCondCount == pState->iCondStackSize )
   {
      pState->iCondStackSize += 5;
      if( pState->pCondStack )
         pState->pCondStack = ( int * ) hb_xrealloc( pState->pCondStack,
                                                     pState->iCondStackSize * sizeof( BOOL ) );
      else
         pState->pCondStack = ( int * ) hb_xgrab( pState->iCondStackSize *
                                                  sizeof( BOOL ) );
   }
   pState->pCondStack[ pState->iCondCount++ ]   = pState->iCondCompile;
   pState->iCondCompile                         = pState->iCondCompile ? HB_PP_COND_DISABLE :
                                                  ( fCond ? 0 : HB_PP_COND_ELSE );
}

static void hb_pp_condCompile( PHB_PP_STATE pState, PHB_PP_TOKEN pToken,
                               BOOL fNot )
{
   if( ! pToken || HB_PP_TOKEN_TYPE( pToken->type ) != HB_PP_TOKEN_KEYWORD ||
       ! HB_PP_TOKEN_ISEOC( pToken->pNext ) )
   {
      hb_pp_error( pState, 'E', HB_PP_ERR_DIRECTIVE_IFDEF, NULL );
   }
   else
   {
      BOOL fCond = FALSE;

      if( pState->iCondCompile == 0 )
      {
         fCond = hb_pp_defineFind( pState, pToken, NULL ) != NULL;
         if( ! fNot )
            fCond = ! fCond;
      }
      hb_pp_conditionPush( pState, fCond );
   }
}

static void hb_pp_condCompileIf( PHB_PP_STATE pState, PHB_PP_TOKEN pToken )
{
   /* preprocess all define(s) */
   hb_pp_processCondDefined( pState, pToken->pNext );
   hb_pp_processDefine( pState, &pToken->pNext );
   hb_pp_conditionPush( pState, hb_pp_calculateValue( pState, pToken->pNext ) != 0 );
}

static void hb_pp_condCompileElif( PHB_PP_STATE pState, PHB_PP_TOKEN pToken )
{
   if( ( pState->iCondCompile & HB_PP_COND_DISABLE ) == 0 )
   {
      if( pState->iCondCompile )
      {
         /* preprocess all define(s) */
         hb_pp_processCondDefined( pState, pToken->pNext );
         hb_pp_processDefine( pState, &pToken->pNext );
         if( hb_pp_calculateValue( pState, pToken->pNext ) != 0 )
            pState->iCondCompile ^= HB_PP_COND_ELSE;
      }
      else
         pState->iCondCompile = HB_PP_COND_DISABLE;
   }
}

#if ! defined( HB_PP_NO_LINEINFO_TOKEN )
static void hb_pp_lineTokens( PHB_PP_TOKEN ** pTokenPtr, char * szFileName, int iLine )
{
   char szLine[ 12 ];

   hb_snprintf( szLine, sizeof( szLine ), "%d", iLine );
   hb_pp_tokenAdd( pTokenPtr, "#", 1, 0, HB_PP_TOKEN_DIRECTIVE | HB_PP_TOKEN_STATIC );
   hb_pp_tokenAdd( pTokenPtr, "line", 4, 0, HB_PP_TOKEN_KEYWORD | HB_PP_TOKEN_STATIC );
   hb_pp_tokenAdd( pTokenPtr, szLine, strlen( szLine ), 1, HB_PP_TOKEN_NUMBER );
   if( szFileName )
      hb_pp_tokenAdd( pTokenPtr, szFileName, strlen( szFileName ), 1, HB_PP_TOKEN_STRING );
   hb_pp_tokenAdd( pTokenPtr, "\n", 1, 0, HB_PP_TOKEN_EOL | HB_PP_TOKEN_STATIC );
}
#endif

static void hb_pp_genLineTokens( PHB_PP_STATE pState )
{
   pState->pNextTokenPtr = &pState->pTokenOut;

#if defined( HB_PP_NO_LINEINFO_TOKEN )
   hb_pp_tokenMoveCommand( pState->pNextTokenPtr, &pState->pFile->pTokenList );
#else
   if( pState->pFile->fGenLineInfo )
   {
      hb_pp_lineTokens( &pState->pNextTokenPtr, pState->pFile->szFileName,
                        pState->pFile->iCurrentLine );
      pState->pFile->fGenLineInfo = FALSE;
   }
   else if( pState->pFile->iLastLine < pState->pFile->iCurrentLine )
   {
      do
      {
         hb_pp_tokenAdd( &pState->pNextTokenPtr, "\n", 1, 0, HB_PP_TOKEN_EOL | HB_PP_TOKEN_STATIC );
      }
      while( ++pState->pFile->iLastLine < pState->pFile->iCurrentLine );
   }
   pState->pFile->iLastLine = pState->pFile->iCurrentLine +
                              hb_pp_tokenMoveCommand( pState->pNextTokenPtr,
                                                      &pState->pFile->pTokenList );
#endif
}

static void hb_pp_includeFile( PHB_PP_STATE pState, char * szFileName, BOOL fSysFile )
{
   if( pState->iFiles >= HB_PP_MAX_INCLUDED_FILES )
   {
      hb_pp_error( pState, 'F', HB_PP_ERR_NESTED_INCLUDES, NULL );
   }
   else
   {
      BOOL        fNested  = FALSE;
      PHB_PP_FILE pFile    = hb_pp_FileNew( pState, szFileName, fSysFile, &fNested,
                                            NULL, TRUE, pState->pOpenFunc );
      if( pFile )
      {
#if defined( HB_PP_STRICT_LINEINFO_TOKEN )
         pState->pNextTokenPtr = &pState->pTokenOut;
         if( pState->pFile->fGenLineInfo )
         {
            hb_pp_lineTokens( &pState->pNextTokenPtr, pState->pFile->szFileName,
                              pState->pFile->iCurrentLine );
            pState->pFile->fGenLineInfo = FALSE;
         }
         hb_pp_lineTokens( &pState->pNextTokenPtr, szFileName, 1 );
#else
         pFile->fGenLineInfo  = TRUE;
#endif
         pFile->pPrev         = pState->pFile;
         pState->pFile        = pFile;
         pState->iFiles++;
      }
      else if( fNested )
         hb_pp_error( pState, 'F', HB_PP_ERR_NESTED_INCLUDES, NULL );
      else
         hb_pp_error( pState, 'F', HB_PP_ERR_CANNOT_OPEN_FILE, szFileName );
   }
}

static void hb_pp_includeClose( PHB_PP_STATE pState )
{
   PHB_PP_FILE pFile = pState->pFile;

   pState->pFile = pFile->pPrev;
   pState->iFiles--;

#if defined( HB_PP_STRICT_LINEINFO_TOKEN )
   if( pFile->fGenLineInfo )
   {
      pState->pNextTokenPtr = &pState->pTokenOut;
      hb_pp_lineTokens( &pState->pNextTokenPtr, pFile->szFileName, pFile->iCurrentLine + 1 );
   }
#endif
   if( pState->pFile )
      pState->pFile->fGenLineInfo = TRUE;

   hb_pp_FileFree( pState, pFile, pState->pCloseFunc );
}

static void hb_pp_preprocessToken( PHB_PP_STATE pState )
{
   while( ! pState->pTokenOut && pState->pFile )
   {
      PHB_PP_TOKEN pToken;

      if( ! pState->pFile->pTokenList )
      {
         while( pState->pFile->pLineBuf ? pState->pFile->ulLineBufLen != 0 :
                ! pState->pFile->fEof )
         {
            hb_pp_getLine( pState );
            if( pState->pFile->pTokenList /* || pState->fError */ )
               break;
         }

         if( ! pState->pFile->pTokenList )
         {
            if( pState->pFile->pLineBuf )
               break;
            /* this condition is only for compiler core code compatibility */
            if( ! pState->pFile->pPrev )
               break;
            hb_pp_includeClose( pState );
            continue;
         }
      }

      if( HB_PP_TOKEN_ISDIRECTIVE( pState->pFile->pTokenList ) )
      {
         BOOL           fError   = FALSE, fDirect;
         /* Store it here to avoid possible problems after #INCLUDE */
         PHB_PP_TOKEN * pFreePtr = &pState->pFile->pTokenList;

         pToken   = pState->pFile->pTokenList;
         fDirect  = HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_DIRECTIVE;
         pToken   = pToken->pNext;
         if( ! pToken )
         {
            fError = TRUE;
         }
#ifndef HB_C52_STRICT
         /* Harbour PP extension */
         else if( fDirect && pState->pFile->iCurrentLine == 1 &&
                  HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_NOT &&
                  pToken->spaces == 0 && pState->pFile->pTokenList->spaces == 0 )
         {
            /* ignore first line if it begins with "#!"
               minor extension which allow to use the same source code
               as scripts in *nix system and compile it, this feature
               will be necessary also when we integrate compiler with HVM and
               add support for direct execution compiled .prg files */
         }
#endif
         else if( HB_PP_TOKEN_TYPE( pToken->type ) != HB_PP_TOKEN_KEYWORD )
         {
            fError = TRUE;
         }
         else if( hb_pp_tokenValueCmp( pToken, "IFDEF", HB_PP_CMP_DBASE ) )
         {
            hb_pp_condCompile( pState, pToken->pNext, TRUE );
         }
         else if( hb_pp_tokenValueCmp( pToken, "IFNDEF", HB_PP_CMP_DBASE ) )
         {
            hb_pp_condCompile( pState, pToken->pNext, FALSE );
         }
#ifndef HB_C52_STRICT
         /* xHarbour PP extension */
         else if( hb_pp_tokenValueCmp( pToken, "IF", HB_PP_CMP_DBASE ) )
         {
            hb_pp_condCompileIf( pState, pToken );
         }
         else if( hb_pp_tokenValueCmp( pToken, "ELIF", HB_PP_CMP_DBASE ) )
         {
            if( pState->iCondCount )
               hb_pp_condCompileElif( pState, pToken );
            else
               hb_pp_error( pState, 'E', HB_PP_ERR_DIRECTIVE_ELSE, NULL );
         }
#endif
         else if( hb_pp_tokenValueCmp( pToken, "ENDIF", HB_PP_CMP_DBASE ) )
         {
            if( pState->iCondCount )
               pState->iCondCompile = pState->pCondStack[ --pState->iCondCount ];
            else
               hb_pp_error( pState, 'E', HB_PP_ERR_DIRECTIVE_ENDIF, NULL );
         }
         else if( hb_pp_tokenValueCmp( pToken, "ELSE", HB_PP_CMP_DBASE ) )
         {
            if( pState->iCondCount )
               pState->iCondCompile ^= HB_PP_COND_ELSE;
            else
               hb_pp_error( pState, 'E', HB_PP_ERR_DIRECTIVE_ELSE, NULL );
         }
         /* #pragma support is always enabled even in strict compatibility
            mode to allow control by programmer some PP issues */
         else if( hb_pp_tokenValueCmp( pToken, "PRAGMA", HB_PP_CMP_DBASE ) )
         {
            hb_pp_pragmaNew( pState, pToken->pNext );
         }
         else if( pState->iCondCompile )
         {
            /* conditional compilation - other preprocessing and output disabled */
         }
         else if( hb_pp_tokenValueCmp( pToken, "INCLUDE", HB_PP_CMP_DBASE ) )
         {
            pToken = pToken->pNext;
            if( pToken && HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_STRING )
               hb_pp_includeFile( pState, pToken->value, FALSE );
            else if( pToken && HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_LT )
            {
               pToken = pToken->pNext;
               hb_membufFlush( pState->pBuffer );
               while( ! HB_PP_TOKEN_ISEOC( pToken ) &&
                      HB_PP_TOKEN_TYPE( pToken->type ) != HB_PP_TOKEN_GT )
               {
                  hb_membufAddData( pState->pBuffer, pToken->value, pToken->len );
                  pToken = pToken->pNext;
               }
               if( hb_membufLen( pState->pBuffer ) > 0 &&
                   ! HB_PP_TOKEN_ISEOC( pToken ) &&
                   HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_GT )
               {
                  hb_membufAddCh( pState->pBuffer, '\0' );
                  hb_pp_includeFile( pState, hb_membufPtr( pState->pBuffer ), TRUE );
               }
               else
                  hb_pp_error( pState, 'F', HB_PP_ERR_WRONG_FILE_NAME, NULL );
            }
            else
               hb_pp_error( pState, 'F', HB_PP_ERR_WRONG_FILE_NAME, NULL );
         }
         else if( hb_pp_tokenValueCmp( pToken, "STDOUT", HB_PP_CMP_DBASE ) )
         {
            hb_pp_disp( pState, hb_pp_tokenListStr( pToken->pNext, NULL, FALSE,
                                                    pState->pBuffer, FALSE, TRUE ) );
         }
         else if( hb_pp_tokenValueCmp( pToken, "ERROR", HB_PP_CMP_DBASE ) )
         {
            hb_pp_error( pState, 'E', HB_PP_ERR_EXPLICIT,
                         hb_pp_tokenListStr( pToken->pNext, NULL, FALSE,
                                             pState->pBuffer, FALSE, FALSE ) );
         }
         else if( hb_pp_tokenValueCmp( pToken, "DEFINE", HB_PP_CMP_DBASE ) )
         {
            hb_pp_defineNew( pState, pToken, fDirect );
         }
         else if( hb_pp_tokenValueCmp( pToken, "UNDEF", HB_PP_CMP_DBASE ) )
         {
            pToken = pToken->pNext;
            if( ! pToken || HB_PP_TOKEN_TYPE( pToken->type ) != HB_PP_TOKEN_KEYWORD ||
                ! HB_PP_TOKEN_ISEOC( pToken->pNext ) )
               hb_pp_error( pState, 'E', HB_PP_ERR_DIRECTIVE_UNDEF, NULL );
            else
               hb_pp_defineDel( pState, pToken );
         }
         else if( hb_pp_tokenValueCmp( pToken, "TRANSLATE", HB_PP_CMP_DBASE ) )
         {
            hb_pp_directiveNew( pState, pToken, HB_PP_CMP_DBASE, FALSE, fDirect, FALSE );
         }
         else if( hb_pp_tokenValueCmp( pToken, "XTRANSLATE", HB_PP_CMP_DBASE ) )
         {
            hb_pp_directiveNew( pState, pToken, HB_PP_CMP_STD, FALSE, fDirect, FALSE );
         }
#ifndef HB_C52_STRICT
         else if( hb_pp_tokenValueCmp( pToken, "YTRANSLATE", HB_PP_CMP_DBASE ) )
         {
            hb_pp_directiveNew( pState, pToken, HB_PP_CMP_CASE, FALSE, fDirect, FALSE );
         }
#endif
         else if( hb_pp_tokenValueCmp( pToken, "COMMAND", HB_PP_CMP_DBASE ) )
         {
            hb_pp_directiveNew( pState, pToken, HB_PP_CMP_DBASE, TRUE, fDirect, FALSE );
         }
         else if( hb_pp_tokenValueCmp( pToken, "XCOMMAND", HB_PP_CMP_DBASE ) )
         {
            hb_pp_directiveNew( pState, pToken, HB_PP_CMP_STD, TRUE, fDirect, FALSE );
         }
#ifndef HB_C52_STRICT
         else if( hb_pp_tokenValueCmp( pToken, "YCOMMAND", HB_PP_CMP_DBASE ) )
         {
            hb_pp_directiveNew( pState, pToken, HB_PP_CMP_CASE, TRUE, fDirect, FALSE );
         }
         /* xHarbour PP extensions */
         else if( hb_pp_tokenValueCmp( pToken, "UNTRANSLATE", HB_PP_CMP_DBASE ) )
         {
            hb_pp_directiveNew( pState, pToken, HB_PP_CMP_KEY | HB_PP_CMP_STD, FALSE, fDirect, TRUE );
         }
         else if( hb_pp_tokenValueCmp( pToken, "XUNTRANSLATE", HB_PP_CMP_DBASE ) )
         {
            hb_pp_directiveNew( pState, pToken, HB_PP_CMP_MATCHRULE | HB_PP_CMP_STD, FALSE, fDirect, TRUE );
         }
         else if( hb_pp_tokenValueCmp( pToken, "YUNTRANSLATE", HB_PP_CMP_DBASE ) )
         {
            hb_pp_directiveNew( pState, pToken, HB_PP_CMP_CASE, FALSE, fDirect, TRUE );
         }
         else if( hb_pp_tokenValueCmp( pToken, "UNCOMMAND", HB_PP_CMP_DBASE ) )
         {
            hb_pp_directiveNew( pState, pToken, HB_PP_CMP_KEY | HB_PP_CMP_STD, TRUE, fDirect, TRUE );
         }
         else if( hb_pp_tokenValueCmp( pToken, "XUNCOMMAND", HB_PP_CMP_DBASE ) )
         {
            hb_pp_directiveNew( pState, pToken, HB_PP_CMP_MATCHRULE | HB_PP_CMP_STD, TRUE, fDirect, TRUE );
         }
         else if( hb_pp_tokenValueCmp( pToken, "YUNCOMMAND", HB_PP_CMP_DBASE ) )
         {
            hb_pp_directiveNew( pState, pToken, HB_PP_CMP_CASE, TRUE, fDirect, TRUE );
         }
         /* Clipper PP does not accept #line and generate error */
         else if( hb_pp_tokenValueCmp( pToken, "LINE", HB_PP_CMP_DBASE ) )
         {
            /* ignore #line directives */
         }
         else if( hb_pp_tokenValueCmp( pToken, "MESSAGE", HB_PP_CMP_DBASE ) )
         {
            pToken = pToken->pNext;
            if( pToken && HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_STRING )
            {
#if defined( __HB_COMPILER__ )
               hb_snprintf( hb_comp_szMsgBuf, SIZE_OF_SZMSGBUF, "%s\n", pToken->value );
               hb_compOutStd( hb_comp_szMsgBuf );
#else
               printf( "%s\n", pToken->value );
#endif
            }
         }
#endif
         else
            fError = TRUE;

         if( fError )
            hb_pp_error( pState, 'F', HB_PP_ERR_INVALID_DIRECTIVE, NULL );
         hb_pp_tokenListFreeCmd( pFreePtr, pState );
         continue;
      }
      else if( pState->iCondCompile )
      {
         hb_pp_tokenListFreeCmd( &pState->pFile->pTokenList, pState );
      }
      else
      {
         BOOL fDirective = FALSE;

         pState->iCycle = 0;
         while( ! HB_PP_TOKEN_ISEOC( pState->pFile->pTokenList ) &&
                pState->iCycle <= pState->iMaxCycles )
         {
            if( HB_PP_TOKEN_ISDIRECTIVE( pState->pFile->pTokenList ) )
            {
               fDirective = TRUE;
               break;
            }
#ifndef HB_C52_STRICT
            /* Harbour extension: concatenate keywords without spaces between
               them */
            hb_pp_concatenateKeywords( pState, &pState->pFile->pTokenList );
#endif
            if( hb_pp_processDefine( pState, &pState->pFile->pTokenList ) )
               continue;
            if( hb_pp_processTranslate( pState, &pState->pFile->pTokenList ) )
               continue;
            if( hb_pp_processCommand( pState, &pState->pFile->pTokenList ) )
               continue;
            break;
         }
         if( ! fDirective && pState->pFile->pTokenList )
            hb_pp_genLineTokens( pState );
      }
   }
}

/*
 * exported functions
 */

/*
 * internal function to initialize predefined PP rules
 */
void hb_pp_initRules( PHB_PP_RULE * pRulesPtr, int * piRules,
                      const HB_PP_DEFRULE pDefRules[], int iDefRules )
{
   PHB_PP_DEFRULE pDefRule;
   PHB_PP_MARKER  pMarkers;
   PHB_PP_RULE    pRule;

   hb_pp_ruleListFree( pRulesPtr );
   *piRules = iDefRules;

   while( --iDefRules >= 0 )
   {
      pDefRule = ( PHB_PP_DEFRULE ) pDefRules + iDefRules;
      if( pDefRule->markers > 0 )
      {
         USHORT   marker;
         ULONG    ulBit;

         pMarkers = ( PHB_PP_MARKER ) hb_xgrab( pDefRule->markers * sizeof( HB_PP_MARKER ) );
         memset( pMarkers, '\0', pDefRule->markers * sizeof( HB_PP_MARKER ) );
         for( marker = 0, ulBit = 1; marker < pDefRule->markers; ++marker, ulBit <<= 1 )
         {
            if( pDefRule->repeatbits & ulBit )
               pMarkers[ marker ].canrepeat = TRUE;
         }
      }
      else
         pMarkers = NULL;
      pRule          = hb_pp_ruleNew( pDefRule->pMatch, pDefRule->pResult,
                                      pDefRule->mode, pDefRule->markers, pMarkers );
      pRule->pPrev   = *pRulesPtr;
      *pRulesPtr     = pRule;
   }
}


/*
 * get preprocessed token
 */
PHB_PP_TOKEN hb_pp_tokenGet( PHB_PP_STATE pState )
{
   pState->fError = FALSE;

   if( pState->pTokenOut )
   {
      PHB_PP_TOKEN pToken = pState->pTokenOut;
      pState->pTokenOut = pToken->pNext;
      hb_pp_tokenFree( pToken );
   }

   if( ! pState->pTokenOut )
      hb_pp_preprocessToken( pState );

   if( pState->fWritePreprocesed && pState->pTokenOut )
   {
      hb_membufFlush( pState->pBuffer );
#if defined( HB_PP_NO_LINEINFO_TOKEN )
      if( pState->pFile->fGenLineInfo )
      {
         fprintf( pState->file_out, "#line %d", pState->pFile->iCurrentLine );
         if( pState->pFile->szFileName )
            fprintf( pState->file_out, " \"%s\"", pState->pFile->szFileName );
         fputc( '\n', pState->file_out );
         pState->pFile->fGenLineInfo = FALSE;
      }
      else if( pState->pFile->iLastLine < pState->pFile->iCurrentLine )
      {
         int i = pState->pFile->iLastLine;

         do
         {
            fputc( '\n', pState->file_out );
         }
         while( ++i < pState->pFile->iCurrentLine );
      }
      /*pState->pFile->iLastLine = pState->pFile->iCurrentLine +*/
      hb_pp_tokenStr( pState->pTokenOut, pState->pBuffer, TRUE, TRUE,
                      pState->iLastType );
#else
      hb_pp_tokenStr( pState->pTokenOut, pState->pBuffer, TRUE, TRUE,
                      pState->iLastType );
#endif
      pState->iLastType = HB_PP_TOKEN_TYPE( pState->pTokenOut->type );
      fwrite( hb_membufPtr( pState->pBuffer ), sizeof( char ),
              ( size_t ) hb_membufLen( pState->pBuffer ), pState->file_out );
   }

   return pState->pTokenOut;
}


/*
 * create new PP context
 */
PHB_PP_STATE hb_pp_new( void )
{
   return hb_pp_stateNew();
}

/*
 * free PP context
 */
void hb_pp_free( PHB_PP_STATE pState )
{
   hb_pp_stateFree( pState );
}

/*
 * initialize PP context
 */
void hb_pp_init( PHB_PP_STATE pState, BOOL fQuiet, int iCycles, void * cargo,
                 PHB_PP_OPEN_FUNC pOpenFunc, PHB_PP_CLOSE_FUNC pCloseFunc,
                 PHB_PP_ERROR_FUNC pErrorFunc, PHB_PP_DISP_FUNC pDispFunc,
                 PHB_PP_DUMP_FUNC pDumpFunc, PHB_PP_INLINE_FUNC pInLineFunc,
                 PHB_PP_SWITCH_FUNC pSwitchFunc )
{
   pState->fQuiet       = fQuiet;
   pState->iMaxCycles   = ( iCycles > 0 ) ? iCycles : HB_PP_MAX_CYCLES;
   pState->cargo        = cargo;
   pState->pOpenFunc    = pOpenFunc;
   pState->pCloseFunc   = pCloseFunc;
   pState->pErrorFunc   = pErrorFunc;
   pState->pDispFunc    = pDispFunc;
   pState->pDumpFunc    = pDumpFunc;
   pState->pInLineFunc  = pInLineFunc;
   pState->pSwitchFunc  = pSwitchFunc;
}

/*
 * reset PP context, used for multiple .prg file compilation
 * with DO ... or *.clp files
 */
void hb_pp_reset( PHB_PP_STATE pState )
{
   pState->fError    = FALSE;
   pState->iErrors   = 0;

   hb_pp_InFileFree( pState );
   hb_pp_OutFileFree( pState );
   hb_pp_TraceFileFree( pState );

   hb_pp_ruleListNonStdFree( &pState->pDefinitions );
   hb_pp_ruleListNonStdFree( &pState->pTranslations );
   hb_pp_ruleListNonStdFree( &pState->pCommands );
}

/*
 * add search path for included files
 */
void hb_pp_addSearchPath( PHB_PP_STATE pState, const char * szPath, BOOL fReplace )
{
   if( fReplace && pState->pIncludePath )
   {
      hb_fsFreeSearchPath( pState->pIncludePath );
      pState->pIncludePath = NULL;
   }

   if( szPath && *szPath )
   {
      hb_fsAddSearchPath( szPath, &pState->pIncludePath );
   }
}

/*
 * mark current rules as standard ones
 */
void hb_pp_setStdBase( PHB_PP_STATE pState )
{
   pState->fError = FALSE;
   hb_pp_ruleListSetStd( pState->pDefinitions );
   hb_pp_ruleListSetStd( pState->pTranslations );
   hb_pp_ruleListSetStd( pState->pCommands );
   memset( pState->pMap, 0, sizeof( pState->pMap ) );
   hb_pp_ruleListSetId( pState, pState->pDefinitions, HB_PP_DEFINE );
   hb_pp_ruleListSetId( pState, pState->pTranslations, HB_PP_TRANSLATE );
   hb_pp_ruleListSetId( pState, pState->pCommands, HB_PP_COMMAND );

   /* clear total number of preprocessed lines so we will report only
    * lines in compiled .prg files
    */
   pState->iLineTot = 0;
}

/*
 * initialize dynamic definitions
 */
void hb_pp_initDynDefines( PHB_PP_STATE pState )
{
   char     szDefine[ 65 ];
   char     szResult[ 65 ];
   char *   pSrc, * pDst, * szPlatform;
   int      iYear, iMonth, iDay, i;

   /* __PLATFORM__* */
   pSrc  = szPlatform = hb_verPlatform();
   pDst  = hb_strncpy( szDefine, "__PLATFORM__", sizeof( szDefine ) - 1 );
   i     = 12;
   while( pSrc[ 0 ] > ' ' && i < ( int ) sizeof( szDefine ) - 1 )
   {
      if( HB_PP_ISNEXTIDCHAR( pSrc[ 0 ] ) )
         pDst[ i++ ] = pSrc[ 0 ];
      pSrc++;
   }
   pDst[ i ]   = '\0';

   i           = 0;
   pDst        = szResult;
   pDst[ i++ ] = '"';
   if( pSrc[ 0 ] == ' ' )
   {
      while( *( ++pSrc ) && i < ( int ) sizeof( szResult ) - 2 )
         pDst[ i++ ] = pSrc[ 0 ];
   }
   pDst[ i++ ] = '"';
   pDst[ i ]   = '\0';

   hb_xfree( szPlatform );

   hb_pp_addDefine( pState, szDefine, szResult );
#ifdef HB_OS_UNIX
   hb_strncpy( szDefine + 12, "UNIX", sizeof( szDefine ) - 13 );
   hb_pp_addDefine( pState, szDefine, szResult );
#endif

#if defined( __HARBOUR__ ) || defined( __XHARBOUR__ )

#if 0
#if defined( __PPGEN__ )
   hb_snprintf( szResult, sizeof( szResult ), "%05d", HB_MAX( ( HB_VER_MAJOR << 8 ) | HB_VER_MINOR, 1 ) );
#else
   hb_snprintf( szResult, sizeof( szResult ), "%05d", HB_VER_CVSID );
#endif
#endif

   hb_snprintf( szResult, sizeof( szResult ), "0x%02i%i%i", HB_VER_MAJOR, HB_VER_MINOR, HB_VER_REVISION );

#ifdef __HARBOUR__
   /* __HARBOUR__ */
   hb_pp_addDefine( pState, "__HARBOUR__", szResult );
#endif
#ifdef __XHARBOUR__
   /* __XHARBOUR__ */
   hb_pp_addDefine( pState, "__XHARBOUR__", szResult );
#endif
#endif

   /* __DATE__ */
   hb_dateToday( &iYear, &iMonth, &iDay );
   hb_dateStrPut( szResult + 1, iYear, iMonth, iDay );
   szResult[ 0 ]  = '"';
   szResult[ 9 ]  = '"';
   szResult[ 10 ] = '\0';
   hb_pp_addDefine( pState, "__DATE__", szResult );

   /* __TIME__ */
   hb_dateTimeStr( szResult + 1 );
   szResult[ 0 ]  = '"';
   szResult[ 9 ]  = '"';
   szResult[ 10 ] = '\0';
   hb_pp_addDefine( pState, "__TIME__", szResult );

   hb_snprintf( szResult, sizeof( szResult ), "%d", ( int ) sizeof( void * ) );
#if defined( HB_ARCH_16BIT )
   hb_pp_addDefine( pState, "__ARCH16BIT__", szResult );
#elif defined( HB_ARCH_32BIT )
   hb_pp_addDefine( pState, "__ARCH32BIT__", szResult );
#elif defined( HB_ARCH_64BIT )
   hb_pp_addDefine( pState, "__ARCH64BIT__", szResult );
#endif

#if defined( HB_LITTLE_ENDIAN )
   hb_pp_addDefine( pState, "__LITTLE_ENDIAN__", szResult );
#elif defined( HB_BIG_ENDIAN )
   hb_pp_addDefine( pState, "__BIG_ENDIAN__", szResult );
#elif defined( HB_PDP_ENDIAN )
   hb_pp_addDefine( pState, "__PDP_ENDIAN__", szResult );
#endif

#ifdef HARBOUR_START_PROCEDURE
   hb_pp_addDefine( pState, "__HB_MAIN__", HARBOUR_START_PROCEDURE );
#endif

#ifdef __XHARBOUR__
   hb_pp_addDefine( pState, "__PRG__", "1" );
   hb_pp_addDefine( pState, "__NAMESPACE__", "1" );
#endif
}

/*
 * read preprocess rules from file
 */
void hb_pp_readRules( PHB_PP_STATE pState, const char * szRulesFile )
{
   char        szFileName[ HB_PATH_MAX ];
   PHB_PP_FILE pFile    = pState->pFile;
   PHB_FNAME   pFileName;
   BOOL        fError   = FALSE;

   pFileName = hb_fsFNameSplit( szRulesFile );
   if( ! pFileName->szExtension )
      pFileName->szExtension = ".ch";
   hb_fsFNameMerge( szFileName, pFileName );
   hb_xfree( pFileName );

   pState->pFile = hb_pp_FileNew( pState, szFileName, FALSE, NULL, NULL,
                                  TRUE, pState->pOpenFunc );
   if( ! pState->pFile )
   {
      pState->pFile = pFile;
      hb_pp_error( pState, 'F', HB_PP_ERR_CANNOT_OPEN_RULES, szFileName );
   }
   else
   {
      pState->iFiles++;
      pState->iLastType = HB_PP_TOKEN_NUL;
      while( hb_pp_tokenGet( pState ) )
      {
         if( pState->fError )
            fError = TRUE;
      }
      if( pState->pFile )
      {
         hb_pp_FileFree( pState, pState->pFile, pState->pCloseFunc );
         pState->iFiles--;
      }
      pState->pFile = pFile;
      if( fError )
         pState->fError = TRUE;
   }
}

/*
 * close all open input files and set the given buffer as input stream
 */
BOOL hb_pp_inBuffer( PHB_PP_STATE pState, const char * pBuffer, HB_SIZE ulLen )
{
   hb_pp_InFileFree( pState );

   pState->fError = FALSE;

   pState->pFile  = hb_pp_FileBufNew( pBuffer, ulLen );
   pState->iFiles++;
   return TRUE;
}

/*
 * close all open input files and set the given one as new
 */
BOOL hb_pp_inFile( PHB_PP_STATE pState, const char * szFileName,
                   BOOL fSearchPath, FILE * file_in, BOOL fError )
{
   hb_pp_InFileFree( pState );

   pState->fError = FALSE;

   pState->pFile  = hb_pp_FileNew( pState, ( char * ) szFileName, FALSE, NULL,
                                   file_in, fSearchPath, NULL );
   if( pState->pFile )
   {
      pState->iFiles++;
      return TRUE;
   }
   if( fError )
      hb_pp_error( pState, 'F', HB_PP_ERR_CANNOT_OPEN_INPUT, szFileName );
   return FALSE;
}

/*
 * set output (.ppo) file
 */
BOOL hb_pp_outFile( PHB_PP_STATE pState, const char * szOutFileName,
                    FILE * file_out )
{
   pState->fError = FALSE;
   hb_pp_OutFileFree( pState );

   if( szOutFileName )
   {

      if( file_out )
         pState->file_out = file_out;
      else
         pState->file_out = hb_fopen( szOutFileName, "w" );

      if( pState->file_out )
      {
         pState->szOutFileName      = hb_strdup( szOutFileName );
         pState->fWritePreprocesed  = TRUE;
      }
      else
      {
         hb_pp_error( pState, 'F', HB_PP_ERR_CANNOT_CREATE_FILE, szOutFileName );
      }
   }
   return ! pState->fError;
}

/*
 * set trace (.ppt) file
 */
BOOL hb_pp_traceFile( PHB_PP_STATE pState, const char * szTraceFileName, FILE * file_trace )
{
   pState->fError = FALSE;
   hb_pp_TraceFileFree( pState );

   if( szTraceFileName )
   {

      if( file_trace )
         pState->file_trace = file_trace;
      else
         pState->file_trace = hb_fopen( szTraceFileName, "w" );

      if( pState->file_trace )
      {
         pState->szTraceFileName = hb_strdup( szTraceFileName );
         pState->fWriteTrace     = TRUE;
      }
      else
      {
         hb_pp_error( pState, 'F', HB_PP_ERR_CANNOT_CREATE_FILE, szTraceFileName );
      }
   }
   return ! pState->fError;
}

/*
 * check error status of last PP operation
 */
BOOL hb_pp_lasterror( PHB_PP_STATE pState )
{
   return pState->fError;
}

/*
 * retrieve number of errors which appeared during preprocessing
 */
int hb_pp_errorCount( PHB_PP_STATE pState )
{
   return pState->iErrors;
}

/*
 * return currently preprocessed file name
 */
char * hb_pp_fileName( PHB_PP_STATE pState )
{
   if( pState->pFile )
      return pState->pFile->szFileName;
   else
      return NULL;
}

/*
 * return currently preprocessed line number
 */
int hb_pp_line( PHB_PP_STATE pState )
{
   if( pState->pFile )
      return pState->pFile->iCurrentLine;
   else
      return 0;
}

int hb_pp_lineTot( PHB_PP_STATE pState )
{
   return pState->iLineTot;
}

/*
 * return output file name (.ppo)
 */
char * hb_pp_outFileName( PHB_PP_STATE pState )
{
   return pState->szOutFileName;
}

/*
 * return trace output file name (.ppt)
 */
char * hb_pp_traceFileName( PHB_PP_STATE pState )
{
   return pState->szTraceFileName;
}

/*
 * return if EOF was reached
 */
BOOL hb_pp_eof( PHB_PP_STATE pState )
{
   return pState->pFile->fEof;
}

/*
 * add new define value
 */
void hb_pp_addDefine( PHB_PP_STATE pState, const char * szDefName,
                      const char * szDefValue )
{
   PHB_PP_TOKEN   pMatch, pResult, pToken;
   PHB_PP_FILE    pFile;

   pState->fError             = FALSE;

   pFile                      = hb_pp_FileBufNew( szDefName, strlen( szDefName ) );
   pFile->pPrev               = pState->pFile;
   pState->pFile              = pFile;
   pState->iFiles++;
   hb_pp_getLine( pState );
   pMatch                     = pState->pFile->pTokenList;
   pState->pFile->pTokenList  = NULL;
   pToken                     = hb_pp_tokenResultEnd( &pMatch, TRUE );
   hb_pp_tokenListFree( &pToken );

   if( szDefValue && ! pState->fError )
   {
      pFile->pLineBuf            = szDefValue;
      pFile->ulLineBufLen        = strlen( szDefValue );
      hb_pp_getLine( pState );
      pResult                    = pState->pFile->pTokenList;
      pState->pFile->pTokenList  = NULL;
      pToken                     = hb_pp_tokenResultEnd( &pResult, TRUE );
      hb_pp_tokenListFree( &pToken );
   }
   else
      pResult = NULL;

   if( pState->fError || ! pMatch )
   {
      hb_pp_tokenListFree( &pMatch );
      hb_pp_tokenListFree( &pResult );
   }
   else
   {
      hb_pp_defineAdd( pState, HB_PP_CMP_CASE, 0, NULL, pMatch, pResult );
   }
   pState->pFile = pFile->pPrev;
   hb_pp_FileFree( pState, pFile, NULL );
   pState->iFiles--;
}

/*
 * delete define value
 */
void hb_pp_delDefine( PHB_PP_STATE pState, const char * szDefName )
{
   PHB_PP_TOKEN pToken;

   pToken = hb_pp_tokenNew( szDefName, strlen( szDefName ),
                            0, HB_PP_TOKEN_KEYWORD );
   hb_pp_defineDel( pState, pToken );
   hb_pp_tokenFree( pToken );
}

/*
 * set stream mode
 */
void hb_pp_setStream( PHB_PP_STATE pState, int iMode )
{
   pState->fError = FALSE;
   switch( iMode )
   {
      case HB_PP_STREAM_DUMP_C:
         pState->iDumpLine    = pState->pFile ? pState->pFile->iCurrentLine : 0;
         if( ! pState->pDumpBuffer )
            pState->pDumpBuffer = hb_membufNew();
         pState->iStreamDump  = iMode;
         break;

      case HB_PP_STREAM_INLINE_C:
         pState->iDumpLine = pState->pFile ? pState->pFile->iCurrentLine : 0;
      case HB_PP_STREAM_CLIPPER:
      case HB_PP_STREAM_PRG:
      case HB_PP_STREAM_C:
         if( ! pState->pStreamBuffer )
            pState->pStreamBuffer = hb_membufNew();
      case HB_PP_STREAM_OFF:
      case HB_PP_STREAM_COMMENT:
         pState->iStreamDump = iMode;
         break;

      default:
         pState->fError = TRUE;
   }
}

/*
 * return next preprocessed line
 */
char * hb_pp_nextLine( PHB_PP_STATE pState, ULONG * pulLen )
{
   if( pState->pFile )
   {
      PHB_PP_TOKEN   pToken;
      BOOL           fError = FALSE;
      USHORT         ltype;

      if( ! pState->pOutputBuffer )
         pState->pOutputBuffer = hb_membufNew();
      else
         hb_membufFlush( pState->pOutputBuffer );

      pState->iLastType = ltype = HB_PP_TOKEN_NUL;
      while( ( pToken = hb_pp_tokenGet( pState ) ) != NULL )
      {
         if( pState->fError )
            fError = TRUE;

         if( hb_pp_tokenStr( pToken, pState->pOutputBuffer, TRUE, TRUE, ltype ) )
         {
            if( HB_PP_TOKEN_ISEOL( pToken ) && ( pToken->type & HB_PP_TOKEN_LINECOUNTER ) )
               pState->pFile->iCurrentLine++;

            break;
         }

         #ifndef __XHARBOUR__
         /* only single command in one call */
         if( ! pState->pTokenOut->pNext )
            break;
         #endif

         ltype = HB_PP_TOKEN_TYPE( pToken->type );
      }
      if( fError )
         pState->fError = TRUE;

      if( pulLen )
         *pulLen = ( ULONG ) hb_membufLen( pState->pOutputBuffer );
      hb_membufAddCh( pState->pOutputBuffer, '\0' );

      return hb_membufPtr( pState->pOutputBuffer );
   }

   if( pulLen )
      *pulLen = 0;
   return NULL;
}

/*
 * preprocess given buffer
 */
char * hb_pp_parseLine( PHB_PP_STATE pState, const char * pLine, HB_SIZE * pulLen )
{
   PHB_PP_TOKEN   pToken;
   PHB_PP_FILE    pFile;
   BOOL           fError = FALSE;
   USHORT         ltype;
   HB_SIZE        ulLen;

   if( ! pState->pOutputBuffer )
      pState->pOutputBuffer = hb_membufNew();
   else
      hb_membufFlush( pState->pOutputBuffer );

   ulLen             = pulLen ? *pulLen : strlen( pLine );

   pFile             = hb_pp_FileBufNew( pLine, ulLen );
   pFile->pPrev      = pState->pFile;
   pState->pFile     = pFile;
   pState->iFiles++;

   pState->iLastType = ltype = HB_PP_TOKEN_NUL;
   while( ( pToken = hb_pp_tokenGet( pState ) ) != NULL )
   {
      if( pState->fError )
         fError = TRUE;
      hb_pp_tokenStr( pToken, pState->pOutputBuffer, TRUE, TRUE, ltype );
      ltype = HB_PP_TOKEN_TYPE( pToken->type );
   }
   if( fError )
      pState->fError = TRUE;

   if( ( ulLen && pLine[ ulLen - 1 ] == '\n' ) ||
       hb_membufLen( pState->pOutputBuffer ) == 0 ||
       hb_membufPtr( pState->pOutputBuffer )
       [ hb_membufLen( pState->pOutputBuffer ) - 1 ] != '\n' )
      hb_membufAddCh( pState->pOutputBuffer, '\0' );
   else
      hb_membufPtr( pState->pOutputBuffer )
      [ hb_membufLen( pState->pOutputBuffer ) - 1 ] = '\0';

   if( pulLen )
      *pulLen = hb_membufLen( pState->pOutputBuffer ) - 1;

   if( pState->pFile == pFile )
   {
      pState->pFile = pFile->pPrev;
      hb_pp_FileFree( pState, pFile, NULL );
      pState->iFiles--;
   }

   return hb_membufPtr( pState->pOutputBuffer );
}

/*
 * create new PP context for macro compiler
 */
PHB_PP_STATE hb_pp_lexNew( const char * pMacroString, HB_SIZE ulLen )
{
   PHB_PP_STATE pState = hb_pp_new();

   pState->fQuiet             = TRUE;
   pState->pFile              = hb_pp_FileBufNew( pMacroString, ulLen );
   hb_pp_getLine( pState );
   pState->pTokenOut          = pState->pFile->pTokenList;
   pState->pFile->pTokenList  = NULL;
   hb_pp_FileFree( pState, pState->pFile, NULL );
   pState->pFile              = NULL;
   if( pState->fError )
   {
      hb_pp_free( pState );
      pState = NULL;
   }
   else
      pState->pNextTokenPtr = &pState->pTokenOut;

   return pState;
}

PHB_PP_TOKEN hb_pp_lexGet( PHB_PP_STATE pState )
{
   PHB_PP_TOKEN pToken = *pState->pNextTokenPtr;

   if( pToken )
      pState->pNextTokenPtr = &pToken->pNext;

   return pToken;
}

BOOL hb_pp_tokenNextExp( PHB_PP_TOKEN * pTokenPtr )
{
   if( hb_pp_tokenCanStartExp( *pTokenPtr ) )
   {
      BOOL fStop = FALSE;
      if( hb_pp_tokenSkipExp( pTokenPtr, NULL, HB_PP_CMP_STD, &fStop ) && ! fStop )
         return TRUE;
   }

   return FALSE;
}

/*
 * convert token letters to upper cases
 * strip leading '&' and trailing '.' (if any) from macrovar token
 */
void hb_pp_tokenUpper( PHB_PP_TOKEN pToken )
{
   if( HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_MACROVAR )
   {
      if( pToken->len > HB_SYMBOL_NAME_LEN + 1 )
         pToken->len = HB_SYMBOL_NAME_LEN + 1;
      if( pToken->value[ pToken->len - 1 ] == '.' )
         pToken->len -= 2;
      else
         pToken->len--;

      if( pToken->len <= 1 )
      {
         UCHAR ucVal = pToken->len ? ( UCHAR ) pToken->value[ 1 ] : 0;
         if( HB_PP_TOKEN_ALLOC( pToken->type ) )
         {
            hb_xfree( pToken->value );
            pToken->type |= HB_PP_TOKEN_STATIC;
         }
         pToken->value = ( char * ) hb_szAscii[ ucVal ];
      }
      else
      {
         if( ! HB_PP_TOKEN_ALLOC( pToken->type ) )
         {
            char * value = ( char * ) hb_xgrab( pToken->len + 1 );
            HB_MEMCPY( value, pToken->value + 1, pToken->len );
            hb_xfree( pToken->value );
            pToken->type &= ~HB_PP_TOKEN_STATIC;
         }
         else
            memmove( pToken->value, pToken->value + 1, pToken->len );
         pToken->value[ pToken->len ] = '\0';
      }
   }
   else if( pToken->len > 1 )
   {
      if( ! HB_PP_TOKEN_ALLOC( pToken->type ) )
      {
         char * value = ( char * ) hb_xgrab( pToken->len + 1 );
         HB_MEMCPY( value, pToken->value, pToken->len + 1 );
         pToken->value  = value;
         pToken->type   &= ~HB_PP_TOKEN_STATIC;
      }
      if( HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_KEYWORD &&
          pToken->len > HB_SYMBOL_NAME_LEN )
      {
         pToken->len                         = HB_SYMBOL_NAME_LEN;
         pToken->value[ HB_SYMBOL_NAME_LEN ] = '\0';
      }
   }

   if( pToken->len <= 1 )
   {
      UCHAR ucVal = ( UCHAR ) HB_PP_UPPER( pToken->value[ 0 ] );
      if( HB_PP_TOKEN_ALLOC( pToken->type ) )
      {
         hb_xfree( pToken->value );
         pToken->type |= HB_PP_TOKEN_STATIC;
      }
      pToken->value = ( char * ) hb_szAscii[ ucVal ];
   }
   else
      hb_strupr( pToken->value );
}

#ifndef __XHARBOUR__
/*
 * convert tokens between '[' and ']' tokens into single string token
 * and replace the converted tokens with the new string
 */
void hb_pp_tokenToString( PHB_PP_STATE pState, PHB_PP_TOKEN pToken )
{
   BOOL fError = TRUE;

   pState->fError = FALSE;
   hb_membufFlush( pState->pBuffer );
   if( HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_LEFT_SB )
   {
      PHB_PP_TOKEN pTok, pFirst, pLast = NULL;
      pFirst = pTok = pToken->pNext;
      while( ! HB_PP_TOKEN_ISEOL( pTok ) )
      {
         pLast = pTok;
         if( HB_PP_TOKEN_TYPE( pTok->type ) == HB_PP_TOKEN_RIGHT_SB )
         {
            while( pTok->spaces > 0 )
            {
               hb_membufAddCh( pState->pBuffer, ' ' );
               pTok->spaces--;
            }
            fError   = FALSE;
            pTok     = pTok->pNext;
            break;
         }
         else if( HB_PP_TOKEN_TYPE( pTok->type ) == HB_PP_TOKEN_EOC &&
                  ! pTok->pNext && pState->pFile->pTokenList )
         {
#if ! defined( HB_PP_NO_LINEINFO_TOKEN )
            pState->pFile->iLastLine = pState->pFile->iCurrentLine +
#endif
            hb_pp_tokenMoveCommand( &pTok->pNext,
                                    &pState->pFile->pTokenList );
         }
         hb_pp_tokenStr( pTok, pState->pBuffer, TRUE, FALSE, 0 );
         pTok = pTok->pNext;
      }
      if( pLast )
      {
         pLast->pNext   = NULL;
         pToken->pNext  = pTok;
         hb_pp_tokenListFree( &pFirst );
      }
      hb_pp_tokenSetValue( pToken, hb_membufPtr( pState->pBuffer ),
                           hb_membufLen( pState->pBuffer ) );
      HB_PP_TOKEN_SETTYPE( pToken, HB_PP_TOKEN_STRING );
      if( pState->fWritePreprocesed )
      {
         if( ! fError )
            hb_membufAddCh( pState->pBuffer, ']' );
         fwrite( hb_membufPtr( pState->pBuffer ), sizeof( char ),
                 hb_membufLen( pState->pBuffer ), pState->file_out );
      }
   }

   if( fError )
   {
      hb_membufAddCh( pState->pBuffer, '\0' );
      hb_pp_error( pState, 'E', HB_PP_ERR_STRING_TERMINATOR,
                   hb_membufPtr( pState->pBuffer ) );
   }
}
#endif

char * hb_pp_tokenBlockString( PHB_PP_STATE pState, PHB_PP_TOKEN pToken,
                               int * piType, int * piLen )
{
   *piType = 0;
   hb_membufFlush( pState->pBuffer );
   if( HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_LEFT_CB )
   {
      USHORT   ltype    = HB_PP_TOKEN_NUL;
      int      iBraces  = 0;
      do
      {
         hb_pp_tokenStr( pToken, pState->pBuffer, FALSE, TRUE, ltype );
         ltype = HB_PP_TOKEN_TYPE( pToken->type );
         if( HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_AMPERSAND )
         {
            if( pToken->pNext &&
                HB_PP_TOKEN_TYPE( pToken->pNext->type ) == HB_PP_TOKEN_LEFT_PB )
               *piType |= HB_BLOCK_MACRO | HB_BLOCK_LATEEVAL;
         }
         else if( HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_MACROVAR ||
                  HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_MACROTEXT )
            *piType |= HB_BLOCK_MACRO;
         else if( HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_RIGHT_CB )
            --iBraces;
         else if( HB_PP_TOKEN_TYPE( pToken->type ) == HB_PP_TOKEN_LEFT_CB )
            ++iBraces;
         pToken = pToken->pNext;
      }
      while( iBraces && ! HB_PP_TOKEN_ISEOC( pToken ) );
   }
   *piLen = ( int ) hb_membufLen( pState->pBuffer );
   hb_membufAddCh( pState->pBuffer, '\0' );
   return hb_membufPtr( pState->pBuffer );
}

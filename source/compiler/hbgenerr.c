/*
 * $Id: hbgenerr.c 9934 2013-03-21 07:40:15Z zsaulius $
 */

/*
 * Harbour Project source code:
 * Compiler parse errors & warnings messages
 *
 * Copyright 1999 {list of individual authors and e-mail addresses}
 * www - http://www.harbour-project.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version, with one exception:
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA (or visit
 * their web site at http://www.gnu.org/).
 *
 */

#include "hbcomp.h"

//extern char *yytext;

/* Table with parse errors */
const char * hb_comp_szErrors[] =
{
/* 1 */ "Statement not allowed outside of procedure or function",
   "Redefinition of procedure or function: \'%s\'",
   "Duplicate variable declaration: \'%s\'",
   "%s declaration follows executable statement",
   "Outer codeblock variable is out of reach: \'%s\'",
   "Invalid numeric format '.'",
   "Unterminated string: \'%s\'",
   "Redefinition of predefined function %s: \'%s\'",
   "Illegal variable \'%s\' initializer: \'%s\'",
/* 10 */ "ENDIF does not match IF",
   "ENDDO does not match WHILE",
   "ENDCASE does not match DO CASE",
   "NEXT does not match FOR",
   "ELSE does not match IF",
   "ELSEIF does not match IF",
   "Syntax error: \'%s\'",
   "Unclosed control structure %s",
   "%s statement with no loop in sight",
   "Syntax error: \'%s\' in: \'%s\'",
/* 20 */ "Incomplete statement or unbalanced delimiters",
   "Incorrect number of arguments: %s %s",
   "Invalid lvalue: \'%s\'",
   "Invalid use of \'@\' (pass by reference): \'%s\'",
   "Formal parameters already declared",
   "Invalid %s from within of SEQUENCE code",
   "Unterminated array index",
   "Could not allocate %s byte(s)",
   "Could not reallocate %s byte(s)",
   "Freeing a NULL memory pointer",
/* 30 */ "Syntax error: \"%s at \'%s\'\"",
   "Jump offset too long",
   "Can't create output file: \'%s\'",
   "Can't create preprocessed output file: \'%s\'",
   "Bad command line option: \'%s\'",
   "Bad command line parameter: \'%s\'",
   "Invalid filename: \'%s\'",
   "Mayhem in CASE handler",
   "Operation not supported for this data type: \'%s\'",
   "Invalid alias expression: \'%s\'",
/* 40 */ "Invalid array index expression: \'%s\'",
   "Bound error: \'%s\'",
   "Macro of declared symbol: \'%s\'",
   "Invalid selector in send: \'%s\'",
   "ANNOUNCEd procedure \'%s\' must be a public symbol",
   "Jump PCode not found",
   "CASE or OTHERWISE does not match DO CASE",
   "Code block contains both macro and declared symbol references",
   "GET contains complex macro",
   "Unterminated inline block in function: \'%s\'",
/* 50 */ "Too many inline blocks %s",
   "Inline C requires C output generation, use -gc[n]",
   "Missing ENDTEXT",
   "Optimized Local [%s] out of range, after offsetting %s PARAMETERS.",
   "WITH Message [%s] with no WITH OBJECT in sight.",
   "Misplaced Global declaration \'%s\'.",
   "Too many Global declarations \'%s\'.",
   "External Global declaration can not include initializer \'%s\'.",
   "Too many Parameter declarations \'%s\'.",
   "Invalid SWITCH Constant \'%s\'.",
/* 60 */ "Cannot write HIL table to \'%s\'.",
   "Illegal character \'%s\'.",
   "Can't create preprocessed tracing file: \'%s\'",
   "Unbalance PRAGMA BEGINDUMP and PRAGMA ENDDUMP",
   "Unterminated /* */ comment",
   "Memory buffer overflow",
   "Memory corruption detected",
   "TRY section requires a CATCH or FINALLY handler",
   "Invalid use of nameless parameter",
   "Too many arguments to function: \'%s\'",
/* 70 */ "IMPLEMENTS NAMESPACE from within a NAMESPACE: %s",
   "NAMESPACE \'%s\' is empty",
   "Definitions not loaded for NAMESPACE: \'%s\'",
   "NAMESPACE: \'%s\' not found in: \'%s\'",
   "EXTERNAL NAMESPACE [%s] with no NAMESPACE in sight.",
   "Cannot use reserved word: \'%s\'."
};

/* Table with parse warnings */
/* NOTE: The first character stores the warning's level that triggers this
 * warning. The warning's level is set by -w<n> command line option.
 */
const char * hb_comp_szWarnings[] =
{
   "1Ambiguous reference: \'%s\'",
   "1Ambiguous reference, assuming memvar: \'%s\'",
   "2Variable: \'%s\' declared but not used in function: \'%s\'",
   "2Codeblock parameter: \'%s\' declared but not used in function: \'%s\'",
   "1RETURN statement with no return value in function",
   "1Procedure returns value",
   "1Function \'%s\' does not end with RETURN statement",
   "3Incompatible type in assignment to: \'%s\' expected: \'%s\'",
   "3Incompatible operand type: \'%s\' expected: \'%s\'",
   "3Incompatible operand types: \'%s\' and: \'%s\'",
   "4Suspicious type in assignment to: \'%s\' expected: \'%s\'",
   "4Suspicious operand type: \'unknown\' expected: \'%s\'",
   "3Can\'t use array index with non-array",
   "3Incompatible return type: \'%s\' expected: \'%s\'",
   "4Suspicious return type: \'%s\' expected: \'%s\'",
   "3Invalid number of parameters: %s expected: %s",
   "3Incompatible parameter: %s expected: \'%s\'",
   "4Suspicious parameter: %s expected: \'%s\'",
   "3Duplicate declaration of %s \'%s\'",
   "3Function \'%s\' conflicting with its declaration",
   "3Variable \'%s\' used but never initialized",
   "3Value of Variable \'%s\' never used",
   "3Incompatible type in assignment to declared array element expected: \'%s\'",
   "4Suspicious type in assignment to declared array element expected: \'%s\'",
   "3Class \'%s\' not known in declaration of \'%s\'",
   "3Message \'%s\' not known in class \'%s\'",
   "0Meaningless use of expression: \'%s\'",
   "2Unreachable code",
   "1Redundant \'ANNOUNCE %s\' statement ignored",
   "1STATIC Function \'%s\' defined but never used",
   "1Variable \'%s\' is a reserved word",
   "3Variable '%s' is assigned but not used in function '%s'",
   "3Variable '%s' is never assigned in function '%s'"
};

void hb_compGenError( const char * szErrors[], char cPrefix, int iError, const char * szError1, const char * szError2 )
{
   int      iLine        = hb_comp_iLine - 1;
   char *   szFile       = hb_pp_fileName( hb_comp_PP );
   char     szTmp[ 256 ] = { '\0' };

   if( cPrefix != 'F' && hb_comp_bError )
      return;

   if( szFile )
      hb_snprintf( hb_comp_szMsgBuf, SIZE_OF_SZMSGBUF, "\r%s(%i) ", szFile, iLine );

   hb_snprintf( szTmp, sizeof( szTmp ), "Error %c%04i  ", cPrefix, iError );
   hb_xstrcat ( hb_comp_szMsgBuf, szTmp, 0 );

   if( cPrefix == 'F' && iError == HB_PP_ERR_UNKNOWN_RESULTMARKER )
      hb_snprintf( szTmp, sizeof( szTmp ), "Unknown result marker %s%s%s in #directive", "<", szError1, ">" );
   else
      hb_snprintf( szTmp, sizeof( szTmp ), szErrors[ iError - 1 ], szError1, szError2 );

   hb_xstrcat( hb_comp_szMsgBuf, szTmp, "\n", 0 );
   hb_compOutErr( hb_comp_szMsgBuf );

   hb_comp_iErrorCount++;
   hb_comp_bError = TRUE;

   /* fatal error - exit immediately */
   if( cPrefix == 'F' )
   {
      hb_compCleanUp();
      exit( EXIT_FAILURE );
   }
}

void hb_compGenWarning( const char * szWarnings[], char cPrefix, int iWarning, const char * szWarning1, const char * szWarning2 )
{
   const char *   szText   = szWarnings[ iWarning - 1 ];
   int            iLine    = hb_comp_iLine - 1;
   char *         szFile   = hb_pp_fileName( hb_comp_PP );

   if( ( ( int ) ( szText[ 0 ] - '0' ) ) <= hb_comp_iWarnings )
   {
      char szTmp[ 256 ];

      if( szFile )
         hb_snprintf( hb_comp_szMsgBuf, SIZE_OF_SZMSGBUF, "\r%s(%i) ", szFile, iLine );

      hb_snprintf( szTmp, sizeof( szTmp ), "Warning %c%04i  ", cPrefix, iWarning );
      hb_xstrcat ( hb_comp_szMsgBuf, szTmp, 0 );
      hb_snprintf( szTmp, sizeof( szTmp ), szText + 1, szWarning1, szWarning2 );
      hb_xstrcat ( hb_comp_szMsgBuf, szTmp, "\n", 0 );
      hb_compOutErr( hb_comp_szMsgBuf );

      hb_comp_bAnyWarning = TRUE;    /* report warnings at exit */
   }
}

HB_EXPR_PTR hb_compErrorLValue( HB_EXPR_PTR pExpr )
{
   char * szDesc = hb_compExprDescription( pExpr );

   hb_compGenError( hb_comp_szErrors, 'E', HB_COMP_ERR_INVALID_LVALUE, szDesc, NULL );
   return pExpr;
}

HB_EXPR_PTR hb_compErrorType( HB_EXPR_PTR pExpr )
{
   char * szDesc = hb_compExprDescription( pExpr );

   hb_compGenError( hb_comp_szErrors, 'E', HB_COMP_ERR_INVALID_TYPE, szDesc, NULL );
   return pExpr;
}

HB_EXPR_PTR hb_compErrorIndex( HB_EXPR_PTR pExpr )
{
   char * szDesc = hb_compExprDescription( pExpr );

   hb_compGenError( hb_comp_szErrors, 'E', HB_COMP_ERR_INVALID_INDEX, szDesc, NULL );
   return pExpr;
}

HB_EXPR_PTR hb_compErrorBound( HB_EXPR_PTR pExpr )
{
   char * szDesc = hb_compExprDescription( pExpr );

   hb_compGenError( hb_comp_szErrors, 'E', HB_COMP_ERR_INVALID_BOUND, szDesc, NULL );
   return pExpr;
}

HB_EXPR_PTR hb_compErrorSyntax( HB_EXPR_PTR pExpr )
{
   char * szDesc = hb_compExprDescription( pExpr );

   hb_compGenError( hb_comp_szErrors, 'E', HB_COMP_ERR_SYNTAX, szDesc, NULL );
   return pExpr;
}

HB_EXPR_PTR hb_compErrorAlias(  HB_EXPR_PTR pExpr )
{
   char * szDesc = hb_compExprDescription( pExpr );

   hb_compGenError( hb_comp_szErrors, 'E', HB_COMP_ERR_INVALID_ALIAS, szDesc, NULL );
   return pExpr;
}

HB_EXPR_PTR hb_compErrorStatic( char * szVarName, HB_EXPR_PTR pExpr )
{
   char * szDesc = hb_compExprDescription( pExpr );

   hb_compGenError( hb_comp_szErrors, 'E', HB_COMP_ERR_ILLEGAL_INIT, szVarName, szDesc );
   return pExpr;
}

void hb_compErrorDuplVar( char * szVarName )
{
   hb_compGenError( hb_comp_szErrors, 'E', HB_COMP_ERR_VAR_DUPL, szVarName, NULL );
}

HB_EXPR_PTR hb_compWarnMeaningless( HB_EXPR_PTR pExpr )
{
   char * szDesc = hb_compExprDescription( pExpr );

   hb_compGenWarning(  hb_comp_szWarnings, 'W', HB_COMP_WARN_MEANINGLESS, szDesc, NULL );
   return pExpr;
}


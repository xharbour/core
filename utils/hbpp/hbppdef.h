/*
 * $Id: hbppdef.h 9279 2011-02-14 18:06:32Z druzus $
 */

/*
 * Harbour Project source code:
 * Header file for the Preprocesor
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

#ifndef HB_PP_H_
#define HB_PP_H_

#include "hbapi.h"
#include "hbapifs.h"
#include "hberrors.h"
#include "hbver.h"

HB_EXTERN_BEGIN

typedef void * PHB_PP_STATE;

struct _DEFINES;
typedef struct _DEFINES
{
   char * name;
   char * pars;
   int    npars;
   char * value;
   struct _DEFINES * last;
} DEFINES;

struct _COMMANDS;
typedef struct _COMMANDS
{
   int com_or_xcom;
   char * name;
   char * mpatt;
   char * value;
   struct _COMMANDS * last;
} COMMANDS;

#define HB_PP_STR_SIZE  16384
#define HB_PP_BUFF_SIZE 4096

#define HB_SKIPTABSPACES( sptr ) while( *sptr == ' ' || *sptr == '\t' ) ( sptr )++

/* PPCORE.C exported functions and variables */

#define HB_INCLUDE_FUNC_( hbFunc ) BOOL hbFunc( char *, HB_PATHNAMES * )
typedef HB_INCLUDE_FUNC_( HB_INCLUDE_FUNC );
typedef HB_INCLUDE_FUNC * HB_INCLUDE_FUNC_PTR;

extern void   hb_pp_SetRules( HB_INCLUDE_FUNC_PTR hb_compInclude, BOOL hb_comp_bQuiet );
extern int    hb_pp_ReadRules( void );
extern void   hb_pp_Init( void );
extern void   hb_pp_Free( void );
extern void   CloseInclude( void );
extern int    hb_pp_ParseDefine( char * );
extern int    hb_pp_ParseDirective( char * ); /* Parsing preprocessor directives ( #... ) */
extern int    hb_pp_ParseExpression( char *, char * ); /* Parsing a line ( without preprocessor directive ) */
extern int    hb_pp_WrStr( FILE *, char * );
extern int    hb_pp_RdStr( FILE *, char *, int, BOOL, char *, int *, int *, int );
extern void   hb_pp_Stuff( char *, char *, int, int, int );
extern int    hb_pp_strocpy( char *, char * );
extern DEFINES * hb_pp_AddDefine( char *, char *, BOOL );         /* Add new #define to a linked list */

extern int    hb_pp_lInclude;
extern int *  hb_pp_aCondCompile;
extern int    hb_pp_nCondCompile;
extern const char * hb_pp_szErrors[];
extern const char * hb_pp_szWarnings[];
extern int    hb_pp_nEmptyStrings;
extern BOOL   hb_pp_bInline;
extern int    hb_pp_LastOutLine;
extern BOOL   hb_pp_bInComment;           /* Counter for comment */
extern int    iBeginDump;
extern int    iEndDump;


/* PPCOMP.C exported functions */

extern int    hb_pp_Internal( FILE *, char * );

/* PPTABLE.C exported functions and variables */

extern void   hb_pp_Table( void );

extern DEFINES *  hb_pp_topDefine;
extern COMMANDS * hb_pp_topCommand;
extern COMMANDS * hb_pp_topTranslate;

/* PRAGMA.C exported functions */

extern void hb_pp_ParsePragma( char * szline, BOOL bValidCode );

#define STATE_INIT      0
#define STATE_NORMAL    1
#define STATE_COMMENT   2
#define STATE_QUOTE1    3
#define STATE_QUOTE2    4
#define STATE_QUOTE3    5
#define STATE_ID_END    6
#define STATE_ID        7
#define STATE_EXPRES    8
#define STATE_EXPRES_ID 9
#define STATE_BRACKET   10
/* Added by Giancarlo Niccolai 2003-05-20*/
#define STATE_QUOTE4    11  /* escaped quote */
/* END */

HB_EXTERN_END

#endif /* HB_PP_H_ */


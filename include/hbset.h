/*
 * $Id: hbset.h 9279 2011-02-14 18:06:32Z druzus $
 */

/*
 * Harbour Project source code:
 * Header file for the Set API
 *
 * Copyright 1999-2003 David G. Holm <dholm@jsd-llc.com>
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

#ifndef HB_SET_H_
#define HB_SET_H_

#include "hbapi.h"
#include "hbapigt.h"
#include "hbapifs.h"

HB_EXTERN_BEGIN

typedef enum
{
   HB_SET_INVALID_      = 0,

   HB_SET_EXACT         = 1,
   HB_SET_FIXED         = 2,
   HB_SET_DECIMALS      = 3,
   HB_SET_DATEFORMAT    = 4,
   HB_SET_EPOCH         = 5,
   HB_SET_PATH          = 6,
   HB_SET_DEFAULT       = 7,

   HB_SET_EXCLUSIVE     = 8,
   HB_SET_SOFTSEEK      = 9,
   HB_SET_UNIQUE        = 10,
   HB_SET_DELETED       = 11,

   HB_SET_CANCEL        = 12,
   HB_SET_DEBUG         = 13,
   HB_SET_TYPEAHEAD     = 14,

   HB_SET_COLOR         = 15,
   HB_SET_CURSOR        = 16,
   HB_SET_CONSOLE       = 17,
   HB_SET_ALTERNATE     = 18,
   HB_SET_ALTFILE       = 19,
   HB_SET_DEVICE        = 20,
   HB_SET_EXTRA         = 21,
   HB_SET_EXTRAFILE     = 22,
   HB_SET_PRINTER       = 23,
   HB_SET_PRINTFILE     = 24,
   HB_SET_MARGIN        = 25,

   HB_SET_BELL          = 26,
   HB_SET_CONFIRM       = 27,
   HB_SET_ESCAPE        = 28,
   HB_SET_INSERT        = 29,
   HB_SET_EXIT          = 30,
   HB_SET_INTENSITY     = 31,
   HB_SET_SCOREBOARD    = 32,
   HB_SET_DELIMITERS    = 33,
   HB_SET_DELIMCHARS    = 34,

   HB_SET_WRAP          = 35,
   HB_SET_MESSAGE       = 36,
   HB_SET_MCENTER       = 37,
   HB_SET_SCROLLBREAK   = 38,

   HB_SET_EVENTMASK     = 39,

   HB_SET_VIDEOMODE     = 40,

   HB_SET_MBLOCKSIZE    = 41,
   HB_SET_MFILEEXT      = 42,

   HB_SET_STRICTREAD    = 43,
   HB_SET_OPTIMIZE      = 44,
   HB_SET_AUTOPEN       = 45,
   HB_SET_AUTORDER      = 46,
   HB_SET_AUTOSHARE     = 47,

   /* Harbour SET extensions start at 100 */
   HB_SET_LANGUAGE      = 100,
   HB_SET_IDLEREPEAT    = 101,
   HB_SET_FILECASE      = 102,
   HB_SET_DIRCASE       = 103,
   HB_SET_DIRSEPARATOR  = 104,
   HB_SET_EOF           = 105,
   HB_SET_HARDCOMMIT    = 106,
   HB_SET_FORCEOPT      = 107,
   HB_SET_DBFLOCKSCHEME = 108,
   HB_SET_DEFEXTENSIONS = 109,
   HB_SET_EOL           = 110,
   HB_SET_TRIMFILENAME  = 111,
   HB_SET_CODEPAGE      = 112,
   HB_SET_OSCODEPAGE    = 113,

   /* xHarbour-only SET extensions start at 200 */
   HB_SET_BACKGROUNDTASKS = 200,
   HB_SET_BACKGROUNDTICK  = 201,
   HB_SET_ERRORLOG        = 202,
   HB_SET_ERRORLOOP       = 203,
   HB_SET_MACROBLOCKVARS  = 204,
   HB_SET_OUTPUTSAFETY    = 205,
   HB_SET_PRINTERJOB      = 206,
   HB_SET_TIMEFORMAT      = 207,
   HB_SET_TRACE           = 208,
   HB_SET_TRACEFILE       = 209,
   HB_SET_TRACESTACK      = 210,
   HB_SET_WORKAREAS_SHARED = 211
} HB_set_enum;

#if defined( _HB_SET_INTERNAL_ ) || defined( _HB_API_INTERNAL_ )
typedef struct
{
   /* Lower case members are indirectly related to a SET */
   BOOL           hb_set_century;
   HB_FHANDLE     hb_set_althan;
   HB_FHANDLE     hb_set_extrahan;
   HB_FHANDLE     hb_set_printhan;
   HB_PATHNAMES * hb_set_path;
   UCHAR          hb_set_oscptransto[ 256 ];
   UCHAR          hb_set_oscptransfrom[ 256 ];
   void *         hb_set_listener;

   /* Upper case members are directly related to a SET */
   BOOL       HB_SET_ALTERNATE;
   char *     HB_SET_ALTFILE;
   BOOL       HB_SET_AUTOPEN;
   int        HB_SET_AUTORDER;
   int        HB_SET_AUTOSHARE;
   BOOL       HB_SET_BELL;
   BOOL       HB_SET_CANCEL;
   char *     HB_SET_COLOR;
   BOOL       HB_SET_CONFIRM;
   BOOL       HB_SET_CONSOLE;
   char *     HB_SET_DATEFORMAT;
   BOOL       HB_SET_DEBUG;
   int        HB_SET_DECIMALS;
   char *     HB_SET_DEFAULT;
   BOOL       HB_SET_DELETED;
   char *     HB_SET_DELIMCHARS;
   BOOL       HB_SET_DELIMITERS;
   char *     HB_SET_DEVICE;
   BOOL       HB_SET_EOF;
   int        HB_SET_EPOCH;
   BOOL       HB_SET_ESCAPE;
   int        HB_SET_EVENTMASK;
   BOOL       HB_SET_EXACT;
   BOOL       HB_SET_EXCLUSIVE;
   BOOL       HB_SET_EXIT;
   BOOL       HB_SET_EXTRA;
   char *     HB_SET_EXTRAFILE;
   BOOL       HB_SET_FIXED;
   BOOL       HB_SET_IDLEREPEAT;
   BOOL       HB_SET_INSERT;
   BOOL       HB_SET_INTENSITY;
   char *     HB_SET_PATH;
   int        HB_SET_MARGIN;
   int        HB_SET_MBLOCKSIZE;
   BOOL       HB_SET_MCENTER;
   int        HB_SET_MESSAGE;
   char *     HB_SET_MFILEEXT;
   BOOL       HB_SET_OPTIMIZE;
   BOOL       HB_SET_PRINTER;
   char *     HB_SET_PRINTFILE;
   BOOL       HB_SET_SCOREBOARD;
   BOOL       HB_SET_SCROLLBREAK;
   BOOL       HB_SET_SOFTSEEK;
   BOOL       HB_SET_STRICTREAD;
   int        HB_SET_TYPEAHEAD;
   BOOL       HB_SET_UNIQUE;
   int        HB_SET_FILECASE;
   int        HB_SET_DIRCASE;
   int        HB_SET_DIRSEPARATOR;
   int        HB_SET_VIDEOMODE;
   BOOL       HB_SET_WRAP;
   int        HB_SET_DBFLOCKSCHEME;

   BOOL       HB_SET_TRACE;
   char       HB_SET_TRACEFILE[ HB_PATH_MAX ];
   char       HB_SET_TRACESTACK;

   int        HB_SET_ERRORLOOP;
   BOOL       HB_SET_OUTPUTSAFETY;
   BOOL       HB_SET_BACKGROUNDTASKS;

   BOOL       hb_set_winprinter;
   FHANDLE    hb_set_winhan;
   char *     hb_set_printerjob;
   int        HB_SET_BACKGROUNDTICK;
   BOOL       HB_SET_HARDCOMMIT;
   BOOL       HB_SET_FORCEOPT;
   BOOL       HB_SET_DEFEXTENSIONS;
   char *     HB_SET_EOL;
   BOOL       HB_SET_TRIMFILENAME;
   char *     HB_SET_OSCODEPAGE;
   BOOL       HB_SET_APPENDERROR;
   char       HB_SET_ERRORLOG[ HB_PATH_MAX];
   char *     HB_SET_TIMEFORMAT;
   BOOL       HB_SET_MACROBLOCKVARS;
   BOOL       HB_SET_WORKAREAS_SHARED;

} HB_SET_STRUCT, * PHB_SET_STRUCT;

extern void hb_setInitialize( PHB_SET_STRUCT pSet );
extern void hb_setRelease( PHB_SET_STRUCT pSet );
extern PHB_SET_STRUCT hb_setClone( PHB_SET_STRUCT pSet );

#else

typedef void * PHB_SET_STRUCT;

#endif /* _HB_SET_INTERNAL_ || _HB_API_INTERNAL_ */

#define HB_SET_CASE_MIXED  0
#define HB_SET_CASE_LOWER  1
#define HB_SET_CASE_UPPER  2

#define HB_SET_DBFLOCK_DEFAULT    0
#define HB_SET_DBFLOCK_CLIP       1
#define HB_SET_DBFLOCK_CL53       2
#define HB_SET_DBFLOCK_VFP        3

#define HB_SET_TRACESTACK_NONE    0
#define HB_SET_TRACESTACK_CURRENT 1
#define HB_SET_TRACESTACK_ALL     2

typedef enum
{
   HB_SET_LISTENER_BEFORE,
   HB_SET_LISTENER_AFTER
} HB_set_listener_enum;
typedef void HB_SET_LISTENER_CALLBACK( HB_set_enum, HB_set_listener_enum );

extern HB_EXPORT int hb_setListenerAdd( HB_SET_LISTENER_CALLBACK * );
extern HB_EXPORT void hb_setListenerNotify( HB_set_enum, HB_set_listener_enum );
extern HB_EXPORT int hb_setListenerRemove( int );

extern HB_EXPORT BOOL         hb_setGetL( HB_set_enum set_specifier );
extern HB_EXPORT const char * hb_setGetCPtr( HB_set_enum set_specifier );
extern HB_EXPORT int          hb_setGetNI( HB_set_enum set_specifier );
extern HB_EXPORT long         hb_setGetNL( HB_set_enum set_specifier );

extern HB_EXPORT BOOL         hb_setSetItem( HB_set_enum set_specifier, PHB_ITEM pItem );
extern HB_EXPORT BOOL         hb_setSetItem2( HB_set_enum set_specifier, PHB_ITEM pItem1, PHB_ITEM pItem2 );

extern HB_EXPORT HB_PATHNAMES * hb_setGetFirstSetPath( void );

extern HB_EXPORT BOOL         hb_setGetCentury( void );
extern HB_EXPORT BOOL         hb_setSetCentury( BOOL );

extern HB_EXPORT HB_FHANDLE   hb_setGetAltHan( void );
extern HB_EXPORT HB_FHANDLE   hb_setGetExtraHan( void );
extern HB_EXPORT HB_FHANDLE   hb_setGetPrintHan( void );
extern HB_EXPORT BOOL         hb_setGetAlternate( void );
extern HB_EXPORT const char * hb_setGetAltFile( void );
extern HB_EXPORT BOOL         hb_setGetAutOpen( void );
extern HB_EXPORT int          hb_setGetAutOrder( void );
extern HB_EXPORT int          hb_setGetAutoShare( void );
extern HB_EXPORT BOOL         hb_setGetBell( void );
extern HB_EXPORT BOOL         hb_setGetCancel( void );
extern HB_EXPORT char *       hb_setGetColor( void );
extern HB_EXPORT BOOL         hb_setGetConfirm( void );
extern HB_EXPORT BOOL         hb_setGetConsole( void );
extern HB_EXPORT const char * hb_setGetDateFormat( void );
extern HB_EXPORT const char * hb_setGetTimeFormat( void );
extern HB_EXPORT BOOL         hb_setGetDebug( void );
extern HB_EXPORT int          hb_setGetDecimals( void );
extern HB_EXPORT const char * hb_setGetDefault( void );
extern HB_EXPORT BOOL         hb_setGetDeleted( void );
extern HB_EXPORT const char * hb_setGetDelimChars( void );
extern HB_EXPORT BOOL         hb_setGetDelimiters( void );
extern HB_EXPORT const char * hb_setGetDevice( void );
extern HB_EXPORT BOOL         hb_setGetEOF( void );
extern HB_EXPORT int          hb_setGetEpoch( void );
extern HB_EXPORT BOOL         hb_setGetEscape( void );
extern HB_EXPORT int          hb_setGetEventMask( void );
extern HB_EXPORT BOOL         hb_setGetExact( void );
extern HB_EXPORT BOOL         hb_setGetExclusive( void );
extern HB_EXPORT BOOL         hb_setGetExit( void );
extern HB_EXPORT BOOL         hb_setGetExtra( void );
extern HB_EXPORT const char * hb_setGetExtraFile( void );
extern HB_EXPORT BOOL         hb_setGetFixed( void );
extern HB_EXPORT BOOL         hb_setGetIdleRepeat( void );
extern HB_EXPORT BOOL         hb_setGetInsert( void );
extern HB_EXPORT BOOL         hb_setGetIntensity( void );
extern HB_EXPORT const char * hb_setGetPath( void );
extern HB_EXPORT int          hb_setGetMargin( void );
extern HB_EXPORT int          hb_setGetMBlockSize( void );
extern HB_EXPORT BOOL         hb_setGetMCenter( void );
extern HB_EXPORT int          hb_setGetMessage( void );
extern HB_EXPORT const char * hb_setGetMFileExt( void );
extern HB_EXPORT BOOL         hb_setGetOptimize( void );
extern HB_EXPORT BOOL         hb_setGetPrinter( void );
extern HB_EXPORT const char * hb_setGetPrintFile( void );
extern HB_EXPORT BOOL         hb_setGetScoreBoard( void );
extern HB_EXPORT BOOL         hb_setGetScrollBreak( void );
extern HB_EXPORT BOOL         hb_setGetSoftSeek( void );
extern HB_EXPORT BOOL         hb_setGetStrictRead( void );
extern HB_EXPORT int          hb_setGetTypeAhead( void );
extern HB_EXPORT BOOL         hb_setGetUnique( void );
extern HB_EXPORT int          hb_setGetFileCase( void );
extern HB_EXPORT int          hb_setGetDirCase( void );
extern HB_EXPORT int          hb_setGetDirSeparator( void );
extern HB_EXPORT int          hb_setGetVideoMode( void );
extern HB_EXPORT BOOL         hb_setGetWrap( void );
extern HB_EXPORT int          hb_setGetDBFLockScheme( void );
extern HB_EXPORT BOOL         hb_setGetHardCommit( void );
extern HB_EXPORT BOOL         hb_setGetForceOpt( void );
extern HB_EXPORT BOOL         hb_setGetDefExtension( void );
extern HB_EXPORT const char * hb_setGetEOL( void );
extern HB_EXPORT BOOL         hb_setGetTrimFileName( void );
extern HB_EXPORT const char * hb_setGetOSCODEPAGE( void );
extern HB_EXPORT BOOL         hb_setGetTrace( void );
extern HB_EXPORT char *       hb_setGetTraceFile( void );
extern HB_EXPORT char         hb_setGetTraceStack( void );
extern HB_EXPORT int          hb_setGetErrorLoop( void );
extern HB_EXPORT BOOL         hb_setGetOutputSafety( void );
extern HB_EXPORT BOOL         hb_setGetBackgroundTasks( void );
extern HB_EXPORT BOOL         hb_setGetWinPrinter( void );
extern HB_EXPORT HB_FHANDLE   hb_setGetWinHan( void );
extern HB_EXPORT char *       hb_setGetPrinterJob( void );
extern HB_EXPORT int          hb_setGetBackGroundTick( void );
extern HB_EXPORT BOOL         hb_setGetAppendError( void );
extern HB_EXPORT char *       hb_setGetErrorLog( void );
extern HB_EXPORT BOOL         hb_setGetWorkareasShared( void );
extern HB_EXPORT UCHAR *      hb_osEncode( UCHAR * szFileName, BOOL * pfFree );
extern HB_EXPORT UCHAR *      hb_osDecode( UCHAR * szFileName, BOOL * pfFree );

HB_EXTERN_END

#endif /* HB_SET_H_ */

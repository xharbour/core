/*
 * $Id: hbapigt.h 9722 2012-10-02 06:13:24Z andijahja $
 */

/*
 * Harbour Project source code:
 * Header file for the Terminal API
 *
 * Copyright 1999 {list of individual authors and e-mail addresses}
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
 * Copyright 1999 David G. Holm <dholm@jsd-llc.com>
 *    Keyboard related declarations
 *    Cursor declarations
 * See above for licensing terms.
 *
 * Copyright 1999-2001 Viktor Szakats <viktor.szakats@syenar.hu>
 *    Mouse related declarations
 *    Undocumented GT API declarations
 *
 * Copyright 2005 Przemyslaw Czerpak < druzus /at/ priv.onet.pl >
 *    Internal GT code reimplemented in differ way
 *
 * See doc/license.txt for licensing terms.
 *
 */

#ifndef HB_APIGT_H_
#define HB_APIGT_H_

#include "hbapi.h"

HB_EXTERN_BEGIN

#include "inkey.ch"
#include "setcurs.ch"
#include "hbgtinfo.ch"

/* maximum length of color string */
#define HB_CLRSTR_LEN           64
#define CLR_STRLEN              HB_CLRSTR_LEN
/* attributes for color strings, these are the same as the ones in color.ch
   but prefixed with HB_ to avoid collision. */
#define HB_CLR_STANDARD         0
#define HB_CLR_ENHANCED         1
#define HB_CLR_BORDER           2
#define HB_CLR_BACKGROUND       3
#define HB_CLR_UNSELECTED       4
#define HB_CLR_MAX_             HB_CLR_UNSELECTED


/* strings for borders (same as box.ch, but defined for use by C) */

/* Note. This part will never be used, but is being kept in the source,
         so that if you use code page 437, you can see what the line
         draw characters are supposed to look like.
                                01234567
#define _B_SINGLE              "�Ŀ�����"
#define _B_DOUBLE              "�ͻ���Ⱥ"
#define _B_SINGLE_DOUBLE       "�ķ���Ӻ"
#define _B_DOUBLE_SINGLE       "�͸���Գ"
#define HB_B_SINGLE_V          '�'
#define HB_B_SINGLE_H          '�'
#define HB_B_DOUBLE_V          '�'
#define HB_B_DOUBLE_H          '�'
*/
#define _B_SINGLE              "\xDA\xC4\xBF\xB3\xD9\xC4\xC0\xB3"
#define _B_DOUBLE              "\xC9\xCD\xBB\xBA\xBC\xCD\xC8\xBA"
#define _B_SINGLE_DOUBLE       "\xD6\xC4\xB7\xBA\xBD\xC4\xD3\xBA"
#define _B_DOUBLE_SINGLE       "\xD5\xCD\xB8\xB3\xBE\xCD\xD4\xB3"
#define HB_B_SINGLE_V          '\xB3'
#define HB_B_SINGLE_H          '\xC4'
#define HB_B_DOUBLE_V          '\xBA'
#define HB_B_DOUBLE_H          '\xCD'

#if defined( HB_COMPAT_C53 ) && !defined( HB_C52_STRICT )
#  define HB_DEFAULT_INKEY_BUFSIZE  50
#else
#  define HB_DEFAULT_INKEY_BUFSIZE  15
#endif


/* standard input/output handles
 * when HB_IO_WIN is set file handles with numbers 0, 1, 2 are
 * transalted inside filesys to
 *    GetStdHandle( STD_INPUT_HANDLE )
 *    GetStdHandle( STD_OUTPUT_HANDLE )
 *    GetStdHandle( STD_ERROR_HANDLE )
 */
#define HB_STDIN_HANDLE    0
#define HB_STDOUT_HANDLE   1
#define HB_STDERR_HANDLE   2


/* structure used to pass/receive parameters in hb_gtInfo() */

typedef struct
{
   PHB_ITEM pNewVal;
   PHB_ITEM pResult;
   PHB_ITEM pNewVal2;
} HB_GT_INFO, * PHB_GT_INFO;

/* Public interface. These should never change, only be added to. */

extern HB_EXPORT HB_ERRCODE hb_gtInit( HB_FHANDLE hFilenoStdin, HB_FHANDLE hFilenoStdout, HB_FHANDLE hFilenoStderr );
extern HB_EXPORT HB_ERRCODE hb_gtExit( void );
extern HB_EXPORT HB_ERRCODE hb_gtBox( SHORT uiTop, SHORT uiLeft, SHORT uiBottom, SHORT uiRight, BYTE * pbyFrame );
extern HB_EXPORT HB_ERRCODE hb_gtBoxD( SHORT uiTop, SHORT uiLeft, SHORT uiBottom, SHORT uiRight );
extern HB_EXPORT HB_ERRCODE hb_gtBoxS( SHORT uiTop, SHORT uiLeft, SHORT uiBottom, SHORT uiRight );
extern HB_EXPORT HB_ERRCODE hb_gtDrawBox( SHORT uiTop, SHORT uiLeft, SHORT uiBottom, SHORT uiRight, BYTE * pbyFrame, int iColor );
extern HB_EXPORT HB_ERRCODE hb_gtColorSelect( USHORT uiColorIndex );
extern HB_EXPORT int     hb_gtColorToN( const char * szColorString );
extern HB_EXPORT HB_ERRCODE hb_gtColorsToString( int * pColors, int iColorCount, char * pszColorString, int iBufSize );
extern HB_EXPORT HB_ERRCODE hb_gtDispBegin( void );
extern HB_EXPORT USHORT  hb_gtDispCount( void );
extern HB_EXPORT HB_ERRCODE hb_gtDispEnd( void );
extern HB_EXPORT HB_ERRCODE hb_gtDrawShadow( USHORT uiTop, USHORT uiLeft, USHORT uiBottom, USHORT uiRight, BYTE byAttr );
extern HB_EXPORT HB_ERRCODE hb_gtGetBlink( BOOL * pbBlink );
extern HB_EXPORT HB_ERRCODE hb_gtGetColorStr( char * pszColorString );
extern HB_EXPORT HB_ERRCODE hb_gtGetCursor( USHORT * puiCursorShape );
extern HB_EXPORT HB_ERRCODE hb_gtGetPos( SHORT * piRow, SHORT * piCol );
extern HB_EXPORT BOOL    hb_gtIsColor( void );
extern HB_EXPORT USHORT  hb_gtMaxCol( void );
extern HB_EXPORT USHORT  hb_gtMaxRow( void );
extern HB_EXPORT HB_ERRCODE hb_gtPostExt( void );
extern HB_EXPORT HB_ERRCODE hb_gtPreExt( void );
extern HB_EXPORT HB_ERRCODE hb_gtSuspend( void ); /* prepare the reminal for shell output */
extern HB_EXPORT HB_ERRCODE hb_gtResume( void ); /* resume the terminal after the shell output */
extern HB_EXPORT int     hb_gtReadKey( int iEventMask );
extern HB_EXPORT HB_ERRCODE hb_gtRectSize( int iTop, int iLeft, int iBottom, int iRight, HB_SIZE * puiBuffSize );
extern HB_EXPORT HB_ERRCODE hb_gtRepChar( USHORT uiRow, USHORT uiCol, BYTE byChar, USHORT uiCount );
extern HB_EXPORT HB_ERRCODE hb_gtSave( USHORT uiTop, USHORT uiLeft, USHORT uiBottom, USHORT uiRight, void * pScrBuff );
extern HB_EXPORT HB_ERRCODE hb_gtRest( USHORT uiTop, USHORT uiLeft, USHORT uiBottom, USHORT uiRight, void * pScrBuff );
extern HB_EXPORT HB_ERRCODE hb_gtGetChar( USHORT uiRow, USHORT uiCol, BYTE * pbColor, BYTE * pbAttr, USHORT * pusChar );
extern HB_EXPORT HB_ERRCODE hb_gtPutChar( USHORT uiRow, USHORT uiCol, BYTE bColor, BYTE bAttr, USHORT usChar );
extern HB_EXPORT HB_ERRCODE hb_gtBeginWrite( void );
extern HB_EXPORT HB_ERRCODE hb_gtEndWrite( void );
extern HB_EXPORT HB_ERRCODE hb_gtScrDim( USHORT * puiHeight, USHORT * puiWidth );
extern HB_EXPORT HB_ERRCODE hb_gtScroll( USHORT uiTop, USHORT uiLeft, USHORT uiBottom, USHORT uiRight, SHORT iRows, SHORT iCols );
extern HB_EXPORT HB_ERRCODE hb_gtScrollUp( USHORT uiRows );
extern HB_EXPORT HB_ERRCODE hb_gtSetAttribute( USHORT uiTop, USHORT uiLeft, USHORT uiBottom, USHORT uiRight, BYTE byAttr );
extern HB_EXPORT HB_ERRCODE hb_gtSetBlink( BOOL bBlink );
extern HB_EXPORT HB_ERRCODE hb_gtSetColorStr( const char * pszColorString );
extern HB_EXPORT HB_ERRCODE hb_gtSetCursor( USHORT uiCursorShape );
extern HB_EXPORT HB_ERRCODE hb_gtSetMode( USHORT uiRows, USHORT uiCols );
extern HB_EXPORT HB_ERRCODE hb_gtSetPos( SHORT iRow, SHORT iCol );
extern HB_EXPORT HB_ERRCODE hb_gtSetSnowFlag( BOOL bNoSnow );
extern HB_EXPORT HB_ERRCODE hb_gtTone( double dFrequency, double dDuration );
extern HB_EXPORT HB_ERRCODE hb_gtWrite( BYTE * pbyStr, HB_SIZE ulLen );
extern HB_EXPORT HB_ERRCODE hb_gtWriteAt( USHORT uiRow, USHORT uiCol, BYTE * pbyStr, HB_SIZE ulLen );
extern HB_EXPORT HB_ERRCODE hb_gtWriteCon( BYTE * pbyStr, HB_SIZE ulLen );
extern HB_EXPORT HB_ERRCODE hb_gtPutText( USHORT uiRow, USHORT uiCol, BYTE * pStr, HB_SIZE ulLength, int iColor );
extern HB_EXPORT const char * hb_gtVersion( int iType );
extern HB_EXPORT HB_ERRCODE hb_gtOutStd( BYTE * pbyStr, HB_SIZE ulLen );
extern HB_EXPORT HB_ERRCODE hb_gtOutErr( BYTE * pbyStr, HB_SIZE ulLen );
extern HB_EXPORT HB_ERRCODE hb_gtSetDispCP( const char * pszTermCDP, const char * pszHostCDP, BOOL fBox );
extern HB_EXPORT HB_ERRCODE hb_gtSetKeyCP( const char * pszTermCDP, const char * pszHostCDP );
extern HB_EXPORT HB_ERRCODE hb_gtInfo( int iType, PHB_GT_INFO pInfo );
extern HB_EXPORT int     hb_gtAlert( PHB_ITEM pMessage, PHB_ITEM pOptions, int iClrNorm, int iClrHigh, double dDelay );
extern HB_EXPORT int     hb_gtSetFlag( int iType, int iNewValue );
extern HB_EXPORT int     hb_gtGetCurrColor( void );
extern HB_EXPORT int     hb_gtGetClearColor( void );
extern HB_EXPORT HB_ERRCODE hb_gtSetClearColor( int );
extern HB_EXPORT int     hb_gtGetClearChar( void );
extern HB_EXPORT HB_ERRCODE hb_gtSetClearChar( int );
extern HB_EXPORT HB_ERRCODE hb_gtGetScrChar( int iRow, int iCol, BYTE * pbColor, BYTE * pbAttr, USHORT * pusChar );
extern HB_EXPORT HB_ERRCODE hb_gtPutScrChar( int iRow, int iCol, BYTE bColor, BYTE bAttr, USHORT usChar );
extern HB_EXPORT HB_ERRCODE hb_gtFlush( void );
extern HB_EXPORT HB_ERRCODE hb_gtGetPosEx( int * piRow, int * piCol );
extern HB_EXPORT HB_ERRCODE hb_gtScrollEx( int iTop, int iLeft, int iBottom, int iRight, int iColor, int iChar, int iRows, int iCols );
extern HB_EXPORT HB_ERRCODE hb_gtBoxEx( int iTop, int iLeft, int iBottom, int iRight, BYTE * pbyFrame, int iColor );
extern HB_EXPORT int     hb_gtGfxPrimitive( int iType, int iTop, int iLeft, int iBottom, int iRight, int iColor );
extern HB_EXPORT HB_ERRCODE hb_gtGfxText( int iTop, int iLeft, const char * szText, int iColor, int iSize, int iWidth );

extern HB_EXPORT BOOL    hb_mouseIsPresent( void );
extern HB_EXPORT BOOL    hb_mouseGetCursor( void );
extern HB_EXPORT void    hb_mouseSetCursor( BOOL bVisible );
extern HB_EXPORT int     hb_mouseCol( void );
extern HB_EXPORT int     hb_mouseRow( void );
extern HB_EXPORT void    hb_mouseGetPos( int * piRow, int * piCol );
extern HB_EXPORT void    hb_mouseSetPos( int iRow, int iCol );
extern HB_EXPORT void    hb_mouseSetBounds( int iTop, int iLeft, int iBottom, int iRight );
extern HB_EXPORT void    hb_mouseGetBounds( int * piTop, int * piLeft, int * piBottom, int * piRight );
extern HB_EXPORT int     hb_mouseStorageSize( void );
extern HB_EXPORT void    hb_mouseSaveState( BYTE * pBuffer );
extern HB_EXPORT void    hb_mouseRestoreState( BYTE * pBuffer );
extern HB_EXPORT int     hb_mouseGetDoubleClickSpeed( void );
extern HB_EXPORT void    hb_mouseSetDoubleClickSpeed( int iSpeed );
extern HB_EXPORT int     hb_mouseCountButton( void );
extern HB_EXPORT BOOL    hb_mouseButtonState( int iButton );
extern HB_EXPORT BOOL    hb_mouseButtonPressed( int iButton, int * piRow, int * piCol );
extern HB_EXPORT BOOL    hb_mouseButtonReleased( int iButton, int * piRow, int * piCol );
extern HB_EXPORT int     hb_mouseReadKey( int iEventMask );

typedef struct
{
   int   iTop;
   int   iLeft;
   int   iBottom;
   int   iRight;
} HB_GT_RECT;
typedef HB_GT_RECT * PHB_GT_RECT;

typedef struct
{
   int   iRow;
   int   iCol;
} HB_GT_CORD;
typedef HB_GT_CORD * PHB_GT_CORD;

/* Undocumented CA-Clipper 5.x GT API calls */

#define HB_GT_WND void
#define HB_GT_RGB void
#define HB_GT_SLR void

extern HB_EXPORT void    hb_gtWCreate( HB_GT_RECT * rect, HB_GT_WND ** wnd );
extern HB_EXPORT void    hb_gtWDestroy( HB_GT_WND * wnd );
extern HB_EXPORT BOOL    hb_gtWFlash( void );
extern HB_EXPORT void    hb_gtWApp( HB_GT_WND ** wnd );
extern HB_EXPORT void    hb_gtWCurrent( HB_GT_WND * wnd );
extern HB_EXPORT void    hb_gtWPos( HB_GT_WND * wnd, HB_GT_RECT * rect );
extern HB_EXPORT BOOL    hb_gtWVis( HB_GT_WND * wnd, USHORT uiStatus );

extern HB_EXPORT HB_ERRCODE hb_gtSLR( HB_GT_SLR * pSLR ); /* System Level Request */
extern HB_EXPORT HB_ERRCODE hb_gtModalRead( void * );
extern HB_EXPORT HB_ERRCODE hb_gtFlushCursor( void );
extern HB_EXPORT HB_ERRCODE hb_gtSetColor( HB_GT_RGB * color );
extern HB_EXPORT HB_ERRCODE hb_gtGetColor( HB_GT_RGB * color );
extern HB_EXPORT HB_ERRCODE hb_gtSetBorder( HB_GT_RGB * color );


/* Keyboard related declarations */

#define HB_BREAK_FLAG 256 /* 256, because that's what DJGPP returns Ctrl+Break as.
                             Clipper has no key code 256, so it may as well be
                             used for all the Harbour builds that need it */

#define INKEY_RAW 256   /* Minimally Decoded Keyboard Events */

/* Harbour keyboard support functions */
extern HB_EXPORT int     hb_inkey( BOOL bWait, double dSeconds, int iEvenMask ); /* Wait for keyboard input */
extern HB_EXPORT void    hb_inkeyPut( int ch );          /* Inserts an inkey code into the keyboard buffer */
extern HB_EXPORT void    hb_inkeyIns( int ch );          /* Inserts an inkey code into the keyboard buffer */
extern HB_EXPORT int     hb_inkeyLast( int iEvenMask );  /* Return the value of the last key that was extracted */
extern HB_EXPORT int     hb_inkeyNext( int iEvenMask );  /* Return the next key without extracting it */
extern HB_EXPORT void    hb_inkeyPoll( void );           /* Poll the console keyboard to stuff the Harbour buffer */
extern HB_EXPORT void    hb_inkeyReset( void );          /* Reset the Harbour keyboard buffer */
extern HB_EXPORT void    hb_inkeySetCancelKeys( int CancelKey, int CancelKeyEx ); /* Set keycodes for Cancel key (usually K_ALT_C) */
extern HB_EXPORT void    hb_inkeySetText( const char * szText, HB_SIZE ulLen ); /* Set text into inkey buffer */
extern HB_EXPORT int     hb_inkeySetLast( int iKey );    /* Set new LASTKEY() value, return previous one */
extern HB_EXPORT void    hb_inkeyExit( void );           /* reset inkey pool to default state and free any allocated resources */

/* SetKey related declarations */

extern HB_EXPORT void    hb_setkeyInit( void );
extern HB_EXPORT void    hb_setkeyExit( void );


HB_EXTERN_END

#endif /* HB_APIGT_H_ */

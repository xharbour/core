/*
 * $Id: gtwvt.h 9791 2012-10-31 11:34:30Z andijahja $
 */

/*
 * Harbour Project source code:
 * Header File for Video subsystem for Win32 using GUI windows instead of Console
 * Copyright 2003 Peter Rees <peter@rees.co.nz>
 *                Rees Software & Systems Ltd
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

#ifndef HB_WVT_H_

#define HB_WVT_H_

#define HB_GT_NAME  WVT


#include <windows.h>
/*#include <winuser.h>*/
/*#include <commctrl.h>*/

#include "hbset.h"
#include "hbgtcore.h"
#include "hbinit.h"
#include "hbapicdp.h"
#include "hbapierr.h"
#include "hbapiitm.h"
#include "inkey.ch"
#include "error.ch"
#include "hbvm.h"
#include "hbgfxdef.ch"

#if defined( __DMC__ )
   typedef DWORD UINT_PTR;
#endif

#define WVT_CHAR_QUEUE_SIZE   128
#define WVT_MAX_TITLE_SIZE    128
#define WVT_MAX_ROWS          256
#define WVT_MAX_COLS          256
#define WVT_MAX_WINDOWS       256
#if defined( HB_OS_WIN_CE )
#  define WVT_DEFAULT_ROWS          15
#  define WVT_DEFAULT_COLS          50
#  define WVT_DEFAULT_FONT_HEIGHT   12
#  define WVT_DEFAULT_FONT_WIDTH     8
#else
#  define WVT_DEFAULT_ROWS          25
#  define WVT_DEFAULT_COLS          80
#  define WVT_DEFAULT_FONT_HEIGHT   20
#  define WVT_DEFAULT_FONT_WIDTH    10
#endif
#define WVT_DEFAULT_FONT_NAME    "Courier New"

#define BLACK          RGB( 0x0 ,0x0 ,0x0  )
#define BLUE           RGB( 0x0 ,0x0 ,0x85 )
#define GREEN          RGB( 0x0 ,0x85,0x0  )
#define CYAN           RGB( 0x0 ,0x85,0x85 )
#define RED            RGB( 0x85,0x0 ,0x0  )
#define MAGENTA        RGB( 0x85,0x0 ,0x85 )
#define BROWN          RGB( 0x85,0x85,0x0  )
#define WHITE          RGB( 0xC6,0xC6,0xC6 )
#define LIGHT_GRAY     RGB( 0x60,0x60,0x60 )
#define BRIGHT_BLUE    RGB( 0x00,0x00,0xFF )
#define BRIGHT_GREEN   RGB( 0x60,0xFF,0x60 )
#define BRIGHT_CYAN    RGB( 0x60,0xFF,0xFF )
#define BRIGHT_RED     RGB( 0xF8,0x00,0x26 )
#define BRIGHT_MAGENTA RGB( 0xFF,0x60,0xFF )
#define YELLOW         RGB( 0xFF,0xFF,0x00 )
#define BRIGHT_WHITE   RGB( 0xFF,0xFF,0xFF )

#define WM_MY_UPDATE_CARET ( WM_USER + 0x0101 )

typedef struct
{
   PHB_GT   pGT;                          /* core GT pointer */
   int      iHandle;                      /* window number */

   USHORT   ROWS;                         /* number of displayable rows in window */
   USHORT   COLS;                         /* number of displayable columns in window */

   COLORREF COLORS[ 16 ];                 /* colors */

   BOOL     CaretExist;                   /* TRUE if a caret has been created */
   BOOL     CaretHidden;                  /* TRUE if a caret has been hiden */
   int      CaretSize;                    /* Size of solid caret */

   POINT    MousePos;                     /* the last mouse position */
   BOOL     MouseMove;                    /* Flag to say whether to return mouse movement events */

   int      Keys[ WVT_CHAR_QUEUE_SIZE ];  /* Array to hold the characters & events */
   int      keyPointerIn;                 /* Offset into key array for character to be placed */
   int      keyPointerOut;                /* Offset into key array of next character to read */
   int      keyLast;                      /* last inkey code value in buffer */

   POINT    PTEXTSIZE;                    /* size of the fixed width font */
   BOOL     FixedFont;                    /* TRUE if current font is a fixed font */
   int      FixedSize[ WVT_MAX_COLS ];    /* buffer for ExtTextOut() to emulate fixed pitch when Proportional font selected */
   int      fontHeight;                   /* requested font height */
   int      fontWidth;                    /* requested font width */
   int      fontWeight;                   /* Bold level */
   int      fontQuality;                  /* requested font quality */
   char     fontFace[ LF_FACESIZE ];      /* requested font face name LF_FACESIZE #defined in wingdi.h */
   HFONT    hFont;                        /* current font handle */

   HWND     hWnd;                         /* the window handle */

   PHB_CODEPAGE hostCDP;                  /* Host/HVM CodePage for unicode output translations */
   PHB_CODEPAGE inCDP;                    /* Host/HVM CodePage for unicode input translations */

   int      CodePage;                     /* Code page to use for display characters */
   BOOL     Win9X;                        /* Flag to say if running on Win9X not NT/2000/XP */
   BOOL     AltF4Close;                   /* Can use Alt+F4 to close application */
   BOOL     CentreWindow;                 /* True if window is to be Reset into centre of window */

   BOOL     IgnoreWM_SYSCHAR;

   BOOL     bMaximized;                   /* Flag is set when window has been maximized */
   BOOL     bBeingMarked;                 /* Flag to control DOS window like copy operation */
   BOOL     bBeginMarked;

   BOOL     bResizable;
   BOOL     bSelectCopy;
   char *   pszSelectCopy;
   BOOL     bClosable;

} HB_GTWVT, * PHB_GTWVT;

/* xHarbour compatible definitions */
#if !defined( K_SH_LEFT )
#define K_SH_LEFT           K_LEFT   /* Shift-Left  == Left  */
#define K_SH_UP             K_UP     /* Shift-Up    == Up    */
#define K_SH_RIGHT          K_RIGHT  /* Shift-Right == Right */
#define K_SH_DOWN           K_DOWN   /* Shift-Down  == Down  */
#define K_SH_INS            K_INS    /* Shift-Ins   == Ins   */
#define K_SH_DEL            K_DEL    /* Shift-Del   == Del   */
#define K_SH_HOME           K_HOME   /* Shift-Home  == Home  */
#define K_SH_END            K_END    /* Shift-End   == End   */
#define K_SH_PGUP           K_PGUP   /* Shift-PgUp  == PgUp  */
#define K_SH_PGDN           K_PGDN   /* Shift-PgDn  == PgDn  */
#define K_SH_RETURN         K_RETURN /* Shift-Enter == Enter */
#define K_SH_ENTER          K_ENTER  /* Shift-Enter == Enter */
#endif

#ifndef WM_MOUSEWHEEL
#  define WM_MOUSEWHEEL 0x020A
#endif

#define SYS_EV_MARK  1000

#endif /* HB_WVT_H_ */

/*
 * $Id: gttrm.c 9725 2012-10-02 10:56:51Z andijahja $
 */

/*
 * Harbour Project source code:
 * Video subsystem - terminal GT driver
 *
 * Unlike GTSLN and GTCRS this GT driver does not use termcap/terminfo
 * for terminal escape sequences but uses hard coded ones so it
 * can be compiled in any system but supports only terminals which
 * exactly pass given capabilities. To reduce possible problems
 * intentionally only basic capabilities are used. It quite often gives
 * better results then the code using [n]Curses or SLang
 *
 * Now it support the following terminals:
 *   linux, pc-ansi, xterm
 *
 * Copyright 2007 Przemyslaw Czerpak <druzus /at/ priv.onet.pl>
 * www - http://www.harbour-project.org
 *
 * I used my code from other GT drivers (GTCRS, GTPCA)
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

/* NOTE: User programs should never call this layer directly! */

#define HB_GT_NAME TRM

#include "hbgtcore.h"
#include "hbinit.h"
#include "hbapicdp.h"
#include "hbapiitm.h"
#include "hbapifs.h"
#include "hbdate.h"
#include "inkey.ch"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#if defined( HB_OS_UNIX )
# include <errno.h>
# include <time.h>
# include <unistd.h>
# include <signal.h>
# include <termios.h>
# include <sys/stat.h>
# include <sys/types.h>
# include <sys/time.h>
# include <sys/ioctl.h>
# include <sys/wait.h>
#endif
#ifdef HAVE_GPM_H
# include <gpm.h>
#endif

#ifndef O_ACCMODE
#  define O_ACCMODE        ( O_RDONLY | O_WRONLY | O_RDWR )
#endif

static int           s_GtId;
static HB_GT_FUNCS   SuperTable;
#define HB_GTSUPER         ( &SuperTable )
#define HB_GTID_PTR        ( &s_GtId )

#define HB_GTTRM_ATTR_CHAR 0x00FF
#define HB_GTTRM_ATTR_STD  0x0000
#if 0
#define HB_GTTRM_ATTR_ALT  0x0100
#define HB_GTTRM_ATTR_PROT 0x0200
#define HB_GTTRM_ATTR_ACSC 0x0400
#else
#define HB_GTTRM_ATTR_ALT  0x0100
#define HB_GTTRM_ATTR_PROT 0x0100
#define HB_GTTRM_ATTR_ACSC 0x0100
#endif
#define HB_GTTRM_ATTR_BOX  0x0800

#define TERM_ANSI          1
#define TERM_LINUX         2
#define TERM_XTERM         3
#define TERM_PUTTY         4

#define NO_STDKEYS         96
#define NO_EXTDKEYS        30

#define STDIN_BUFLEN       128

#define ESC_DELAY          25

#define IS_EVTFDSTAT( x ) ( ( x ) >= 0x01 && ( x ) <= 0x03 )
#define EVTFDSTAT_RUN      0x01
#define EVTFDSTAT_STOP     0x02
#define EVTFDSTAT_DEL      0x03

/* mouse button states */
#define M_BUTTON_LEFT      0x0001
#define M_BUTTON_RIGHT     0x0002
#define M_BUTTON_MIDDLE    0x0004
#define M_BUTTON_LDBLCK    0x0010
#define M_BUTTON_RDBLCK    0x0020
#define M_BUTTON_MDBLCK    0x0040
#define M_BUTTON_WHEELUP   0x0100
#define M_BUTTON_WHEELDOWN 0x0200
#define M_CURSOR_MOVE      0x0400
#define M_BUTTON_KEYMASK   ( M_BUTTON_LEFT | M_BUTTON_RIGHT | M_BUTTON_MIDDLE )
#define M_BUTTON_DBLMASK   ( M_BUTTON_LDBLCK | M_BUTTON_RDBLCK | M_BUTTON_MDBLCK )

#define MOUSE_NONE         0
#define MOUSE_GPM          1
#define MOUSE_XTERM        2

#if defined( HB_OS_UNIX )

#define TIMEVAL_GET( tv )           gettimeofday( &( tv ), NULL )
#define TIMEVAL_LESS( tv1, tv2 )    ( ( ( tv1 ).tv_sec == ( tv2 ).tv_sec ) ?  \
                                      ( ( tv1 ).tv_usec < ( tv2 ).tv_usec ) :  \
                                      ( ( tv1 ).tv_sec < ( tv2 ).tv_sec ) )
#define TIMEVAL_ADD( dst, src, n )  do {                                \
      ( dst ).tv_sec = ( src ).tv_sec + ( n ) / 1000;                   \
      if( ( ( dst ).tv_usec = ( src ).tv_usec + ( n % 1000 ) * 1000 ) >= 1000000 ) { \
         ( dst ).tv_usec -= 1000000; ( dst ).tv_sec++;                   \
      } \
} while( 0 )

#else

#define TIMEVAL_GET( tv )           do { ( tv ) = hb_dateSeconds(); } while( 0 )
#define TIMEVAL_LESS( tv1, tv2 )    ( ( tv1 ) < ( tv2 ) )
#define TIMEVAL_ADD( dst, src, n )  do { ( dst ) = ( src ) + n / 1000; } while( 0 )

#endif

#define KEY_ALTMASK  0x10000000
#define KEY_CTRLMASK 0x20000000
#define KEY_EXTDMASK 0x40000000
#define KEY_CLIPMASK 0x80000000
#define KEY_MASK     0xF0000000

#define CLR_KEYMASK( x )   ( ( x ) & ~KEY_MASK )
#define GET_KEYMASK( x )   ( ( x ) & KEY_MASK )

#define IS_CLIPKEY( x )    ( ( ( ( x ) & ~0xffff ) ^ KEY_CLIPMASK ) == 0 )
#define SET_CLIPKEY( x )   ( ( ( x ) & 0xffff ) | KEY_CLIPMASK )
#define GET_CLIPKEY( x )   ( ( ( ( x ) & 0x8000 ) ? ~0xffff : 0 ) | ( ( x ) & 0xffff ) )

#define CTRL_SEQ        "\036"
#define ALT_SEQ         "\037"
/*#define NATION_SEQ      "\016"*/

#define EXKEY_F1        ( 0 | KEY_EXTDMASK )
#define EXKEY_F2        ( 1 | KEY_EXTDMASK )
#define EXKEY_F3        ( 2 | KEY_EXTDMASK )
#define EXKEY_F4        ( 3 | KEY_EXTDMASK )
#define EXKEY_F5        ( 4 | KEY_EXTDMASK )
#define EXKEY_F6        ( 5 | KEY_EXTDMASK )
#define EXKEY_F7        ( 6 | KEY_EXTDMASK )
#define EXKEY_F8        ( 7 | KEY_EXTDMASK )
#define EXKEY_F9        ( 8 | KEY_EXTDMASK )
#define EXKEY_F10       ( 9 | KEY_EXTDMASK )
#define EXKEY_F11       ( 10 | KEY_EXTDMASK )
#define EXKEY_F12       ( 11 | KEY_EXTDMASK )
#define EXKEY_UP        ( 12 | KEY_EXTDMASK )
#define EXKEY_DOWN      ( 13 | KEY_EXTDMASK )
#define EXKEY_LEFT      ( 14 | KEY_EXTDMASK )
#define EXKEY_RIGHT     ( 15 | KEY_EXTDMASK )
#define EXKEY_INS       ( 16 | KEY_EXTDMASK )
#define EXKEY_DEL       ( 17 | KEY_EXTDMASK )
#define EXKEY_HOME      ( 18 | KEY_EXTDMASK )
#define EXKEY_END       ( 19 | KEY_EXTDMASK )
#define EXKEY_PGUP      ( 20 | KEY_EXTDMASK )
#define EXKEY_PGDN      ( 21 | KEY_EXTDMASK )
#define EXKEY_BS        ( 22 | KEY_EXTDMASK )
#define EXKEY_TAB       ( 23 | KEY_EXTDMASK )
#define EXKEY_ESC       ( 24 | KEY_EXTDMASK )
#define EXKEY_ENTER     ( 25 | KEY_EXTDMASK )
#define EXKEY_KPENTER   ( 26 | KEY_EXTDMASK )
#define EXKEY_CENTER    ( 27 | KEY_EXTDMASK )
#define EXKEY_PRTSCR    ( 28 | KEY_EXTDMASK )
#define EXKEY_PAUSE     ( 29 | KEY_EXTDMASK )

#define K_UNDEF         0x10000
#define K_METAALT       0x10001
#define K_METACTRL      0x10002
#define K_NATIONAL      0x10003
#define K_MOUSETERM     0x10004
#define K_RESIZE        0x10005
#define K_PRTSCR        0x10006
#define K_PAUSE         0x10007

/* xHarbour compatible definitions */
#if ! defined( K_SH_LEFT )
#define K_SH_LEFT       K_LEFT         /* Shift-Left  == Left  */
#define K_SH_UP         K_UP           /* Shift-Up    == Up    */
#define K_SH_RIGHT      K_RIGHT        /* Shift-Right == Right */
#define K_SH_DOWN       K_DOWN         /* Shift-Down  == Down  */
#define K_SH_INS        K_INS          /* Shift-Ins   == Ins   */
#define K_SH_DEL        K_DEL          /* Shift-Del   == Del   */
#define K_SH_HOME       K_HOME         /* Shift-Home  == Home  */
#define K_SH_END        K_END          /* Shift-End   == End   */
#define K_SH_PGUP       K_PGUP         /* Shift-PgUp  == PgUp  */
#define K_SH_PGDN       K_PGDN         /* Shift-PgDn  == PgDn  */
#define K_SH_RETURN     K_RETURN       /* Shift-Enter == Enter */
#define K_SH_ENTER      K_ENTER        /* Shift-Enter == Enter */
#endif

typedef struct
{
   int fd;
   int mode;
   int status;
   void * cargo;
   int ( * eventFunc )( int, int, void * );
} evtFD;

typedef struct
{
   int row, col;
   int buttonstate;
   int lbuttons;
   int lbup_row, lbup_col;
   int lbdn_row, lbdn_col;
   int rbup_row, rbup_col;
   int rbdn_row, rbdn_col;
   int mbup_row, mbup_col;
   int mbdn_row, mbdn_col;
   /* to analize DBLCLK on xterm */
#if defined( HB_OS_UNIX )
   struct timeval BL_time;
   struct timeval BR_time;
   struct timeval BM_time;
#else
   double BL_time;
   double BR_time;
   double BM_time;
#endif
} mouseEvent;

typedef struct _keyTab
{
   int ch;
   int key;
   struct _keyTab * nextCh;
   struct _keyTab * otherCh;
} keyTab;

typedef struct
{
   int key;
   int alt_key;
   int ctrl_key;
   int shift_key;
} ClipKeyCode;

typedef struct
{
   int key;
   const char * seq;
} keySeq;

#define HB_GTTRM_PTR struct _HB_GTTRM *
#define HB_GTTRM_GET( p ) ( ( PHB_GTTRM ) HB_GTLOCAL( p ) )

typedef struct _HB_GTTRM
{
   PHB_GT pGT;

   HB_FHANDLE hFileno;
   HB_FHANDLE hFilenoStdin;
   HB_FHANDLE hFilenoStdout;
   HB_FHANDLE hFilenoStderr;
   int iRow;
   int iCol;
   int iLineBufSize;
   BYTE * pLineBuf;
   int iCurrentSGR, iFgColor, iBgColor, iBold, iBlink, iACSC, iAM;
   int iAttrMask;
   int iCursorStyle;

   BOOL fOutTTY;
   BOOL fStdinTTY;
   BOOL fStdoutTTY;
   BOOL fStderrTTY;

   BOOL fPosAnswer;

#ifndef HB_CDP_SUPPORT_OFF
   PHB_CODEPAGE cdpHost;
   PHB_CODEPAGE cdpOut;
   PHB_CODEPAGE cdpIn;
   PHB_CODEPAGE cdpEN;
#endif
   BOOL fUTF8;
   BYTE keyTransTbl[ 256 ];
   int charmap[ 256 ];

   int chrattr[ 256 ];
   int boxattr[ 256 ];

   int iOutBufSize;
   int iOutBufIndex;
   BYTE * pOutBuf;

   int terminal_type;
   int terminal_ext;

#if defined( HB_OS_UNIX )
   struct termios saved_TIO, curr_TIO;
   BOOL fRestTTY;
#endif

   double dToneSeconds;

   /* input events */
   keyTab * pKeyTab;
   int key_flag;
   int esc_delay;
   int key_counter;
   int nation_mode;

   int mouse_type;
   int mButtons;
   int nTermMouseChars;
   unsigned char cTermMouseBuf[ 3 ];
   mouseEvent mLastEvt;
#ifdef HAVE_GPM_H
   Gpm_Connect Conn;
#endif

   unsigned char stdin_buf[ STDIN_BUFLEN ];
   int stdin_ptr_l;
   int stdin_ptr_r;
   int stdin_inbuf;

   evtFD ** event_fds;
   int efds_size;
   int efds_no;

   /* terminal functions */

   void ( * Init )( HB_GTTRM_PTR );
   void ( * Exit )( HB_GTTRM_PTR );
   void ( * SetTermMode )( HB_GTTRM_PTR, int );
   BOOL ( * GetCursorPos )( HB_GTTRM_PTR, int *, int *, const char * );
   void ( * SetCursorPos )( HB_GTTRM_PTR, int, int );
   void ( * SetCursorStyle )( HB_GTTRM_PTR, int );
   void ( * SetAttributes )( HB_GTTRM_PTR, int );
   BOOL ( * SetMode )( HB_GTTRM_PTR, int *, int * );
   int ( * GetAcsc )( HB_GTTRM_PTR, unsigned char );
   void ( * Tone )( HB_GTTRM_PTR, double, double );
   void ( * Bell )( HB_GTTRM_PTR );
   const char * szAcsc;
} HB_TERM_STATE, HB_GTTRM, * PHB_GTTRM;

/* static variables use by signal handler */
#if defined( HB_OS_UNIX )
static volatile BOOL s_WinSizeChangeFlag  = FALSE;
static volatile BOOL s_fRestTTY           = FALSE;
#endif

/* save old hilit tracking & enable mouse tracking */
static const char *  s_szMouseOn    = "\033[?1001s\033[?1002h";
/* disable mouse tracking & restore old hilit tracking */
static const char *  s_szMouseOff   = "\033[?1002l\033[?1001r";
static const BYTE    s_szBell[]     = { HB_CHAR_BEL, 0 };

/* The tables below are indexed by internal key value,
 * It cause that we don't have to make any linear scans
 * to access information proper ClipKeyCode entry
 */
static const ClipKeyCode   stdKeyTab[ NO_STDKEYS ] = {
   { K_SPACE,   0,            0,        0 },             /*  32 */
   { '!',       0,            0,        0 },             /*  33 */
   { '"',       0,            0,        0 },             /*  34 */
   { '#',       0,            0,        0 },             /*  35 */
   { '$',       0,            0,        0 },             /*  36 */
   { '%',       0,            0,        0 },             /*  37 */
   { '&',       0,            0,        0 },             /*  38 */
   { '\'',      296,          7,        0 },             /*  39 */
   { '(',       0,            0,        0 },             /*  40 */
   { ')',       0,            0,        0 },             /*  41 */
   { '*',       0,            0,        0 },             /*  42 */
   { '+',       0,            0,        0 },             /*  43 */
   { ',',       307,          0,        0 },             /*  44 */
   { '-',       386,          31,       0 },             /*  45 */
   { '.',       308,          0,        0 },             /*  46 */
   { '/',       309,          127,      0 },             /*  47 */
   { '0',       K_ALT_0,      0,        0 },             /*  48 */
   { '1',       K_ALT_1,      0,        0 },             /*  49 */
   { '2',       K_ALT_2,      259,      0 },             /*  50 */
   { '3',       K_ALT_3,      27,       0 },             /*  51 */
   { '4',       K_ALT_4,      28,       0 },             /*  52 */
   { '5',       K_ALT_5,      29,       0 },             /*  53 */
   { '6',       K_ALT_6,      30,       0 },             /*  54 */
   { '7',       K_ALT_7,      31,       0 },             /*  55 */
   { '8',       K_ALT_8,      127,      0 },             /*  56 */
   { '9',       K_ALT_9,      0,        0 },             /*  57 */
   { ':',       0,            0,        0 },             /*  58 */
   { ';',       295,          0,        0 },             /*  59 */
   { '<',       0,            0,        0 },             /*  60 */
   { '=',       K_ALT_EQUALS, 0,        0 },             /*  61 */
   { '>',       0,            0,        0 },             /*  62 */
   { '?',       0,            0,        0 },             /*  63 */
   { '@',       0,            0,        0 },             /*  64 */
   { 'A',       K_ALT_A,      K_CTRL_A, 0 },             /*  65 */
   { 'B',       K_ALT_B,      K_CTRL_B, 0 },             /*  66 */
   { 'C',       K_ALT_C,      K_CTRL_C, 0 },             /*  67 */
   { 'D',       K_ALT_D,      K_CTRL_D, 0 },             /*  68 */
   { 'E',       K_ALT_E,      K_CTRL_E, 0 },             /*  69 */
   { 'F',       K_ALT_F,      K_CTRL_F, 0 },             /*  70 */
   { 'G',       K_ALT_G,      K_CTRL_G, 0 },             /*  71 */
   { 'H',       K_ALT_H,      K_CTRL_H, 0 },             /*  72 */
   { 'I',       K_ALT_I,      K_CTRL_I, 0 },             /*  73 */
   { 'J',       K_ALT_J,      K_CTRL_J, 0 },             /*  74 */
   { 'K',       K_ALT_K,      K_CTRL_K, 0 },             /*  75 */
   { 'L',       K_ALT_L,      K_CTRL_L, 0 },             /*  76 */
   { 'M',       K_ALT_M,      K_CTRL_M, 0 },             /*  77 */
   { 'N',       K_ALT_N,      K_CTRL_N, 0 },             /*  78 */
   { 'O',       K_ALT_O,      K_CTRL_O, 0 },             /*  79 */
   { 'P',       K_ALT_P,      K_CTRL_P, 0 },             /*  80 */
   { 'Q',       K_ALT_Q,      K_CTRL_Q, 0 },             /*  81 */
   { 'R',       K_ALT_R,      K_CTRL_R, 0 },             /*  82 */
   { 'S',       K_ALT_S,      K_CTRL_S, 0 },             /*  83 */
   { 'T',       K_ALT_T,      K_CTRL_T, 0 },             /*  84 */
   { 'U',       K_ALT_U,      K_CTRL_U, 0 },             /*  85 */
   { 'V',       K_ALT_V,      K_CTRL_V, 0 },             /*  86 */
   { 'W',       K_ALT_W,      K_CTRL_W, 0 },             /*  87 */
   { 'X',       K_ALT_X,      K_CTRL_X, 0 },             /*  88 */
   { 'Y',       K_ALT_Y,      K_CTRL_Y, 0 },             /*  89 */
   { 'Z',       K_ALT_Z,      K_CTRL_Z, 0 },             /*  90 */
   { '[',       282,          27,       0 },             /*  91 */
   { '\\',      299,          28,       0 },             /*  92 */
   { ']',       283,          29,       0 },             /*  93 */
   { '^',       K_ALT_6,      30,       0 },             /*  94 */
   { '_',       386,          31,       0 },             /*  95 */
   { '`',       297,          297,      0 },             /*  96 */
   { 'a',       K_ALT_A,      K_CTRL_A, 0 },             /*  97 */
   { 'b',       K_ALT_B,      K_CTRL_B, 0 },             /*  98 */
   { 'c',       K_ALT_C,      K_CTRL_C, 0 },             /*  99 */
   { 'd',       K_ALT_D,      K_CTRL_D, 0 },             /* 100 */
   { 'e',       K_ALT_E,      K_CTRL_E, 0 },             /* 101 */
   { 'f',       K_ALT_F,      K_CTRL_F, 0 },             /* 102 */
   { 'g',       K_ALT_G,      K_CTRL_G, 0 },             /* 103 */
   { 'h',       K_ALT_H,      K_CTRL_H, 0 },             /* 104 */
   { 'i',       K_ALT_I,      K_CTRL_I, 0 },             /* 105 */
   { 'j',       K_ALT_J,      K_CTRL_J, 0 },             /* 106 */
   { 'k',       K_ALT_K,      K_CTRL_K, 0 },             /* 107 */
   { 'l',       K_ALT_L,      K_CTRL_L, 0 },             /* 108 */
   { 'm',       K_ALT_M,      K_CTRL_M, 0 },             /* 109 */
   { 'n',       K_ALT_N,      K_CTRL_N, 0 },             /* 110 */
   { 'o',       K_ALT_O,      K_CTRL_O, 0 },             /* 111 */
   { 'p',       K_ALT_P,      K_CTRL_P, 0 },             /* 112 */
   { 'q',       K_ALT_Q,      K_CTRL_Q, 0 },             /* 113 */
   { 'r',       K_ALT_R,      K_CTRL_R, 0 },             /* 114 */
   { 's',       K_ALT_S,      K_CTRL_S, 0 },             /* 115 */
   { 't',       K_ALT_T,      K_CTRL_T, 0 },             /* 116 */
   { 'u',       K_ALT_U,      K_CTRL_U, 0 },             /* 117 */
   { 'v',       K_ALT_V,      K_CTRL_V, 0 },             /* 118 */
   { 'w',       K_ALT_W,      K_CTRL_W, 0 },             /* 119 */
   { 'x',       K_ALT_X,      K_CTRL_X, 0 },             /* 120 */
   { 'y',       K_ALT_Y,      K_CTRL_Y, 0 },             /* 121 */
   { 'z',       K_ALT_Z,      K_CTRL_Z, 0 },             /* 122 */
   { '{',       282,          27,       0 },             /* 123 */
   { '|',       299,          28,       0 },             /* 124 */
   { '}',       283,          29,       0 },             /* 125 */
   { '~',       297,          297,      0 },             /* 126 */
   { K_CTRL_BS, K_ALT_BS,     127,      0 }              /* 127 */
};

static const ClipKeyCode   extdKeyTab[ NO_EXTDKEYS ] = {
   { K_F1,      K_ALT_F1,     K_CTRL_F1,     K_SH_F1    },  /*  00 */
   { K_F2,      K_ALT_F2,     K_CTRL_F2,     K_SH_F2    },  /*  01 */
   { K_F3,      K_ALT_F3,     K_CTRL_F3,     K_SH_F3    },  /*  02 */
   { K_F4,      K_ALT_F4,     K_CTRL_F4,     K_SH_F4    },  /*  03 */
   { K_F5,      K_ALT_F5,     K_CTRL_F5,     K_SH_F5    },  /*  04 */
   { K_F6,      K_ALT_F6,     K_CTRL_F6,     K_SH_F6    },  /*  05 */
   { K_F7,      K_ALT_F7,     K_CTRL_F7,     K_SH_F7    },  /*  06 */
   { K_F8,      K_ALT_F8,     K_CTRL_F8,     K_SH_F8    },  /*  07 */
   { K_F9,      K_ALT_F9,     K_CTRL_F9,     K_SH_F9    },  /*  08 */
   { K_F10,     K_ALT_F10,    K_CTRL_F10,    K_SH_F10   },  /*  09 */
   { K_F11,     K_ALT_F11,    K_CTRL_F11,    K_SH_F11   },  /*  10 */
   { K_F12,     K_ALT_F12,    K_CTRL_F12,    K_SH_F12   },  /*  11 */

   { K_UP,      K_ALT_UP,     K_CTRL_UP,     K_SH_UP    },  /*  12 */
   { K_DOWN,    K_ALT_DOWN,   K_CTRL_DOWN,   K_SH_DOWN  },  /*  13 */
   { K_LEFT,    K_ALT_LEFT,   K_CTRL_LEFT,   K_SH_LEFT  },  /*  14 */
   { K_RIGHT,   K_ALT_RIGHT,  K_CTRL_RIGHT,  K_SH_RIGHT },  /*  15 */
   { K_INS,     K_ALT_INS,    K_CTRL_INS,    K_SH_INS   },  /*  16 */
   { K_DEL,     K_ALT_DEL,    K_CTRL_DEL,    K_SH_DEL   },  /*  17 */
   { K_HOME,    K_ALT_HOME,   K_CTRL_HOME,   K_SH_HOME  },  /*  18 */
   { K_END,     K_ALT_END,    K_CTRL_END,    K_SH_END   },  /*  19 */
   { K_PGUP,    K_ALT_PGUP,   K_CTRL_PGUP,   K_SH_PGUP  },  /*  20 */
   { K_PGDN,    K_ALT_PGDN,   K_CTRL_PGDN,   K_SH_PGDN  },  /*  21 */

   { K_BS,      K_ALT_BS,     127,           K_SH_BS    },  /*  22 */
   { K_TAB,     K_ALT_TAB,    K_CTRL_TAB,    K_SH_TAB   },  /*  23 */
   { K_ESC,     K_ALT_ESC,    K_ESC,         0          },  /*  24 */

   { K_ENTER,   K_ALT_ENTER,  K_CTRL_ENTER,  K_SH_ENTER },  /*  25 */

   { K_ENTER,   KP_ALT_ENTER, K_CTRL_ENTER,  0          },  /*  26 */
   { KP_CENTER, 0,            KP_CTRL_5,     0          },  /*  27 */
   { K_PRTSCR,  0,            K_CTRL_PRTSCR, 0          },  /*  28 */
   { K_PAUSE,   0,            0,             0          } /*  29 */
};

static int getClipKey( int nKey )
{
   int nRet = 0, nFlag, n;

   if( IS_CLIPKEY( nKey ) )
      nRet = GET_CLIPKEY( nKey );
   else
   {
      nFlag = GET_KEYMASK( nKey );
      nKey  = CLR_KEYMASK( nKey );
      if( nFlag & KEY_EXTDMASK )
      {
         if( nKey >= 0 && nKey < NO_EXTDKEYS )
         {
            if( ( nFlag & KEY_ALTMASK ) && ( nFlag & KEY_CTRLMASK ) &&
                extdKeyTab[ nKey ].shift_key != 0 )
               nRet = extdKeyTab[ nKey ].shift_key;
            else if( ( nFlag & KEY_ALTMASK ) && extdKeyTab[ nKey ].alt_key != 0 )
               nRet = extdKeyTab[ nKey ].alt_key;
            else if( ( nFlag & KEY_CTRLMASK )
                     && extdKeyTab[ nKey ].ctrl_key != 0 )
               nRet = extdKeyTab[ nKey ].ctrl_key;
            else
               nRet = extdKeyTab[ nKey ].key;
         }
      }
      else
      {
         if( nKey > 0 && nKey < 32 )
         {
            nFlag |= KEY_CTRLMASK;
            nKey  += ( 'A' - 1 );
         }
         n = nKey - 32;
         if( n >= 0 && n < NO_STDKEYS )
         {
            if( ( nFlag & KEY_ALTMASK ) && ( nFlag & KEY_CTRLMASK ) &&
                stdKeyTab[ n ].shift_key != 0 )
               nRet = stdKeyTab[ n ].shift_key;
            else if( ( nFlag & KEY_ALTMASK ) && stdKeyTab[ n ].alt_key != 0 )
               nRet = stdKeyTab[ n ].alt_key;
            else if( ( nFlag & KEY_CTRLMASK ) && stdKeyTab[ n ].ctrl_key != 0 )
               nRet = stdKeyTab[ n ].ctrl_key;
            else
               nRet = stdKeyTab[ n ].key;
         }
         else
            nRet = nKey;

      }
   }

   return nRet;
}


/* SA_NOCLDSTOP in #if is a hack to detect POSIX compatible environment */
#if defined( HB_OS_UNIX ) && defined( SA_NOCLDSTOP )

static void sig_handler( int iSigNo )
{
   int   e = errno, stat;
   pid_t pid;

   switch( iSigNo )
   {
      case SIGCHLD:
         while( ( pid = waitpid( -1, &stat, WNOHANG ) ) > 0 )
         {
            ;
         }
         break;
      case SIGWINCH:
         s_WinSizeChangeFlag = TRUE;
         break;
      case SIGINT:
         /* s_InetrruptFlag = TRUE; */
         break;
      case SIGQUIT:
         /* s_BreakFlag = TRUE; */
         break;
      case SIGTSTP:
         /* s_DebugFlag = TRUE; */
         break;
      case SIGTTOU:
         s_fRestTTY = FALSE;
         break;
   }
   errno = e;
}

static void set_sig_handler( int iSig )
{
   struct sigaction act;

   sigaction( iSig, 0, &act );
   act.sa_handler = sig_handler;
   act.sa_flags   = SA_RESTART | ( iSig == SIGCHLD ? SA_NOCLDSTOP : 0 );
   sigaction( iSig, &act, 0 );
}

static void set_signals( void )
{
   int i, sigs[] = { SIGINT, SIGQUIT, SIGTSTP, SIGWINCH, SIGCHLD, 0 };

   /* Ignore SIGPIPEs so they don't kill us. */
   signal( SIGPIPE, SIG_IGN );
   for( i = 0; sigs[ i ]; ++i )
   {
      set_sig_handler( sigs[ i ] );
   }
}

#endif

static int hb_gt_trm_getSize( PHB_GTTRM pTerm, int * piRows, int * piCols )
{
   *piRows = *piCols = 0;

#if defined( HB_OS_UNIX )
   if( pTerm->fOutTTY )
   {
      struct winsize win;

      if( ioctl( pTerm->hFileno, TIOCGWINSZ, ( char * ) &win ) != -1 )
      {
         *piRows  = win.ws_row;
         *piCols  = win.ws_col;
      }
   }
#else
   HB_SYMBOL_UNUSED( pTerm );
#endif

   if( *piRows <= 0 || *piCols <= 0 )
   {
      char * env;
      if( ( env = getenv( "COLUMNS" ) ) != NULL )
         *piCols = atoi( env );
      if( ( env = getenv( "LINES" ) ) != NULL )
         *piRows = atoi( env );
   }

   return *piRows > 0 && *piCols > 0;
}

static void hb_gt_trm_termFlush( PHB_GTTRM pTerm )
{
   if( pTerm->iOutBufIndex > 0 )
   {
      hb_fsWriteLarge( pTerm->hFileno, pTerm->pOutBuf, pTerm->iOutBufIndex );
      pTerm->iOutBufIndex = 0;
   }
}

static void hb_gt_trm_termOut( PHB_GTTRM pTerm, const BYTE * pStr, int iLen )
{
   if( pTerm->iOutBufSize )
   {
      int i;
      while( iLen > 0 )
      {
         if( pTerm->iOutBufSize == pTerm->iOutBufIndex )
            hb_gt_trm_termFlush( pTerm );
         i = pTerm->iOutBufSize - pTerm->iOutBufIndex;
         if( i > iLen )
            i = iLen;
         memcpy( pTerm->pOutBuf + pTerm->iOutBufIndex, pStr, i );
         pTerm->iOutBufIndex  += i;
         pStr                 += i;
         iLen                 -= i;
      }
   }
}

static void hb_gt_trm_termOutTrans( PHB_GTTRM pTerm, BYTE * pStr, int iLen, int iAttr )
{
   if( pTerm->iOutBufSize )
   {
#ifdef HB_CDP_SUPPORT_OFF
      HB_SYMBOL_UNUSED( iAttr );
#else
      PHB_CODEPAGE cdp = NULL;

      if( pTerm->fUTF8 )
      {
         if( ( iAttr & ( HB_GTTRM_ATTR_ACSC | HB_GTTRM_ATTR_BOX ) ) &&
             pTerm->cdpEN )
            cdp = pTerm->cdpEN;
         else if( pTerm->cdpHost )
            cdp = pTerm->cdpHost;
         else
            cdp = hb_cdppage();
      }

      if( cdp )
      {
         while( iLen > 0 )
         {
            int i = ( pTerm->iOutBufSize - pTerm->iOutBufIndex ) >> 2;
            if( i < 4 )
            {
               hb_gt_trm_termFlush( pTerm );
               i = pTerm->iOutBufSize >> 2;
            }
            if( i > iLen )
               i = iLen;
            pTerm->iOutBufIndex  += hb_cdpStrnToUTF8( cdp, TRUE, pStr, i,
                                                      pTerm->pOutBuf + pTerm->iOutBufIndex );
            pStr                 += i;
            iLen                 -= i;
         }
      }
      else
#endif
      {
         hb_gt_trm_termOut( pTerm, pStr, iLen );
      }
   }
}

/* ************************************************************************* */

/*
 * KEYBOARD and MOUSE
 */

static int add_efds( PHB_GTTRM pTerm, int fd, int mode,
                     int ( * eventFunc )( int, int, void * ), void * cargo )
{
   evtFD *  pefd = NULL;
   int      i;

   if( eventFunc == NULL && mode != O_RDONLY )
      return -1;

#if defined( HB_OS_UNIX )
   {
      int fl;
      if( ( fl = fcntl( fd, F_GETFL, 0 ) ) == -1 )
         return -1;

      fl &= O_ACCMODE;
      if( ( fl == O_RDONLY && mode == O_WRONLY ) ||
          ( fl == O_WRONLY && mode == O_RDONLY ) )
         return -1;
   }
#endif

   for( i = 0; i < pTerm->efds_no && ! pefd; i++ )
      if( pTerm->event_fds[ i ]->fd == fd )
         pefd = pTerm->event_fds[ i ];

   if( pefd )
   {
      pefd->mode        = mode;
      pefd->cargo       = cargo;
      pefd->eventFunc   = eventFunc;
      pefd->status      = EVTFDSTAT_RUN;
   }
   else
   {
      if( pTerm->efds_size <= pTerm->efds_no )
      {
         if( pTerm->event_fds == NULL )
            pTerm->event_fds = ( evtFD ** )
                               hb_xgrab( ( pTerm->efds_size += 10 ) * sizeof( evtFD * ) );
         else
            pTerm->event_fds = ( evtFD ** )
                               hb_xrealloc( pTerm->event_fds,
                                            ( pTerm->efds_size += 10 ) * sizeof( evtFD * ) );
      }

      pefd                                   = ( evtFD * ) hb_xgrab( sizeof( evtFD ) );
      pefd->fd                               = fd;
      pefd->mode                             = mode;
      pefd->cargo                            = cargo;
      pefd->eventFunc                        = eventFunc;
      pefd->status                           = EVTFDSTAT_RUN;
      pTerm->event_fds[ pTerm->efds_no++ ]   = pefd;
   }

   return fd;
}

#ifdef HAVE_GPM_H
static void del_efds( PHB_GTTRM pTerm, int fd )
{
   int i, n = -1;

   for( i = 0; i < pTerm->efds_no && n == -1; i++ )
      if( pTerm->event_fds[ i ]->fd == fd )
         n = i;

   if( n != -1 )
   {
      hb_xfree( pTerm->event_fds[ n ] );
      pTerm->efds_no--;
      for( i = n; i < pTerm->efds_no; i++ )
         pTerm->event_fds[ i ] = pTerm->event_fds[ i + 1 ];
   }
}
#endif

static void del_all_efds( PHB_GTTRM pTerm )
{
   int i;

   if( pTerm->event_fds != NULL )
   {
      for( i = 0; i < pTerm->efds_no; i++ )
         hb_xfree( pTerm->event_fds[ i ] );

      hb_xfree( pTerm->event_fds );

      pTerm->event_fds  = NULL;
      pTerm->efds_no    = pTerm->efds_size = 0;
   }
}

static int getMouseKey( mouseEvent * mEvt )
{
   int nKey = 0;

   if( mEvt->lbuttons != mEvt->buttonstate )
   {
      if( mEvt->buttonstate & M_CURSOR_MOVE )
      {
         nKey              = K_MOUSEMOVE;
         mEvt->buttonstate &= ~M_CURSOR_MOVE;
      }
      else if( mEvt->buttonstate & M_BUTTON_WHEELUP )
      {
         nKey              = K_MWFORWARD;
         mEvt->buttonstate &= ~M_BUTTON_WHEELUP;
      }
      else if( mEvt->buttonstate & M_BUTTON_WHEELDOWN )
      {
         nKey              = K_MWBACKWARD;
         mEvt->buttonstate &= ~M_BUTTON_WHEELDOWN;
      }
      else
      {
         int butt = mEvt->lbuttons ^ mEvt->buttonstate;

         if( butt & M_BUTTON_LEFT )
         {
            if( mEvt->buttonstate & M_BUTTON_LEFT )
            {
               mEvt->lbdn_row = mEvt->row;
               mEvt->lbdn_col = mEvt->col;
            }
            else
            {
               mEvt->lbup_row = mEvt->row;
               mEvt->lbup_col = mEvt->col;
            }
            nKey              = ( mEvt->buttonstate & M_BUTTON_LEFT ) ?
                                ( ( mEvt->buttonstate & M_BUTTON_LDBLCK ) ? K_LDBLCLK :
                     K_LBUTTONDOWN ) : K_LBUTTONUP;
            mEvt->lbuttons    ^= M_BUTTON_LEFT;
            mEvt->buttonstate &= ~M_BUTTON_LDBLCK;
         }
         else if( butt & M_BUTTON_RIGHT )
         {
            if( mEvt->buttonstate & M_BUTTON_RIGHT )
            {
               mEvt->rbdn_row = mEvt->row;
               mEvt->rbdn_col = mEvt->col;
            }
            else
            {
               mEvt->rbup_row = mEvt->row;
               mEvt->rbup_col = mEvt->col;
            }
            nKey              = ( mEvt->buttonstate & M_BUTTON_RIGHT ) ?
                                ( ( mEvt->buttonstate & M_BUTTON_RDBLCK ) ? K_RDBLCLK :
                     K_RBUTTONDOWN ) : K_RBUTTONUP;
            mEvt->lbuttons    ^= M_BUTTON_RIGHT;
            mEvt->buttonstate &= ~M_BUTTON_RDBLCK;
         }
         else if( butt & M_BUTTON_MIDDLE )
         {
            if( mEvt->buttonstate & M_BUTTON_MIDDLE )
            {
               mEvt->mbdn_row = mEvt->row;
               mEvt->mbdn_col = mEvt->col;
            }
            else
            {
               mEvt->mbup_row = mEvt->row;
               mEvt->mbup_col = mEvt->col;
            }
            nKey              = ( mEvt->buttonstate & M_BUTTON_MIDDLE ) ?
                                ( ( mEvt->buttonstate & M_BUTTON_MDBLCK ) ? K_MDBLCLK :
                     K_MBUTTONDOWN ) : K_MBUTTONUP;
            mEvt->lbuttons    ^= M_BUTTON_MIDDLE;
            mEvt->buttonstate &= ~M_BUTTON_MDBLCK;
         }
         else
            mEvt->lbuttons = mEvt->buttonstate;
      }
   }

   return nKey;
}

static void chk_mevtdblck( PHB_GTTRM pTerm )
{
   int newbuttons = ( pTerm->mLastEvt.buttonstate & ~pTerm->mLastEvt.lbuttons ) & M_BUTTON_KEYMASK;

   if( newbuttons != 0 )
   {
#if defined( HB_OS_UNIX )
      struct timeval tv;
#else
      double         tv;
#endif

      TIMEVAL_GET( tv );
      if( newbuttons & M_BUTTON_LEFT )
      {
         if( TIMEVAL_LESS( tv, pTerm->mLastEvt.BL_time ) )
            pTerm->mLastEvt.buttonstate |= M_BUTTON_LDBLCK;
         TIMEVAL_ADD( pTerm->mLastEvt.BL_time, tv,
                      HB_GTSELF_MOUSEGETDOUBLECLICKSPEED( pTerm->pGT ) );
      }
      if( newbuttons & M_BUTTON_MIDDLE )
      {
         if( TIMEVAL_LESS( tv, pTerm->mLastEvt.BM_time ) )
            pTerm->mLastEvt.buttonstate |= M_BUTTON_MDBLCK;
         TIMEVAL_ADD( pTerm->mLastEvt.BM_time, tv,
                      HB_GTSELF_MOUSEGETDOUBLECLICKSPEED( pTerm->pGT ) );
      }
      if( newbuttons & M_BUTTON_RIGHT )
      {
         if( TIMEVAL_LESS( tv, pTerm->mLastEvt.BR_time ) )
            pTerm->mLastEvt.buttonstate |= M_BUTTON_RDBLCK;
         TIMEVAL_ADD( pTerm->mLastEvt.BR_time, tv,
                      HB_GTSELF_MOUSEGETDOUBLECLICKSPEED( pTerm->pGT ) );
      }
   }
}

static void set_tmevt( PHB_GTTRM pTerm, unsigned char * cMBuf, mouseEvent * mEvt )
{
   int row, col;

   col   = cMBuf[ 1 ] - 33;
   row   = cMBuf[ 2 ] - 33;
   if( mEvt->row != row || mEvt->col != col )
   {
      mEvt->buttonstate |= M_CURSOR_MOVE;
      mEvt->row         = row;
      mEvt->col         = col;
   }

   switch( cMBuf[ 0 ] & 0xC3 )
   {
      case 0x0:
         mEvt->buttonstate |= M_BUTTON_LEFT;
         break;
      case 0x1:
         mEvt->buttonstate |= M_BUTTON_MIDDLE;
         break;
      case 0x2:
         mEvt->buttonstate |= M_BUTTON_RIGHT;
         break;
      case 0x3:
         mEvt->buttonstate &= ~( M_BUTTON_KEYMASK | M_BUTTON_DBLMASK );
         break;
      case 0x40:
         if( cMBuf[ 0 ] & 0x20 )
            mEvt->buttonstate |= M_BUTTON_WHEELUP;
         break;
      case 0x41:
         if( cMBuf[ 0 ] & 0x20 )
            mEvt->buttonstate |= M_BUTTON_WHEELDOWN;
         break;
   }
   chk_mevtdblck( pTerm );
   /* printf("\n\rmouse event: %02x, %02x, %02x\n\r", cMBuf[0], cMBuf[1], cMBuf[2]); */
}

#ifdef HAVE_GPM_H
static int set_gpmevt( int fd, int mode, void * cargo )
{
   int         nKey = 0;
   PHB_GTTRM   pTerm;
   Gpm_Event   gEvt;

   HB_SYMBOL_UNUSED( fd );
   HB_SYMBOL_UNUSED( mode );

   pTerm = ( PHB_GTTRM ) cargo;

   if( Gpm_GetEvent( &gEvt ) > 0 )
   {
      pTerm->mLastEvt.row  = gEvt.y;
      pTerm->mLastEvt.col  = gEvt.x;
      if( gEvt.type & GPM_MOVE )
         pTerm->mLastEvt.buttonstate |= M_CURSOR_MOVE;
      if( gEvt.type & GPM_DOWN )
      {
         if( gEvt.buttons & GPM_B_LEFT )
            pTerm->mLastEvt.buttonstate |= M_BUTTON_LEFT;
         if( gEvt.buttons & GPM_B_MIDDLE )
            pTerm->mLastEvt.buttonstate |= M_BUTTON_MIDDLE;
         if( gEvt.buttons & GPM_B_RIGHT )
            pTerm->mLastEvt.buttonstate |= M_BUTTON_RIGHT;
      }
      else if( gEvt.type & GPM_UP )
      {
         if( gEvt.buttons & GPM_B_LEFT )
            pTerm->mLastEvt.buttonstate &= ~M_BUTTON_LEFT;
         if( gEvt.buttons & GPM_B_MIDDLE )
            pTerm->mLastEvt.buttonstate &= ~M_BUTTON_MIDDLE;
         if( gEvt.buttons & GPM_B_RIGHT )
            pTerm->mLastEvt.buttonstate &= ~M_BUTTON_RIGHT;
      }
   }
   chk_mevtdblck( pTerm );
   nKey = getMouseKey( &pTerm->mLastEvt );

   return nKey ? SET_CLIPKEY( nKey ) : 0;
}

static void flush_gpmevt( PHB_GTTRM pTerm )
{
   if( gpm_fd >= 0 )
   {
      struct timeval tv = { 0, 0 };
      fd_set         rfds;

      FD_ZERO( &rfds );
      FD_SET( gpm_fd, &rfds );

      while( select( gpm_fd + 1, &rfds, NULL, NULL, &tv ) > 0 )
         set_gpmevt( gpm_fd, O_RDONLY, ( void * ) pTerm );

      while( getMouseKey( &pTerm->mLastEvt ) )
         ;
   }
}
#endif

static void disp_mousecursor( PHB_GTTRM pTerm )
{
#ifdef HAVE_GPM_H
   if( ( pTerm->mouse_type & MOUSE_GPM ) && gpm_visiblepointer )
   {
      Gpm_DrawPointer( pTerm->mLastEvt.col, pTerm->mLastEvt.row,
                       gpm_consolefd );
   }
#else
   HB_SYMBOL_UNUSED( pTerm );
#endif
}

static void mouse_init( PHB_GTTRM pTerm )
{
   if( pTerm->terminal_type == TERM_XTERM ||
       pTerm->terminal_type == TERM_LINUX )
   {
      hb_gt_trm_termOut( pTerm, ( BYTE * ) s_szMouseOn, strlen( s_szMouseOn ) );
      hb_gt_trm_termFlush( pTerm );
      memset( ( void * ) &pTerm->mLastEvt, 0, sizeof( pTerm->mLastEvt ) );
      pTerm->mouse_type |= MOUSE_XTERM;
      pTerm->mButtons   = 3;
   }
#ifdef HAVE_GPM_H
   if( pTerm->terminal_type == TERM_LINUX )
   {
      pTerm->Conn.eventMask   =
         GPM_MOVE | GPM_DRAG | GPM_UP | GPM_DOWN | GPM_DOUBLE;
      /* give me move events but handle them anyway */
      pTerm->Conn.defaultMask = GPM_MOVE | GPM_HARD;
      /* only pure mouse events, no Ctrl,Alt,Shft events */
      pTerm->Conn.minMod      = pTerm->Conn.maxMod = 0;
      gpm_zerobased           = 1;
      gpm_visiblepointer      = 0;
      if( Gpm_Open( &pTerm->Conn, 0 ) >= 0 && gpm_fd >= 0 )
      {
         int flags;

         if( ( flags = fcntl( gpm_fd, F_GETFL, 0 ) ) != -1 )
            fcntl( gpm_fd, F_SETFL, flags | O_NONBLOCK );

         memset( ( void * ) &pTerm->mLastEvt, 0, sizeof( pTerm->mLastEvt ) );
         flush_gpmevt( pTerm );
         add_efds( pTerm, gpm_fd, O_RDONLY, set_gpmevt, ( void * ) pTerm );
         pTerm->mouse_type |= MOUSE_GPM;

         /*
          * In recent GPM versions it produce unpleasure noice on the screen
          * so I covered it with this macro, [druzus]
          */
#ifdef HB_GPM_USE_XTRA
         pTerm->mButtons   = Gpm_GetSnapshot( NULL );
#else
         pTerm->mButtons   = 3;
#endif
      }
   }
#endif
}

static void mouse_exit( PHB_GTTRM pTerm )
{
   if( pTerm->mouse_type & MOUSE_XTERM )
   {
      hb_gt_trm_termOut( pTerm, ( BYTE * ) s_szMouseOff, strlen( s_szMouseOff ) );
      hb_gt_trm_termFlush( pTerm );
   }
#ifdef HAVE_GPM_H
   if( ( pTerm->mouse_type & MOUSE_GPM ) && gpm_fd >= 0 )
   {
      del_efds( pTerm, gpm_fd );
      Gpm_Close();
   }
#endif
}

static int get_inch( PHB_GTTRM pTerm, int milisec )
{
   int            nRet  = 0, npfd = -1, nchk = pTerm->efds_no, lRead = 0;
   int            mode, i, n, counter;
   struct timeval tv, * ptv;
   evtFD *        pefd  = NULL;
   fd_set         rfds, wfds;

   if( milisec == 0 )
      ptv = NULL;
   else
   {
      if( milisec < 0 )
         milisec = 0;
      tv.tv_sec   = ( milisec / 1000 );
      tv.tv_usec  = ( milisec % 1000 ) * 1000;
      ptv         = &tv;
   }

   while( nRet == 0 && lRead == 0 )
   {
      n = -1;
      FD_ZERO( &rfds );
      FD_ZERO( &wfds );
      for( i = 0; i < pTerm->efds_no; i++ )
      {
         if( pTerm->event_fds[ i ]->status == EVTFDSTAT_RUN )
         {
            if( pTerm->event_fds[ i ]->mode == O_RDWR
                || pTerm->event_fds[ i ]->mode == O_RDONLY )
            {
               FD_SET( pTerm->event_fds[ i ]->fd, &rfds );
               if( n < pTerm->event_fds[ i ]->fd )
                  n = pTerm->event_fds[ i ]->fd;
            }
            if( pTerm->event_fds[ i ]->mode == O_RDWR
                || pTerm->event_fds[ i ]->mode == O_WRONLY )
            {
               FD_SET( pTerm->event_fds[ i ]->fd, &wfds );
               if( n < pTerm->event_fds[ i ]->fd )
                  n = pTerm->event_fds[ i ]->fd;
            }
         }
      }

      counter = pTerm->key_counter;
      if( select( n + 1, &rfds, &wfds, NULL, ptv ) > 0 )
      {
         for( i = 0; i < pTerm->efds_no; i++ )
         {
            n = ( FD_ISSET( pTerm->event_fds[ i ]->fd, &rfds ) ? 1 : 0 ) |
                ( FD_ISSET( pTerm->event_fds[ i ]->fd, &wfds ) ? 2 : 0 );
            if( n != 0 )
            {
               if( pTerm->event_fds[ i ]->eventFunc == NULL )
               {
                  lRead = 1;
                  if( STDIN_BUFLEN > pTerm->stdin_inbuf )
                  {
                     unsigned char buf[ STDIN_BUFLEN ];

#if defined( HB_OS_UNIX )
                     n  = read( pTerm->event_fds[ i ]->fd, buf,
                                STDIN_BUFLEN - pTerm->stdin_inbuf );
#else
                     n  = hb_fsRead( pTerm->event_fds[ i ]->fd, buf,
                                     STDIN_BUFLEN - pTerm->stdin_inbuf );
#endif
                     if( n == 0 )
                        pTerm->event_fds[ i ]->status = EVTFDSTAT_STOP;
                     else
                        for( i = 0; i < n; i++ )
                        {
                           pTerm->stdin_buf[ pTerm->stdin_ptr_r++ ] = buf[ i ];
                           if( pTerm->stdin_ptr_r == STDIN_BUFLEN )
                              pTerm->stdin_ptr_r = 0;
                           pTerm->stdin_inbuf++;
                        }
                  }
               }
               else if( nRet == 0 && counter == pTerm->key_counter )
               {
                  if( n == 3 )
                     mode = O_RDWR;
                  else if( n == 2 )
                     mode = O_WRONLY;
                  else
                     mode = O_RDONLY;
                  pTerm->event_fds[ i ]->status = EVTFDSTAT_STOP;
                  n                             = ( pTerm->event_fds[ i ]->eventFunc )( pTerm->
                                                                                        event_fds[ i ]->fd,
                                                                                        mode,
                                                                                        pTerm->
                                                                                        event_fds[ i ]->
                                                                                        cargo );
                  if( IS_EVTFDSTAT( n ) )
                  {
                     pTerm->event_fds[ i ]->status = n;
                     if( nchk > i )
                        nchk = i;
                  }
                  else
                  {
                     pTerm->event_fds[ i ]->status = EVTFDSTAT_RUN;
                     if( IS_CLIPKEY( n ) )
                     {
                        nRet  = n;
                        npfd  = pTerm->event_fds[ i ]->fd;
                        if( nchk > i )
                           nchk = i;
                     }
                  }
               }
            }
         }
      }
      else
         lRead = 1;
   }

   for( i = n = nchk; i < pTerm->efds_no; i++ )
   {
      if( pTerm->event_fds[ i ]->status == EVTFDSTAT_DEL )
         hb_xfree( pTerm->event_fds[ i ] );
      else if( pTerm->event_fds[ i ]->fd == npfd )
         pefd = pTerm->event_fds[ i ];
      else
      {
         if( i > n )
            pTerm->event_fds[ n ] = pTerm->event_fds[ i ];
         n++;
      }
   }
   if( pefd )
      pTerm->event_fds[ n++ ] = pefd;
   pTerm->efds_no = n;

   return nRet;
}

static int test_bufch( PHB_GTTRM pTerm, int n, int delay )
{
   int nKey = 0;

   if( pTerm->stdin_inbuf == n )
      nKey = get_inch( pTerm, delay );

   return IS_CLIPKEY( nKey ) ? nKey :
          ( pTerm->stdin_inbuf > n ) ?
          pTerm->stdin_buf[ ( pTerm->stdin_ptr_l + n ) % STDIN_BUFLEN ] : -1;
}

static void free_bufch( PHB_GTTRM pTerm, int n )
{
   if( n > pTerm->stdin_inbuf )
      n = pTerm->stdin_inbuf;
   pTerm->stdin_ptr_l   = ( pTerm->stdin_ptr_l + n ) % STDIN_BUFLEN;
   pTerm->stdin_inbuf   -= n;
}

static int wait_key( PHB_GTTRM pTerm, int milisec )
{
   int      nKey, esc, n, i, ch, counter;
   keyTab * ptr;

#if defined( HB_OS_UNIX )
   if( s_WinSizeChangeFlag )
   {
      s_WinSizeChangeFlag = FALSE;
      return K_RESIZE;
   }
#endif

 restart:
   counter  = ++( pTerm->key_counter );
   nKey     = esc = n = i = 0;
 again:
   if( ( nKey = getMouseKey( &pTerm->mLastEvt ) ) != 0 )
      return nKey;

   ch = test_bufch( pTerm, i, pTerm->nTermMouseChars ? pTerm->esc_delay : milisec );
   if( counter != pTerm->key_counter )
      goto restart;

   if( ch >= 0 && ch <= 255 )
   {
      ++i;
      if( pTerm->nTermMouseChars )
      {
         pTerm->cTermMouseBuf[ 3 - pTerm->nTermMouseChars ] = ch;
         free_bufch( pTerm, i );
         i                                                  = 0;
         if( --pTerm->nTermMouseChars == 0 )
            set_tmevt( pTerm, pTerm->cTermMouseBuf, &pTerm->mLastEvt );
         goto again;
      }

      nKey  = ch;
      ptr   = pTerm->pKeyTab;
      if( i == 1 && nKey == K_ESC && esc == 0 )
         esc = 1;
      while( ch >= 0 && ch <= 255 && ptr != NULL )
      {
         if( ptr->ch == ch )
         {
            if( ptr->key != K_UNDEF )
            {
               nKey = ptr->key;
               switch( nKey )
               {
                  case K_METAALT:
                     pTerm->key_flag         |= KEY_ALTMASK;
                     break;
                  case K_METACTRL:
                     pTerm->key_flag         |= KEY_CTRLMASK;
                     break;
                  case K_NATIONAL:
                     pTerm->nation_mode      = ! pTerm->nation_mode;
                     break;
                  case K_MOUSETERM:
                     pTerm->nTermMouseChars  = 3;
                     break;
                  default:
                     n                       = i;
               }
               if( n != i )
               {
                  free_bufch( pTerm, i );
                  i     = n = nKey = 0;
                  if( esc == 2 )
                     break;
                  esc   = 0;
                  goto again;
               }
            }
            ptr = ptr->nextCh;
            if( ptr )
               if( ( ch = test_bufch( pTerm, i, pTerm->esc_delay ) ) != -1 )
                  ++i;
            if( counter != pTerm->key_counter )
               goto restart;
         }
         else
            ptr = ptr->otherCh;
      }
   }
   if( ch == -1 && pTerm->nTermMouseChars )
      pTerm->nTermMouseChars = 0;

   if( ch != -1 && IS_CLIPKEY( ch ) )
      nKey = GET_CLIPKEY( ch );
   else
   {
      if( esc == 1 && n == 0 && ( ch != -1 || i >= 2 ) )
      {
         nKey  = 0;
         esc   = 2;
         i     = n = 1;
         goto again;
      }
      if( esc == 2 )
      {
         if( nKey != 0 )
            pTerm->key_flag |= KEY_ALTMASK;
         else
            nKey = K_ESC;
         if( n == 1 && i > 1 )
            n = 2;
      }
      else if( n == 0 && i > 0 )
         n = 1;

      if( n > 0 )
         free_bufch( pTerm, n );

      if( pTerm->key_flag != 0 && nKey != 0 )
      {
         nKey              |= pTerm->key_flag;
         pTerm->key_flag   = 0;
      }

#ifndef HB_CDP_SUPPORT_OFF
      if( nKey > 0 && nKey <= 255 && pTerm->fUTF8 && pTerm->cdpIn )
      {
         USHORT uc = 0;
         n = i = 0;
         if( hb_cdpGetFromUTF8( pTerm->cdpIn, FALSE, ( BYTE ) nKey, &n, &uc ) )
         {
            while( n > 0 )
            {
               ch = test_bufch( pTerm, i++, pTerm->esc_delay );
               if( ch < 0 || ch > 255 ||
                   ! hb_cdpGetFromUTF8( pTerm->cdpIn, FALSE, ch, &n, &uc ) )
                  break;
            }
            if( n == 0 )
            {
               free_bufch( pTerm, i );
               nKey = uc;
            }
         }
      }
#endif
      if( nKey > 0 && nKey <= 255 && pTerm->keyTransTbl[ nKey ] )
         nKey = pTerm->keyTransTbl[ nKey ];
/*
      if( pTerm->nation_transtbl && pTerm->nation_mode &&
           nKey >= 32 && nKey < 128 && pTerm->nation_transtbl[nKey] )
         nKey = pTerm->nation_transtbl[nKey];
 */
      if( nKey )
         nKey = getClipKey( nKey );
   }

   return nKey;
}

/* ************************************************************************* */

/*
 * LINUX terminal operations
 */
static void hb_gt_trm_LinuxSetTermMode( PHB_GTTRM pTerm, int iAM )
{
   HB_TRACE( HB_TR_DEBUG, ( "hb_gt_trm_LinuxSetTermMode(%p,%d)", pTerm, iAM ) );

   if( iAM != pTerm->iAM )
   {
      if( iAM == 0 )
         hb_gt_trm_termOut( pTerm, ( BYTE * ) "\x1B[m", 3 );

      hb_gt_trm_termOut( pTerm, ( BYTE * ) ( iAM ? "\x1B[?7h" : "\x1B[?7l" ), 5 );
      pTerm->iAM = iAM;
   }
}

static void hb_gt_trm_LinuxTone( PHB_GTTRM pTerm, double dFrequency, double dDuration )
{
   char escseq[ 64 ];

   HB_TRACE( HB_TR_DEBUG, ( "hb_gt_trm_LinuxTone(%p,%lf,%lf)", pTerm, dFrequency, dDuration ) );

   if( pTerm->iACSC )
   {
      hb_gt_trm_termOut( pTerm, ( BYTE * ) "\033[10m", 5 );
      pTerm->iACSC = 0;
   }
   hb_snprintf( escseq, sizeof( escseq ), "\033[10;%d]\033[11;%d]\007",
                ( int ) dFrequency, ( int ) ( dDuration * 1000.0 / 18.2 ) );
   hb_gt_trm_termOut( pTerm, ( BYTE * ) escseq, strlen( escseq ) );
   hb_gt_trm_termFlush( pTerm );

   /* convert Clipper (DOS) timer tick units to seconds ( x / 18.2 ) */
   hb_idleSleep( dDuration / 18.2 );
}

static void hb_gt_trm_LinuxSetCursorStyle( PHB_GTTRM pTerm, int iStyle )
{
   HB_TRACE( HB_TR_DEBUG, ( "hb_gt_trm_LinuxSetCursorStyle(%p,%d)", pTerm, iStyle ) );

   if( pTerm->iCursorStyle != iStyle )
   {
      int lcurs = -1;

      switch( iStyle )
      {
         case SC_NONE:
            lcurs = 1;
            break;
         case SC_NORMAL:
            lcurs = 2;
            break;
         case SC_INSERT:
            lcurs = 4;
            break;
         case SC_SPECIAL1:
            lcurs = 8;
            break;
         case SC_SPECIAL2:
            /* TODO: find a proper sequqnce to set a cursor
               to SC_SPECIAL2 under Linux console?
               There is no such mode in current stable kernels (2.4.20)
             */
            lcurs = 4;
            break;
      }
      if( lcurs != -1 )
      {
         char escseq[ 64 ];
         hb_snprintf( escseq, sizeof( escseq ), "\033[?25%c\033[?%hdc",
                      iStyle == SC_NONE ? 'l' : 'h', lcurs );
         hb_gt_trm_termOut( pTerm, ( BYTE * ) escseq, strlen( escseq ) );
         pTerm->iCursorStyle = iStyle;
      }
   }
}


/*
 * XTERM terminal operations
 */
static BOOL hb_gt_trm_XtermSetMode( PHB_GTTRM pTerm, int * piRows, int * piCols )
{
   int   iHeight, iWidth;
   char  escseq[ 64 ];

   HB_TRACE( HB_TR_DEBUG, ( "hb_gt_trm_XtermSetMode(%p,%d,%d)", pTerm, *piRows, *piCols ) );

   HB_GTSELF_GETSIZE( pTerm->pGT, &iHeight, &iWidth );
   hb_snprintf( escseq, sizeof( escseq ), "\033[8;%d;%dt", *piRows, *piCols );
   hb_gt_trm_termOut( pTerm, ( BYTE * ) escseq, strlen( escseq ) );
   hb_gt_trm_termFlush( pTerm );

#if defined( HB_OS_UNIX )
   /* dirty hack - wait for SIGWINCH */
   if( *piRows != iHeight || *piCols != iWidth )
      sleep( 3 );
   if( s_WinSizeChangeFlag )
      s_WinSizeChangeFlag = FALSE;
#endif

   hb_gt_trm_getSize( pTerm, piRows, piCols );

   return TRUE;
}

static void hb_gt_trm_XtermSetAttributes( PHB_GTTRM pTerm, int iAttr )
{
   static const int s_AnsiColors[] = { 0, 4, 2, 6, 1, 5, 3, 7 };

   HB_TRACE( HB_TR_DEBUG, ( "hb_gt_trm_XtermSetAttributes(%p,%d)", pTerm, iAttr ) );

   if( pTerm->iCurrentSGR != iAttr )
   {
      int   i, acsc, bg, fg, bold, blink;
      BYTE  buff[ 32 ];

      i           = 2;
      buff[ 0 ]   = 0x1b;
      buff[ 1 ]   = '[';

      acsc        = ( iAttr & HB_GTTRM_ATTR_ACSC ) && ! pTerm->fUTF8 ? 1 : 0;
      bg          = s_AnsiColors[ ( iAttr >> 4 ) & 0x07 ];
      fg          = s_AnsiColors[ iAttr & 0x07 ];
      bold        = iAttr & 0x08 ? 1 : 0;
      blink       = iAttr & 0x80 ? 1 : 0;

      if( pTerm->iCurrentSGR == -1 )
      {
         buff[ i++ ] = 'm';
         buff[ i++ ] = 0x1b;
         buff[ i++ ] = '(';
         buff[ i++ ] = acsc ? '0' : 'B';

         buff[ i++ ] = 0x1b;
         buff[ i++ ] = '[';

         if( bold )
         {
            buff[ i++ ] = '1';
            buff[ i++ ] = ';';
         }
         if( blink )
         {
            buff[ i++ ] = '5';
            buff[ i++ ] = ';';
         }
         buff[ i++ ]       = '3';
         buff[ i++ ]       = '0' + fg;
         buff[ i++ ]       = ';';
         buff[ i++ ]       = '4';
         buff[ i++ ]       = '0' + bg;
         buff[ i++ ]       = 'm';
         pTerm->iACSC      = acsc;
         pTerm->iBold      = bold;
         pTerm->iBlink     = blink;
         pTerm->iFgColor   = fg;
         pTerm->iBgColor   = bg;
      }
      else
      {
         if( pTerm->iBold != bold )
         {
            if( bold )
               buff[ i++ ] = '1';
            else
            {
               buff[ i++ ] = '2';
               buff[ i++ ] = '2';
            }
            buff[ i++ ]    = ';';
            pTerm->iBold   = bold;
         }
         if( pTerm->iBlink != blink )
         {
            if( ! blink )
               buff[ i++ ] = '2';
            buff[ i++ ]    = '5';
            buff[ i++ ]    = ';';
            pTerm->iBlink  = blink;
         }
         if( pTerm->iFgColor != fg )
         {
            buff[ i++ ]       = '3';
            buff[ i++ ]       = '0' + fg;
            buff[ i++ ]       = ';';
            pTerm->iFgColor   = fg;
         }
         if( pTerm->iBgColor != bg )
         {
            buff[ i++ ]       = '4';
            buff[ i++ ]       = '0' + bg;
            buff[ i++ ]       = ';';
            pTerm->iBgColor   = bg;
         }
         buff[ i - 1 ] = 'm';
         if( pTerm->iACSC != acsc )
         {
            if( i <= 2 )
               i = 0;
            buff[ i++ ]    = 0x1b;
            buff[ i++ ]    = '(';
            buff[ i++ ]    = acsc ? '0' : 'B';
            pTerm->iACSC   = acsc;
         }
      }
      pTerm->iCurrentSGR = iAttr;
      if( i > 2 )
      {
         hb_gt_trm_termOut( pTerm, buff, i );
      }
   }
}



/*
 * ANSI terminal operations
 */
static void hb_gt_trm_AnsiSetTermMode( PHB_GTTRM pTerm, int iAM )
{
   HB_TRACE( HB_TR_DEBUG, ( "hb_gt_trm_AnsiSetTermMode(%p,%d)", pTerm, iAM ) );

   if( iAM != pTerm->iAM )
   {
      if( iAM == 0 )
      {
         hb_gt_trm_termOut( pTerm, ( BYTE * ) "\x1B[0m", 4 );
      }
      /*
       * disabled until I'll find good PC-ANSI terminal documentation with
       * detail Auto Margin and Auto Line Wrapping description, [druzus]
       */
#if 0
      hb_gt_trm_termOut( pTerm, ( BYTE * ) ( iAM ? "\x1B[?7h" : "\x1B[?7l" ), 5 );
#endif
      pTerm->iAM = iAM;
   }
}

static BOOL hb_gt_trm_AnsiGetCursorPos( PHB_GTTRM pTerm, int * iRow, int * iCol,
                                        const char * szPost )
{
   HB_TRACE( HB_TR_DEBUG, ( "hb_gt_trm_AnsiGetCursorPos(%p,%p,%p,%p)", pTerm, iRow, iCol, szPost ) );

   if( pTerm->fPosAnswer )
   {
      char  rdbuf[ 64 ];
      int   i, n, y, x;

      hb_gt_trm_termOut( pTerm, ( BYTE * ) "\x1B[6n", 4 );
      if( szPost )
         hb_gt_trm_termOut( pTerm, ( BYTE * ) szPost, strlen( szPost ) );
      hb_gt_trm_termFlush( pTerm );

      *iRow             = *iCol = -1;
      n                 = 0;
      pTerm->fPosAnswer = FALSE;

#ifdef HB_OS_UNIX
      {
         struct timeval tv;
         fd_set         rdfds;

         FD_ZERO( &rdfds );
         FD_SET( pTerm->hFilenoStdin, &rdfds );
         tv.tv_sec   = 2;
         tv.tv_usec  = 0;

         while( select( pTerm->hFilenoStdin + 1, &rdfds, NULL, NULL, &tv ) > 0 )
         {
            i = read( pTerm->hFilenoStdin, rdbuf + n, sizeof( rdbuf ) - 1 - n );
            if( i <= 0 )
               break;
            if( n == 0 )
            {
               while( i > 0 && rdbuf[ 0 ] != '\033' )
               {
                  if( szPost && i >= 5 && hb_strnicmp( rdbuf, "PuTTY", 5 ) == 0 )
                  {
                     pTerm->terminal_ext |= TERM_PUTTY;
                     memmove( rdbuf, rdbuf + 5, i -= 5 );
                  }
                  else
                     memmove( rdbuf, rdbuf + 1, i-- );
               }
            }
            n += i;
            if( n >= 6 )
            {
               rdbuf[ n ] = '\0';
               if( sscanf( rdbuf, "\033[%d;%dR", &y, &x ) == 2 )
               {
                  pTerm->fPosAnswer = TRUE;
                  break;
               }
               else if( n == sizeof( rdbuf ) )
                  break;
            }
         }
      }
#else
      {
         double dTime = hb_dateSeconds(), d;

         do
         {
            i = getc( stdin );
            if( i != EOF && ( n || i == '\033' ) )
            {
               rdbuf[ n++ ] = ( char ) i;
               if( n >= 6 && i == 'R' )
               {
                  rdbuf[ n ] = '\0';
                  if( sscanf( rdbuf, "\033[%d;%dR", &y, &x ) == 2 )
                  {
                     pTerm->fPosAnswer = TRUE;
                     break;
                  }
                  n = 0;
               }
            }
            d = hb_dateSeconds();
         }
         while( d <= dTime + 2.0 && d > dTime );
      }
#endif
      if( pTerm->fPosAnswer )
      {
         *iRow = y - 1;
         *iCol = x - 1;
      }
      else
      {
         *iRow = *iCol = -1;
      }
   }
   return pTerm->fPosAnswer;
}

static void hb_gt_trm_AnsiSetCursorPos( PHB_GTTRM pTerm, int iRow, int iCol )
{
   HB_TRACE( HB_TR_DEBUG, ( "hb_gt_trm_AnsiSetCursorPos(%p,%d,%d)", pTerm, iRow, iCol ) );

   if( pTerm->iRow != iRow || pTerm->iCol != iCol )
   {
      char buff[ 16 ];
      hb_snprintf( buff, sizeof( buff ), "\x1B[%d;%dH", iRow + 1, iCol + 1 );
      hb_gt_trm_termOut( pTerm, ( BYTE * ) buff, strlen( buff ) );
      pTerm->iRow = iRow;
      pTerm->iCol = iCol;
   }
}

static void hb_gt_trm_AnsiSetCursorStyle( PHB_GTTRM pTerm, int iStyle )
{
   HB_TRACE( HB_TR_DEBUG, ( "hb_gt_trm_AnsiSetCursorStyle(%p,%d)", pTerm, iStyle ) );

   if( pTerm->iCursorStyle != iStyle )
   {
      hb_gt_trm_termOut( pTerm, ( BYTE * ) ( iStyle == SC_NONE ?
                                             "\x1B[?25l" : "\x1B[?25h" ), 6 );
      pTerm->iCursorStyle = iStyle;
   }
}

static void hb_gt_trm_AnsiSetAttributes( PHB_GTTRM pTerm, int iAttr )
{
   static const int s_AnsiColors[] = { 0, 4, 2, 6, 1, 5, 3, 7 };

   HB_TRACE( HB_TR_DEBUG, ( "hb_gt_trm_AnsiSetAttributes(%p,%d)", pTerm, iAttr ) );

   if( pTerm->iCurrentSGR != iAttr )
   {
      int   i, acsc, bg, fg, bold, blink;
      BYTE  buff[ 32 ];

      i           = 2;
      buff[ 0 ]   = 0x1b;
      buff[ 1 ]   = '[';

      acsc        = iAttr & HB_GTTRM_ATTR_ACSC ? 1 : 0;
      bg          = s_AnsiColors[ ( iAttr >> 4 ) & 0x07 ];
      fg          = s_AnsiColors[ iAttr & 0x07 ];
      bold        = iAttr & 0x08 ? 1 : 0;
      blink       = iAttr & 0x80 ? 1 : 0;

      if( pTerm->iCurrentSGR == -1 )
      {
         buff[ i++ ] = '0';
         buff[ i++ ] = ';';
         buff[ i++ ] = '1';
         buff[ i++ ] = acsc ? '1' : '0';
         buff[ i++ ] = ';';
         if( bold )
         {
            buff[ i++ ] = '1';
            buff[ i++ ] = ';';
         }
         if( blink )
         {
            buff[ i++ ] = '5';
            buff[ i++ ] = ';';
         }
         buff[ i++ ]       = '3';
         buff[ i++ ]       = '0' + fg;
         buff[ i++ ]       = ';';
         buff[ i++ ]       = '4';
         buff[ i++ ]       = '0' + bg;
         buff[ i++ ]       = 'm';
         pTerm->iACSC      = acsc;
         pTerm->iBold      = bold;
         pTerm->iBlink     = blink;
         pTerm->iFgColor   = fg;
         pTerm->iBgColor   = bg;
      }
      else
      {
         if( pTerm->iACSC != acsc )
         {
            buff[ i++ ]    = '1';
            buff[ i++ ]    = acsc ? '1' : '0';
            buff[ i++ ]    = ';';
            pTerm->iACSC   = acsc;
         }
         if( pTerm->iBold != bold )
         {
            if( bold )
               buff[ i++ ] = '1';
            else
            {
               buff[ i++ ] = '2';
               buff[ i++ ] = '2';
            }
            buff[ i++ ]    = ';';
            pTerm->iBold   = bold;
         }
         if( pTerm->iBlink != blink )
         {
            if( ! blink )
               buff[ i++ ] = '2';
            buff[ i++ ]    = '5';
            buff[ i++ ]    = ';';
            pTerm->iBlink  = blink;
         }
         if( pTerm->iFgColor != fg )
         {
            buff[ i++ ]       = '3';
            buff[ i++ ]       = '0' + fg;
            buff[ i++ ]       = ';';
            pTerm->iFgColor   = fg;
         }
         if( pTerm->iBgColor != bg )
         {
            buff[ i++ ]       = '4';
            buff[ i++ ]       = '0' + bg;
            buff[ i++ ]       = ';';
            pTerm->iBgColor   = bg;
         }
         buff[ i - 1 ] = 'm';
      }
      pTerm->iCurrentSGR = iAttr;
      if( i > 2 )
      {
         hb_gt_trm_termOut( pTerm, buff, i );
      }
   }
}

static int hb_gt_trm_AnsiGetAcsc( PHB_GTTRM pTerm, unsigned char c )
{
   unsigned char * ptr;

   HB_TRACE( HB_TR_DEBUG, ( "hb_gt_trm_AnsiGetAcsc(%p,%d)", pTerm, c ) );

   for( ptr = ( unsigned char * ) pTerm->szAcsc; *ptr && *( ptr + 1 ); ptr += 2 )
   {
      if( *ptr == c )
         return *( ptr + 1 ) | HB_GTTRM_ATTR_ACSC;
   }

   switch( c )
   {
      case '.':
         return 'v' | HB_GTTRM_ATTR_STD;
      case ',':
         return '<' | HB_GTTRM_ATTR_STD;
      case '+':
         return '>' | HB_GTTRM_ATTR_STD;
      case '-':
         return '^' | HB_GTTRM_ATTR_STD;
      case 'a':
         return '#' | HB_GTTRM_ATTR_STD;
      case '0':
      case 'h':
         return hb_gt_trm_AnsiGetAcsc( pTerm, 'a' );
   }

   return c | HB_GTTRM_ATTR_ALT;
}

static BOOL hb_gt_trm_AnsiSetMode( PHB_GTTRM pTerm, int * piRow, int * piCol )
{
   HB_TRACE( HB_TR_DEBUG, ( "hb_gt_trm_AnsiSetMode(%p,%d,%d)", pTerm, *piRow, *piCol ) );

   HB_SYMBOL_UNUSED( pTerm );
   HB_SYMBOL_UNUSED( piRow );
   HB_SYMBOL_UNUSED( piCol );

   return FALSE;
}

static void hb_gt_trm_AnsiBell( PHB_GTTRM pTerm )
{
   HB_TRACE( HB_TR_DEBUG, ( "hb_gt_trm_AnsiBell(%p)", pTerm ) );

   hb_gt_trm_termOut( pTerm, s_szBell, 1 );
   hb_gt_trm_termFlush( pTerm );
}

static void hb_gt_trm_AnsiTone( PHB_GTTRM pTerm, double dFrequency, double dDuration )
{
   double dCurrentSeconds;

   HB_TRACE( HB_TR_DEBUG, ( "hb_gt_trm_AnsiTone(%p,%lf,%lf)", pTerm, dFrequency, dDuration ) );

   /* Output an ASCII BEL character to cause a sound */
   /* but throttle to max once per second, in case of sound */
   /* effects prgs calling lots of short tone sequences in */
   /* succession leading to BEL hell on the terminal */

   dCurrentSeconds = hb_dateSeconds();
   if( dCurrentSeconds < pTerm->dToneSeconds ||
       dCurrentSeconds - pTerm->dToneSeconds > 0.5 )
   {
      hb_gt_trm_AnsiBell( pTerm );
      pTerm->dToneSeconds = dCurrentSeconds;
   }

   HB_SYMBOL_UNUSED( dFrequency );

   /* convert Clipper (DOS) timer tick units to seconds ( x / 18.2 ) */
   hb_idleSleep( dDuration / 18.2 );
}

static void hb_gt_trm_AnsiInit( PHB_GTTRM pTerm )
{
   HB_TRACE( HB_TR_DEBUG, ( "hb_gt_trm_AnsiInit(%p)", pTerm ) );

   pTerm->iCurrentSGR                                                                                 = pTerm->iRow = pTerm->iCol =
                                                                                  pTerm->iCursorStyle = pTerm->iACSC = pTerm->iAM = -1;
}

static void hb_gt_trm_AnsiExit( PHB_GTTRM pTerm )
{
   HB_TRACE( HB_TR_DEBUG, ( "hb_gt_trm_AnsiExit(%p)", pTerm ) );

   /* set default color */
   pTerm->SetAttributes( pTerm, 0x07 & pTerm->iAttrMask );
   pTerm->SetCursorStyle( pTerm, SC_NORMAL );
   pTerm->SetTermMode( pTerm, 1 );
}

/* ************************************************************************* */

/*
 * common functions
 */
static BOOL hb_trm_isUTF8( PHB_GTTRM pTerm )
{
   char * szLang;

   if( pTerm->fPosAnswer )
   {
      int   iRow = 0, iCol = 0;
      BOOL  fSize;

      hb_gt_trm_termOut( pTerm, ( BYTE * ) "\005\r\303\255", 4 );
      fSize       = pTerm->GetCursorPos( pTerm, &iRow, &iCol, "\r   \r" );
      pTerm->iCol = 0;
      if( fSize )
         return iCol == 1;
   }
   szLang = getenv( "LANG" );
   return szLang && strstr( szLang, "UTF-8" ) != NULL;
}

static void hb_gt_trm_PutStr( PHB_GTTRM pTerm, int iRow, int iCol, int iAttr, BYTE * pStr, int iLen )
{
   HB_TRACE( HB_TR_DEBUG, ( "hb_gt_trm_PutStr(%p,%d,%d,%d,%p,%d)", pTerm, iRow, iCol, iAttr, pStr, iLen ) );

   if( pTerm->iOutBufSize )
   {
      pTerm->SetCursorPos( pTerm, iRow, iCol );
      pTerm->SetAttributes( pTerm, iAttr & pTerm->iAttrMask );
      hb_gt_trm_termOutTrans( pTerm, pStr, iLen, iAttr );
   }

   pTerm->iCol += iLen;
}

static void hb_gt_trm_SetKeyTrans( PHB_GTTRM pTerm, char * pSrcChars, char * pDstChars )
{
   int i;

   HB_TRACE( HB_TR_DEBUG, ( "hb_gt_trm_SetKeyTrans(%p,%s,%s)", pTerm, pSrcChars, pDstChars ) );

   for( i = 0; i < 256; ++i )
      pTerm->keyTransTbl[ i ] = ( BYTE ) i;

   if( pSrcChars && pDstChars )
   {
      BYTE c;
      for( i = 0; i < 256 && ( c = ( BYTE ) pSrcChars[ i ] ) != 0; ++i )
         pTerm->keyTransTbl[ c ] = ( BYTE ) pDstChars[ i ];
   }
}

static void hb_gt_trm_SetDispTrans( PHB_GTTRM pTerm, char * src, char * dst, int box )
{
   unsigned char  c, d;
   int            i, ch, mode;

   memset( pTerm->chrattr, 0, sizeof( pTerm->chrattr ) );
   memset( pTerm->boxattr, 0, sizeof( pTerm->boxattr ) );

   for( i = 0; i < 256; i++ )
   {
      ch    = pTerm->charmap[ i ] & 0xffff;
      mode  = ! pTerm->fUTF8 ? ( pTerm->charmap[ i ] >> 16 ) & 0xff : 1;

      switch( mode )
      {
         case 1:
            pTerm->chrattr[ i ]  = pTerm->boxattr[ i ] = HB_GTTRM_ATTR_STD;
            break;
         case 2:
            pTerm->chrattr[ i ]  = pTerm->boxattr[ i ] = HB_GTTRM_ATTR_ALT;
            break;
         case 3:
            pTerm->chrattr[ i ]  = pTerm->boxattr[ i ] = HB_GTTRM_ATTR_PROT;
            break;
         case 4:
            pTerm->chrattr[ i ]  = pTerm->boxattr[ i ] = HB_GTTRM_ATTR_ALT | HB_GTTRM_ATTR_PROT;
            break;
         case 5:
            ch                   = pTerm->GetAcsc( pTerm, ch & 0xff );
            pTerm->chrattr[ i ]  = pTerm->boxattr[ i ] = ch & ~HB_GTTRM_ATTR_CHAR;
            break;
         case 0:
         default:
            pTerm->chrattr[ i ]  = HB_GTTRM_ATTR_STD;
            pTerm->boxattr[ i ]  = HB_GTTRM_ATTR_ALT;
            break;
      }
      pTerm->chrattr[ i ]  |= ch;
      pTerm->boxattr[ i ]  |= ch;
   }

   if( src && dst )
   {
      for( i = 0; i < 256 && ( c = ( unsigned char ) src[ i ] ) != 0; i++ )
      {
         d                    = ( unsigned char ) dst[ i ];
         pTerm->chrattr[ c ]  = d | HB_GTTRM_ATTR_STD;
         if( box )
            pTerm->boxattr[ c ] = d | HB_GTTRM_ATTR_STD;
      }
   }
}

static int addKeyMap( PHB_GTTRM pTerm, int nKey, const char * cdesc )
{
   int         ret = K_UNDEF, i = 0, c;
   keyTab **   ptr;

   if( cdesc == NULL )
      return ret;

   c     = ( unsigned char ) cdesc[ i++ ];
   ptr   = &pTerm->pKeyTab;

   while( c )
   {
      if( *ptr == NULL )
      {
         *ptr              = ( keyTab * ) hb_xgrab( sizeof( keyTab ) );
         ( *ptr )->ch      = c;
         ( *ptr )->key     = K_UNDEF;
         ( *ptr )->nextCh  = NULL;
         ( *ptr )->otherCh = NULL;
      }
      if( ( *ptr )->ch == c )
      {
         c = ( unsigned char ) cdesc[ i++ ];
         if( c )
            ptr = &( ( *ptr )->nextCh );
         else
         {
            ret            = ( *ptr )->key;
            ( *ptr )->key  = nKey;
         }
      }
      else
         ptr = &( ( *ptr )->otherCh );
   }
   return ret;
}

static int removeKeyMap( PHB_GTTRM pTerm, const char * cdesc )
{
   int         ret = K_UNDEF, i = 0, c;
   keyTab **   ptr;

   c     = ( unsigned char ) cdesc[ i++ ];
   ptr   = &pTerm->pKeyTab;

   while( c && *ptr != NULL )
   {
      if( ( *ptr )->ch == c )
      {
         c = ( unsigned char ) cdesc[ i++ ];
         if( ! c )
         {
            ret            = ( *ptr )->key;
            ( *ptr )->key  = K_UNDEF;
            if( ( *ptr )->nextCh == NULL && ( *ptr )->otherCh == NULL )
            {
               hb_xfree( *ptr );
               *ptr = NULL;
            }
         }
         else
            ptr = &( ( *ptr )->nextCh );
      }
      else
         ptr = &( ( *ptr )->otherCh );
   }
   return ret;
}

static void removeAllKeyMap( PHB_GTTRM pTerm, keyTab ** ptr )
{
   if( ( *ptr )->nextCh != NULL )
      removeAllKeyMap( pTerm, &( ( *ptr )->nextCh ) );
   if( ( *ptr )->otherCh != NULL )
      removeAllKeyMap( pTerm, &( ( *ptr )->otherCh ) );

   hb_xfree( *ptr );
   *ptr = NULL;
}

static void addKeyTab( PHB_GTTRM pTerm, const keySeq * keys )
{
   while( keys->key )
   {
      addKeyMap( pTerm, keys->key, keys->seq );
      ++keys;
   }
}

static void init_keys( PHB_GTTRM pTerm )
{

   static const keySeq  stdKeySeq[] = {
      /* virual CTRL/ALT sequences */
      { K_METACTRL,  CTRL_SEQ   },
      { K_METAALT,   ALT_SEQ    },
#ifdef NATION_SEQ
      /* national mode key sequences */
      { K_NATIONAL,  NATION_SEQ },
#endif
      { EXKEY_ENTER, "\r"       },
      /* terminal mouse event */
      { K_MOUSETERM, "\033[M"   },
      { 0,           NULL       }
   };

   static const keySeq  stdFnKeySeq[] = {

      { EXKEY_F1,                               "\033[11~" },  /* kf1  */
      { EXKEY_F2,                               "\033[12~" },  /* kf2  */
      { EXKEY_F3,                               "\033[13~" },  /* kf3  */
      { EXKEY_F4,                               "\033[14~" },  /* kf4  */
      { EXKEY_F5,                               "\033[15~" },  /* kf5  */

      { EXKEY_F6,                               "\033[17~" },  /* kf6  */
      { EXKEY_F7,                               "\033[18~" },  /* kf7  */
      { EXKEY_F8,                               "\033[19~" },  /* kf8  */
      { EXKEY_F9,                               "\033[20~" },  /* kf9  */
      { EXKEY_F10,                              "\033[21~" },  /* kf10 */
      { EXKEY_F11,                              "\033[23~" },  /* kf11 */
      { EXKEY_F12,                              "\033[24~" },  /* kf12 */

      { EXKEY_F1 | KEY_CTRLMASK | KEY_ALTMASK,  "\033[25~" },  /* kf13 */
      { EXKEY_F2 | KEY_CTRLMASK | KEY_ALTMASK,  "\033[26~" },  /* kf14 */
      { EXKEY_F3 | KEY_CTRLMASK | KEY_ALTMASK,  "\033[28~" },  /* kf15 */
      { EXKEY_F4 | KEY_CTRLMASK | KEY_ALTMASK,  "\033[29~" },  /* kf16 */
      { EXKEY_F5 | KEY_CTRLMASK | KEY_ALTMASK,  "\033[31~" },  /* kf17 */
      { EXKEY_F6 | KEY_CTRLMASK | KEY_ALTMASK,  "\033[32~" },  /* kf18 */
      { EXKEY_F7 | KEY_CTRLMASK | KEY_ALTMASK,  "\033[33~" },  /* kf19 */
      { EXKEY_F8 | KEY_CTRLMASK | KEY_ALTMASK,  "\033[34~" },  /* kf20 */
      { EXKEY_F9 | KEY_CTRLMASK | KEY_ALTMASK,  "\033[35~" },  /* kf21 */
      { EXKEY_F10 | KEY_CTRLMASK | KEY_ALTMASK, "\033[36~" },  /* kf22 */
      { EXKEY_F11 | KEY_CTRLMASK | KEY_ALTMASK, "\033[37~" },  /* kf23 */
      { EXKEY_F12 | KEY_CTRLMASK | KEY_ALTMASK, "\033[38~" },  /* kf24 */

      { EXKEY_F1 | KEY_CTRLMASK,                "\033[39~" },  /* kf25 */
      { EXKEY_F2 | KEY_CTRLMASK,                "\033[40~" },  /* kf26 */
      { EXKEY_F3 | KEY_CTRLMASK,                "\033[41~" },  /* kf27 */
      { EXKEY_F4 | KEY_CTRLMASK,                "\033[42~" },  /* kf28 */
      { EXKEY_F5 | KEY_CTRLMASK,                "\033[43~" },  /* kf29 */
      { EXKEY_F6 | KEY_CTRLMASK,                "\033[44~" },  /* kf30 */
      { EXKEY_F7 | KEY_CTRLMASK,                "\033[45~" },  /* kf31 */
      { EXKEY_F8 | KEY_CTRLMASK,                "\033[46~" },  /* kf32 */
      { EXKEY_F9 | KEY_CTRLMASK,                "\033[47~" },  /* kf33 */
      { EXKEY_F10 | KEY_CTRLMASK,               "\033[48~" },  /* kf34 */
      { EXKEY_F11 | KEY_CTRLMASK,               "\033[49~" },  /* kf35 */
      { EXKEY_F12 | KEY_CTRLMASK,               "\033[50~" },  /* kf36 */

      { EXKEY_F1 | KEY_ALTMASK,                 "\033[51~" },  /* kf37 */
      { EXKEY_F2 | KEY_ALTMASK,                 "\033[52~" },  /* kf38 */
      { EXKEY_F3 | KEY_ALTMASK,                 "\033[53~" },  /* kf39 */
      { EXKEY_F4 | KEY_ALTMASK,                 "\033[54~" },  /* kf40 */
      { EXKEY_F5 | KEY_ALTMASK,                 "\033[55~" },  /* kf41 */
      { EXKEY_F6 | KEY_ALTMASK,                 "\033[56~" },  /* kf42 */
      { EXKEY_F7 | KEY_ALTMASK,                 "\033[57~" },  /* kf43 */
      { EXKEY_F8 | KEY_ALTMASK,                 "\033[58~" },  /* kf44 */
      { EXKEY_F9 | KEY_ALTMASK,                 "\033[59~" },  /* kf45 */
      { EXKEY_F10 | KEY_ALTMASK,                "\033[70~" },  /* kf46 */
      { EXKEY_F11 | KEY_ALTMASK,                "\033[71~" },  /* kf47 */
      { EXKEY_F12 | KEY_ALTMASK,                "\033[72~" },  /* kf48 */

      { 0,                                      NULL       }
   };

   static const keySeq  stdCursorKeySeq[] = {
      { EXKEY_HOME, "\033[1~" },    /* khome */
      { EXKEY_INS,  "\033[2~" },    /* kich1 */
      { EXKEY_DEL,  "\033[3~" },    /* kdch1 */
      { EXKEY_END,  "\033[4~" },    /* kend  */
      { EXKEY_PGUP, "\033[5~" },    /* kpp   */
      { EXKEY_PGDN, "\033[6~" },    /* knp   */

      { 0,          NULL      }
   };

   static const keySeq  puttyKeySeq[] = {
      /* In XTerm (XFree 3.x.x) they are without CTRL,
         kcuu1, kcud1, kcuf1, kcub1 */
      { EXKEY_UP | KEY_CTRLMASK,     "\033OA" },
      { EXKEY_DOWN | KEY_CTRLMASK,   "\033OB" },
      { EXKEY_RIGHT | KEY_CTRLMASK,  "\033OC" },
      { EXKEY_LEFT | KEY_CTRLMASK,   "\033OD" },

      { EXKEY_CENTER | KEY_CTRLMASK, "\033OG" },

      { 0,                           NULL     }
   };

   static const keySeq  rxvtKeySeq[] = {

      { EXKEY_HOME, "\033[H" },
      { EXKEY_END,  "\033Ow" },

      { 0,          NULL     }
   };

   static const keySeq  xtermModKeySeq[] = {

      { EXKEY_F1 | KEY_CTRLMASK,                 "\033O5P"    },
      { EXKEY_F2 | KEY_CTRLMASK,                 "\033O5Q"    },
      { EXKEY_F3 | KEY_CTRLMASK,                 "\033O5R"    },
      { EXKEY_F4 | KEY_CTRLMASK,                 "\033O5S"    },

      { EXKEY_F1 | KEY_CTRLMASK,                 "\033[11;5~" },
      { EXKEY_F2 | KEY_CTRLMASK,                 "\033[12;5~" },
      { EXKEY_F3 | KEY_CTRLMASK,                 "\033[13;5~" },
      { EXKEY_F4 | KEY_CTRLMASK,                 "\033[14;5~" },
      { EXKEY_F5 | KEY_CTRLMASK,                 "\033[15;5~" },
      { EXKEY_F6 | KEY_CTRLMASK,                 "\033[17;5~" },
      { EXKEY_F7 | KEY_CTRLMASK,                 "\033[18;5~" },
      { EXKEY_F8 | KEY_CTRLMASK,                 "\033[19;5~" },
      { EXKEY_F9 | KEY_CTRLMASK,                 "\033[20;5~" },
      { EXKEY_F10 | KEY_CTRLMASK,                "\033[21;5~" },
      { EXKEY_F11 | KEY_CTRLMASK,                "\033[23;5~" },
      { EXKEY_F12 | KEY_CTRLMASK,                "\033[24;5~" },

      { EXKEY_HOME | KEY_CTRLMASK,               "\033[1;5~"  },
      { EXKEY_INS | KEY_CTRLMASK,                "\033[2;5~"  },
      { EXKEY_DEL | KEY_CTRLMASK,                "\033[3;5~"  },
      { EXKEY_END | KEY_CTRLMASK,                "\033[4;5~"  },
      { EXKEY_PGUP | KEY_CTRLMASK,               "\033[5;5~"  },
      { EXKEY_PGDN | KEY_CTRLMASK,               "\033[6;5~"  },

      { EXKEY_F1 | KEY_ALTMASK,                  "\033O3P"    },
      { EXKEY_F2 | KEY_ALTMASK,                  "\033O3Q"    },
      { EXKEY_F3 | KEY_ALTMASK,                  "\033O3R"    },
      { EXKEY_F4 | KEY_ALTMASK,                  "\033O3S"    },

      { EXKEY_F1 | KEY_ALTMASK,                  "\033[11;3~" },
      { EXKEY_F2 | KEY_ALTMASK,                  "\033[12;3~" },
      { EXKEY_F3 | KEY_ALTMASK,                  "\033[13;3~" },
      { EXKEY_F4 | KEY_ALTMASK,                  "\033[14;3~" },
      { EXKEY_F5 | KEY_ALTMASK,                  "\033[15;3~" },
      { EXKEY_F6 | KEY_ALTMASK,                  "\033[17;3~" },
      { EXKEY_F7 | KEY_ALTMASK,                  "\033[18;3~" },
      { EXKEY_F8 | KEY_ALTMASK,                  "\033[19;3~" },
      { EXKEY_F9 | KEY_ALTMASK,                  "\033[20;3~" },
      { EXKEY_F10 | KEY_ALTMASK,                 "\033[21;3~" },
      { EXKEY_F11 | KEY_ALTMASK,                 "\033[23;3~" },
      { EXKEY_F12 | KEY_ALTMASK,                 "\033[24;3~" },

      { EXKEY_HOME | KEY_ALTMASK,                "\033[1;3~"  },
      { EXKEY_INS | KEY_ALTMASK,                 "\033[2;3~"  },
      { EXKEY_DEL | KEY_ALTMASK,                 "\033[3;3~"  },
      { EXKEY_END | KEY_ALTMASK,                 "\033[4;3~"  },
      { EXKEY_PGUP | KEY_ALTMASK,                "\033[5;3~"  },
      { EXKEY_PGDN | KEY_ALTMASK,                "\033[6;3~"  },

      { EXKEY_F1 | KEY_CTRLMASK | KEY_ALTMASK,   "\033O2P"    },
      { EXKEY_F2 | KEY_CTRLMASK | KEY_ALTMASK,   "\033O2Q"    },
      { EXKEY_F3 | KEY_CTRLMASK | KEY_ALTMASK,   "\033O2R"    },
      { EXKEY_F4 | KEY_CTRLMASK | KEY_ALTMASK,   "\033O2S"    },

      { EXKEY_F1 | KEY_CTRLMASK | KEY_ALTMASK,   "\033[11;2~" },
      { EXKEY_F2 | KEY_CTRLMASK | KEY_ALTMASK,   "\033[12;2~" },
      { EXKEY_F3 | KEY_CTRLMASK | KEY_ALTMASK,   "\033[13;2~" },
      { EXKEY_F4 | KEY_CTRLMASK | KEY_ALTMASK,   "\033[14;2~" },
      { EXKEY_F5 | KEY_CTRLMASK | KEY_ALTMASK,   "\033[15;2~" },
      { EXKEY_F6 | KEY_CTRLMASK | KEY_ALTMASK,   "\033[17;2~" },
      { EXKEY_F7 | KEY_CTRLMASK | KEY_ALTMASK,   "\033[18;2~" },
      { EXKEY_F8 | KEY_CTRLMASK | KEY_ALTMASK,   "\033[19;2~" },
      { EXKEY_F9 | KEY_CTRLMASK | KEY_ALTMASK,   "\033[20;2~" },
      { EXKEY_F10 | KEY_CTRLMASK | KEY_ALTMASK,  "\033[21;2~" },
      { EXKEY_F11 | KEY_CTRLMASK | KEY_ALTMASK,  "\033[23;2~" },
      { EXKEY_F12 | KEY_CTRLMASK | KEY_ALTMASK,  "\033[24;2~" },

      { EXKEY_HOME | KEY_CTRLMASK | KEY_ALTMASK, "\033[1;2~"  },
      { EXKEY_INS | KEY_CTRLMASK | KEY_ALTMASK,  "\033[2;2~"  },
      { EXKEY_DEL | KEY_CTRLMASK | KEY_ALTMASK,  "\033[3;2~"  },
      { EXKEY_END | KEY_CTRLMASK | KEY_ALTMASK,  "\033[4;2~"  },
      { EXKEY_PGUP | KEY_CTRLMASK | KEY_ALTMASK, "\033[5;2~"  },
      { EXKEY_PGDN | KEY_CTRLMASK | KEY_ALTMASK, "\033[6;2~"  },

      { EXKEY_BS | KEY_ALTMASK,                  "\033\010"   },

      { 0,                                       NULL         }
   };

   static const keySeq  xtermFnKeySeq[] = {

      { EXKEY_F1, "\033OP" }, /* kf1  */
      { EXKEY_F2, "\033OQ" }, /* kf2  */
      { EXKEY_F3, "\033OR" }, /* kf3  */
      { EXKEY_F4, "\033OS" }, /* kf4  */

      { 0,        NULL     }
   };

   static const keySeq  xtermKeySeq[] = {

      { EXKEY_BS,                                  "\010"      }, /* kbs   */
      { EXKEY_TAB,                                 "\011"      }, /* ht    */
      { EXKEY_BS,                                  "\177"      },

      /* cursor keys */
      { EXKEY_UP,                                  "\033[A"    },
      { EXKEY_DOWN,                                "\033[B"    },
      { EXKEY_RIGHT,                               "\033[C"    },
      { EXKEY_LEFT,                                "\033[D"    },

      { EXKEY_CENTER,                              "\033[E"    }, /* XTerm */
      { EXKEY_END,                                 "\033[F"    }, /* XTerm */
      { EXKEY_CENTER,                              "\033[G"    }, /* PuTTY */
      { EXKEY_HOME,                                "\033[H"    }, /* XTerm */

      { EXKEY_TAB | KEY_CTRLMASK | KEY_ALTMASK,    "\033[Z"    }, /* kcbt, XTerm */

      /* XTerm  with modifiers */
      { EXKEY_UP | KEY_CTRLMASK,                   "\033[1;5A" },
      { EXKEY_DOWN | KEY_CTRLMASK,                 "\033[1;5B" },
      { EXKEY_RIGHT | KEY_CTRLMASK,                "\033[1;5C" },
      { EXKEY_LEFT | KEY_CTRLMASK,                 "\033[1;5D" },
      { EXKEY_CENTER | KEY_CTRLMASK,               "\033[1;5E" },
      { EXKEY_END | KEY_CTRLMASK,                  "\033[1;5F" },
      { EXKEY_HOME | KEY_CTRLMASK,                 "\033[1;5H" },

      { EXKEY_UP | KEY_ALTMASK,                    "\033[1;3A" },
      { EXKEY_DOWN | KEY_ALTMASK,                  "\033[1;3B" },
      { EXKEY_RIGHT | KEY_ALTMASK,                 "\033[1;3C" },
      { EXKEY_LEFT | KEY_ALTMASK,                  "\033[1;3D" },
      { EXKEY_CENTER | KEY_ALTMASK,                "\033[1;3E" },
      { EXKEY_END | KEY_ALTMASK,                   "\033[1;3F" },
      { EXKEY_HOME | KEY_ALTMASK,                  "\033[1;3H" },

      { EXKEY_UP | KEY_CTRLMASK | KEY_ALTMASK,     "\033[1;2A" },
      { EXKEY_DOWN | KEY_CTRLMASK | KEY_ALTMASK,   "\033[1;2B" },
      { EXKEY_RIGHT | KEY_CTRLMASK | KEY_ALTMASK,  "\033[1;2C" },
      { EXKEY_LEFT | KEY_CTRLMASK | KEY_ALTMASK,   "\033[1;2D" },
      { EXKEY_CENTER | KEY_CTRLMASK | KEY_ALTMASK, "\033[1;2E" },
      { EXKEY_END | KEY_CTRLMASK | KEY_ALTMASK,    "\033[1;2F" },
      { EXKEY_HOME | KEY_CTRLMASK | KEY_ALTMASK,   "\033[1;2H" },

      /* Konsole */
      { EXKEY_ENTER | KEY_CTRLMASK | KEY_ALTMASK,  "\033OM"    },

      { EXKEY_END,                                 "\033Ow"    }, /* rxvt */

      /* gnome-terminal */
      { EXKEY_END,                                 "\033OF"    }, /* kend  */
      { EXKEY_HOME,                                "\033OH"    }, /* khome */
      { EXKEY_ENTER | KEY_ALTMASK,                 "\033\012"  },

      { EXKEY_UP | KEY_CTRLMASK,                   "\033[5A"   },
      { EXKEY_DOWN | KEY_CTRLMASK,                 "\033[5B"   },
      { EXKEY_RIGHT | KEY_CTRLMASK,                "\033[5C"   },
      { EXKEY_LEFT | KEY_CTRLMASK,                 "\033[5D"   },
      { EXKEY_CENTER | KEY_CTRLMASK,               "\033[5E"   }, /* --- */
      { EXKEY_END | KEY_CTRLMASK,                  "\033[5F"   }, /* --- */
      { EXKEY_HOME | KEY_CTRLMASK,                 "\033[5H"   }, /* --- */

      { EXKEY_UP | KEY_ALTMASK,                    "\033[3A"   },
      { EXKEY_DOWN | KEY_ALTMASK,                  "\033[3B"   },
      { EXKEY_RIGHT | KEY_ALTMASK,                 "\033[3C"   },
      { EXKEY_LEFT | KEY_ALTMASK,                  "\033[3D"   },
      { EXKEY_CENTER | KEY_ALTMASK,                "\033[3E"   }, /* --- */
      { EXKEY_END | KEY_ALTMASK,                   "\033[3F"   }, /* --- */
      { EXKEY_HOME | KEY_ALTMASK,                  "\033[3H"   }, /* --- */

      { EXKEY_UP | KEY_CTRLMASK | KEY_ALTMASK,     "\033[2A"   },
      { EXKEY_DOWN | KEY_CTRLMASK | KEY_ALTMASK,   "\033[2B"   },
      { EXKEY_RIGHT | KEY_CTRLMASK | KEY_ALTMASK,  "\033[2C"   },
      { EXKEY_LEFT | KEY_CTRLMASK | KEY_ALTMASK,   "\033[2D"   },
      { EXKEY_CENTER | KEY_CTRLMASK | KEY_ALTMASK, "\033[2E"   }, /* --- */
      { EXKEY_END | KEY_CTRLMASK | KEY_ALTMASK,    "\033[2F"   }, /* --- */
      { EXKEY_HOME | KEY_CTRLMASK | KEY_ALTMASK,   "\033[2H"   }, /* --- */

#if 0
      /* key added for gnome-terminal and teraterm */
      { EXKEY_ENTER | KEY_CTRLMASK,                "\033[7;5~" },
      { EXKEY_TAB | KEY_CTRLMASK,                  "\033[8;5~" },

      { EXKEY_UP | KEY_CTRLMASK | KEY_ALTMASK,     "\033[6A"   },
      { EXKEY_DOWN | KEY_CTRLMASK | KEY_ALTMASK,   "\033[6B"   },
      { EXKEY_RIGHT | KEY_CTRLMASK | KEY_ALTMASK,  "\033[6C"   },
      { EXKEY_LEFT | KEY_CTRLMASK | KEY_ALTMASK,   "\033[6D"   },
      { EXKEY_CENTER | KEY_CTRLMASK | KEY_ALTMASK, "\033[6E"   },
      { EXKEY_END | KEY_CTRLMASK | KEY_ALTMASK,    "\033[6F"   },
      { EXKEY_HOME | KEY_CTRLMASK | KEY_ALTMASK,   "\033[6H"   },

      { EXKEY_INS | KEY_CTRLMASK | KEY_ALTMASK,    "\033[2;6~" },
      { EXKEY_DEL | KEY_CTRLMASK | KEY_ALTMASK,    "\033[3;6~" },
      { EXKEY_PGUP | KEY_CTRLMASK | KEY_ALTMASK,   "\033[5;6~" },
      { EXKEY_PGDN | KEY_CTRLMASK | KEY_ALTMASK,   "\033[6;6~" },
      { EXKEY_ENTER | KEY_CTRLMASK | KEY_ALTMASK,  "\033[7;6~" },

      { EXKEY_BS | KEY_CTRLMASK | KEY_ALTMASK,     "\033[W"    },
#endif

      { 0,                                         NULL        }
   };

   static const keySeq  linuxKeySeq[] = {

      { EXKEY_TAB,               "\011"    },   /* ht    */
      { EXKEY_BS,                "\177"    },   /* kbs   */

      { EXKEY_UP,                "\033[A"  },   /* kcuu1 */
      { EXKEY_DOWN,              "\033[B"  },   /* kcud1 */
      { EXKEY_RIGHT,             "\033[C"  },   /* kcuf1 */
      { EXKEY_LEFT,              "\033[D"  },   /* kcub1 */
      { EXKEY_CENTER,            "\033[G"  },   /* kb2 */

      { EXKEY_F1,                "\033[[A" },   /* kf1  */
      { EXKEY_F2,                "\033[[B" },   /* kf2  */
      { EXKEY_F3,                "\033[[C" },   /* kf3  */
      { EXKEY_F4,                "\033[[D" },   /* kf4  */
      { EXKEY_F5,                "\033[[E" },   /* kf5  */

      { EXKEY_TAB | KEY_ALTMASK, "\033[Z"  },   /* kcbt */

      { 0,                       NULL      }
   };

   static const keySeq  ansiKeySeq[] = {

      { EXKEY_BS,                               "\010"    },   /* kbs   */
      { EXKEY_TAB,                              "\011"    },   /* ht    */
      { EXKEY_DEL,                              "\177"    },   /* kdch1 */
      /* cursor keys */
      { EXKEY_UP,                               "\033[A"  },   /* kcuu1 */
      { EXKEY_DOWN,                             "\033[B"  },   /* kcud1 */
      { EXKEY_RIGHT,                            "\033[C"  },   /* kcuf1 */
      { EXKEY_LEFT,                             "\033[D"  },   /* kcub1 */
      { EXKEY_CENTER,                           "\033[E"  },   /* kb2   */
      { EXKEY_END,                              "\033[F"  },   /* kend  */
      { EXKEY_PGDN,                             "\033[G"  },   /* knp   */
      { EXKEY_HOME,                             "\033[H"  },   /* khome */
      { EXKEY_PGUP,                             "\033[I"  },   /* kpp   */
      { EXKEY_INS,                              "\033[L"  },   /* kich1 */

      { EXKEY_F1,                               "\033[M"  },   /* kf1  */
      { EXKEY_F2,                               "\033[N"  },   /* kf2  */
      { EXKEY_F3,                               "\033[O"  },   /* kf3  */
      { EXKEY_F4,                               "\033[P"  },   /* kf4  */
      { EXKEY_F5,                               "\033[Q"  },   /* kf5  */
      { EXKEY_F6,                               "\033[R"  },   /* kf6  */
      { EXKEY_F7,                               "\033[S"  },   /* kf7  */
      { EXKEY_F8,                               "\033[T"  },   /* kf8  */
      { EXKEY_F9,                               "\033[U"  },   /* kf9  */
      { EXKEY_F10,                              "\033[V"  },   /* kf10 */
      { EXKEY_F11,                              "\033[W"  },   /* kf11 */
      { EXKEY_F12,                              "\033[X"  },   /* kf12 */

      { EXKEY_F1 | KEY_CTRLMASK | KEY_ALTMASK,  "\033[Y"  },   /* kf13 */
      { EXKEY_F2 | KEY_CTRLMASK | KEY_ALTMASK,  "\033[Z"  },   /* kf14 */
      { EXKEY_F3 | KEY_CTRLMASK | KEY_ALTMASK,  "\033[a"  },   /* kf15 */
      { EXKEY_F4 | KEY_CTRLMASK | KEY_ALTMASK,  "\033[b"  },   /* kf16 */
      { EXKEY_F5 | KEY_CTRLMASK | KEY_ALTMASK,  "\033[c"  },   /* kf17 */
      { EXKEY_F6 | KEY_CTRLMASK | KEY_ALTMASK,  "\033[d"  },   /* kf18 */
      { EXKEY_F7 | KEY_CTRLMASK | KEY_ALTMASK,  "\033[e"  },   /* kf19 */
      { EXKEY_F8 | KEY_CTRLMASK | KEY_ALTMASK,  "\033[f"  },   /* kf20 */
      { EXKEY_F9 | KEY_CTRLMASK | KEY_ALTMASK,  "\033[g"  },   /* kf21 */
      { EXKEY_F10 | KEY_CTRLMASK | KEY_ALTMASK, "\033[h"  },   /* kf22 */
      { EXKEY_F11 | KEY_CTRLMASK | KEY_ALTMASK, "\033[j"  },   /* kf23 */
      { EXKEY_F12 | KEY_CTRLMASK | KEY_ALTMASK, "\033[j"  },   /* kf24 */

      { EXKEY_F1 | KEY_CTRLMASK,                "\033[k"  },   /* kf25 */
      { EXKEY_F2 | KEY_CTRLMASK,                "\033[l"  },   /* kf26 */
      { EXKEY_F3 | KEY_CTRLMASK,                "\033[m"  },   /* kf27 */
      { EXKEY_F4 | KEY_CTRLMASK,                "\033[n"  },   /* kf28 */
      { EXKEY_F5 | KEY_CTRLMASK,                "\033[o"  },   /* kf29 */
      { EXKEY_F6 | KEY_CTRLMASK,                "\033[p"  },   /* kf30 */
      { EXKEY_F7 | KEY_CTRLMASK,                "\033[q"  },   /* kf31 */
      { EXKEY_F8 | KEY_CTRLMASK,                "\033[r"  },   /* kf32 */
      { EXKEY_F9 | KEY_CTRLMASK,                "\033[s"  },   /* kf33 */
      { EXKEY_F10 | KEY_CTRLMASK,               "\033[t"  },   /* kf34 */
      { EXKEY_F11 | KEY_CTRLMASK,               "\033[u"  },   /* kf35 */
      { EXKEY_F12 | KEY_CTRLMASK,               "\033[v"  },   /* kf36 */

      { EXKEY_F1 | KEY_ALTMASK,                 "\033[w"  },   /* kf37 */
      { EXKEY_F2 | KEY_ALTMASK,                 "\033[x"  },   /* kf38 */
      { EXKEY_F3 | KEY_ALTMASK,                 "\033[y"  },   /* kf39 */
      { EXKEY_F4 | KEY_ALTMASK,                 "\033[z"  },   /* kf40 */
      { EXKEY_F5 | KEY_ALTMASK,                 "\033[@"  },   /* kf41 */
      { EXKEY_F6 | KEY_ALTMASK,                 "\033[["  },   /* kf42 */
      { EXKEY_F7 | KEY_ALTMASK,                 "\033[\\" },   /* kf43 */
      { EXKEY_F8 | KEY_ALTMASK,                 "\033[]"  },   /* kf44 */
      { EXKEY_F9 | KEY_ALTMASK,                 "\033[^"  },   /* kf45 */
      { EXKEY_F10 | KEY_ALTMASK,                "\033[_"  },   /* kf46 */
      { EXKEY_F11 | KEY_ALTMASK,                "\033[`"  },   /* kf47 */
      { EXKEY_F12 | KEY_ALTMASK,                "\033[{"  },   /* kf48 */

      { 0,                                      NULL      }
   };


   addKeyTab( pTerm, stdKeySeq );
   if( pTerm->terminal_type == TERM_XTERM )
   {
      addKeyTab( pTerm, xtermKeySeq );
      addKeyTab( pTerm, xtermFnKeySeq );
      addKeyTab( pTerm, stdFnKeySeq );
      addKeyTab( pTerm, stdCursorKeySeq );
      addKeyTab( pTerm, xtermModKeySeq );
      addKeyTab( pTerm, puttyKeySeq );
   }
   else if( pTerm->terminal_type == TERM_LINUX )
   {
      addKeyTab( pTerm, linuxKeySeq );
      addKeyTab( pTerm, stdFnKeySeq );
      addKeyTab( pTerm, stdCursorKeySeq );
      addKeyTab( pTerm, xtermFnKeySeq );
      addKeyTab( pTerm, xtermModKeySeq );
      addKeyTab( pTerm, puttyKeySeq );
      /* if( pTerm->terminal_ext & TERM_PUTTY ) for PuTTY */
      addKeyTab( pTerm, rxvtKeySeq );
   }
   else if( pTerm->terminal_type == TERM_ANSI )
   {
      addKeyTab( pTerm, ansiKeySeq );
   }

#if 0
   static const keySeq oldCTerm3XKeySeq[] = {
      { EXKEY_UP,                    "\033OA" },   /* kcuu1 */
      { EXKEY_DOWN,                  "\033OB" },   /* kcud1 */
      { EXKEY_RIGHT,                 "\033OC" },   /* kcuf1 */
      { EXKEY_LEFT,                  "\033OD" },   /* kcub1 */

      { EXKEY_CENTER,                "\033OE" },   /* kb2   */
      { EXKEY_ENTER,                 "\033OM" },   /* kent  */

      { EXKEY_CENTER | KEY_CTRLMASK, "\033OE" },
      { EXKEY_END | KEY_CTRLMASK,    "\033OF" },
      { EXKEY_HOME | KEY_CTRLMASK,   "\033OH" },
      { 0,                           NULL     }
   };
#endif
#if 0
   /* (curses) termcap/terminfo sequences */
   /* FlagShip extension */
   addKeyMap( pTerm, EXKEY_HOME | KEY_CTRLMASK, tiGetS( "ked"   ) );
   addKeyMap( pTerm, EXKEY_END | KEY_CTRLMASK, tiGetS( "kel"   ) );
   addKeyMap( pTerm, EXKEY_PGUP | KEY_CTRLMASK, tiGetS( "kri"   ) );
   addKeyMap( pTerm, EXKEY_PGDN | KEY_CTRLMASK, tiGetS( "kind"  ) );
   addKeyMap( pTerm, EXKEY_RIGHT | KEY_CTRLMASK, tiGetS( "kctab" ) );
   addKeyMap( pTerm, EXKEY_LEFT | KEY_CTRLMASK, tiGetS( "khts"  ) );

   /* some xterms extension */
   addKeyMap( pTerm, EXKEY_HOME, tiGetS( "kfnd"  ) );
   addKeyMap( pTerm, EXKEY_END, tiGetS( "kslt"  ) );

   /* keypad */
   addKeyMap( pTerm, EXKEY_CENTER, tiGetS( "kb2"   ) );
   addKeyMap( pTerm, EXKEY_HOME, tiGetS( "ka1"   ) );
   addKeyMap( pTerm, EXKEY_END, tiGetS( "kc1"   ) );
   addKeyMap( pTerm, EXKEY_PGUP, tiGetS( "ka3"   ) );
   addKeyMap( pTerm, EXKEY_PGDN, tiGetS( "kc3"   ) );

   /* other keys */
   addKeyMap( pTerm, EXKEY_ENTER, tiGetS( "kent"  ) );
   addKeyMap( pTerm, EXKEY_END, tiGetS( "kend"  ) );
   addKeyMap( pTerm, EXKEY_PGUP, tiGetS( "kpp"   ) );
   addKeyMap( pTerm, EXKEY_PGDN, tiGetS( "knp"   ) );
   addKeyMap( pTerm, EXKEY_UP, tiGetS( "kcuu1" ) );
   addKeyMap( pTerm, EXKEY_DOWN, tiGetS( "kcud1" ) );
   addKeyMap( pTerm, EXKEY_RIGHT, tiGetS( "kcuf1" ) );
   addKeyMap( pTerm, EXKEY_LEFT, tiGetS( "kcub1" ) );
   addKeyMap( pTerm, EXKEY_HOME, tiGetS( "khome" ) );
   addKeyMap( pTerm, EXKEY_INS, tiGetS( "kich1" ) );
   addKeyMap( pTerm, EXKEY_DEL, tiGetS( "kdch1" ) );
   addKeyMap( pTerm, EXKEY_TAB, tiGetS( "ht"    ) );
   addKeyMap( pTerm, EXKEY_BS, tiGetS( "kbs"   ) );

   addKeyMap( pTerm, EXKEY_TAB | KEY_ALTMASK, tiGetS( "kcbt" ) );

   /* function keys */
   addKeyMap( pTerm, EXKEY_F1, tiGetS( "kf1"   ) );
   addKeyMap( pTerm, EXKEY_F2, tiGetS( "kf2"   ) );
   addKeyMap( pTerm, EXKEY_F3, tiGetS( "kf3"   ) );
   addKeyMap( pTerm, EXKEY_F4, tiGetS( "kf4"   ) );
   addKeyMap( pTerm, EXKEY_F5, tiGetS( "kf5"   ) );
   addKeyMap( pTerm, EXKEY_F6, tiGetS( "kf6"   ) );
   addKeyMap( pTerm, EXKEY_F7, tiGetS( "kf7"   ) );
   addKeyMap( pTerm, EXKEY_F8, tiGetS( "kf8"   ) );
   addKeyMap( pTerm, EXKEY_F9, tiGetS( "kf9"   ) );
   addKeyMap( pTerm, EXKEY_F10, tiGetS( "kf10"  ) );
   addKeyMap( pTerm, EXKEY_F11, tiGetS( "kf11"  ) );
   addKeyMap( pTerm, EXKEY_F12, tiGetS( "kf12"  ) );

   /* shifted function keys */
   addKeyMap( pTerm, EXKEY_F1 | KEY_CTRLMASK | KEY_ALTMASK, tiGetS( "kf13" ) );
   addKeyMap( pTerm, EXKEY_F2 | KEY_CTRLMASK | KEY_ALTMASK, tiGetS( "kf14" ) );
   addKeyMap( pTerm, EXKEY_F3 | KEY_CTRLMASK | KEY_ALTMASK, tiGetS( "kf15" ) );
   addKeyMap( pTerm, EXKEY_F4 | KEY_CTRLMASK | KEY_ALTMASK, tiGetS( "kf16" ) );
   addKeyMap( pTerm, EXKEY_F5 | KEY_CTRLMASK | KEY_ALTMASK, tiGetS( "kf17" ) );
   addKeyMap( pTerm, EXKEY_F6 | KEY_CTRLMASK | KEY_ALTMASK, tiGetS( "kf18" ) );
   addKeyMap( pTerm, EXKEY_F7 | KEY_CTRLMASK | KEY_ALTMASK, tiGetS( "kf19" ) );
   addKeyMap( pTerm, EXKEY_F8 | KEY_CTRLMASK | KEY_ALTMASK, tiGetS( "kf20" ) );
   addKeyMap( pTerm, EXKEY_F9 | KEY_CTRLMASK | KEY_ALTMASK, tiGetS( "kf21" ) );
   addKeyMap( pTerm, EXKEY_F10 | KEY_CTRLMASK | KEY_ALTMASK, tiGetS( "kf22" ) );
   addKeyMap( pTerm, EXKEY_F11 | KEY_CTRLMASK | KEY_ALTMASK, tiGetS( "kf23" ) );
   addKeyMap( pTerm, EXKEY_F12 | KEY_CTRLMASK | KEY_ALTMASK, tiGetS( "kf24" ) );
#endif
}

static void hb_gt_trm_SetTerm( PHB_GTTRM pTerm )
{
   static const char *  szAcsc      = "``aaffggiijjkkllmmnnooppqqrrssttuuvvwwxxyyzz{{||}}~~";
   static const char *  szExtAcsc   = "+\020,\021-\030.\0310\333`\004a\261f\370g\361h\260i\316j\331k\277l\332m\300n\305o~p\304q\304r\304s_t\303u\264v\301w\302x\263y\363z\362{\343|\330}\234~\376";
   const char *         szTerm;

   HB_TRACE( HB_TR_DEBUG, ( "hb_gt_trm_SetTerm(%p)", pTerm ) );

   if( pTerm->iOutBufSize == 0 )
   {
      pTerm->iOutBufIndex  = 0;
      pTerm->iOutBufSize   = 16384;
      pTerm->pOutBuf       = ( BYTE * ) hb_xgrab( pTerm->iOutBufSize );
   }
   pTerm->mouse_type = MOUSE_NONE;
   pTerm->esc_delay  = ESC_DELAY;
   pTerm->iAttrMask  = ~HB_GTTRM_ATTR_BOX;

   szTerm            = getenv( "HB_TERM" );
   if( szTerm == NULL || *szTerm == '\0' )
   {
      szTerm = getenv( "TERM" );
      if( szTerm == NULL || *szTerm == '\0' )
         szTerm = "ansi";
   }

   if( strncmp( szTerm, "linux", 5 ) == 0 ||
       strcmp( szTerm, "tterm" ) == 0 ||
       strcmp( szTerm, "teraterm" ) == 0 )
   {
      pTerm->Init             = hb_gt_trm_AnsiInit;
      pTerm->Exit             = hb_gt_trm_AnsiExit;
      pTerm->SetTermMode      = hb_gt_trm_LinuxSetTermMode;
      pTerm->GetCursorPos     = hb_gt_trm_AnsiGetCursorPos;
      pTerm->SetCursorPos     = hb_gt_trm_AnsiSetCursorPos;
      pTerm->SetCursorStyle   = hb_gt_trm_LinuxSetCursorStyle;
      pTerm->SetAttributes    = hb_gt_trm_AnsiSetAttributes;
      pTerm->SetMode          = hb_gt_trm_AnsiSetMode;
      pTerm->GetAcsc          = hb_gt_trm_AnsiGetAcsc;
      pTerm->Tone             = hb_gt_trm_LinuxTone;
      pTerm->Bell             = hb_gt_trm_AnsiBell;
      pTerm->szAcsc           = szExtAcsc;
      pTerm->terminal_type    = TERM_LINUX;
   }
   else if( ( pTerm->terminal_ext & TERM_PUTTY ) ||
            strstr( szTerm, "xterm" ) != NULL ||
            strncmp( szTerm, "rxvt", 4 ) == 0 ||
            strcmp( szTerm, "putty" ) == 0 ||
            strncmp( szTerm, "screen", 6 ) == 0 )
   {
      pTerm->Init             = hb_gt_trm_AnsiInit;
      pTerm->Exit             = hb_gt_trm_AnsiExit;
      pTerm->SetTermMode      = hb_gt_trm_LinuxSetTermMode;
      pTerm->GetCursorPos     = hb_gt_trm_AnsiGetCursorPos;
      pTerm->SetCursorPos     = hb_gt_trm_AnsiSetCursorPos;
      pTerm->SetCursorStyle   = hb_gt_trm_AnsiSetCursorStyle;
      pTerm->SetAttributes    = hb_gt_trm_XtermSetAttributes;
      pTerm->SetMode          = hb_gt_trm_XtermSetMode;
      pTerm->GetAcsc          = hb_gt_trm_AnsiGetAcsc;
      pTerm->Tone             = hb_gt_trm_AnsiTone;
      pTerm->Bell             = hb_gt_trm_AnsiBell;
      pTerm->szAcsc           = szAcsc;
      pTerm->terminal_type    = TERM_XTERM;
   }
   else
   {
      pTerm->Init             = hb_gt_trm_AnsiInit;
      pTerm->Exit             = hb_gt_trm_AnsiExit;
      pTerm->SetTermMode      = hb_gt_trm_AnsiSetTermMode;
      pTerm->GetCursorPos     = hb_gt_trm_AnsiGetCursorPos;
      pTerm->SetCursorPos     = hb_gt_trm_AnsiSetCursorPos;
      pTerm->SetCursorStyle   = hb_gt_trm_AnsiSetCursorStyle;
      pTerm->SetAttributes    = hb_gt_trm_AnsiSetAttributes;
      pTerm->SetMode          = hb_gt_trm_AnsiSetMode;
      pTerm->GetAcsc          = hb_gt_trm_AnsiGetAcsc;
      pTerm->Tone             = hb_gt_trm_AnsiTone;
      pTerm->Bell             = hb_gt_trm_AnsiBell;
      pTerm->szAcsc           = szExtAcsc;
      pTerm->terminal_type    = TERM_ANSI;
   }

   pTerm->fStdinTTY  = hb_fsIsDevice( pTerm->hFilenoStdin );
   pTerm->fStdoutTTY = hb_fsIsDevice( pTerm->hFilenoStdout );
   pTerm->fStderrTTY = hb_fsIsDevice( pTerm->hFilenoStderr );
   pTerm->hFileno    = pTerm->hFilenoStdout;
   pTerm->fOutTTY    = pTerm->fStdoutTTY;
   if( ! pTerm->fOutTTY && pTerm->fStdinTTY )
   {
      pTerm->hFileno = pTerm->hFilenoStdin;
      pTerm->fOutTTY = TRUE;
   }
   pTerm->fPosAnswer = pTerm->fOutTTY;
   pTerm->fUTF8      = FALSE;

   hb_fsSetDevMode( pTerm->hFileno, FD_BINARY );

   hb_gt_chrmapinit( pTerm->charmap, szTerm, pTerm->terminal_type == TERM_XTERM );

#ifndef HB_CDP_SUPPORT_OFF
   pTerm->cdpHost = pTerm->cdpOut = pTerm->cdpIn = NULL;
   pTerm->cdpEN   = hb_cdpFind( "EN" );
#endif
   add_efds( pTerm, pTerm->hFilenoStdin, O_RDONLY, NULL, NULL );
   init_keys( pTerm );
   mouse_init( pTerm );
}

static void hb_gt_trm_Init( PHB_GT pGT, HB_FHANDLE hFilenoStdin, HB_FHANDLE hFilenoStdout, HB_FHANDLE hFilenoStderr )
{
   int         iRows = 24, iCols = 80;
   PHB_GTTRM   pTerm;

   HB_TRACE( HB_TR_DEBUG, ( "hb_gt_trm_Init(%p,%p,%p,%p)", pGT, ( void * ) ( HB_PTRDIFF ) hFilenoStdin, ( void * ) ( HB_PTRDIFF ) hFilenoStdout, ( void * ) ( HB_PTRDIFF ) hFilenoStderr ) );

   pTerm                = ( PHB_GTTRM ) hb_xgrab( sizeof( HB_GTTRM ) );
   memset( pTerm, 0, sizeof( HB_GTTRM ) );
   HB_GTLOCAL( pGT )    = pTerm;
   pTerm->pGT           = pGT;
   pTerm->hFilenoStdin  = hFilenoStdin;
   pTerm->hFilenoStdout = hFilenoStdout;
   pTerm->hFilenoStderr = hFilenoStderr;

   hb_gt_trm_SetTerm( pTerm );

/* SA_NOCLDSTOP in #if is a hack to detect POSIX compatible environment */
#if defined( HB_OS_UNIX ) && defined( SA_NOCLDSTOP )

   if( pTerm->fStdinTTY )
   {
      struct sigaction act, old;

      s_fRestTTY = TRUE;

      /* if( pTerm->saved_TIO.c_lflag & TOSTOP ) != 0 */
      sigaction( SIGTTOU, NULL, &old );
      memcpy( &act, &old, sizeof( struct sigaction ) );
      act.sa_handler = sig_handler;
      /* do not use SA_RESTART - new Linux kernels will repeat the operation */
#if defined( SA_ONESHOT )
      act.sa_flags   = SA_ONESHOT;
#elif defined( SA_RESETHAND )
      act.sa_flags   = SA_RESETHAND;
#else
      act.sa_flags   = 0;
#endif
      sigaction( SIGTTOU, &act, 0 );

      tcgetattr( pTerm->hFilenoStdin, &pTerm->saved_TIO );
      memcpy( &pTerm->curr_TIO, &pTerm->saved_TIO, sizeof( struct termios ) );
      /* atexit( restore_input_mode ); */
      pTerm->curr_TIO.c_lflag &= ~( ECHO | ECHONL | ICANON | ISIG | IEXTEN );
      pTerm->curr_TIO.c_lflag |= NOFLSH;
      pTerm->curr_TIO.c_cflag &= ~( CSIZE | PARENB );
      pTerm->curr_TIO.c_cflag |= CS8 | CREAD;
      pTerm->curr_TIO.c_iflag &= ~( IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON );
      pTerm->curr_TIO.c_oflag &= ~OPOST;
      /* Enable LF->CR+LF translation */
      pTerm->curr_TIO.c_oflag = ONLCR | OPOST;

      memset( pTerm->curr_TIO.c_cc, 0, NCCS );
      /* pTerm->curr_TIO.c_cc[ VMIN ] = 0; */
      /* pTerm->curr_TIO.c_cc[ VTIME ] = 0; */
      tcsetattr( pTerm->hFilenoStdin, TCSAFLUSH, &pTerm->curr_TIO );
      act.sa_handler    = SIG_DFL;

      sigaction( SIGTTOU, &old, NULL );
      pTerm->fRestTTY   = s_fRestTTY;
   }
   set_signals();
   if( ! hb_gt_trm_getSize( pTerm, &iRows, &iCols ) )
   {
      iRows = 24;
      iCols = 80;
   }
#endif

   HB_GTSUPER_INIT( pGT, hFilenoStdin, hFilenoStdout, hFilenoStderr );
   HB_GTSELF_RESIZE( pGT, iRows, iCols );
   HB_GTSELF_SETFLAG( pGT, HB_GTI_COMPATBUFFER, FALSE );
   HB_GTSELF_SETFLAG( pGT, HB_GTI_STDOUTCON, pTerm->fStdoutTTY );
   HB_GTSELF_SETFLAG( pGT, HB_GTI_STDERRCON, pTerm->fStderrTTY );

   pTerm->Init( pTerm );
   pTerm->SetTermMode( pTerm, 0 );
   if( pTerm->GetCursorPos( pTerm, &pTerm->iRow, &pTerm->iCol, NULL ) )
      HB_GTSELF_SETPOS( pGT, pTerm->iRow, pTerm->iCol );
   pTerm->fUTF8 = hb_trm_isUTF8( pTerm );
   hb_gt_trm_SetKeyTrans( pTerm, NULL, NULL );
   hb_gt_trm_SetDispTrans( pTerm, NULL, NULL, 0 );
   HB_GTSELF_SETBLINK( pGT, TRUE );
   if( pTerm->fOutTTY )
      HB_GTSELF_SEMICOLD( pGT );
}

static void hb_gt_trm_Exit( PHB_GT pGT )
{
   PHB_GTTRM pTerm;

   HB_TRACE( HB_TR_DEBUG, ( "hb_gt_trm_Exit(%p)", pGT ) );

   HB_GTSELF_REFRESH( pGT );

   pTerm = HB_GTTRM_GET( pGT );
   if( pTerm )
   {
      mouse_exit( pTerm );
      del_all_efds( pTerm );
      if( pTerm->pKeyTab )
         removeAllKeyMap( pTerm, &pTerm->pKeyTab );

      pTerm->Exit( pTerm );
      if( pTerm->fOutTTY && pTerm->iCol > 0 )
         hb_gt_trm_termOut( pTerm, ( BYTE * ) "\n\r", 2 );
      hb_gt_trm_termFlush( pTerm );
   }

   HB_GTSUPER_EXIT( pGT );

   if( pTerm )
   {
#if defined( HB_OS_UNIX )
      if( pTerm->fRestTTY )
         tcsetattr( pTerm->hFilenoStdin, TCSANOW, &pTerm->saved_TIO );
#endif
      if( pTerm->iLineBufSize > 0 )
         hb_xfree( pTerm->pLineBuf );
      if( pTerm->iOutBufSize > 0 )
         hb_xfree( pTerm->pOutBuf );
      hb_xfree( pTerm );
   }
}

static BOOL hb_gt_trm_mouse_IsPresent( PHB_GT pGT )
{
   HB_TRACE( HB_TR_DEBUG, ( "hb_gt_trm_mouse_IsPresent(%p)", pGT ) );

   return HB_GTTRM_GET( pGT )->mouse_type != MOUSE_NONE;
}

static void hb_gt_trm_mouse_Show( PHB_GT pGT )
{
   PHB_GTTRM pTerm;

   HB_TRACE( HB_TR_DEBUG, ( "hb_gt_trm_mouse_Show(%p)", pGT ) );

   pTerm = HB_GTTRM_GET( pGT );
#ifdef HAVE_GPM_H
   if( pTerm->mouse_type & MOUSE_GPM )
      gpm_visiblepointer = 1;
#endif
   disp_mousecursor( pTerm );
}

static void hb_gt_trm_mouse_Hide( PHB_GT pGT )
{
   HB_TRACE( HB_TR_DEBUG, ( "hb_gt_trm_mouse_Hide(%p)", pGT ) );

#ifdef HAVE_GPM_H
   if( HB_GTTRM_GET( pGT )->mouse_type & MOUSE_GPM )
   {
      gpm_visiblepointer = 0;
   }
#else
   HB_SYMBOL_UNUSED( pGT );
#endif
}

static void hb_gt_trm_mouse_GetPos( PHB_GT pGT, int * piRow, int * piCol )
{
   PHB_GTTRM pTerm;

   HB_TRACE( HB_TR_DEBUG, ( "hb_gt_trm_mouse_Col(%p,%p,%p)", pGT, piRow, piCol ) );

   pTerm    = HB_GTTRM_GET( pGT );
   *piRow   = pTerm->mLastEvt.row;
   *piCol   = pTerm->mLastEvt.col;
}

static void hb_gt_trm_mouse_SetPos( PHB_GT pGT, int iRow, int iCol )
{
   PHB_GTTRM pTerm;

   HB_TRACE( HB_TR_DEBUG, ( "hb_gt_trm_mouse_SetPos(%p,%i,%i)", pGT, iRow, iCol ) );

   pTerm                = HB_GTTRM_GET( pGT );
   /* it does really nothing */
   pTerm->mLastEvt.col  = iCol;
   pTerm->mLastEvt.row  = iRow;
   disp_mousecursor( pTerm );
}

static BOOL hb_gt_trm_mouse_ButtonState( PHB_GT pGT, int iButton )
{
   PHB_GTTRM   pTerm;
   BOOL        ret = FALSE;

   HB_TRACE( HB_TR_DEBUG, ( "hb_gt_trm_mouse_ButtonState(%p,%i)", pGT, iButton ) );

   pTerm = HB_GTTRM_GET( pGT );
   if( pTerm->mouse_type != MOUSE_NONE )
   {
      int mask;

      if( iButton == 0 )
         mask = M_BUTTON_LEFT;
      else if( iButton == 1 )
         mask = M_BUTTON_RIGHT;
      else if( iButton == 2 )
         mask = M_BUTTON_MIDDLE;
      else
         mask = 0;

      ret = ( pTerm->mLastEvt.buttonstate & mask ) != 0;
   }

   return ret;
}

static int hb_gt_trm_mouse_CountButton( PHB_GT pGT )
{
   HB_TRACE( HB_TR_DEBUG, ( "hb_gt_trm_mouse_CountButton(%p)", pGT ) );

   return HB_GTTRM_GET( pGT )->mButtons;
}

static int hb_gt_trm_ReadKey( PHB_GT pGT, int iEventMask )
{
   int iKey;

   HB_TRACE( HB_TR_DEBUG, ( "hb_gt_trm_ReadKey(%p,%d)", pGT, iEventMask ) );

   HB_SYMBOL_UNUSED( iEventMask );

   iKey = wait_key( HB_GTTRM_GET( pGT ), -1 );

   if( iKey == K_RESIZE )
   {
      int iRows, iCols;

      if( hb_gt_trm_getSize( HB_GTTRM_GET( pGT ), &iRows, &iCols ) )
         HB_GTSELF_RESIZE( pGT, iRows, iCols );
      iKey = 0;
   }

   return iKey;
}

static void hb_gt_trm_Tone( PHB_GT pGT, double dFrequency, double dDuration )
{
   PHB_GTTRM pTerm;

   HB_TRACE( HB_TR_DEBUG, ( "hb_gt_trm_Tone(%p,%lf,%lf)", pGT, dFrequency, dDuration ) );

   pTerm = HB_GTTRM_GET( pGT );
   pTerm->Tone( pTerm, dFrequency, dDuration );
}

static void hb_gt_trm_Bell( PHB_GT pGT )
{
   PHB_GTTRM pTerm;

   HB_TRACE( HB_TR_DEBUG, ( "hb_gt_trm_Bell(%p)", pGT ) );

   pTerm = HB_GTTRM_GET( pGT );
   pTerm->Bell( pTerm );
}

static const char * hb_gt_trm_Version( PHB_GT pGT, int iType )
{
   HB_TRACE( HB_TR_DEBUG, ( "hb_gt_trm_Version(%p,%d)", pGT, iType ) );

   HB_SYMBOL_UNUSED( pGT );

   if( iType == 0 )
      return HB_GT_DRVNAME( HB_GT_NAME );

   return "Harbour terminal driver";
}

static BOOL hb_gt_trm_Suspend( PHB_GT pGT )
{
   PHB_GTTRM pTerm;

   HB_TRACE( HB_TR_DEBUG, ( "hb_gt_trm_Suspend(%p)", pGT ) );

   pTerm = HB_GTTRM_GET( pGT );
#if defined( HB_OS_UNIX )
   if( pTerm->fRestTTY )
      tcsetattr( pTerm->hFilenoStdin, TCSANOW, &pTerm->saved_TIO );
#endif
   /* Enable line wrap when cursor set after last column */
   pTerm->SetTermMode( pTerm, 1 );
   return TRUE;
}

static BOOL hb_gt_trm_Resume( PHB_GT pGT )
{
   PHB_GTTRM   pTerm;
   int         iHeight, iWidth;

   HB_TRACE( HB_TR_DEBUG, ( "hb_gt_trm_Resume(%p)", pGT ) );

   pTerm = HB_GTTRM_GET( pGT );
#if defined( HB_OS_UNIX )
   if( pTerm->fRestTTY )
      tcsetattr( pTerm->hFilenoStdin, TCSANOW, &pTerm->curr_TIO );
#endif
   if( pTerm->mouse_type & MOUSE_XTERM )
      hb_gt_trm_termOut( pTerm, ( BYTE * ) s_szMouseOn, strlen( s_szMouseOn ) );

   pTerm->Init( pTerm );

   HB_GTSELF_GETSIZE( pGT, &iHeight, &iWidth );
   HB_GTSELF_EXPOSEAREA( pGT, 0, 0, iHeight, iWidth );

   HB_GTSELF_REFRESH( pGT );

   return TRUE;
}

static void hb_gt_trm_Scroll( PHB_GT pGT, int iTop, int iLeft, int iBottom, int iRight,
                              BYTE bColor, BYTE bChar, int iRows, int iCols )
{
   HB_TRACE( HB_TR_DEBUG, ( "hb_gt_trm_Scroll(%p,%d,%d,%d,%d,%d,%d,%d,%d)", pGT, iTop, iLeft, iBottom, iRight, bColor, bChar, iRows, iCols ) );

   /* Provide some basic scroll support for full screen */
   if( iCols == 0 && iRows > 0 && iTop == 0 && iLeft == 0 )
   {
      PHB_GTTRM   pTerm = HB_GTTRM_GET( pGT );
      int         iHeight, iWidth;

      HB_GTSELF_GETSIZE( pGT, &iHeight, &iWidth );
      if( iBottom >= iHeight - 1 && iRight >= iWidth - 1 &&
          pTerm->iRow == iHeight - 1 )
      {
         /* scroll up the internal screen buffer */
         HB_GTSELF_SCROLLUP( pGT, iRows, bColor, bChar );
         /* set default color for terminals which use it to erase
          * scrolled area */
         pTerm->SetAttributes( pTerm, bColor & pTerm->iAttrMask );
         /* update our internal row position */
         do
         {
            hb_gt_trm_termOut( pTerm, ( BYTE * ) "\n\r", 2 );
         }
         while( --iRows > 0 );
         pTerm->iCol = 0;
         return;
      }
   }

   HB_GTSUPER_SCROLL( pGT, iTop, iLeft, iBottom, iRight, bColor, bChar, iRows, iCols );
}

static BOOL hb_gt_trm_SetMode( PHB_GT pGT, int iRows, int iCols )
{
   PHB_GTTRM pTerm;

   HB_TRACE( HB_TR_DEBUG, ( "hb_gt_trm_SetMode(%p,%d,%d)", pGT, iRows, iCols ) );

   pTerm = HB_GTTRM_GET( pGT );
   if( pTerm->SetMode( pTerm, &iRows, &iCols ) )
   {
      HB_GTSELF_RESIZE( pGT, iRows, iCols );
      return TRUE;
   }
   return FALSE;
}

static void hb_gt_trm_SetBlink( PHB_GT pGT, BOOL fBlink )
{
   PHB_GTTRM pTerm;

   HB_TRACE( HB_TR_DEBUG, ( "hb_gt_trm_SetBlink(%p,%d)", pGT, ( int ) fBlink ) );

   pTerm = HB_GTTRM_GET( pGT );

#if 0
   /* This is not portable extension - temporary disabled */
   if( pTerm->terminal_ext & TERM_PUTTY )
   {
      static const char *  szBlinkOff  = "\033[=0E";  /* disable blinking, highlight bkg */
      static const char *  szBlinkOn   = "\033[=1E";  /* enable blinking */

      const char *         szBlink     = fBlink ? szBlinkOn : szBlinkOff;

      pTerm->iAttrMask |= 0x0080;
      hb_gt_trm_termOut( pTerm, ( BYTE * ) szBlink, strlen( szBlink ) );
      hb_gt_trm_termFlush( pTerm );
   }
   else
#endif
   {
      if( fBlink )
         pTerm->iAttrMask |= 0x0080;
      else
         pTerm->iAttrMask &= ~0x0080;
   }

   HB_GTSUPER_SETBLINK( pGT, fBlink );
}

static BOOL hb_gt_trm_SetDispCP( PHB_GT pGT, const char * pszTermCDP, const char * pszHostCDP, BOOL fBox )
{
   HB_TRACE( HB_TR_DEBUG, ( "hb_gt_trm_SetDispCP(%p,%s,%s,%d)", pGT, pszTermCDP, pszHostCDP, ( int ) fBox ) );

   HB_GTSUPER_SETDISPCP( pGT, pszTermCDP, pszHostCDP, fBox );

#ifndef HB_CDP_SUPPORT_OFF
   if( ! pszHostCDP )
      pszHostCDP = hb_cdpID();
   if( ! pszTermCDP )
      pszTermCDP = pszHostCDP;

   if( pszTermCDP && pszHostCDP )
   {
      PHB_GTTRM pTerm = HB_GTTRM_GET( pGT );

      pTerm->cdpOut  = hb_cdpFind( pszTermCDP );
      pTerm->cdpHost = hb_cdpFind( pszHostCDP );

      if( pTerm->cdpOut && pTerm->cdpHost &&
          pTerm->cdpHost->nChars &&
          pTerm->cdpHost->nChars == pTerm->cdpOut->nChars )
      {
         int      iChars         = pTerm->cdpHost->nChars;
         char *   pszHostLetters = ( char * ) hb_xgrab( iChars * 2 + 1 );
         char *   pszTermLetters = ( char * ) hb_xgrab( iChars * 2 + 1 );

         memcpy( pszHostLetters, pTerm->cdpHost->CharsUpper, iChars );
         memcpy( pszHostLetters + iChars, pTerm->cdpHost->CharsLower, iChars + 1 );
         memcpy( pszTermLetters, pTerm->cdpOut->CharsUpper, iChars );
         memcpy( pszTermLetters + iChars, pTerm->cdpOut->CharsLower, iChars + 1 );

         hb_gt_trm_SetDispTrans( pTerm, pszHostLetters, pszTermLetters, fBox ? 1 : 0 );

         hb_xfree( pszHostLetters );
         hb_xfree( pszTermLetters );
      }
      return TRUE;
   }
#endif

   return FALSE;
}

static BOOL hb_gt_trm_SetKeyCP( PHB_GT pGT, const char * pszTermCDP, const char * pszHostCDP )
{
   HB_TRACE( HB_TR_DEBUG, ( "hb_gt_trm_SetKeyCP(%p,%s,%s)", pGT, pszTermCDP, pszHostCDP ) );

   HB_GTSUPER_SETKEYCP( pGT, pszTermCDP, pszHostCDP );

#ifndef HB_CDP_SUPPORT_OFF
   if( ! pszHostCDP )
      pszHostCDP = hb_cdpID();
   if( ! pszTermCDP )
      pszTermCDP = pszHostCDP;

   if( pszTermCDP && pszHostCDP )
   {
      PHB_GTTRM      pTerm    = HB_GTTRM_GET( pGT );
      PHB_CODEPAGE   cdpTerm  = hb_cdpFind( pszTermCDP ),
                     cdpHost  = hb_cdpFind( pszHostCDP );
      if( cdpTerm && cdpHost && cdpTerm != cdpHost &&
          cdpTerm->nChars && cdpTerm->nChars == cdpHost->nChars )
      {
         char *   pszHostLetters = ( char * ) hb_xgrab( ( cdpHost->nChars << 1 ) + 1 );
         char *   pszTermLetters = ( char * ) hb_xgrab( ( cdpTerm->nChars << 1 ) + 1 );

         memcpy( pszHostLetters, cdpHost->CharsUpper, cdpHost->nChars );
         memcpy( pszHostLetters + cdpHost->nChars, cdpHost->CharsLower, cdpHost->nChars );
         pszHostLetters[ cdpHost->nChars << 1 ] = '\0';
         memcpy( pszTermLetters, cdpTerm->CharsUpper, cdpTerm->nChars );
         memcpy( pszTermLetters + cdpTerm->nChars, cdpTerm->CharsLower, cdpTerm->nChars );
         pszTermLetters[ cdpTerm->nChars << 1 ] = '\0';

         hb_gt_trm_SetKeyTrans( pTerm, pszTermLetters, pszHostLetters );

         hb_xfree( pszHostLetters );
         hb_xfree( pszTermLetters );
      }
      else
         hb_gt_trm_SetKeyTrans( pTerm, NULL, NULL );

      pTerm->cdpIn = cdpTerm;

      return TRUE;
   }
#endif

   return FALSE;
}

static void hb_gt_trm_Redraw( PHB_GT pGT, int iRow, int iCol, int iSize )
{
   PHB_GTTRM   pTerm;
   BYTE        bColor, bAttr;
   USHORT      usChar;
   int         iLen = 0, iAttribute = 0, iColor;

   HB_TRACE( HB_TR_DEBUG, ( "hb_gt_trm_Redraw(%p,%d,%d,%d)", pGT, iRow, iCol, iSize ) );

   pTerm = HB_GTTRM_GET( pGT );
   pTerm->SetTermMode( pTerm, 0 );
   pTerm->SetCursorStyle( pTerm, SC_NONE );
   while( iSize-- )
   {
      if( ! HB_GTSELF_GETSCRCHAR( pGT, iRow, iCol + iLen, &bColor, &bAttr, &usChar ) )
         break;

      usChar &= 0xff;
      if( bAttr & HB_GT_ATTR_BOX )
      {
         iColor = bColor | ( pTerm->boxattr[ usChar ] & ~HB_GTTRM_ATTR_CHAR );
         if( ! pTerm->fUTF8 )
            usChar = pTerm->boxattr[ usChar ] & HB_GTTRM_ATTR_CHAR;
         else
            iColor |= HB_GTTRM_ATTR_BOX;
      }
      else
      {
         iColor = bColor | ( pTerm->chrattr[ usChar ] & ~HB_GTTRM_ATTR_CHAR );
         if( ! pTerm->fUTF8 )
            usChar = pTerm->chrattr[ usChar ] & HB_GTTRM_ATTR_CHAR;
      }

      if( iLen == 0 )
         iAttribute = iColor;
      else if( iColor != iAttribute )
      {
         hb_gt_trm_PutStr( pTerm, iRow, iCol, iAttribute, pTerm->pLineBuf, iLen );
         iCol        += iLen;
         iLen        = 0;
         iAttribute  = iColor;
      }
      pTerm->pLineBuf[ iLen++ ] = ( BYTE ) usChar;
   }
   if( iLen )
   {
      hb_gt_trm_PutStr( pTerm, iRow, iCol, iAttribute, pTerm->pLineBuf, iLen );
   }
}

static void hb_gt_trm_Refresh( PHB_GT pGT )
{
   int         iWidth, iHeight, iRow, iCol, iStyle;
   PHB_GTTRM   pTerm;

   HB_TRACE( HB_TR_DEBUG, ( "hb_gt_trm_Refresh(%p)", pGT ) );

   HB_GTSELF_GETSIZE( pGT, &iHeight, &iWidth );

   pTerm = HB_GTTRM_GET( pGT );
   if( pTerm->iLineBufSize == 0 )
   {
      pTerm->pLineBuf      = ( BYTE * ) hb_xgrab( iWidth );
      pTerm->iLineBufSize  = iWidth;
   }
   else if( pTerm->iLineBufSize != iWidth )
   {
      pTerm->pLineBuf      = ( BYTE * ) hb_xrealloc( pTerm->pLineBuf, iWidth );
      pTerm->iLineBufSize  = iWidth;
   }

   HB_GTSUPER_REFRESH( pGT );

   HB_GTSELF_GETSCRCURSOR( pGT, &iRow, &iCol, &iStyle );
   if( iStyle != SC_NONE )
   {
      if( iRow >= 0 && iCol >= 0 && iRow < iHeight && iCol < iWidth )
         pTerm->SetCursorPos( pTerm, iRow, iCol );
      else
         iStyle = SC_NONE;
   }
   pTerm->SetCursorStyle( pTerm, iStyle );
   hb_gt_trm_termFlush( pTerm );
   disp_mousecursor( pTerm );
}

static BOOL hb_gt_trm_Info( PHB_GT pGT, int iType, PHB_GT_INFO pInfo )
{
   PHB_GTTRM      pTerm;
   const char *   szVal;
   int            iVal;

   HB_TRACE( HB_TR_DEBUG, ( "hb_gt_trm_Info(%p,%d,%p)", pGT, iType, pInfo ) );

   pTerm = HB_GTTRM_GET( pGT );
   switch( iType )
   {
      case HB_GTI_FULLSCREEN:
      case HB_GTI_KBDSUPPORT:
         pInfo->pResult = hb_itemPutL( pInfo->pResult, TRUE );
         break;

      case HB_GTI_ISUNICODE:
         pInfo->pResult = hb_itemPutL( pInfo->pResult, pTerm->fUTF8 );
         break;

      case HB_GTI_ESCDELAY:
         pInfo->pResult = hb_itemPutNI( pInfo->pResult, pTerm->esc_delay );
         if( hb_itemType( pInfo->pNewVal ) & HB_IT_NUMERIC )
            pTerm->esc_delay = hb_itemGetNI( pInfo->pNewVal );
         break;

      case HB_GTI_DELKEYMAP:
         szVal = hb_itemGetCPtr( pInfo->pNewVal );
         if( szVal && *szVal )
            removeKeyMap( pTerm, hb_itemGetCPtr( pInfo->pNewVal ) );
         break;

      case HB_GTI_ADDKEYMAP:
         iVal  = hb_arrayGetNI( pInfo->pNewVal, 1 );
         szVal = hb_arrayGetCPtr( pInfo->pNewVal, 2 );
         if( iVal && szVal && *szVal )
            addKeyMap( pTerm, SET_CLIPKEY( iVal ), szVal );
         break;

      default:
         return HB_GTSUPER_INFO( pGT, iType, pInfo );
   }

   return TRUE;
}

static BOOL hb_gt_FuncInit( PHB_GT_FUNCS pFuncTable )
{
   HB_TRACE( HB_TR_DEBUG, ( "hb_gt_FuncInit(%p)", pFuncTable ) );

   pFuncTable->Init              = hb_gt_trm_Init;
   pFuncTable->Exit              = hb_gt_trm_Exit;
   pFuncTable->Redraw            = hb_gt_trm_Redraw;
   pFuncTable->Refresh           = hb_gt_trm_Refresh;
   pFuncTable->Scroll            = hb_gt_trm_Scroll;
   pFuncTable->Version           = hb_gt_trm_Version;
   pFuncTable->Suspend           = hb_gt_trm_Suspend;
   pFuncTable->Resume            = hb_gt_trm_Resume;
   pFuncTable->SetMode           = hb_gt_trm_SetMode;
   pFuncTable->SetBlink          = hb_gt_trm_SetBlink;
   pFuncTable->SetDispCP         = hb_gt_trm_SetDispCP;
   pFuncTable->SetKeyCP          = hb_gt_trm_SetKeyCP;
   pFuncTable->Tone              = hb_gt_trm_Tone;
   pFuncTable->Bell              = hb_gt_trm_Bell;
   pFuncTable->Info              = hb_gt_trm_Info;

   pFuncTable->ReadKey           = hb_gt_trm_ReadKey;

   pFuncTable->MouseIsPresent    = hb_gt_trm_mouse_IsPresent;
   pFuncTable->MouseShow         = hb_gt_trm_mouse_Show;
   pFuncTable->MouseHide         = hb_gt_trm_mouse_Hide;
   pFuncTable->MouseGetPos       = hb_gt_trm_mouse_GetPos;
   pFuncTable->MouseSetPos       = hb_gt_trm_mouse_SetPos;
   pFuncTable->MouseButtonState  = hb_gt_trm_mouse_ButtonState;
   pFuncTable->MouseCountButton  = hb_gt_trm_mouse_CountButton;

   return TRUE;
}

/* ********************************************************************** */

static const HB_GT_INIT gtInit = { ( char * ) HB_GT_DRVNAME( HB_GT_NAME ),
                                   hb_gt_FuncInit,
                                   HB_GTSUPER,
                                   HB_GTID_PTR };

HB_GT_ANNOUNCE( HB_GT_NAME )

HB_CALL_ON_STARTUP_BEGIN( _hb_startup_gt_Init_ )
hb_gtRegister( &gtInit );
HB_CALL_ON_STARTUP_END( _hb_startup_gt_Init_ )

#if defined( HB_PRAGMA_STARTUP )
   #pragma startup _hb_startup_gt_Init_
#elif defined( HB_DATASEG_STARTUP )
   #define HB_DATASEG_BODY HB_DATASEG_FUNC( _hb_startup_gt_Init_ )
   #include "hbiniseg.h"
#endif
/* *********************************************************************** */

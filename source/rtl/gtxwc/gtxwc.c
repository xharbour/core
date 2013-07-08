/*
 * $Id: gtxwc.c 9725 2012-10-02 10:56:51Z andijahja $
 */

/*
 * [x]Harbour Project source code:
 *    XWindow Console
 * Copyright 2003 - Giancarlo Niccolai <antispam /at/ niccolai.ws>
 * Copyright 2004/2006 - Przemyslaw Czerpak <druzus /at/ priv.onet.pl>
 *
 * www - http://www.harbour-project.org
 * www - http://www.xharbour.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.   If not, write to
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
 * not apply to the code that you add in this way.   To avoid misleading
 * anyone as to the status of such modified files, you must delete
 * this exception notice from them.
 *
 * If you write modifications of your own for Harbour, it is your choice
 * whether to permit this exception to apply to your modifications.
 * If you do not wish that, delete this exception notice.
 *
 */

/* NOTE: User programs should never call this layer directly! */

/* #define XWC_DEBUG */
#include "gtxwc.h"

static int           s_GtId;
static HB_GT_FUNCS   SuperTable;
#define HB_GTSUPER            ( &SuperTable )
#define HB_GTID_PTR           ( &s_GtId )

#define HB_GTXWC_GET( p ) ( ( PXWND_DEF ) HB_GTLOCAL( p ) )

#ifdef HB_XWC_XLIB_NEEDLOCKS
static HB_CRITICAL_NEW( s_xwcMtx );
#  define HB_XWC_XLIB_LOCK    hb_threadEnterCriticalSection( &s_xwcMtx );
#  define HB_XWC_XLIB_UNLOCK  hb_threadLeaveCriticalSection( &s_xwcMtx );
#else
#  define HB_XWC_XLIB_LOCK
#  define HB_XWC_XLIB_UNLOCK
#endif

/* mouse button mapping into Clipper keycodes */
static const int  mousePressKeys[ XWC_MAX_BUTTONS ] = { K_LBUTTONDOWN, K_MBUTTONDOWN, K_RBUTTONDOWN, K_MWFORWARD, K_MWBACKWARD };
static const int  mouseReleaseKeys[ XWC_MAX_BUTTONS ] = { K_LBUTTONUP, K_MBUTTONUP, K_RBUTTONUP };
static const int  mouseDblPressKeys[ XWC_MAX_BUTTONS ] = { K_LDBLCLK, K_MDBLCLK, K_RDBLCLK, K_MWFORWARD, K_MWBACKWARD };

typedef struct tag_ClipKeyCode
{
   int key;
   int alt_key;
   int ctrl_key;
   int shift_key;
} ClipKeyCode;

/* The tables below are indexed by internal key value,
 * It cause that we don't have to make any linear scans
 * to access proper ClipKeyCode entry
 */
static const ClipKeyCode   stdKeyTab[ CLIP_STDKEY_COUNT ] = {
   { K_SPACE,   0,               0,               0 },   /*  32 */
   { '!',       0,               0,               0 },   /*  33 */
   { '"',       0,               0,               0 },   /*  34 */
   { '#',       0,               0,               0 },   /*  35 */
   { '$',       0,               0,               0 },   /*  36 */
   { '%',       0,               0,               0 },   /*  37 */
   { '&',       0,               0,               0 },   /*  38 */
   { '\'',      296,             7,               0 },   /*  39 */
   { '(',       0,               0,               0 },   /*  40 */
   { ')',       0,               0,               0 },   /*  41 */
   { '*',       0,               0,               0 },   /*  42 */
   { '+',       0,               0,               0 },   /*  43 */
   { ',',       307,             0,               0 },   /*  44 */
   { '-',       386,             31,              0 },   /*  45 */
   { '.',       308,             0,               0 },   /*  46 */
   { '/',       K_CTRL_QUESTION, 127,             0 },   /*  47 */
   { '0',       K_ALT_0,         0,               0 },   /*  48 */
   { '1',       K_ALT_1,         0,               0 },   /*  49 */
   { '2',       K_ALT_2,         259,             0 },   /*  50 */
   { '3',       K_ALT_3,         27,              0 },   /*  51 */
   { '4',       K_ALT_4,         28,              0 },   /*  52 */
   { '5',       K_ALT_5,         29,              0 },   /*  53 */
   { '6',       K_ALT_6,         30,              0 },   /*  54 */
   { '7',       K_ALT_7,         31,              0 },   /*  55 */
   { '8',       K_ALT_8,         127,             0 },   /*  56 */
   { '9',       K_ALT_9,         0,               0 },   /*  57 */
   { ':',       0,               0,               0 },   /*  58 */
   { ';',       295,             0,               0 },   /*  59 */
   { '<',       0,               0,               0 },   /*  60 */
   { '=',       K_ALT_EQUALS,    0,               0 },   /*  61 */
   { '>',       0,               0,               0 },   /*  62 */
   { '?',       0,               K_CTRL_QUESTION, 0 },   /*  63 */
   { '@',       0,               0,               0 },   /*  64 */
   { 'A',       K_ALT_A,         K_CTRL_A,        0 },   /*  65 */
   { 'B',       K_ALT_B,         K_CTRL_B,        0 },   /*  66 */
   { 'C',       K_ALT_C,         K_CTRL_C,        0 },   /*  67 */
   { 'D',       K_ALT_D,         K_CTRL_D,        0 },   /*  68 */
   { 'E',       K_ALT_E,         K_CTRL_E,        0 },   /*  69 */
   { 'F',       K_ALT_F,         K_CTRL_F,        0 },   /*  70 */
   { 'G',       K_ALT_G,         K_CTRL_G,        0 },   /*  71 */
   { 'H',       K_ALT_H,         K_CTRL_H,        0 },   /*  72 */
   { 'I',       K_ALT_I,         K_CTRL_I,        0 },   /*  73 */
   { 'J',       K_ALT_J,         K_CTRL_J,        0 },   /*  74 */
   { 'K',       K_ALT_K,         K_CTRL_K,        0 },   /*  75 */
   { 'L',       K_ALT_L,         K_CTRL_L,        0 },   /*  76 */
   { 'M',       K_ALT_M,         K_CTRL_M,        0 },   /*  77 */
   { 'N',       K_ALT_N,         K_CTRL_N,        0 },   /*  78 */
   { 'O',       K_ALT_O,         K_CTRL_O,        0 },   /*  79 */
   { 'P',       K_ALT_P,         K_CTRL_P,        0 },   /*  80 */
   { 'Q',       K_ALT_Q,         K_CTRL_Q,        0 },   /*  81 */
   { 'R',       K_ALT_R,         K_CTRL_R,        0 },   /*  82 */
   { 'S',       K_ALT_S,         K_CTRL_S,        0 },   /*  83 */
   { 'T',       K_ALT_T,         K_CTRL_T,        0 },   /*  84 */
   { 'U',       K_ALT_U,         K_CTRL_U,        0 },   /*  85 */
   { 'V',       K_ALT_V,         K_CTRL_V,        0 },   /*  86 */
   { 'W',       K_ALT_W,         K_CTRL_W,        0 },   /*  87 */
   { 'X',       K_ALT_X,         K_CTRL_X,        0 },   /*  88 */
   { 'Y',       K_ALT_Y,         K_CTRL_Y,        0 },   /*  89 */
   { 'Z',       K_ALT_Z,         K_CTRL_Z,        0 },   /*  90 */
   { '[',       282,             27,              0 },   /*  91 */
   { '\\',      299,             28,              0 },   /*  92 */
   { ']',       283,             29,              0 },   /*  93 */
   { '^',       K_ALT_6,         30,              0 },   /*  94 */
   { '_',       386,             31,              0 },   /*  95 */
   { '`',       297,             297,             0 },   /*  96 */
   { 'a',       K_ALT_A,         K_CTRL_A,        0 },   /*  97 */
   { 'b',       K_ALT_B,         K_CTRL_B,        0 },   /*  98 */
   { 'c',       K_ALT_C,         K_CTRL_C,        0 },   /*  99 */
   { 'd',       K_ALT_D,         K_CTRL_D,        0 },   /* 100 */
   { 'e',       K_ALT_E,         K_CTRL_E,        0 },   /* 101 */
   { 'f',       K_ALT_F,         K_CTRL_F,        0 },   /* 102 */
   { 'g',       K_ALT_G,         K_CTRL_G,        0 },   /* 103 */
   { 'h',       K_ALT_H,         K_CTRL_H,        0 },   /* 104 */
   { 'i',       K_ALT_I,         K_CTRL_I,        0 },   /* 105 */
   { 'j',       K_ALT_J,         K_CTRL_J,        0 },   /* 106 */
   { 'k',       K_ALT_K,         K_CTRL_K,        0 },   /* 107 */
   { 'l',       K_ALT_L,         K_CTRL_L,        0 },   /* 108 */
   { 'm',       K_ALT_M,         K_CTRL_M,        0 },   /* 109 */
   { 'n',       K_ALT_N,         K_CTRL_N,        0 },   /* 110 */
   { 'o',       K_ALT_O,         K_CTRL_O,        0 },   /* 111 */
   { 'p',       K_ALT_P,         K_CTRL_P,        0 },   /* 112 */
   { 'q',       K_ALT_Q,         K_CTRL_Q,        0 },   /* 113 */
   { 'r',       K_ALT_R,         K_CTRL_R,        0 },   /* 114 */
   { 's',       K_ALT_S,         K_CTRL_S,        0 },   /* 115 */
   { 't',       K_ALT_T,         K_CTRL_T,        0 },   /* 116 */
   { 'u',       K_ALT_U,         K_CTRL_U,        0 },   /* 117 */
   { 'v',       K_ALT_V,         K_CTRL_V,        0 },   /* 118 */
   { 'w',       K_ALT_W,         K_CTRL_W,        0 },   /* 119 */
   { 'x',       K_ALT_X,         K_CTRL_X,        0 },   /* 120 */
   { 'y',       K_ALT_Y,         K_CTRL_Y,        0 },   /* 121 */
   { 'z',       K_ALT_Z,         K_CTRL_Z,        0 },   /* 122 */
   { '{',       282,             27,              0 },   /* 123 */
   { '|',       299,             28,              0 },   /* 124 */
   { '}',       283,             29,              0 },   /* 125 */
   { '~',       297,             297,             0 },   /* 126 */
   { K_CTRL_BS, K_ALT_BS,        127,             0 },   /* 127 */
};

static const ClipKeyCode   extKeyTab[ CLIP_EXTKEY_COUNT ] = {
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
   { K_ENTER,   K_ALT_ENTER,  K_CTRL_ENTER,  0          },  /*  25 */
   { K_ENTER,   KP_ALT_ENTER, K_CTRL_ENTER,  0          },  /*  26 */
   { KP_CENTER, 0,            KP_CTRL_5,     0          },  /*  27 */
   { 0,         0,            K_CTRL_PRTSCR, 0          },  /*  28 */
   { 0,         0,            HB_BREAK_FLAG, 0          } /*  29 */
};

#ifdef HB_CDP_SUPPORT_OFF
static const UnixBoxChar   boxTranslate[] = {
   { 16,  HB_GTXVG_ARROW_R     },
   { 17,  HB_GTXVG_ARROW_L     },
   { 30,  HB_GTXVG_ARROW_U     },
   { 31,  HB_GTXVG_ARROW_D     },
   { 176, HB_GTXVG_FILLER1     },
   { 177, HB_GTXVG_FILLER2     },
   { 178, HB_GTXVG_FILLER3     },
   { 179, HB_GTXWC_SNG_VRT     },
   { 180, HB_GTXWC_SNG_VR      },
   { 181, HB_GTXWC_SNG_V_DBL_R },
   { 182, HB_GTXWC_DBL_V_SNG_R },
   { 183, HB_GTXWC_SNG_R_DBL_T },
   { 184, HB_GTXWC_DBL_R_SNG_T },
   { 185, HB_GTXWC_DBL_VR      },
   { 186, HB_GTXWC_DBL_VRT     },
   { 187, HB_GTXWC_DBL_RT      },
   { 188, HB_GTXWC_DBL_RB      },
   { 189, HB_GTXWC_SNG_R_DBL_B },
   { 190, HB_GTXWC_DBL_R_SNG_B },
   { 191, HB_GTXWC_SNG_RT      },
   { 192, HB_GTXWC_SNG_LB      },
   { 193, HB_GTXWC_SNG_BU      },
   { 194, HB_GTXWC_SNG_TD      },
   { 195, HB_GTXWC_SNG_VL      },
   { 196, HB_GTXWC_SNG_HOR     },
   { 197, HB_GTXWC_SNG_CRS     },
   { 198, HB_GTXWC_SNG_V_DBL_L },
   { 199, HB_GTXWC_DBL_V_SNG_L },
   { 200, HB_GTXWC_DBL_LB      },
   { 201, HB_GTXWC_DBL_LT      },
   { 202, HB_GTXWC_DBL_BU      },
   { 203, HB_GTXWC_DBL_TD      },
   { 204, HB_GTXWC_DBL_VL      },
   { 205, HB_GTXWC_DBL_HOR     },
   { 206, HB_GTXWC_DBL_CRS     },
   { 207, HB_GTXWC_DBL_B_SNG_U },
   { 208, HB_GTXWC_SNG_B_DBL_U },
   { 209, HB_GTXWC_DBL_T_SNG_D },
   { 210, HB_GTXWC_SNG_T_DBL_D },
   { 211, HB_GTXWC_DBL_L_SNG_B },
   { 212, HB_GTXWC_SNG_L_DBL_B },
   { 213, HB_GTXWC_SNG_L_DBL_T },
   { 214, HB_GTXWC_DBL_L_SNG_T },
   { 215, HB_GTXWC_DBL_SNG_CRS },
   { 216, HB_GTXWC_SNG_DBL_CRS },
   { 217, HB_GTXWC_SNG_RB      },
   { 218, HB_GTXWC_SNG_LT      },
   { 219, HB_GTXVG_FULL        },
   { 220, HB_GTXVG_FULL_B      },
   { 221, HB_GTXVG_FULL_L      },
   { 222, HB_GTXVG_FULL_R      },
   { 223, HB_GTXVG_FULL_T      },
   { 254, HB_GTXVG_SQUARE      }
};
#define XWC_BOX_CHARS ( sizeof( boxTranslate ) / sizeof( UnixBoxChar ) )
#endif

/* these are standard PC console colors in RGB */
static const char *  rgb_colors[] = {
   "rgb:00/00/00",   /* black         */
   "rgb:00/00/AA",   /* blue          */
   "rgb:00/AA/00",   /* green         */
   "rgb:00/AA/AA",   /* cyan          */
   "rgb:AA/00/00",   /* red           */
   "rgb:AA/00/AA",   /* magenta       */
   "rgb:AA/55/00",   /* brown         */
   "rgb:AA/AA/AA",   /* light gray    */
   "rgb:55/55/55",   /* gray          */
   "rgb:55/55/FF",   /* light blue    */
   "rgb:55/FF/55",   /* light green   */
   "rgb:55/FF/FF",   /* light cyan    */
   "rgb:FF/55/55",   /* light red     */
   "rgb:FF/55/FF",   /* light magenta */
   "rgb:FF/FF/55",   /* yellow        */
   "rgb:FF/FF/FF"    /* white         */
};

static Atom          s_atomDelWin;
static Atom          s_atomTimestamp;
static Atom          s_atomAtom;
static Atom          s_atomInteger;
static Atom          s_atomString;
static Atom          s_atomUTF8String;
static Atom          s_atomPrimary;
static Atom          s_atomSecondary;
static Atom          s_atomClipboard;
static Atom          s_atomTargets;
static Atom          s_atomCutBuffer0;
static Atom          s_atomText;
static Atom          s_atomCompoundText;


typedef struct tag_rect
{
   int top;
   int left;
   int right;
   int bottom;
} RECT;

typedef struct tag_modifiers
{
   BOOL bCtrl;
   BOOL bAlt;
   BOOL bAltGr;
   BOOL bShift;
} MODIFIERS;

typedef struct tag_x_wnddef
{
   PHB_GT pGT;

   Display * dpy;
   Window window;
   GC gc;
   Colormap colors;
   HB_GT_PIXELTYPE pixels[ 16 ];
   Pixmap pm;
   Drawable drw;

   void ( * evt_callback )( void );

   /* is main window initialized */
   BOOL fInit;
   /* is anything written to screen */
   BOOL fData;

   /* block recursive refresh calls */
   BOOL fRefresh;

   /* window size in character cells */
   USHORT cols;
   USHORT rows;

   /* window size in pixels */
   USHORT width;
   USHORT height;

   /* Set to true when Windows is resized */
   BOOL fWinResize;
   USHORT newWidth;
   USHORT newHeight;

   /* window title */
   char * szTitle;
   BOOL fDspTitle;

   /* used font informations */
   XFontStruct * xfs;
   char * szFontName;
   char * szFontWeight;
   char * szFontEncoding;
   int fontHeight;
   int fontWidth;
   /* if font has bad metric then try to fix it and display only single
      char at cell position at once */
   BOOL fFixMetric;
   /* if font has bad size and doesn't clear background cell we can
      try to fix it and clear background drawing rectangle before
      displaying font */
   BOOL fClearBkg;
   /* if TRUE then BOX characters will be drawn by GTXWC instead of
      using build in font ones */
   BOOL fDrawBox;

   /* CodePage support */
   PHB_CODEPAGE hostCDP;
   /* PHB_CODEPAGE outCDP; */
   PHB_CODEPAGE inCDP;

   /* current cursor and color settings */
   int col;
   int row;
   int cursorType;

   /* last cursor position and shape */
   int lastCursorCol;
   int lastCursorRow;
   int lastCursorType;

   BOOL cursorState;
   ULONG cursorBlinkRate;
   ULONG cursorStateTime;

   /* Mouse informations */
   int mouseCol;
   int mouseRow;
   int mouseGotoCol;
   int mouseGotoRow;
   int mouseNumButtons;
   int mouseButtonsState;
   BYTE mouseButtonsMap[ XWC_MAX_BUTTONS ];
   Time mouseButtonsTime[ XWC_MAX_BUTTONS ];

   /* current screen contents (attr<<24)|(color<<16)|char */
   ULONG * pCurrScr;

   /* character translation table, it changes BYTES in screen buffer into UNICODE or graphs primitives */
   XWC_CharTrans charTrans[ 256 ];

   BOOL fInvalidChr;
   RECT rInvalidChr;

   BOOL fInvalidPts;
   RECT rInvalidPts;

   /* Keyboard buffer */
   int keyBuffPointer;
   int keyBuffNO;
   int KeyBuff[ XWC_CHAR_QUEUE_SIZE ];
   MODIFIERS keyModifiers;

   /* Clipboard buffer */
   unsigned char * ClipboardData;
   ULONG ClipboardSize;
   Atom ClipboardRequest;
   Time ClipboardTime;
   BOOL ClipboardOwner;
   BOOL ClipboardRcvd;

   /* Clipping */
   XRectangle ClipRect;

   /* Keep last event time */
   Time lastEventTime;

} XWND_DEF, * PXWND_DEF;

/******************************************************************/

static void hb_gt_xwc_ProcessMessages( PXWND_DEF wnd );
static void hb_gt_xwc_InvalidatePts( PXWND_DEF wnd, int left, int top, int right, int bottom );

/************************ globals ********************************/

static PXWND_DEF  s_wnd          = NULL;
static BOOL       s_fNoXServer   = FALSE;

#if 1
static int        s_updateMode   = XWC_SYNC_UPDATE;
#else
static int        s_updateMode   = XWC_ASYNC_UPDATE;
#endif
static int        s_iUpdateCounter;

static BOOL       s_fIgnoreErrors = FALSE;

/* *********************************************************************** */

static int s_errorHandler( Display * dpy, XErrorEvent * e )
{
   char errorText[ 1024 ];

   hb_strncpy( errorText, "Xlib error: ", sizeof( errorText ) - 1 );
   XGetErrorText( dpy, e->error_code, errorText + strlen( errorText ),
                  sizeof( errorText ) - strlen( errorText ) );

   if( ! s_fIgnoreErrors )
   {
      s_fNoXServer = TRUE;
      hb_errInternal( 10001, errorText, NULL, NULL );
   }

   fprintf( stderr, "%s\n", errorText );

   return 1;
}

/* *********************************************************************** */

static void hb_gt_xwc_SigHandler( int iSig )
{
   PXWND_DEF wnd = s_wnd;

   HB_SYMBOL_UNUSED( iSig );

   if( s_updateMode == XWC_ASYNC_UPDATE && wnd && wnd->fInit )
   {
      if( s_iUpdateCounter )
         --s_iUpdateCounter;
      hb_gt_xwc_ProcessMessages( wnd );
   }
}

/* *********************************************************************** */

static void hb_gt_xwc_Disable( void )
{
   if( s_updateMode == XWC_ASYNC_UPDATE )
   {
      signal( SIGALRM, SIG_IGN );
   }
}

/* *********************************************************************** */

static void hb_gt_xwc_Enable( void )
{
   if( s_updateMode == XWC_ASYNC_UPDATE )
   {
      struct itimerval itv;

      signal( SIGALRM, hb_gt_xwc_SigHandler );
      itv.it_interval.tv_sec  = 0;
      itv.it_interval.tv_usec = 25000;
      itv.it_value            = itv.it_interval;
      setitimer( ITIMER_REAL, &itv, NULL );
   }
}


/* *********************************************************************** */

/*
 *  functions for building character conversion and box chars shapes
 */

/* *********************************************************************** */

static BOOL hb_gt_xwc_DefineBoxChar( PXWND_DEF wnd, USHORT usCh, XWC_CharTrans * bxCh )
{
   XSegment       segs[ 9 ];
   XPoint         pts[ XWC_MAX_CHAR_POINTS ];
   XRectangle     rect[ 4 ];
   XWC_CharType   type     = CH_CHAR;
   int            size     = 0;
   BOOL           inverse  = FALSE;

   int            cellx    = wnd->fontWidth;
   int            celly    = wnd->fontHeight;
   int            i;

   switch( usCh )
   {
      case HB_GTXVG_FILLER1:
      case HB_GTXVG_FILLER2:
      case HB_GTXVG_FILLER3:
      {
         int x, y, xx, yy, skip, start, mod;

         if( usCh == HB_GTXVG_FILLER1 )
         {
            skip  = 4;
            start = mod = 1;
         }
         else if( usCh == HB_GTXVG_FILLER2 )
         {
            skip  = 2;
            start = 0;
            mod   = 1;
         }
         else
         {
            skip     = 4;
            start    = mod = 0;
            inverse  = TRUE;
         }
         xx = yy = 0;
         for( y = 0; y < celly; y++ )
         {
            for( x = start + ( skip >> 1 ) * ( ( y & 1 ) ^ mod ); x < cellx; x += skip )
            {
               /* this is font size dependent, we have to add this checking
                * to at least avoid GPF for if user set very large font though
                * character definition will not be finished
                */
               if( size >= XWC_MAX_CHAR_POINTS )
               {
                  break;
               }
               pts[ size ].x  = x - xx;
               pts[ size ].y  = y - yy;
               xx             = x;
               yy             = y;
               size++;
            }
         }
         type = CH_PTS;
         break;
      }

      case HB_GTXVG_ARROW_R:
         i           = HB_MIN( ( celly >> 1 ), cellx ) - 3;
         pts[ 0 ].x  = ( ( cellx - i ) >> 1 );
         pts[ 0 ].y  = ( celly >> 1 ) - i;
         pts[ 1 ].x  = i;
         pts[ 1 ].y  = i;
         pts[ 2 ].x  = -i;
         pts[ 2 ].y  = i;
         size        = 3;
         type        = CH_POLY;
         break;

      case HB_GTXVG_ARROW_L:
         i           = HB_MIN( ( celly >> 1 ), cellx ) - 3;
         pts[ 0 ].x  = ( ( cellx - i ) >> 1 ) + i;
         pts[ 0 ].y  = ( celly >> 1 ) - i;
         pts[ 1 ].x  = -i;
         pts[ 1 ].y  = i;
         pts[ 2 ].x  = i;
         pts[ 2 ].y  = i;
         size        = 3;
         type        = CH_POLY;
         break;

      case HB_GTXVG_ARROW_U:
         i           = HB_MIN( celly, cellx >> 1 );
         pts[ 0 ].x  = ( cellx >> 1 ) - i;
         pts[ 0 ].y  = ( ( celly - i ) >> 1 ) + i;
         pts[ 1 ].x  = i;
         pts[ 1 ].y  = -i;
         pts[ 2 ].x  = i;
         pts[ 2 ].y  = i;
         size        = 3;
         type        = CH_POLY;
         break;

      case HB_GTXVG_ARROW_D:
         i           = HB_MIN( celly, cellx >> 1 );
         pts[ 0 ].x  = ( cellx >> 1 ) - i;
         pts[ 0 ].y  = ( ( celly - i ) >> 1 );
         pts[ 1 ].x  = i;
         pts[ 1 ].y  = i;
         pts[ 2 ].x  = i;
         pts[ 2 ].y  = -i;
         size        = 3;
         type        = CH_POLY;
         break;

      case HB_GTXVG_FULL:
         inverse  = TRUE;
         type     = CH_NONE;
         break;

      case HB_GTXVG_FULL_B:
         inverse           = TRUE;
      case HB_GTXVG_FULL_T:
         rect[ 0 ].x       = 0;
         rect[ 0 ].y       = 0;
         rect[ 0 ].width   = cellx;
         rect[ 0 ].height  = celly / 2;
         size              = 1;
         type              = CH_RECT;
         break;

      case HB_GTXVG_FULL_R:
         inverse           = TRUE;
      case HB_GTXVG_FULL_L:
         rect[ 0 ].x       = 0;
         rect[ 0 ].y       = 0;
         rect[ 0 ].width   = cellx / 2;
         rect[ 0 ].height  = celly;
         size              = 1;
         type              = CH_RECT;
         break;

      case HB_GTXWC_SNG_LT:
         segs[ 0 ].x1   = cellx / 2;
         segs[ 0 ].y1   = celly - 1;
         segs[ 0 ].x2   = cellx / 2;
         segs[ 0 ].y2   = celly / 2;

         segs[ 1 ].x1   = segs[ 0 ].x2;
         segs[ 1 ].y1   = segs[ 0 ].y2;
         segs[ 1 ].x2   = cellx - 1;
         segs[ 1 ].y2   = celly / 2;

         size           = 2;
         type           = CH_SEG;
         break;

      case HB_GTXWC_SNG_TD:
         segs[ 0 ].x1   = 0;
         segs[ 0 ].y1   = celly / 2;
         segs[ 0 ].x2   = cellx - 1;
         segs[ 0 ].y2   = celly / 2;

         segs[ 1 ].x1   = cellx / 2;
         segs[ 1 ].y1   = segs[ 0 ].y1;
         segs[ 1 ].x2   = segs[ 1 ].x1;
         segs[ 1 ].y2   = celly - 1;

         size           = 2;
         type           = CH_SEG;
         break;

      case HB_GTXWC_SNG_RT:
         segs[ 0 ].x1   = cellx / 2;
         segs[ 0 ].y1   = celly - 1;
         segs[ 0 ].x2   = cellx / 2;
         segs[ 0 ].y2   = celly / 2;

         segs[ 1 ].x1   = segs[ 0 ].x2;
         segs[ 1 ].y1   = segs[ 0 ].y2;
         segs[ 1 ].x2   = 0;
         segs[ 1 ].y2   = celly / 2;

         size           = 2;
         type           = CH_SEG;
         break;

      case HB_GTXWC_SNG_LB:
         segs[ 0 ].x1   = cellx / 2;
         segs[ 0 ].y1   = 0;
         segs[ 0 ].x2   = cellx / 2;
         segs[ 0 ].y2   = celly / 2;

         segs[ 1 ].x1   = segs[ 0 ].x2;
         segs[ 1 ].y1   = segs[ 0 ].y2;
         segs[ 1 ].x2   = cellx - 1;
         segs[ 1 ].y2   = celly / 2;

         size           = 2;
         type           = CH_SEG;
         break;

      case HB_GTXWC_SNG_BU:
         segs[ 0 ].x1   = cellx / 2;
         segs[ 0 ].y1   = 0;
         segs[ 0 ].x2   = cellx / 2;
         segs[ 0 ].y2   = celly / 2;

         segs[ 1 ].x1   = 0;
         segs[ 1 ].y1   = segs[ 0 ].y2;
         segs[ 1 ].x2   = cellx - 1;
         segs[ 1 ].y2   = celly / 2;

         size           = 2;
         type           = CH_SEG;
         break;

      case HB_GTXWC_SNG_RB:
         segs[ 0 ].x1   = cellx / 2;
         segs[ 0 ].y1   = 0;
         segs[ 0 ].x2   = cellx / 2;
         segs[ 0 ].y2   = celly / 2;

         segs[ 1 ].x1   = segs[ 0 ].x2;
         segs[ 1 ].y1   = segs[ 0 ].y2;
         segs[ 1 ].x2   = 0;
         segs[ 1 ].y2   = celly / 2;

         size           = 2;
         type           = CH_SEG;
         break;

      case HB_GTXWC_SNG_VL:
         segs[ 0 ].x1   = cellx / 2;
         segs[ 0 ].y1   = 0;
         segs[ 0 ].x2   = segs[ 0 ].x1;
         segs[ 0 ].y2   = celly - 1;

         segs[ 1 ].x1   = segs[ 0 ].x1;
         segs[ 1 ].y1   = celly / 2;
         segs[ 1 ].x2   = cellx - 1;
         segs[ 1 ].y2   = segs[ 1 ].y1;

         size           = 2;
         type           = CH_SEG;
         break;

      case HB_GTXWC_SNG_VR:
         segs[ 0 ].x1   = cellx / 2;
         segs[ 0 ].y1   = 0;
         segs[ 0 ].x2   = segs[ 0 ].x1;
         segs[ 0 ].y2   = celly - 1;

         segs[ 1 ].x1   = segs[ 0 ].x1;
         segs[ 1 ].y1   = celly / 2;
         segs[ 1 ].x2   = 0;
         segs[ 1 ].y2   = segs[ 1 ].y1;

         size           = 2;
         type           = CH_SEG;
         break;

      case HB_GTXWC_SNG_CRS:
         segs[ 0 ].x1   = cellx / 2;
         segs[ 0 ].y1   = 0;
         segs[ 0 ].x2   = segs[ 0 ].x1;
         segs[ 0 ].y2   = celly - 1;

         segs[ 1 ].x1   = 0;
         segs[ 1 ].y1   = celly / 2;
         segs[ 1 ].x2   = cellx - 1;
         segs[ 1 ].y2   = segs[ 1 ].y1;

         size           = 2;
         type           = CH_SEG;
         break;

      case HB_GTXWC_SNG_HOR:
         segs[ 0 ].x1   = 0;
         segs[ 0 ].y1   = celly / 2;
         segs[ 0 ].x2   = cellx - 1;
         segs[ 0 ].y2   = segs[ 0 ].y1;

         size           = 1;
         type           = CH_SEG;
         break;

      case HB_GTXWC_SNG_VRT:
         segs[ 0 ].x1   = cellx / 2;
         segs[ 0 ].y1   = 0;
         segs[ 0 ].x2   = segs[ 0 ].x1;
         segs[ 0 ].y2   = celly - 1;

         size           = 1;
         type           = CH_SEG;
         break;

      case HB_GTXWC_DBL_LT:
         segs[ 0 ].x1   = cellx / 2 - 1;
         segs[ 0 ].y1   = celly - 1;
         segs[ 0 ].x2   = segs[ 0 ].x1;
         segs[ 0 ].y2   = celly / 2 - 1;

         segs[ 1 ].x1   = segs[ 0 ].x2;
         segs[ 1 ].y1   = segs[ 0 ].y2;
         segs[ 1 ].x2   = cellx - 1;
         segs[ 1 ].y2   = segs[ 0 ].y2;

         segs[ 2 ].x1   = cellx / 2 + 1;
         segs[ 2 ].y1   = celly - 1;
         segs[ 2 ].x2   = segs[ 2 ].x1;
         segs[ 2 ].y2   = celly / 2 + 1;

         segs[ 3 ].x1   = segs[ 2 ].x2;
         segs[ 3 ].y1   = segs[ 2 ].y2;
         segs[ 3 ].x2   = cellx - 1;
         segs[ 3 ].y2   = segs[ 2 ].y2;

         size           = 4;
         type           = CH_SEG;
         break;

      case HB_GTXWC_DBL_TD:
         segs[ 0 ].x1   = 0;
         segs[ 0 ].y1   = celly / 2 - 1;
         segs[ 0 ].x2   = cellx - 1;
         segs[ 0 ].y2   = segs[ 0 ].y1;

         segs[ 1 ].x1   = segs[ 0 ].x1;
         segs[ 1 ].y1   = celly / 2 + 1;
         segs[ 1 ].x2   = cellx / 2 - 1;
         segs[ 1 ].y2   = segs[ 1 ].y1;

         segs[ 2 ].x1   = cellx / 2 + 1;
         segs[ 2 ].y1   = celly / 2 + 1;
         segs[ 2 ].x2   = cellx - 1;
         segs[ 2 ].y2   = segs[ 2 ].y1;

         segs[ 3 ].x1   = segs[ 1 ].x2;
         segs[ 3 ].y1   = segs[ 1 ].y1;
         segs[ 3 ].x2   = segs[ 1 ].x2;
         segs[ 3 ].y2   = celly - 1;

         segs[ 4 ].x1   = segs[ 2 ].x1;
         segs[ 4 ].y1   = segs[ 2 ].y1;
         segs[ 4 ].x2   = segs[ 2 ].x1;
         segs[ 4 ].y2   = celly - 1;

         size           = 5;
         type           = CH_SEG;
         break;

      case HB_GTXWC_DBL_RT:
         segs[ 0 ].x1   = cellx / 2 - 1;
         segs[ 0 ].y1   = celly - 1;
         segs[ 0 ].x2   = segs[ 0 ].x1;
         segs[ 0 ].y2   = celly / 2 + 1;

         segs[ 1 ].x1   = segs[ 0 ].x2;
         segs[ 1 ].y1   = segs[ 0 ].y2;
         segs[ 1 ].x2   = 0;
         segs[ 1 ].y2   = segs[ 0 ].y2;

         segs[ 2 ].x1   = cellx / 2 + 1;
         segs[ 2 ].y1   = celly - 1;
         segs[ 2 ].x2   = segs[ 2 ].x1;
         segs[ 2 ].y2   = celly / 2 - 1;

         segs[ 3 ].x1   = segs[ 2 ].x2;
         segs[ 3 ].y1   = segs[ 2 ].y2;
         segs[ 3 ].x2   = 0;
         segs[ 3 ].y2   = segs[ 2 ].y2;

         size           = 4;
         type           = CH_SEG;
         break;

      case HB_GTXWC_DBL_LB:
         segs[ 0 ].x1   = cellx / 2 - 1;
         segs[ 0 ].y1   = 0;
         segs[ 0 ].x2   = segs[ 0 ].x1;
         segs[ 0 ].y2   = celly / 2 + 1;

         segs[ 1 ].x1   = segs[ 0 ].x2;
         segs[ 1 ].y1   = segs[ 0 ].y2;
         segs[ 1 ].x2   = cellx - 1;
         segs[ 1 ].y2   = segs[ 0 ].y2;

         segs[ 2 ].x1   = cellx / 2 + 1;
         segs[ 2 ].y1   = 0;
         segs[ 2 ].x2   = segs[ 2 ].x1;
         segs[ 2 ].y2   = celly / 2 - 1;

         segs[ 3 ].x1   = segs[ 2 ].x2;
         segs[ 3 ].y1   = segs[ 2 ].y2;
         segs[ 3 ].x2   = cellx - 1;
         segs[ 3 ].y2   = segs[ 2 ].y2;

         size           = 4;
         type           = CH_SEG;
         break;

      case HB_GTXWC_DBL_BU:
         segs[ 0 ].x1   = 0;
         segs[ 0 ].y1   = celly / 2 + 1;
         segs[ 0 ].x2   = cellx - 1;
         segs[ 0 ].y2   = segs[ 0 ].y1;

         segs[ 1 ].x1   = segs[ 0 ].x1;
         segs[ 1 ].y1   = celly / 2 - 1;
         segs[ 1 ].x2   = cellx / 2 - 1;
         segs[ 1 ].y2   = segs[ 1 ].y1;

         segs[ 2 ].x1   = cellx / 2 + 1;
         segs[ 2 ].y1   = celly / 2 - 1;
         segs[ 2 ].x2   = cellx - 1;
         segs[ 2 ].y2   = segs[ 2 ].y1;

         segs[ 3 ].x1   = segs[ 1 ].x2;
         segs[ 3 ].y1   = segs[ 1 ].y1;
         segs[ 3 ].x2   = segs[ 1 ].x2;
         segs[ 3 ].y2   = 0;

         segs[ 4 ].x1   = segs[ 2 ].x1;
         segs[ 4 ].y1   = segs[ 2 ].y1;
         segs[ 4 ].x2   = segs[ 2 ].x1;
         segs[ 4 ].y2   = 0;

         size           = 5;
         type           = CH_SEG;
         break;

      case HB_GTXWC_DBL_RB:
         segs[ 0 ].x1   = cellx / 2 - 1;
         segs[ 0 ].y1   = 0;
         segs[ 0 ].x2   = segs[ 0 ].x1;
         segs[ 0 ].y2   = celly / 2 - 1;

         segs[ 1 ].x1   = segs[ 0 ].x2;
         segs[ 1 ].y1   = segs[ 0 ].y2;
         segs[ 1 ].x2   = 0;
         segs[ 1 ].y2   = segs[ 0 ].y2;

         segs[ 2 ].x1   = cellx / 2 + 1;
         segs[ 2 ].y1   = 0;
         segs[ 2 ].x2   = segs[ 2 ].x1;
         segs[ 2 ].y2   = celly / 2 + 1;

         segs[ 3 ].x1   = segs[ 2 ].x2;
         segs[ 3 ].y1   = segs[ 2 ].y2;
         segs[ 3 ].x2   = 0;
         segs[ 3 ].y2   = segs[ 2 ].y2;

         size           = 4;
         type           = CH_SEG;
         break;

      case HB_GTXWC_DBL_VL:
         segs[ 0 ].x1   = cellx / 2 - 1;
         segs[ 0 ].y1   = 0;
         segs[ 0 ].x2   = segs[ 0 ].x1;
         segs[ 0 ].y2   = celly - 1;

         segs[ 1 ].x1   = cellx / 2 + 1;
         segs[ 1 ].y1   = 0;
         segs[ 1 ].x2   = segs[ 1 ].x1;
         segs[ 1 ].y2   = celly / 2 - 1;

         segs[ 2 ].x1   = segs[ 1 ].x1;
         segs[ 2 ].y1   = celly / 2 + 1;
         segs[ 2 ].x2   = segs[ 1 ].x1;
         segs[ 2 ].y2   = celly - 1;

         segs[ 3 ].x1   = segs[ 1 ].x1;
         segs[ 3 ].y1   = segs[ 1 ].y2;
         segs[ 3 ].x2   = cellx - 1;
         segs[ 3 ].y2   = segs[ 3 ].y1;

         segs[ 4 ].x1   = segs[ 2 ].x1;
         segs[ 4 ].y1   = segs[ 2 ].y1;
         segs[ 4 ].x2   = cellx - 1;
         segs[ 4 ].y2   = segs[ 2 ].y1;

         size           = 5;
         type           = CH_SEG;
         break;


      case HB_GTXWC_DBL_VR:
         segs[ 0 ].x1   = cellx / 2 + 1;
         segs[ 0 ].y1   = 0;
         segs[ 0 ].x2   = segs[ 0 ].x1;
         segs[ 0 ].y2   = celly - 1;

         segs[ 1 ].x1   = cellx / 2 - 1;
         segs[ 1 ].y1   = 0;
         segs[ 1 ].x2   = segs[ 1 ].x1;
         segs[ 1 ].y2   = celly / 2 - 1;

         segs[ 2 ].x1   = segs[ 1 ].x1;
         segs[ 2 ].y1   = celly / 2 + 1;
         segs[ 2 ].x2   = segs[ 1 ].x1;
         segs[ 2 ].y2   = celly - 1;

         segs[ 3 ].x1   = segs[ 1 ].x1;
         segs[ 3 ].y1   = segs[ 1 ].y2;
         segs[ 3 ].x2   = 0;
         segs[ 3 ].y2   = segs[ 3 ].y1;

         segs[ 4 ].x1   = segs[ 2 ].x1;
         segs[ 4 ].y1   = segs[ 2 ].y1;
         segs[ 4 ].x2   = 0;
         segs[ 4 ].y2   = segs[ 2 ].y1;

         size           = 5;
         type           = CH_SEG;
         break;

      case HB_GTXWC_DBL_CRS:
         segs[ 0 ].x1   = cellx / 2 - 1;
         segs[ 0 ].y1   = 0;
         segs[ 0 ].x2   = segs[ 0 ].x1;
         segs[ 0 ].y2   = celly / 2 - 1;

         segs[ 1 ].x1   = segs[ 0 ].x1;
         segs[ 1 ].y1   = celly / 2 + 1;
         segs[ 1 ].x2   = segs[ 0 ].x1;
         segs[ 1 ].y2   = celly - 1;

         segs[ 2 ].x1   = segs[ 0 ].x1;
         segs[ 2 ].y1   = segs[ 0 ].y2;
         segs[ 2 ].x2   = 0;
         segs[ 2 ].y2   = segs[ 2 ].y1;

         segs[ 3 ].x1   = segs[ 1 ].x1;
         segs[ 3 ].y1   = segs[ 1 ].y1;
         segs[ 3 ].x2   = 0;
         segs[ 3 ].y2   = segs[ 1 ].y1;

         segs[ 4 ].x1   = cellx / 2 + 1;
         segs[ 4 ].y1   = 0;
         segs[ 4 ].x2   = segs[ 4 ].x1;
         segs[ 4 ].y2   = celly / 2 - 1;

         segs[ 5 ].x1   = segs[ 4 ].x1;
         segs[ 5 ].y1   = celly / 2 + 1;
         segs[ 5 ].x2   = segs[ 4 ].x1;
         segs[ 5 ].y2   = celly - 1;

         segs[ 6 ].x1   = segs[ 4 ].x1;
         segs[ 6 ].y1   = segs[ 4 ].y2;
         segs[ 6 ].x2   = cellx - 1;
         segs[ 6 ].y2   = segs[ 6 ].y1;

         segs[ 7 ].x1   = segs[ 5 ].x1;
         segs[ 7 ].y1   = segs[ 5 ].y1;
         segs[ 7 ].x2   = cellx - 1;
         segs[ 7 ].y2   = segs[ 5 ].y1;

         size           = 8;
         type           = CH_SEG;
         break;

      case HB_GTXWC_DBL_HOR:
         segs[ 0 ].x1   = 0;
         segs[ 0 ].y1   = celly / 2 + 1;
         segs[ 0 ].x2   = cellx - 1;
         segs[ 0 ].y2   = segs[ 0 ].y1;

         segs[ 1 ].x1   = 0;
         segs[ 1 ].y1   = celly / 2 - 1;
         segs[ 1 ].x2   = cellx - 1;
         segs[ 1 ].y2   = segs[ 1 ].y1;

         size           = 2;
         type           = CH_SEG;
         break;

      case HB_GTXWC_DBL_VRT:
         segs[ 0 ].x1   = cellx / 2 - 1;
         segs[ 0 ].y1   = 0;
         segs[ 0 ].x2   = segs[ 0 ].x1;
         segs[ 0 ].y2   = celly - 1;

         segs[ 1 ].x1   = cellx / 2 + 1;
         segs[ 1 ].y1   = 0;
         segs[ 1 ].x2   = segs[ 1 ].x1;
         segs[ 1 ].y2   = celly - 1;

         size           = 2;
         type           = CH_SEG;
         break;

      case HB_GTXWC_SNG_L_DBL_T:
         segs[ 0 ].x1   = cellx / 2;
         segs[ 0 ].y1   = celly / 2 - 1;
         segs[ 0 ].x2   = cellx - 1;
         segs[ 0 ].y2   = segs[ 0 ].y1;

         segs[ 1 ].x1   = segs[ 0 ].x1;
         segs[ 1 ].y1   = celly / 2 + 1;
         segs[ 1 ].x2   = segs[ 0 ].x2;
         segs[ 1 ].y2   = segs[ 1 ].y1;

         segs[ 2 ].x1   = cellx / 2;
         segs[ 2 ].y1   = celly / 2 - 1;
         segs[ 2 ].x2   = segs[ 2 ].x1;
         segs[ 2 ].y2   = celly - 1;

         size           = 3;
         type           = CH_SEG;
         break;

      case HB_GTXWC_SNG_T_DBL_D:
         segs[ 0 ].x1   = 0;
         segs[ 0 ].y1   = celly / 2;
         segs[ 0 ].x2   = cellx - 1;
         segs[ 0 ].y2   = segs[ 0 ].y1;

         segs[ 1 ].x1   = cellx / 2 - 1;
         segs[ 1 ].y1   = celly / 2;
         segs[ 1 ].x2   = segs[ 1 ].x1;
         segs[ 1 ].y2   = celly - 1;

         segs[ 2 ].x1   = cellx / 2 + 1;
         segs[ 2 ].y1   = segs[ 1 ].y1;
         segs[ 2 ].x2   = segs[ 2 ].x1;
         segs[ 2 ].y2   = segs[ 1 ].y2;

         size           = 3;
         type           = CH_SEG;
         break;

      case HB_GTXWC_SNG_R_DBL_T:
         segs[ 0 ].x1   = 0;
         segs[ 0 ].y1   = celly / 2;
         segs[ 0 ].x2   = cellx / 2 + 1;
         segs[ 0 ].y2   = segs[ 0 ].y1;

         segs[ 1 ].x1   = cellx / 2 + 1;
         segs[ 1 ].y1   = segs[ 0 ].y1;
         segs[ 1 ].x2   = segs[ 0 ].x2;
         segs[ 1 ].y2   = celly - 1;

         segs[ 2 ].x1   = cellx / 2 - 1;
         segs[ 2 ].y1   = celly / 2;
         segs[ 2 ].x2   = segs[ 2 ].x1;
         segs[ 2 ].y2   = celly - 1;

         size           = 3;
         type           = CH_SEG;
         break;

      case HB_GTXWC_SNG_L_DBL_B:
         segs[ 0 ].x1   = cellx / 2;
         segs[ 0 ].y1   = celly / 2 - 1;
         segs[ 0 ].x2   = cellx - 1;
         segs[ 0 ].y2   = segs[ 0 ].y1;

         segs[ 1 ].x1   = segs[ 0 ].x1;
         segs[ 1 ].y1   = celly / 2 + 1;
         segs[ 1 ].x2   = segs[ 0 ].x2;
         segs[ 1 ].y2   = segs[ 1 ].y1;

         segs[ 2 ].x1   = cellx / 2;
         segs[ 2 ].y1   = 0;
         segs[ 2 ].x2   = segs[ 2 ].x1;
         segs[ 2 ].y2   = celly / 2 + 1;

         size           = 3;
         type           = CH_SEG;
         break;

      case HB_GTXWC_SNG_B_DBL_U:
         segs[ 0 ].x1   = 0;
         segs[ 0 ].y1   = celly / 2;
         segs[ 0 ].x2   = cellx - 1;
         segs[ 0 ].y2   = segs[ 0 ].y1;

         segs[ 1 ].x1   = cellx / 2 - 1;
         segs[ 1 ].y1   = 0;
         segs[ 1 ].x2   = segs[ 1 ].x1;
         segs[ 1 ].y2   = celly / 2;

         segs[ 2 ].x1   = cellx / 2 + 1;
         segs[ 2 ].y1   = 0;
         segs[ 2 ].x2   = segs[ 2 ].x1;
         segs[ 2 ].y2   = celly / 2;

         size           = 3;
         type           = CH_SEG;
         break;

      case HB_GTXWC_SNG_R_DBL_B:
         segs[ 0 ].x1   = 0;
         segs[ 0 ].y1   = celly / 2;
         segs[ 0 ].x2   = cellx / 2 + 1;
         segs[ 0 ].y2   = segs[ 0 ].y1;

         segs[ 1 ].x1   = cellx / 2 + 1;
         segs[ 1 ].y1   = 0;
         segs[ 1 ].x2   = segs[ 1 ].x1;
         segs[ 1 ].y2   = celly / 2;

         segs[ 2 ].x1   = cellx / 2 - 1;
         segs[ 2 ].y1   = 0;
         segs[ 2 ].x2   = segs[ 2 ].x1;
         segs[ 2 ].y2   = celly / 2;

         size           = 3;
         type           = CH_SEG;
         break;

      case HB_GTXWC_SNG_V_DBL_L:
         segs[ 0 ].x1   = cellx / 2;
         segs[ 0 ].y1   = 0;
         segs[ 0 ].x2   = segs[ 0 ].x1;
         segs[ 0 ].y2   = celly - 1;

         segs[ 1 ].x1   = segs[ 0 ].x1;
         segs[ 1 ].y1   = celly / 2 - 1;
         segs[ 1 ].x2   = cellx - 1;
         segs[ 1 ].y2   = segs[ 1 ].y1;

         segs[ 2 ].x1   = segs[ 0 ].x1;
         segs[ 2 ].y1   = celly / 2 + 1;
         segs[ 2 ].x2   = segs[ 1 ].x2;
         segs[ 2 ].y2   = segs[ 2 ].y1;

         size           = 3;
         type           = CH_SEG;
         break;

      case HB_GTXWC_SNG_V_DBL_R:
         segs[ 0 ].x1   = cellx / 2;
         segs[ 0 ].y1   = 0;
         segs[ 0 ].x2   = segs[ 0 ].x1;
         segs[ 0 ].y2   = celly - 1;

         segs[ 1 ].x1   = 0;
         segs[ 1 ].y1   = celly / 2 - 1;
         segs[ 1 ].x2   = segs[ 0 ].x1;
         segs[ 1 ].y2   = segs[ 1 ].y1;

         segs[ 2 ].x1   = 0;
         segs[ 2 ].y1   = celly / 2 + 1;
         segs[ 2 ].x2   = segs[ 0 ].x1;
         segs[ 2 ].y2   = segs[ 2 ].y1;

         size           = 3;
         type           = CH_SEG;
         break;

      case HB_GTXWC_SNG_DBL_CRS:
         segs[ 0 ].x1   = cellx / 2;
         segs[ 0 ].y1   = 0;
         segs[ 0 ].x2   = segs[ 0 ].x1;
         segs[ 0 ].y2   = celly - 1;

         segs[ 1 ].x1   = 0;
         segs[ 1 ].y1   = celly / 2 - 1;
         segs[ 1 ].x2   = cellx - 1;
         segs[ 1 ].y2   = segs[ 1 ].y1;

         segs[ 2 ].x1   = 0;
         segs[ 2 ].y1   = celly / 2 + 1;
         segs[ 2 ].x2   = segs[ 1 ].x2;
         segs[ 2 ].y2   = segs[ 2 ].y1;

         size           = 3;
         type           = CH_SEG;
         break;

      case HB_GTXWC_DBL_L_SNG_T:
         segs[ 0 ].x1   = cellx / 2 - 1;
         segs[ 0 ].y1   = celly / 2;
         segs[ 0 ].x2   = segs[ 0 ].x1;
         segs[ 0 ].y2   = celly - 1;

         segs[ 1 ].x1   = cellx / 2 + 1;
         segs[ 1 ].y1   = segs[ 0 ].y1;
         segs[ 1 ].x2   = segs[ 1 ].x1;
         segs[ 1 ].y2   = segs[ 0 ].y2;

         segs[ 2 ].x1   = segs[ 0 ].x1;
         segs[ 2 ].y1   = segs[ 0 ].y1;
         segs[ 2 ].x2   = cellx - 1;
         segs[ 2 ].y2   = segs[ 0 ].y1;

         size           = 3;
         type           = CH_SEG;
         break;

      case HB_GTXWC_DBL_T_SNG_D:
         segs[ 0 ].x1   = 0;
         segs[ 0 ].y1   = celly / 2 - 1;
         segs[ 0 ].x2   = cellx - 1;
         segs[ 0 ].y2   = segs[ 0 ].y1;

         segs[ 1 ].x1   = segs[ 0 ].x1;
         segs[ 1 ].y1   = celly / 2 + 1;
         segs[ 1 ].x2   = segs[ 0 ].x2;
         segs[ 1 ].y2   = segs[ 1 ].y1;

         segs[ 2 ].x1   = cellx / 2;
         segs[ 2 ].y1   = celly / 2 + 1;
         segs[ 2 ].x2   = segs[ 2 ].x1;
         segs[ 2 ].y2   = celly - 1;

         size           = 3;
         type           = CH_SEG;
         break;

      case HB_GTXWC_DBL_R_SNG_T:
         segs[ 0 ].x1   = 0;
         segs[ 0 ].y1   = celly / 2 - 1;
         segs[ 0 ].x2   = cellx / 2;
         segs[ 0 ].y2   = segs[ 0 ].y1;

         segs[ 1 ].x1   = segs[ 0 ].x1;
         segs[ 1 ].y1   = celly / 2 + 1;
         segs[ 1 ].x2   = segs[ 0 ].x2;
         segs[ 1 ].y2   = segs[ 1 ].y1;

         segs[ 2 ].x1   = cellx / 2;
         segs[ 2 ].y1   = celly / 2 - 1;
         segs[ 2 ].x2   = segs[ 2 ].x1;
         segs[ 2 ].y2   = celly - 1;

         size           = 3;
         type           = CH_SEG;
         break;

      case HB_GTXWC_DBL_L_SNG_B:
         segs[ 0 ].x1   = cellx / 2 - 1;
         segs[ 0 ].y1   = 0;
         segs[ 0 ].x2   = segs[ 0 ].x1;
         segs[ 0 ].y2   = celly / 2;

         segs[ 1 ].x1   = cellx / 2 + 1;
         segs[ 1 ].y1   = segs[ 0 ].y1;
         segs[ 1 ].x2   = segs[ 1 ].x1;
         segs[ 1 ].y2   = segs[ 0 ].y2;

         segs[ 2 ].x1   = cellx / 2 - 1;
         segs[ 2 ].y1   = segs[ 0 ].y2;
         segs[ 2 ].x2   = cellx - 1;
         segs[ 2 ].y2   = segs[ 0 ].y2;

         size           = 3;
         type           = CH_SEG;
         break;

      case HB_GTXWC_DBL_B_SNG_U:
         segs[ 0 ].x1   = 0;
         segs[ 0 ].y1   = celly / 2 - 1;
         segs[ 0 ].x2   = cellx - 1;
         segs[ 0 ].y2   = segs[ 0 ].y1;

         segs[ 1 ].x1   = segs[ 0 ].x1;
         segs[ 1 ].y1   = celly / 2 + 1;
         segs[ 1 ].x2   = segs[ 0 ].x2;
         segs[ 1 ].y2   = segs[ 1 ].y1;

         segs[ 2 ].x1   = cellx / 2;
         segs[ 2 ].y1   = 0;
         segs[ 2 ].x2   = segs[ 2 ].x1;
         segs[ 2 ].y2   = celly / 2 - 1;

         size           = 3;
         type           = CH_SEG;
         break;

      case HB_GTXWC_DBL_R_SNG_B:
         segs[ 0 ].x1   = 0;
         segs[ 0 ].y1   = celly / 2 - 1;
         segs[ 0 ].x2   = cellx / 2;
         segs[ 0 ].y2   = segs[ 0 ].y1;

         segs[ 1 ].x1   = segs[ 0 ].x1;
         segs[ 1 ].y1   = celly / 2 + 1;
         segs[ 1 ].x2   = segs[ 0 ].x2;
         segs[ 1 ].y2   = segs[ 1 ].y1;

         segs[ 2 ].x1   = cellx / 2;
         segs[ 2 ].y1   = 0;
         segs[ 2 ].x2   = segs[ 2 ].x1;
         segs[ 2 ].y2   = celly / 2 + 1;

         size           = 3;
         type           = CH_SEG;
         break;

      case HB_GTXWC_DBL_V_SNG_L:
         segs[ 0 ].x1   = cellx / 2 - 1;
         segs[ 0 ].y1   = 0;
         segs[ 0 ].x2   = segs[ 0 ].x1;
         segs[ 0 ].y2   = celly - 1;

         segs[ 1 ].x1   = cellx / 2 + 1;
         segs[ 1 ].y1   = segs[ 0 ].y1;
         segs[ 1 ].x2   = segs[ 1 ].x1;
         segs[ 1 ].y2   = segs[ 0 ].y2;

         segs[ 2 ].x1   = segs[ 1 ].x1;
         segs[ 2 ].y1   = celly / 2;
         segs[ 2 ].x2   = cellx - 1;
         segs[ 2 ].y2   = segs[ 2 ].y1;

         size           = 3;
         type           = CH_SEG;
         break;

      case HB_GTXWC_DBL_V_SNG_R:
         segs[ 0 ].x1   = cellx / 2 - 1;
         segs[ 0 ].y1   = 0;
         segs[ 0 ].x2   = segs[ 0 ].x1;
         segs[ 0 ].y2   = celly - 1;

         segs[ 1 ].x1   = cellx / 2 + 1;
         segs[ 1 ].y1   = segs[ 0 ].y1;
         segs[ 1 ].x2   = segs[ 1 ].x1;
         segs[ 1 ].y2   = segs[ 0 ].y2;

         segs[ 2 ].x1   = 0;
         segs[ 2 ].y1   = celly / 2;
         segs[ 2 ].x2   = segs[ 0 ].x1;
         segs[ 2 ].y2   = segs[ 2 ].y1;

         size           = 3;
         type           = CH_SEG;
         break;

      case HB_GTXWC_DBL_SNG_CRS:
         segs[ 0 ].x1   = cellx / 2 - 1;
         segs[ 0 ].y1   = 0;
         segs[ 0 ].x2   = segs[ 0 ].x1;
         segs[ 0 ].y2   = celly - 1;

         segs[ 1 ].x1   = cellx / 2 + 1;
         segs[ 1 ].y1   = segs[ 0 ].y1;
         segs[ 1 ].x2   = segs[ 1 ].x1;
         segs[ 1 ].y2   = segs[ 0 ].y2;

         segs[ 2 ].x1   = 0;
         segs[ 2 ].y1   = celly / 2;
         segs[ 2 ].x2   = cellx - 1;
         segs[ 2 ].y2   = segs[ 2 ].y1;

         size           = 3;
         type           = CH_SEG;
         break;

      case HB_GTXVG_SQUARE:
         rect[ 0 ].x       = ( ( cellx - rect[ 0 ].width ) >> 1 );
         rect[ 0 ].y       = ( ( celly - rect[ 0 ].height ) >> 1 );
         rect[ 0 ].width   = cellx - HB_MAX( cellx >> 2, 2 );
         rect[ 0 ].height  = rect[ 0 ].width;
         size              = 1;
         type              = CH_RECT;
         break;
/*
      default:
         rect[0].x = 1;
         rect[0].y = 1;
         rect[0].width = cellx - 2;
         rect[0].height = celly - 2;
         size = 1;
         type = CH_RECT;
         break;
 */
   }

   if( size > 0 )
   {
      bxCh->type     = type;
      bxCh->u.ch16   = usCh;
      bxCh->size     = size;
      bxCh->inverse  = inverse;
      switch( type )
      {
         case CH_SEG:
            bxCh->u.seg = ( XSegment * ) hb_xgrab( sizeof( XSegment ) * size );
            memcpy( bxCh->u.seg, segs, sizeof( XSegment ) * size );
            break;
         case CH_RECT:
            bxCh->u.rect = ( XRectangle * ) hb_xgrab( sizeof( XRectangle ) * size );
            memcpy( bxCh->u.rect, rect, sizeof( XRectangle ) * size );
            break;
         case CH_PTS:
         case CH_LINE:
         case CH_POLY:
            bxCh->u.pts = ( XPoint * ) hb_xgrab( sizeof( XPoint ) * size );
            memcpy( bxCh->u.pts, pts, sizeof( XPoint ) * size );
            break;
         default:
            break;
      }
      return TRUE;
   }
   return FALSE;
}

/* *********************************************************************** */

static void hb_gt_xwc_DestroyCharTrans( PXWND_DEF wnd )
{
   int i;

   for( i = 0; i < 256; i++ )
   {
      switch( wnd->charTrans[ i ].type )
      {
         case CH_IMG:
            XDestroyImage( wnd->charTrans[ i ].u.img );
            break;
         case CH_SEG:
            hb_xfree( wnd->charTrans[ i ].u.seg );
            break;
         case CH_RECT:
            hb_xfree( wnd->charTrans[ i ].u.rect );
            break;
         case CH_PTS:
         case CH_LINE:
         case CH_POLY:
            hb_xfree( wnd->charTrans[ i ].u.pts );
            break;
         default:
            break;
      }
      memset( &wnd->charTrans[ i ], 0, sizeof( XWC_CharTrans ) );
   }
}

/* *********************************************************************** */

static void hb_gt_xwc_BuildCharTrans( PXWND_DEF wnd )
{
   int      i;
   USHORT   usCh16;

   hb_gt_xwc_DestroyCharTrans( wnd );

   for( i = 0; i < 256; i++ )
   {
#ifndef HB_CDP_SUPPORT_OFF
      usCh16   = hb_cdpGetU16( wnd->hostCDP, TRUE, ( BYTE ) i );
#else
      int j;
      usCh16   = ( USHORT ) i;
      for( j = 0; j < ( int ) XWC_BOX_CHARS; j++ )
      {
         if( boxTranslate[ j ].c == usCh16 )
         {
            usCh16 = boxTranslate[ j ].u16;
            break;
         }
      }
#endif
      wnd->charTrans[ i ].type      = CH_CHAR;
      wnd->charTrans[ i ].u.ch16    = usCh16;
      wnd->charTrans[ i ].size      = 0;
      wnd->charTrans[ i ].inverse   = FALSE;
      if( wnd->fDrawBox )
      {
         hb_gt_xwc_DefineBoxChar( wnd, usCh16, &wnd->charTrans[ i ] );
      }
   }
}

/* *********************************************************************** */

/*
 *  functions for handling the input queues for the mouse and keyboard
 */

/* *********************************************************************** */

static void hb_gt_xwc_MouseInit( PXWND_DEF wnd )
{
   wnd->mouseNumButtons = XGetPointerMapping( wnd->dpy,
                                              wnd->mouseButtonsMap,
                                              XWC_MAX_BUTTONS );

   if( wnd->mouseNumButtons > XWC_MAX_BUTTONS )
   {
      wnd->mouseNumButtons = XWC_MAX_BUTTONS;
   }
   wnd->mouseButtonsState  = 0;
   wnd->mouseGotoCol       = -1;
   wnd->mouseGotoRow       = -1;
}

static void hb_gt_xwc_AddCharToInputQueue( PXWND_DEF wnd, int keyCode )
{
   if( wnd->keyBuffNO < XWC_CHAR_QUEUE_SIZE )
   {
      wnd->KeyBuff[ wnd->keyBuffPointer++ ] = keyCode;
      if( wnd->keyBuffPointer == XWC_CHAR_QUEUE_SIZE )
      {
         wnd->keyBuffPointer = 0;
      }
      wnd->keyBuffNO++;
   }
}

/* *********************************************************************** */

static BOOL hb_gt_xwc_GetCharFromInputQueue( PXWND_DEF wnd, int * keyCode )
{
   *keyCode = 0;
   if( wnd->keyBuffNO > 0 )
   {
      int index = wnd->keyBuffPointer - wnd->keyBuffNO;
      if( index < 0 )
      {
         index += XWC_CHAR_QUEUE_SIZE;
      }
      *keyCode = wnd->KeyBuff[ index ];
      wnd->keyBuffNO--;
      return TRUE;
   }
   return FALSE;
}

/* *********************************************************************** */

static int hb_gt_xwc_LastCharInInputQueue( PXWND_DEF wnd )
{
   if( wnd->keyBuffNO > 0 )
   {
      int index = wnd->keyBuffPointer - 1;
      if( index < 0 )
      {
         index += XWC_CHAR_QUEUE_SIZE;
      }
      return wnd->KeyBuff[ index ];
   }
   return 0;
}

/* *********************************************************************** */

static void hb_gt_xwc_TranslateKey( PXWND_DEF wnd, int key )
{
   const ClipKeyCode * clipKey = NULL;

   if( key >= K_SPACE && key <= K_CTRL_BS )
   {
      clipKey = &stdKeyTab[ key - K_SPACE ];
   }
   else if( key > 0 && key < K_SPACE && wnd->keyModifiers.bCtrl )
   {
      clipKey = &stdKeyTab[ key + '@' ];
   }
   else if( XWC_IS_EXTKEY( key ) )
   {
      clipKey = &extKeyTab[ XWC_CLR_KEYMASK( key ) ];
   }
   if( clipKey != NULL )
   {
      if( wnd->keyModifiers.bAlt )
      {
         key = clipKey->alt_key;
      }
      else if( wnd->keyModifiers.bCtrl )
      {
         key = clipKey->ctrl_key;
      }
      else if( wnd->keyModifiers.bShift )
      {
         key = clipKey->shift_key;
      }
      else
      {
         key = clipKey->key;
      }
   }
   if( key != 0 )
   {
      hb_gt_xwc_AddCharToInputQueue( wnd, key );
   }
}

/* *********************************************************************** */

static void hb_gt_xwc_ProcessKey( PXWND_DEF wnd, XKeyEvent * evt )
{
   unsigned char  buf[ 5 ];
   KeySym         outISO, out = XLookupKeysym( evt, 0 );
   int            ikey = 0, n, i;

#ifdef XWC_DEBUG
   n                       = XLookupString( evt, ( char * ) buf, sizeof( buf ), &outISO, NULL );
   buf[ HB_MAX( n, 0 ) ]   = '\0';
   printf( "KeySym=%lx, keySymISO=%lx, keystr=[%s]\r\n", out, outISO, buf ); fflush( stdout );
#endif

   /* First look for keys which should be processed before XLookupKeysym */
   switch( out )
   {
      /* First of all, let's scan for special codes */
      case XK_Shift_L:
      case XK_Shift_R:
         wnd->keyModifiers.bShift = TRUE;
         return;

      case XK_Control_L:
      case XK_Control_R:
         wnd->keyModifiers.bCtrl = TRUE;
         return;

      case XK_Meta_L:
      case XK_Alt_L:
         wnd->keyModifiers.bAlt = TRUE;
         return;

      case XK_Meta_R:
      case XK_Alt_R:
         wnd->keyModifiers.bAltGr = TRUE;
         return;

      /* Then we scan for movement */
      case XK_Left:
         ikey  = EXKEY_LEFT;
         break;
      case XK_Right:
         ikey  = EXKEY_RIGHT;
         break;
      case XK_Up:
         ikey  = EXKEY_UP;
         break;
      case XK_Down:
         ikey  = EXKEY_DOWN;
         break;
      /* case XK_Begin: case XK_KP_Begin: */
      case XK_Home:
         ikey  = EXKEY_HOME;
         break;
      case XK_End:
         ikey  = EXKEY_END;
         break;
      case XK_Page_Up:
         ikey  = EXKEY_PGUP;
         break;
      case XK_Page_Down:
         ikey  = EXKEY_PGDN;
         break;

      /* Special cursor operations */
      case XK_Delete:
         ikey  = EXKEY_DEL;
         break;
      case XK_Insert:
         ikey  = EXKEY_INS;
         break;
      case XK_BackSpace:
         ikey  = EXKEY_BS;
         break;
      case XK_Tab:
         ikey  = EXKEY_TAB;
         break;
      case XK_Linefeed:
      case XK_Return:
         ikey  = EXKEY_ENTER;
         break;
      case XK_KP_Enter:
         ikey  = EXKEY_KPENTER;
         break;
      case XK_Escape:
         ikey  = EXKEY_ESC;
         break;

      /* then we scan for function keys */
      case XK_F1:
         ikey  = EXKEY_F1;
         break;
      case XK_F2:
         ikey  = EXKEY_F2;
         break;
      case XK_F3:
         ikey  = EXKEY_F3;
         break;
      case XK_F4:
         ikey  = EXKEY_F4;
         break;
      case XK_F5:
         ikey  = EXKEY_F5;
         break;
      case XK_F6:
         ikey  = EXKEY_F6;
         break;
      case XK_F7:
         ikey  = EXKEY_F7;
         break;
      case XK_F8:
         ikey  = EXKEY_F8;
         break;
      case XK_F9:
         ikey  = EXKEY_F9;
         break;
      case XK_F10:
         ikey  = EXKEY_F10;
         break;
      case XK_F11:
         ikey  = EXKEY_F11;
         break;
      case XK_F12:
         ikey  = EXKEY_F12;
         break;

      /* Keys with special meanings to clipper */
      case XK_Pause:
         ikey  = EXKEY_PAUSE;
         break;
      case XK_Print:
         ikey  = EXKEY_PRTSCR;
         break;
   }
   if( ikey )
   {
      hb_gt_xwc_TranslateKey( wnd, ikey );
      return;
   }

   /* First check if there is no string bound with with a key, because
      we not check all modifiers in all possible keyboards */
   if( ( n = XLookupString( evt, ( char * ) buf, sizeof( buf ), &outISO, NULL ) ) <= 0 )
   {
      /*
       * This is a temporary hack for Latin-x input see gt_SetKeyCP
       */
      if( outISO >= 0x0100 && outISO <= 0x0fff && ( outISO & 0x80 ) == 0x80 )
      {
         buf[ 0 ] = ( BYTE ) ( outISO & 0xff );
         n        = 1;
      }
      if( outISO == 0x20ac )
      {
         buf[ 0 ] = ( BYTE ) 0x80;
         n        = 1;
      }
   }
   if( n > 0 )
   {
#ifndef HB_CDP_SUPPORT_OFF
      if( wnd->inCDP && wnd->hostCDP && wnd->inCDP != wnd->hostCDP )
      {
         hb_cdpnTranslate( ( char * ) buf, wnd->inCDP, wnd->hostCDP, n );
      }
#endif
#ifdef XWC_DEBUG
      buf[ n ] = '\0';
      printf( "keySymISO=%lx keystr=[%s]\r\n", outISO, buf ); fflush( stdout );
#endif
      for( i = 0; i < n; i++ )
      {
         if( wnd->keyModifiers.bAlt || wnd->keyModifiers.bCtrl )
         {
            hb_gt_xwc_TranslateKey( wnd, buf[ i ] );
         }
         else
         {
            hb_gt_xwc_AddCharToInputQueue( wnd, buf[ i ] );
         }
      }
      return;
   }

   /* look for special keys if they haven't been processed by XLookupString,
      mostly keypad keys */
   switch( out )
   {
      case XK_KP_Left:
         ikey  = EXKEY_LEFT;
         break;
      case XK_KP_Right:
         ikey  = EXKEY_RIGHT;
         break;
      case XK_KP_Up:
         ikey  = EXKEY_UP;
         break;
      case XK_KP_Down:
         ikey  = EXKEY_DOWN;
         break;
      case XK_KP_Home:
         ikey  = EXKEY_HOME;
         break;
      case XK_KP_End:
         ikey  = EXKEY_END;
         break;
      case XK_KP_Page_Up:
         ikey  = EXKEY_PGUP;
         break;
      case XK_KP_Page_Down:
         ikey  = EXKEY_PGDN;
         break;
      case XK_KP_Begin:
      case XK_KP_5:
         ikey  = EXKEY_CENTER;
         break;
      case XK_KP_Insert:
         ikey  = EXKEY_INS;
         break;
      case XK_KP_Delete:
         ikey  = EXKEY_DEL;
         break;
      case XK_KP_Enter:
         ikey  = EXKEY_KPENTER;
         break;
      case XK_KP_Multiply:
         ikey  = '*';
         break;
      case XK_KP_Add:
         ikey  = '+';
         break;
      case XK_KP_Subtract:
         ikey  = '-';
         break;
      case XK_KP_Divide:
         ikey  = '/';
         break;
   }
   if( ikey )
   {
      hb_gt_xwc_TranslateKey( wnd, ikey );
   }
}

/* *********************************************************************** */

static void hb_gt_xwc_WndProc( PXWND_DEF wnd, XEvent * evt )
{
   KeySym out;

   switch( evt->type )
   {
      case Expose:
#ifdef XWC_DEBUG
         printf( "Event: Expose\r\n" ); fflush( stdout );
#endif
         hb_gt_xwc_InvalidatePts( wnd,
                                  evt->xexpose.x, evt->xexpose.y,
                                  evt->xexpose.x + evt->xexpose.width,
                                  evt->xexpose.y + evt->xexpose.height );
         break;

      case NoExpose:
#ifdef XWC_DEBUG
         printf( "Event: NoExpose\r\n" ); fflush( stdout );
#endif
         break;

      case KeyPress:
#ifdef XWC_DEBUG
         printf( "Event: KeyPress\r\n" ); fflush( stdout );
#endif
         if( evt->xkey.time != CurrentTime )
            wnd->lastEventTime = evt->xkey.time;
         hb_gt_xwc_ProcessKey( wnd, &evt->xkey );
         break;

      case KeyRelease:
#ifdef XWC_DEBUG
         printf( "Event: KeyRelease\r\n" ); fflush( stdout );
#endif
         if( evt->xkey.time != CurrentTime )
            wnd->lastEventTime = evt->xkey.time;
         out = XLookupKeysym( &evt->xkey, 0 );
         switch( out )
         {
            /* First of all, let's scan for special codes */
            case XK_Shift_L: case XK_Shift_R:
               wnd->keyModifiers.bShift = FALSE;
               return;

            case XK_Control_L: case XK_Control_R:
               wnd->keyModifiers.bCtrl = FALSE;
               return;

            case XK_Meta_L: case XK_Alt_L:
               wnd->keyModifiers.bAlt = FALSE;
               return;

            case XK_Meta_R: case XK_Alt_R:
               wnd->keyModifiers.bAltGr = FALSE;
               return;
         }
         break;

      case MotionNotify:
#ifdef XWC_DEBUG
         printf( "Event: MotionNotify\r\n" ); fflush( stdout );
#endif
         if( evt->xmotion.time != CurrentTime )
            wnd->lastEventTime = evt->xmotion.time;

         wnd->mouseCol  = evt->xmotion.x / wnd->fontWidth;
         wnd->mouseRow  = evt->xmotion.y / wnd->fontHeight;
         if( hb_gt_xwc_LastCharInInputQueue( wnd ) != K_MOUSEMOVE )
         {
            hb_gt_xwc_AddCharToInputQueue( wnd, K_MOUSEMOVE );
         }
         break;

      case ButtonPress:
      case ButtonRelease:
      {
         int button = evt->xbutton.button - 1;

#ifdef XWC_DEBUG
         printf( "Event: %s, button=%d\r\n", evt->type == ButtonPress ? "ButtonPress" : "ButtonRelease", button ); fflush( stdout );
#endif
         if( evt->xbutton.time != CurrentTime )
            wnd->lastEventTime = evt->xbutton.time;
         if( button >= 0 && button < XWC_MAX_BUTTONS )
         {
            button = wnd->mouseButtonsMap[ button ] - 1;
         }
         if( button >= 0 && button < wnd->mouseNumButtons )
         {
            int key = 0;

            if( evt->type == ButtonPress )
            {
               Time evtTime = ( ( XButtonEvent * ) evt )->time;
               if( evtTime - wnd->mouseButtonsTime[ button ] <
                   ( Time ) HB_GTSELF_MOUSEGETDOUBLECLICKSPEED( wnd->pGT ) )
               {
                  key = mouseDblPressKeys[ button ];
               }
               else
               {
                  key = mousePressKeys[ button ];
               }
               wnd->mouseButtonsState           |= 1 << button;
               wnd->mouseButtonsTime[ button ]  = evtTime;
            }
            else
            {
               key                     = mouseReleaseKeys[ button ];
               wnd->mouseButtonsState  &= ~( 1 << button );
            }
            if( key != 0 )
            {
               hb_gt_xwc_AddCharToInputQueue( wnd, key );
            }
         }
         break;
      }

      case CreateNotify:
#ifdef XWC_DEBUG
         printf( "Event: CreateNotify\r\n" ); fflush( stdout );
#endif
         break;

      case MappingNotify:
#ifdef XWC_DEBUG
         printf( "Event: MappingNotify\r\n" ); fflush( stdout );
#endif
         XRefreshKeyboardMapping( &evt->xmapping );
         break;

      case FocusIn:
#ifdef XWC_DEBUG
         printf( "Event: FocusIn\r\n" ); fflush( stdout );
#endif
         XRefreshKeyboardMapping( &evt->xmapping );
         wnd->keyModifiers.bCtrl             =
            wnd->keyModifiers.bAlt           =
               wnd->keyModifiers.bAltGr      =
                  wnd->keyModifiers.bShift   = FALSE;
         break;

      case FocusOut:
#ifdef XWC_DEBUG
         printf( "Event: FocusOut\r\n" ); fflush( stdout );
#endif
         break;

      case ConfigureNotify:
#ifdef XWC_DEBUG
         printf( "Event: ConfigureNotify\r\n" ); fflush( stdout );
#endif
         wnd->newWidth     = evt->xconfigure.width;
         wnd->newHeight    = evt->xconfigure.height;
         wnd->fWinResize   = TRUE;
         break;

      case ClientMessage:
#ifdef XWC_DEBUG
         printf( "Event: ClientMessage:%ld (%s)\r\n", evt->xclient.data.l[ 0 ], XGetAtomName( wnd->dpy, ( Atom ) evt->xclient.data.l[ 0 ] ) ); fflush( stdout );
#endif
         if( ( Atom ) evt->xclient.data.l[ 0 ] == s_atomDelWin )
         {
            hb_vmRequestQuit();
         }
         break;

      case SelectionNotify:
      {
         Atom aNextRequest = None;
#ifdef XWC_DEBUG
         printf( "Event: SelectionNotify: selection=%ld (%s), property=%ld (%s), target=%ld (%s) => %ld (%s)\r\n",
                 evt->xselection.selection,
                 evt->xselection.selection == None ? "None" : XGetAtomName( wnd->dpy, evt->xselection.selection ),
                 evt->xselection.property,
                 evt->xselection.property == None ? "None" : XGetAtomName( wnd->dpy, evt->xselection.property ),
                 evt->xselection.target,
                 evt->xselection.target == None ? "None" : XGetAtomName( wnd->dpy, evt->xselection.target ),
                 wnd->ClipboardRequest,
                 wnd->ClipboardRequest == None ? "None" : XGetAtomName( wnd->dpy, wnd->ClipboardRequest ) ); fflush( stdout );
#endif
         if( evt->xselection.property != None )
         {
            XTextProperty  text;
            unsigned long  nItem;

            if( XGetTextProperty( wnd->dpy, wnd->window, &text,
                                  evt->xselection.property ) != 0 )
            {
#ifndef HB_CDP_SUPPORT_OFF
               if( evt->xselection.target == s_atomUTF8String && text.format == 8 )
               {
#ifdef XWC_DEBUG
                  printf( "UTF8String='%s'\r\n", text.value ); fflush( stdout );
#endif
                  nItem                = hb_cdpUTF8StringLength( text.value, text.nitems );
                  if( wnd->ClipboardData != NULL )
                     hb_xfree( wnd->ClipboardData );
                  wnd->ClipboardData   = ( unsigned char * ) hb_xgrab( nItem + 1 );
                  wnd->ClipboardSize   = nItem;
                  hb_cdpUTF8ToStrn( wnd->hostCDP, FALSE, text.value, text.nitems,
                                    wnd->ClipboardData, nItem + 1 );
                  wnd->ClipboardTime   = evt->xselection.time;
                  wnd->ClipboardRcvd   = TRUE;
               }
               else
#endif
               if( evt->xselection.target == s_atomString && text.format == 8 )
               {
#ifdef XWC_DEBUG
                  printf( "String='%s'\r\n", text.value ); fflush( stdout );
#endif
                  if( wnd->ClipboardData != NULL )
                     hb_xfree( wnd->ClipboardData );
                  wnd->ClipboardData = ( unsigned char * ) hb_xgrab( text.nitems + 1 );
                  memcpy( wnd->ClipboardData, text.value, text.nitems );
#ifndef HB_CDP_SUPPORT_OFF
                  if( wnd->inCDP && wnd->hostCDP && wnd->inCDP != wnd->hostCDP )
                     hb_cdpnTranslate( ( char * ) wnd->ClipboardData, wnd->inCDP, wnd->hostCDP, text.nitems );
#endif
                  wnd->ClipboardData[ text.nitems ]   = '\0';
                  wnd->ClipboardSize                  = text.nitems;
                  wnd->ClipboardTime                  = evt->xselection.time;
                  wnd->ClipboardRcvd                  = TRUE;
               }
               else if( evt->xselection.target == s_atomTargets && text.format == 32 )
               {
                  Atom aValue;
#ifdef XWC_DEBUG
                  printf( "text.nitems=%ld, text.format=%d\r\n", text.nitems, text.format ); fflush( stdout );
#endif
                  for( nItem = 0; nItem < text.nitems; ++nItem )
                  {
                     aValue = ( ( unsigned int * ) text.value )[ nItem ];
                     if( aValue == s_atomUTF8String )
                        aNextRequest = s_atomUTF8String;
                     else if( aValue == s_atomString && aNextRequest == None )
                        aNextRequest = s_atomString;
#ifdef XWC_DEBUG
                     printf( "%ld, %8lx (%s)\r\n", nItem, aValue, XGetAtomName( wnd->dpy, aValue ) ); fflush( stdout );
#endif
                  }
               }
            }
         }
         else if( wnd->ClipboardRequest == s_atomTargets )
         {
            aNextRequest = s_atomUTF8String;
         }
         else if( wnd->ClipboardRequest == s_atomUTF8String )
         {
            aNextRequest = s_atomString;
         }
         wnd->ClipboardRequest = aNextRequest;
         break;
      }

      case SelectionRequest:
      {
         XSelectionRequestEvent *   req = &evt->xselectionrequest;
         XEvent                     respond;

#ifdef XWC_DEBUG
         printf( "Event: SelectionRequest: %ld (%s)\r\n", req->target,
                 XGetAtomName( wnd->dpy, req->target ) ); fflush( stdout );
#endif
         respond.xselection.property = req->property;

         if( req->target == s_atomTimestamp )
         {
            XChangeProperty( wnd->dpy, req->requestor, req->property,
                             s_atomInteger, 8 * sizeof( Time ), PropModeReplace,
                             ( unsigned char * ) &wnd->ClipboardTime, 1 );
         }
         else if( req->target == s_atomTargets )
         {
            Atom aProp[] = { s_atomTimestamp,    s_atomTargets,
                             s_atomString,       s_atomUTF8String,
                             s_atomCompoundText, s_atomText };
            XChangeProperty( wnd->dpy, req->requestor, req->property,
                             s_atomAtom, 8 * sizeof( Atom ), PropModeReplace,
                             ( unsigned char * ) aProp, sizeof( aProp ) / sizeof( Atom ) );
         }
         else if( req->target == s_atomString )
         {
#ifndef HB_CDP_SUPPORT_OFF
            if( wnd->inCDP && wnd->hostCDP && wnd->inCDP != wnd->hostCDP )
            {
               BYTE * pBuffer = ( BYTE * ) hb_xgrab( wnd->ClipboardSize + 1 );
               memcpy( pBuffer, wnd->ClipboardData, wnd->ClipboardSize + 1 );
               hb_cdpnTranslate( ( char * ) pBuffer, wnd->hostCDP, wnd->inCDP, wnd->ClipboardSize );
               XChangeProperty( wnd->dpy, req->requestor, req->property,
                                s_atomString, 8, PropModeReplace,
                                pBuffer, wnd->ClipboardSize );
               hb_xfree( pBuffer );
            }
            else
#endif
            {
               XChangeProperty( wnd->dpy, req->requestor, req->property,
                                s_atomString, 8, PropModeReplace,
                                wnd->ClipboardData, wnd->ClipboardSize );
            }
         }
#ifndef HB_CDP_SUPPORT_OFF
         else if( req->target == s_atomUTF8String )
         {
            ULONG    ulLen    = hb_cdpStringInUTF8Length( wnd->hostCDP, FALSE, wnd->ClipboardData, wnd->ClipboardSize );
            BYTE *   pBuffer  = ( BYTE * ) hb_xgrab( ulLen + 1 );

            hb_cdpStrnToUTF8( wnd->hostCDP, FALSE, wnd->ClipboardData, wnd->ClipboardSize, pBuffer );
#ifdef XWC_DEBUG
            printf( "SelectionRequest: (%s)->(%s) [%s]\r\n", wnd->ClipboardData, pBuffer, wnd->hostCDP->id ); fflush( stdout );
#endif
            XChangeProperty( wnd->dpy, req->requestor, req->property,
                             s_atomUTF8String, 8, PropModeReplace,
                             pBuffer, ulLen );
            hb_xfree( pBuffer );
         }
#endif
         else
         {
            respond.xselection.property = None;
         }

         respond.xselection.type       = SelectionNotify;
         respond.xselection.display    = req->display;
         respond.xselection.requestor  = req->requestor;
         respond.xselection.selection  = req->selection;
         respond.xselection.target     = req->target;
         respond.xselection.time       = req->time;

         XSendEvent( wnd->dpy, req->requestor, 0, 0, &respond );
         break;
      }

      case SelectionClear:
#ifdef XWC_DEBUG
         printf( "Event: SelectionClear\r\n" ); fflush( stdout );
#endif
         wnd->ClipboardOwner = FALSE;
         break;

      case PropertyNotify:
#ifdef XWC_DEBUG
         printf( "Event: PropertyNotify\r\n" ); fflush( stdout );
#endif
         if( evt->xproperty.time != CurrentTime )
            wnd->lastEventTime = evt->xproperty.time;
         break;

#ifdef XWC_DEBUG
      default:
         printf( "Event: #%d\r\n", evt->type ); fflush( stdout );
         break;
#endif
   }
}

/* *********************************************************************** */

/*
 * functions for drawing on the virtual screen (pixmap) and window updating
 */

/* *********************************************************************** */
/* collor allocation */
static int hb_gt_xwc_GetColormapSize( PXWND_DEF wnd )
{
   XVisualInfo visInfo, * visInfoPtr;
   int         uiCMapSize = -1, nItems;

   visInfo.visualid  = XVisualIDFromVisual( DefaultVisual( wnd->dpy,
                                                           DefaultScreen( wnd->dpy ) ) );
   visInfoPtr        = XGetVisualInfo( wnd->dpy, ( long ) VisualIDMask,
                                       &visInfo, &nItems );
   if( nItems >= 1 )
   {
      uiCMapSize = visInfoPtr->colormap_size;
   }
   XFree( ( char * ) visInfoPtr );

   return uiCMapSize;
}

/* *********************************************************************** */

static BOOL hb_gt_xwc_AllocColor( PXWND_DEF wnd, XColor * pColor )
{
   BOOL  fOK = FALSE;
   int   uiCMapSize;

   if( XAllocColor( wnd->dpy, wnd->colors, pColor ) != 0 )
   {
      /* the exact color allocated */
      fOK = TRUE;
   }
   else if( ( uiCMapSize = hb_gt_xwc_GetColormapSize( wnd ) ) > 0 )
   {
      /* try to find the best approximation of chosen color in
       * already allocated colors
       * Based on xterm "find_closest_color()" which was based on
       * Monish Shah's "find_closest_color()" for Vim 6.0, modified
       * with ideas from David Tong's "noflash" library ;-)
       */
      int      i, uiClosestColor;
      double   dDiff, dDistance, dClosestColorDist = 0;
      XColor * colorTable;
      BYTE *   checkTable;

      colorTable  = ( XColor * ) hb_xgrab( uiCMapSize * sizeof( XColor ) );
      checkTable  = ( BYTE * ) hb_xgrab( uiCMapSize * sizeof( BYTE ) );
      for( i = 0; i < uiCMapSize; i++ )
      {
         colorTable[ i ].pixel   = ( HB_GT_PIXELTYPE ) i;
         checkTable[ i ]         = FALSE;
      }
      XQueryColors( wnd->dpy, wnd->colors, colorTable, uiCMapSize );

      do
      {
         uiClosestColor    = -1;
         /*
          * Look for the color that best approximates the desired one
          * and has not been checked so far and try to allocate it.
          * If allocation fails, it must mean that the color was read-write
          * (so we can't use it, since its owner might change it) or else
          * it was already freed. Repeat until something succeeds or
          * we test all colors in given maximum of approximation.
          *
          * set the maximum for accepted approximation,
          * now we accept any valid color MAX_INT * MAX_INT * 3 < 1e20
          */
         dClosestColorDist = 1e20;
         for( i = 0; i < uiCMapSize; i++ )
         {
            if( ! checkTable[ uiClosestColor ] )
            {
               /*
                * Use Euclidean distance in RGB space, weighted by Y (of YIQ)
                * as the objective function, this accounts for differences
                * in the color sensitivity of the eye.
                */
               dDiff       = 0.30 * ( ( ( int ) pColor->red ) - ( int ) colorTable[ i ].red );
               dDistance   = dDiff * dDiff;
               dDiff       = 0.61 * ( ( ( int ) pColor->green ) - ( int ) colorTable[ i ].green );
               dDistance   += dDiff * dDiff;
               dDiff       = 0.11 * ( ( ( int ) pColor->blue ) - ( int ) colorTable[ i ].blue );
               dDistance   += dDiff * dDiff;
               if( dDistance < dClosestColorDist )
               {
                  uiClosestColor    = i;
                  dClosestColorDist = dDistance;
               }
            }
         }
         if( uiClosestColor > 0 )
         {
            if( XAllocColor( wnd->dpy, wnd->colors, &colorTable[ uiClosestColor ] ) != 0 )
            {
               *pColor  = colorTable[ uiClosestColor ];
               fOK      = TRUE;
               break;
            }
            checkTable[ uiClosestColor ] = TRUE;
         }
      }
      while( uiClosestColor > 0 );

      hb_xfree( colorTable );
      hb_xfree( checkTable );
   }

   return fOK;
}

/* *********************************************************************** */

static void hb_gt_xwc_DrawString( PXWND_DEF wnd, int col, int row, BYTE color, USHORT * usChBuf, int len )
{
   if( wnd->fClearBkg )
   {
      XSetForeground( wnd->dpy, wnd->gc, wnd->pixels[ color >> 4 ] );
      XFillRectangle( wnd->dpy, wnd->drw, wnd->gc,
                      col * wnd->fontWidth, row * wnd->fontHeight,
                      wnd->fontWidth * len, wnd->fontHeight );
      XSetForeground( wnd->dpy, wnd->gc, wnd->pixels[ color & 0x0F ] );
      XDrawString16( wnd->dpy, wnd->drw, wnd->gc,
                     col * wnd->fontWidth,
                     row * wnd->fontHeight + wnd->xfs->ascent,
                     ( XChar2b * ) usChBuf, len );
   }
   else
   {
      XSetBackground( wnd->dpy, wnd->gc, wnd->pixels[ color >> 4 ] );
      XSetForeground( wnd->dpy, wnd->gc, wnd->pixels[ color & 0x0F ] );
      XDrawImageString16( wnd->dpy, wnd->drw, wnd->gc,
                          col * wnd->fontWidth,
                          row * wnd->fontHeight + wnd->xfs->ascent,
                          ( XChar2b * ) usChBuf, len );
   }
}

/* *********************************************************************** */

static ULONG hb_gt_xwc_HashCurrChar( BYTE attr, BYTE color, USHORT chr )
{
   return ( ( ULONG ) attr << 24 ) | ( ( ULONG ) color << 16 ) | ( ULONG ) chr;
}

/* *********************************************************************** */

static void hb_gt_xwc_RepaintChar( PXWND_DEF wnd, int colStart, int rowStart, int colStop, int rowStop )
{
   USHORT   irow, icol, index, startCol = 0, len, basex, basey, nsize;
   BYTE     oldColor = 0, color, attr;
   USHORT   usCh16, usChBuf[ XWC_MAX_COLS ];
   ULONG    ulCurr   = 0xFFFFFFFFL;
   int      i;

#ifdef XWC_DEBUG
   printf( "Repaint(%d,%d,%d,%d)[%dx%d]\r\n", rowStart, colStart, rowStop, colStop, wnd->fontHeight, wnd->fontWidth ); fflush( stdout );
#endif

   if( rowStop >= wnd->rows )
      rowStop = wnd->rows - 1;
   if( colStop >= wnd->cols )
      colStop = wnd->cols - 1;
   if( colStart < 0 )
      colStart = 0;
   if( rowStart < 0 )
      rowStart = 0;

   for( irow = rowStart; irow <= rowStop; irow++ )
   {
      icol  = colStart;
      index = icol + irow * wnd->cols;
      len   = 0;
      /* attribute may change mid line...
       * so buffer up text with same attrib, and output it
       * then do next section with same attrib, etc
       */
      while( icol <= colStop )
      {
         HB_GTSELF_GETSCRCHAR( wnd->pGT, irow, icol, &color, &attr, &usCh16 );
         ulCurr = hb_gt_xwc_HashCurrChar( attr, color, usCh16 );
         if( wnd->charTrans[ usCh16 ].inverse )
         {
            color = ( color << 4 ) | ( color >> 4 );
         }
         if( len > 0 && ( wnd->charTrans[ usCh16 ].type != CH_CHAR ||
                          color != oldColor || ulCurr == wnd->pCurrScr[ index ] ) )
         {
            hb_gt_xwc_DrawString( wnd, startCol, irow, oldColor, usChBuf, len );
            len = 0;
         }
         if( wnd->pCurrScr[ index ] != ulCurr )
         {
            switch( wnd->charTrans[ usCh16 ].type )
            {
               case CH_CHAR:
                  if( wnd->fFixMetric )
                  {
                     HB_PUT_BE_UINT16( &usChBuf[ 0 ], wnd->charTrans[ usCh16 ].u.ch16 );
                     hb_gt_xwc_DrawString( wnd, icol, irow, color, usChBuf, 1 );
                  }
                  else
                  {
                     if( len == 0 )
                     {
                        oldColor = color;
                        startCol = icol;
                     }
                     HB_PUT_BE_UINT16( &usChBuf[ len ], wnd->charTrans[ usCh16 ].u.ch16 );
                     len++;
                  }
                  break;

               case CH_CHBX:
                  /* hb_gt_xwc_DrawBoxChar( wnd, icol, irow, wnd->charTrans[ usCh16 ].u.ch16, color ); */
                  break;

               case CH_NONE:
                  XSetForeground( wnd->dpy, wnd->gc, wnd->pixels[ color >> 4 ] );
                  XFillRectangle( wnd->dpy, wnd->drw, wnd->gc,
                                  icol * wnd->fontWidth, irow * wnd->fontHeight,
                                  wnd->fontWidth, wnd->fontHeight );
                  break;

               case CH_IMG:
                  XSetBackground( wnd->dpy, wnd->gc, wnd->pixels[ color >> 4 ] );
                  XSetForeground( wnd->dpy, wnd->gc, wnd->pixels[ color & 0x0F ] );
                  XPutImage( wnd->dpy, wnd->drw, wnd->gc,
                             wnd->charTrans[ usCh16 ].u.img, 0, 0,
                             icol * wnd->fontWidth, irow * wnd->fontHeight,
                             wnd->fontWidth, wnd->fontHeight );
                  break;

               case CH_PTS:
                  /* we use CoordModePrevious so only first point posiotion has to be updated */
                  wnd->charTrans[ usCh16 ].u.pts[ 0 ].x  = ( wnd->charTrans[ usCh16 ].u.pts[ 0 ].x % wnd->fontWidth ) + icol * wnd->fontWidth;
                  wnd->charTrans[ usCh16 ].u.pts[ 0 ].y  = ( wnd->charTrans[ usCh16 ].u.pts[ 0 ].y % wnd->fontHeight ) + irow * wnd->fontHeight;
                  XSetForeground( wnd->dpy, wnd->gc, wnd->pixels[ color >> 4 ] );
                  XFillRectangle( wnd->dpy, wnd->drw, wnd->gc,
                                  icol * wnd->fontWidth, irow * wnd->fontHeight,
                                  wnd->fontWidth, wnd->fontHeight );
                  XSetForeground( wnd->dpy, wnd->gc, wnd->pixels[ color & 0x0F ] );
                  XDrawPoints( wnd->dpy, wnd->drw, wnd->gc,
                               wnd->charTrans[ usCh16 ].u.pts,
                               wnd->charTrans[ usCh16 ].size, CoordModePrevious );
                  break;

               case CH_LINE:
                  /* we use CoordModePrevious so only first point posiotion has to be updated */
                  wnd->charTrans[ usCh16 ].u.pts[ 0 ].x  = ( wnd->charTrans[ usCh16 ].u.pts[ 0 ].x % wnd->fontWidth ) + icol * wnd->fontWidth;
                  wnd->charTrans[ usCh16 ].u.pts[ 0 ].y  = ( wnd->charTrans[ usCh16 ].u.pts[ 0 ].y % wnd->fontHeight ) + irow * wnd->fontHeight;
                  XSetForeground( wnd->dpy, wnd->gc, wnd->pixels[ color >> 4 ] );
                  XFillRectangle( wnd->dpy, wnd->drw, wnd->gc,
                                  icol * wnd->fontWidth, irow * wnd->fontHeight,
                                  wnd->fontWidth, wnd->fontHeight );
                  XSetForeground( wnd->dpy, wnd->gc, wnd->pixels[ color & 0x0F ] );
                  XDrawLines( wnd->dpy, wnd->drw, wnd->gc,
                              wnd->charTrans[ usCh16 ].u.pts,
                              wnd->charTrans[ usCh16 ].size, CoordModePrevious );
                  break;

               case CH_POLY:
                  /* we use CoordModePrevious so only first point posiotion has to be updated */
                  wnd->charTrans[ usCh16 ].u.pts[ 0 ].x  = ( wnd->charTrans[ usCh16 ].u.pts[ 0 ].x % wnd->fontWidth ) + icol * wnd->fontWidth;
                  wnd->charTrans[ usCh16 ].u.pts[ 0 ].y  = ( wnd->charTrans[ usCh16 ].u.pts[ 0 ].y % wnd->fontHeight ) + irow * wnd->fontHeight;
                  XSetForeground( wnd->dpy, wnd->gc, wnd->pixels[ color >> 4 ] );
                  XFillRectangle( wnd->dpy, wnd->drw, wnd->gc,
                                  icol * wnd->fontWidth, irow * wnd->fontHeight,
                                  wnd->fontWidth, wnd->fontHeight );
                  XSetForeground( wnd->dpy, wnd->gc, wnd->pixels[ color & 0x0F ] );
                  XFillPolygon( wnd->dpy, wnd->drw, wnd->gc,
                                wnd->charTrans[ usCh16 ].u.pts,
                                wnd->charTrans[ usCh16 ].size,
                                Convex, CoordModePrevious );
                  break;

               case CH_SEG:
                  basex = icol * wnd->fontWidth;
                  basey = irow * wnd->fontHeight;
                  nsize = wnd->charTrans[ usCh16 ].size;
                  for( i = 0; i < nsize; i++ )
                  {
                     wnd->charTrans[ usCh16 ].u.seg[ i ].x1 = ( wnd->charTrans[ usCh16 ].u.seg[ i ].x1 % wnd->fontWidth ) + basex;
                     wnd->charTrans[ usCh16 ].u.seg[ i ].y1 = ( wnd->charTrans[ usCh16 ].u.seg[ i ].y1 % wnd->fontHeight ) + basey;
                     wnd->charTrans[ usCh16 ].u.seg[ i ].x2 = ( wnd->charTrans[ usCh16 ].u.seg[ i ].x2 % wnd->fontWidth ) + basex;
                     wnd->charTrans[ usCh16 ].u.seg[ i ].y2 = ( wnd->charTrans[ usCh16 ].u.seg[ i ].y2 % wnd->fontHeight ) + basey;
                  }
                  XSetForeground( wnd->dpy, wnd->gc, wnd->pixels[ color >> 4 ] );
                  XFillRectangle( wnd->dpy, wnd->drw, wnd->gc,
                                  basex, basey, wnd->fontWidth, wnd->fontHeight );
                  XSetForeground( wnd->dpy, wnd->gc, wnd->pixels[ color & 0x0F ] );
                  XDrawSegments( wnd->dpy, wnd->drw, wnd->gc,
                                 wnd->charTrans[ usCh16 ].u.seg, nsize );
                  break;

               case CH_RECT:
                  basex = icol * wnd->fontWidth;
                  basey = irow * wnd->fontHeight;
                  nsize = wnd->charTrans[ usCh16 ].size;
                  for( i = 0; i < nsize; i++ )
                  {
                     wnd->charTrans[ usCh16 ].u.rect[ i ].x = ( wnd->charTrans[ usCh16 ].u.rect[ i ].x % wnd->fontWidth ) + basex;
                     wnd->charTrans[ usCh16 ].u.rect[ i ].y = ( wnd->charTrans[ usCh16 ].u.rect[ i ].y % wnd->fontHeight ) + basey;
                  }
                  XSetForeground( wnd->dpy, wnd->gc, wnd->pixels[ color >> 4 ] );
                  XFillRectangle( wnd->dpy, wnd->drw, wnd->gc,
                                  basex, basey, wnd->fontWidth, wnd->fontHeight );
                  XSetForeground( wnd->dpy, wnd->gc, wnd->pixels[ color & 0x0F ] );
                  XFillRectangles( wnd->dpy, wnd->drw, wnd->gc,
                                   wnd->charTrans[ usCh16 ].u.rect, nsize );
                  break;

            }
            wnd->pCurrScr[ index ] = ulCurr;
         }
         icol++;
         index++;
      }
      if( len > 0 )
      {
         hb_gt_xwc_DrawString( wnd, startCol, irow, oldColor, usChBuf, len );
      }
   }
}

/* *********************************************************************** */

static void hb_gt_xwc_RestoreArea( PXWND_DEF wnd,
                                   int left, int top, int right, int bottom )
{
   XCopyArea( wnd->dpy, wnd->pm, wnd->window, wnd->gc,
              wnd->fontWidth * left, wnd->fontHeight * top,
              wnd->fontWidth * ( right - left + 1 ), wnd->fontHeight * ( bottom - top + 1 ),
              wnd->fontWidth * left, wnd->fontHeight * top );
}

/* *********************************************************************** */

static void hb_gt_xwc_InvalidateChar( PXWND_DEF wnd,
                                      int left, int top, int right, int bottom )
{
   if( wnd->fInvalidChr == FALSE )
   {
      wnd->rInvalidChr.top    = top;
      wnd->rInvalidChr.left   = left;
      wnd->rInvalidChr.bottom = bottom;
      wnd->rInvalidChr.right  = right;
   }
   else
   {
      if( wnd->rInvalidChr.top > top )
         wnd->rInvalidChr.top = top;
      if( wnd->rInvalidChr.left > left )
         wnd->rInvalidChr.left = left;
      if( wnd->rInvalidChr.right < right )
         wnd->rInvalidChr.right = right;
      if( wnd->rInvalidChr.bottom < bottom )
         wnd->rInvalidChr.bottom = bottom;
   }
   /*
    * It's a race condition in async update mode.
    * wnd->fInvalidChr has to be set _always_ after update region is defined
    * to make sure that screen will be updated (sometimes maybe twice but
    * it shouldn't hurt us)
    */
   wnd->fInvalidChr = TRUE;
}

/* *********************************************************************** */

static void hb_gt_xwc_InvalidatePts( PXWND_DEF wnd,
                                     int left, int top, int right, int bottom )
{
   if( ! wnd->fInvalidPts )
   {
      wnd->rInvalidPts.top    = top;
      wnd->rInvalidPts.left   = left;
      wnd->rInvalidPts.bottom = bottom;
      wnd->rInvalidPts.right  = right;
   }
   else
   {
      if( wnd->rInvalidPts.top > top )
         wnd->rInvalidPts.top = top;
      if( wnd->rInvalidPts.left > left )
         wnd->rInvalidPts.left = left;
      if( wnd->rInvalidPts.right < right )
         wnd->rInvalidPts.right = right;
      if( wnd->rInvalidPts.bottom < bottom )
         wnd->rInvalidPts.bottom = bottom;
   }
   wnd->fInvalidPts = TRUE;
}

/* *********************************************************************** */

static void hb_gt_xwc_UpdateCursor( PXWND_DEF wnd )
{
   int cursorType = wnd->cursorState ? wnd->cursorType : SC_NONE;

   /* must the mouse cursor be positioned? */
   if( wnd->mouseGotoRow >= 0 && wnd->mouseGotoCol >= 0 )
   {
      XWarpPointer( wnd->dpy, None, wnd->window, 0, 0, 0, 0,
                    wnd->mouseGotoCol * wnd->fontWidth + ( wnd->fontWidth >> 1 ),
                    wnd->mouseGotoRow * wnd->fontHeight + ( wnd->fontHeight >> 1 ) );
      wnd->mouseGotoRow = -1;
   }

   /* must the screen cursor be repainted? */
   if( cursorType != wnd->lastCursorType ||
       wnd->lastCursorCol != wnd->col || wnd->lastCursorRow != wnd->row )
   {
      if( wnd->lastCursorType != SC_NONE )
      {
         /* restore character under previous cursor position */
         hb_gt_xwc_RestoreArea( wnd, wnd->lastCursorCol, wnd->lastCursorRow,
                                wnd->lastCursorCol, wnd->lastCursorRow );
      }
      if( cursorType != SC_NONE )
      {
         USHORT basex   = wnd->col * wnd->fontWidth,
                basey   = wnd->row * wnd->fontHeight,
                size;

         switch( cursorType )
         {
            case SC_NORMAL:
               size  = 2;
               basey += wnd->fontHeight - 3;
               break;
            case SC_INSERT:
               size  = ( wnd->fontHeight - 2 ) >> 1;
               basey += wnd->fontHeight - size - 1;
               break;
            case SC_SPECIAL1:
               size  = wnd->fontHeight - 2;
               basey += 1;
               break;
            case SC_SPECIAL2:
               size  = ( wnd->fontHeight - 2 ) >> 1;
               basey += 1;
               break;
            default:
               size  = 0;
               break;
         }
         if( size )
         {
            BYTE     color, attr;
            USHORT   usChar;

            HB_GTSELF_GETSCRCHAR( wnd->pGT, wnd->row, wnd->col, &color, &attr, &usChar );
            XSetForeground( wnd->dpy, wnd->gc, wnd->pixels[ color & 0x0f ] );
            XFillRectangle( wnd->dpy, wnd->window, wnd->gc,
                            basex, basey, wnd->fontWidth, size );
         }
      }
      wnd->lastCursorType  = cursorType;
      wnd->lastCursorCol   = wnd->col;
      wnd->lastCursorRow   = wnd->row;
   }
}

/* *********************************************************************** */

static void hb_gt_xwc_UpdatePts( PXWND_DEF wnd )
{
   if( wnd->fInvalidPts )
   {
      int left, top, right, bottom;

      left              = wnd->rInvalidPts.left;
      top               = wnd->rInvalidPts.top;
      right             = wnd->rInvalidPts.right;
      bottom            = wnd->rInvalidPts.bottom;
      wnd->fInvalidPts  = FALSE;
      XCopyArea( wnd->dpy, wnd->pm, wnd->window, wnd->gc,
                 left, top, right - left + 1, bottom - top + 1, left, top );

      /* if we've just overwritten the cursor then set it last state as SC_NONE */
      if( wnd->lastCursorType != SC_NONE )
      {
         int col  = wnd->lastCursorCol * wnd->fontWidth,
             row  = wnd->lastCursorRow * wnd->fontHeight;
         if( left <= col && top <= row && right >= col && bottom >= row )
         {
            wnd->lastCursorType = SC_NONE;
         }
      }
   }
}

/* *********************************************************************** */

static void hb_gt_xwc_UpdateChr( PXWND_DEF wnd )
{
   if( wnd->fInvalidChr )
   {
      int left, top, right, bottom;

      left              = wnd->rInvalidChr.left;
      top               = wnd->rInvalidChr.top;
      right             = wnd->rInvalidChr.right;
      bottom            = wnd->rInvalidChr.bottom;
      wnd->fInvalidChr  = FALSE;

      hb_gt_xwc_RepaintChar( wnd, left, top, right, bottom );

      left     = wnd->fontWidth * left;
      top      = wnd->fontHeight * top;
      right    = wnd->fontWidth * ( right + 1 ) - 1;
      bottom   = wnd->fontHeight * ( bottom + 1 ) - 1;

      hb_gt_xwc_InvalidatePts( wnd, left, top, right, bottom );
   }
}

/* *********************************************************************** */

static void hb_gt_xwc_UpdateSize( PXWND_DEF wnd )
{
   if( wnd->fWinResize )
   {
      wnd->fWinResize = FALSE;
      HB_GTSELF_SETMODE( wnd->pGT, wnd->newHeight / wnd->fontHeight,
                         wnd->newWidth / wnd->fontWidth );
   }
}

/* *********************************************************************** */

static ULONG hb_gt_xwc_CurrentTime( void )
{
   struct timeval tv;

   gettimeofday( &tv, NULL );
   return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

/* *********************************************************************** */

static void hb_gt_xwc_ProcessMessages( PXWND_DEF wnd )
{
   if( wnd->cursorType != SC_NONE )
   {
      if( wnd->cursorBlinkRate == 0 )
      {
         wnd->cursorState = TRUE;
      }
      else
      {
         ULONG ulCurrentTime = hb_gt_xwc_CurrentTime();

         if( ulCurrentTime - wnd->cursorStateTime > wnd->cursorBlinkRate )
         {
            wnd->cursorState     = ! wnd->cursorState;
            wnd->cursorStateTime = ulCurrentTime;
         }
      }
   }

   HB_XWC_XLIB_LOCK

   hb_gt_xwc_UpdateChr( wnd );
   if( wnd->fDspTitle )
   {
      wnd->fDspTitle = FALSE;
      XStoreName( wnd->dpy, wnd->window, wnd->szTitle ? wnd->szTitle : "" );
   }

#if 1
   do
   {
      while( XEventsQueued( wnd->dpy, QueuedAfterFlush ) )
      {
         XEvent evt;
         XNextEvent( wnd->dpy, &evt );
         hb_gt_xwc_WndProc( wnd, &evt );
      }
      hb_gt_xwc_UpdateSize( wnd );
      hb_gt_xwc_UpdatePts( wnd );
      hb_gt_xwc_UpdateCursor( wnd );
   }
   while( XEventsQueued( wnd->dpy, QueuedAfterFlush ) );

#else
   {
      BOOL fRepeat;
      do
      {
         XEvent evt;
         fRepeat = FALSE;
         while( XCheckWindowEvent( wnd->dpy, wnd->window, XWC_STD_MASK, &evt ) )
         {
            hb_gt_xwc_WndProc( wnd, &evt );
            fRepeat = TRUE;
         }
         hb_gt_xwc_UpdateSize( wnd );
         hb_gt_xwc_UpdatePts( wnd );
         hb_gt_xwc_UpdateCursor( wnd );
      }
      while( fRepeat );
   }
#endif

   HB_XWC_XLIB_UNLOCK

}

/* *********************************************************************** */

static void hb_gt_xwc_SetScrBuff( PXWND_DEF wnd, USHORT cols, USHORT rows )
{
   if( rows <= XWC_MAX_ROWS && cols <= XWC_MAX_COLS &&
       ( wnd->cols != cols || wnd->rows != rows || wnd->pCurrScr == NULL ) )
   {
      int iSize = cols * rows;

      wnd->cols   = cols;
      wnd->rows   = rows;

      if( wnd->pCurrScr == NULL )
         wnd->pCurrScr = ( ULONG * ) hb_xgrab( iSize * sizeof( ULONG ) );
      else
         wnd->pCurrScr = ( ULONG * ) hb_xrealloc( wnd->pCurrScr, iSize * sizeof( ULONG ) );

      memset( wnd->pCurrScr, 0xFFFFFFFFL, iSize * sizeof( ULONG ) );
      hb_gt_xwc_InvalidateChar( wnd, 0, 0, wnd->cols - 1, wnd->rows - 1 );
      HB_GTSELF_RESIZE( wnd->pGT, wnd->rows, wnd->cols );
   }
}

/* *********************************************************************** */

static BOOL hb_gt_xwc_Resize( PXWND_DEF wnd, USHORT cols, USHORT rows )
{
   if( rows <= XWC_MAX_ROWS && cols <= XWC_MAX_COLS )
   {
      USHORT width, height;

      hb_gt_xwc_SetScrBuff( wnd, cols, rows );

      width    = cols * wnd->fontWidth;
      height   = rows * wnd->fontHeight;

      if( width != wnd->width || height != wnd->height )
      {
         wnd->width  = width;
         wnd->height = height;

         if( wnd->window )
         {
            if( wnd->pm )
            {
               XFreePixmap( wnd->dpy, wnd->pm );
            }
            wnd->pm  = XCreatePixmap( wnd->dpy, wnd->window,
                                      wnd->width, wnd->height,
                                      DefaultDepth( wnd->dpy, DefaultScreen( wnd->dpy ) ) );
            wnd->drw = wnd->pm;
            XResizeWindow( wnd->dpy, wnd->window, wnd->width, wnd->height );
         }
      }

      return TRUE;
   }

   return FALSE;
}

/* *********************************************************************** */

static BOOL hb_gt_xwc_SetFont( PXWND_DEF wnd, const char * fontFace,
                               const char * weight, int size,
                               const char * encoding )
{
   char           fontString[ 250 ];
   XFontStruct *  xfs;

/*
   hb_snprintf( fontString, sizeof(fontString)-1, "-*-%s-%s-r-normal--%d-*-*-*-*-*-%s",
             fontFace, weight, size, encoding == NULL ? "*-*" : encoding );
 */
   hb_snprintf( fontString, sizeof( fontString ) - 1, "-*-%s-%s-r-*--%d-*-*-*-*-*-%s",
                fontFace, weight, size, encoding == NULL ? "*-*" : encoding );

   xfs = XLoadQueryFont( wnd->dpy, fontString );

   if( xfs == NULL )
      return FALSE;

   /* a shortcut for window height and width */
   wnd->fontHeight   = xfs->max_bounds.ascent + xfs->max_bounds.descent;
   wnd->fontWidth    = xfs->max_bounds.rbearing - xfs->min_bounds.lbearing;
   wnd->xfs          = xfs;

   return TRUE;
}

/* *********************************************************************** */

static PXWND_DEF hb_gt_xwc_CreateWndDef( PHB_GT pGT )
{
   PHB_FNAME   pFileName;
   PXWND_DEF   wnd = ( PXWND_DEF ) hb_xgrab( sizeof( XWND_DEF ) );

   /* clear whole structure */
   memset( wnd, 0, sizeof( XWND_DEF ) );

   wnd->pGT          = pGT;
   wnd->dpy          = NULL;
   wnd->fInit        = wnd->fData = FALSE;
   hb_gt_xwc_SetScrBuff( wnd, XWC_DEFAULT_COLS, XWC_DEFAULT_ROWS );
   wnd->fWinResize   = FALSE;
#ifndef HB_CDP_SUPPORT_OFF
   wnd->hostCDP      = hb_cdppage();
#endif
   wnd->cursorType   = SC_NORMAL;

   /* Window Title */
   pFileName         = hb_fsFNameSplit( hb_cmdargARGV()[ 0 ] );
   wnd->szTitle      = hb_strdup( pFileName->szName );
   wnd->fDspTitle    = TRUE;
   hb_xfree( pFileName );

   /* Font parameters */
   wnd->fontHeight            = XWC_DEFAULT_FONT_HEIGHT;
   wnd->fontWidth             = XWC_DEFAULT_FONT_WIDTH;
   wnd->szFontName            = hb_strdup( XWC_DEFAULT_FONT_NAME );
   wnd->szFontWeight          = hb_strdup( XWC_DEFAULT_FONT_WEIGHT );
   wnd->szFontEncoding        = hb_strdup( XWC_DEFAULT_FONT_ENCODING );
   /* set GTXWC extension for chosen font */
   wnd->fFixMetric            = XWC_DEFAULT_FONT_FIXMETRIC;
   wnd->fClearBkg             = XWC_DEFAULT_FONT_CLRBKG;
   wnd->fDrawBox              = XWC_DEFAULT_FONT_DRAWBOX;

   /* Clear keyboard buffer */
   wnd->keyBuffNO             = 0;
   wnd->keyBuffPointer        = 0;
   wnd->keyModifiers.bCtrl    = FALSE;
   wnd->keyModifiers.bAlt     = FALSE;
   wnd->keyModifiers.bAltGr   = FALSE;
   wnd->keyModifiers.bShift   = FALSE;

   wnd->lastEventTime         = CurrentTime;

   return wnd;
}

/* *********************************************************************** */

static BOOL hb_gt_xwc_ConnectX( PXWND_DEF wnd, BOOL fExit )
{
   if( wnd->dpy != NULL )
      return TRUE;

   HB_XWC_XLIB_LOCK

   /* with NULL, it gets the DISPLAY environment variable. */
   wnd->dpy = XOpenDisplay( NULL );

   if( wnd->dpy == NULL )
   {
      HB_XWC_XLIB_UNLOCK
      if( fExit )
      {
         /* TODO: a standard Harbour error should be generated here when
                  it can run without console!
            hb_errRT_TERM( EG_CREATE, 10001, NULL, "Can't connect to X server", 0, 0 );
          */
         s_fNoXServer = TRUE;
         hb_errInternal( 10001, "Can't connect to X server.", NULL, NULL );
      }
      return FALSE;
   }
   XSetErrorHandler( s_errorHandler );
   hb_gt_xwc_MouseInit( wnd );

   /* set atom identifiers for atom names we will use */
   s_atomDelWin         = XInternAtom( wnd->dpy, "WM_DELETE_WINDOW", True );
   s_atomTimestamp      = XInternAtom( wnd->dpy, "TIMESTAMP", False );
   s_atomAtom           = XInternAtom( wnd->dpy, "ATOM", False );
   s_atomInteger        = XInternAtom( wnd->dpy, "INTEGER", False );
   s_atomString         = XInternAtom( wnd->dpy, "STRING", False );
   s_atomUTF8String     = XInternAtom( wnd->dpy, "UTF8_STRING", False );
   s_atomPrimary        = XInternAtom( wnd->dpy, "PRIMARY", False );
   s_atomSecondary      = XInternAtom( wnd->dpy, "SECONDARY", False );
   s_atomClipboard      = XInternAtom( wnd->dpy, "CLIPBOARD", False );
   s_atomTargets        = XInternAtom( wnd->dpy, "TARGETS", False );
   s_atomCutBuffer0     = XInternAtom( wnd->dpy, "CUT_BUFFER0", False );
   s_atomText           = XInternAtom( wnd->dpy, "TEXT", False );
   s_atomCompoundText   = XInternAtom( wnd->dpy, "COMPOUND_TEXT", False );

   HB_XWC_XLIB_UNLOCK

   return TRUE;
}

static void hb_gt_xwc_DissConnectX( PXWND_DEF wnd )
{
   HB_XWC_XLIB_LOCK

   if( wnd->dpy != NULL )
   {
      hb_gt_xwc_DestroyCharTrans( wnd );

      if( wnd->pm )
      {
         XFreePixmap( wnd->dpy, wnd->pm );
         wnd->pm = 0;
      }
      if( wnd->xfs )
      {
         XFreeFont( wnd->dpy, wnd->xfs );
         wnd->xfs = NULL;
      }
      if( wnd->gc )
      {
         XFreeGC( wnd->dpy, wnd->gc );
         wnd->gc = 0;
      }
      if( wnd->window )
      {
         XDestroyWindow( wnd->dpy, wnd->window );
         wnd->window = 0;
      }

      XSync( wnd->dpy, True );

      XCloseDisplay( wnd->dpy );
      wnd->dpy          = NULL;

      /* Hack to avoid race condition inside some XLIB library - it looks
       * in heavy stres MT tests that it can receive some events bound with
       * destroyed objects and executes our error handler.
       */
      s_fIgnoreErrors   = TRUE;
   }

   HB_XWC_XLIB_UNLOCK
}

/* *********************************************************************** */

static void hb_gt_xwc_DestroyWndDef( PXWND_DEF wnd )
{
   hb_gt_xwc_DissConnectX( wnd );

   if( wnd->szTitle )
      hb_xfree( wnd->szTitle );
   if( wnd->szFontName )
      hb_xfree( wnd->szFontName );
   if( wnd->szFontWeight )
      hb_xfree( wnd->szFontWeight );
   if( wnd->szFontEncoding )
      hb_xfree( wnd->szFontEncoding );
   if( wnd->pCurrScr )
      hb_xfree( wnd->pCurrScr );
   if( wnd->ClipboardData )
      hb_xfree( wnd->ClipboardData );

   hb_xfree( wnd );
}

/* *********************************************************************** */

static void hb_gt_xwc_CreateWindow( PXWND_DEF wnd )
{
   int         whiteColor, blackColor, i;
   XSizeHints  xsize;
   XColor      color, dummy;

   HB_XWC_XLIB_LOCK

   /* load the standard font */
   if( ! hb_gt_xwc_SetFont( wnd, wnd->szFontName, wnd->szFontWeight, wnd->fontHeight, wnd->szFontEncoding ) )
   {
      if( ! hb_gt_xwc_SetFont( wnd, XWC_DEFAULT_FONT_NAME, XWC_DEFAULT_FONT_WEIGHT, XWC_DEFAULT_FONT_HEIGHT, XWC_DEFAULT_FONT_ENCODING ) )
      {
         HB_XWC_XLIB_UNLOCK

         /* TODO: a standard Harbour error should be generated here when
                  it can run without console!
            hb_errRT_TERM( EG_CREATE, 10001, NULL, "Can't load 'fixed' font", 0, 0 );
            return;
          */
            s_fNoXServer = TRUE;
         hb_errInternal( 10001, "Can't load 'fixed' font", NULL, NULL );
      }
   }

   /* build character translation table (after font selection) */
   hb_gt_xwc_BuildCharTrans( wnd );

   /* Set standard colors */
   wnd->colors = DefaultColormap( wnd->dpy, DefaultScreen( wnd->dpy ) );
   for( i = 0; i < 16; i++ )
   {
      if( XLookupColor( wnd->dpy, wnd->colors, rgb_colors[ i ], &dummy, &color ) != 0 )
      {
         if( hb_gt_xwc_AllocColor( wnd, &color ) )
         {
            wnd->pixels[ i ] = color.pixel;
#ifdef XWC_DEBUG
            printf( "hb_gt_xwc_AllocColor[%d]='%x/%x/%x'\r\n", i, color.red, color.green, color.blue ); fflush( stdout );
#endif
         }
      }
   }

   whiteColor  = WhitePixel( wnd->dpy, DefaultScreen( wnd->dpy ) );
   blackColor  = BlackPixel( wnd->dpy, DefaultScreen( wnd->dpy ) );
   wnd->window = XCreateSimpleWindow( wnd->dpy, DefaultRootWindow( wnd->dpy ),
                                      0, 0,
                                      wnd->fontWidth * wnd->cols,
                                      wnd->fontHeight * wnd->rows,
                                      0, blackColor, blackColor );
   wnd->gc = XCreateGC( wnd->dpy, wnd->window, 0, NULL );

   /* Line width 2 */
   XSetLineAttributes( wnd->dpy, wnd->gc, 1, LineSolid, CapRound, JoinBevel );

   XSetFont( wnd->dpy, wnd->gc, wnd->xfs->fid );
   XStoreName( wnd->dpy, wnd->window, wnd->szTitle );

   /* wnd->fWinResize = TRUE; */
   hb_gt_xwc_Resize( wnd, wnd->cols, wnd->rows );

   XMapWindow( wnd->dpy, wnd->window );
   /* ok, now we can inform the X manager about our new status: */
   /* xsize.flags = PWinGravity | PBaseSize | PResizeInc | PMinSize; */
   xsize.flags       = PWinGravity | PResizeInc | PMinSize | PMaxSize;
   xsize.win_gravity = CenterGravity;
   xsize.width_inc   = wnd->fontWidth;
   xsize.height_inc  = wnd->fontHeight;
   xsize.min_width   = wnd->fontWidth * XWC_MIN_COLS;
   xsize.min_height  = wnd->fontHeight * XWC_MIN_ROWS;
   xsize.max_width   = wnd->fontWidth * XWC_MAX_COLS;
   xsize.max_height  = wnd->fontHeight * XWC_MAX_ROWS;
   xsize.base_width  = wnd->width;
   xsize.base_height = wnd->height;
   XSetWMNormalHints( wnd->dpy, wnd->window, &xsize );

   /* Request WM to deliver destroy event */
   XSetWMProtocols( wnd->dpy, wnd->window, &s_atomDelWin, 1 );

   XSelectInput( wnd->dpy, wnd->window, XWC_STD_MASK );
#ifdef XWC_DEBUG
   printf( "Window created\r\n" ); fflush( stdout );
#endif

   HB_XWC_XLIB_UNLOCK
}

/* *********************************************************************** */

static void hb_gt_xwc_Initialize( PXWND_DEF wnd )
{
   if( ! wnd->fInit )
   {
      if( hb_gt_xwc_ConnectX( wnd, TRUE ) )
      {
         hb_gt_xwc_CreateWindow( wnd );
         wnd->fInit = TRUE;
         hb_gt_xwc_Enable();
      }
   }
}

/* *********************************************************************** */

static void hb_gt_xwc_SetSelection( PXWND_DEF wnd, const char * szData, ULONG ulSize )
{
   HB_XWC_XLIB_LOCK

   if( wnd->ClipboardOwner && ulSize == 0 )
   {
      XSetSelectionOwner( wnd->dpy, s_atomPrimary, None, wnd->ClipboardTime );
      XSetSelectionOwner( wnd->dpy, s_atomClipboard, None, wnd->ClipboardTime );
   }

   if( wnd->ClipboardData != NULL )
      hb_xfree( wnd->ClipboardData );
   wnd->ClipboardData            = ( unsigned char * ) hb_xgrab( ulSize + 1 );
   memcpy( wnd->ClipboardData, szData, ulSize );
   wnd->ClipboardData[ ulSize ]  = '\0';
   wnd->ClipboardSize            = ulSize;
   wnd->ClipboardTime            = wnd->lastEventTime;
   wnd->ClipboardOwner           = FALSE;

   if( ulSize > 0 )
   {
      XSetSelectionOwner( wnd->dpy, s_atomPrimary, wnd->window, wnd->ClipboardTime );
      if( XGetSelectionOwner( wnd->dpy, s_atomPrimary ) == wnd->window )
      {
         wnd->ClipboardOwner = TRUE;
         XSetSelectionOwner( wnd->dpy, s_atomClipboard, wnd->window, wnd->ClipboardTime );
      }
      else
      {
         const char * cMsg = "Cannot set primary selection\r\n";
         HB_GTSELF_OUTERR( wnd->pGT, ( BYTE * ) cMsg, strlen( cMsg ) );
      }
   }

   HB_XWC_XLIB_UNLOCK
}

/* *********************************************************************** */

static void hb_gt_xwc_RequestSelection( PXWND_DEF wnd )
{
   if( ! wnd->ClipboardOwner )
   {
      Atom  aRequest;
      ULONG ulCurrentTime  = hb_gt_xwc_CurrentTime();
      int   iConnFD        = ConnectionNumber( wnd->dpy );

      wnd->ClipboardRcvd      = FALSE;
      wnd->ClipboardRequest   = s_atomTargets;
      aRequest                = None;

      if( s_updateMode == XWC_ASYNC_UPDATE )
         s_iUpdateCounter = 150;

      do
      {
         if( aRequest != wnd->ClipboardRequest )
         {
            aRequest = wnd->ClipboardRequest;
            if( aRequest == None )
               break;

            HB_XWC_XLIB_LOCK

#ifdef XWC_DEBUG
            printf( "XConvertSelection: %ld (%s)\r\n", aRequest,
                    XGetAtomName( wnd->dpy, aRequest ) ); fflush( stdout );
#endif
            XConvertSelection( wnd->dpy, s_atomPrimary, aRequest,
                               s_atomCutBuffer0, wnd->window, wnd->lastEventTime );

            HB_XWC_XLIB_UNLOCK
         }

         if( s_updateMode == XWC_ASYNC_UPDATE )
         {
            if( s_iUpdateCounter == 0 )
               break;
            sleep( 1 );
         }
         else
         {
            hb_gt_xwc_ProcessMessages( wnd );
            if( ! wnd->ClipboardRcvd && wnd->ClipboardRequest == aRequest )
            {
               struct timeval timeout;
               fd_set         readfds;

               if( hb_gt_xwc_CurrentTime() - ulCurrentTime > 3 )
                  break;

               timeout.tv_sec    = 3;
               timeout.tv_usec   = 0;

               FD_ZERO( &readfds );
               FD_SET( iConnFD, &readfds );
               if( select( iConnFD + 1, &readfds, NULL, NULL, &timeout ) <= 0 )
                  break;
            }
         }
      }
      while( ! wnd->ClipboardRcvd && wnd->ClipboardRequest != None );

      wnd->ClipboardRequest = None;
   }
}

/* *********************************************************************** */

/* *********************************************************************** */

static void hb_gt_xwc_RealRefresh( PXWND_DEF wnd )
{
   hb_gt_xwc_Initialize( wnd );

   if( s_updateMode == XWC_SYNC_UPDATE && ! wnd->fRefresh )
   {
      wnd->fRefresh  = TRUE;
      hb_gt_xwc_ProcessMessages( wnd );
      wnd->fRefresh  = FALSE;
   }
}

/* *********************************************************************** */

static void hb_gt_xwc_LateRefresh( PXWND_DEF wnd )
{
   if( wnd->fInit )
      hb_gt_xwc_RealRefresh( wnd );
}

/* *********************************************************************** */

/* *********************************************************************** */

static void hb_gt_xwc_Init( PHB_GT pGT, HB_FHANDLE hFilenoStdin, HB_FHANDLE hFilenoStdout, HB_FHANDLE hFilenoStderr )
{
   PXWND_DEF wnd;

   HB_TRACE( HB_TR_DEBUG, ( "hb_gt_xwc_Init(%p,%p,%p,%p)", pGT, ( void * ) ( HB_PTRDIFF ) hFilenoStdin, ( void * ) ( HB_PTRDIFF ) hFilenoStdout, ( void * ) ( HB_PTRDIFF ) hFilenoStderr ) );

   HB_GTSUPER_INIT( pGT, hFilenoStdin, hFilenoStdout, hFilenoStderr );

#ifndef HB_XWC_XLIB_NEEDLOCKS
   if( hb_vmIsMt() )
   {
      if( ! XInitThreads() )
         hb_errInternal( 10002, "XInitThreads() failed !!!", NULL, NULL );
   }
#endif

   wnd                  = hb_gt_xwc_CreateWndDef( pGT );
   HB_GTLOCAL( pGT )    = wnd;

   /* for signal handler */
   s_wnd                = wnd;

   wnd->cursorState     = TRUE;
   wnd->cursorBlinkRate = 350;
   wnd->cursorStateTime = 0;

   HB_GTSELF_RESIZE( pGT, wnd->rows, wnd->cols );
   HB_GTSELF_SEMICOLD( pGT );

   /* For immediate connection to XSarver and screen Window show */
   /* hb_gt_xwc_Initialize( wnd ); */

   /* For connection to XSarver only */
   /* hb_gt_xwc_ConnectX( wnd, TRUE ); */
}

/* *********************************************************************** */

static void hb_gt_xwc_Exit( PHB_GT pGT )
{
   PXWND_DEF wnd;

   HB_TRACE( HB_TR_DEBUG, ( "hb_gt_xwc_Exit(%p)", pGT ) );

   wnd = HB_GTXWC_GET( pGT );

   hb_gt_xwc_Disable();
   HB_GTSUPER_EXIT( pGT );

   if( wnd )
   {
      hb_gt_xwc_DestroyWndDef( wnd );
   }
}

/* *********************************************************************** */

static BOOL hb_gt_xwc_SetMode( PHB_GT pGT, int iRow, int iCol )
{
   BOOL fResult = FALSE;

   HB_TRACE( HB_TR_DEBUG, ( "hb_gt_xwc_SetMode(%p,%d,%d)", pGT, iRow, iCol ) );

   if( iCol >= XWC_MIN_COLS && iRow >= XWC_MIN_ROWS &&
       iCol <= XWC_MAX_COLS && iRow <= XWC_MAX_ROWS )
   {
      PXWND_DEF wnd = HB_GTXWC_GET( pGT );

      if( iCol == wnd->cols && iRow == wnd->rows )
      {
         fResult = TRUE;
         HB_GTSELF_RESIZE( pGT, wnd->rows, wnd->cols );
      }
      else if( ! wnd->fInit )
      {
         hb_gt_xwc_SetScrBuff( wnd, iCol, iRow );
         fResult = TRUE;
      }
      else
      {
         hb_gt_xwc_Disable();
         HB_XWC_XLIB_LOCK
            fResult = hb_gt_xwc_Resize( wnd, iCol, iRow );
         HB_XWC_XLIB_UNLOCK
         if( fResult )
            HB_GTSELF_RESIZE( pGT, wnd->rows, wnd->cols );
         hb_gt_xwc_Enable();
      }
   }

   return fResult;
}

/* *********************************************************************** */

static BOOL hb_gt_xwc_GetBlink( PHB_GT pGT )
{
   HB_TRACE( HB_TR_DEBUG, ( "hb_gt_xwc_GetBlink(%p)", pGT ) );

   HB_SYMBOL_UNUSED( pGT );

   return FALSE;
}

/* *********************************************************************** */

static const char * hb_gt_xwc_Version( PHB_GT pGT, int iType )
{
   HB_SYMBOL_UNUSED( pGT );

   if( iType == 0 )
      return HB_GT_DRVNAME( HB_GT_NAME );

   return "Harbour Terminal: XWindow GUI console (XWC)";
}

/* *********************************************************************** */

static int hb_gt_xwc_ReadKey( PHB_GT pGT, int iEventMask )
{
   PXWND_DEF   wnd;
   int         c = 0;

   HB_TRACE( HB_TR_DEBUG, ( "hb_wxc_gt_ReadKey(%p,%d)", pGT, iEventMask ) );

   HB_SYMBOL_UNUSED( iEventMask );

   wnd = HB_GTXWC_GET( pGT );
   hb_gt_xwc_LateRefresh( wnd );
   /* hb_gt_xwc_RealRefresh( wnd ); */

   if( hb_gt_xwc_GetCharFromInputQueue( wnd, &c ) )
      return c;
   else
      return 0;
}


/* *********************************************************************** */

static void hb_gt_xwc_Tone( PHB_GT pGT, double dFrequency, double dDuration )
{
   PXWND_DEF wnd;

   HB_TRACE( HB_TR_DEBUG, ( "hb_gt_xwc_Tone(%p,%lf,%lf)", pGT, dFrequency, dDuration ) );

   /* The conversion from Clipper (DOS) timer tick units to
      milliseconds is * 1000.0 / 18.2. */
   dDuration   /= 18.2;

   wnd         = HB_GTXWC_GET( pGT );
   if( wnd->dpy != NULL )
   {
      XKeyboardControl XkbCtrl;

      XkbCtrl.bell_pitch      = ( int ) dFrequency;
      XkbCtrl.bell_duration   = ( int ) ( dDuration * 1000 );

      HB_XWC_XLIB_LOCK
      XChangeKeyboardControl( wnd->dpy, KBBellPitch | KBBellDuration, &XkbCtrl );
      XBell( wnd->dpy, 0 );
      HB_XWC_XLIB_UNLOCK
   }
   hb_idleSleep( dDuration );
}

/* *********************************************************************** */

static BOOL hb_gt_xwc_mouse_IsPresent( PHB_GT pGT )
{
   HB_TRACE( HB_TR_DEBUG, ( "hb_gt_xwc_mouse_IsPresent(%p)", pGT ) );

   return HB_GTXWC_GET( pGT )->mouseNumButtons > 0;
}

/* *********************************************************************** */

static void hb_gt_xwc_mouse_GetPos( PHB_GT pGT, int * piRow, int * piCol )
{
   PXWND_DEF wnd;

   HB_TRACE( HB_TR_DEBUG, ( "hb_gt_xwc_mouse_GetPos(%p,%p,%p)", pGT, piRow, piCol ) );

   wnd = HB_GTXWC_GET( pGT );
   if( wnd )
   {
      hb_gt_xwc_LateRefresh( wnd );
      *piRow   = wnd->mouseRow;
      *piCol   = wnd->mouseCol;
   }
}

/* *********************************************************************** */

static void hb_gt_xwc_mouse_SetPos( PHB_GT pGT, int iRow, int iCol )
{
   PXWND_DEF wnd;

   HB_TRACE( HB_TR_DEBUG, ( "hb_gt_xwc_mouse_SetPos(%p,%d,%d)", pGT, iRow, iCol ) );

   wnd               = HB_GTXWC_GET( pGT );
   wnd->mouseGotoRow = iRow;
   wnd->mouseGotoCol = iCol;
   hb_gt_xwc_LateRefresh( wnd );
}

/* *********************************************************************** */

static BOOL hb_gt_xwc_mouse_ButtonState( PHB_GT pGT, int iButton )
{
   PXWND_DEF wnd;

   HB_TRACE( HB_TR_DEBUG, ( "hb_gt_xwc_mouse_ButtonState(%p,%i)", pGT, iButton ) );

   wnd = HB_GTXWC_GET( pGT );
   if( iButton >= 0 && iButton < wnd->mouseNumButtons )
      return ( wnd->mouseButtonsState & 1 << iButton ) != 0;
   else
      return FALSE;
}

/* *********************************************************************** */

static int hb_gt_xwc_mouse_CountButton( PHB_GT pGT )
{
   PXWND_DEF wnd;

   HB_TRACE( HB_TR_DEBUG, ( "hb_gt_xwc_mouse_CountButton(%p)", pGT ) );

   wnd = HB_GTXWC_GET( pGT );
   hb_gt_xwc_RealRefresh( wnd );

   return wnd->mouseNumButtons;
}

/* *********************************************************************** */

static BOOL hb_gt_xwc_SetDispCP( PHB_GT pGT, const char * pszTermCDP, const char * pszHostCDP, BOOL fBox )
{

   HB_GTSUPER_SETDISPCP( pGT, pszTermCDP, pszHostCDP, fBox );

#ifndef HB_CDP_SUPPORT_OFF
   /*
    * We are displaying text in U16 so pszTermCDP is unimportant.
    * We only have to know what is the internal application codepage
    * to make proper translation
    */
   if( ! pszHostCDP || ! *pszHostCDP )
   {
      if( hb_cdppage() )
         pszHostCDP = hb_cdpID();
      else if( pszTermCDP && *pszTermCDP )
         pszHostCDP = pszTermCDP;
   }
   if( pszHostCDP && *pszHostCDP )
   {
      PHB_CODEPAGE   cdpHost  = hb_cdpFind( pszHostCDP );
      PXWND_DEF      wnd      = HB_GTXWC_GET( pGT );

      if( cdpHost && cdpHost != wnd->hostCDP )
      {
         wnd->hostCDP = cdpHost;
         if( wnd->fInit )
         {
            HB_XWC_XLIB_LOCK
            hb_gt_xwc_BuildCharTrans( wnd );
            HB_XWC_XLIB_UNLOCK
         }
      }
   }
#endif

   return TRUE;
}

/* *********************************************************************** */

static BOOL hb_gt_xwc_SetKeyCP( PHB_GT pGT, const char * pszTermCDP, const char * pszHostCDP )
{

   HB_GTSUPER_SETKEYCP( pGT, pszTermCDP, pszHostCDP );

#ifndef HB_CDP_SUPPORT_OFF
   /*
    * Basic Xlib api has no function to return character key val in
    * unicode so far. We can use some nonstandard extension or try
    * to make translation XKEY_* to unicode ourself.
    * Now I don't have time to build the full conversion table so I only
    * add a simple hack which should work for LATIN-x encoding and
    * probably some others
    */
   if( ! pszTermCDP || ! *pszTermCDP )
   {
      if( hb_cdppage() )
         pszTermCDP = hb_cdpID();
      else if( pszHostCDP && *pszHostCDP )
         pszTermCDP = pszHostCDP;
   }
   if( pszTermCDP && *pszTermCDP )
   {
      PHB_CODEPAGE cdpTerm = hb_cdpFind( pszTermCDP );
      if( cdpTerm )
      {
         HB_GTXWC_GET( pGT )->inCDP = cdpTerm;
      }
   }
#endif

   return TRUE;
}

/* *********************************************************************** */

static int hb_gt_xwc_getKbdState( PXWND_DEF wnd )
{
   int iKbdState = 0;

   if( wnd->keyModifiers.bShift )
      iKbdState |= HB_GTI_KBD_SHIFT;
   if( wnd->keyModifiers.bCtrl )
      iKbdState |= HB_GTI_KBD_CTRL;
   if( wnd->keyModifiers.bAlt )
      iKbdState |= HB_GTI_KBD_ALT;

   return iKbdState;
}

static BOOL hb_gt_xwc_Info( PHB_GT pGT, int iType, PHB_GT_INFO pInfo )
{
   PXWND_DEF   wnd;
   int         iVal;

   HB_TRACE( HB_TR_DEBUG, ( "hb_gt_xwc_Info(%p,%d,%p)", pGT, iType, pInfo ) );

   wnd = HB_GTXWC_GET( pGT );
   if( ! wnd->dpy )
   {
      switch( iType )
      {
         case HB_GTI_FULLSCREEN:
         case HB_GTI_KBDSUPPORT:
         case HB_GTI_ISGRAPHIC:
            hb_gt_xwc_ConnectX( wnd, FALSE );
            break;

         case HB_GTI_INPUTFD:
         case HB_GTI_SCREENDEPTH:
         case HB_GTI_DESKTOPDEPTH:
         case HB_GTI_DESKTOPWIDTH:
         case HB_GTI_DESKTOPHEIGHT:
         case HB_GTI_DESKTOPCOLS:
         case HB_GTI_DESKTOPROWS:
         case HB_GTI_CLIPBOARDDATA:
            hb_gt_xwc_ConnectX( wnd, TRUE );
            break;
      }
   }

   switch( iType )
   {
      case HB_GTI_FULLSCREEN:
      case HB_GTI_KBDSUPPORT:
      case HB_GTI_ISGRAPHIC:
         pInfo->pResult = hb_itemPutL( pInfo->pResult, wnd->dpy != NULL );
         break;

      case HB_GTI_ISUNICODE:
         pInfo->pResult = hb_itemPutL( pInfo->pResult, TRUE );
         break;

      case HB_GTI_INPUTFD:
         pInfo->pResult = hb_itemPutNI( pInfo->pResult, ConnectionNumber( wnd->dpy ) );
         break;

      case HB_GTI_SCREENWIDTH:
         pInfo->pResult = hb_itemPutNI( pInfo->pResult, wnd->width );
         break;

      case HB_GTI_SCREENHEIGHT:
         pInfo->pResult = hb_itemPutNI( pInfo->pResult, wnd->height );
         break;

      case HB_GTI_SCREENDEPTH:
      case HB_GTI_DESKTOPDEPTH:
         pInfo->pResult = hb_itemPutNI( pInfo->pResult,
                                        DefaultDepth( wnd->dpy, DefaultScreen( wnd->dpy ) ) );
         break;

      case HB_GTI_FONTSIZE:
         pInfo->pResult = hb_itemPutNI( pInfo->pResult, wnd->fontHeight );
         iVal           = hb_itemGetNI( pInfo->pNewVal );
         if( iVal > 0 ) /* TODO */
            wnd->fontHeight = iVal;
         break;

      case HB_GTI_FONTWIDTH:
         pInfo->pResult = hb_itemPutNI( pInfo->pResult, wnd->fontWidth );
         iVal           = hb_itemGetNI( pInfo->pNewVal );
         if( iVal > 0 ) /* TODO */
            wnd->fontWidth = iVal;
         break;

      case HB_GTI_FONTNAME:
         pInfo->pResult = hb_itemPutC( pInfo->pResult, wnd->szFontName );
         if( hb_itemType( pInfo->pNewVal ) & HB_IT_STRING ) /* TODO */
         {
            if( wnd->szFontName )
               hb_xfree( wnd->szFontName );
            wnd->szFontName = hb_strdup( hb_itemGetCPtr( pInfo->pNewVal ) );
         }
         break;

      case HB_GTI_DESKTOPWIDTH:
      case HB_GTI_DESKTOPHEIGHT:
      case HB_GTI_DESKTOPCOLS:
      case HB_GTI_DESKTOPROWS:
      {
         XWindowAttributes wndAttr;
         HB_XWC_XLIB_LOCK
         XGetWindowAttributes( wnd->dpy, DefaultRootWindow( wnd->dpy ), &wndAttr );
         HB_XWC_XLIB_UNLOCK
         switch( iType )
         {
            case HB_GTI_DESKTOPWIDTH:
               iVal  = wndAttr.width;
               break;
            case HB_GTI_DESKTOPHEIGHT:
               iVal  = wndAttr.height;
               break;
            case HB_GTI_DESKTOPCOLS:
               iVal  = wndAttr.width / wnd->fontWidth;
               break;
            case HB_GTI_DESKTOPROWS:
               iVal  = wndAttr.height / wnd->fontHeight;
               break;
            default:
               iVal  = 0;
         }
         pInfo->pResult = hb_itemPutNI( pInfo->pResult, iVal );
         break;
      }

      case HB_GTI_WINTITLE:
         pInfo->pResult = hb_itemPutC( pInfo->pResult, wnd->szTitle );
         if( hb_itemType( pInfo->pNewVal ) & HB_IT_STRING )
         {
            if( wnd->szTitle )
               hb_xfree( wnd->szTitle );

            if( hb_itemGetCLen( pInfo->pNewVal ) > 0 )
               wnd->szTitle = hb_strdup( hb_itemGetCPtr( pInfo->pNewVal ) );
            else
               wnd->szTitle = NULL;

            wnd->fDspTitle = TRUE;
         }
         break;

      case HB_GTI_VIEWMAXWIDTH:
         pInfo->pResult = hb_itemPutNI( pInfo->pResult, wnd->cols );
         break;

      case HB_GTI_VIEWMAXHEIGHT:
         pInfo->pResult = hb_itemPutNI( pInfo->pResult, wnd->rows );
         break;

      case HB_GTI_CLIPBOARDDATA:
         if( hb_itemType( pInfo->pNewVal ) & HB_IT_STRING )
         {
            hb_gt_xwc_RealRefresh( wnd );
            hb_gt_xwc_SetSelection( wnd, hb_itemGetCPtr( pInfo->pNewVal ),
                                    hb_itemGetCLen( pInfo->pNewVal ) );
            hb_gt_xwc_RealRefresh( wnd );
         }
         else
         {
            hb_gt_xwc_RealRefresh( wnd );
            hb_gt_xwc_RequestSelection( wnd );
            pInfo->pResult = hb_itemPutCL( pInfo->pResult,
                                           ( char * ) wnd->ClipboardData,
                                           wnd->ClipboardSize );
         }
         break;

      case HB_GTI_CURSORBLINKRATE:
         pInfo->pResult = hb_itemPutNI( pInfo->pResult, wnd->cursorBlinkRate );
         if( hb_itemType( pInfo->pNewVal ) & HB_IT_NUMERIC )
            wnd->cursorBlinkRate = hb_itemGetNI( pInfo->pNewVal );
         break;

      case HB_GTI_KBDSHIFTS:
         pInfo->pResult = hb_itemPutNI( pInfo->pResult,
                                        hb_gt_xwc_getKbdState( wnd ) );
         break;

      default:
         return HB_GTSUPER_INFO( pGT, iType, pInfo );
   }

   return TRUE;
}

static int hb_gt_xwc_gfx_Primitive( PHB_GT pGT, int iType, int iTop, int iLeft, int iBottom, int iRight, int iColor )
{
   PXWND_DEF   wnd;
   int         iRet = 1;
   XColor      color;

   HB_TRACE( HB_TR_DEBUG, ( "hb_gt_xwc_gfx_Primitive(%p,%d,%d,%d,%d,%d,%d)", pGT, iType, iTop, iLeft, iBottom, iRight, iColor ) );

   wnd = HB_GTXWC_GET( pGT );
   hb_gt_xwc_Initialize( wnd );
   HB_GTSELF_REFRESH( pGT );

   switch( iType )
   {
      case HB_GFX_ACQUIRESCREEN:
         /* TODO: */
         break;

      case HB_GFX_RELEASESCREEN:
         /* TODO: */
         break;

      case HB_GFX_MAKECOLOR:
         /* TODO: */
         color.red   = iTop * 256;
         color.green = iLeft * 256;
         color.blue  = iBottom * 256;
         color.flags = DoRed | DoGreen | DoBlue;
         HB_XWC_XLIB_LOCK
         hb_gt_xwc_AllocColor( wnd, &color );
         HB_XWC_XLIB_UNLOCK
            iRet = color.pixel;
         break;

      case HB_GFX_CLIPTOP:
         iRet = wnd->ClipRect.y;
         break;

      case HB_GFX_CLIPLEFT:
         iRet = wnd->ClipRect.x;
         break;

      case HB_GFX_CLIPBOTTOM:
         iRet = wnd->ClipRect.y + wnd->ClipRect.height - 1;
         break;

      case HB_GFX_CLIPRIGHT:
         iRet = wnd->ClipRect.x + wnd->ClipRect.width - 1;
         break;

      case HB_GFX_SETCLIP:
         wnd->ClipRect.y      = iTop;
         wnd->ClipRect.x      = iLeft;
         wnd->ClipRect.width  = iBottom;
         wnd->ClipRect.height = iRight;
         HB_XWC_XLIB_LOCK
         XSetClipRectangles( wnd->dpy, wnd->gc, 0, 0, &wnd->ClipRect, 1, YXBanded );
         HB_XWC_XLIB_UNLOCK
         break;

      case HB_GFX_DRAWINGMODE:
         iRet = HB_GFX_MODE_SOLID;
         break;

      case HB_GFX_GETPIXEL:
         /* TODO: */
         iRet = 0;
         break;

      case HB_GFX_PUTPIXEL:
         HB_XWC_XLIB_LOCK
         XSetForeground( wnd->dpy, wnd->gc, iBottom );
         XDrawPoint( wnd->dpy, wnd->drw, wnd->gc, iLeft, iTop );
         HB_XWC_XLIB_UNLOCK
         hb_gt_xwc_InvalidatePts( wnd, iLeft, iTop, iLeft, iTop );
         break;

      case HB_GFX_LINE:
         HB_XWC_XLIB_LOCK
         XSetForeground( wnd->dpy, wnd->gc, iColor );
         XDrawLine( wnd->dpy, wnd->drw, wnd->gc,
                    iLeft, iTop, iRight, iBottom );
         HB_XWC_XLIB_UNLOCK
         hb_gt_xwc_InvalidatePts( wnd, iLeft, iTop, iRight, iBottom );
         break;

      case HB_GFX_RECT:
         HB_XWC_XLIB_LOCK
         XSetForeground( wnd->dpy, wnd->gc, iColor );
         XDrawRectangle( wnd->dpy, wnd->drw, wnd->gc,
                         iLeft, iTop, iRight - iLeft, iBottom - iTop );
         HB_XWC_XLIB_UNLOCK
         hb_gt_xwc_InvalidatePts( wnd, iLeft, iTop, iRight, iBottom );
         break;

      case HB_GFX_FILLEDRECT:
         HB_XWC_XLIB_LOCK
         XSetForeground( wnd->dpy, wnd->gc, iColor );
         XFillRectangle( wnd->dpy, wnd->drw, wnd->gc,
                         iLeft, iTop, iRight - iLeft, iBottom - iTop );
         HB_XWC_XLIB_UNLOCK
         hb_gt_xwc_InvalidatePts( wnd, iLeft, iTop, iRight, iBottom );
         break;

      case HB_GFX_CIRCLE:
         HB_XWC_XLIB_LOCK
         XSetForeground( wnd->dpy, wnd->gc, iRight );
         XDrawArc( wnd->dpy, wnd->drw, wnd->gc,
                   iLeft, iTop, iBottom, iBottom, 0, 360 * 64 );
         HB_XWC_XLIB_UNLOCK
         hb_gt_xwc_InvalidatePts( wnd, iLeft - iBottom, iTop - iBottom,
                                  iLeft + iBottom, iTop + iBottom );
         break;

      case HB_GFX_FILLEDCIRCLE:
         HB_XWC_XLIB_LOCK
         XSetForeground( wnd->dpy, wnd->gc, iRight );
         XFillArc( wnd->dpy, wnd->drw, wnd->gc,
                   iLeft, iTop, iBottom, iBottom, 0, 360 * 64 );
         HB_XWC_XLIB_UNLOCK
         hb_gt_xwc_InvalidatePts( wnd, iLeft - iBottom, iTop - iBottom,
                                  iLeft + iBottom, iTop + iBottom );
         break;

      case HB_GFX_ELLIPSE:
         HB_XWC_XLIB_LOCK
         XSetForeground( wnd->dpy, wnd->gc, iColor );
         XDrawArc( wnd->dpy, wnd->drw, wnd->gc,
                   iLeft, iTop, iRight, iBottom, 0, 360 * 64 );
         HB_XWC_XLIB_UNLOCK
         hb_gt_xwc_InvalidatePts( wnd, iLeft - iRight, iTop - iBottom,
                                  iLeft + iRight, iTop + iBottom );
         break;

      case HB_GFX_FILLEDELLIPSE:
         HB_XWC_XLIB_LOCK
         XSetForeground( wnd->dpy, wnd->gc, iColor );
         XFillArc( wnd->dpy, wnd->drw, wnd->gc,
                   iLeft, iTop, iRight, iBottom, 0, 360 * 64 );
         HB_XWC_XLIB_UNLOCK
         hb_gt_xwc_InvalidatePts( wnd, iLeft - iRight, iTop - iBottom,
                                  iLeft + iRight, iTop + iBottom );
         break;

      case HB_GFX_FLOODFILL:
         /* TODO: */
         hb_gt_xwc_InvalidatePts( wnd, 0, 0, wnd->width, wnd->height );
         break;

      default:
         return HB_GTSUPER_GFXPRIMITIVE( pGT, iType, iTop, iLeft, iBottom, iRight, iColor );
   }

   if( HB_GTSELF_DISPCOUNT( pGT ) == 0 )
   {
      hb_gt_xwc_RealRefresh( wnd );
   }

   return iRet;
}

/* *********************************************************************** */

static void hb_gt_xwc_Redraw( PHB_GT pGT, int iRow, int iCol, int iSize )
{
   PXWND_DEF wnd;

   HB_TRACE( HB_TR_DEBUG, ( "hb_gt_xwc_Redraw(%p,%d,%d,%d)", pGT, iRow, iCol, iSize ) );

   wnd = HB_GTXWC_GET( pGT );
   if( wnd && ! s_fNoXServer )
   {
      if( wnd->fInit )
      {
#if 1
         hb_gt_xwc_InvalidateChar( wnd, iCol, iRow, iCol + iSize - 1, iRow );
#else
         hb_gt_xwc_RepaintChar( wnd, iCol, iRow, iCol + iSize - 1, iRow );
         iCol  *= wnd->fontWidth;
         iRow  *= wnd->fontHeight;
         hb_gt_xwc_InvalidatePts( wnd, iCol, iRow,
                                  iCol + iSize * wnd->fontWidth - 1,
                                  iRow + wnd->fontHeight - 1 );
#endif
      }
#if 0
      else if( ! wnd->fData )
      {
         BYTE     bDefColor = HB_GTSELF_GETCOLOR( pGT );
         BYTE     bColor, bAttr;
         USHORT   usChar;

         while( iSize-- )
         {
            if( ! HB_GTSELF_GETSCRCHAR( pGT, iRow, iCol++, &bColor, &bAttr, &usChar ) )
               break;
            if( bColor != bDefColor || usChar != ' ' )
            {
               wnd->fData = TRUE;
               break;
            }
         }
      }
#else
      else
         wnd->fData = TRUE;
#endif
   }
}

/* *********************************************************************** */

static void hb_gt_xwc_Refresh( PHB_GT pGT )
{
   PXWND_DEF wnd;

   HB_TRACE( HB_TR_DEBUG, ( "hb_gt_xwc_Refresh(%p)", pGT ) );

   wnd = HB_GTXWC_GET( pGT );
   HB_GTSUPER_REFRESH( pGT );

   if( wnd && ! s_fNoXServer )
   {
      HB_GTSELF_GETSCRCURSOR( pGT, &wnd->row, &wnd->col, &wnd->cursorType );

      if( wnd->fInit || wnd->fData )
         hb_gt_xwc_RealRefresh( wnd );
   }
}

/* *********************************************************************** */

static BOOL hb_gt_FuncInit( PHB_GT_FUNCS pFuncTable )
{
   HB_TRACE( HB_TR_DEBUG, ( "hb_gt_FuncInit(%p)", pFuncTable ) );

   pFuncTable->Init              = hb_gt_xwc_Init;
   pFuncTable->Exit              = hb_gt_xwc_Exit;
   pFuncTable->SetMode           = hb_gt_xwc_SetMode;
   pFuncTable->Redraw            = hb_gt_xwc_Redraw;
   pFuncTable->Refresh           = hb_gt_xwc_Refresh;
   pFuncTable->GetBlink          = hb_gt_xwc_GetBlink;
   pFuncTable->Version           = hb_gt_xwc_Version;
   pFuncTable->Tone              = hb_gt_xwc_Tone;
   pFuncTable->Info              = hb_gt_xwc_Info;
   pFuncTable->SetDispCP         = hb_gt_xwc_SetDispCP;
   pFuncTable->SetKeyCP          = hb_gt_xwc_SetKeyCP;

   pFuncTable->ReadKey           = hb_gt_xwc_ReadKey;

   pFuncTable->MouseIsPresent    = hb_gt_xwc_mouse_IsPresent;
   pFuncTable->MouseGetPos       = hb_gt_xwc_mouse_GetPos;
   pFuncTable->MouseSetPos       = hb_gt_xwc_mouse_SetPos;
   pFuncTable->MouseButtonState  = hb_gt_xwc_mouse_ButtonState;
   pFuncTable->MouseCountButton  = hb_gt_xwc_mouse_CountButton;

   pFuncTable->GfxPrimitive      = hb_gt_xwc_gfx_Primitive;

   return TRUE;
}

/* ********************************************************************** */

static const HB_GT_INIT gtInit = { HB_GT_DRVNAME( HB_GT_NAME ),
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

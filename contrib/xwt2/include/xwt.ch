/*
   XWT - xHarbour Windowing Toolkit

   (C) 2003 Giancarlo Niccolai

   $Id: xwt.ch 9279 2011-02-14 18:06:32Z druzus $

   Definitions
*/

#ifndef XWT_CH
#define XWT_CH

/* Event types */
#define XWT_E_SHOW         1  // No params
#define XWT_E_HIDE         2  // No params
#define XWT_E_MOVE         3  // x,y
#define XWT_E_RESIZE       4  // width, height (-1 == unchanged )
#define XWT_E_TEXT         5  // cText
#define XWT_E_ADDCHILD     6  // oChild
#define XWT_E_REMOVECHILD  7  // oChild
#define XWT_E_ADDEDTO      8  // oContainer
#define XWT_E_REMOVEDFROM  9  // oContainer

#define XWT_E_ENTER        10 // No Params
#define XWT_E_LEAVE        11 // No Params
#define XWT_E_MOUSEMOVE    12 // X,Y

#define XWT_E_PRESSED      20 // No Params
#define XWT_E_RELEASED     21 // No Params
#define XWT_E_CLICKED      22 // No Params or x,y if available

#define XWT_E_CHANGED      30  // Depending on the sender object, generally a text
#define XWT_E_UPDATED      31  // Means a "definitive" change, like pressing enter in a textbox
#define XWT_E_REVERTED     32  // Means a "Revert" request, or a cancelation of current op
#define XWT_E_CANCELED     32  // Synonym

#define XWT_E_SCROLL       40  // Scrolling is set (x,y)
#define XWT_E_SCROLLED     41  // Manin widget sends this to child (x,y)

#define XWT_E_CREATE       100 // No Params
#define XWT_E_DESTROY_REQ  101 // No Params
#define XWT_E_DESTROY      102 // No Params


/**** Widget properties ****/
#define XWT_PROP_FOCUS     1  // bool
#define XWT_PROP_POSITION  2  // int, int
#define XWT_PROP_SIZE      3  // int, int
#define XWT_PROP_TEXT      4  // text (char *)
#define XWT_PROP_NAME      5  // text (char *)
#define XWT_PROP_SENSIBLE  6  // Verbosity of mouse events; BOOL

#define XWT_PROP_EDITABLE  10 // bool
#define XWT_PROP_VISIBLE   11 // bool
#define XWT_PROP_SELREGION 12 // x,y (stacked in position)

#define XWT_PROP_CONTENT   20 // data * (number generally holds the size)
#define XWT_PROP_RESOURCE  21 // Char * or number depending driver

#define XWT_PROP_LAYMODE   30 // numeric, 0 or 1 (horiz/vert)
#define XWT_PROP_PADDING   31 // numeric ( RowPad, ColPad (size) for GRID )
#define XWT_PROP_EXPAND    32 // BOOL
#define XWT_PROP_FILL      33 // BOOL
#define XWT_PROP_HOMOGENEOUS  34 // BOOL
#define XWT_PROP_BOX       35    // BOOL
#define XWT_PROP_BORDER    36    // Numeric

#define XWT_PROP_ATTACH    40 // Pseudo prop: {child, row, col, width, height}
#define XWT_PROP_COLROWS   41 // width, height
#define XWT_PROP_SHRINK    42 // BOOL

#define XWT_PROP_IMAGE     50 // text

#define XWT_PROP_SCROLL    60 // text

#define XWT_PROP_STATUS    70 // Integer or bool, depending on widget
#define XWT_PROP_RADIOGROUP    71 // Pointer
#define XWT_PROP_GETDATEMODAL   72 //date
#define XWT_PROP_GETDATE   73 //date
#define XWT_PROP_SETDATE   74 //date

#define XWT_PROP_FILEMASK  80
#define XWT_PROP_FILENAME  81


#define XWT_PROP_FIRSTWID  90 // Widget (data)
#define XWT_PROP_SECWID    91 // Widget (data)
#define XWT_PROP_FIRSTSHRINK  92 //bool
#define XWT_PROP_SECSHRINK 93 //bool

#define XWT_PROP_FIXED      100 // bool
#define XWT_PROP_MODAL      101 // bool
#define XWT_PROP_VISIBILITY 102 // int (see widget visibility)

#define XWT_PROP_TITLES     110 //PHB_ITEM (HB_IT_ARRAY) in data
#define XWT_PROP_COLUMNS    111 //Number of columns in a table
#define XWT_PROP_COLEDITABLE    112 //Number of columns in a table, negative for disable

#define XWT_PROP_FONT       120
#define XWT_PROP_FGCOLOR      130
#define XWT_PROP_BGCOLOR      131
#define XWT_PROP_BASECOLOR      132
#define XWT_PROP_TEXTCOLOR      133

#define XWT_PROP_FONTNAME        140

#define XWT_PROP_PROGRESSTYPE   150
#define XWT_PROP_PROGRESSFRAC   151

#define XWT_PROP_SETCOMBOITEMS 160
#define XWT_PROP_SETCOMBOEDIT  161
#define XWT_PROP_GETCOMBOSELECT 162
#define XWT_PROP_SETLISTITEMS 170

// Global widget management status
#define XWT_PROP_UPDATE    200  // boolean (ready or not)

/* Fake properties */
#define XWT_PROP_SETMENUBAR 1000 // PHB_ITEM (HB_IT_ARRAY) in data
#define XWT_PROP_RSTMENUBAR 1001 // idem

/*User defined properties */
#define XWT_PROP_USER       10000

/* Widget layout mode */
#define XWT_LM_HORIZ         0
#define XWT_LM_VERT          1

/* Widget visibility */
#define XWT_VIS_HIDDEN       0
#define XWT_VIS_NORMAL       1
#define XWT_VIS_MAXIMIZED_H  2
#define XWT_VIS_MAXIMIZED_V  3
#define XWT_VIS_MAXIMIZED    4
#define XWT_VIS_MINIMIZED    5

/* Widget alignment */
#define XWT_ALIGN_TOP        0
#define XWT_ALIGN_LEFT       0
#define XWT_ALIGN_CENTER     1
#define XWT_ALIGN_BOTTOM     2
#define XWT_ALIGN_RIGHT      2


/*** Message box ***/
#define XWT_MSGBOX_INFO     1
#define XWT_MSGBOX_QUESTION 2
#define XWT_MSGBOX_ERROR    3
#define XWT_MSGBOX_WARNING  4

#define XWT_MSGBOX_OK        1
#define XWT_MSGBOX_CANCEL    2
#define XWT_MSGBOX_CLOSE     4
#define XWT_MSGBOX_ABORT     8
#define XWT_MSGBOX_RETRY     16
#define XWT_MSGBOX_YES       32
#define XWT_MSGBOX_NO        64

/**** Image status ********/
#define XWT_IMG_NOTREADY      0
#define XWT_IMG_ERROR        -1
#define XWT_IMG_PROGRESS      1
#define XWT_IMG_READY         2


/**** Type of widgets *****/
#define XWT_TYPE_WIDGET   0
#define XWT_TYPE_WINDOW   1
#define XWT_TYPE_FRAME    2
#define XWT_TYPE_PANE     3
#define XWT_TYPE_LAYOUT   4
#define XWT_TYPE_VIEWPORT 5
#define XWT_TYPE_GRID     6

#define XWT_TYPE_BUTTON   10
#define XWT_TYPE_RADIOBUTTON   11
#define XWT_TYPE_CHECKBOX 12
#define XWT_TYPE_TOGGLEBUTTON 13

#define XWT_TYPE_LABEL    20
#define XWT_TYPE_MENU     30
#define XWT_TYPE_MENUITEM 31
#define XWT_TYPE_TEXTBOX  40
#define XWT_TYPE_TEXTAREA 41
#define XWT_TYPE_IMAGE    50
#define XWT_TYPE_FILESEL  60
#define XWT_TYPE_SPLITTER 70
#define XWT_TYPE_TREEITEM 80
#define XWT_TYPE_TREELIST 81
#define XWT_TYPE_BROWSE   82
#define XWT_TYPE_FONTSEL  90
#define XWT_TYPE_PROGRESSBAR 100
#define XWT_TYPE_CALENDAR 110
#define XWT_TYPE_CALENDARM 120
#define XWT_TYPE_COMBOBOX    130
#define XWT_TYPE_LISTBOX    140
#define XWT_TYPE_COLORSELECT 150
// Container of Widgets ---------------------------

#endif

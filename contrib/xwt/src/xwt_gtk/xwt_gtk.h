/*
   XWT - xHarbour Windowing Toolkit

   (C) 2003 Giancarlo Niccolai

   $Id: xwt_gtk.h 9279 2011-02-14 18:06:32Z druzus $

   GTK interface
*/

#ifndef XWT_GTK_H
#define XWT_GTK_H

#include <gtk/gtk.h>
#include <xwt_api.h>

#define XWT_GTK_MAKESELF( var )\
   HB_ITEM Self; \
   Self.type = HB_IT_OBJECT;\
   Self.item.asArray.value = (PHB_BASEARRAY ) (var);

gboolean xwt_idle_function( gpointer data );
void *xwt_gtk_get_topwidget_neuter( void *);

typedef struct tag_xwt_gtk_base
{
   GtkWidget *main_widget;
} XWT_GTK_BASE, *PXWT_GTK_BASE;

typedef struct tag_xwt_gtk_modal
{
  XWT_GTK_BASE a;
  BOOL modal;
  BOOL canceled;
} XWT_GTK_MODAL, *PXWT_GTK_MODAL;

typedef struct tag_xwt_gtk_splitter
{
   XWT_GTK_BASE a;
   GtkWidget *first_widget;
   GtkWidget *second_widget;
   BOOL bShrink1;
   BOOL bShrink2;
} XWT_GTK_SPLITTER, *PXWT_GTK_SPLITTER;

typedef struct tag_xwt_gtk_wnd
{
   XWT_GTK_BASE a;
   GtkWidget *window;
} XWT_GTK_WND, *PXWT_GTK_WND;

typedef struct tag_xwt_gtk_framewnd
{
   XWT_GTK_WND a; 
   GtkWidget *vbox;
   GtkWidget *menu_bar;
   GtkWidget *menu_box;
   GtkWidget *status_bar;
} XWT_GTK_FRAMEWND, *PXWT_GTK_FRAMEWND;


typedef struct tag_xwt_gtk_menuitem
{
   XWT_GTK_BASE a;
   GtkWidget *hbox;
   GtkWidget *image;
   GtkWidget *label;
   GtkWidget *align;
} XWT_GTK_MENUITEM, *PXWT_GTK_MENUITEM;


typedef struct tag_xwt_gtk_align
{
   XWT_GTK_BASE a;
   GtkWidget *align;
   int iVAlign;
   int iHAlign;
} XWT_GTK_ALIGN, *PXWT_GTK_ALIGN;

typedef struct tag_xwt_gtk_sensible
{
   XWT_GTK_ALIGN a;
   GtkWidget *evt_window;
} XWT_GTK_SENSIBLE, *PXWT_GTK_SENSIBLE;

typedef struct tag_xwt_gtk_image
{
   XWT_GTK_SENSIBLE a;
   GdkPixmap *pixmap;
   char *filename;
} XWT_GTK_IMAGE, *PXWT_GTK_IMAGE;


typedef struct tag_xwt_gtk_container
{
   XWT_GTK_ALIGN a;
   GtkWidget *frame;
} XWT_GTK_CONTAINER, *PXWT_GTK_CONTAINER;


typedef struct tag_xwt_gtk_laycontainer
{
   XWT_GTK_CONTAINER a;
   BOOL bFill;
   BOOL bExpand;
} XWT_GTK_LAYCONTAINER, *PXWT_GTK_LAYCONTAINER;


typedef struct tag_xwt_gtk_layout
{
   XWT_GTK_LAYCONTAINER a;
   
   int iMode;
   int iPadding;
} XWT_GTK_LAYOUT, *PXWT_GTK_LAYOUT;


typedef struct tag_xwt_gtk_grid
{
   XWT_GTK_LAYCONTAINER a;
   int iRows;
   int iCols;
   BOOL bShrink;
   int iYPad, iXPad;
} XWT_GTK_GRID, *PXWT_GTK_GRID;

typedef struct tag_xwt_gtk_calendar
{
  gboolean  settings[5];
  GtkWidget *window;
  GtkWidget *calendar;

  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *hbbox;  
  GtkWidget *button;
  GtkWidget *button1;
  GtkWidget *frame;
  GtkWidget *bbox;
} XWT_GTK_CALENDAR, *PXWT_GTK_CALENDAR;




BOOL xwt_gtk_createButton( PXWT_WIDGET wSelf );
BOOL xwt_gtk_createFrameWindow( PXWT_WIDGET wSelf );
BOOL xwt_gtk_createLabel( PXWT_WIDGET wSelf );
BOOL xwt_gtk_createMenu( PXWT_WIDGET wSelf );
BOOL xwt_gtk_createMenuItem( PXWT_WIDGET wSelf );
BOOL xwt_gtk_createPane( PXWT_WIDGET wSelf );
BOOL xwt_gtk_createTextBox( PXWT_WIDGET wSelf );
BOOL xwt_gtk_createWindow( PXWT_WIDGET wSelf );
BOOL xwt_gtk_createImage( PXWT_WIDGET wSelf );
BOOL xwt_gtk_createLayout( PXWT_WIDGET wSelf );
BOOL xwt_gtk_createGrid( PXWT_WIDGET wSelf );
BOOL xwt_gtk_createViewPort( PXWT_WIDGET wSelf );
BOOL xwt_gtk_createRadioButton( PXWT_WIDGET wSelf );
BOOL xwt_gtk_createFileSelection( PXWT_WIDGET wSelf );
BOOL xwt_gtk_createCheckbox( PXWT_WIDGET wSelf );
BOOL xwt_gtk_createSplitter( PXWT_WIDGET wSelf );
BOOL xwt_gtk_createToggleButton( PXWT_WIDGET wSelf );
BOOL xwt_gtk_createTreelist( PXWT_WIDGET wSelf );
BOOL xwt_gtk_createBrowse( PXWT_WIDGET wSelf );
BOOL xwt_gtk_createFontSelection( PXWT_WIDGET wSelf );
BOOL xwt_gtk_createCalendar( PXWT_WIDGET wSelf );
BOOL xwt_gtk_createCalendarModal( PXWT_WIDGET wSelf );
BOOL xwt_gtk_createComboBox( PXWT_WIDGET wSelf );
BOOL xwt_gtk_createListBox( PXWT_WIDGET xwtData );
BOOL xwt_gtk_createProgressBar( PXWT_WIDGET xwtData );
BOOL xwt_gtk_createColorSelection( PXWT_WIDGET xwtData );
BOOL xwt_gtk_createNoteBook( PXWT_WIDGET xwtData );
BOOL xwt_gtk_createEditBox( PXWT_WIDGET xwtData );
void xwt_gtk_ComboAddItem(PXWT_WIDGET xwtData, PHB_ITEM pComboArray );
void xwt_gtk_setMenuBar( PXWT_WIDGET xwtData, PHB_ITEM pMenuArray );
void xwt_gtk_resetMenuBar( PXWT_WIDGET xwtData, PHB_ITEM pMenuArray );
BOOL xwt_gtk_imageLoad( PXWT_WIDGET xwtData, const char *fname );
BOOL xwt_gtk_image_setSensible( PXWT_WIDGET wSelf );
void xwt_gtk_ListAddItem(PXWT_WIDGET xwtData, PHB_ITEM pComboArray );

BOOL xwt_gtk_layout_create_with_mode( PXWT_WIDGET wWidget, int mode );
BOOL xwt_gtk_splitter_create_with_mode( PXWT_WIDGET wWidget, int mode );

BOOL xwt_gtk_container_set_box( PXWT_WIDGET wWidget );
BOOL xwt_gtk_container_reset_box( PXWT_WIDGET wWidget );

void *container_get_mainwidget( void *data );
void *container_get_topwidget( void *data );

void *xwt_gtk_get_topwidget_neuter( void *data );
void *xwt_gtk_get_mainwidget_base( void *data );
void *xwt_gtk_get_topwidget_base( void *data );
void *xwt_gtk_get_topwidget_align( void *data );
void *xwt_gtk_get_topwidget_sensible( void *data );

BOOL xwt_gtk_treelist_set_content( PXWT_WIDGET xwtData, PHB_ITEM pContent );
BOOL xwt_gtk_treelist_set_columns( PXWT_WIDGET xwtData, PHB_ITEM pCols );
BOOL xwt_gtk_treelist_create_columns( PXWT_WIDGET xwtData, int nCols );
BOOL xwt_gtk_treelist_set_colattr( PXWT_WIDGET xwtData, char *prop, void *data );

/* In the browse model, all the data we need is in the owner (XWTBrowse). */
BOOL xwt_gtk_browse_set_content( PXWT_WIDGET xwtData );

/*** Putting a widget in a frame ****/
GtkWidget *xwt_gtk_enframe( GtkWidget *framed );
void xwt_gtk_deframe( GtkWidget *frame, GtkWidget *framed );

/*** XWT GTK ALIGNMENT ***/
void xwt_gtk_set_alignment( XWT_GTK_ALIGN* widget );

/*** Type translation between xharbour and gtk type representation ***/
int xwt_gtk_translate_type( int iType );

#endif

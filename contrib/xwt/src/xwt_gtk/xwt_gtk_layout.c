/*
   XWT_GTK - xHarbour Windowing Toolkit/ GTK interface

   (C) 2003 Giancarlo Niccolai

   $Id: xwt_gtk_layout.c 9279 2011-02-14 18:06:32Z druzus $

   Layout - Horizontal or vertical layout manager
*/
#include "hbapi.h"
#include <xwt_api.h>
#include <xwt_gtk.h>

BOOL xwt_gtk_createLayout( PXWT_WIDGET xwtData )
{
   // We can't create the widget right now, as we need to know how the widget
   // will be layed (horiz/vert): they are 2 different layout systems
   PXWT_GTK_LAYOUT gtkLayout;

   gtkLayout = ( PXWT_GTK_LAYOUT ) hb_xgrab( sizeof( XWT_GTK_LAYOUT ) );
   gtkLayout->iMode = -1; // still undefined

   gtkLayout->a.a.frame = NULL; // no frame for now
   gtkLayout->a.a.a.align = NULL; // no frame for now
   gtkLayout->a.a.a.iHAlign = XWT_ALIGN_CENTER; // no frame for now
   gtkLayout->a.a.a.iVAlign = XWT_ALIGN_TOP; // no frame for now

   gtkLayout->a.a.a.a.main_widget = NULL; // still not available

   gtkLayout->iPadding = 0;
   gtkLayout->a.bFill = FALSE;
   gtkLayout->a.bExpand = FALSE;

   // no need for destructor, the data is just our widget for now
   xwtData->widget_data = (void *) gtkLayout;
   xwtData->destructor = hb_xfree;
   xwtData->get_main_widget = xwt_gtk_get_mainwidget_base;
   xwtData->get_top_widget = container_get_topwidget;

   return TRUE;
}

/* This function creates the real gtk widget. */
BOOL xwt_gtk_layout_create_with_mode( PXWT_WIDGET wWidget, int mode  )
{
   PXWT_GTK_LAYOUT lay = (PXWT_GTK_LAYOUT ) wWidget->widget_data;

   if ( lay->iMode != -1 )
   {
      return FALSE;
   }

   lay->iMode = mode;
   if ( mode == XWT_LM_HORIZ )
   {
      lay->a.a.a.a.main_widget = gtk_hbox_new( FALSE, 0 );
   }
   else
   {
      lay->a.a.a.a.main_widget = gtk_vbox_new( FALSE, 0 );
   }
   gtk_widget_show( lay->a.a.a.a.main_widget );

   return TRUE;
 }

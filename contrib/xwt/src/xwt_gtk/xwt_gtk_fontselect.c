/*
   XWT - xHarbour Windowing Toolkit

   (C) 2003 Luiz Rafael Culik

   $Id: xwt_gtk_fontselect.c 9280 2011-02-14 18:16:32Z druzus $

   GTK interface - File Selection Box 
*/

#include "hbapi.h"
#include "hbapiitm.h"
#include <xwt_api.h>
#include <xwt_gtk.h>

static void font_ok_sel( GtkWidget *widget,  gpointer cb_data )
{
   PXWT_GTK_MODAL xwtFilew = (PXWT_GTK_MODAL) ((PXWT_WIDGET)cb_data)->widget_data;
   HB_ITEM itmFileName;
   const char *fname;
   // this builds the Self object
   // If you use this macro, you must put it AFTER variable decl,
   // and BEFORE any other statement
   XWT_GTK_MAKESELF( (((PXWT_WIDGET)cb_data)->owner) );
   fname = gtk_font_selection_dialog_get_font_name( GTK_FONT_SELECTION( xwtFilew->a.main_widget ) );

   // itemPutC uses the char* parameter as it were const: it does not
   // mangles with that, it just creates a new local copy of the param.
   hb_itemPutC( &itmFileName, ( char *) fname );

   //rising the updated event, to signal that we have a candidate filename
   xwt_rise_event( &Self, XWT_E_UPDATED, 1, &itmFileName );

   // now we can reset the modal status
   xwtFilew->modal = FALSE;
}

static void font_cancel_sel( GtkWidget *widget,  gpointer cb_data )
{
   PXWT_GTK_MODAL xwtFilew = (PXWT_GTK_MODAL) ((PXWT_WIDGET)cb_data)->widget_data;

   XWT_GTK_MAKESELF( (((PXWT_WIDGET)cb_data)->owner) );

   //rising the updated event, to signal that we have a candidate filename
   xwt_rise_event( &Self, XWT_E_CANCELED, 0);

   // now we can reset the modal status
   xwtFilew->modal = FALSE;
   // and say we have been canceled
   xwtFilew->canceled = TRUE;
}

BOOL xwt_gtk_createFontSelection( PXWT_WIDGET xwtData )
{
   GtkWidget *filew;
   PXWT_GTK_MODAL xwtFilew;

   xwtFilew = (PXWT_GTK_MODAL) hb_xgrab( sizeof( XWT_GTK_MODAL ) );

   filew = gtk_font_selection_dialog_new("");
   // this widget is NOT displayed by default
   xwtFilew->a.main_widget = filew;
   xwtFilew->modal = FALSE;
   xwtFilew->canceled = FALSE;

   // we need both the owner of the widget, and the widget itself;
   // so it is useful to pass the xwt_gtk data.
   g_signal_connect (
      G_OBJECT (GTK_FONT_SELECTION_DIALOG (filew)->ok_button),
      "clicked", G_CALLBACK (font_ok_sel), xwtData
   );

   g_signal_connect(
      G_OBJECT (GTK_FONT_SELECTION_DIALOG (filew)->cancel_button),
      "clicked", G_CALLBACK (font_cancel_sel),xwtData);

   // you ALWAYS need to set the xwtData->widget_data.
   // if no driver level widget wrapper is needed, you can
   // use the gtkWidget here, and set NULL for the destructor.
   xwtData->widget_data = xwtFilew;
   // xwtData->widget_data is just allocated with hb_xgrab;
   // an xfree will be enough to get rid of it.
   xwtData->destructor = hb_xfree;
   xwtData->get_main_widget = xwt_gtk_get_mainwidget_base;
   xwtData->get_top_widget = xwt_gtk_get_mainwidget_base;

   return TRUE;
}


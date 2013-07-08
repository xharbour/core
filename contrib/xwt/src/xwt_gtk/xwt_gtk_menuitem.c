/*
   XWT_GTK - xHarbour Windowing Toolkit/ GTK interface

   (C) 2003 Giancarlo Niccolai

   $Id: xwt_gtk_menuitem.c 9279 2011-02-14 18:06:32Z druzus $

   Menu item managemetn
*/
#include "hbapi.h"
#include <xwt_api.h>
#include <xwt_gtk.h>

static void mi_activate( GtkWidget *widget,  gpointer cb_data )
{
   XWT_GTK_MAKESELF( cb_data );
   xwt_rise_event( &Self, XWT_E_CLICKED, 0 );
}

/* Currently, menuitems are not containers */
static void *mi_get_topwidget( void *data )
{
   return data;
}


BOOL xwt_gtk_createMenuItem( PXWT_WIDGET xwtData )
{
   PXWT_GTK_MENUITEM menuitem;

   menuitem = (PXWT_GTK_MENUITEM) hb_xgrab( sizeof( XWT_GTK_MENUITEM ) );
   menuitem->a.main_widget = gtk_menu_item_new ();
   menuitem->hbox = gtk_hbox_new( FALSE, 2 );
   menuitem->image = gtk_image_new();
   menuitem->label = gtk_label_new("");
   menuitem->align = gtk_alignment_new( 0.0, 0.5, 0.0, 0.0 );
   gtk_container_add( GTK_CONTAINER( menuitem->hbox), menuitem->image );
   gtk_container_add( GTK_CONTAINER( menuitem->hbox ), menuitem->label );
   gtk_container_add( GTK_CONTAINER( menuitem->align), menuitem->hbox );

   gtk_container_add( GTK_CONTAINER( menuitem->a.main_widget), menuitem->align );

   gtk_widget_show( menuitem->hbox );
   gtk_widget_show( menuitem->label );
   gtk_widget_show( menuitem->image );
   gtk_widget_show( menuitem->align );


   // add a container to the window
   g_signal_connect (G_OBJECT (menuitem->a.main_widget), "activate", G_CALLBACK (mi_activate),
      xwtData->owner );
   gtk_widget_show( menuitem->a.main_widget );

   // no need for destructor, the data is just our widget for now
   xwtData->widget_data = menuitem;
   xwtData->destructor = hb_xfree;
   xwtData->get_main_widget = xwt_gtk_get_topwidget_base;
   xwtData->get_top_widget = xwt_gtk_get_topwidget_base;

   return TRUE;
}

/*
   XWT - xHarbour Windowing Toolkit

   (C) 2004 Luiz Rafael Culik

   $Id: xwt_gtk_listbox.c 9279 2011-02-14 18:06:32Z druzus $

   GTK interface - management of combobox
*/

#include "hbapi.h"
#include "hbapiitm.h"
#include <xwt_api.h>
#include <xwt_gtk.h>

static void listbox_list_append(GtkWidget *combo,char * string)
{
   GtkWidget *li=gtk_list_item_new_with_label(string);
   gtk_container_add(GTK_CONTAINER(combo),li);
   gtk_widget_show(li);
}


void xwt_gtk_ListAddItem(PXWT_WIDGET xwtData, PHB_ITEM pComboArray )
{
   GtkWidget * combo;
   PHB_ITEM pItem;
   ULONG ulPos;
   combo  = (GtkList*) xwtData->widget_data;

   for ( ulPos = 0; ulPos < hb_arrayLen(pComboArray); ulPos++ )
   {  
      pItem = hb_arrayGetItemPtr( pComboArray, ulPos + 1 );
      listbox_list_append(combo,hb_itemGetCPtr(pItem));
   }      

}


static void box_changed( GtkWidget *widget,  gpointer cb_data )
{
   PHB_ITEM pString = hb_itemNew( NULL );
   XWT_GTK_MAKESELF( cb_data );

   hb_itemPutC( pString, (char *)gtk_entry_get_text( GTK_ENTRY( widget ) ) );
   xwt_rise_event( &Self, XWT_E_CHANGED, 1, pString );
   hb_itemRelease( pString );
}

static void box_activate( GtkWidget *widget,  gpointer cb_data )
{
   PHB_ITEM pString = hb_itemNew( NULL );
   XWT_GTK_MAKESELF( cb_data );

   hb_itemPutC( pString, (char *)gtk_entry_get_text( GTK_ENTRY( widget ) ) );
   if ( ! xwt_rise_event( &Self, XWT_E_TEXT, 1, pString ) )
   {
      xwt_rise_event( &Self, XWT_E_UPDATED, 0 );
   }
   hb_itemRelease( pString );
}

BOOL xwt_gtk_createListBox( PXWT_WIDGET xwtData )
{
   GtkWidget *button;

   button = gtk_list_new ();
   // add a container to the window

//   g_signal_connect (GTK_ENTRY(GTK_COMBO(button)->entry), "activate", G_CALLBACK (box_activate), xwtData->owner );
//   g_signal_connect (GTK_ENTRY(GTK_COMBO(button)->entry), "changed", G_CALLBACK (box_changed), xwtData->owner );

   xwtData->widget_data = button;
   xwtData->destructor = NULL;
   xwtData->get_main_widget = xwtData->get_top_widget = xwt_gtk_get_topwidget_neuter;

   gtk_widget_show( button );

   return TRUE;
}


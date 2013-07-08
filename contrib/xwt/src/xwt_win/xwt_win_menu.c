/*
   XWT_WIN - xHarbour Windowing Toolkit/ MS-Windows interface

   (C) 2003 Giancarlo Niccolai

   $Id: xwt_win_menu.c 9279 2011-02-14 18:06:32Z druzus $

   Menu management
*/

#include "hbapi.h"
#include <xwt_api.h>
#include <xwt_win.h>


BOOL xwt_win_createMenu( PXWT_WIDGET xwtData )
{
   PXWT_WIN_MENUDATA data;

   data = (PXWT_WIN_MENUDATA) hb_xgrab( sizeof( XWT_WIN_MENUDATA ) );
   data->hBitmap = NULL;
   data->szLabel = NULL;
   
   // no need for destructor, the data is just our widget for now
   xwtData->widget_data = (void *) data;
   xwtData->destructor = hb_xfree;
   xwtData->get_main_widget = xwt_win_get_neuter;
   xwtData->get_top_widget = xwt_win_get_neuter;

   return TRUE;
}

#if 0
BOOL xwt_win_createMenuItem( PXWT_WIDGET xwtData )
{
   PXWT_WIN_MENUDATA data;

   data = (PXWT_WIN_MENUDATA) hb_xgrab( sizeof( XWT_WIN_MENUDATA ) );
   data->hBitmap = NULL;
   data->szLabel = NULL;
   
   // no need for destructor, the data is just our widget for now
   xwtData->widget_data = (void *) data;
   xwtData->destructor = xwt_win_delete_menu;
   xwtData->get_main_widget = xwt_win_get_neuter;
   xwtData->get_top_widget = xwt_win_get_neuter;

   return TRUE;
}
#endif

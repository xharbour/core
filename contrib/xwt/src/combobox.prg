/*
   XWT - xHarbour Windowing Toolkit

   (C) 2004 Luiz Rafael Culik

   $Id: combobox.prg 9279 2011-02-14 18:06:32Z druzus $

   Widget class - basic widget & event management
*/

#include "hbclass.ch"
#include "xwt.ch"

CLASS XWTComboBox FROM XWTWidget
   Data aItems
   METHOD New( cText, bStatus )
   METHOD GetItem()
ENDCLASS

METHOD New( cText, nX, nY, oParent ,aItems ) CLASS XWTComboBox

   LOCAL bVis := .T.

   ::Super:New()
   ::nWidgetType := XWT_TYPE_COMBOBOX
   ::oRawWidget := XWT_Create( Self, XWT_TYPE_COMBOBOX )
   
   IF ValType( aItems )  == "A"
   ::aItems :=  aItems
      XWT_SetProperty(::oRawWidget,XWT_PROP_SETCOMBOITEMS,aItems)
   ENDIF  
   
   IF .not. Empty( cText )
      XWT_SetProperty( ::oRawWidget, XWT_PROP_TEXT, cText )
   ENDIF

   
   IF ValType( nX ) == "N" .and. ValType( nY ) == "N"
      ::Move( nX, nY )
   ENDIF

   IF oParent != NIL
      oParent:Add( Self )
   ENDIF

RETURN Self

METHOD GetiTem() CLASS XWTComboBox
   LOCAL cItem

   IF XWT_GetProperty( ::oRawWidget, XWT_PROP_TEXT, @cItem )
   tracelog(cItem)
      RETURN cItem
   ENDIF

RETURN ""

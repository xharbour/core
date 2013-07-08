/*
   XWT - xHarbour Windowing Toolkit

   (C) 2003 Luiz Rafael

   $Id: filesel.prg 9279 2011-02-14 18:06:32Z druzus $

   Widget class - basic widget & event management
*/

#include "hbclass.ch"
#include "xwt.ch"

CLASS XWTFileSel FROM XWTWidget
   METHOD New( cText, cFileName )
   METHOD GetFile()
   METHOD SetFile( cFileName )
   METHOD DoModal()
ENDCLASS

METHOD New( cText, cFileName ) CLASS XWTFileSel
   ::Super:New()
   ::nWidgetType := XWT_TYPE_FILESEL
   ::oRawWidget := XWT_Create( Self, XWT_TYPE_FILESEL )
   IF .not. Empty( cText )
      XWT_SetProperty( ::oRawWidget, XWT_PROP_TEXT, cText )
   ENDIF
   IF .not. Empty( cFileName )
      ::SetFile( cFileName )
   ENDIF
RETURN Self

METHOD GetFile() Class XWTFileSel
   LOCAL cFile := Space( 1024 )

   IF  XWT_GetProperty( ::oRawWidget, XWT_PROP_FILENAME, @cFile)
      RETURN cFile
   ENDIF

RETURN ""


METHOD SetFile( cFileName ) Class XWTFileSel
   IF  XWT_SetProperty( ::oRawWidget, XWT_PROP_FILENAME, cFileName )
      RETURN .T.
   ENDIF
RETURN .F.


METHOD DoModal() Class XWTFileSel
   LOCAL cFile

   ::Show()
   XWT_Modal( ::oRawWidget )
   cFile := ::GetFile()
   ::Destroy()

RETURN cFile

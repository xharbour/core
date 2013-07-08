/*
 * $Id: lightning.prg 9561 2012-07-12 02:07:53Z andijahja $
 */

#message "This program requires freetype6.dll"
#message "This program requires libcairo-2.dll"
#message "This program requires libexpat-1.dll"
#message "This program requires libfontconfig-1.dll"
#message "This program requires libpng14-14.dll"
#message "This program requires zlib1.dll"
#message "Download the binaries at: http://www.gtk.org/download/win32.php"
#message "Create an import lib of libcairo-2.dll and link it to application"
#message ""

#include "hbcairo.ch"

PROC main()
   LOCAL hSurface, hCairo, nI

   hSurface := cairo_pdf_surface_create( "lightning.pdf", 566.9, 793.7 )  // 200x280 mm in pt
   hCairo := cairo_create( hSurface )

   // 20 pages
   FOR nI := 1 TO 20
     cairo_set_source_rgb( hCairo, 0, 0, 0 ) 
     cairo_paint( hCairo )
     cairo_set_source_rgb( hCairo, 1, 0.7, 1 ) 
     DrawLightning( hCairo, 250, 50, 700, 3 )
     cairo_show_page( hCairo )
   NEXT
   cairo_destroy( hCairo )
   cairo_surface_destroy( hSurface )
RETURN


PROC DrawLightning( hCairo, nX, nY, nLen, nW, nInit )
   LOCAL nI, nK, nW0, nX2

   cairo_move_to( hCairo, nX, nY )
   nW0 := nW
   IF nInit == NIL
      nInit := 0
   ENDIF
   nK := 0
   FOR nI := 1 TO nLen
     // AR(1) process
     nInit := nInit * 0.9 + (hb_random() - 0.5)
     // ARIMA(1, 1, 0) process
     nK += nInit 
     // ARIMA(1, 1, 0) + white noise
     nX2 := nX + nK + hb_random()
     cairo_line_to( hCairo, nX2, nI + nY )
     nW -= 0.003
     IF nW < nW0 - 0.1
        cairo_set_line_width( hCairo, nW0 )
        cairo_stroke( hCairo )
        cairo_move_to( hCairo, nX2, nI + nY )
        nW0 := nW
     ENDIF
     // Branch
     IF ABS(nInit) > 1.6
        cairo_set_line_width( hCairo, nW0 )
        cairo_stroke( hCairo )
        DrawLightning( hCairo, nX2, nI + nY, INT((nLen - nI) / 2), nW / 2, nInit * 0.5 )
        nInit *= -0.3
        cairo_move_to( hCairo, nX2, nI + nY )
     ENDIF
   NEXT
   cairo_set_line_width( hCairo, nW0 )
   cairo_stroke( hCairo )
RETURN

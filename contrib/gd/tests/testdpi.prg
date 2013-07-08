/*
 * GD graphic library.
 * graphic font DPI demo
 *
 * Copyright 2005 Francesco Saverio Giudice <info@fsgiudice.com>
 */

#include "gd.ch"
#include "common.ch"

#define IMAGES_IN  "images_in/"
#define IMAGES_OUT "images_out/"

PROCEDURE Main()

 oI := GDImage( 600, 300 )

 white := oI:SetColor( 255, 255, 255 )
 black := oI:SetColor( 0, 0, 0 )

 oI:SetColor( black )
 oI:SetFontName("c:\windows\fonts\arial.ttf")
 oI:SetFontPitch( 10 )

 // Resolution = 96 dpi, default
 oI:SayFreeType( 10, 100, "GD_RESOLUTION:  96 dpi" )

 // Resolution = 150 dpi, using parameter 12
 oI:SayFreeType( 10, 150, "GD_RESOLUTION: 150 dpi",,,,,,,, 150 )

 // Resolution = 300 dpi, using parameter 12
 oI:SayFreeType( 10, 200, "GD_RESOLUTION: 300 dpi",,,,,,,, 300 )

 oI:SavePng(  IMAGES_OUT + "testdpi.png" )
 oI:SaveJpeg( IMAGES_OUT + "testdpi.jpg" )
 oI:SaveGif(  IMAGES_OUT + "testdpi.gif" )

 RETURN


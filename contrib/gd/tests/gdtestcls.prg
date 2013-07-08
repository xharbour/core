/*
 * $Id: gdtestcls.prg 9279 2011-02-14 18:06:32Z druzus $
 */

/*
 * Copyright 2004-2005 Francesco Saverio Giudice <info@fsgiudice.com>
 *
 * GD Class test file
 */


#include "gd.ch"
#include "common.ch"

#define IMAGES_IN  "images_in/"
#define IMAGES_OUT "images_out/"

PROCEDURE Main()

   LOCAL im, im2
   LOCAL black, white, blue, red, green, cyan, gray
   LOCAL aClip, color, font, aRect
   LOCAL oI, oI2, oI3, oI4, nThick, n, nSecs
   LOCAL oI5
   LOCAL oB

   // To set fonts run this command:
   // for windows: SET GDFONTPATH=c:\windows\fonts
   // per linux  : export GDFONTPATH=/usr/share/fonts/default/TrueType

   // SET GDFONTPATH=c:\windows\fonts
   IF GetEnv( "GDFONTPATH" ) == ""
      ? "Please set GDFONTPATH"
      ? "On Windows: SET GDFONTPATH=c:\windows\fonts"
      ? "On Linux  : export GDFONTPATH=/usr/share/fonts/default/TrueType"
      ?
   ENDIF

   // Check output directory
   IF !ISDirectory( IMAGES_OUT )
      DirMake( IMAGES_OUT )
   ENDIF

   /* Create an image in memory */
   oI := GDImage( 200, 200 )

   /* Load an image in memory from file */
   oI2 := GDImage():LoadFromJpeg( IMAGES_IN + "fsg.jpg" )
   oI5 := GDImage():LoadFromJpeg( IMAGES_IN + "xharbour.jpg" )

   /* Now work on first empty image */

   /* Allocate background */
   white := oI:SetColor( 255, 255, 255 )

   /* Allocate drawing color */
   black := oI:SetColor( 0, 0, 0 )
   blue  := oI:SetColor( 0, 0, 255 )
   red   := oI:SetColor( 255, 0, 0 )
   green := oI:SetColor( 0, 255, 0 )
   cyan  := oI:SetColor( 0, 255, 255 )

   /* Draw rectangle */
   oI:Rectangle( 0, 0, 199, 199, .T., cyan )
   oI:Rectangle( 0, 0, 199, 199,, black)

   oI:SetColor( blue )

   /* Draw pixel */
   oI:SetPixel( 50, 5 )

   /* Draw lines */
   oI:Line( 0, 0, 199, 199, blue )
   oI:DashedLine(0, 199, 199, 0, blue)
   nThick := oI:SetThickness( 5 )
   oI:Line( 50, 150, 100, 150 )
   oI:SetThickness( nThick )

   oI:AddStyle( red )
   oI:AddStyle( red )
   oI:AddStyle( red )
   oI:AddStyle( gdTransparent )
   oI:AddStyle( gdTransparent )
   oI:AddStyle( gdTransparent )
   oI:SetStyle()
   oI:Line( 50, 180, 100, 180, gdStyled )

   oI:ResetStyles()
   oI:AddStyle( black )
   oI:AddStyle( gdTransparent )
   oI:SetStyle()
   oI:Line( 50, 185, 100, 185, gdStyled )


   /* Draw polygons */
   oI:AddPoint( 10, 10 )
   oI:AddPoint( 70, 10 )
   oI:AddPoint( 80, 60 )
   oI:Polygon()

   oI:ResetPoints()
   oI:AddPoint( 160, 180 )
   oI:AddPoint( 170, 110 )
   oI:AddPoint( 150, 160 )
   oI:Polygon(,.T., green)

   /* Draw an arc */
   oI:Arc(50, 50, 40, 40, 30, 190,, red )
   oI:Circle(50, 150, 45, .t., green )
   oI:Ellipse(120, 120, 50, 20, , green )

   /* Draw a character. */
   oI:SetFontLarge()
   ? "Font Dims", oI:GetFontWidth(), oI:GetFontHeight()
   oI:SetColor( black )
   //__OutDebug( "Font", font )
   oI:Say( 0, 0, 'Test')
   oI:Say( 0, 15, 'P')
   oI:Say( 0, 30, 'W')

   oI:SayVertical( 70, 90, 'Test')
   oI:SayVertical( 70, 15, 'P')
   oI:SayVertical( 70, 30, 'W')

   oI:SayFreeType( 20, 30, "Test", "arial", 24, 15 )
   oI:SayFreeType( 40, 70, "Test2" )


   /* Set Clip Rectangle */
   oI:SetClippingArea(25, 25, 75, 75)

   /* Retrieve Clipping rectangle */
   aClip := oI:GetClippingArea()
   ? "Clipping rectangle values"
   ? hb_DumpVar( aClip )

   /* Query functions */

   color := oI:GetPixel( oI:Width() / 2, oI:Height() / 2)
   ? "Pixel Color is: ", color
   ? "RGB Values: ", oI:Red(color), oI:Green(color), oI:Blue(color)
   ? "Alpha Value: ",  oI:Alpha(color)

   /* Write Images on files */
   oI:SavePng( IMAGES_OUT + "rect.png" )
   oI2:SavePng( IMAGES_OUT + "test.png" )
   oI2:SaveJpeg( IMAGES_OUT + "test.jpg" )
   oI2:SaveGif( IMAGES_OUT + "test.gif" )
   //oI2:SaveWBmp( IMAGES_OUT + "vale1.bmp", black )

   /* test copy functions */

   //oI3 := GDImage():CreateTrueColor( oI2:Width * 2, oI2:Height * 2 )
   //oI2:CopyResampled( 0, 0, oI2:Width, oI2:Height, 0, 0, oI3:Width, oI3:Height, oI3 )
   //oI3:SaveJpeg("vale2.jpg")


   nSecs := Seconds()
   ? "start copy zoomed"
   oI3 := oI2:CopyZoomed( 150 )
   ? "end", Seconds() - nSecs
   nSecs := Seconds()
   ? "start save"
   oI3:SaveJpeg( IMAGES_OUT + "zoom.jpg" )
   ? "end", Seconds() - nSecs

   nSecs := Seconds()
   ? "start clone & zoom"
   oI4:= oI2:Clone():Zoom( 200 )
   ? "end", Seconds() - nSecs

   nSecs := Seconds()
   ? "start clone"
   oI4:= oI5:Clone()
   ? "end", Seconds() - nSecs

   nSecs := Seconds()
   ? "start zoom"
   oI4:Zoom( 200 )
   ? "end", Seconds() - nSecs

   //__OutDebug( oI2:pImage )
   //oI4:SetFontGiant()
   gray := oI4:SetColor(30, 30, 30)
   blue := oI4:SetColor(0, 0, 200)

   //oI4:SetColor( black )
   //oI4:Say( 100, 10, "Valentina" )
   IF OS() = "Linux"
      oI4:SayFreeType( oI4:CenterWidth(), oI4:CenterHeight(), "GD power", "arib____", 40, 45 )
   ELSE
      nSecs := Seconds()
      ? "start write"
      FOR n := 0 TO 350 STEP 10
          oI4:SayFreeType( oI4:CenterWidth(), oI4:CenterHeight(), "             GD Font Power", "arial", 20, n )
      NEXT
      ? "end", Seconds() - nSecs
      oI4:SetTransparent( blue )
      oI4:SayFreeType( oI4:CenterWidth()-4, oI4:CenterHeight()+4, "xHarbour", "verdana", 70, n, gray )
      oI4:SayFreeType( oI4:CenterWidth(), oI4:CenterHeight(), "xHarbour", "verdana", 70, n, blue )
   ENDIF
   oI4:SaveJpeg( IMAGES_OUT + "writing.jpg" )


   //oI4 := __ObjClone( oI2 )
   oI4 := oI2:Clone()

   nSecs := Seconds()
   ? "start rotate outside"
   oI2:Rotate( 45 )
   ? "end", Seconds() - nSecs
   oI2:SaveJpeg( IMAGES_OUT + "rotateout.jpg" )

   nSecs := Seconds()
   ? "start rotate inside"
   oI4:RotateInside( 45 )
   ? "end", Seconds() - nSecs
   //oI2:CopyRotated( , , , , , , 90, oI4 )
   oI4:SaveJpeg( IMAGES_OUT + "rotatein.jpg" )


   oI5:Zoom( 40 )
   //oI5:Rotate( 90 )
   blue := oI5:SetColor(0, 0, 200)
   oI5:SayFreeType( oI5:CenterWidth(), oI5:CenterHeight(), "xHarbour", "verdana", 20, 0, blue )
   oI5:SaveJpeg( IMAGES_OUT + "xh_zoom.jpg" )


   oI5 := GDChart( 400, 400 )
   // Set background
   white := oI5:SetColor(255, 255, 255)
   // Define piece colors
   blue  := oI5:SetColor(0, 0, 200)
   gray  := oI5:SetColor(30, 30, 30)
   green := oI5:SetColor(0, 250, 0)
   red   := oI5:SetColor(250, 0, 0)

   // Load an image as brush
   oB := GDImage():LoadFromGif( IMAGES_IN + "italia.gif" )
   oB:Zoom(15)

   //oI5:Circle( 200, 200, oI5:Width() )
   //oI5:Line( 0, 200, 200, 200 )

   oI5:AddDef( "FONTPITCH", "GIANT" )

   oI5:SetData( { ;
                   { "LABEL" => "One"  , "VALUE" => 10, "COLOR"  => blue , "FILLED" => TRUE, "EXTRUDE" => 40/*, "TILE" => oB*/ },;
                   { "LABEL" => "Two"  , "VALUE" => 35, "COLOR"  => gray , "FILLED" => TRUE, "FONT" => { "NAME" => "Verdana", "PITCH" => 12, "ANGLE" => 0, "COLOR" => red }  },;
                   { "LABEL" => "Three", "VALUE" => 55, "COLOR"  => green, "FILLED" => TRUE }, ;
                   { "LABEL" => "Four" , "VALUE" => 55, "FILLED" => TRUE , "TILE"   => oB }, ;
                   { "LABEL" => "Five" , "VALUE" => 55, "COLOR"  => red  , "FILLED" => TRUE, "EXTRUDE" => 20}, ;
                   { "LABEL" => "Six"  , "VALUE" => 55, "FILLED" => TRUE , "TILE"   => oB }, ;
                   { "LABEL" => "Seven", "VALUE" => 55, "FILLED" => TRUE , "COLOR"  => green } ;
                } )

   //oI5:VerticalBarChart()
   oI5:PieChart()


   oI5:SaveJpeg( IMAGES_OUT + "pie.jpg" )

   oI5 := GDChart( 640, 480 )
   // Set background
   white := oI5:SetColor(255, 255, 255)
   // Define piece colors
   blue  := oI5:SetColor(0, 0, 200)
   gray  := oI5:SetColor(30, 30, 30)
   green := oI5:SetColor(0, 250, 0)
   red   := oI5:SetColor(250, 0, 0)

   // Load an image as brush
   oB := GDImage():LoadFromJpeg( IMAGES_IN + "fsg.jpg" )
   oB:Zoom(15)

   oI5:AddDef( "MAXVALUE", 150 )
   oI5:AddDef( "AXISPICT", "@E 999" )
   oI5:AddDef( "FONTPITCH", "GIANT" )
   oI5:AddDef( "COLOR", blue )

   //oI5:AddSeries( "LABEL"  => "Primo",;
   //               "VALUES" => { 10, 23, 54, 11, 32, 25 }, ;
   //               "COLOR"  => blue )

   oI5:SetData( { ;
                  { "LABEL" => "One", "VALUE" => 10, "COLOR" => blue, "FILLED" => TRUE, "EXTRUDE" => 40/*, "TILE" => oB*/ },;
                  { "LABEL" => "Two", "VALUE" => 35, "COLOR" => gray, "FILLED" => TRUE, "FONT" => { "NAME" => "Verdana", "PITCH" => 12, "ANGLE" => 0, "COLOR" => red }  },;
                  { "LABEL" => "Three", "VALUE" => 55, "COLOR" => green, "FILLED" => TRUE }, ;
                  { "LABEL" => "Four", "VALUE" => 65, "FILLED" => TRUE, "TILE" => oB }, ;
                  { "LABEL" => "Five", "VALUE" => 34, "FILLED" => TRUE, "COLOR" => green }, ;
                  { "LABEL" => "Six", "VALUE" => 100 }, ;
                  { "LABEL" => "Seven", "VALUE" => 57, "FILLED" => TRUE, "COLOR" => red }, ;
                  { "LABEL" => "Eight", "VALUE" => 22 }, ;
                  { "LABEL" => "Nine", "VALUE" => 36, "COLOR" => blue, "FILLED" => TRUE } ;
                } )
   //oI5:VerticalBarChart()
   //oI5:HorizontalBarChart()
   oI5:LineChart()
   oI5:SaveJpeg( IMAGES_OUT + "hystogram.jpg" )


   //oI4 := GDImage():CreateTrueColor( oI2:Width * 2, oI2:Height * 2 )
   //oI2:CopyResampled( 0, 0, oI2:Width, oI2:Height, 0, 0, oI2:Width, oI2:Height, oI4 )
   //oI2:CopyResampled( 0, 0, oI2:Width, oI2:Height, oI4:CenterWidth(), oI4:CenterHeight(), oI2:Width, oI2:Height, oI4 )
   //oI4:SaveJpeg("vale3.jpg")

   /* Destroy images in memory */
   // Class does it auto

   ?
   ? "Look at " + IMAGES_OUT + " folder for output images"
   ?

RETURN


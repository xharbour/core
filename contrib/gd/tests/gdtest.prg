/*
 * $Id: gdtest.prg 9279 2011-02-14 18:06:32Z druzus $
 */

/*
 * Copyright 2004-2005 Francesco Saverio Giudice <info@fsgiudice.com>
 *
 * GD API test file
 */

#include "gd.ch"
#include "common.ch"

#define IMAGES_IN  "images_in/"
#define IMAGES_OUT "images_out/"

PROCEDURE Main()

   LOCAL im, im2
   LOCAL black, white, blue, red, green, cyan
   LOCAL aClip, color, font, aRect

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

   ? gdVersion()

   /* Create an image in memory */
   im = gdImageCreate(200, 200)

   /* Load an image in memory from file */
   im2 = gdImageCreateFromJpeg( IMAGES_IN + "xharbour.jpg")

   /* Now work on first empty image */

   /* Allocate background */
   white = gdImageColorAllocate(im, 255, 255, 255)

   /* Allocate drawing color */
   black = gdImageColorAllocate(im, 0, 0, 0)
   blue  = gdImageColorAllocate(im, 0, 0, 255)
   red   = gdImageColorAllocate(im, 255, 0, 0)
   green = gdImageColorAllocate(im, 0, 255, 0)
   cyan  = gdImageColorAllocate(im, 0, 255, 255)

   /* Draw rectangle */
   gdImageFilledRectangle(im, 0, 0, 199, 199, cyan)
   gdImageRectangle(im, 0, 0, 199, 199, black)

   /* Draw pixel */
   gdImageSetPixel(im, 50, 5, blue)
   gdImageSetPixel(im, 50, 15, blue)

   /* Draw lines */
   gdImageLine(im, 0, 0, 199, 199, blue)
   gdImageDashedLine(im, 0, 199, 199, 0, blue)

   /* Draw polygons */
   gdImagePolygon(im, { { 10, 10 }, { 70, 10 }, { 80, 60 } }, red)
   gdImageFilledPolygon(im, { { 160, 180 }, { 170, 110 }, { 150, 160 } }, green)

   /* Draw an arc */
   gdImageArc(im, 50, 50, 40, 40, 30, 190, red )
   gdImageFilledCircle(im, 50, 150, 45, green )
   gdImageEllipse(im, 120, 120, 50, 20, blue )

   /* Draw some characters */
   font := gdFontGetLarge()

   gdImageString(im, font, 0, 0, 'Test', black)
   gdImageString(im, font, 0, 15, 'P', black)
   gdImageChar(im, font, 0, 30, 'W', black)

   gdImageStringUp(im, font, 70, 90, 'Test', black)
   gdImageStringUp(im, font, 70, 15, 'P', black)
   gdImageCharUp(im, font, 70, 30, 'W', black)

   gdImageStringFt(im, blue, "arial", 20, 30, 20, 90, 'Test')

   ? gdImageStringFTCircle(im, 120, 120, 50, 25, 0.8, "arial", 24, "Up", /*"Down"*/, red)

   /* Set Clip Rectangle */
   gdImageSetClip(im, 25, 25, 75, 75)

   /* Retrieve Clipping rectangle */
   aClip := gdImageGetClip(im)

   ? "Clipping rectangle values"
   ? hb_DumpVar( aClip )

   /* Query functions */
   color := gdImageGetPixel(im, gdImageSX(im) / 2, gdImageSY(im) / 2)
   ? "Pixel Color is: ", color
   ? "RGB Values: ", gdImageRed(im,color), gdImageGreen(im,color), gdImageBlue(im,color)
   ? "Alpha Value: ",  gdImageAlpha(im,color)

   /* Write Images on files */
   gdImagePng(im, IMAGES_OUT + "rect.png")

   gdImagePng(im2, IMAGES_OUT + "xharbour.png")
   gdImageJpeg(im2, IMAGES_OUT + "xharbour.jpg")

   /* Destroy images in memory */
   gdImageDestroy(im)
   gdImageDestroy(im2)

   ?
   ? "Look at " + IMAGES_OUT + " folder for output images"
   ?

RETURN

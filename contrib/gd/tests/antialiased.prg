/*
 * $Id: antialiased.prg 9279 2011-02-14 18:06:32Z druzus $
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

   LOCAL im
   LOCAL white, blue, black

   // Check output directory
   IF !ISDirectory( IMAGES_OUT )
      DirMake( IMAGES_OUT )
   ENDIF

   /*
     This sample shows differences on use of antiliased command between a
     palette based image and a true color image.
     Normally antialias works better with a true color image.

     From GD official documentation:

     Antialiased lines can be drawn on both truecolor and palette-based images.
     However, attempts to draw antialiased lines on highly complex palette-based backgrounds
     may not give satisfactory results, due to the limited number of colors available in the
     palette. Antialiased line-drawing on simple backgrounds should work well with palette-based
     images; otherwise create or fetch a truecolor image instead.

   */

   /* ***** DRAW A LINE IN A PALETTE BASED IMAGE ***** */

   /* First we create a true color image */
   im := gdImageCreatePalette(100, 100)    // alias of gdImageCreate()

   /* First allocate color is Background color */
   black := gdImageColorAllocate(im, 0, 0, 0)

   /* set foreground color */
   blue  := gdImageColorAllocate(im, 0, 0, 255)

   /* Now we draw an aliased line */
   gdImageLine(im, 0, 0, 99, 40, blue)

   /* Then we set anti-alias color */
   gdImageSetAntiAliased(im, blue)

   /* and re-draw the line in antialiased mode */
   gdImageLine(im, 0, 40, 99, 80, gdAntiAliased)

   /* saving the image */
   gdImageJpeg(im, IMAGES_OUT + "antialiasedpal.jpg")

   /* Destroy it */
   gdImageDestroy(im)

   /* ***** DRAW A LINE IN A TRUE COLOR IMAGE ***** */

   /* First we create a true color image */
   im := gdImageCreateTrueColor(100, 100)

   /* Background color (true color comes with black background, we have to fill it) */
   white := gdTrueColor( 255, 255, 255 )
   gdImageFilledRectangle(im, 0, 0, 100, 100, white)

   /* set foreground color */
   blue  := gdImageColorAllocate(im, 0, 0, 255)

   /* Now we draw an aliased line */
   gdImageLine(im, 0, 0, 99, 40, blue)

   /* Then we set anti-alias color */
   gdImageSetAntiAliased(im, blue)

   /* and re-draw the line in antialiased mode */
   gdImageLine(im, 0, 40, 99, 80, gdAntiAliased)

   /* saving the image */
   gdImageJpeg(im, IMAGES_OUT + "antialiasedtrue.jpg")

   /* Destroy it */
   gdImageDestroy(im)

   ?
   ? "Look at " + IMAGES_OUT + " folder for output images"
   ?

RETURN

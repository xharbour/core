/*
 * $Id: fitest.prg 9279 2011-02-14 18:06:32Z druzus $
 */

/*
 * Copyright 2005 Francesco Saverio Giudice <info@fsgiudice.com>
 *
 * FreeImage API test file
 */

#include "FreeImage.ch"
#include "common.ch"

#define IMAGES_IN  "images_in/"
#define IMAGES_OUT "images_out/"

PROCEDURE Main(IsMemory)

   LOCAL im, clone, rotated, rotatedEx, rescale, im2, im3
   LOCAL centerX, centerY, width, height, appo
   LOCAL bmpinfoheader IS BITMAPINFOHEADER
   LOCAL bmpinfo       IS BITMAPINFO
   LOCAL bkcolor       IS RGBQUAD
   LOCAL iccprofile    IS FIICCPROFILE
   LOCAL nH, cStr, nLen

   //? "Press Alt-D + Enter to activate debug"
   //AltD( .T. )
   //Inkey(0)
 altd()
   // Check output directory
   IF !ISDirectory( IMAGES_OUT )
      DirMake( IMAGES_OUT )
   ENDIF

   ? "Initialise"
   fi_Initialise()
   //---------------------------//

   ? "Set Error Message:", fi_SetOutPutMessage( @fi_Error() )
   //? "Set Error Message:", fi_SetOutPutMessage( NIL )

   ? "Version          :", fi_GetVersion()
   ? "Copyright        :", fi_GetCopyrightMessage()
   ? "File type        :", fi_GetFileType( IMAGES_IN + "xharbour.jpg" )
   ? "Load JPEG"

   if IsMemory <> nil .and. (nH := FOpen(IMAGES_IN + "xharbour.jpg")) # -1

      nLen := FSeek(nH, 0, 2)
      FSeek(nH, 0, 0)
      cStr := space(nLen)
      fRead(nH, @cStr, nLen)
      FClose(nH)
      im := fi_LoadFromMem( FIF_JPEG, cStr, JPEG_DEFAULT )

   else
      im := fi_Load( FIF_JPEG, IMAGES_IN + "xharbour.jpg", JPEG_DEFAULT )
   endif

   ? "Clone image"
   clone := fi_Clone( im )

   ? "Pointer          :", ValToPrg( im )

   ? "Image Type       :", fi_GetImageType( im )
   ? "Color Used       :", fi_GetColorsUsed( im )
   ? "Pixel size       :", fi_GetBPP( im )
   ? "Width            :", fi_GetWidth( im )
   ? "Height           :", fi_GetHeight( im )
   ? "Byte Size        :", fi_GetLine( im )
   ? "Pitch            :", fi_GetPitch( im )
   ? "DIB Size         :", fi_GetDIBSize( im )
   ? "Dots per Meter X :", fi_GetDotsPerMeterX( im )
   ? "Dots per Meter Y :", fi_GetDotsPerMeterY( im )
   ? "Color Type       :", fi_GetColorType( im )
   ? "Red Mask         :", fi_GetRedMask( im )
   ? "Green Mask       :", fi_GetGreenMask( im )
   ? "Blue Mask        :", fi_GetBlueMask( im )
   ? "Transp. Count    :", fi_GetTransparencyCount( im )
   ? "Is Transparent ? :", fi_IsTransparent( im )
   ?
   ? "Save BMP ?       :", fi_Save( FIF_BMP , im, IMAGES_OUT + "xharbour.bmp", BMP_DEFAULT  )
   ? "Save JPG ?       :", fi_Save( FIF_JPEG, im, IMAGES_OUT + "xharbour.jpg", JPEG_DEFAULT )
   ? "Save PNG ?       :", fi_Save( FIF_PNG , im, IMAGES_OUT + "xharbour.png", PNG_DEFAULT  )

   ? "Save TIFF ?      :", fi_Save( FIF_TIFF, clone, IMAGES_OUT + "xharbour.tif", TIFF_DEFAULT )
   ? "Flip Horizontal ?:", fi_FlipHorizontal( clone )
   ? "Save JPG ?       :", fi_Save( FIF_JPEG, clone, IMAGES_OUT + "horizontal.jpg", JPEG_DEFAULT )
   ? "Flip Vertical ?  :", fi_FlipVertical( clone )
   ? "Save JPG ?       :", fi_Save( FIF_JPEG, clone, IMAGES_OUT + "vertical.jpg", JPEG_DEFAULT )

   ? "Rotate Classic   :", ValToPrg( rotated := fi_RotateClassic( clone, 90 ) )
   ? "Save JPG ?       :", fi_Save( FIF_JPEG, rotated, IMAGES_OUT + "rotate.jpg", JPEG_DEFAULT )
   fi_Unload( rotated )

   centerx := fi_GetWidth( clone ) / 2
   centery := fi_GetHeight( clone ) / 2
   ? "Rotate Ex        :", ValToPrg( rotatedEx := fi_RotateEx( clone, 15, 0, 0, centerx, centery, TRUE ) )
   ? "Save JPG ?       :", fi_Save( FIF_JPEG, rotatedEx, IMAGES_OUT + "rotateEx.jpg", JPEG_DEFAULT )
   fi_Unload( rotatedEx )

   width   := fi_GetWidth( im )
   height  := fi_GetHeight( im )

   ? "Rescale          :", ValToPrg( rescale := fi_Rescale( im, width / 2, height / 2, FILTER_BICUBIC ) )
   ? "Save JPG ?       :", fi_Save( FIF_JPEG, rescale, IMAGES_OUT + "rescale.jpg", JPEG_DEFAULT )
   fi_Unload( rescale )

   im2 := fi_Clone( im )
   ? "Adjust Gamma ?   :", fi_AdjustGamma( im2, 3.0 )
   ? "Save JPG ?       :", fi_Save( FIF_JPEG, im2, IMAGES_OUT + "adjgamma.jpg", JPEG_DEFAULT )
   fi_Unload( im2 )

   im2 := fi_Clone( im )
   ? "Adjust Brightness:", fi_AdjustBrightness( im2, -30 )
   ? "Save JPG ?       :", fi_Save( FIF_JPEG, im2, IMAGES_OUT + "adjbright.jpg", JPEG_DEFAULT )
   fi_Unload( im2 )

   im2 := fi_Clone( im )
   ? "Adjust Contrast ?:", fi_AdjustContrast( im2, -30 )
   ? "Save JPG ?       :", fi_Save( FIF_JPEG, im2, IMAGES_OUT + "adjcontrast.jpg", JPEG_DEFAULT )
   fi_Unload( im2 )

   im2 := fi_Clone( im )
   ? "Invert ?         :", fi_Invert( im2 )
   ? "Save JPG ?       :", fi_Save( FIF_JPEG, im2, IMAGES_OUT + "invert.jpg", JPEG_DEFAULT )
   fi_Unload( im2 )

   ? "Red Channel      :", ValToPrg( im2 := fi_GetChannel( im, FICC_RED ) )
   ? "Save JPG ?       :", fi_Save( FIF_JPEG, im2, IMAGES_OUT + "red.jpg", JPEG_DEFAULT )
   fi_Unload( im2 )

   ? "Green Channel    :", ValToPrg( im2 := fi_GetChannel( im, FICC_GREEN ) )
   ? "Save JPG ?       :", fi_Save( FIF_JPEG, im2, IMAGES_OUT + "green.jpg", JPEG_DEFAULT )
   fi_Unload( im2 )

   ? "Blue Channel     :", ValToPrg( im2 := fi_GetChannel( im, FICC_BLUE ) )
   ? "Save JPG ?       :", fi_Save( FIF_JPEG, im2, IMAGES_OUT + "blue.jpg", JPEG_DEFAULT )
   fi_Unload( im2 )

   ? "Copy             :", ValToPrg( im2 := fi_Copy( im, 300, 100, 800, 200 ) )
   ? "Save JPG ?       :", fi_Save( FIF_JPEG, im2, IMAGES_OUT + "copy.jpg", JPEG_DEFAULT )

   im3 := fi_Clone( im )
   ? "Paste ?          :", fi_Paste( im3, im2, 10, 10, 70 )
   ? "Save JPG ?       :", fi_Save( FIF_JPEG, im3, IMAGES_OUT + "paste.jpg", JPEG_DEFAULT )
   fi_Unload( im2 )
   fi_Unload( im3 )

   ? "Allocate Bitmap  :", ValToPrg( im3 := fi_AllocateT( FIT_BITMAP, 320, 200, 32 ) )
   ? "Save JPG ?       :", fi_Save( FIF_JPEG, im3, IMAGES_OUT + "allocate.jpg", JPEG_DEFAULT )
   fi_Unload( im3 )

   ? "Create ERROR     :"
   ? "Save GIF ?       :", fi_Save( FIF_GIF, im, IMAGES_OUT + "wrong.gif", 0 )

   //? ValToPrg( fi_GetInfoHeader( im ) )
   //bmpinfoheader:Buffer( fi_GetInfoHeader( im ), TRUE )
   bmpinfoheader:Pointer( fi_GetInfoHeader( im ) )
   ? "Header           :", ValToPrg( bmpinfoheader )
   ? bmpinfoheader:SayMembers(" ", .t., .t.)

   bmpinfo:Pointer( fi_GetInfo( im ) )
   ? "Info           :", ValToPrg( bmpinfo )
   ? bmpinfo:SayMembers(" ", .t., .t.)
   ? "-----------------------------------------------------"
   ? ValType( bmpinfo:Devalue() )
   //Tracelog( "bmpinfoheader", ValToPrg( bmpinfoheader ), ;
   //          bmpinfoheader:SayMembers(, .t.), bmpinfoheader:Value(), bmpinfoheader:DeValue(), hb_dumpvar( bmpinfoheader:Array() ), hb_dumpvar( bmpinfoheader:acMembers ) )

   //appo := bkcolor:Value()
   ? bkcolor:Pointer( fi_GetBackgroundColor( im ) )
   //? fi_GetBackgroundColor( im, @bkcolor:Value() )
   //bkcolor:Buffer( appo )
   ? bkcolor:SayMembers(" ", .t., .t.)

   bkcolor:rgbBlue := 205
   //? fi_SetBackgroundColor( im, hb_String2Pointer( bkcolor:Value() ) )
   Tracelog("linha 168")
   ? fi_SetBackgroundColor( im, bkcolor:Value() )
   Tracelog("linha 170")
   ? bkcolor:SayMembers(" ", .t., .t.)
Tracelog("linha 162")
//   ? bkcolor:Pointer( fi_GetBackgroundColor( im ) )
   //? fi_GetBackgroundColor( im, @bkcolor:Value() )
   //bkcolor:Buffer( appo )
Tracelog("linha 176")
   ? bkcolor:SayMembers(" ", .t., .t.)

Tracelog("linha 179")
   iccprofile:Pointer( fi_GetICCProfile( im ) )
   Tracelog("linha 181")
   ? "Header           :", ValToPrg( iccprofile )
   Tracelog("linha 183")
   ? iccprofile:SayMembers(" ", .t., .t.)

   //bmpinfoheader:Reset()
   //appo := NIL
   //bmpinfoheader := NIL
   //hb_GCAll( .T. )

   ? "Unload images from memory"
   fi_Unload( im )
   fi_Unload( clone )


   //---------------------------//
   ? "DeInitialise"
   fi_Deinitialise()

   ?
   ? "Look at " + IMAGES_OUT + " folder for output images"
   ?

RETURN

PROCEDURE fi_Error( cFormat, cMessage )
   ? "ERROR!..."
   ? "Format  : ", cFormat
   ? "Message : ", cMessage
RETURN

/*
 * $Id: gdimage.prg 9908 2013-01-13 05:23:04Z andijahja $
 */

/*
 * Harbour Project source code:
 * GD graphic library class
 *
 * Copyright 2004-2005 Francesco Saverio Giudice <info@fsgiudice.com>
 * www - http://www.xharbour.org http://www.harbour-project.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307 USA (or visit the web site http://www.gnu.org/).
 *
 * As a special exception, the Harbour Project gives permission for
 * additional uses of the text contained in its release of Harbour.
 *
 * The exception is that, if you link the Harbour libraries with other
 * files to produce an executable, this does not by itself cause the
 * resulting executable to be covered by the GNU General Public License.
 * Your use of that executable is in no way restricted on account of
 * linking the Harbour library code into it.
 *
 * This exception does not however invalidate any other reasons why
 * the executable file might be covered by the GNU General Public License.
 *
 * This exception applies only to the code released by the Harbour
 * Project under the name Harbour.  If you copy code from other
 * Harbour Project or Free Software Foundation releases into a copy of
 * Harbour, as the General Public License permits, the exception does
 * not apply to the code that you add in this way.  To avoid misleading
 * anyone as to the status of such modified files, you must delete
 * this exception notice from them.
 *
 * If you write modifications of your own for Harbour, it is your choice
 * whether to permit this exception to apply to your modifications.
 * If you do not wish that, delete this exception notice.
 *
 */

/*
 *
 * See doc/license files for licensing terms.
 *
 */

#include "common.ch"
#include "hbclass.ch"
#include "gd.ch"

#define DEFAULT( x, y ) IIF( x == NIL, x := y, )

CLASS GDImage

   HIDDEN:
   DATA pImage
   DATA pBrush
   DATA pTile
   DATA pFont
   DATA pColor

   DATA cFontName    INIT "Arial"
   DATA nFontPitch   INIT 20
   DATA nFontAngle   INIT 0

   DATA aPoints      INIT {}
   DATA aStyles      INIT {}
   DATA lDestroy     INIT TRUE

   EXPORTED:
   DATA hFile
   DATA cType
   DATA cMime

   METHOD New( sx, sy )  CONSTRUCTOR

   /* IMAGE CREATION, DESTRUCTION, LOADING AND SAVING  */

   // Create in memory
   METHOD Create( sx, sy )                 INLINE ::pImage := gdImageCreate( sx, sy ), Self
   METHOD CreateTrueColor( sx, sy )        INLINE ::pImage := gdImageCreateTrueColor( sx, sy ), Self

   // Load From File
   METHOD LoadFromPng( cFile )             INLINE ::pImage := gdImageCreateFromPng( cFile ), Self
   METHOD LoadFromJpeg( cFile )            INLINE ::pImage := gdImageCreateFromJpeg( cFile ), Self
   METHOD LoadFromWBmp( cFile )            INLINE ::pImage := gdImageCreateFromWBMP( cFile ), Self
   METHOD LoadFromGd( cFile )              INLINE ::pImage := gdImageCreateFromGD( cFile ), Self
   METHOD LoadFromGif( cFile )             INLINE ::pImage := gdImageCreateFromGif( cFile ), Self

   // Load From a specific File handle
   METHOD InputPng( nHandle, nSize )       INLINE ::pImage := gdImageCreateFromPng( nHandle, nSize ), Self
   METHOD InputJpeg( nHandle, nSize )      INLINE ::pImage := gdImageCreateFromJpeg( nHandle, nSize ), Self
   METHOD InputWBmp( nHandle, nSize )      INLINE ::pImage := gdImageCreateFromWBMP( nHandle, nSize ), Self
   METHOD InputGd( nHandle, nSize )        INLINE ::pImage := gdImageCreateFromGD( nHandle, nSize ), Self
   METHOD InputGif( nHandle, nSize )       INLINE ::pImage := gdImageCreateFromGif( nHandle, nSize ), Self

   // Create from an image pointer in memory
   METHOD CreateFromPng( pImage, nSize )   INLINE ::pImage := gdImageCreateFromPng( pImage, nSize ), Self
   METHOD CreateFromJpeg( pImage, nSize )  INLINE ::pImage := gdImageCreateFromJpeg( pImage, nSize ), Self
   METHOD CreateFromWBmp( pImage, nSize )  INLINE ::pImage := gdImageCreateFromWBMP( pImage, nSize ), Self
   METHOD CreateFromGd( pImage, nSize )    INLINE ::pImage := gdImageCreateFromGD( pImage, nSize ), Self
   METHOD CreateFromGif( pImage, nSize )   INLINE ::pImage := gdImageCreateFromGif( pImage, nSize ), Self

   METHOD LoadFromFile( cFile )

   // Save To File Name
   METHOD SavePng( cFile, nLevel )         INLINE gdImagePng( ::pImage, cFile, nLevel )
   METHOD SaveJpeg( cFile, nLevel )        INLINE gdImageJpeg( ::pImage, cFile, nLevel )
   METHOD SaveWBmp( cFile, nFG )           INLINE gdImageWBmp( ::pImage, cFile, nFG )
   METHOD SaveGd( cFile )                  INLINE gdImageGd( ::pImage, cFile )
   METHOD SaveGif( cFile )                 INLINE gdImageGif( ::pImage, cFile )

   METHOD SaveToFile( cFile )              INLINE gdImageToFile( Self, cFile )

   // Output To a specified File handle
   METHOD OutputPng( nHandle, nLevel )     INLINE IIF( nHandle == NIL, nHandle := 1, ), gdImagePng( ::pImage, nHandle, nLevel )
   METHOD OutputJpeg( nHandle, nLevel )    INLINE IIF( nHandle == NIL, nHandle := 1, ), gdImageJpeg( ::pImage, nHandle, nLevel )
   METHOD OutputWBmp( nHandle, nFG )       INLINE IIF( nHandle == NIL, nHandle := 1, ), gdImageWBmp( ::pImage, nHandle, nFG )
   METHOD OutputGd( nHandle )              INLINE IIF( nHandle == NIL, nHandle := 1, ), gdImageGd( ::pImage, nHandle )
   METHOD OutputGif( nHandle )             INLINE IIF( nHandle == NIL, nHandle := 1, ), gdImageGif( ::pImage, nHandle )

   // Output To a string
   METHOD ToStringPng( nLevel )            INLINE gdImagePng( ::pImage, NIL, nLevel )
   METHOD ToStringJpeg( nLevel )           INLINE gdImageJpeg( ::pImage, NIL, nLevel )
   METHOD ToStringWBmp( nFG )              INLINE gdImageWBmp( ::pImage, NIL, nFG )
   METHOD ToStringGd()                     INLINE gdImageGd( ::pImage, NIL )
   METHOD ToStringGif()                    INLINE gdImageGif( ::pImage, NIL )

   METHOD ToString()                       INLINE gdImageToString( Self )

   // Destructor
   METHOD Destroy()                        INLINE gdImageDestroy( ::pImage )
   DESTRUCTOR Destruct()



   /* DRAWING FUNCTIONS */
   METHOD SetPixel( x, y, color )          INLINE DEFAULT( color, ::pColor ), gdImageSetPixel( ::pImage, x, y, color )
   METHOD Line( x1, y1, x2, y2, color )    INLINE DEFAULT( color, ::pColor ), gdImageLine( ::pImage, x1, y1, x2, y2, color )
   METHOD DashedLine( x1, y1, x2, y2, color )    INLINE DEFAULT( color, ::pColor ), gdImageDashedLine( ::pImage, x1, y1, x2, y2, color )

   // Functions usefull for polygons
   METHOD Polygon( aPoints, lFilled, color )
#if ( GD_VERS >= 2033 )
   METHOD OpenPolygon( aPoints, color )
#endif
   METHOD AddPoint( x, y )                 INLINE aAdd( ::aPoints, { x, y } )
   METHOD ResetPoints()                    INLINE ::aPoints := {}
   METHOD Points()                         INLINE Len( ::aPoints )

   METHOD Rectangle( x1, y1, x2, y2, lFilled, color )
   METHOD Arc( x, y, nWidth, nHeight, nStartDegree, nEndDegree, lFilled, nColor )
   METHOD Ellipse( x, y, nWidth, nHeight, lFilled, nColor )
   METHOD Circle( x, y, nRadius, lFilled, nColor ) ;
                                           INLINE ::Ellipse( x, y, nRadius, nRadius, lFilled, nColor )

   METHOD Fill( x, y, color )              INLINE DEFAULT( color, ::pColor ), gdImageFill( ::pImage, x, y, color )
   METHOD FillToBorder( x, y, border, color ) ;
                                           INLINE DEFAULT( color, ::pColor ), gdImageFillToBorder( ::pImage, x, y, border, color )
   METHOD SetAntiAliased( color )          INLINE DEFAULT( color, ::pColor ), gdImageSetAntiAliased( ::pImage, color )
   METHOD SetAntiAliasedDontBlend( lDontBlend, color ) ;
                                           INLINE DEFAULT( color, ::pColor ), gdImageSetAntiAliasedDontBlend( ::pImage, color, lDontBlend )

   METHOD SetBrush( pBrush )               INLINE gdImageSetBrush( ::pImage, pBrush:pImage ), ::pBrush := pBrush
   METHOD SetTile( pTile )                 INLINE gdImageSetTile( ::pImage, pTile:pImage ), ::pTile := pTile

   // Functions usefull for style
   METHOD SetStyle( aStyle )               INLINE DEFAULT( aStyle, ::aStyles ), gdImageSetStyle( ::pImage, aStyle )
   METHOD AddStyle( pColor )               INLINE aAdd( ::aStyles, pColor )
   METHOD ResetStyles()                    INLINE ::aStyles := {}
   METHOD StyleLenght()                    INLINE Len( ::aStyles )

   METHOD SetThickness( nThickness )       INLINE gdImageSetThickness( ::pImage, nThickness )
   METHOD SetAlphaBlending( lAlphaBlending )  INLINE gdImageAlphaBlending( ::pImage, lAlphaBlending )
   METHOD SetSaveAlpha( lSaveAlpha )       INLINE gdImageSaveAlpha( ::pImage, lSaveAlpha )
   METHOD SetClippingArea( x1, y1, x2, y2 )   INLINE gdImageSetClip( ::pImage, x1, y1, x2, y2 )

   /* QUERY FUNCTIONS */
   METHOD ColorsTotal()                    INLINE gdImageColorsTotal( ::pImage )
   METHOD Alpha( color )                   INLINE DEFAULT( color, ::pColor ), gdImageAlpha( ::pImage, color )
   METHOD Red( color )                     INLINE DEFAULT( color, ::pColor ), gdImageRed( ::pImage, color )
   METHOD Green( color )                   INLINE DEFAULT( color, ::pColor ), gdImageGreen( ::pImage, color )
   METHOD Blue( color )                    INLINE DEFAULT( color, ::pColor ), gdImageBlue( ::pImage, color )
   METHOD Width()                          INLINE gdImageSx( ::pImage )
   METHOD Height()                         INLINE gdImageSy( ::pImage )
   METHOD CenterWidth()                    INLINE ::Width() / 2
   METHOD CenterHeight()                   INLINE ::Height() / 2
   METHOD GetPixel( x, y )                 INLINE gdImageGetPixel( ::pImage, x, y )
   METHOD GetColor()                       INLINE ::pColor
   METHOD GetClippingArea()                INLINE gdImageGetClip( ::pImage )
   METHOD IsBoundsSafe( x, y )             INLINE gdImageBoundsSafe( ::pImage, x, y )
   METHOD IsInterlaced()                   INLINE gdImageGetInterlaced( ::pImage )
   METHOD GetTransparent()                 INLINE gdImageGetTransparent( ::pImage )
   METHOD IsTransparent()                  INLINE ::GetTransparent() > 0
   METHOD IsTrueColor()                    INLINE gdImageTrueColor( ::pImage )

   METHOD ConvertFromTrueColorToPalette( lDither, nColorsWanted ) ;
                                           INLINE gdImageTrueColorToPalette ( ::pImage, lDither, nColorsWanted )
   METHOD CreatePaletteFromTrueColor( lDither, nColorsWanted ) ;
                                           INLINE gdImageCreatePaletteFromTrueColor( ::pImage, lDither, nColorsWanted )
   METHOD GetPalette( x, y )               INLINE gdImagePalettePixel( ::pImage, x, y )
   // METHOD GetTrueColor( x, y )             INLINE gdImageTrueColorPixel( ::pImage, x, y )
   METHOD GetThickness()                   INLINE gdImageGetThickness( ::pImage )

   /* FONTS AND TEXT-HANDLING FUNCTIONS */
   METHOD SetFontSmall()                   INLINE ::pFont := gdFontGetSmall()
   METHOD SetFontLarge()                   INLINE ::pFont := gdFontGetLarge()
   METHOD SetFontMediumBold()              INLINE ::pFont := gdFontGetMediumBold()
   METHOD SetFontGiant()                   INLINE ::pFont := gdFontGetGiant()
   METHOD SetFontTiny()                    INLINE ::pFont := gdFontGetTiny()
   METHOD Say( x, y, cString, color, nAlign )
   METHOD SayVertical( x, y, cString, color )  INLINE DEFAULT( color, ::pColor ), gdImageStringUp( ::pImage, ::pFont, x, y, cString, color )

   METHOD SetFontName( cFontName )         INLINE ::cFontName  := cFontName
   METHOD SetFontPitch( nPitch )           INLINE ::nFontPitch := nPitch
   METHOD SetFontAngle( nAngle )           INLINE ::nFontAngle := nAngle
   METHOD SayFreeType( x, y, cString, cFontName, nPitch, nAngle, color, nAlign, ;
                       nLineSpacing, nCharMap, nResolution )

   METHOD SayFreeTypeCircle( x, y, cStringTop, cStringBottom, color, nRadius, nTextRadius, nFillPortion, cFontName, nPitch ) ;
                                           INLINE DEFAULT( color, ::pColor ), gdImageStringFTCircle( ::pImage, x, y, nRadius, ;
                                                           nTextRadius, nFillPortion, cFontName, nPitch, cStringTop, cStringBottom, color )

   METHOD GetFont()                        INLINE ::pFont
   METHOD GetFontWidth( pFont )            INLINE DEFAULT( pFont, ::pFont ), gdFontGetWidth( pFont )
   METHOD GetFontHeight( pFont )           INLINE DEFAULT( pFont, ::pFont ), gdFontGetHeight( pFont )

   METHOD GetFTFontWidth( cFontName, nPitch )  INLINE DEFAULT( cFontName, ::cFontName ), ;
                                                      DEFAULT( nPitch, ::nFontPitch )  , ;
                                                      gdImageFTWidth( cFontName, nPitch )

   METHOD GetFTFontHeight( cFontName, nPitch ) INLINE DEFAULT( cFontName, ::cFontName ), ;
                                                      DEFAULT( nPitch, ::nFontPitch )  , ;
                                                      gdImageFTHeight( cFontName, nPitch )

   METHOD GetFTStringSize( cString, cFontName, nPitch ) INLINE DEFAULT( cFontName, ::cFontName ), ;
                                                      DEFAULT( nPitch, ::nFontPitch )  , ;
                                                      gdImageFTSize( cString, cFontName, nPitch )

   /* COLOR HANDLING FUNCTIONS */
   METHOD SetColor( r, g, b )              INLINE IIF( PCount() == 2, ::pColor := r, ::pColor := gdImageColorAllocate( ::pImage, r, g, b ) )
   METHOD DelColor( pColor )               INLINE ::pColor := NIL, gdImageColorDeAllocate( ::pImage, pColor )
   METHOD SetColorAlpha( r, g, b, a )      INLINE ::pColor := gdImageColorAllocateAlpha( ::pImage, r, g, b, a)
   METHOD SetColorClosest( r, g, b )       INLINE ::pColor := gdImageColorClosest( ::pImage, r, g, b )
   METHOD SetColorClosestAlpha( r, g, b, a ) INLINE ::pColor := gdImageColorClosestAlpha( ::pImage, r, g, b, a)
   METHOD SetColorClosestHWB( r, g, b )    INLINE ::pColor := gdImageColorClosestHWB( ::pImage, r, g, b )
   METHOD SetColorExact( r, g, b )         INLINE ::pColor := gdImageColorExact( ::pImage, r, g, b )
   METHOD SetColorResolve( r, g, b )       INLINE ::pColor := gdImageColorResolve( ::pImage, r, g, b )
   METHOD SetColorResolveAlpha( r, g, b, a ) INLINE ::pColor := gdImageColorResolveAlpha( ::pImage, r, g, b, a)
   METHOD SetTransparent( pColor )         INLINE gdImageColorTransparent( ::pImage, pColor )
   METHOD SetSharpen( nPerc )              INLINE gdImageSharpen( ::pImage, nPerc )
   METHOD SetInterlace( lOnOff )           INLINE gdImageInterlace( ::pImage, lOnOff )
   METHOD SetInterlaceOn()                 INLINE gdImageInterlace( ::pImage, TRUE )
   METHOD SetInterlaceOff()                INLINE gdImageInterlace( ::pImage, FALSE )

   METHOD GetTrueColor( r, g, b )          INLINE Self, gdTrueColor( r, g, b )
   METHOD GetTrueColorAlpha( r, g, b, a )  INLINE Self, gdTrueColorAlpha( r, g, b, a )

   /* COPY AND RESIZING FUNCTIONS */
   METHOD Copy()
   METHOD CopyResized()
   METHOD CopyResampled()
   METHOD CopyRotated()
   METHOD CopyMerge()
   METHOD CopyMergeGray()

   /* New implemented */
   METHOD Clone()
   METHOD CopyZoomed()
   METHOD Crop()
   METHOD Zoom()
   METHOD Resize()
   METHOD Rotate()
   METHOD RotateInside( nAngle )           INLINE ::Rotate( nAngle, .T. )

   METHOD PaletteCopy( oDestImage )        INLINE gdImagePaletteCopy( oDestImage:pImage, ::pImage )
   METHOD SquareToCircle( nRadius )        INLINE gdImageSquareToCircle( ::pImage, nRadius )
   METHOD Compare( oDestImage )            INLINE gdImageCompare( oDestImage:pImage, ::pImage )


   METHOD Radians( nAngle )                INLINE Self, PI() * nAngle / 180
   METHOD Degres( nRadians )               INLINE Self, nRadians * 180 / PI()

   METHOD Version()                        INLINE Self, gdVersion()

ENDCLASS

METHOD New( sx, sy ) CLASS GDImage
   ::Create( sx, sy )
RETURN Self

PROCEDURE Destruct() CLASS GDImage
  //__OutDebug( "Destroyed" )
  IF ::lDestroy
     ::Destroy()
  ENDIF
RETURN

METHOD Polygon( aPoints, lFilled, color ) CLASS GDImage
  DEFAULT aPoints TO ::aPoints
  DEFAULT lFilled TO FALSE
  DEFAULT color   TO ::pColor
  IF lFilled
     gdImageFilledPolygon( ::pImage, aPoints, color )
  ELSE
     gdImagePolygon( ::pImage, aPoints, color )
  ENDIF
RETURN Self

#if ( GD_VERS >= 2033 )
METHOD OpenPolygon( aPoints, color ) CLASS GDImage
  DEFAULT aPoints TO ::aPoints
  DEFAULT color   TO ::pColor
  gdImageOpenPolygon( ::pImage, aPoints, color )
RETURN Self
#endif

METHOD Rectangle( x1, y1, x2, y2, lFilled, color ) CLASS GDImage
  DEFAULT lFilled TO FALSE
  DEFAULT color   TO ::pColor
  IF lFilled
     gdImageFilledRectangle( ::pImage, x1, y1, x2, y2, color )
  ELSE
     gdImageRectangle( ::pImage, x1, y1, x2, y2, color )
  ENDIF
RETURN Self

METHOD Arc( x, y, nWidth, nHeight, nStartDegree, nEndDegree, lFilled, color, nStyle ) CLASS GDImage
  DEFAULT lFilled TO FALSE
  DEFAULT color   TO ::pColor
  DEFAULT nStyle  TO gdArc
  IF lFilled
     gdImageFilledArc( ::pImage, x, y, nWidth, nHeight, nStartDegree, nEndDegree, color, nStyle )
  ELSE
     gdImageArc( ::pImage, x, y, nWidth, nHeight, nStartDegree, nEndDegree, color )
  ENDIF
RETURN Self

METHOD Ellipse( x, y, nWidth, nHeight, lFilled, color ) CLASS GDImage
  DEFAULT lFilled TO FALSE
  DEFAULT color   TO ::pColor
  IF lFilled
     gdImageFilledEllipse( ::pImage, x, y, nWidth, nHeight, color )
  ELSE
     gdImageEllipse( ::pImage, x, y, nWidth, nHeight, color )
  ENDIF
RETURN Self

METHOD LoadFromFile( cFile ) CLASS GDImage
   LOCAL aLoad
   aLoad := gdImageFromFile( cFile )
   //TraceLog( ValToPrg( aLoad ) )

   //Self  := aLoad[1]:Clone()
   ::Destroy()
   __ObjectClone( aLoad[1], Self )
   aLoad[1]:lDestroy := FALSE
   aLoad[1] := NIL

   ::hFile := aLoad[2]
   ::cType := aLoad[3]
   ::cMime := aLoad[4]
RETURN Self

METHOD Copy( nSrcX, nSrcY, nWidth, nHeight, nDstX, nDstY, oDestImage ) CLASS GDImage

  DEFAULT nSrcX      TO 0
  DEFAULT nSrcY      TO 0
  DEFAULT nWidth     TO ::Width()
  DEFAULT nHeight    TO ::Height()
  DEFAULT nDstX      TO 0
  DEFAULT nDstY      TO 0

  IF oDestImage == NIL
     IF ::IsTrueColor()
        oDestImage := GDImage():CreateTrueColor( nWidth, nHeight )
     ELSE
        oDestImage := GDImage():Create( nWidth, nHeight )
     ENDIF
  ENDIF
  gdImageCopy( oDestImage:pImage, ::pImage, nDstX, nDstY, nSrcX, nSrcY, nWidth, nHeight )
RETURN oDestImage

METHOD CopyResized( nSrcX, nSrcY, nSrcWidth, nSrcHeight, nDstX, nDstY, nDstWidth, nDstHeight, oDestImage ) CLASS GDImage
  DEFAULT nSrcX      TO 0
  DEFAULT nSrcY      TO 0
  DEFAULT nSrcWidth  TO ::Width()
  DEFAULT nSrcHeight TO ::Height()
  DEFAULT nDstX      TO 0
  DEFAULT nDstY      TO 0
  DEFAULT nDstWidth  TO ::Width()
  DEFAULT nDstHeight TO ::Height()

  IF oDestImage == NIL
     IF ::IsTrueColor()
        oDestImage := GDImage():CreateTrueColor( nDstWidth, nDstHeight )
     ELSE
        oDestImage := GDImage():Create( nDstWidth, nDstHeight )
     ENDIF
  ENDIF
  gdImageCopyResized( oDestImage:pImage, ::pImage, nDstX, nDstY, nSrcX, nSrcY, nDstWidth, nDstHeight, nSrcWidth, nSrcHeight )
RETURN oDestImage

METHOD CopyResampled( nSrcX, nSrcY, nSrcWidth, nSrcHeight, nDstX, nDstY, nDstWidth, nDstHeight, oDestImage ) CLASS GDImage
  DEFAULT nSrcX      TO 0
  DEFAULT nSrcY      TO 0
  DEFAULT nSrcWidth  TO ::Width()
  DEFAULT nSrcHeight TO ::Height()
  DEFAULT nDstX      TO 0
  DEFAULT nDstY      TO 0
  DEFAULT nDstWidth  TO ::Width()
  DEFAULT nDstHeight TO ::Height()

  IF oDestImage == NIL
     IF ::IsTrueColor()
        oDestImage := GDImage():CreateTrueColor( nDstWidth, nDstHeight )
     ELSE
        oDestImage := GDImage():Create( nDstWidth, nDstHeight )
     ENDIF
  ENDIF
  gdImageCopyResampled( oDestImage:pImage, ::pImage, nDstX, nDstY, nSrcX, nSrcY, nDstWidth, nDstHeight, nSrcWidth, nSrcHeight )
RETURN oDestImage

METHOD CopyRotated( nSrcX, nSrcY, nWidth, nHeight, nDstX, nDstY, nAngle, oDestImage ) CLASS GDImage
  DEFAULT nSrcX      TO 0
  DEFAULT nSrcY      TO 0
  DEFAULT nWidth     TO ::Width
  DEFAULT nHeight    TO ::Height
  DEFAULT nDstX      TO nWidth / 2
  DEFAULT nDstY      TO nHeight / 2
  DEFAULT nAngle     TO 90

  IF oDestImage == NIL
     IF ::IsTrueColor()
        oDestImage := GDImage():CreateTrueColor( nWidth, nHeight )
     ELSE
        oDestImage := GDImage():Create( nWidth, nHeight )
     ENDIF
  ENDIF
  //__OutDebug( nAngle )
  gdImageCopyRotated( oDestImage:pImage, ::pImage, nDstX, nDstY, nSrcX, nSrcY, nWidth, nHeight, nAngle )
RETURN oDestImage

METHOD CopyMerge( nSrcX, nSrcY, nWidth, nHeight, nDstX, nDstY, nPerc, oDestImage ) CLASS GDImage
  DEFAULT nSrcX      TO 0
  DEFAULT nSrcY      TO 0
  DEFAULT nWidth     TO ::Width
  DEFAULT nHeight    TO ::Height
  DEFAULT nDstX      TO 0
  DEFAULT nDstY      TO 0
  DEFAULT nPerc      TO 100

  IF oDestImage == NIL
     IF ::IsTrueColor()
        oDestImage := GDImage():CreateTrueColor( nWidth, nHeight )
     ELSE
        oDestImage := GDImage():Create( nWidth, nHeight )
     ENDIF
  ENDIF
  gdImageCopyMerge( oDestImage:pImage, ::pImage, nDstX, nDstY, nSrcX, nSrcY, nWidth, nHeight, nPerc )
RETURN oDestImage

METHOD CopyMergeGray( nSrcX, nSrcY, nWidth, nHeight, nDstX, nDstY, nPerc, oDestImage ) CLASS GDImage
  DEFAULT nSrcX      TO 0
  DEFAULT nSrcY      TO 0
  DEFAULT nWidth     TO ::Width
  DEFAULT nHeight    TO ::Height
  DEFAULT nDstX      TO 0
  DEFAULT nDstY      TO 0
  DEFAULT nPerc      TO 100

  IF oDestImage == NIL
     IF ::IsTrueColor()
        oDestImage := GDImage():CreateTrueColor( nWidth, nHeight )
     ELSE
        oDestImage := GDImage():Create( nWidth, nHeight )
     ENDIF
  ENDIF
  gdImageCopyMergeGray( oDestImage:pImage, ::pImage, nDstX, nDstY, nSrcX, nSrcY, nWidth, nHeight, nPerc )
RETURN oDestImage

METHOD CopyZoomed( nPerc, nSrcX, nSrcY, nSrcWidth, nSrcHeight ) CLASS GDImage
  LOCAL oDestImage
  LOCAL nDstX, nDstY, nDstWidth, nDstHeight

  DEFAULT nPerc      TO 100
  DEFAULT nSrcX      TO 0
  DEFAULT nSrcY      TO 0
  DEFAULT nSrcWidth  TO ::Width()
  DEFAULT nSrcHeight TO ::Height()

  IF nPerc < 0
     nPerc := 100
  ENDIF

  nDstX      := 0
  nDstY      := 0
  nDstWidth  := nSrcWidth * nPerc / 100
  nDstHeight := nSrcHeight * nPerc / 100

  IF ::IsTrueColor()
     oDestImage := GDImage():CreateTrueColor( nDstWidth, nDstHeight )
  ELSE
     oDestImage := GDImage():Create( nDstWidth, nDstHeight )
  ENDIF
  gdImageCopyResampled( oDestImage:pImage, ::pImage, nDstX, nDstY, nSrcX, nSrcY, nDstWidth, nDstHeight, nSrcWidth, nSrcHeight )

RETURN oDestImage

METHOD Rotate( nAngle, lInside ) CLASS GDImage
  LOCAL oDestImage
  LOCAL nWidth, nHeight
  LOCAL nAngRad := nAngle * PI() / 180

  DEFAULT lInside TO FALSE

  IF !lInside
     nWidth  := ::Width * cos( nAngRad ) + ::Height * sin( nAngRad )
     nHeight := ::Width * sin( nAngRad ) + ::Height * cos( nAngRad )
  ELSE
     nWidth  := ::Width
     nHeight := ::Height
  ENDIF
  //__OutDebug( ::Width, ::Height )
  //__OutDebug( nWidth, nHeight )

  IF ::IsTrueColor()
     oDestImage := GDImage():CreateTrueColor( nWidth, nHeight )
  ELSE
     oDestImage := GDImage():Create( nWidth, nHeight )
  ENDIF
  IF !lInside
      ::CopyRotated( ,,,, nWidth - nWidth/2, nHeight - nHeight/2, nAngle, oDestImage )
  ELSE
      ::CopyRotated( ,,,,,, nAngle, oDestImage )
  ENDIF
  ::Destroy()
  __ObjectClone( oDestImage, Self )
  //Self := __ObjClone( oDestImage ) // non funziona

  // Move new image to existing one
  // Signal that this image must not be destroyed
  oDestImage:lDestroy := FALSE
  oDestImage := NIL

RETURN Self

METHOD Crop( nX, nY, nWidth, nHeight ) CLASS GDImage
  LOCAL oDestImage

  oDestImage := ::CopyResized( nX, nY, nWidth, nHeight, 0, 0, nWidth, nHeight )
  ::Destroy()
  __ObjectClone( oDestImage, Self )
  //Self := __ObjClone( oDestImage ) // non funziona

  // Move new image to existing one
  // Signal that this image must not be destroyed
  oDestImage:lDestroy := FALSE
  oDestImage := NIL

RETURN Self

METHOD Resize( nWidth, nHeight ) CLASS GDImage
  LOCAL oDestImage

  oDestImage := ::CopyResampled( 0, 0, NIL, NIL, 0, 0, nWidth, nHeight )
  ::Destroy()
  __ObjectClone( oDestImage, Self )
  //Self := __ObjClone( oDestImage ) // non funziona

  // Move new image to existing one
  // Signal that this image must not be destroyed
  oDestImage:lDestroy := FALSE
  oDestImage := NIL

RETURN Self

METHOD Zoom( nPerc ) CLASS GDImage
  LOCAL oDestImage

  oDestImage := ::CopyZoomed( nPerc )
  ::Destroy()
  __ObjectClone( oDestImage, Self )
  //Self := __ObjClone( oDestImage ) // non funziona

  // Move new image to existing one
  // Signal that this image must not be destroyed
  oDestImage:lDestroy := FALSE
  oDestImage := NIL

RETURN Self

METHOD Clone() CLASS GDImage
  LOCAL oDestImage
  LOCAL pImage

  IF ::IsTrueColor()
     oDestImage := GDImage():CreateTrueColor( ::Width, ::Height )
  ELSE
     oDestImage := GDImage():Create( ::Width, ::Height )
  ENDIF

  pImage := oDestImage:pImage
  __ObjectClone( Self, oDestImage )
  oDestImage:pImage := pImage
  ::Copy( 0, 0, ::Width, ::Height, 0, 0, oDestImage )


  //pImage := oDestImage:pImage
  //// Signal that this image must not be destroyed
  //oDestImage:lDestroy := FALSE
  //oDestImage := NIL
  //oDestImage:pImage := pImage

RETURN oDestImage

METHOD Say( x, y, cString, color, nAlign ) CLASS GDImage
  LOCAL nWidth, nLen
  LOCAL nPosX

  DEFAULT color  TO ::pColor
  DEFAULT nAlign TO gdAlignLeft

  IF     nAlign == gdAlignCenter
     nWidth := ::GetFontWidth()
     nLen   := Len( cString )
     nPosX  := x - ( nLen / 2 * nWidth )
  ELSEIF nAlign == gdAlignRight
     nWidth := ::GetFontWidth()
     nLen   := Len( cString )
     nPosX  := x - ( nLen * nWidth )
  ELSE
     nPosX  := x
  ENDIF

  gdImageString( ::pImage, ::pFont, nPosX, y, cString, color )
RETURN Self

METHOD SayFreeType( x, y, cString, cFontName, nPitch, nAngle, color, nAlign, ;
                    nLineSpacing, nCharMap, nResolution )  CLASS GDImage
  LOCAL nWidth, nLen
  LOCAL nPosX

  DEFAULT nAlign    TO gdAlignLeft
  DEFAULT color     TO ::pColor
  DEFAULT cFontName TO ::cFontName
  DEFAULT nPitch    TO ::nFontPitch
  DEFAULT nAngle    TO ::nFontAngle

  IF     nAlign == gdAlignCenter
     nWidth := nPitch //gdImageFTWidth( cFontName, nPitch )//, ::Radians( nAngle ) ) //::GetFontWidth()
     //__OutDebug( "nWidth", nWidth  )
     nLen   := Len( cString )
     nPosX  := x - ( (nLen / 2) * nWidth )
  ELSEIF nAlign == gdAlignRight
     nWidth := gdImageFTWidth( cFontName, nPitch ) //, ::Radians( nAngle ) ) //::GetFontWidth()
     nLen   := Len( cString )
     nPosX  := x - ( nLen * nWidth )
  ELSE
     nPosX  := x
  ENDIF

  gdImageStringFT( ::pImage, color, cFontName, nPitch, ::Radians( nAngle ), nPosX, y, ;
                   cString, nLineSpacing, nCharMap, nResolution )

RETURN Self

PROCEDURE __ObjectClone( oSrc, oDst )
   LOCAL aProps, xProp
   LOCAL lOldScope := __SetClassScope( .F. )
   LOCAL cType

   // Get data
   aProps    := __objGetValueFullList( oSrc, NIL, HB_OO_CLSTP_HIDDEN + HB_OO_CLSTP_PROTECTED + HB_OO_CLSTP_EXPORTED + HB_OO_CLSTP_PUBLISHED )

   FOR EACH xProp IN aProps
       cType := ValType( xProp[ HB_OO_DATA_VALUE ] )
       DO CASE
          CASE cType == "A"
               __ObjSendMsgCase( oDst, xProp[ HB_OO_DATA_SYMBOL ], AClone( xProp[ HB_OO_DATA_VALUE ] ) )
          CASE cType == "O"
               __ObjSendMsgCase( oDst, xProp[ HB_OO_DATA_SYMBOL ], __ObjClone( xProp[ HB_OO_DATA_VALUE ] ) )
          CASE cType == "H"
               __ObjSendMsgCase( oDst, xProp[ HB_OO_DATA_SYMBOL ], HClone( xProp[ HB_OO_DATA_VALUE ] ) )
          OTHERWISE
               __ObjSendMsgCase( oDst, xProp[ HB_OO_DATA_SYMBOL ], xProp[ HB_OO_DATA_VALUE ] )
       ENDCASE
       //__OutDebug( xProp[ HB_OO_DATA_SYMBOL ] + " := " + cStr( xProp[ HB_OO_DATA_VALUE ] ) )
   NEXT

   __SetClassScope( lOldScope )

RETURN

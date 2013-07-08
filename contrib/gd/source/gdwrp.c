/*
 * $Id: gdwrp.c 9773 2012-10-19 08:29:51Z andijahja $
 */

/*
 * Harbour Project source code:
 * GD graphic library low level (client api) interface code.
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

/* NOTE: we need this to prevent base types redefinition */
#define _CLIPDEFS_H
#if defined(HB_OS_WIN_USED)
   #include <windows.h>
#endif

#include "hbapi.h"
#include "hbapiitm.h"
#include "hbfast.h"
#include "hbstack.h"
#include "hbapierr.h"
#include "hbapifs.h"
//#include "hrbdll.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "gd.h"
#include "gdfontt.h"
#include "gdfonts.h"
#include "gdfontmb.h"
#include "gdfontl.h"
#include "gdfontg.h"

#define IMAGE_JPEG     1
#define IMAGE_GIF      2
#define IMAGE_PNG      3
#define IMAGE_WBMP     4
#define IMAGE_GD       5


/* ---------------------------------------------------------------------------*/
/* Internal functions                                                         */
/* ---------------------------------------------------------------------------*/

static void * LoadImageFromHandle( FHANDLE fhandle, int sz )
{
   void *iptr;

   if ( !( fhandle ) )
   {
      fhandle = 0; // 0 = std input
   }

   /* Read file */
   iptr = ( BYTE * ) hb_xgrab( sz );
   hb_fsReadLarge( fhandle, ( BYTE *) iptr, (ULONG) sz );
   //   TraceLog( NULL, "Error dim %i, read %i", sz, iRead );

   return iptr;

}

/* ---------------------------------------------------------------------------*/

static void * LoadImageFromFile( const char *szFile, int *sz )
{
   void *iptr;
   FHANDLE fhandle;

   if ( ( fhandle = hb_fsOpen( szFile, FO_READ ) ) != FS_ERROR )
   {
      /* get lenght */
      *sz = hb_fsSeek( fhandle, 0, FS_END );
      /* rewind */
      hb_fsSeek( fhandle, 0, FS_SET );

      /* Read file */
      iptr = ( BYTE * ) hb_xgrab( *sz );
      hb_fsReadLarge( fhandle, ( BYTE *) iptr, (ULONG) *sz );
      //   TraceLog( NULL, "Error dim %i, read %i", sz, iRead );

      /* Close file */
      hb_fsClose( fhandle );
   }
   else
   {
      /* File error */
      iptr = NULL;
      *sz  = 0;
   }

   return iptr;

}

/* ---------------------------------------------------------------------------*/

static void SaveImageToHandle( FHANDLE fhandle, void * iptr, int sz )
{
   if ( !(fhandle) )
   {
      fhandle = 1; // 1 = std output
   }

   /* Write Image */
   hb_fsWriteLarge( fhandle, ( BYTE *) iptr, (ULONG) sz );
}

/* ---------------------------------------------------------------------------*/

static void SaveImageToFile( const char * szFile, void * iptr, int sz )
{
   FHANDLE fhandle;

   if ( ( fhandle = hb_fsCreate( szFile, FC_NORMAL ) ) != FS_ERROR )
   {
      /* Write Image */
      SaveImageToHandle( fhandle, ( BYTE *) iptr, (ULONG) sz );

      /* Close file */
      hb_fsClose( fhandle );
   }
}

/* ---------------------------------------------------------------------------*/

static void AddImageToFile( const char * szFile, void * iptr, int sz )
{
   FHANDLE fhandle;

   if ( ( fhandle = hb_fsOpen( szFile, FO_READWRITE ) ) != FS_ERROR )
   {
      /* move to end of file */
      hb_fsSeek(fhandle, 0, FS_END);

      /* Write Image */
      SaveImageToHandle( fhandle, ( BYTE *) iptr, (ULONG) sz );

      /* Close file */
      hb_fsClose( fhandle );
   }
}

/* ---------------------------------------------------------------------------*/

static void GDImageCreateFrom( int nType )
{
   gdImagePtr im = NULL;
   const char *szFile;
   int sz;
   void *iptr;

   //TraceLog( NULL, "Params = %i, 1 = %i, 2 = %i \n\r", hb_pcount(), hb_parinfo( 1 ), hb_parinfo( 2 ) );

   if ( hb_pcount() == 1 &&
        ( hb_parinfo( 1 ) & HB_IT_STRING )
      )
   {
      /* Retrieve file name */
      szFile = hb_parcx( 1 );
      /* Retrieve image */
      iptr   = LoadImageFromFile( szFile, &sz );
   }

   /* From Image Pointer + size */
   else if ( hb_pcount() == 2 &&
        ( hb_parinfo( 1 ) & HB_IT_POINTER ) &&
        ( hb_parinfo( 2 ) & HB_IT_NUMERIC )
      )
   {

      /* Retrieve image pointer */
      iptr = hb_parptr( 1 );
      /* Retrieve image size */
      sz     = hb_parni( 2 );
   }

   /* From file handle */
   else if ( hb_pcount() == 2 &&
        ( hb_parinfo( 1 ) & HB_IT_NUMERIC ) &&
        ( hb_parinfo( 2 ) & HB_IT_NUMERIC )
      )
   {

      FHANDLE fhandle;

      /* Retrieve file handle */
      fhandle = ( hb_parinfo( 1 ) & HB_IT_NUMERIC ) ? hb_parnl( 1 ) : 0; // 0 = std input
      /* Retrieve image size */
      sz     = hb_parni( 2 );

      /* retrieve image from handle */
      iptr = LoadImageFromHandle( fhandle, sz );

   }
   else
   {

      // Parameter error
      hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
         "GDIMAGECREATEFROM*", 2,
         hb_paramError( 1 ), hb_paramError( 2 ) );
      return;
   }

   if ( iptr && sz )
   {
      /* Create Image */
      switch ( nType )
      {
         case IMAGE_JPEG :
              im = gdImageCreateFromJpegPtr( sz, ( BYTE *) iptr );
              break;
         case IMAGE_GIF  :
              im = gdImageCreateFromGifPtr( sz, ( BYTE *) iptr );
              break;
         case IMAGE_PNG  :
              im = gdImageCreateFromPngPtr( sz, ( BYTE *) iptr );
              break;
         case IMAGE_WBMP :
              im = gdImageCreateFromWBMPPtr( sz, ( BYTE *) iptr );
              break;
         case IMAGE_GD   :
              im = gdImageCreateFromGdPtr( sz, ( BYTE *) iptr );
              break;
      }

      /* Return image pointer */
      hb_retptr( im );

      /* Free memory */
      hb_xfree( iptr );

   }
}

/* ---------------------------------------------------------------------------*/

static void GDImageSaveTo( int nType )
{
   if ( hb_pcount() >= 1 &&
        hb_parinfo( 1 ) & HB_IT_POINTER
      )
   {
      gdImagePtr im;
      const char *szFile;
      int sz = 0;
      void *iptr = NULL;
      FHANDLE fhandle;
      int level = 0, fg = 0;

      /* Retrieve image pointer */
       im = (gdImagePtr)hb_parptr(1);

      /* Get file name or an output handler or NIL it I want a return string */
      if ( !( ISNIL(2) ||
              hb_parinfo( 2 ) & HB_IT_STRING ||
              hb_parinfo( 2 ) & HB_IT_NUMERIC ) )
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0,
            "Second argument must be NIL or numeric or a string.",
            "GDIMAGE* (Save functions)", 2,
            hb_paramError( 2 ) );
         return;
      }

      /* Retrieve compression level */
      //TraceLog( NULL, "Count = %i\n\r", hb_pcount() );
      /* check if is numeric */
      if ( !( ISNIL(3) || hb_parinfo( 3 ) & HB_IT_NUMERIC ) )
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0,
            "Tirdh argument must be NIL or numeric.",
            "GDIMAGE* (Save functions)", 1,
            hb_paramError( 3 ) );
         return;
      }

      if ( nType == IMAGE_JPEG )
      {
         /* check range */
         level = ( hb_parinfo( 3 ) & HB_IT_NUMERIC ? hb_parni( 3 ) : -1 );
         if ( !( level >= -1 && level <= 95 ) )
         {
            hb_errRT_BASE_SubstR( EG_ARG, 0,
               "Compression level must be -1 (default) or a value between 0 and 95.",
               "GDIMAGEJPEG (Save functions)", 1,
               hb_paramError( 3 ) );
            return;
         }
      }
      else if ( nType == IMAGE_PNG )
      {
         /* check range */
         level = ( hb_parinfo( 3 ) & HB_IT_NUMERIC ? hb_parni( 3 ) : -1 );
         if ( !( level >= -1 && level <= 9 ) )
         {
            hb_errRT_BASE_SubstR( EG_ARG, 0,
               "Compression level must be -1 (default) or a value between 0 and 9.",
               "GDIMAGEPNG (Save functions)", 1,
               hb_paramError( 3 ) );
            return;
         }
      }
      else if ( nType == IMAGE_WBMP )
      {
         if ( !( hb_parinfo( 3 ) & HB_IT_NUMERIC ) )
         {
            hb_errRT_BASE_SubstR( EG_ARG, 0,
               "Foreground color nedeed",
               "GDIMAGEWBMP (Save functions)", 1,
               hb_paramError( 3 ) );
            return;
         }
         fg = hb_parni( 3 );
      }

      switch ( nType )
      {
        case IMAGE_JPEG  :
             /* Get image Ptr */
             iptr = gdImageJpegPtr (im, &sz, level);
             break;
        case IMAGE_GIF   :
             /* Get image Ptr */
             iptr = gdImageGifPtr (im, &sz);
             break;
        case IMAGE_PNG   :
             /* Get image Ptr */
             iptr = gdImagePngPtrEx (im, &sz, level);
             break;
        case IMAGE_WBMP  :
             /* Get image Ptr */
             iptr = gdImageWBMPPtr (im, &sz, fg);
             break;
        case IMAGE_GD    :
             /* Get image Ptr */
             iptr = gdImageGdPtr (im, &sz);
             break;
      }

      /* If i get a file name */
      if ( hb_parinfo( 2 ) & HB_IT_STRING )
      {
         szFile = hb_parcx( 2 );
         SaveImageToFile( szFile, iptr, sz );

      }

      /* Write to file handle (1 = std output) */
      else if ( hb_parinfo( 2 ) & HB_IT_NUMERIC )
      {
         /* Write to std output or to a passed file */
         fhandle = ( hb_parnl( 2 ) > -1 ) ? hb_parnl( 2 ) : 1 ; // 1 = std output

         /* Write Image */
         SaveImageToHandle( fhandle, iptr, sz );
      }

      /* Return image as string) */
      else
      {
         /* Return as string */
         hb_retclen( (const char *) iptr, (ULONG) sz );
      }

      /* Free memory */
      gdFree( iptr );
   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGE* (save functions)", 1,
            hb_paramError( 1 ) );
         return;
      }
   }
}

/* ************************* WRAPPED FUNCTIONS ****************************** */

HB_FUNC( GDVERSION ) // gdImagePtr gdImageCreate(sx, sy)
{
#if ( GD_VERS >= 2033 )
   hb_retc( "GD Version 2.0.33" );
#else
   hb_retc( "GD Version 2.0.28" );
#endif
}

/* ---------------------------------------------------------------------------*/

/* ---------------------------------------------------------------------------*/
/* IMAGE CREATION, DESTRUCTION, LOADING AND SAVING                            */
/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGECREATE ) // gdImagePtr gdImageCreate(sx, sy)
{
   if ( hb_pcount() == 2 &&
        hb_parinfo( 1 ) & HB_IT_NUMERIC &&
        hb_parinfo( 2 ) & HB_IT_NUMERIC )
   {
      gdImagePtr im;
      int sx, sy;

      /* Retrieve dimensions */
      sx = hb_parni( 1 );
      sy = hb_parni( 2 );

      /* Create the image */
      im = gdImageCreate( sx, sy );

      /* Return image pointer */
      hb_retptr( im );
   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGECREATE", 2,
            hb_paramError( 1 ), hb_paramError( 2 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGECREATEPALETTE ) // gdImagePtr gdImageCreatePalette(sx, sy)
{
   // Alias of GDCreate()
   HB_FUNCNAME( GDIMAGECREATE )();
}


/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGECREATETRUECOLOR ) // gdImageCreateTrueColor(sx, sy)
{
   if ( hb_pcount() == 2 &&
        hb_parinfo( 1 ) & HB_IT_NUMERIC &&
        hb_parinfo( 2 ) & HB_IT_NUMERIC )
   {
      gdImagePtr im;
      int sx, sy;

      /* Retrieve dimensions */
      sx = hb_parni( 1 );
      sy = hb_parni( 2 );

      /* Create the image */
      im = gdImageCreateTrueColor( sx, sy );

      /* Return image pointer */
      hb_retptr( im );
   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGECREATETRUECOLOR", 2,
            hb_paramError( 1 ), hb_paramError( 2 ) );
         return;
      }
   }
}


/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGECREATEFROMJPEG ) // gdImageCreateFromJpegPtr(int size, void *data)
                                 // implementation: gdImagePtr gdImageCreateFromJpeg( char *szFile )
{
     GDImageCreateFrom( IMAGE_JPEG );
}


/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGECREATEFROMGIF ) // gdImageCreateFromGifPtr(int size, void *data)
                                 // implementation: gdImagePtr gdImageCreateFromGif( char *szFile )
{
   GDImageCreateFrom( IMAGE_GIF );
}


/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGECREATEFROMPNG ) // gdImageCreateFromPngPtr(int size, void *data)
                                 // implementation: gdImagePtr gdImageCreateFromPng( char *szFile )
{
   GDImageCreateFrom( IMAGE_PNG );
}


/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGECREATEFROMWBMP ) // gdImagePtr gdImageCreateFromWBMPPtr (int size, void *data)
                                 // implementation: gdImagePtr gdImageCreateFromWBMP ( char *szFile )
{
   GDImageCreateFrom( IMAGE_WBMP );
}


/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGECREATEFROMGD ) // gdImagePtr gdImageCreateFromGdPtr (int size, void *data)
                               // implementation: gdImagePtr gdImageCreateFromGd ( char *szFile )
{
   GDImageCreateFrom( IMAGE_GD );
}


/* ---------------------------------------------------------------------------*/

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGEJPEG ) // original: void gdImageJpeg(gdImagePtr im, FILE *out)
                      // implementation: void gdImageJpeg(gdImagePtr im, char *szFile)
{
   GDImageSaveTo( IMAGE_JPEG );
}


/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGEGIF ) // original: void gdImageGif(gdImagePtr im, FILE *out)
                      // implementation: void gdImageGif(gdImagePtr im, char *szFile)
{
   GDImageSaveTo( IMAGE_GIF );
}

HB_FUNC( GDIMAGEPNG ) // original: void gdImagePngEx(gdImagePtr im, FILE *out, int level)
                      // implementation: void gdImagePng(gdImagePtr im, char *szFile [, int level] )
{
   GDImageSaveTo( IMAGE_PNG );
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGEWBMP ) // original: void gdImageWBMP(gdImagePtr im, FILE *out)
                      // implementation: void gdImageWBMP(gdImagePtr im, char *szFile, int fg)
{
   GDImageSaveTo( IMAGE_WBMP );
}


/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGEGD ) // original: void gdImageGD(gdImagePtr im, FILE *out)
                      // implementation: void gdImageGD(gdImagePtr im, char *szFile)
{
   GDImageSaveTo( IMAGE_GD );
}


/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGEDESTROY ) // gdImageDestroy(gdImagePtr im)
{
   if ( hb_pcount() == 1 &&
        hb_parinfo( 1 ) & HB_IT_POINTER )
   {
      gdImagePtr im;

      /* Retrieve image pointer */
      im = (gdImagePtr)hb_parptr( 1 );

      /* Destroy the image */
      gdImageDestroy( im );

   }
}


/* ---------------------------------------------------------------------------*/

/* ---------------------------------------------------------------------------*/
/* DRAWING FUNCTIONS                                                          */
/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGESETPIXEL ) // void gdImageSetPixel(gdImagePtr im, int x, int y, int color)
{
   if ( hb_pcount() == 4 &&
        hb_parinfo( 1 ) & HB_IT_POINTER &&
        hb_parinfo( 2 ) & HB_IT_NUMERIC &&
        hb_parinfo( 3 ) & HB_IT_NUMERIC &&
        hb_parinfo( 4 ) & HB_IT_NUMERIC
      )
   {
      gdImagePtr im;
      int x, y;
      int color;

      /* Retrieve image pointer */
      im = (gdImagePtr)hb_parptr( 1 );

      /* Retrieve coord values */
      x = hb_parni( 2 );
      y = hb_parni( 3 );

      /* Retrieve color value */
      color = hb_parni( 4 );

      /* Draw a rectangle */
      gdImageSetPixel(im, x, y, color);

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGESETPIXEL", 4,
            hb_paramError( 1 ), hb_paramError( 2 ), hb_paramError( 3 ), hb_paramError( 4 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGELINE ) // void gdImageLine(gdImagePtr im, int x1, int y1, int x2, int y2, int color)
{
   if ( hb_pcount() == 6 &&
        hb_parinfo( 1 ) & HB_IT_POINTER &&
        hb_parinfo( 2 ) & HB_IT_NUMERIC &&
        hb_parinfo( 3 ) & HB_IT_NUMERIC &&
        hb_parinfo( 4 ) & HB_IT_NUMERIC &&
        hb_parinfo( 5 ) & HB_IT_NUMERIC &&
        hb_parinfo( 6 ) & HB_IT_NUMERIC
      )
   {
      gdImagePtr im;
      int x1, y1, x2, y2;
      int color;

      /* Retrieve image pointer */
      im = (gdImagePtr)hb_parptr( 1 );

      /* Retrieve coord values */
      x1 = hb_parni( 2 );
      y1 = hb_parni( 3 );
      x2 = hb_parni( 4 );
      y2 = hb_parni( 5 );

      /* Retrieve color value */
      color = hb_parni( 6 );

      /* Draw a rectangle */
      gdImageLine(im, x1, y1, x2, y2, color);

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGELINE", 6,
            hb_paramError( 1 ), hb_paramError( 2 ), hb_paramError( 3 ), hb_paramError( 4 ),
            hb_paramError( 5 ), hb_paramError( 6 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGEDASHEDLINE ) // void gdImageDashedLine(gdImagePtr im, int x1, int y1, int x2, int y2, int color)
{
   if ( hb_pcount() == 6 &&
        hb_parinfo( 1 ) & HB_IT_POINTER &&
        hb_parinfo( 2 ) & HB_IT_NUMERIC &&
        hb_parinfo( 3 ) & HB_IT_NUMERIC &&
        hb_parinfo( 4 ) & HB_IT_NUMERIC &&
        hb_parinfo( 5 ) & HB_IT_NUMERIC &&
        hb_parinfo( 6 ) & HB_IT_NUMERIC
      )
   {
      gdImagePtr im;
      int x1, y1, x2, y2;
      int color;

      /* Retrieve image pointer */
      im = (gdImagePtr)hb_parptr( 1 );

      /* Retrieve coord values */
      x1 = hb_parni( 2 );
      y1 = hb_parni( 3 );
      x2 = hb_parni( 4 );
      y2 = hb_parni( 5 );

      /* Retrieve color value */
      color = hb_parni( 6 );

      /* Draw a rectangle */
      gdImageDashedLine(im, x1, y1, x2, y2, color);

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGEDASHEDLINE", 6,
            hb_paramError( 1 ), hb_paramError( 2 ), hb_paramError( 3 ), hb_paramError( 4 ),
            hb_paramError( 5 ), hb_paramError( 6 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGEPOLYGON ) // original: void gdImagePolygon(gdImagePtr im, gdPointPtr points, int pointsTotal, int color)
                          // implementation: void gdImagePolygon(gdImagePtr im, gdPointPtr points, int color)
{
   if ( hb_pcount() == 3 &&
        hb_parinfo( 1 ) & HB_IT_POINTER &&
        hb_parinfo( 2 ) & HB_IT_ARRAY   &&
        hb_parinfo( 3 ) & HB_IT_NUMERIC
      )
   {
      gdImagePtr im;
      //gdPointPtr points;
      int pointsTotal;
      int color;
      PHB_ITEM pPoints;
      int i;

      /* Max Points of polygon */
      gdPoint points[50]; // TODO: make this dynamic

      /* Retrieve image pointer */
      im = (gdImagePtr)hb_parptr( 1 );

      /* Retrieve point array */
      pPoints     = hb_param( 2, HB_IT_ARRAY );
      pointsTotal = (int) hb_arrayLen( pPoints );

      for( i = 0; i < pointsTotal; i ++ )
      {
         PHB_ITEM pPoint = hb_arrayGetItemPtr( pPoints, i+1 );
         if( HB_IS_ARRAY( pPoint ) )
         {
            points[ i ].x = hb_arrayGetNI( pPoint, 1 );
            points[ i ].y = hb_arrayGetNI( pPoint, 2 );
         }
      }

      /* Retrieve color value */
      color = hb_parni( 3 );

      /* Draw a polygon */
      gdImagePolygon(im, (gdPointPtr) points, pointsTotal, color);

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGEPOLYGON", 3,
            hb_paramError( 1 ), hb_paramError( 2 ), hb_paramError( 3 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/
#if ( GD_VERS >= 2033 )
HB_FUNC( GDIMAGEOPENPOLYGON ) // original: void gdImageOpenPolygon(gdImagePtr im, gdPointPtr points, int pointsTotal, int color)
                          // implementation: void gdImageOpenPolygon(gdImagePtr im, gdPointPtr points, int color)
{
   if ( hb_pcount() == 3 &&
        hb_parinfo( 1 ) & HB_IT_POINTER &&
        hb_parinfo( 2 ) & HB_IT_ARRAY   &&
        hb_parinfo( 3 ) & HB_IT_NUMERIC
      )
   {
      gdImagePtr im;
      //gdPointPtr points;
      int pointsTotal;
      int color;
      PHB_ITEM pPoints;
      int i;

      /* Max Points of polygon */
      gdPoint points[50]; // TODO: make this dynamic

      /* Retrieve image pointer */
      im = (gdImagePtr)hb_parptr( 1 );

      /* Retrieve point array */
      pPoints     = hb_param( 2, HB_IT_ARRAY );
      pointsTotal = (int) hb_arrayLen( pPoints );

      for( i = 0; i < pointsTotal; i ++ )
      {
         PHB_ITEM pPoint = hb_arrayGetItemPtr( pPoints, i+1 );
         if( HB_IS_ARRAY( pPoint ) )
         {
            points[ i ].x = hb_arrayGetNI( pPoint, 1 );
            points[ i ].y = hb_arrayGetNI( pPoint, 2 );
         }
      }

      /* Retrieve color value */
      color = hb_parni( 3 );

      /* Draw a polygon */
      gdImageOpenPolygon(im, (gdPointPtr) points, pointsTotal, color);

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGEOPENPOLYGON", 3,
            hb_paramError( 1 ), hb_paramError( 2 ), hb_paramError( 3 ) );
         return;
      }
   }
}
#endif // ( GD_VERS >= 2033 )

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGERECTANGLE ) // void gdImageRectangle(gdImagePtr im, int x1, int y1, int x2, int y2, int color)
{
   if ( hb_pcount() == 6 &&
        hb_parinfo( 1 ) & HB_IT_POINTER &&
        hb_parinfo( 2 ) & HB_IT_NUMERIC &&
        hb_parinfo( 3 ) & HB_IT_NUMERIC &&
        hb_parinfo( 4 ) & HB_IT_NUMERIC &&
        hb_parinfo( 5 ) & HB_IT_NUMERIC &&
        hb_parinfo( 6 ) & HB_IT_NUMERIC
      )
   {
      gdImagePtr im;
      int x1, y1, x2, y2;
      int color;

      /* Retrieve image pointer */
      im = (gdImagePtr)hb_parptr( 1 );

      /* Retrieve coord values */
      x1 = hb_parni( 2 );
      y1 = hb_parni( 3 );
      x2 = hb_parni( 4 );
      y2 = hb_parni( 5 );

      /* Retrieve color value */
      color = hb_parni( 6 );

      /* Draw a rectangle */
      gdImageRectangle(im, x1, y1, x2, y2, color);

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGERECTANGLE", 6,
            hb_paramError( 1 ), hb_paramError( 2 ), hb_paramError( 3 ), hb_paramError( 4 ),
            hb_paramError( 5 ), hb_paramError( 6 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

//HB_FUNC( GDIMAGEELLIPSE ) // void gdImageEllipse(gdImagePtr im, int cx, int cy, int w, int h, int color)
//{
//   if ( hb_pcount() == 6 &&
//        hb_parinfo( 1 ) & HB_IT_POINTER &&
//        hb_parinfo( 2 ) & HB_IT_NUMERIC &&
//        hb_parinfo( 3 ) & HB_IT_NUMERIC &&
//        hb_parinfo( 4 ) & HB_IT_NUMERIC &&
//        hb_parinfo( 5 ) & HB_IT_NUMERIC &&
//        hb_parinfo( 6 ) & HB_IT_NUMERIC
//      )
//   {
//      gdImagePtr im;
//      int cx, cy, w, h, color;
//
//      /* Retrieve image pointer */
//      im = (gdImagePtr) hb_parptr( 1 );
//
//      /* Retrieve point values */
//      cx = hb_parni( 2 );
//      cy = hb_parni( 3 );
//      /* Retrieve width and height values */
//      w  = hb_parni( 4 );
//      h  = hb_parni( 5 );
//      /* Retrieve color value */
//      color = hb_parni( 6 );
//
//      /* Draw an ellipse */
//      gdImageEllipse(im, cx, cy, w, h, color);
//
//   }
//   else
//   {
//      // Parameter error
//      {
//         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
//            "GDIMAGEELLIPSE", 2,
//            hb_paramError( 1 ), hb_paramError( 2 ), hb_paramError( 3 ), hb_paramError( 4 ),
//            hb_paramError( 5 ), hb_paramError( 6 ) );
//         return;
//      }
//   }
//}
//
///* ---------------------------------------------------------------------------*/
//
HB_FUNC( GDIMAGEFILLEDPOLYGON ) // original: void gdImageFilledPolygon(gdImagePtr im, gdPointPtr points, int pointsTotal, int color)
                                // implementation: void gdImageFilledPolygon(gdImagePtr im, gdPointPtr points, int color)
{
   if ( hb_pcount() == 3 &&
        hb_parinfo( 1 ) & HB_IT_POINTER &&
        hb_parinfo( 2 ) & HB_IT_ARRAY   &&
        hb_parinfo( 3 ) & HB_IT_NUMERIC
      )
   {
      gdImagePtr im;
      //gdPointPtr points;
      int pointsTotal;
      int color;
      PHB_ITEM pPoints;
      int i;

      /* Max Points of polygon */
      gdPoint points[50];

      /* Retrieve image pointer */
      im = (gdImagePtr)hb_parptr( 1 );

      /* Retrieve point array */
      pPoints     = hb_param( 2, HB_IT_ARRAY );
      pointsTotal = (int) hb_arrayLen( pPoints );

      for( i = 0; i < pointsTotal; i ++ )
      {
         PHB_ITEM pPoint = hb_arrayGetItemPtr( pPoints, i+1 );
         if( HB_IS_ARRAY( pPoint ) )
         {
            points[ i ].x = hb_arrayGetNI( pPoint, 1 );
            points[ i ].y = hb_arrayGetNI( pPoint, 2 );
         }
      }

      /* Retrieve color value */
      color = hb_parni( 3 );

      /* Draw a filled polygon */
      gdImageFilledPolygon(im, (gdPointPtr) points, pointsTotal, color);

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGEFILLEDPOLYGON", 3,
            hb_paramError( 1 ), hb_paramError( 2 ), hb_paramError( 3 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGEFILLEDRECTANGLE ) // void gdImageFilledRectangle(gdImagePtr im, int x1, int y1, int x2, int y2, int color)
{
   if ( hb_pcount() == 6 &&
        hb_parinfo( 1 ) & HB_IT_POINTER &&
        hb_parinfo( 2 ) & HB_IT_NUMERIC &&
        hb_parinfo( 3 ) & HB_IT_NUMERIC &&
        hb_parinfo( 4 ) & HB_IT_NUMERIC &&
        hb_parinfo( 5 ) & HB_IT_NUMERIC &&
        hb_parinfo( 6 ) & HB_IT_NUMERIC
      )
   {
      gdImagePtr im;
      int x1, y1, x2, y2;
      int color;

      /* Retrieve image pointer */
      im = (gdImagePtr)hb_parptr( 1 );

      /* Retrieve coord values */
      x1 = hb_parni( 2 );
      y1 = hb_parni( 3 );
      x2 = hb_parni( 4 );
      y2 = hb_parni( 5 );

      /* Retrieve color value */
      color = hb_parni( 6 );

      /* Draw a filled rectangle */
      gdImageFilledRectangle(im, x1, y1, x2, y2, color);

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGEFILLEDRECTANGLE", 6,
            hb_paramError( 1 ), hb_paramError( 2 ), hb_paramError( 3 ), hb_paramError( 4 ),
            hb_paramError( 5 ), hb_paramError( 6 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGEARC ) // void gdImageArc(gdImagePtr im, int cx, int cy, int w, int h, int s, int e, int color)
{
   if ( hb_pcount() == 8 &&
        hb_parinfo( 1 ) & HB_IT_POINTER &&
        hb_parinfo( 2 ) & HB_IT_NUMERIC &&
        hb_parinfo( 3 ) & HB_IT_NUMERIC &&
        hb_parinfo( 4 ) & HB_IT_NUMERIC &&
        hb_parinfo( 5 ) & HB_IT_NUMERIC &&
        hb_parinfo( 6 ) & HB_IT_NUMERIC &&
        hb_parinfo( 7 ) & HB_IT_NUMERIC &&
        hb_parinfo( 8 ) & HB_IT_NUMERIC
      )
   {
      gdImagePtr im;
      int cx, cy, w, h, s, e, color;

      /* Retrieve image pointer */
      im = (gdImagePtr)hb_parptr( 1 );

      /* Retrieve point values */
      cx = hb_parni( 2 );
      cy = hb_parni( 3 );
      /* Retrieve width and height values */
      w  = hb_parni( 4 );
      h  = hb_parni( 5 );
      /* Retrieve starting and ending degree values */
      s  = hb_parni( 6 );
      e  = hb_parni( 7 );
      /* Retrieve color value */
      color = hb_parni( 8 );

      /* Draw an arc */
      gdImageArc(im, cx, cy, w, h, s, e, color);

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGEARC", 8,
            hb_paramError( 1 ), hb_paramError( 2 ), hb_paramError( 3 ), hb_paramError( 4 ),
            hb_paramError( 5 ), hb_paramError( 6 ), hb_paramError( 7 ), hb_paramError( 8 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGEFILLEDARC ) // void gdImageFilledArc(gdImagePtr im, int cx, int cy, int w, int h, int s, int e, int color[, int style])
{
   if ( hb_pcount() >= 8 &&
        hb_parinfo( 1 ) & HB_IT_POINTER &&
        hb_parinfo( 2 ) & HB_IT_NUMERIC &&
        hb_parinfo( 3 ) & HB_IT_NUMERIC &&
        hb_parinfo( 4 ) & HB_IT_NUMERIC &&
        hb_parinfo( 5 ) & HB_IT_NUMERIC &&
        hb_parinfo( 6 ) & HB_IT_NUMERIC &&
        hb_parinfo( 7 ) & HB_IT_NUMERIC &&
        hb_parinfo( 8 ) & HB_IT_NUMERIC
      )
   {
      gdImagePtr im;
      int cx, cy, w, h, s, e, color, style;

      /* Retrieve image pointer */
      im = (gdImagePtr)hb_parptr( 1 );

      /* Retrieve point values */
      cx = hb_parni( 2 );
      cy = hb_parni( 3 );
      /* Retrieve width and height values */
      w  = hb_parni( 4 );
      h  = hb_parni( 5 );
      /* Retrieve starting and ending degree values */
      s  = hb_parni( 6 );
      e  = hb_parni( 7 );
      /* Retrieve color value */
      color = hb_parni( 8 );

      /* Retrieve style value */
      style = ( hb_parinfo( 9 ) & HB_IT_NUMERIC ? hb_parni( 9 ) : gdNoFill );

      /* Draw a filled arc */
      gdImageFilledArc(im, cx, cy, w, h, s, e, color, style);

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGEFILLEDARC", 9,
            hb_paramError( 1 ), hb_paramError( 2 ), hb_paramError( 3 ), hb_paramError( 4 ),
            hb_paramError( 5 ), hb_paramError( 6 ), hb_paramError( 7 ), hb_paramError( 8 ),
            hb_paramError( 9 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGEFILLEDELLIPSE ) // void gdImageFilledEllipse(gdImagePtr im, int cx, int cy, int w, int h, int color)
{
   if ( hb_pcount() == 6 &&
        hb_parinfo( 1 ) & HB_IT_POINTER &&
        hb_parinfo( 2 ) & HB_IT_NUMERIC &&
        hb_parinfo( 3 ) & HB_IT_NUMERIC &&
        hb_parinfo( 4 ) & HB_IT_NUMERIC &&
        hb_parinfo( 5 ) & HB_IT_NUMERIC &&
        hb_parinfo( 6 ) & HB_IT_NUMERIC
      )
   {
      gdImagePtr im;
      int cx, cy, w, h, color;

      /* Retrieve image pointer */
      im = (gdImagePtr)hb_parptr( 1 );

      /* Retrieve point values */
      cx = hb_parni( 2 );
      cy = hb_parni( 3 );
      /* Retrieve width and height values */
      w  = hb_parni( 4 );
      h  = hb_parni( 5 );
      /* Retrieve color value */
      color = hb_parni( 6 );

      /* Draw a filled ellipse */
      gdImageFilledEllipse(im, cx, cy, w, h, color);

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGEFILLEDELLIPSE", 6,
            hb_paramError( 1 ), hb_paramError( 2 ), hb_paramError( 3 ), hb_paramError( 4 ),
            hb_paramError( 5 ), hb_paramError( 6 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGEFILLTOBORDER ) // void gdImageFillToBorder(gdImagePtr im, int x, int y, int border, int color)
{
   if ( hb_pcount() == 5 &&
        hb_parinfo( 1 ) & HB_IT_POINTER &&
        hb_parinfo( 2 ) & HB_IT_NUMERIC &&
        hb_parinfo( 3 ) & HB_IT_NUMERIC &&
        hb_parinfo( 4 ) & HB_IT_NUMERIC &&
        hb_parinfo( 5 ) & HB_IT_NUMERIC
      )
   {
      gdImagePtr im;
      int x, y, border, color;

      /* Retrieve image pointer */
      im = (gdImagePtr) hb_parptr( 1 );

      /* Retrieve point values */
      x = hb_parni( 2 );
      y = hb_parni( 3 );
      /* Retrieve color to use as border */
      border = hb_parni( 4 );
      /* Retrieve color value */
      color = hb_parni( 5 );

      /* Fill image to border */
      gdImageFillToBorder(im, x, y, border, color);

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGEFILLTOBORDER", 5,
            hb_paramError( 1 ), hb_paramError( 2 ), hb_paramError( 3 ), hb_paramError( 4 ),
            hb_paramError( 5 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGEFILL ) // void gdImageFill(gdImagePtr im, int x, int y, int color)
{
   if ( hb_pcount() == 4 &&
        hb_parinfo( 1 ) & HB_IT_POINTER &&
        hb_parinfo( 2 ) & HB_IT_NUMERIC &&
        hb_parinfo( 3 ) & HB_IT_NUMERIC &&
        hb_parinfo( 4 ) & HB_IT_NUMERIC
      )
   {
      gdImagePtr im;
      int x, y, color;

      /* Retrieve image pointer */
      im = (gdImagePtr) hb_parptr( 1 );

      /* Retrieve point values */
      x = hb_parni( 2 );
      y = hb_parni( 3 );
      /* Retrieve color value */
      color = hb_parni( 4 );

      /* Fill image */
      gdImageFill(im, x, y, color);

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGEFILL", 4,
            hb_paramError( 1 ), hb_paramError( 2 ), hb_paramError( 3 ), hb_paramError( 4 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGESETANTIALIASED ) // void gdImageSetAntiAliased(gdImagePtr im, int color)
{
   if ( hb_pcount() == 2 &&
        hb_parinfo( 1 ) & HB_IT_POINTER &&
        hb_parinfo( 2 ) & HB_IT_NUMERIC
      )
   {
      gdImagePtr im;
      int color;

      /* Retrieve image pointer */
      im = (gdImagePtr) hb_parptr( 1 );

      /* Retrieve color value */
      color = hb_parni( 2 );

      /* Set Antialias */
      gdImageSetAntiAliased(im, color);

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGESETANTIALIASED", 2,
            hb_paramError( 1 ), hb_paramError( 2 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGESETANTIALIASEDDONTBLEND ) // void gdImageSetAntiAliasedDontBlend(gdImagePtr im, int c, int dont_blend)
{
   if ( hb_pcount() == 3 &&
        hb_parinfo( 1 ) & HB_IT_POINTER &&
        hb_parinfo( 2 ) & HB_IT_NUMERIC &&
        hb_parinfo( 3 ) & HB_IT_NUMERIC
      )
   {
      gdImagePtr im;
      int color;
      int dont_blend;

      /* Retrieve image pointer */
      im = (gdImagePtr) hb_parptr( 1 );

      /* Retrieve color value */
      color = hb_parni( 2 );

      /* Retrieve dont_blend value */
      dont_blend = hb_parni( 3 );

      /* Set Antialias but don't blend */
      gdImageSetAntiAliasedDontBlend(im, color, dont_blend);

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGESETANTIALIASEDDONTBLEND", 3,
            hb_paramError( 1 ), hb_paramError( 2 ), hb_paramError( 3 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGESETBRUSH ) // void gdImageSetBrush(gdImagePtr im, gdImagePtr brush)
{
   if ( hb_pcount() == 2 &&
        hb_parinfo( 1 ) & HB_IT_POINTER &&
        hb_parinfo( 2 ) & HB_IT_POINTER
      )
   {
      gdImagePtr im;
      gdImagePtr brush;

      /* Retrieve image pointer */
      im = (gdImagePtr) hb_parptr( 1 );

      /* Retrieve brush pointer */
      brush = (gdImagePtr)hb_parptr( 2 );

      /* Set Brush */
      gdImageSetBrush(im, brush);

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGESETBRUSH", 2,
            hb_paramError( 1 ), hb_paramError( 2 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGESETTILE ) // void gdImageSetTile(gdImagePtr im, gdImagePtr tile)
{
   if ( hb_pcount() == 2 &&
        hb_parinfo( 1 ) & HB_IT_POINTER &&
        hb_parinfo( 2 ) & HB_IT_POINTER
      )
   {
      gdImagePtr im;
      gdImagePtr tile;

      /* Retrieve image pointer */
      im = (gdImagePtr) hb_parptr( 1 );

      /* Retrieve tile pointer */
      tile = (gdImagePtr)hb_parptr( 2 );

      /* Set Tile */
      gdImageSetTile(im, tile);

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGESETTILE", 2,
            hb_paramError( 1 ), hb_paramError( 2 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGESETSTYLE ) // original: void gdImageSetStyle(gdImagePtr im, int *style, int styleLength)
                           // implementation: void gdImageSetStyle(gdImagePtr im, int *style)
{
   if ( hb_pcount() == 2 &&
        hb_parinfo( 1 ) & HB_IT_POINTER &&
        hb_parinfo( 2 ) & HB_IT_ARRAY
      )
   {
      gdImagePtr im;
      PHB_ITEM pStyles;
      int styleLength;
      int i;

      /* Max numbery of Styles */
      int styles[50]; // TODO: make this dynamic

      /* Retrieve image pointer */
      im = (gdImagePtr) hb_parptr( 1 );

      /* Retrieve style array */
      pStyles     = hb_param( 2, HB_IT_ARRAY );
      styleLength = (int) hb_arrayLen( pStyles );

      for( i = 0; i < styleLength; i ++ )
      {
         styles[ i ] = hb_arrayGetNI( pStyles, i+1 );
      }

      /* Set style */
      gdImageSetStyle(im, (int *) styles, styleLength);
   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGESETSTYLE", 2,
            hb_paramError( 1 ), hb_paramError( 2 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGESETTHICKNESS ) // void gdImageSetThickness(gdImagePtr im, int thickness)
{
   if ( hb_pcount() == 2 &&
        hb_parinfo( 1 ) & HB_IT_POINTER &&
        hb_parinfo( 2 ) & HB_IT_NUMERIC
      )
   {
      gdImagePtr im;
      int thickness;
      int oldthick;

      /* Retrieve image pointer */
      im = (gdImagePtr) hb_parptr( 1 );

      /* Retrieve thickness value */
      thickness = hb_parni( 2 );

      /* Previous value */
      oldthick = im->thick;

      /* Set thickness */
      gdImageSetThickness(im, thickness);

      /* Return previous */
      hb_retni( oldthick );

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGESETTHICKNESS", 2,
            hb_paramError( 1 ), hb_paramError( 2 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGEALPHABLENDING ) // void gdImageAlphaBlending(gdImagePtr im, int blending)
{
   if ( hb_pcount() == 2 &&
        hb_parinfo( 1 ) & HB_IT_POINTER &&
        hb_parinfo( 2 ) & HB_IT_LOGICAL
      )
   {
      gdImagePtr im;
      int blending;

      /* Retrieve image pointer */
      im = (gdImagePtr) hb_parptr( 1 );

      /* Retrieve blending value - logical */
      blending = hb_parl( 2 );

      /* Set blending */
      gdImageAlphaBlending(im, blending);

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGEALPHABLENDING", 2,
            hb_paramError( 1 ), hb_paramError( 2 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGESAVEALPHA ) // void gdImageSaveAlpha(gdImagePtr im, int saveFlag)
{
   if ( hb_pcount() == 2 &&
        hb_parinfo( 1 ) & HB_IT_POINTER &&
        hb_parinfo( 2 ) & HB_IT_LOGICAL
      )
   {
      gdImagePtr im;
      int saveFlag;

      /* Retrieve image pointer */
      im = (gdImagePtr) hb_parptr( 1 );

      /* Retrieve saveFlag value - logical */
      saveFlag = hb_parl( 2 );

      /* Set saveFlag */
      gdImageSaveAlpha(im, saveFlag);

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGESAVEALPHA", 2,
            hb_paramError( 1 ), hb_paramError( 2 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGESETCLIP ) // void gdImageSetClip(gdImagePtr im, int x1, int y1, int x2, int y2)
{
   if ( hb_pcount() == 5 &&
        hb_parinfo( 1 ) & HB_IT_POINTER &&
        hb_parinfo( 2 ) & HB_IT_NUMERIC &&
        hb_parinfo( 3 ) & HB_IT_NUMERIC &&
        hb_parinfo( 4 ) & HB_IT_NUMERIC &&
        hb_parinfo( 5 ) & HB_IT_NUMERIC
      )
   {
      gdImagePtr im;
      int x1, y1, x2, y2;

      /* Retrieve image pointer */
      im = (gdImagePtr) hb_parptr( 1 );

      /* Retrieve coords value */
      x1 = hb_parni( 2 );
      y1 = hb_parni( 3 );
      x2 = hb_parni( 4 );
      y2 = hb_parni( 5 );

      /* Set clipping rectangle */
      gdImageSetClip(im, x1, y1, x2, y2);

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGESETCLIP", 5,
            hb_paramError( 1 ), hb_paramError( 2 ), hb_paramError( 3 ), hb_paramError( 4 ),
            hb_paramError( 5 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGEGETCLIP ) // original: void gdImageGetClip(gdImagePtr im, int *x1P, int *y1P, int *x2P, int *y2P)
                          // implementation: array gdImageGetClip(gdImagePtr im)
{
   if ( hb_pcount() == 1 &&
        hb_parinfo( 1 ) & HB_IT_POINTER
      )
   {
      gdImagePtr im;
      int x1, y1, x2, y2;
      HB_ITEM pClipArray;

      pClipArray.type = HB_IT_NIL;

      /* Retrieve image pointer */
      im = (gdImagePtr) hb_parptr( 1 );

      /* Get clipping rectangle */
      gdImageGetClip(im, &x1, &y1, &x2, &y2);

      /* Return clipping rectangle value in an array */
      hb_arrayNew( &pClipArray, 4 );
      hb_itemPutNI( hb_arrayGetItemPtr( &pClipArray, 1 ), x1 );
      hb_itemPutNI( hb_arrayGetItemPtr( &pClipArray, 2 ), y1 );
      hb_itemPutNI( hb_arrayGetItemPtr( &pClipArray, 3 ), x2 );
      hb_itemPutNI( hb_arrayGetItemPtr( &pClipArray, 4 ), y2 );

      /* return array */
      hb_itemReturnForward( &pClipArray );

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGEGETCLIP", 1,
            hb_paramError( 1 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

/* ---------------------------------------------------------------------------*/
/* QUERY FUNCTIONS                                                            */
/* ---------------------------------------------------------------------------*/


HB_FUNC( GDIMAGECOLORSTOTAL ) // int gdImageColorsTotal(gdImagePtr im)
{
   if ( hb_pcount() == 1 &&
        hb_parinfo( 1 ) & HB_IT_POINTER
      )
   {
      gdImagePtr im;

      /* Retrieve image pointer */
      im = (gdImagePtr) hb_parptr( 1 );

      /* Get Colors total */
      hb_retni( gdImageColorsTotal(im) );

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGECOLORSTOTAL", 1,
            hb_paramError( 1 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGEALPHA ) // int gdImageAlpha(gdImagePtr im, int color)
{
   if ( hb_pcount() == 2 &&
        hb_parinfo( 1 ) & HB_IT_POINTER &&
        hb_parinfo( 2 ) & HB_IT_NUMERIC
      )
   {
      gdImagePtr im;
      int color;

      /* Retrieve image pointer */
      im = (gdImagePtr) hb_parptr( 1 );

      /* Retrieve saveFlag value */
      color = hb_parni( 2 );

      /* Get Alpha Level */
      hb_retni( gdImageAlpha(im, color) );

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGEALPHA", 2,
            hb_paramError( 1 ), hb_paramError( 2 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGERED ) // int gdImageRed(gdImagePtr im, int color)
{
   if ( hb_pcount() == 2 &&
        hb_parinfo( 1 ) & HB_IT_POINTER &&
        hb_parinfo( 2 ) & HB_IT_NUMERIC
      )
   {
      gdImagePtr im;
      int color;

      /* Retrieve image pointer */
      im = (gdImagePtr) hb_parptr( 1 );

      /* Retrieve saveFlag value */
      color = hb_parni( 2 );

      /* Get Red Level */
      hb_retni( gdImageRed(im, color) );

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGERED", 2,
            hb_paramError( 1 ), hb_paramError( 2 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGEGREEN ) // int gdImageGreen(gdImagePtr im, int color)
{
   if ( hb_pcount() == 2 &&
        hb_parinfo( 1 ) & HB_IT_POINTER &&
        hb_parinfo( 2 ) & HB_IT_NUMERIC
      )
   {
      gdImagePtr im;
      int color;

      /* Retrieve image pointer */
      im = (gdImagePtr) hb_parptr( 1 );

      /* Retrieve saveFlag value */
      color = hb_parni( 2 );

      /* Get Green Level */
      hb_retni( gdImageGreen(im, color) );

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGEGREEN", 2,
            hb_paramError( 1 ), hb_paramError( 2 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGEBLUE ) // int gdImageBlue(gdImagePtr im, int color)
{
   if ( hb_pcount() == 2 &&
        hb_parinfo( 1 ) & HB_IT_POINTER &&
        hb_parinfo( 2 ) & HB_IT_NUMERIC
      )
   {
      gdImagePtr im;
      int color;

      /* Retrieve image pointer */
      im = (gdImagePtr) hb_parptr( 1 );

      /* Retrieve saveFlag value */
      color = hb_parni( 2 );

      /* Get Blue Level */
      hb_retni( gdImageBlue(im, color) );

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGEBLUE", 2,
            hb_paramError( 1 ), hb_paramError( 2 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGESX ) // int gdImageSX(gdImagePtr im)
{
   if ( hb_pcount() == 1 &&
        hb_parinfo( 1 ) & HB_IT_POINTER
      )
   {
      gdImagePtr im;

      /* Retrieve image pointer */
      im = (gdImagePtr) hb_parptr( 1 );

      /* Get Image Width in pixels */
      hb_retni( gdImageSX(im) );

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGESX", 1,
            hb_paramError( 1 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGESY ) // int gdImageSX(gdImagePtr im)
{
   if ( hb_pcount() == 1 &&
        hb_parinfo( 1 ) & HB_IT_POINTER
      )
   {
      gdImagePtr im;

      /* Retrieve image pointer */
      im = (gdImagePtr) hb_parptr( 1 );

      /* Get Image Height in pixels */
      hb_retni( gdImageSY(im) );

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGESY", 1,
            hb_paramError( 1 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGEGETPIXEL ) // int gdImageGetPixel(gdImagePtr im, int x, int y)
{
   if ( hb_pcount() == 3 &&
        hb_parinfo( 1 ) & HB_IT_POINTER &&
        hb_parinfo( 2 ) & HB_IT_NUMERIC &&
        hb_parinfo( 3 ) & HB_IT_NUMERIC
      )
   {
      gdImagePtr im;
      int x,y;

      /* Retrieve image pointer */
      im = (gdImagePtr) hb_parptr( 1 );

      /* Retrieve coord value */
      x = hb_parni( 2 );
      y = hb_parni( 3 );

      /* Get Color of a pixel */
      hb_retni( gdImageGetPixel(im, x, y) );

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGEGETPIXEL", 3,
            hb_paramError( 1 ), hb_paramError( 2 ), hb_paramError( 3 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGEBOUNDSSAFE ) // int gdImageBoundsSafe(gdImagePtr im, int x, int y)
{
   if ( hb_pcount() == 3 &&
        hb_parinfo( 1 ) & HB_IT_POINTER &&
        hb_parinfo( 2 ) & HB_IT_NUMERIC &&
        hb_parinfo( 3 ) & HB_IT_NUMERIC
      )
   {
      gdImagePtr im;
      int x,y;

      /* Retrieve image pointer */
      im = (gdImagePtr) hb_parptr( 1 );

      /* Retrieve coord value */
      x = hb_parni( 2 );
      y = hb_parni( 3 );

      /* Get if pixel in Clipping region */
      hb_retl( gdImageBoundsSafe(im, x, y) );

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGEBOUNDSSAFE", 3,
            hb_paramError( 1 ), hb_paramError( 2 ), hb_paramError( 3 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGEGETINTERLACED ) // int gdImageGetInterlaced(gdImagePtr im)
{
   if ( hb_pcount() == 1 &&
        hb_parinfo( 1 ) & HB_IT_POINTER
      )
   {
      gdImagePtr im;

      /* Retrieve image pointer */
      im = (gdImagePtr) hb_parptr( 1 );

      /* Image is interlaced ? */
      hb_retl( gdImageGetInterlaced(im) );

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGEGETINTERLACED", 3,
            hb_paramError( 1 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGEGETTRANSPARENT ) // int gdImageGetTransparent(gdImagePtr im)
{
   if ( hb_pcount() == 1 &&
        hb_parinfo( 1 ) & HB_IT_POINTER
      )
   {
      gdImagePtr im;

      /* Retrieve image pointer */
      im = (gdImagePtr) hb_parptr( 1 );

      /* Image is trasparent ? */
      hb_retl( gdImageGetTransparent(im) );

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGEGETTRANSPARENT", 1,
            hb_paramError( 1 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGETRUECOLOR ) // int gdImageTrueColor(gdImagePtr im)
{
   if ( hb_pcount() == 1 &&
        hb_parinfo( 1 ) & HB_IT_POINTER
      )
   {
      gdImagePtr im;

      /* Retrieve image pointer */
      im = (gdImagePtr) hb_parptr( 1 );

      /* Is TrueColor ? */
      hb_retl( gdImageTrueColor(im) );

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGETRUECOLOR", 1,
            hb_paramError( 1 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGETRUECOLORTOPALETTE ) // void gdImageTrueColorToPalette (gdImagePtr im, int ditherFlag, int colorsWanted)
{
   if ( hb_pcount() == 3 &&
        hb_parinfo( 1 ) & HB_IT_POINTER &&
        hb_parinfo( 2 ) & HB_IT_LOGICAL &&
        hb_parinfo( 3 ) & HB_IT_NUMERIC
      )
   {
      gdImagePtr im;
      int ditherFlag, colorsWanted;

      /* Retrieve image pointer */
      im = (gdImagePtr) hb_parptr( 1 );

      /* Retrieve ditherFlag - logical */
      ditherFlag = hb_parl( 2 );

      /* Retrieve number of color wanted */
      colorsWanted = hb_parni( 3 );

      /* Converts a truecolor image to a palette-based image */
      gdImageTrueColorToPalette(im, ditherFlag, colorsWanted);

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGETRUECOLORTOPALETTE", 3,
            hb_paramError( 1 ), hb_paramError( 2 ), hb_paramError( 3 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGECREATEPALETTEFROMTRUECOLOR ) // gdImagePtr gdImageCreatePaletteFromTrueColor(gdImagePtr im, int ditherFlag, int colorsWanted)
{
   if ( hb_pcount() == 3 &&
        hb_parinfo( 1 ) & HB_IT_POINTER &&
        hb_parinfo( 2 ) & HB_IT_LOGICAL &&
        hb_parinfo( 3 ) & HB_IT_NUMERIC
      )
   {
      gdImagePtr im;
      gdImagePtr imNew;
      int ditherFlag, colorsWanted;

      /* Retrieve image pointer */
      im = (gdImagePtr) hb_parptr( 1 );

      /* Retrieve ditherFlag - logical */
      ditherFlag = hb_parl( 2 );

      /* Retrieve number of color wanted */
      colorsWanted = hb_parni( 3 );

      /* Converts a truecolor image to a palette-based image and return the image */
      imNew = gdImageCreatePaletteFromTrueColor(im, ditherFlag, colorsWanted);

      /* Return image pointer */
      hb_retptr( imNew );

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGECREATEPALETTEFROMTRUECOLOR", 3,
            hb_paramError( 1 ), hb_paramError( 2 ), hb_paramError( 3 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGEPALETTEPIXEL ) // int gdImagePalettePixel(gdImagePtr im, int x, int y)
{
   if ( hb_pcount() == 3 &&
        hb_parinfo( 1 ) & HB_IT_POINTER &&
        hb_parinfo( 2 ) & HB_IT_NUMERIC &&
        hb_parinfo( 3 ) & HB_IT_NUMERIC
      )
   {
      gdImagePtr im;
      int x,y;

      /* Retrieve image pointer */
      im = (gdImagePtr) hb_parptr( 1 );

      /* Retrieve coord value */
      x = hb_parni( 2 );
      y = hb_parni( 3 );

      /* Get Color of a pixel */
      hb_retni( gdImagePalettePixel(im, x, y) );

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGEPALETTEPIXEL", 3,
            hb_paramError( 1 ), hb_paramError( 2 ), hb_paramError( 3 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGETRUECOLORPIXEL ) // int gdImageTrueColorPixel(gdImagePtr im, int x, int y)
{
   if ( hb_pcount() == 3 &&
        hb_parinfo( 1 ) & HB_IT_POINTER &&
        hb_parinfo( 2 ) & HB_IT_NUMERIC &&
        hb_parinfo( 3 ) & HB_IT_NUMERIC
      )
   {
      gdImagePtr im;
      int x,y;

      /* Retrieve image pointer */
      im = (gdImagePtr) hb_parptr( 1 );

      /* Retrieve coord value */
      x = hb_parni( 2 );
      y = hb_parni( 3 );

      /* Get Color of a pixel */
      hb_retni( gdImageTrueColorPixel(im, x, y) );

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGETRUECOLORPIXEL", 3,
            hb_paramError( 1 ), hb_paramError( 2 ), hb_paramError( 3 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGEGETTHICKNESS ) // void gdImageGetThickness(gdImagePtr im)
{
   if ( hb_pcount() == 1 &&
        hb_parinfo( 1 ) & HB_IT_POINTER
      )
   {
      gdImagePtr im;

      /* Retrieve image pointer */
      im = (gdImagePtr) hb_parptr( 1 );

      /* Return previous */
      hb_retni( im->thick );

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGEGETTHICKNESS", 1,
            hb_paramError( 1 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

/* ---------------------------------------------------------------------------*/
/* FONTS AND TEXT-HANDLING FUNCTIONS                                          */
/* ---------------------------------------------------------------------------*/

HB_FUNC( GDFONTGETSMALL ) // gdFontPtr gdFontGetSmall(void)
{
   gdFontPtr font;

   /* Get font pointer */
   font = gdFontGetSmall();

   /* Return font pointer */
   hb_retptr( font );
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDFONTGETLARGE ) // gdFontPtr gdFontGetLarge(void)
{
   gdFontPtr font;

   /* Get font pointer */
   font = gdFontGetLarge();

   /* Return font pointer */
   hb_retptr( font );
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDFONTGETMEDIUMBOLD ) // gdFontPtr gdFontGetMediumBold(void)
{
   gdFontPtr font;

   /* Get font pointer */
   font = gdFontGetMediumBold();

   /* Return font pointer */
   hb_retptr( font );
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDFONTGETGIANT ) // gdFontPtr gdFontGetGiant(void)
{
   gdFontPtr font;

   /* Get font pointer */
   font = gdFontGetGiant();

   /* Return font pointer */
   hb_retptr( font );
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDFONTGETTINY ) // gdFontPtr gdFontGetTiny(void)
{
   gdFontPtr font;

   /* Get font pointer */
   font = gdFontGetTiny();

   /* Return font pointer */
   hb_retptr( font );
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGESTRING ) // void gdImageChar(gdImagePtr im, gdFontPtr font, int x, int y, int c, int color)
                         // void gdImageString(gdImagePtr im, gdFontPtr font, int x, int y, unsigned char *s, int color)
{
   if ( hb_pcount() == 6 &&
        hb_parinfo( 1 ) & HB_IT_POINTER &&
        hb_parinfo( 2 ) & HB_IT_POINTER &&
        hb_parinfo( 3 ) & HB_IT_NUMERIC &&
        hb_parinfo( 4 ) & HB_IT_NUMERIC &&
        hb_parinfo( 5 ) & HB_IT_STRING  &&
        hb_parinfo( 6 ) & HB_IT_NUMERIC
      )
   {
      gdImagePtr im;
      gdFontPtr  font;
      int x, y, c, color;
      unsigned char *s;

      /* Retrieve image pointer */
      im = (gdImagePtr)hb_parptr( 1 );

      /* Retrieve font pointer */
      font = (gdFontPtr)hb_parptr( 2 );

      /* Retrieve coord value */
      x = hb_parni( 3 );
      y = hb_parni( 4 );

      /* Retrieve color value */
      color = hb_parni( 6 );

      /* Write char */
      if ( hb_parclen(5) == 1 )
      {
         /* Retrieve char value */
         c = hb_parni( 5 );
         gdImageChar(im, font, x, y, c, color);
      }
      else
      {
         /* Retrieve string value */
         s = ( unsigned char * )hb_parcx( 5 );
         gdImageString(im, font, x, y, s, color);
      }

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGESTRING", 6,
            hb_paramError( 1 ), hb_paramError( 2 ), hb_paramError( 3 ), hb_paramError( 4 ),
            hb_paramError( 5 ), hb_paramError( 6 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGESTRINGUP ) // void gdImageCharUp(gdImagePtr im, gdFontPtr font, int x, int y, int c, int color)
                           // void gdImageStringUp(gdImagePtr im, gdFontPtr font, int x, int y, unsigned char *s, int color)
{
   if ( hb_pcount() == 6 &&
        hb_parinfo( 1 ) & HB_IT_POINTER &&
        hb_parinfo( 2 ) & HB_IT_POINTER &&
        hb_parinfo( 3 ) & HB_IT_NUMERIC &&
        hb_parinfo( 4 ) & HB_IT_NUMERIC &&
        hb_parinfo( 5 ) & HB_IT_STRING  &&
        hb_parinfo( 6 ) & HB_IT_NUMERIC
      )
   {
      gdImagePtr im;
      gdFontPtr  font;
      int x, y, c, color;
      unsigned char *s;

      /* Retrieve image pointer */
      im = (gdImagePtr) hb_parptr( 1 );

      /* Retrieve font pointer */
      font = (gdFontPtr)hb_parptr( 2 );

      /* Retrieve coord value */
      x = hb_parni( 3 );
      y = hb_parni( 4 );

      /* Retrieve color value */
      color = hb_parni( 6 );

      /* Write char */
      if ( hb_parclen(5) == 1 )
      {
         /* Retrieve char value */
         c = hb_parni( 5 );
         gdImageCharUp(im, font, x, y, c, color);
      }
      else
      {
         /* Retrieve string value */
         s = ( unsigned char * )hb_parcx( 5 );
         gdImageStringUp(im, font, x, y, s, color);
      }

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGESTRINGUP", 6,
            hb_paramError( 1 ), hb_paramError( 2 ), hb_paramError( 3 ), hb_paramError( 4 ),
            hb_paramError( 5 ), hb_paramError( 6 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/
// char *gdImageStringFTEx(gdImagePtr im, int *brect, int fg, char *fontname, double ptsize, double angle, int x, int y, char *string, gdFTStringExtraPtr strex)
// implementation: cError := gdImageStringFTEx( im, aRect, int fg, cFontname, nPtSize, nAngle, x, y, cString, nLinespacing, nCharmap, nResolution )
HB_FUNC( GDIMAGESTRINGFTEX )
{
   //TraceLog( NULL, "Parameters: %i, Type 1 =%i=\n\r", hb_pcount(), hb_parinfo( 1 ) );

   if ( hb_pcount() >= 9 &&
        ( ISNIL(1) || hb_parinfo( 1 ) & ( HB_IT_POINTER ) ) &&
        hb_parinfo( 2 ) & HB_IT_ARRAY   &&
        hb_parinfo( 3 ) & HB_IT_NUMERIC &&
        hb_parinfo( 4 ) & HB_IT_STRING  &&
        hb_parinfo( 5 ) & HB_IT_NUMERIC &&
        hb_parinfo( 6 ) & HB_IT_NUMERIC &&
        hb_parinfo( 7 ) & HB_IT_NUMERIC &&
        hb_parinfo( 8 ) & HB_IT_NUMERIC &&
        hb_parinfo( 9 ) & HB_IT_STRING
      )
   {
      gdImagePtr im;
      gdFTStringExtra extra;
      int fg;
      char *fontname;
      double ptsize, angle;
      int x, y, i;
      char *string;
      PHB_ITEM pRect;
      int aRect[8];
      char *err;
      int flags;
      double linespacing;
      int charmap;
      int resolution;

      /* Retrieve image pointer */
      im = (gdImagePtr)( hb_parinfo( 1 ) & HB_IT_POINTER ? hb_parptr( 1 ) : NULL );
      //TraceLog( NULL, "Image pointer: %p\n\r", im );

      /* Retrieve rectangle array */
      pRect = hb_param( 2, HB_IT_ARRAY );
      for( i = 0; i < 8; i ++ )
      {
         aRect[ i ] = hb_arrayGetNI( pRect, i+1 );
      }

      /* Retrieve foreground color value */
      fg = hb_parni( 3 );
      //TraceLog( NULL, "Forecolor: %i\n\r", fg );

      /* Retrieve fontname value */
      fontname = (char*) hb_parc( 4 );
      //TraceLog( NULL, "Font: %s\n\r", fontname );

      /* Retrieve point size value */
      ptsize = hb_parni( 5 );
      //TraceLog( NULL, "PTSize: %lf\n\r", ptsize );

      /* Retrieve angle value in radians */
      angle = hb_parnd( 6 );
      //TraceLog( NULL, "Angle: %lf\n\r", angle );

      /* Retrieve pos value */
      x = hb_parni( 7 );
      y = hb_parni( 8 );
      //TraceLog( NULL, "Pos: %i, %i\n\r", x, y );

      /* Retrieve string value */
      string = (char*) hb_parcx( 9 );
      //TraceLog( NULL, "String: %s\n\r", string );

      /* EXTENDED FLAGS */
      flags       = 0;

      /* defaults */
      linespacing = 1.05;
      charmap     = gdFTEX_Unicode;
      resolution  = 96;

      /* Retrieve line spacing */
      if ( hb_parinfo( 10 ) & HB_IT_DOUBLE )
      {
         linespacing = hb_parnd( 10 );
         flags |= gdFTEX_LINESPACE;
      }

      /* Retrieve charmap */
      if ( hb_parinfo( 11 ) & HB_IT_NUMERIC )
      {
         charmap = hb_parni( 11 );
         flags |= gdFTEX_CHARMAP;
      }

      /* Retrieve resolution */
      if ( hb_parinfo( 12 ) & HB_IT_NUMERIC )
      {
         resolution = hb_parni( 12 );
         flags |= gdFTEX_RESOLUTION;
      }

      if ( !( flags == 0 ) )
      {
         extra.flags       = flags;
         extra.linespacing = ( flags & gdFTEX_LINESPACE  ? linespacing : 1.05 );
         extra.charmap     = ( flags & gdFTEX_CHARMAP    ? charmap : gdFTEX_Unicode );
         extra.hdpi        = ( flags & gdFTEX_RESOLUTION ? resolution : 96 );
         extra.vdpi        = ( flags & gdFTEX_RESOLUTION ? resolution : 96 );
      }

      /* Write string */
      err = gdImageStringFTEx(im, &aRect[0], fg, fontname, ptsize, angle, x, y, string, ( !( flags == 0 ) ? &extra : 0 ));
      if ( !( err ) )
      {
         /* Save in array the correct text rectangle dimensions */
         HB_ITEM pArray;
         pArray.type = HB_IT_NIL;
         hb_arrayNew( &pArray, 8 );
         for( i = 0; i < 8; i ++ )
         {
            hb_itemPutNI( hb_arrayGetItemPtr( &pArray, i+1 ), aRect[i] );
         }
         hb_itemCopy( pRect, &pArray );
      }
      hb_retc( err );

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGESTRINGFTEX", 12,
            hb_paramError( 1 ), hb_paramError( 2 ), hb_paramError( 3 ), hb_paramError( 4 ),
            hb_paramError( 5 ), hb_paramError( 6 ), hb_paramError( 7 ), hb_paramError( 8 ),
            hb_paramError( 9 ), hb_paramError( 10 ), hb_paramError( 11 ), hb_paramError( 12 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGESTRINGFTCIRCLE ) // char *gdImageStringFTCircle(gdImagePtr im, int cx, int cy, double radius, double textRadius, double fillPortion, char *font, double points, char *top, char *bottom, int fgcolor)
{
   //TraceLog( NULL, "Parameters: %i, Type 9 =%i=\n\r", hb_pcount(), hb_parinfo( 10 ) );

   if ( hb_pcount() == 11 &&
        hb_parinfo( 1 ) & HB_IT_POINTER &&
        hb_parinfo( 2 ) & HB_IT_NUMERIC &&
        hb_parinfo( 3 ) & HB_IT_NUMERIC &&
        hb_parinfo( 4 ) & HB_IT_NUMERIC &&
        hb_parinfo( 5 ) & HB_IT_NUMERIC &&
        hb_parinfo( 6 ) & HB_IT_NUMERIC &&
        hb_parinfo( 7 ) & HB_IT_STRING  &&
        hb_parinfo( 8 ) & HB_IT_NUMERIC &&
        ( ISNIL( 9 ) || ( hb_parinfo( 9 ) & HB_IT_STRING ) ) &&
        ( ISNIL( 10 ) || ( hb_parinfo( 10 ) & HB_IT_STRING ) ) &&
        hb_parinfo( 11 ) & HB_IT_NUMERIC
      )
   {
      gdImagePtr im;
      int cx, cy;
      double radius, textRadius, fillPortion, points;
      char *top;
      char *bottom;
      int fgcolor;
      char *font;
      char *err;

      /* Retrieve image pointer */
      im = (gdImagePtr) hb_parptr( 1 );

      /* Retrieve pos value */
      cx = hb_parni( 2 );
      cy = hb_parni( 3 );

      /* Retrieve radius value */
      radius = hb_parnd( 4 );

      /* Retrieve textRadius value */
      textRadius = hb_parnd( 5 );

      /* Retrieve textRadius value */
      fillPortion = hb_parnd( 6 );

      /* Retrieve fontname value */
      font = (char*) hb_parcx( 7 );

      /* Retrieve points value */
      points = hb_parnd( 8 );

      /* Retrieve top string value */
      top = (char*) hb_parcx( 9 );

      /* Retrieve top string value */
      bottom = (char*) hb_parcx( 10 );

      /* Retrieve foreground color value */
      fgcolor = hb_parni( 11 );

      /* Write string */
      err = gdImageStringFTCircle(im, cx, cy, radius, textRadius, fillPortion, font, points, top, bottom, fgcolor);
      hb_retc( err );

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGESTRINGFTCIRCLE", 11,
            hb_paramError( 1 ), hb_paramError( 2 ), hb_paramError( 3 ), hb_paramError( 4 ),
            hb_paramError( 5 ), hb_paramError( 6 ), hb_paramError( 7 ), hb_paramError( 8 ),
            hb_paramError( 9 ), hb_paramError( 10 ), hb_paramError( 11 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDFONTCACHESETUP ) // int gdFontCacheSetup (void)
{
   /* This function initializes the font cache for freetype text output functions */
   hb_retl( gdFontCacheSetup() );
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDFONTCACHESHUTDOWN ) // void gdFontCacheShutdown (void)
{
   /* This function initializes the font cache for freetype text output functions */
   gdFontCacheShutdown();
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDFONTGETWIDTH )
{
   if ( hb_pcount() == 1 &&
        hb_parinfo( 1 ) & HB_IT_POINTER
      )
   {
      gdFontPtr font;

      /* Retrieve font pointer */
      font = (gdFontPtr)hb_parptr( 1 );

      /* Return value */
      hb_retni( font->w );

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDFONTGETWIDTH", 1,
            hb_paramError( 1 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDFONTGETHEIGHT )
{
   if ( hb_pcount() == 1 &&
        hb_parinfo( 1 ) & HB_IT_POINTER
      )
   {
      gdFontPtr font;

      /* Retrieve font pointer */
      font = (gdFontPtr)hb_parptr( 1 );

      /* Return value */
      hb_retni( font->h );

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDFONTGETHEIGHT", 1,
            hb_paramError( 1 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

/* ---------------------------------------------------------------------------*/
/* COLOR HANDLING FUNCTIONS                                                   */
/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGECOLORALLOCATE ) // int gdImageColorAllocate(gdImagePtr im, int r, int g, int b)
{
   if ( hb_pcount() == 4 &&
        hb_parinfo( 1 ) & HB_IT_POINTER &&
        hb_parinfo( 2 ) & HB_IT_NUMERIC &&
        hb_parinfo( 3 ) & HB_IT_NUMERIC &&
        hb_parinfo( 4 ) & HB_IT_NUMERIC
      )
   {
      gdImagePtr im;
      int r, g, b;
      int color;

      /* Retrieve image pointer */
      im = (gdImagePtr) hb_parptr( 1 );

      /* Retrieve RGB values */
      r = hb_parni( 2 );
      g = hb_parni( 3 );
      b = hb_parni( 4 );

      /* Allocate color */
      color = gdImageColorAllocate(im, r, g, b);

      /* return color */
      hb_retni( color );

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGECOLORALLOCATE", 4,
            hb_paramError( 1 ), hb_paramError( 2 ), hb_paramError( 3 ), hb_paramError( 4 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGECOLORDEALLOCATE ) // void gdImageColorDeallocate(gdImagePtr im, int color)
{
   if ( hb_pcount() == 2 &&
        hb_parinfo( 1 ) & HB_IT_POINTER &&
        hb_parinfo( 2 ) & HB_IT_NUMERIC
      )
   {
      gdImagePtr im;
      int color;

      /* Retrieve image pointer */
      im = (gdImagePtr) hb_parptr( 1 );

      /* Retrieve saveFlag value */
      color = hb_parni( 2 );

      /* Deallocate color */
      gdImageColorDeallocate(im, color);

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGECOLORDEALLOCATE", 2,
            hb_paramError( 1 ), hb_paramError( 2 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGECOLORALLOCATEALPHA ) // int gdImageColorAllocateAlpha(gdImagePtr im, int r, int g, int b, int a)
{
   if ( hb_pcount() == 5 &&
        hb_parinfo( 1 ) & HB_IT_POINTER &&
        hb_parinfo( 2 ) & HB_IT_NUMERIC &&
        hb_parinfo( 3 ) & HB_IT_NUMERIC &&
        hb_parinfo( 4 ) & HB_IT_NUMERIC &&
        hb_parinfo( 5 ) & HB_IT_NUMERIC
      )
   {
      gdImagePtr im;
      int r, g, b;
      int color;
      int a;

      /* Retrieve image pointer */
      im = (gdImagePtr) hb_parptr( 1 );

      /* Retrieve RGB values */
      r = hb_parni( 2 );
      g = hb_parni( 3 );
      b = hb_parni( 4 );

      /* Retrieve Alpha value */
      a = hb_parni( 5 );

      /* Allocate color */
      color = gdImageColorAllocateAlpha(im, r, g, b, a);

      /* return color */
      hb_retni( color );

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGECOLORALLOCATEALPHA", 5,
            hb_paramError( 1 ), hb_paramError( 2 ), hb_paramError( 3 ), hb_paramError( 4 ),
            hb_paramError( 5 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGECOLORCLOSEST ) // int gdImageColorClosest(gdImagePtr im, int r, int g, int b)
{
   if ( hb_pcount() == 4 &&
        hb_parinfo( 1 ) & HB_IT_POINTER &&
        hb_parinfo( 2 ) & HB_IT_NUMERIC &&
        hb_parinfo( 3 ) & HB_IT_NUMERIC &&
        hb_parinfo( 4 ) & HB_IT_NUMERIC
      )
   {
      gdImagePtr im;
      int r, g, b;
      int color;

      /* Retrieve image pointer */
      im = (gdImagePtr) hb_parptr( 1 );

      /* Retrieve RGB values */
      r = hb_parni( 2 );
      g = hb_parni( 3 );
      b = hb_parni( 4 );

      /* Search color closest */
      color = gdImageColorClosest(im, r, g, b);

      /* return color */
      hb_retni( color );

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGECOLORCLOSEST", 4,
            hb_paramError( 1 ), hb_paramError( 2 ), hb_paramError( 3 ), hb_paramError( 4 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGECOLORCLOSESTALPHA ) // int gdImageColorClosestAlpha(gdImagePtr im, int r, int g, int b, int a)
{
   if ( hb_pcount() == 5 &&
        hb_parinfo( 1 ) & HB_IT_POINTER &&
        hb_parinfo( 2 ) & HB_IT_NUMERIC &&
        hb_parinfo( 3 ) & HB_IT_NUMERIC &&
        hb_parinfo( 4 ) & HB_IT_NUMERIC &&
        hb_parinfo( 5 ) & HB_IT_NUMERIC
      )
   {
      gdImagePtr im;
      int r, g, b;
      int color;
      int a;

      /* Retrieve image pointer */
      im = (gdImagePtr) hb_parptr( 1 );

      /* Retrieve RGB values */
      r = hb_parni( 2 );
      g = hb_parni( 3 );
      b = hb_parni( 4 );

      /* Retrieve Alpha value */
      a = hb_parni( 5 );

      /* Allocate color */
      color = gdImageColorClosestAlpha(im, r, g, b, a);

      /* return color */
      hb_retni( color );

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGECOLORCLOSESTALPHA", 5,
            hb_paramError( 1 ), hb_paramError( 2 ), hb_paramError( 3 ), hb_paramError( 4 ),
            hb_paramError( 5 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGECOLORCLOSESTHWB ) //  gdImageColorClosestHWB(gdImagePtr im, int r, int g, int b)
{
   if ( hb_pcount() == 4 &&
        hb_parinfo( 1 ) & HB_IT_POINTER &&
        hb_parinfo( 2 ) & HB_IT_NUMERIC &&
        hb_parinfo( 3 ) & HB_IT_NUMERIC &&
        hb_parinfo( 4 ) & HB_IT_NUMERIC
      )
   {
      gdImagePtr im;
      int r, g, b;
      int color;

      /* Retrieve image pointer */
      im = (gdImagePtr) hb_parptr( 1 );

      /* Retrieve RGB values */
      r = hb_parni( 2 );
      g = hb_parni( 3 );
      b = hb_parni( 4 );

      /* Search color closest */
      color = gdImageColorClosestHWB(im, r, g, b);

      /* return color */
      hb_retni( color );

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGECOLORCLOSESTHWB", 4,
            hb_paramError( 1 ), hb_paramError( 2 ), hb_paramError( 3 ), hb_paramError( 4 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGECOLOREXACT ) // int gdImageColorExact(gdImagePtr im, int r, int g, int b)
{
   if ( hb_pcount() == 4 &&
        hb_parinfo( 1 ) & HB_IT_POINTER &&
        hb_parinfo( 2 ) & HB_IT_NUMERIC &&
        hb_parinfo( 3 ) & HB_IT_NUMERIC &&
        hb_parinfo( 4 ) & HB_IT_NUMERIC
      )
   {
      gdImagePtr im;
      int r, g, b;
      int color;

      /* Retrieve image pointer */
      im = (gdImagePtr) hb_parptr( 1 );

      /* Retrieve RGB values */
      r = hb_parni( 2 );
      g = hb_parni( 3 );
      b = hb_parni( 4 );

      /* Search if there is the color */
      color = gdImageColorExact(im, r, g, b);

      /* return color */
      hb_retni( color );

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGECOLOREXACT", 4,
            hb_paramError( 1 ), hb_paramError( 2 ), hb_paramError( 3 ), hb_paramError( 4 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGECOLORRESOLVE ) // int gdImageColorResolve(gdImagePtr im, int r, int g, int b)
{
   if ( hb_pcount() == 4 &&
        hb_parinfo( 1 ) & HB_IT_POINTER &&
        hb_parinfo( 2 ) & HB_IT_NUMERIC &&
        hb_parinfo( 3 ) & HB_IT_NUMERIC &&
        hb_parinfo( 4 ) & HB_IT_NUMERIC
      )
   {
      gdImagePtr im;
      int r, g, b;
      int color;

      /* Retrieve image pointer */
      im = (gdImagePtr) hb_parptr( 1 );

      /* Retrieve RGB values */
      r = hb_parni( 2 );
      g = hb_parni( 3 );
      b = hb_parni( 4 );

      /* Search if there is the color or similar, if not it creates */
      color = gdImageColorResolve(im, r, g, b);

      /* return color */
      hb_retni( color );

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGECOLORRESOLVE", 4,
            hb_paramError( 1 ), hb_paramError( 2 ), hb_paramError( 3 ), hb_paramError( 4 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGECOLORRESOLVEALPHA ) // int gdImageColorResolveAlpha(gdImagePtr im, int r, int g, int b, int a)
{
   if ( hb_pcount() == 5 &&
        hb_parinfo( 1 ) & HB_IT_POINTER &&
        hb_parinfo( 2 ) & HB_IT_NUMERIC &&
        hb_parinfo( 3 ) & HB_IT_NUMERIC &&
        hb_parinfo( 4 ) & HB_IT_NUMERIC &&
        hb_parinfo( 5 ) & HB_IT_NUMERIC
      )
   {
      gdImagePtr im;
      int r, g, b;
      int color;
      int a;

      /* Retrieve image pointer */
      im = (gdImagePtr) hb_parptr( 1 );

      /* Retrieve RGB values */
      r = hb_parni( 2 );
      g = hb_parni( 3 );
      b = hb_parni( 4 );

      /* Retrieve Alpha value */
      a = hb_parni( 5 );

      /* Allocate color */
      color = gdImageColorResolveAlpha(im, r, g, b, a);

      /* return color */
      hb_retni( color );

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGECOLORRESOLVEALPHA", 5,
            hb_paramError( 1 ), hb_paramError( 2 ), hb_paramError( 3 ), hb_paramError( 4 ),
            hb_paramError( 5 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGECOLORTRANSPARENT ) // void gdImageColorTransparent(gdImagePtr im, int color)
{
   if ( hb_pcount() == 2 &&
        hb_parinfo( 1 ) & HB_IT_POINTER &&
        hb_parinfo( 2 ) & HB_IT_NUMERIC
      )
   {
      gdImagePtr im;
      int color;

      /* Retrieve image pointer */
      im = (gdImagePtr) hb_parptr( 1 );

      /* Retrieve saveFlag value */
      color = hb_parni( 2 );

      /* Set transparent color (to define no transparent color set -1) */
      gdImageColorTransparent(im, color);

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGECOLORTRANSPARENT", 2,
            hb_paramError( 1 ), hb_paramError( 2 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDTRUECOLOR ) // int gdTrueColor(int red, int green, int blue)
{
   if ( hb_pcount() == 3 &&
        hb_parinfo( 1 ) & HB_IT_NUMERIC &&
        hb_parinfo( 2 ) & HB_IT_NUMERIC &&
        hb_parinfo( 3 ) & HB_IT_NUMERIC
      )
   {
      int r, g, b;
      int color;

      /* Retrieve RGB values */
      r = hb_parni( 1 );
      g = hb_parni( 2 );
      b = hb_parni( 3 );

      /* Allocate color */
      color = gdTrueColor(r, g, b);

      /* return color */
      hb_retni( color );

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDTRUECOLOR", 3,
            hb_paramError( 1 ), hb_paramError( 2 ), hb_paramError( 3 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDTRUECOLORALPHA ) // int gdTrueColorAlpha(int red, int green, int blue, int alpha)
{
   if ( hb_pcount() == 4 &&
        hb_parinfo( 1 ) & HB_IT_NUMERIC &&
        hb_parinfo( 2 ) & HB_IT_NUMERIC &&
        hb_parinfo( 3 ) & HB_IT_NUMERIC &&
        hb_parinfo( 4 ) & HB_IT_NUMERIC
      )
   {
      int r, g, b, a;
      int color;

      /* Retrieve RGB values */
      r = hb_parni( 1 );
      g = hb_parni( 2 );
      b = hb_parni( 3 );
      a = hb_parni( 4 );

      /* Allocate color */
      color = gdTrueColorAlpha(r, g, b, a);

      /* return color */
      hb_retni( color );

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDTRUECOLORALPHA", 4,
            hb_paramError( 1 ), hb_paramError( 2 ), hb_paramError( 3 ), hb_paramError( 4 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

/* ---------------------------------------------------------------------------*/
/* COPY AND RESIZING FUNCTIONS                                                */
/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGECOPY ) // void gdImageCopy(gdImagePtr dst, gdImagePtr src, int dstX, int dstY, int srcX, int srcY, int w, int h)
{
   if ( hb_pcount() == 8 &&
        hb_parinfo( 1 ) & HB_IT_POINTER &&
        hb_parinfo( 2 ) & HB_IT_POINTER &&
        hb_parinfo( 3 ) & HB_IT_NUMERIC &&
        hb_parinfo( 4 ) & HB_IT_NUMERIC &&
        hb_parinfo( 5 ) & HB_IT_NUMERIC &&
        hb_parinfo( 6 ) & HB_IT_NUMERIC &&
        hb_parinfo( 7 ) & HB_IT_NUMERIC &&
        hb_parinfo( 8 ) & HB_IT_NUMERIC
      )
   {
      gdImagePtr dst, src;
      int dstX, dstY, srcX, srcY, w, h;

      /* Retrieve destination image pointer */
      dst = (gdImagePtr)hb_parptr( 1 );

      /* Retrieve source image pointer */
      src = (gdImagePtr)hb_parptr( 2 );

      /* Retrieve destination pos value */
      dstX = hb_parni( 3 );
      dstY = hb_parni( 4 );

      /* Retrieve source pos value */
      srcX = hb_parni( 5 );
      srcY = hb_parni( 6 );

      /* Retrieve width value */
      w = hb_parni( 7 );

      /* Retrieve height value */
      h = hb_parni( 8 );

      /* Perform copy */
      gdImageCopy(dst, src, dstX, dstY, srcX, srcY, w, h);
   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGECOPY", 8,
            hb_paramError( 1 ), hb_paramError( 2 ), hb_paramError( 3 ), hb_paramError( 4 ),
            hb_paramError( 5 ), hb_paramError( 6 ), hb_paramError( 7 ), hb_paramError( 8 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGECOPYRESIZED ) // void gdImageCopyResized(gdImagePtr dst, gdImagePtr src, int dstX, int dstY, int srcX, int srcY, int dstW, int dstH, int srcW, int srcH)
{
   if ( hb_pcount() == 10 &&
        hb_parinfo( 1 ) & HB_IT_POINTER &&
        hb_parinfo( 2 ) & HB_IT_POINTER &&
        hb_parinfo( 3 ) & HB_IT_NUMERIC &&
        hb_parinfo( 4 ) & HB_IT_NUMERIC &&
        hb_parinfo( 5 ) & HB_IT_NUMERIC &&
        hb_parinfo( 6 ) & HB_IT_NUMERIC &&
        hb_parinfo( 7 ) & HB_IT_NUMERIC &&
        hb_parinfo( 8 ) & HB_IT_NUMERIC &&
        hb_parinfo( 9 ) & HB_IT_NUMERIC &&
        hb_parinfo( 10 ) & HB_IT_NUMERIC
      )
   {
      gdImagePtr dst, src;
      int dstX, dstY, srcX, srcY;
      int dstW, dstH, srcW, srcH;

      /* Retrieve destination image pointer */
      dst = (gdImagePtr)hb_parptr( 1 );

      /* Retrieve source image pointer */
      src = (gdImagePtr)hb_parptr( 2 );

      /* Retrieve destination pos value */
      dstX = hb_parni( 3 );
      dstY = hb_parni( 4 );

      /* Retrieve source pos value */
      srcX = hb_parni( 5 );
      srcY = hb_parni( 6 );

      /* Retrieve dest width value */
      dstW = hb_parni( 7 );

      /* Retrieve dest height value */
      dstH = hb_parni( 8 );

      /* Retrieve source width value */
      srcW = hb_parni( 9 );

      /* Retrieve source height value */
      srcH = hb_parni( 10 );

      /* Perform copy */
      gdImageCopyResized(dst, src, dstX, dstY, srcX, srcY, dstW, dstH, srcW, srcH);
   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGECOPYRESIZED", 10,
            hb_paramError( 1 ), hb_paramError( 2 ), hb_paramError( 3 ), hb_paramError( 4 ),
            hb_paramError( 5 ), hb_paramError( 6 ), hb_paramError( 7 ), hb_paramError( 8 ),
            hb_paramError( 9 ), hb_paramError( 10 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGECOPYRESAMPLED ) // void gdImageCopyResampled(gdImagePtr dst, gdImagePtr src, int dstX, int dstY, int srcX, int srcY, int dstW, int dstH, int srcW, int srcH)
{
   if ( hb_pcount() == 10 &&
        hb_parinfo( 1 ) & HB_IT_POINTER &&
        hb_parinfo( 2 ) & HB_IT_POINTER &&
        hb_parinfo( 3 ) & HB_IT_NUMERIC &&
        hb_parinfo( 4 ) & HB_IT_NUMERIC &&
        hb_parinfo( 5 ) & HB_IT_NUMERIC &&
        hb_parinfo( 6 ) & HB_IT_NUMERIC &&
        hb_parinfo( 7 ) & HB_IT_NUMERIC &&
        hb_parinfo( 8 ) & HB_IT_NUMERIC &&
        hb_parinfo( 9 ) & HB_IT_NUMERIC &&
        hb_parinfo( 10 ) & HB_IT_NUMERIC
      )
   {
      gdImagePtr dst, src;
      int dstX, dstY, srcX, srcY;
      int dstW, dstH, srcW, srcH;

      /* Retrieve destination image pointer */
      dst = (gdImagePtr)hb_parptr( 1 );

      /* Retrieve source image pointer */
      src = (gdImagePtr)hb_parptr( 2 );

      /* Retrieve destination pos value */
      dstX = hb_parni( 3 );
      dstY = hb_parni( 4 );

      /* Retrieve source pos value */
      srcX = hb_parni( 5 );
      srcY = hb_parni( 6 );

      /* Retrieve dest width value */
      dstW = hb_parni( 7 );

      /* Retrieve dest height value */
      dstH = hb_parni( 8 );

      /* Retrieve source width value */
      srcW = hb_parni( 9 );

      /* Retrieve source height value */
      srcH = hb_parni( 10 );

      /* Perform copy */
      gdImageCopyResampled(dst, src, dstX, dstY, srcX, srcY, dstW, dstH, srcW, srcH);
   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGECOPYRESAMPLED", 10,
            hb_paramError( 1 ), hb_paramError( 2 ), hb_paramError( 3 ), hb_paramError( 4 ),
            hb_paramError( 5 ), hb_paramError( 6 ), hb_paramError( 7 ), hb_paramError( 8 ),
            hb_paramError( 9 ), hb_paramError( 10 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGECOPYROTATED ) // void gdImageCopyRotated(gdImagePtr dst, gdImagePtr src, double dstX, double dstY, int srcX, int srcY, int srcW, int srcH, int angle)
{
   if ( hb_pcount() == 9 &&
        hb_parinfo( 1 ) & HB_IT_POINTER &&
        hb_parinfo( 2 ) & HB_IT_POINTER &&
        hb_parinfo( 3 ) & HB_IT_NUMERIC &&
        hb_parinfo( 4 ) & HB_IT_NUMERIC &&
        hb_parinfo( 5 ) & HB_IT_NUMERIC &&
        hb_parinfo( 6 ) & HB_IT_NUMERIC &&
        hb_parinfo( 7 ) & HB_IT_NUMERIC &&
        hb_parinfo( 8 ) & HB_IT_NUMERIC &&
        hb_parinfo( 9 ) & HB_IT_NUMERIC
      )
   {
      gdImagePtr dst, src;
      double dstX, dstY;
      int srcX, srcY, srcW, srcH, angle;

      /* Retrieve destination image pointer */
      dst = (gdImagePtr)hb_parptr( 1 );

      /* Retrieve source image pointer */
      src = (gdImagePtr)hb_parptr( 2 );

      /* Retrieve destination pos value */
      dstX = hb_parnd( 3 );
      dstY = hb_parnd( 4 );

      /* Retrieve source pos value */
      srcX = hb_parni( 5 );
      srcY = hb_parni( 6 );

      /* Retrieve source width value */
      srcW = hb_parni( 7 );

      /* Retrieve source height value */
      srcH = hb_parni( 8 );

      /* Retrieve angle value */
      angle = hb_parni( 9 );

      /* Perform rotation */
      gdImageCopyRotated(dst, src, dstX, dstY, srcX, srcY, srcW, srcH, angle);
   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGECOPYROTATED", 9,
            hb_paramError( 1 ), hb_paramError( 2 ), hb_paramError( 3 ), hb_paramError( 4 ),
            hb_paramError( 5 ), hb_paramError( 6 ), hb_paramError( 7 ), hb_paramError( 8 ),
            hb_paramError( 9 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGECOPYMERGE ) // void gdImageCopyMerge(gdImagePtr dst, gdImagePtr src, int dstX, int dstY, int srcX, int srcY, int w, int h, int pct)
{
   if ( hb_pcount() == 9 &&
        hb_parinfo( 1 ) & HB_IT_POINTER &&
        hb_parinfo( 2 ) & HB_IT_POINTER &&
        hb_parinfo( 3 ) & HB_IT_NUMERIC &&
        hb_parinfo( 4 ) & HB_IT_NUMERIC &&
        hb_parinfo( 5 ) & HB_IT_NUMERIC &&
        hb_parinfo( 6 ) & HB_IT_NUMERIC &&
        hb_parinfo( 7 ) & HB_IT_NUMERIC &&
        hb_parinfo( 8 ) & HB_IT_NUMERIC &&
        hb_parinfo( 9 ) & HB_IT_NUMERIC
      )
   {
      gdImagePtr dst, src;
      int dstX, dstY, srcX, srcY, w, h, pct;

      /* Retrieve destination image pointer */
      dst =(gdImagePtr)hb_parptr( 1 );

      /* Retrieve source image pointer */
      src = (gdImagePtr)hb_parptr( 2 );

      /* Retrieve destination pos value */
      dstX = hb_parni( 3 );
      dstY = hb_parni( 4 );

      /* Retrieve source pos value */
      srcX = hb_parni( 5 );
      srcY = hb_parni( 6 );

      /* Retrieve width value */
      w = hb_parni( 7 );

      /* Retrieve height value */
      h = hb_parni( 8 );

      /* Retrieve percentual value */
      pct = hb_parni( 9 );

      /* Perform copy */
      gdImageCopyMerge(dst, src, dstX, dstY, srcX, srcY, w, h, pct);
   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGECOPYMERGE", 9,
            hb_paramError( 1 ), hb_paramError( 2 ), hb_paramError( 3 ), hb_paramError( 4 ),
            hb_paramError( 5 ), hb_paramError( 6 ), hb_paramError( 7 ), hb_paramError( 8 ),
            hb_paramError( 9 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGECOPYMERGEGRAY ) // void gdImageCopyMergeGray(gdImagePtr dst, gdImagePtr src, int dstX, int dstY, int srcX, int srcY, int w, int h, int pct)
{
   if ( hb_pcount() == 9 &&
        hb_parinfo( 1 ) & HB_IT_POINTER &&
        hb_parinfo( 2 ) & HB_IT_POINTER &&
        hb_parinfo( 3 ) & HB_IT_NUMERIC &&
        hb_parinfo( 4 ) & HB_IT_NUMERIC &&
        hb_parinfo( 5 ) & HB_IT_NUMERIC &&
        hb_parinfo( 6 ) & HB_IT_NUMERIC &&
        hb_parinfo( 7 ) & HB_IT_NUMERIC &&
        hb_parinfo( 8 ) & HB_IT_NUMERIC &&
        hb_parinfo( 9 ) & HB_IT_NUMERIC
      )
   {
      gdImagePtr dst, src;
      int dstX, dstY, srcX, srcY, w, h, pct;

      /* Retrieve destination image pointer */
      dst = (gdImagePtr)hb_parptr( 1 );

      /* Retrieve source image pointer */
      src = (gdImagePtr)hb_parptr( 2 );

      /* Retrieve destination pos value */
      dstX = hb_parni( 3 );
      dstY = hb_parni( 4 );

      /* Retrieve source pos value */
      srcX = hb_parni( 5 );
      srcY = hb_parni( 6 );

      /* Retrieve width value */
      w = hb_parni( 7 );

      /* Retrieve height value */
      h = hb_parni( 8 );

      /* Retrieve percentual value */
      pct = hb_parni( 9 );

      /* Perform copy */
      gdImageCopyMergeGray(dst, src, dstX, dstY, srcX, srcY, w, h, pct);
   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGECOPYMERGEGRAY", 9,
            hb_paramError( 1 ), hb_paramError( 2 ), hb_paramError( 3 ), hb_paramError( 4 ),
            hb_paramError( 5 ), hb_paramError( 6 ), hb_paramError( 7 ), hb_paramError( 8 ),
            hb_paramError( 9 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGEPALETTECOPY ) // void gdImagePaletteCopy(gdImagePtr dst, gdImagePtr src)
{
   if ( hb_pcount() == 2 &&
        hb_parinfo( 1 ) & HB_IT_POINTER &&
        hb_parinfo( 2 ) & HB_IT_POINTER
      )
   {
      gdImagePtr dst, src;

      /* Retrieve destination image pointer */
      dst = (gdImagePtr)hb_parptr( 1 );

      /* Retrieve source image pointer */
      src = (gdImagePtr)hb_parptr( 2 );

      /* Perform copy */
      gdImagePaletteCopy(dst, src);
   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGECOPYMERGEGRAY", 2,
            hb_paramError( 1 ), hb_paramError( 2 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGESQUARETOCIRCLE ) // void gdImageSquareToCircle(gdImagePtr im, int radius)
{
   if ( hb_pcount() == 2 &&
        hb_parinfo( 1 ) & HB_IT_POINTER &&
        hb_parinfo( 2 ) & HB_IT_NUMERIC
      )
   {
      gdImagePtr im;
      int radius;

      /* Retrieve image pointer */
      im = (gdImagePtr) hb_parptr( 1 );

      /* Retrieve radius value */
      radius = hb_parni( 2 );

      /* TODO */
      gdImageSquareToCircle(im, radius);

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGESQUARETOCIRCLE", 2,
            hb_paramError( 1 ), hb_paramError( 2 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGESHARPEN ) // void gdImageSharpen(gdImagePtr im, int pct)
{
   if ( hb_pcount() == 2 &&
        hb_parinfo( 1 ) & HB_IT_POINTER &&
        hb_parinfo( 2 ) & HB_IT_NUMERIC
      )
   {
      gdImagePtr im;
      int pct;

      /* Retrieve image pointer */
      im = (gdImagePtr) hb_parptr( 1 );

      /* Retrieve percentual value */
      pct = hb_parni( 2 );

      /* Sharpens the specified image */
      gdImageSharpen(im, pct);

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGESHARPEN", 2,
            hb_paramError( 1 ), hb_paramError( 2 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

/* ---------------------------------------------------------------------------*/
/* MISCELLANEOUS FUNCTIONS                                                    */
/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGECOMPARE ) // int gdImageCompare(gdImagePtr im1, gdImagePtr im2)
{
   if ( hb_pcount() == 2 &&
        hb_parinfo( 1 ) & HB_IT_POINTER &&
        hb_parinfo( 2 ) & HB_IT_POINTER
      )
   {
      gdImagePtr dst, src;

      /* Retrieve destination image pointer */
      dst = (gdImagePtr)hb_parptr( 1 );

      /* Retrieve source image pointer */
      src = (gdImagePtr)hb_parptr( 2 );

      /* Compare images - if return <> 0 check value for infos */
      hb_retni( gdImageCompare(dst, src) );
   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGECOMPARE", 2,
            hb_paramError( 1 ), hb_paramError( 2 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

HB_FUNC( GDIMAGEINTERLACE ) // void gdImageInterlace(gdImagePtr im, int interlace)
{
   if ( hb_pcount() == 2 &&
        hb_parinfo( 1 ) & HB_IT_POINTER &&
        hb_parinfo( 2 ) & HB_IT_LOGICAL
      )
   {
      gdImagePtr im;
      int interlace;

      /* Retrieve image pointer */
      im = (gdImagePtr) hb_parptr( 1 );

      /* Retrieve interlace value */
      interlace = hb_parl( 2 );

      /* Set interlace */
      gdImageInterlace(im, interlace);

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGEINTERLACE", 2,
            hb_paramError( 1 ), hb_paramError( 2 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

#if ( GD_VERS >= 2033 )
//BGD_DECLARE(void *) gdImageGifAnimBeginPtr(gdImagePtr im, int *size, int GlobalCM, int Loops);
// implementation: (void *) gdImageGifAnimBegin( gdImagePtr im, cFile | nHandle, int GlobalCM, int Loops);
HB_FUNC( GDIMAGEGIFANIMBEGIN )
{
   if ( hb_pcount() == 4 &&
        hb_parinfo( 1 ) & HB_IT_POINTER &&
        ( hb_parinfo( 2 ) & HB_IT_STRING || hb_parinfo( 2 ) & HB_IT_NUMERIC || ISNIL( 2 ) ) &&
        hb_parinfo( 3 ) & HB_IT_NUMERIC &&
        hb_parinfo( 4 ) & HB_IT_NUMERIC
      )
   {
      gdImagePtr im;
      void *iptr;
      int size;
      int GlobalCM, Loops;

      /* Retrieve image pointer */
      im = (gdImagePtr) hb_parptr( 1 );

      /* Retrieve global color map value */
      GlobalCM = hb_parni( 3 );

      /* Retrieve Loops value */
      Loops = hb_parni( 4 );

      /* run function */
      iptr = gdImageGifAnimBeginPtr(im, &size, GlobalCM, Loops);

      /* Check if 2nd parameter is a file name or an handle */
      if ( hb_parinfo( 2 ) & HB_IT_STRING )
      {
         const char *szFile;
         szFile = hb_parcx( 2 );

         SaveImageToFile( szFile, iptr, size );
      }
      else if ( hb_parinfo( 2 ) & HB_IT_NUMERIC || ISNIL( 2 ) )
      {
         FHANDLE fhandle;

         /* Retrieve file handle */
         fhandle = ( hb_parinfo( 2 ) & HB_IT_NUMERIC ) ? hb_parnl( 2 ) : 1; // 1 = std output

         SaveImageToHandle( fhandle, iptr, size );

      }

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGEGIFANIMBEGIN", 3,
            hb_paramError( 1 ), hb_paramError( 2 ), hb_paramError( 3 ) );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

//BGD_DECLARE(void *) gdImageGifAnimAddPtr(gdImagePtr im, int *size, int LocalCM, int LeftOfs, int TopOfs, int Delay, int Disposal, gdImagePtr previm);
// implementation: (void *) gdImageGifAnimAdd( gdImagePtr im, cFile | nHandle, int LocalCM, int LeftOfs, int TopOfs, int Delay, int Disposal, gdImagePtr previm);
HB_FUNC( GDIMAGEGIFANIMADD )
{
   if ( hb_pcount() == 8 &&
        hb_parinfo( 1 ) & HB_IT_POINTER &&
        ( hb_parinfo( 2 ) & HB_IT_STRING || hb_parinfo( 2 ) & HB_IT_NUMERIC || ISNIL( 2 ) ) &&
        hb_parinfo( 3 ) & HB_IT_NUMERIC &&
        hb_parinfo( 4 ) & HB_IT_NUMERIC &&
        hb_parinfo( 5 ) & HB_IT_NUMERIC &&
        hb_parinfo( 6 ) & HB_IT_NUMERIC &&
        hb_parinfo( 7 ) & HB_IT_NUMERIC &&
        ( hb_parinfo( 8 ) & HB_IT_POINTER || ISNIL( 8 ) )
      )
   {
      gdImagePtr im, previm;
      void *iptr;
      int size;
      int LocalCM, LeftOfs, TopOfs, Delay, Disposal;

      /* Retrieve parameters */
      im       = (gdImagePtr)hb_parptr( 1 );

      LocalCM  = hb_parni( 3 );
      LeftOfs  = hb_parni( 4 );
      TopOfs   = hb_parni( 5 );
      Delay    = hb_parni( 6 );
      Disposal = hb_parni( 7 );
      previm   = (gdImagePtr)hb_parptr( 8 );

      /* Run function and return value */
      iptr = gdImageGifAnimAddPtr(im, &size, LocalCM, LeftOfs, TopOfs, Delay, Disposal, previm);

      /* Check if 2nd parameter is a file name or an handle */
      if ( hb_parinfo( 2 ) & HB_IT_STRING )
      {
         const char *szFile;
         szFile = hb_parcx( 2 );

         AddImageToFile( szFile, iptr, size );
      }
      else if ( hb_parinfo( 2 ) & HB_IT_NUMERIC || ISNIL( 2 ) )
      {
         FHANDLE fhandle;

         /* Retrieve file handle */
         fhandle = ( hb_parinfo( 2 ) & HB_IT_NUMERIC ) ? hb_parnl( 2 ) : 1; // 1 = std output

         SaveImageToHandle( fhandle, iptr, size );

      }

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGEGIFANIMADD", 8,
            hb_paramError( 1 ), hb_paramError( 2 ), hb_paramError( 3 ), hb_paramError( 4 ),
            hb_paramError( 5 ), hb_paramError( 6 ), hb_paramError( 7 ), hb_paramError( 8 )
         );
         return;
      }
   }
}

/* ---------------------------------------------------------------------------*/

//BGD_DECLARE(void *) gdImageGifAnimEndPtr(int *size);
// implementation: gdImageGifAnimEnd( cFile | nHandle );
HB_FUNC( GDIMAGEGIFANIMEND )
{
   if ( hb_pcount() == 1 &&
        ( hb_parinfo( 1 ) & HB_IT_STRING || hb_parinfo( 1 ) & HB_IT_NUMERIC || ISNIL( 1 ) )
      )
   {
      void *iptr;
      int size;

      /* Run function and return value */
      iptr = gdImageGifAnimEndPtr(&size);

      /* Check if 1st parameter is a file name or an handle */
      if ( hb_parinfo( 1 ) & HB_IT_STRING )
      {
         const char *szFile;
         szFile = hb_parcx( 1 );

         AddImageToFile( szFile, iptr, size );
      }
      else if ( hb_parinfo( 2 ) & HB_IT_NUMERIC || ISNIL( 2 ) )
      {
         FHANDLE fhandle;

         /* Retrieve file handle */
         fhandle = ( hb_parinfo( 1 ) & HB_IT_NUMERIC ) ? hb_parnl( 1 ) : 1; // 1 = std output

         SaveImageToHandle( fhandle, iptr, size );

      }

   }
   else
   {
      // Parameter error
      {
         hb_errRT_BASE_SubstR( EG_ARG, 0, NULL,
            "GDIMAGEGIFANIMEND", 1,
            hb_paramError( 1 )
         );
         return;
      }
   }
}


/* ---------------------------------------------------------------------------*/
#endif // ( GD_VERS >= 2033 )

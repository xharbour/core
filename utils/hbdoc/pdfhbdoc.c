/*
 * $Id: pdfhbdoc.c 9747 2012-10-13 00:13:02Z andijahja $
 */

/*
 * Harbour Project source code:
 * PDF low level Api for HBDOC
 *
 * Copyright 2000-2004 Luiz Rafael Culik <culikr /at/ brturbo.com>
 * www - http://www.harbour-project.org
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

/* Pacify MSVS2005 and above */
#if defined(_MSC_VER) && (_MSC_VER>=1300)
   #pragma warning (disable:4995)
#endif

#include <hbapi.h>
#include <hbapiitm.h>
#include "pdflib.h"

PDF *szPDFFile;
#define FONTBOLD "Courier-Bold"
#define FONTNORMAL "Courier"
#define FONTSIZE 10.0/*10.0*/
#define FONTSIZEBOLD 15.0
#define FONTSIZESMALL 10.0     /*10.0*/
#define FONTSIZETABLE 8.5
#define LEAD ((int) (FONTSIZESMALL * 1.0))
#define LEADLINK ((int) (FONTSIZESMALL * 1.3))
#define LEADTABLE ((int) (FONTSIZESMALL * 1.2))

static float fRow = 800;
static float fOldPos;
static float iWidth;
static float fCol;
static int sziFontBold = 0;
static int sziFont = 0;
static int iPage = 1;
static HB_SIZE uiLen;
static HB_SIZE uiCount;
static BOOL bTItems;
static BOOL bFItems;
static float /* fCurRow, */ fCurCol = 0 ,fCurLastCol, fLastRow;
static float fStartCol, fStartCol2;
static HB_ITEM pArray = { 0 };
static BOOL bInitArray = 0;
static float hb_checkStringWidth( const char * szString );
/* static float hb_pdfGetHeight( const char *szText ); */
static void hb_ProcessTableItem( PHB_ITEM p1, PHB_ITEM p2, PHB_ITEM p3, PHB_ITEM p4 );
float getText( PDF *p, const char *szText, int iFont, float frow, float fcol, float iw);
void  setText( PDF *p, const char *szText, float frow, float fcol, float h, float iw);

HB_FUNC(HB_PDFNEW)
{
   const char *szFileResult;
   szFileResult= ( char * ) hb_parc( 1 );

   if ( szPDFFile == NULL)
   {
      szPDFFile = PDF_new();
      PDF_set_value( szPDFFile, "compress", ( float ) 9);
   }

   if ( PDF_open_file( szPDFFile, szFileResult ) == -1 )
   {
     hb_retl( 1 );
   }

   PDF_set_info( szPDFFile, "Keywords", "XHarbour compiler" );
   PDF_set_info( szPDFFile, "Subject" , "XHarbour" ) ;
   PDF_set_info( szPDFFile, "Title"   , "XHarbour doc guide" );
   PDF_set_info( szPDFFile, "Creator" , "HBDOC XHarbour Document Extractor" );
   PDF_set_info( szPDFFile, "Author"  , "Luiz Rafael Culik" );
   sziFont = PDF_findfont( szPDFFile, FONTNORMAL, "host", 0);
   sziFontBold = PDF_findfont( szPDFFile, FONTBOLD, "host", 0);
   hb_retl( 0 );
}

HB_FUNC(HB_PDFNEWPAGE)
{
   const char *szTitleT;
   fCurCol = 0;
   szTitleT= ( char * ) hb_parc( 1 );
   PDF_begin_page( szPDFFile, a4_width, a4_height ); /* start a new page */
   PDF_setfont( szPDFFile, sziFontBold, FONTSIZEBOLD );
   PDF_show_xy( szPDFFile, szTitleT, 50, fRow -= LEAD );
}

HB_FUNC(HB_PDFENDPAGE)
{
   fRow = 800;
   PDF_end_page( szPDFFile );                /* close page       */
   iPage ++ ;
}

HB_FUNC(HB_PDFWRITEBOLDTEXT)
{
   const char *szTextT;
   szTextT = ( char * ) hb_parc( 1 );

   if ( fRow<=40)
   {
      fRow=800;
      PDF_end_page( szPDFFile );
      sziFont = PDF_findfont( szPDFFile, FONTNORMAL, "host", 0 );
      sziFontBold = PDF_findfont( szPDFFile, FONTBOLD, "host", 0 );
      PDF_begin_page( szPDFFile, a4_width, a4_height ); /* start a new page */
      iPage ++ ;
   }

   PDF_setfont( szPDFFile, sziFontBold, FONTSIZESMALL );

   PDF_show_xy( szPDFFile, szTextT, 25, fRow -= LEAD );
   PDF_setfont( szPDFFile, sziFont, FONTSIZESMALL );
}

HB_FUNC(HB_PDFWRITEBOLDTEXT1)
{
   const char *szTextT;
   szTextT = ( char * ) hb_parc( 1 );

   if ( fRow <= 40 )
   {
      fRow = 800;
      PDF_end_page( szPDFFile );
      PDF_begin_page( szPDFFile, a4_width, a4_height ); /* start a new page */
      sziFont = PDF_findfont( szPDFFile, FONTNORMAL, "host", 0);
      sziFontBold = PDF_findfont( szPDFFile, FONTBOLD, "host", 0);
      iPage ++;
   }

   PDF_setfont( szPDFFile, sziFontBold, FONTSIZEBOLD );
   PDF_show_xy( szPDFFile, szTextT, 25, fRow -= LEAD );
   PDF_setfont( szPDFFile, sziFont, FONTSIZESMALL );
}

HB_FUNC(HB_PDFWRITETEXT)
{
   const char *szTextT;
   szTextT = ( char * ) hb_parc( 1 );

   if ( fRow <= 40 )
   {
      fRow=800;
      PDF_end_page( szPDFFile );
      sziFont = PDF_findfont( szPDFFile, FONTNORMAL, "host", 0);
      sziFontBold = PDF_findfont( szPDFFile, FONTBOLD, "host", 0);
      PDF_begin_page( szPDFFile, a4_width, a4_height); /* start a new page */
      iPage ++;
   }

   PDF_setfont( szPDFFile, sziFont, FONTSIZESMALL );
   PDF_show_xy( szPDFFile, szTextT, 25, fRow -= LEAD);
}

HB_FUNC(HB_PDFCLOSE)
{
   hb_itemClear( &pArray );
   PDF_close( szPDFFile );               /* close PDF document   */
   PDF_delete( szPDFFile );             /* delete the PDF object */
   szPDFFile=NULL;
   sziFontBold=0;
   sziFont=0;
}

HB_FUNC(HB_PDFWRITEARG)
{
   const char *szTextT;
   const char *szBTextT;

   szTextT  = ( char * ) hb_parc( 2 );
   szBTextT = ( char * ) hb_parc( 1 );

   if ( fRow <= 40 )
   {
      fRow = 800;
      PDF_end_page( szPDFFile );
      sziFont = PDF_findfont( szPDFFile, FONTNORMAL, "host", 0);
      sziFontBold = PDF_findfont( szPDFFile, FONTBOLD, "host", 0);
      PDF_begin_page( szPDFFile, a4_width, a4_height); /* start a new page */
      iPage ++;
   }

   PDF_setfont( szPDFFile, sziFontBold, FONTSIZESMALL );
   PDF_show_xy( szPDFFile, szBTextT, 25, fRow -= LEAD );
   PDF_setfont( szPDFFile, sziFont, FONTSIZESMALL );
   PDF_show( szPDFFile, szTextT );
}

HB_FUNC(HB_PDFADDLINK)
{
   int iPagetoGo = hb_parni( 2 ) - 1;
   const char *szLink;

   szLink = ( char * ) hb_parc( 1 );
   if (fRow <= 40 )
   {
      fRow = 800;
      PDF_end_page( szPDFFile );
      sziFont = PDF_findfont( szPDFFile, FONTNORMAL, "host", 0 );
      sziFontBold = PDF_findfont( szPDFFile, FONTBOLD, "host", 0 );
      PDF_begin_page( szPDFFile, a4_width, a4_height ); /* start a new page */
      PDF_setfont( szPDFFile, sziFont, FONTSIZESMALL );
      iPage ++;
   }

   fCol   = ( float ) fRow;
   iWidth = hb_checkStringWidth( szLink );

   // New Page,  so curCol is 0
   if ( fCurCol == 0)
   {
      fCurCol = 60 ;
      fLastRow = ( fRow - LEADLINK );
      fCurLastCol = ( fCol - LEAD );
      fStartCol = 49 ;
      fStartCol2 = 50 ;

   }

   if ( fCurCol + iWidth + 10  <= a4_width )
   {
      PDF_set_border_color( szPDFFile, ( float ) 1 ,( float )  1 ,( float ) 1 );
      PDF_add_locallink( szPDFFile, fStartCol, fLastRow, fCurCol + iWidth, fCol, iPagetoGo, "fitwidth");
      PDF_setrgbcolor( szPDFFile, ( float ) 0, ( float ) 0, ( float ) 1 );
      PDF_set_parameter( szPDFFile, "underline", "true" );
      PDF_show_xy( szPDFFile, szLink, fStartCol2, fCurLastCol );
      PDF_setrgbcolor( szPDFFile, ( float ) 0, ( float ) 0, ( float ) 0);
      PDF_set_parameter( szPDFFile, "underline", "false");
      fCurCol += 10 + iWidth;
      fStartCol +=  iWidth;
      fStartCol2 += iWidth;


      if( fCurCol > a4_width )
      {
          fCurCol = 00 ;
          fStartCol = 49 ;
          fStartCol2 = 50 ;
          fRow -= LEADLINK;
          fCol -= LEAD;

      }
    }
      iWidth = 0 ;
}

HB_FUNC(HB_PDFINITBOOK)
{
   PHB_ITEM pItems;
   pItems = hb_param( 1, HB_IT_ARRAY );
   uiLen = hb_arrayLen( pItems );

   if ( !bInitArray )
   {
      pArray.type = HB_IT_NIL;
      hb_arrayNew( &pArray, 0 );
      bInitArray = 1;
   }

   for ( uiCount = 1 ; uiCount <= uiLen ; uiCount++ )
   {
      const char *szBook = ( char * ) hb_arrayGetCPtr( pItems, uiCount );
      int iLevel;
      HB_ITEM TempArray;
      HB_ITEM Temp;

      Temp.type=HB_IT_NIL;
      TempArray.type=HB_IT_NIL;

      hb_arrayNew( &TempArray, 2 );
      iLevel=PDF_add_bookmark( szPDFFile, szBook, 0, 0);
      hb_arraySetForward( &TempArray, 1, hb_itemPutC( &Temp, hb_arrayGetCPtr( pItems, uiCount ) ) );

      hb_arraySetForward( &TempArray, 2, hb_itemPutNI( &Temp, iLevel));
      hb_arraySetForward( &pArray, uiCount, &TempArray );
      hb_itemClear( &Temp ) ;

   }
}

HB_FUNC(HB_PDFBOOKMARK)
{
   const char *szBook1 =( char * )hb_parc( 2 );
   HB_SIZE uiPos;
   HB_ITEM TempArray;
   int iParent = 0;
   PHB_ITEM pBlock = hb_param( 3, HB_IT_BLOCK );

   TempArray.type = HB_IT_NIL;
   uiPos = hb_arrayScan( &pArray, pBlock, NULL, NULL, 0, 0 );

   if ( uiPos > 0 )
   {
      if ( hb_arrayGet( &pArray, uiPos, &TempArray ) )
      {
         iParent = hb_arrayGetNI( &TempArray, 2      );
         hb_itemClear( &TempArray );
      }
   }

   PDF_add_bookmark(szPDFFile, szBook1, iParent, 0);
}

HB_FUNC(HB_GETPAGE)
{
   hb_retni( iPage );
}

#if 0
static  BOOL hb_checkRow( float iLine )
{
   if ( iLine <= 20 )
   {
      fRow = 800;
      PDF_end_page( szPDFFile );
      PDF_begin_page( szPDFFile, a4_width, a4_height); /* start a new page */
      iPage ++;
      return TRUE;
   }

   return FALSE;
}
#endif

static  float hb_checkStringWidth(const char *szString)
{
   float fReturn;
   fReturn = (float) PDF_stringwidth( szPDFFile, szString, sziFont, FONTSIZESMALL);
   return fReturn;
}

HB_FUNC(HB_PDFTABLE)
{
   PHB_ITEM pTableItem  ;
   PHB_ITEM pTableItem1 ;
   PHB_ITEM pTableItem2 ;
   PHB_ITEM pTableItem3 ;
   ULONG ulPos;

   pTableItem  = hb_param( 1, HB_IT_ARRAY );
   pTableItem1 = hb_param( 2, HB_IT_ARRAY );
   pTableItem2 = hb_param( 3, HB_IT_ARRAY );
   pTableItem3 = hb_param( 4, HB_IT_ARRAY );

   if ( pTableItem2 != NULL)
   {
      bTItems = TRUE;
   }

   if ( pTableItem3 != NULL )
   {
      bFItems = TRUE;
      bTItems = FALSE;
   }

   fRow -= LEAD;

   for ( ulPos = 1 ; ulPos <= hb_arrayLen( pTableItem ) ; ulPos ++ )
   {
      HB_ITEM TempArray  ;
      HB_ITEM TempArray1 ;
      TempArray.type = HB_IT_NIL;
      TempArray1.type = HB_IT_NIL;

      fCol = fRow;
      hb_arrayGet( pTableItem, ulPos, &TempArray);
      hb_arrayGet( pTableItem1, ulPos, &TempArray1 );

      if ( !bTItems && !bFItems)
      {
         hb_ProcessTableItem( &TempArray, &TempArray1, NULL, NULL);
         hb_itemClear( &TempArray );
         hb_itemClear( &TempArray1 );
      }

      if ( bTItems && !bFItems)
      {
         HB_ITEM TempArray2;
         TempArray2.type = HB_IT_NIL;

         hb_arrayGet( pTableItem2 , ulPos, &TempArray2 );
         hb_ProcessTableItem( &TempArray, &TempArray1, &TempArray2, NULL );
         hb_itemClear( &TempArray );
         hb_itemClear( &TempArray1 );
         hb_itemClear( &TempArray2 );
      }

      if (!bTItems && bFItems)
      {
         HB_ITEM TempArray2 ;
         HB_ITEM TempArray3 ;

         TempArray3.type = HB_IT_NIL;
         TempArray2.type = HB_IT_NIL;

         hb_arrayGet( pTableItem2, ulPos, &TempArray2 );
         hb_arrayGet( pTableItem3, ulPos, &TempArray3 );
         hb_ProcessTableItem( &TempArray, &TempArray1, &TempArray2, &TempArray3 );
         hb_itemClear( &TempArray );
         hb_itemClear( &TempArray1 );
         hb_itemClear( &TempArray2 );
         hb_itemClear( &TempArray3 );
     }
   }

   bTItems=FALSE;
   bFItems=FALSE;
   fOldPos=0;
}

static void hb_ProcessTableItem( PHB_ITEM p1, PHB_ITEM p2, PHB_ITEM p3, PHB_ITEM p4)
{
   ULONG ulTempPos;
   HB_SIZE ulLen;
   /*
   float fHeight;
   float fI; */
   float iitem, iitem1, iitem2,iitem3;

   ulLen = hb_arrayLen( p1 );

   for ( ulTempPos = 1 ; ulTempPos <= ulLen ; ulTempPos ++ )
   {
      const char *szTemp  = ( char * ) hb_arrayGetCPtr( p1, ulTempPos );
      const char *szTemp1 = ( char * ) hb_arrayGetCPtr( p2, ulTempPos );

      if (!bTItems && !bFItems)
      {
         PDF_setfont( szPDFFile, sziFont, FONTSIZETABLE);
         iitem  = getText( szPDFFile, szTemp1, sziFont, 293, fRow, 261);
         iitem1 = getText( szPDFFile, szTemp, sziFont, 27, fRow, 261);
         if ( iitem < iitem1)
         {
            setText( szPDFFile, szTemp1, 293, fRow, iitem1, 261);
            setText( szPDFFile, szTemp, 27, fRow, iitem1, 261);
            fRow -= iitem1;
         }
         else
         {
            setText( szPDFFile, szTemp1, 293, fRow, iitem, 261);
            setText( szPDFFile, szTemp, 27, fRow, iitem, 261);
            fRow -= iitem;
         }

      }

      if (bTItems && !bFItems)
      {
         const char * szTemp2=( char * ) hb_arrayGetCPtr( p3, ulTempPos );

         PDF_setfont( szPDFFile, sziFont, FONTSIZETABLE);
         iitem  = getText( szPDFFile, szTemp2, sziFont, 381, fRow, 173);
         iitem1 = getText( szPDFFile, szTemp1, sziFont, 204, fRow, 172);
         iitem2 = getText( szPDFFile, szTemp,  sziFont, 27,  fRow, 172);

         if ( iitem > iitem1 && iitem > iitem2)
         {
            setText( szPDFFile, szTemp2, 381, fRow, iitem, 173);
            setText( szPDFFile, szTemp1, 204, fRow, iitem, 172);
            setText( szPDFFile, szTemp , 27,  fRow, iitem, 172);
            fRow -= iitem;
         }

         if ( iitem2 > iitem && iitem2 >iitem1)
         {
            setText( szPDFFile, szTemp2, 381, fRow, iitem2, 173);
            setText( szPDFFile, szTemp1, 204, fRow, iitem2, 172);
            setText( szPDFFile, szTemp,  27,  fRow, iitem2, 172);
            fRow -= iitem2;
         }

         if ( iitem1 > iitem && iitem1 > iitem2 )
         {
            setText( szPDFFile, szTemp2, 381, fRow, iitem1, 173);
            setText( szPDFFile, szTemp1, 204, fRow, iitem1, 172);
            setText( szPDFFile, szTemp,  27,  fRow, iitem1, 172);
            fRow -= iitem1;
         }

      }

      if ( !bTItems && bFItems )
      {
         const char * szTemp2 = ( char * ) hb_arrayGetCPtr( p3, ulTempPos );
         const char * szTemp3 = ( char * ) hb_arrayGetCPtr( p4, ulTempPos );

         PDF_setfont( szPDFFile, sziFont, FONTSIZETABLE);
         iitem1 = getText( szPDFFile, szTemp3, sziFont, 426, fRow, 128 );
         iitem2 = getText( szPDFFile, szTemp2, sziFont, 293, fRow, 128 );
         iitem  = getText( szPDFFile, szTemp1, sziFont, 160, fRow, 128 );
         iitem3 = getText( szPDFFile, szTemp, sziFont,  27,  fRow, 128 );
         if (iitem > iitem1 && iitem > iitem2 && iitem > iitem3)
         {
            setText( szPDFFile, szTemp3, 426, fRow, iitem, 128);
            setText( szPDFFile, szTemp2, 293, fRow, iitem, 128);
            setText( szPDFFile, szTemp1, 160, fRow, iitem, 128);
            setText( szPDFFile, szTemp,  27,  fRow, iitem, 128);
            fRow -= iitem;
         }
         else
         {
            if (iitem2 > iitem && iitem2 > iitem1 && iitem2 > iitem3)
            {
               setText( szPDFFile, szTemp3, 426, fRow, iitem2, 128);
               setText( szPDFFile, szTemp2, 293, fRow, iitem2, 128);
               setText( szPDFFile, szTemp1, 160, fRow, iitem2, 128);
               setText( szPDFFile, szTemp,   27, fRow, iitem2, 128);
               fRow -= iitem2;
            }
            else
            {
               if ( iitem1 > iitem && iitem1 > iitem2 && iitem1 > iitem3)
               {
                   setText(  szPDFFile, szTemp3, 426, fRow, iitem1, 128);
                   setText(  szPDFFile, szTemp2, 293, fRow, iitem1, 128);
                   setText(  szPDFFile, szTemp1, 160, fRow, iitem1, 128);
                   setText(  szPDFFile, szTemp,   27, fRow, iitem1, 128);
                   fRow -= iitem1;
               }
               else
               {
                  if ( iitem3 > iitem && iitem3 > iitem2 && iitem3 > iitem)
                  {
                     setText( szPDFFile, szTemp3, 426, fRow, iitem3, 128);
                     setText( szPDFFile, szTemp2, 293, fRow, iitem3, 128);
                     setText( szPDFFile, szTemp1, 160, fRow, iitem3, 128);
                     setText( szPDFFile, szTemp,   27, fRow, iitem3, 128);
                     fRow -= iitem3;
                  }
                  else
                  {
                     setText( szPDFFile, szTemp3, 426, fRow, iitem3, 128);
                     setText( szPDFFile, szTemp2, 293, fRow, iitem3, 128);
                     setText( szPDFFile, szTemp1, 160, fRow, iitem3, 128);
                     setText( szPDFFile, szTemp,   27, fRow, iitem3, 128);
                     fRow -= iitem3;

                  }
               }
            }
         }
      }
   }
}

float getText( PDF *p, const char *szText, int iFont, float frow, float fcol, float iw )
{
     float h;
     /* float w; */
     int c;
     h = FONTSIZETABLE * 2;
     PDF_setfont( p, iFont, 7.0);

     c = PDF_show_boxed( p, szText, frow, fcol, iw, h, "justify", "blind" );

     while ( c > 0)
     {
        h += FONTSIZETABLE;
        c = PDF_show_boxed(p, szText, frow, fcol, iw, h, "justify", "blind" );
     }

     return h;
}

void setText( PDF *p, const char *szText, float frow, float fcol, float h, float iw )
{
     PDF_show_boxed( p, szText, frow, fcol, iw, h, "justify", "" );
     PDF_rect( p, frow, fcol, iw + 5, h );
     PDF_stroke( p );
}

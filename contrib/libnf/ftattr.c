/*
 * $Id: ftattr.c 9773 2012-10-19 08:29:51Z andijahja $
 */

/*
 * File......: SAVEATT.ASM
 * Author....: Ted Means
 * CIS ID....: 73067,3332
 *
 * This is an original work by Ted Means and is placed in the
 * public domain.
 *
 * Modification history:
 * ---------------------
 *
 *     Rev 1.2   03 Oct 1992 14:35:14   GLENN
 *  Ted Means made modifications to support use of dispBegin()/dispEnd().
 *
 *     Rev 1.1   15 Aug 1991 23:07:58   GLENN
 *  Forest Belt proofread/edited/cleaned up doc
 *
 *     Rev 1.0   12 Jun 1991 01:30:20   GLENN
 *  Initial revision.
 *

 *  $DOC$
 *  $FUNCNAME$
 *     FT_SAVEATT()
 *  $CATEGORY$
 *     Video
 *  $ONELINER$
 *     Save the attribute bytes of a specified screen region.
 *  $SYNTAX$
 *     FT_SAVEATT( <nTop>, <nLeft>, <nBottom>, <nRight>  ) -> cAttributes
 *  $ARGUMENTS$
 *     <nTop>, <nLeft>, <nBottom>, and <nRight> define the screen region.
 *  $RETURNS$
 *     A character string containing the screen attribute bytes for the
 *     specified region.  If the memory to store the return value could
 *     not be allocated, the function returns NIL.
 *  $DESCRIPTION$
 *     This function is similar to Clipper's SaveScreen(), except that it only
 *     saves the attribute bytes.  This is useful if you want to change the
 *     screen color without affecting the text.
 *
 *     *** INTERNALS ALERT ***
 *
 *     This function calls the Clipper internal __gtMaxCol to obtain the
 *     maximum column value for the current video mode.  If you're too gutless
 *     to use internals, then this function isn't for you.
 *  $EXAMPLES$
 *     // Save attributes of row 4
 *     cBuffer := FT_SAVEATT( 4, 0, 4, maxcol())
 *
 *     // Save attributes from middle of screen
 *     cBuffer := FT_SAVEATT(10,20,14,59)
 *  $SEEALSO$
 *     FT_RESTATT()
 *  $END$
 *
 */

/* This is the original FT_SAVEATT() code
   IDEAL

   Public   FT_SaveAtt

   Extrn    __ParNI:Far
   Extrn    __RetCLen:Far
   Extrn    __xGrab:Far
   Extrn    __xFree:Far
   Extrn    __gtSave:Far

   nTop     EQU       Word Ptr BP - 2
   nLeft    EQU       Word Ptr BP - 4
   nBottom  EQU       Word Ptr BP - 6
   nRight   EQU       Word Ptr BP - 8
   nAttr    EQU       Byte Ptr BP - 10
   nBufLen  EQU       Word Ptr BP - 12

   cBuffer  EQU       DWord Ptr BP - 16
   nBufOfs  EQU       Word Ptr BP - 16
   nBufSeg  EQU       Word Ptr BP - 14

   Segment  _NanFor   Word      Public    "CODE"
         Assume    CS:_NanFor

   Proc     FT_SaveAtt          Far

         Push      BP                        ; Save BP
         Mov       BP,SP                     ; Set up stack reference
         Sub       SP,16                     ; Allocate locals

         Mov       CX,4                      ; Set param count
   @@Coord: Push      CX                        ; Put on stack
         Call      __ParNI                   ; Retrieve param
         Pop       CX                        ; Get count back
         Push      AX                        ; Put value on stack
         Loop      @@Coord                   ; Get next value

         Pop       [nTop]                    ; Store top coordinate
         Pop       [nLeft]                   ; Store left coordinate
         Pop       [nBottom]                 ; Store bottom coordinate
         Pop       [nRight]                  ; Store right coordinate

         Mov       AX,[nBottom]              ; Load bottom coordinate
         Sub       AX,[nTop]                 ; Subtract top
         Inc       AX                        ; Calc length

         Mov       CX,[nRight]               ; Load right coordinate
         Sub       CX,[nLeft]                ; Subtract left
         Inc       CX                        ; Calc width
         Mul       CX                        ; Multiply length by width
         SHL       AX,1                      ; Calc buffer size
         Mov       [nBufLen],AX              ; Store buffer size

   @@Alloc: Push      AX                        ; Put size on stack
         Call      __xGrab                   ; Allocate memory
         Add       SP,2                      ; Realign stack
         Mov       [nBufSeg],DX              ; Store segment
         Mov       [nBufOfs],AX              ; Store offset

         Push      DX                        ; Load parameters for __gtSave
         Push      AX                        ; onto stack
         Push      [nRight]
         Push      [nBottom]
         Push      [nLeft]
         Push      [nTop]
         Call      __gtSave                  ; Grab screen image

         Push      DS                        ; Save required registers
         Push      SI
         Push      DI

         Mov       DS,[nBufSeg]              ; Load pointer to buffer
         Mov       SI,[nBufOfs]              ; into DS:SI

         Push      DS                        ; Duplicate pointer in ES:DI
         Push      SI
         Pop       DI
         Pop       ES
         Inc       SI                        ; Start with attribute byte

         Mov       CX,[nBufLen]              ; Load buffer length
         SHR       CX,1                      ; Divide by two
   @@Attr:  Lodsw                               ; Grab a screen word
         Stosb                               ; Store attribute only
         Loop      @@Attr                    ; Do next

         Pop       DI                        ; Restore registers
         Pop       SI
         Pop       DS

   Done:    Mov       AX,[nBufLen]              ; Load buffer length
         SHR       AX,1                      ; Divide by 2
         Push      AX                        ; Put length on stack
         Push      [nBufSeg]                 ; Put segment on stack
         Push      [nBufOfs]                 ; Put offset on stack
         Call      __RetClen                 ; Return attribute string
         Call      __xFree                   ; Free memory
         Mov       SP,BP                     ; Realign stack
         Pop       BP                        ; Restore BP
         Ret
   Endp     FT_SaveAtt
   Ends     _NanFor
   End
 */

#include "hbapi.h"
#include "hbapigt.h"

/* This is the New one Rewriten in C */

HB_FUNC( FT_SAVEATT )
{
   USHORT   uiTop    = ( USHORT ) hb_parni( 1 );  /* Defaults to zero on bad type */
   USHORT   uiLeft   = ( USHORT ) hb_parni( 2 );  /* Defaults to zero on bad type */
   USHORT   uiMaxRow = hb_gtMaxRow();
   USHORT   uiMaxCol = hb_gtMaxCol();
   USHORT   uiBottom = ISNUM( 3 ) ? ( USHORT ) hb_parni( 3 ) : uiMaxRow;
   USHORT   uiRight  = ISNUM( 4 ) ? ( USHORT ) hb_parni( 4 ) : uiMaxRow;

   ULONG    ulSize;
   char *   pBuffer;
   char *   pAttrib;

   if( uiBottom > uiMaxRow )
      uiBottom = uiMaxRow;
   if( uiRight > uiMaxCol )
      uiRight = uiMaxCol;

   if( uiTop <= uiBottom && uiLeft <= uiRight )
   {
      ulSize   = ( uiBottom - uiTop + 1 ) * ( uiRight - uiLeft + 1 );
      pBuffer  = pAttrib = ( char * ) hb_xgrab( ulSize + 1 );
      while( uiTop <= uiBottom )
      {
         USHORT uiCol = uiLeft;
         while( uiCol <= uiRight )
         {
            BYTE     bColor, bAttr;
            USHORT   usChar;
            hb_gtGetChar( uiTop, uiCol, &bColor, &bAttr, &usChar );
            *pBuffer++ = ( char ) bColor;
            ++uiCol;
         }
         ++uiTop;
      }
      hb_retclen_buffer( pAttrib, ulSize );
   }
   else
      hb_retc( NULL );
}

/*
 * File......: RESTATT.ASM
 * Author....: Ted Means
 * CIS ID....: 73067,3332
 *
 * This is an original work by Ted Means and is placed in the
 * public domain.
 *
 * Modification history:
 * ---------------------
 *
 *     Rev 1.2   03 Oct 1992 14:33:46   GLENN
 *  Ted Means made modifications so these functions will work with
 *  dispBegin() and dispEnd().
 *
 *     Rev 1.1   15 Aug 1991 23:08:02   GLENN
 *  Forest Belt proofread/edited/cleaned up doc
 *
 *     Rev 1.0   12 Jun 1991 01:30:14   GLENN
 *  Initial revision.
 *

 *  $DOC$
 *  $FUNCNAME$
 *     FT_RESTATT()
 *  $CATEGORY$
 *     Video
 *  $ONELINER$
 *     Restore the attribute bytes of a specified screen region.
 *  $SYNTAX$
 *     FT_RESTATT( <nTop>, <nLeft>, <nBottom>, <nRight>, <cAttributes> ) -> NIL
 *  $ARGUMENTS$
 *     <nTop>, <nLeft>, <nBottom>, and <nRight> define the screen region.
 *     <cAttributes> is a character string containing the attribute bytes
 *                   for the screen region.  This will most often be a string
 *                   previously returned by FT_SAVEATT(), but any character
 *                   string may be used (provided it is of the proper size).
 *  $RETURNS$
 *     NIL
 *  $DESCRIPTION$
 *     This function is similar to Clipper's RestScreen(), except that it only
 *     restores the attribute bytes.  This is useful if you want to change the
 *     screen color without affecting the text.
 *
 *     *** INTERNALS ALERT ***
 *
 *     This function calls the Clipper internals __gtSave and __gtRest to
 *     manipulate the the screen image.  If you're too gutless to use
 *     internals, then this function isn't for you.
 *  $EXAMPLES$
 *     // Restore attributes of row 4
 *     FT_RESTATT( 4, 0, 4, maxcol(), cBuffer)
 *
 *     // Restore attributes to middle of screen
 *     FT_RESTATT(10,20,14,59,cBuffer)
 *  $SEEALSO$
 *     FT_SAVEATT()
 *  $END$
 *
 */

/* This is the Original FT_RESTATT() code
   IDEAL

   Public   FT_RestAtt

   Extrn    __ParNI:Far
   Extrn    __ParC:Far
   Extrn    __XGrab:Far
   Extrn    __XFree:Far
   Extrn    __gtSave:Far
   Extrn    __gtRest:Far

   nTop     EQU       Word Ptr BP - 2
   nLeft    EQU       Word Ptr BP - 4
   nBottom  EQU       Word Ptr BP - 6
   nRight   EQU       Word Ptr BP - 8
   nAttr    EQU       Byte Ptr BP - 10
   nBufLen  EQU       Word Ptr BP - 12

   cBuffer  EQU       DWord Ptr BP - 16
   nBufOfs  EQU       Word Ptr BP - 16
   nBufSeg  EQU       Word Ptr BP - 14

   Segment  _NanFor   Word      Public    "CODE"
         Assume    CS:_NanFor

   Proc     FT_RestAtt          Far

         Push      BP                        ; Save BP
         Mov       BP,SP                     ; Set up stack reference
         Sub       SP,16                     ; Allocate locals

         Mov       CX,4                      ; Set param count
   @@Coord: Push      CX                        ; Put on stack
         Call      __ParNI                   ; Retrieve param
         Pop       CX                        ; Get count back
         Push      AX                        ; Put value on stack
         Loop      @@Coord                   ; Get next value

         Pop       [nTop]                    ; Store top coordinate
         Pop       [nLeft]                   ; Store left coordinate
         Pop       [nBottom]                 ; Store bottom coordinate
         Pop       [nRight]                  ; Store right coordinate

         Mov       AX,[nBottom]              ; Load bottom coordinate
         Sub       AX,[nTop]                 ; Subtract top
         Inc       AX                        ; Calc length

         Mov       CX,[nRight]               ; Load right coordinate
         Sub       CX,[nLeft]                ; Subtract left
         Inc       CX                        ; Calc width
         Mul       CX                        ; Multiply length by width
         SHL       AX,1                      ; Calc buffer size
         Mov       [nBufLen],AX              ; Store buffer size

   @@Alloc: Push      AX                        ; Put size on stack
         Call      __xGrab                   ; Allocate memory
         Add       SP,2                      ; Realign stack
         Mov       [nBufSeg],DX              ; Store segment
         Mov       [nBufOfs],AX              ; Store offset

         Push      DX                        ; Load parameters for __gtSave
         Push      AX                        ; onto stack
         Push      [nRight]
         Push      [nBottom]
         Push      [nLeft]
         Push      [nTop]
         Call      __gtSave                  ; Grab screen image

         Push      DS                        ; Save required registers
         Push      SI
         Push      DI

         Mov       AX,5                      ; Specify 5th param
         Push      AX                        ; Put on stack
         Call      __ParC                    ; Get pointer to attr string
         Add       SP,2                      ; Realign stack

         Mov       DS,DX                     ; Load pointer to string
         Mov       SI,AX                     ; into DS:SI
         Mov       ES,[nBufSeg]              ; Load pointer to buffer
         Mov       DI,[nBufOfs]              ; into ES:DI
         Mov       CX,[nBufLen]              ; Load buffer length
         SHR       CX,1                      ; Divide by two

   @@Attr:  Inc       DI                        ; Point DI to attribute
         Lodsb                               ; Grab an attribute byte
         Stosb                               ; Store attribute
         Loop      @@Attr                    ; Do next

         Pop       DI                        ; Restore registers
         Pop       SI
         Pop       DS
         Call      __gtRest                  ; Restore screen image

   Done:    Push      [nBufSeg]                 ; Put segment on stack
         Push      [nBufOfs]                 ; Put offset on stack
         Call      __xFree                   ; Free memory
         Mov       SP,BP                     ; Realign stack
         Pop       BP                        ; Restore BP
         Ret
   Endp     FT_RestAtt
   Ends     _NanFor
   End
 */

/* This is the New one Rewriten in C */

HB_FUNC( FT_RESTATT )
{
   ULONG ulLen = (ULONG) hb_parclen( 5 );

   if( ulLen )
   {
      USHORT         uiTop    = ( USHORT ) hb_parni( 1 );  /* Defaults to zero on bad type */
      USHORT         uiLeft   = ( USHORT ) hb_parni( 2 );  /* Defaults to zero on bad type */
      USHORT         uiMaxRow = hb_gtMaxRow();
      USHORT         uiMaxCol = hb_gtMaxCol();
      USHORT         uiBottom = ISNUM( 3 ) ? ( USHORT ) hb_parni( 3 ) : hb_gtMaxRow();
      USHORT         uiRight  = ISNUM( 4 ) ? ( USHORT ) hb_parni( 4 ) : hb_gtMaxCol();
      const char *   pAttrib  = hb_parc( 5 );

      if( uiBottom > uiMaxRow )
         uiBottom = uiMaxRow;
      if( uiRight > uiMaxCol )
         uiRight = uiMaxCol;

      if( uiTop <= uiBottom && uiLeft <= uiRight )
      {
         while( ulLen && uiTop <= uiBottom )
         {
            USHORT uiCol = uiLeft;
            while( ulLen && uiCol <= uiRight )
            {
               BYTE     bColor, bAttr;
               USHORT   usChar;
               hb_gtGetChar( uiTop, uiCol, &bColor, &bAttr, &usChar );
               bColor = *pAttrib++;
               hb_gtPutChar( uiTop, uiCol, bColor, bAttr, usChar );
               ++uiCol;
               --ulLen;
            }
            ++uiTop;
         }
      }
   }
}

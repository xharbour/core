*****************************************************
* Demo of realtime string compression
*
* Giancarlo Niccolai
*
* $Id: compress.prg 9279 2011-02-14 18:06:32Z druzus $
*

#include "hbcomprs.ch"

PROCEDURE MAIN()
   LOCAL cText := "A  text  to  be  compressed             "
   LOCAL cComp
   LOCAL cDecomp
   LOCAL nError, nBufLen


   CLEAR SCREEN
   @1,15 SAY "X H A R B O U R - ZLIB based compression test"

   @2,5 SAY "TEST 1: using on-the-fly Buffer creation"

   nBufLen := HB_CompressBuflen( Len( cText ) )
   //cComp and cDecomp will be created with the correct length
   cComp := HB_Compress( cText )
   cDecomp := HB_Uncompress( nBuflen, cComp )

   @3,7 SAY "Uncompressed: (" +Alltrim( Str( Len(cText) ) ) +")"+ cText + "<<"
   @4,7 SAY "Compressed ("+Alltrim( Str( Len( cComp ) ) ) +")"+ cComp +"<<"
   @5,7 SAY "Decompressed: (" +Alltrim( Str( Len(cDecomp) ) ) +")"+ cDecomp + "<<"

   @7,5 SAY "TEST 2: using preallocated buffers"

   cComp := Space( nBufLen )
   //We allocate more space (manual says 0.1% + 12, but you can never know...)
   // to allow compression of uncompressable strings to grow a little.
   cDecomp := Space( Int(nBufLen * 1.1) )

   //on exit, nBuflen will contain the length of the compressed buffer
   HB_Compress( cText, Len( cText ), @cComp, @nBuflen )
   HB_Uncompress( Len( cText ), cComp, nBuflen , @cDecomp )

   @8,7 SAY "Uncompressed: (" +Alltrim( Str( Len( cText) ) ) +")"+ cText + "<<"
   @9,7 SAY "Compressed ("+Alltrim( Str( nBuflen ) ) +")"+ cComp + "<<"
   //Notice: this time the lenght of the destination buffer is not the lenght of
   // the original buffer, but Int(nBufLen * 1.1)
   @10,7 SAY "Decompressed: ("+Alltrim( Str( Len( cDecomp ) )) +")"+ cDecomp + "<<"

   @12,5 SAY "TEST 3: Generating an error"

   nBufLen := HB_CompressBuflen( Len( cText ) )
   cComp := Space( nBufLen )
   cDecomp := Space( Len( cText ) )

   // we generate an error: 3 is not a valid length for this buffer
   nBuflen := 3
   nError := HB_Compress( cText, Len( cText), @cComp, @nBuflen )

   IF nError != HB_Z_OK
      @13,7 SAY "Error generated ("+Alltrim( Str( Len( cComp ) ) ) +")"+ ;
            HB_CompressErrorDesc( nError )
   ELSE
      @13,7 SAY "NO Error generated ("+Alltrim( Str( Len( cComp ) ) ) +")"+ ;
            HB_CompressErrorDesc( nError )
   ENDIF

   @22,25 SAY "Press a key to terminate"
   INKEY(0)
RETURN


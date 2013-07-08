/*
 * $Id: bitclr.prg 9686 2012-09-16 08:14:35Z andijahja $
 */
/*
 * File......: BITCLR.PRG
 * Author....: Forest Belt, Computer Diagnostic Services, Inc.
 * CIS ID....: ?
 *
 * This is an original work by Forest Belt and is placed in the
 * public domain.
 *
 * Modification history:
 * ---------------------
 *
 *    Rev 1.2   15 Aug 1991 23:02:50   GLENN
 * Forest Belt proofread/edited/cleaned up doc
 *
 *    Rev 1.1   14 Jun 1991 19:50:58   GLENN
 * Minor edit to file header
 *
 *    Rev 1.0   01 Apr 1991 01:00:40   GLENN
 * Nanforum Toolkit
 *
 */


/*  $DOC$
 *  $FUNCNAME$
 *     FT_BITCLR()
 *  $CATEGORY$
 *     String
 *  $ONELINER$
 *     Clear (reset) selected bit in a byte
 *  $SYNTAX$
 *     FT_BITCLR( <cByte>, <nBitPos> ) -> cByte
 *  $ARGUMENTS$
 *     <cByte> is a character from CHR(0) to CHR(255).
 *
 *     <nBitPos> is a number from 0 to 7 conforming to standard
 *     right-to-left bit numbering convention and representing the
 *     position of the bit within the byte.
 *  $RETURNS$
 *     Returns new byte, with designated bit cleared (reset).
 *     If parameters are faulty, returns NIL.
 *  $DESCRIPTION$
 *     In effect, ANDs argument byte with a byte that has all bits set except
 *     the target bit.  If bit is already clear (0), it remains clear.
 *     Note: Calls FT_ISBIT() which is also in this Library.
 *
 *     This function is presented to illustrate that bit-wise operations
 *     are possible with Clipper code.  For greater speed, write .C or
 *     .ASM versions and use the Clipper Extend system.
 *  $EXAMPLES$
 *     This code would clear bit 4 in a byte represented by CHR(115):
 *
 *       cNewByte := FT_BITCLR( CHR(115), 4 )
 *       ? ASC( cNewbyte )         // result: 99
 *       ? cNewByte                // result: 'c'
 *
 *     This code would clear bit 5 in the byte represented by letter 'A':
 *
 *       FT_BITCLR( 'A', 5 )       // result: 'A', since
 *                                 // bit 5 already clear
 *
 *     For a demonstration of Clipper bit manipulations, compile and
 *     link the program BITTEST.PRG in the Nanforum Toolkit source code.
 *  $SEEALSO$
 *     FT_BITSET() FT_ISBIT()
 *  $END$
 */

FUNCTION FT_BITCLR( cInbyte, nBitpos )

   LOCAL cByte

   IF ValType( cInbyte ) != "C" .OR. ValType( nBitpos ) != "N"  // parameter check
      cByte := NIL
   ELSE
      IF ( nBitPos > 7 ) .OR. ( nBitPos < 0 ) .OR. ( nBitPos != Int( nBitPos ) )
         cByte := NIL
      ELSE
         cByte := iif( .NOT. FT_ISBIT( cInByte, nBitpos ), cInByte, ;
            Chr( Asc( cInByte ) - ( 2 ^ nBitpos ) ) )
      ENDIF
   ENDIF

   RETURN cByte


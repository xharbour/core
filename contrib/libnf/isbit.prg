/*
 * $Id: isbit.prg 9686 2012-09-16 08:14:35Z andijahja $
 */
/*
 * File......: ISBIT.PRG
 * Author....: Forest Belt, Computer Diagnostic Services, Inc.
 * CIS ID....: ?
 *
 * This is an original work by Forest Belt and is placed in the
 * public domain.
 *
 * Modification history:
 * ---------------------
 *
 *    Rev 1.2   15 Aug 1991 23:03:46   GLENN
 * Forest Belt proofread/edited/cleaned up doc
 *
 *    Rev 1.1   14 Jun 1991 19:52:02   GLENN
 * Minor edit to file header
 *
 *    Rev 1.0   01 Apr 1991 01:01:32   GLENN
 * Nanforum Toolkit
 *
 */


/*  $DOC$
 *  $FUNCNAME$
 *     FT_ISBIT()
 *  $CATEGORY$
 *     String
 *  $ONELINER$
 *     Test the status of an individual bit
 *  $SYNTAX$
 *     FT_ISBIT( <cByte>, <nBitPos> ) -> lResult
 *  $ARGUMENTS$
 *     <cByte> is a character from CHR(0) to CHR(255)
 *
 *     <nBitPos> is a number from 0 to 7 conforming to standard right-to-left
 *               bit-numbering convention and representing the position of the
 *               bit within the byte.
 *  $RETURNS$
 *     .T. if designated bit is set (1), .F. if not set (0), NIL if
 *      invalid parameters.
 *  $DESCRIPTION$
 *     Tests for status of any selected bit in the byte passed as a parameter.
 *     Byte must be presented in CHR() form, as a literal constant, or as the
 *     one-byte character result of an expression.
 *
 *     This function is presented to illustrate that bit-wise operations
 *     are possible with Clipper code.  For greater speed, write .C or
 *     .ASM versions and use the Clipper Extend system.
 *  $EXAMPLES$
 *     This code tests whether bit 3 is set in the byte represented by
 *     CHR(107):
 *
 *      lBitflag := FT_ISBIT(CHR(107), 3)
 *      ? lBitflag                  // result: .T.
 *
 *      This code tests whether bit 5 is set in the byte represented by ASCII
 *      65 (letter 'A')
 *
 *      ? FT_ISBIT('A', 5)          // result: .F.
 *
 *     For a demonstration of Clipper bit manipulations, compile and
 *     link the program BITTEST.PRG in the Nanforum Toolkit source code.
 *  $SEEALSO$
 *     FT_BITSET() FT_BITCLR()
 *  $END$
 */

FUNCTION FT_ISBIT( cInbyte, nBitPos )

   LOCAL lBitStat

   IF ValType( cInbyte ) != "C" .OR. ValType( nBitPos ) != "N"  // parameter check
      lBitStat := NIL
   ELSE
      IF ( nBitPos > 7 ) .OR. ( nBitPos < 0 ) .OR. ( nBitPos != Int( nBitPos ) )
         lBitStat := NIL
      ELSE
         lBitStat := Int( ( (Asc(cInByte ) * (2 ^ (7 - nBitPos ) ) ) % 256 ) / 128 ) == 1
      ENDIF
   ENDIF

   RETURN lBitStat


/*
 * $Id: hex2dec.prg 9686 2012-09-16 08:14:35Z andijahja $
 */
/*
 * File......: HEX2DEC.PRG
 * Author....: Robert A. DiFalco
 * CIS ID....: ?
 *
 * This is an original work by Robert DiFalco and is placed in the
 * public domain.
 *
 * Modification history:
 * ---------------------
 *
 *    Rev 1.3   17 Aug 1991 15:32:56   GLENN
 * Don Caton fixed some spelling errors in the doc
 *
 *    Rev 1.2   15 Aug 1991 23:03:42   GLENN
 * Forest Belt proofread/edited/cleaned up doc
 *
 *    Rev 1.1   14 Jun 1991 19:51:58   GLENN
 * Minor edit to file header
 *
 *    Rev 1.0   01 Apr 1991 01:01:28   GLENN
 * Nanforum Toolkit
 *
 */


/*
 * $DOC$
 * $FUNCNAME$
 *    FT_HEX2DEC()
 *  $CATEGORY$
 *    Conversion
 * $ONELINER$
 *    Convert a hex number to decimal
 * $SYNTAX$
 *    FT_HEX2DEC( <cHexNum> ) -> nDecNum
 * $ARGUMENTS$
 *    <cHexNum> is a character string representing a hex number.
 * $RETURNS$
 *    A decimal number.
 * $DESCRIPTION$
 *    Converts a hexadecimal number to a BASE 10 decimal number.
 *    Useful for using FT_INT86().
 * $EXAMPLES$
 *    FT_INT86( HEX2DEC( "21" ), aRegs )
 *
 *    Converts 21h, the Dos Interrupt, to its decimal equivalent,
 *    33, for use by FT_INT86().
 * $END$
 */

#define HEXTABLE "0123456789ABCDEF"

#ifdef FT_TEST

FUNCTION MAIN( cHexNum )

   QOut( FT_HEX2DEC( cHexNum ) )

   RETURN ( nil )

#endif

FUNCTION FT_HEX2DEC( cHexNum )

   LOCAL n, nDec := 0, nHexPower := 1

   FOR n := Len( cHexNum ) TO 1 step - 1
      nDec += ( At( subs( Upper(cHexNum ), n, 1 ), HEXTABLE ) - 1 ) * nHexPower
      nHexPower *= 16
   NEXT

   RETURN nDec

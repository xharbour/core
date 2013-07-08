/*
 * $Id: isshare.prg 9686 2012-09-16 08:14:35Z andijahja $
 */
/*
 * File......: ISSHARE.PRG
 * Author....: Glenn Scott (from Tom Leylan C source)
 * CIS ID....: ?
 *
 * This is an original work by tom leylan and is placed in the
 * public domain.
 *
 * Modification history:
 * ---------------------
 *
 *    Rev 1.3   15 Aug 1991 23:03:48   GLENN
 * Forest Belt proofread/edited/cleaned up doc
 *
 *    Rev 1.2   14 Jun 1991 19:52:06   GLENN
 * Minor edit to file header
 *
 *    Rev 1.1   12 Jun 1991 02:14:56   GLENN
 * Documentation adjustment and checking ft_int86() call for compatibility
 * with new return value.
 *
 *    Rev 1.0   01 Apr 1991 01:01:34   GLENN
 * Nanforum Toolkit
 *
 */


/*  $DOC$
 *  $FUNCNAME$
 *      FT_ISSHARE()
 *  $CATEGORY$
 *      DOS/BIOS
 *  $ONELINER$
 *      Determine if DOS "Share" is installed
 *  $SYNTAX$
 *      FT_ISSHARE() -> nRetCode
 *  $ARGUMENTS$
 *      None
 *  $RETURNS$
 *      nRetcode will be set as follows on exit:
 *
 *          0 if SHARE not loaded but ok to load
 *          1 if SHARE not loaded and not ok to load
 *        255 if SHARE loaded
 *  $DESCRIPTION$
 *      Uses DOS interrupt 2Fh (MultiPlex interrupt), service 10h
 *      to determine if DOS SHARE.COM is loaded.
 *  $EXAMPLES$
 *     IF FT_ISSHARE() != 255
 *        Qout("SHARE must be loaded!")
 *     ENDIF
 *  $SEEALSO$
 *     FT_INT86()
 *  $END$
 */

#include "ftint86.ch"

#ifdef FT_TEST

FUNCTION main()

   LOCAL nLoaded := ft_isshare()

   DO CASE
   CASE nLoaded == 0
      QOut( "Share not loaded, but ok to load" )
   CASE nLoaded == 1
      QOut( "Share not loaded, but NOT ok to load!" )
   CASE nLoaded == 255
      QOut( "Share is loaded!" )
   ENDCASE

   QOut( "Retcode: " + Str( nLoaded ) )

   RETURN nil

#endif

FUNCTION ft_isshare()
   /*
  local aRegs[ INT86_MAX_REGS ]          // Declare the register array

  aRegs[ AX ] := makehi(16)              // share service
  aRegs[ CX ] := 0                       // Specify file attribute

  FT_Int86( 47, aRegs)                   // multiplex interrupt


RETURN lowbyte( aRegs[AX] )
  */

   RETURN   _ft_isshare()


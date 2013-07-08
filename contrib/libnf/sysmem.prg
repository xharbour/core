/*
 * $Id: sysmem.prg 9686 2012-09-16 08:14:35Z andijahja $
 */
/*
 * File......: SYSMEM.PRG
 * Author....: Glenn Scott
 * CIS ID....: 71620,1521
 *
 * This is an original work by Glenn Scott and is placed in the
 * public domain.
 *
 * Modification history:
 * ---------------------
 *
 *    Rev 1.4   17 Aug 1991 15:46:10   GLENN
 * Don Caton fixed some spelling errors in the doc
 *
 *    Rev 1.3   15 Aug 1991 23:04:40   GLENN
 * Forest Belt proofread/edited/cleaned up doc
 *
 *    Rev 1.2   14 Jun 1991 19:53:04   GLENN
 * Minor edit to file header
 *
 *    Rev 1.1   12 Jun 1991 02:41:50   GLENN
 * Documentation mod and check for ft_int86() compatibility
 *
 *    Rev 1.0   01 Apr 1991 01:02:20   GLENN
 * Nanforum Toolkit
 *
 */


/*  $DOC$
 *  $FUNCNAME$
 *     FT_SYSMEM()
 *  $CATEGORY$
 *     DOS/BIOS
 *  $ONELINER$
 *     Determine the amount of conventional memory installed
 *  $SYNTAX$
 *     FT_SYSMEM() -> nMemSize
 *  $ARGUMENTS$
 *     None
 *  $RETURNS$
 *     A numeric corresponding to the number of K memory.
 *  $DESCRIPTION$
 *     FT_SYSMEM() simply reports the amount of conventional memory
 *     (up to 640K) installed.
 *
 *     FT_SYSMEM() uses DOS interrupt 12h to get this information.
 *     For information, refer to Peter Norton's _Programmer's Guide
 *     to the IBM PC_ (Brady).
 *
 *  $EXAMPLES$
 *     QOut( "Conventional memory installed: " + Str( FT_SYSMEM() ) + "K" )
 *  $END$
 */

#include "ftint86.ch"

#define MEMSIZE    18

#ifdef FT_TEST

FUNCTION MAIN()

   QOut( "Conventional memory: " + Str( FT_SYSMEM() ) + "K installed" )

   RETURN ( nil )

#endif

FUNCTION FT_SYSMEM()

   LOCAL aRegs[ INT86_MAX_REGS ]

   aRegs[ AX ] := 0
   FT_INT86( MEMSIZE, aRegs )

   RETURN ( aRegs[ AX ] )


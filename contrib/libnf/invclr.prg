/*
 * $Id: invclr.prg 9686 2012-09-16 08:14:35Z andijahja $
 */
/*
 * File......: InvClr.Prg
 * Author....: David Husnian
 * CIS ID....: ?
 *
 * This is an original work by David Husnian and is placed in the
 * public domain.
 *
 * Modification history:
 * ---------------------
 *
 *    Rev 1.2   15 Aug 1991 23:03:44   GLENN
 * Forest Belt proofread/edited/cleaned up doc
 *
 *    Rev 1.1   14 Jun 1991 19:52:00   GLENN
 * Minor edit to file header
 *
 *    Rev 1.0   01 Apr 1991 01:01:30   GLENN
 * Nanforum Toolkit
 *
 */


/*  $DOC$
 *  $FUNCNAME$
 *     FT_INVCLR()
 *  $CATEGORY$
 *     Conversion
 *  $ONELINER$
 *     Get the inverse of a color
 *  $SYNTAX$
 *     FT_INVCLR( [ <cDsrdColor> ] ) -> cColor
 *  $ARGUMENTS$
 *     <cDsrdColor> is the color to get the inverse of.  Defaults to
 *     current color.
 *  $RETURNS$
 *     The inverse of the passed color.
 *  $DESCRIPTION$
 *     This function inverts a passed color (in the Clipper format: ??/??),
 *     e.g., "W/N" is converted to "N/W".
 *  $EXAMPLES$
 *     cInverse := FT_INVCLR()            // Get Inverse of Current Color
 *     cInvErr  := FT_INVCLR( cErrColor ) // Get Inverse of cErrorColor
 *  $END$
 */

#command    DEFAULT <Param1> TO <Def1> [, <ParamN> TO <DefN> ] ;
      => ;
      < Param1 > := IF( < Param1 > == NIL, < Def1 > , < Param1 > ) ;
      [; <ParamN> := IF(<ParamN> == NIL,<DefN>,<ParamN>)]

#define     NULL    ""

FUNCTION FT_INVCLR( cDsrdColor )

   LOCAL cBackground, ;                 // The Background Color, New Foreground
      cForeground, ;                 // The Foreground Color, New Background
      cModifiers                     // Any Color Modifiers (+*)

   DEFAULT cDsrdColor TO SetColor()
// Remove Anything Past 1st Color
   cDsrdColor := Left( cDsrdColor, At( ",", cDsrdColor + "," ) - 1 )

// Get Any Modifiers
   cModifiers := IF( "*" $ cDsrdColor, "*", NULL ) + ;
      IF( "+" $ cDsrdColor, "+", NULL )

// Separate the Fore/Background Colors
   cForeground := AllTrim( Left( cDsrdColor,   At("/", cDsrdColor ) - 1 ) )
   cBackground := AllTrim( SubStr( cDsrdColor, At("/", cDsrdColor ) + 1 ) )

   RETURN ( StrTran( StrTran(cBackground, "+" ), "*" ) + cModifiers + "/" + ;
      StrTran( StrTran( cForeground, "+" ), "*" ) )

/*
 * $Id: rand1.prg 9686 2012-09-16 08:14:35Z andijahja $
 */
/*
 * File......: RAND1.PRG
 * Author....: Gary Baren
 * CIS ID....: 75470,1027
 *
 * This is an original work by Gary Baren and is hereby placed in the
 * public domain.
 *
 * Modification history:
 * ---------------------
 *
 *    Rev 1.2   15 Aug 1991 23:04:30   GLENN
 * Forest Belt proofread/edited/cleaned up doc
 *
 *    Rev 1.1   14 Jun 1991 19:52:46   GLENN
 * Minor edit to file header
 *
 *    Rev 1.0   07 Jun 1991 23:03:38   GLENN
 * Initial revision.
 *
 */


/*  $DOC$
 *  $FUNCNAME$
 *     FT_RAND1()
 *  $CATEGORY$
 *     Math
 *  $ONELINER$
 *     Generate a random number
 *  $SYNTAX$
 *     FT_RAND1( <nMax> ) -> nRand
 *  $ARGUMENTS$
 *     <nMax>  Maximum limit of value to be produced.
 *  $RETURNS$
 *     nRand is a random number between 0 (inclusive) and <nMax> (exclusive).
 *  $DESCRIPTION$
 *     Generates a non-integer random number based on the Linear
 *     Congruential Method.
 *
 *     If you need a random number between 1 and <nMax> inclusive, INT()
 *     the result and add 1.
 *
 *     If you need a random number between 0 and <nMax> inclusive,
 *     then you should ROUND() the result.
 *  $EXAMPLES$
 *      nResult := INT( FT_RAND1(100) ) + 1    // 1 <= nResult <= 100
 *      nResult := ROUND( FT_RAND1(100), 0 )   // 0 <= nResult <= 100
 *      nResult := FT_RAND1( 1 )               // 0 <= nResult < 1
 *  $END$
 */

#ifdef FT_TEST

// Write 100 random numbers from 1 to 100 to stdout.
// Run it multiple times and redirect output to a file
// to check it

FUNCTION main()

   LOCAL x

   FOR x := 1 TO 100
      OutStd( Int( ft_rand1(100 ) ) )
      OutStd( Chr( 13 ) + Chr( 10 ) )
   NEXT

   RETURN nil

#endif

FUNCTION ft_rand1( nMax )

   STATIC nSeed
   LOCAL m := 100000000, b := 31415621

   nSeed := iif( nSeed == NIL, Seconds(), nSeed )   // init_seed()

   RETURN( nMax * ( ( nSeed := Mod( nSeed * b + 1, m ) ) / m ) )

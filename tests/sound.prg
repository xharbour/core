/*
 * $Id: sound.prg 9279 2011-02-14 18:06:32Z druzus $
 */

function main()
local start := seconds(), stop
   qout( "start   ", start )
/*
   According to the Clipper NG, the duration in ticks is truncated to the
   interger portion  ... Depending on the platform, xHarbour allows a finer
   resolution, but the minimum is 1 tick (for compatibility)
 */
   tone( 440, 9.1 )
   tone( 880, 9.1 )
   tone( 440, 9.1 )
   stop := seconds()
   qout( "stop    ", stop )
   qout( "duration", ( stop - start ), "(should be close to 1.5)" )
   //Inkey(0)
return nil

/*
 * $Id: test16.prg 9576 2012-07-17 16:41:57Z andijahja $
 */
/*
   test16.prg
   setting up valid RDD ....
*/
#include "sixapi.ch"
#define EOL chr(10)
#command ? => outstd(EOL)
#command ? <xx,...> => outstd(<xx>, EOL)
 
PROCEDURE MAIN( x )

   LOCAL e

   ?
   ? '================================='
   ? 'These are the valid RDD options :'
   ? '---------------------------------'
   ? 'This is the default driver :', sx_RDDSetDefault()
   ? 'Setting RDD to SDENTX     ', ', Before :', sx_RDDSetDefault( "SDENTX" ), ', Now :', sx_RDDSetDefault()
   ? 'Setting RDD to SDENSX     ', ', Before :', sx_RDDSetDefault( "SDENSX" ), ', Now :', sx_RDDSetDefault()
   ? 'Setting RDD to SDEFOX     ', ', Before :', sx_RDDSetDefault( "SDEFOX" ), ', Now :', sx_RDDSetDefault()
   ? 'Setting RDD to SDENSXDBT  ', ', Before :', sx_RDDSetDefault( "SDENSXDBT" ), ', Now :', sx_RDDSetDefault()
   ? 'This is now the default driver :', sx_RDDSetDefault()
   // The next will cause an Internal Error
   ? sx_RDDSetDefault( "SIXCDX" )

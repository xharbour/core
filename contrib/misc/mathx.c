/*
 * $Id: mathx.c 9598 2012-07-26 23:30:29Z andijahja $
 */

#include <math.h>

#include "hbapi.h"

#ifndef M_PI_2
   #define M_PI_2      1.57079632679489661923
#endif

HB_FUNC_EXTERN( CEILING ); /* from CT library */

HB_FUNC( FRACTION )
{
   if( ISNUM( 1 ) )
      hb_retnd( hb_parnd( 1 ) - hb_parnl( 1 ) );
   else
      /* TODO: Error or return NAN */
      hb_retnd( 0 );
}

HB_FUNC( CEIL )
{
   HB_FUNC_EXEC( CEILING );
}

#if 0

HB_FUNC( ACOS )
{
   if( hb_pcount() > 0 )
   {
      double dNumber = hb_parnd(1);

      if( dNumber >= -1 && dNumber <= 1  )
         hb_retnd( acos(dNumber) );
      else
         /* TODO: Error or return NAN */
         hb_retnd(0);
   }
   else
      /* TODO: Error or return NAN */
      hb_retnd(0);
}

HB_FUNC( ASIN )
{
   if( hb_pcount() > 0 )
   {
      double dNumber = hb_parnd(1);

      if( dNumber >= -1 && dNumber <= 1  )
         hb_retnd( asin(dNumber) );
      else
         /* TODO: Error or return NAN */
         hb_retnd(0);
   }
   else
      /* TODO: Error or return NAN */
      hb_retnd(0);
}

HB_FUNC( ATAN )
{
   if( hb_pcount() > 0 )
   {
      double dNumber = hb_parnd(1);

      if( dNumber >= -M_PI_2 && dNumber <= M_PI_2  )
         hb_retnd( atan(dNumber) );
      else
         /* TODO: Error or return NAN */
         hb_retnd(0);
   }
   else
      /* TODO: Error or return NAN */
      hb_retnd(0);
}

HB_FUNC( COS )
{
   if( hb_pcount() > 0 )
   {
      hb_retnd( cos( hb_parnd(1) ) );
   }
   else
      /* TODO: Error or return NAN */
      hb_retnd(0);
}

HB_FUNC( COSH )
{
   if( hb_pcount() > 0 )
   {
      hb_retnd( cosh( hb_parnd(1) ) );
   }
   else
      /* TODO: Error or return NAN */
      hb_retnd(0);
}

HB_FUNC( LOG10 )
{
   if( hb_pcount() > 0 )
   {
      hb_retnd( log10( hb_parnd(1) ) );
   }
   else
      /* TODO: Error or return NAN */
      hb_retnd(0);
}

HB_FUNC( SIN )
{
   if( hb_pcount() > 0 )
   {
      hb_retnd( sin( hb_parnd(1) ) );
   }
   else
      /* TODO: Error or return NAN */
      hb_retnd(0);
}

HB_FUNC( SINH )
{
   if( hb_pcount() > 0 )
   {
      hb_retnd( sinh( hb_parnd(1) ) );
   }
   else
      /* TODO: Error or return NAN */
      hb_retnd(0);
}

HB_FUNC( TAN )
{
   if( hb_pcount() > 0 )
   {
      hb_retnd( tan( hb_parnd(1) ) );
   }
   else
      /* TODO: Error or return NAN */
      hb_retnd(0);
}

HB_FUNC( TANH )
{
   if( hb_pcount() > 0 )
   {
      hb_retnd( tanh( hb_parnd(1) ) );
   }
   else
      /* TODO: Error or return NAN */
      hb_retnd(0);
}

HB_FUNC( PI )
{
   hb_retnd( 3.141592653589793 );
}

HB_FUNC( CEIL )
{
   if( ISNUM( 1 ) )
      hb_retnd( ceil( hb_parnd( 1 ) ) );
   else
      /* TODO: Error or return NAN */
      hb_retnd( 0 );
}

HB_FUNC( FLOOR )
{
   if( ISNUM( 1 ) )
      hb_retnd( floor( hb_parnd( 1 ) ) );
   else
      /* TODO: Error or return NAN */
      hb_retnd( 0 );
}

#endif

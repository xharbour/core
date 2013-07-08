/*
 * $Id: video.c 9692 2012-09-16 11:25:59Z andijahja $
 *
 * xHarbour Project source code:
 *
 * CT3 video functions
 *
 * CHARPIX, SETFONT, VGAPALETTE, VIDEOTYPE
 * Copyright 2004 Phil Krylov <phil@newstar.rinet.ru>
 *
 * www - http://www.xharbour.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307 USA (or visit the web site http://www.gnu.org/).
 *
 * As a special exception, the Harbour Project gives permission for
 * additional uses of the text contained in its release of Harbour.
 *
 * The exception is that, if you link the Harbour libraries with other
 * files to produce an executable, this does not by itself cause the
 * resulting executable to be covered by the GNU General Public License.
 * Your use of that executable is in no way restricted on account of
 * linking the Harbour library code into it.
 *
 * This exception does not however invalidate any other reasons why
 * the executable file might be covered by the GNU General Public License.
 *
 * This exception applies only to the code released by the Harbour
 * Project under the name Harbour.  If you copy code from other
 * Harbour Project or Free Software Foundation releases into a copy of
 * Harbour, as the General Public License permits, the exception does
 * not apply to the code that you add in this way.  To avoid misleading
 * anyone as to the status of such modified files, you must delete
 * this exception notice from them.
 *
 * If you write modifications of your own for Harbour, it is your choice
 * whether to permit this exception to apply to your modifications.
 * If you do not wish that, delete this exception notice.
 *
 *
 * See doc/license.txt for licensing terms.
 *
 */

#include "hbapi.h"

#if defined( HB_OS_DOS )

#if defined( __DJGPP__ )
#include <dpmi.h>
#include <go32.h>
#include <pc.h>
#include <sys/farptr.h>
#endif

#include "ctvideo.ch"

HB_FUNC( CHARPIX )
{
#ifdef __DJGPP__
   hb_retni( _farpeekw( _dos_ds, 0x485 ) );
#endif
}

HB_FUNC( SETFONT )
{
   char *   font     = hb_parcx( 1 );
   int      len      = hb_parclen( 1 );
   int      area     = hb_parni( 2 );
   int      offset   = 0;
   int      count    = 256;
   int      height   = 16;

   if( ! area )
      area = 1;
   if( ISNUM( 3 ) )
      offset = hb_parni( 3 );
   if( ISNUM( 4 ) )
      count = hb_parni( 4 );
   if( ISLOG( 3 ) )
      if( hb_parl( 3 ) && count != 0 )
         height = len / count;

#ifdef __DJGPP__
   {
      __dpmi_regs r;

      r.x.ax   = 0x1110; /* Load user-defined text-mode display font */
      r.h.bl   = area - 1;
      r.h.bh   = height;
      r.x.cx   = count;
      r.x.dx   = offset;
      r.x.es   = __tb >> 4;
      r.x.bp   = __tb & 0xF;
      dosmemput( font, len, __tb );
      __dpmi_int( 0x10, &r );
   }
#endif

   hb_retni( 0 );
}

HB_FUNC( VGAPALETTE )
{
   char *   color_string;
   char     red, green, blue;
   char     attr = 0;

   if( hb_pcount() < 4 )
   {
      /* Resetting palette registers to default values is not supported yet */
      hb_retl( FALSE );
      return;
   }
   if( ISNUM( 1 ) && hb_parni( 1 ) < 16 )
   {
      attr = hb_parni( 1 );
   }
   else if( ISCHAR( 1 ) )
   {
      char * s;

      color_string = hb_parcx( 1 );
      for( s = color_string; *s; s++ )
      {
         switch( *s )
         {
            case 'N':
            case 'n': attr |= 0; break;
            case 'B':
            case 'b': attr |= 1; break;
            case 'G':
            case 'g': attr |= 2; break;
            case 'R':
            case 'r': attr |= 4; break;
            case 'W':
            case 'w': attr |= 7; break;
            case '+': attr |= 8; break;
            case 'U':
            case 'u':
            case 'I':
            case 'i':
            case 'X':
            case 'x':
               /* these seem to be used only in mono */
               break;
            default:
               hb_retl( FALSE );
               return;
         }
      }
   }
   else
   {
      /* An invalid argument */
      hb_retl( FALSE );
      return;
   }

   red   = hb_parni( 2 );
   green = hb_parni( 3 );
   blue  = hb_parni( 4 );

#ifdef __DJGPP__
   {
      __dpmi_regs r;
      int         iflag;

      /* Get palette register for this attribute to BH using BIOS -
       * I couldn't manage to get it through ports */
      r.x.ax   = 0x1007;
      r.h.bl   = attr;
      __dpmi_int( 0x10, &r );

      iflag    = __dpmi_get_and_disable_virtual_interrupt_state();

      /* Wait for vertical retrace (for old VGA cards) */
      while( inportb( 0x3DA ) & 8 )
         ;
      while( ! ( inportb( 0x3DA ) & 8 ) )
         ;

      outportb( 0x3C8, r.h.bh );
      outportb( 0x3C9, red );
      outportb( 0x3C9, green );
      outportb( 0x3C9, blue );

      if( iflag )
         __dpmi_get_and_enable_virtual_interrupt_state();
   }
   hb_retl( TRUE );
#else
   hb_retl( FALSE );
#endif
}

HB_FUNC( VIDEOTYPE )
{
#if defined( __DJGPP__ )
   __dpmi_regs r;

   r.h.ah   = 0x12;  /* Alternate Select */
   r.h.bl   = 0x10;  /* Get EGA info */
   __dpmi_int( 0x10, &r );
   if( r.h.bl == 0x10 )
   {
      /* CGA/HGC/MDA */
      hb_retni( VCARD_MONOCHROME );
   }
   else
   {
      /* EGA/VGA */
      r.x.ax = 0x1A00;
      __dpmi_int( 0x10, &r );
      if( r.h.al == 0x1A )
         hb_retni( VCARD_VGA );
      else
         hb_retni( VCARD_EGA );
   }
#endif
}

#endif /* HB_OS_DOS */

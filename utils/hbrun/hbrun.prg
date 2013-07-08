/*
 * $Id: hbrun.prg 9845 2012-12-01 03:29:25Z andijahja $
 */

/*
 * Harbour Project source code:
 * Standalone Harbour Portable Object file runner
 *
 * Copyright 1999 Ryszard Glab <rglab@imid.med.pl>
 * www - http://www.harbour-project.org
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
 */

#include "hbextern.ch"

/* NOTE: Undocumented CA-Clipper _APPMAIN is used instead of Main to avoid
         collision with user function in HRB file with that name. [ckedem]
*/
FUNCTION _APPMAIN( cHRBFile, cPar1, cPar2, cPar3, cPar4, cPar5, cPar6, cPar7, cPar8, cPar9 )
   LOCAL xRetVal

   IF Empty( cHRBFile )
      OutStd( "xHarbour Runner" + HB_OSNewLine() +;
              "Copyright 1999-2013, http://www.harbour-project.org, http://www.xharbour.org" + HB_OSNewLine() +;
              HB_OSNewLine() +;
              "Syntax:  hbrun <hrbfile[.hrb]> [parameters]" + HB_OSNewLine() + ;
              HB_OSNewLine() +;
              "Note:  Linked with " + Version() + HB_OSNewLine() )
      ERRORLEVEL( 1 )
   ELSE
      xRetVal := __hrbRun( cHRBFile, cPar1, cPar2, cPar3, cPar4, cPar5, cPar6, cPar7, cPar8, cPar9 )
   ENDIF

   RETURN xRetVal

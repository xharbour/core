/*
 * $Id: pickfile.prg 9941 2013-03-25 17:42:55Z enricomaria $
 */

*+��������������������������������������������������������������������
*+
*+    Source Module => D:\SRC\PBMAKE\PICKFILE.PRG
*+
*+    PBMake is a Clipper, C, ASM, Xbase++ make engine.
*+    
*+    Copyright(C) 1996-1999 by Phil Barnett.
*+       
*+    This program is free software; you can redistribute it and/or modify it
*+    under the terms of the GNU General Public License as published by the
*+    Free Software Foundation; either version 2 of the License, or (at your
*+    option) any later version.
*+    
*+    This program is distributed in the hope that it will be useful, but
*+    WITHOUT ANY WARRANTY; without even the implied warranty of
*+    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*+    General Public License for more details.
*+    
*+    You should have received a copy of the GNU General Public License along
*+    with this program; if not, write to the Free Software Foundation, Inc.,
*+    675 Mass Ave, Cambridge, MA 02139, USA.
*+    
*+    You can contact me at:
*+    
*+    Phil Barnett
*+    Box 944
*+    Plymouth, Florida  32768
*+    
*+    or
*+    
*+    philb@iag.net
*+    
*+
*+    Functions: Function pickfile()
*+
*+    Reformatted by Click! 2.03 on Mar-30-1999 at 11:19 pm
*+
*+��������������������������������������������������������������������

/*
   PICKFILE.PRG

   Author     : Phil Barnett

   Written    : 18-Apr-93

   Function   : PICKFILE()

   Purpose    : Generic file picking routine that pops a picklist of files.

   Syntax     : PICKFILE( [FILESPEC] )

   Parameters : FILESPEC is a DOS filename. Wildcards permitted

   Returns    : Character file name of selected file or "" if nothing picked.

   Example    : yourfile := pickfile( '*.dbf' )

                if empty(yourfile)
                  ? 'You pressed Escape or No Matching File'
                else
                  ? 'The file you selected is: '+yourfile
                endif

   Released to Public Domain by Author.             

*/

#include "common.ch"
#include "box.ch"
#ifndef __HARBOUR__
#include 'hbclip.ch'
#endif
#include "directry.ch"

*+��������������������������������������������������������������������
*+
*+    Function pickfile()
*+
*+    Called from ( makelink.prg )   1 - function makelink()
*+                ( pbinit.prg   )   1 - procedure pbinit()
*+
*+��������������������������������������������������������������������
*+
function pickfile( cFileSpec )

local cOldString := savescreen( 8, 19, 16, 61 )
local aFiles   as array  := directory( cFileSpec )
local aPickList := {}
local cRetVal    := ''
local sel

default cFileSpec to '*.*'

dispbox( 8, 19, 16, 61, B_SINGLE + " ", "+W/R" )

if len( aFiles ) > 0

   aeval( aFiles, { | xx | aadd( aPickList, ;
                    pad( xx[ F_NAME ], 13 ) + ;
                    str( xx[ F_SIZE ], 8 ) + '  ' + ;
                    dtoc( xx[ F_DATE ] ) + '  ' + ;
                    xx[ F_TIME ] ) } )

   sel := achoice( 9, 20, 15, 60, aPickList )

   cRetVal := iif( lastkey() == 27, '', aFiles[ sel, 1 ] )

else

   achoice( 9, 20, 15, 60, { "No files match " + cFileSpec } )

endif

restscreen( 8, 19, 16, 61, cOldString )

return cRetVal

function pickafile( afiles )

local cOldString := savescreen( 8, 19, 16, 61 )
//local aPickList  := {}
local cRetVal    := ''
local sel

/*default cFileSpec to '*.*'*/

dispbox( 8, 19, 16, 61, B_SINGLE + " ", "+W/R" )

if len( aFiles ) > 0

/*   aeval( aFiles, { | xx | aadd( aPickList, ;
          xx)} )*/

   sel := achoice( 9, 20, 15, 60, aFiles )

   cRetVal := iif( lastkey() == 27, '', substr(aFiles[ sel ],1,at(' ',afiles[sel])-1) )

endif

restscreen( 8, 19, 16, 61, cOldString )

return cRetVal


*+ EOF: PICKFILE.PRG

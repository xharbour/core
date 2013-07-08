/*
 * $Id: tindex.prg 9576 2012-07-17 16:41:57Z andijahja $
 */

/*
 * SixAPI Project source code:
 *
 * Copyright 2010 Andi Jahja <xharbour@telkom.net.id>
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
 */

#include "hbclass.ch"

CLASS TIndex

   DATA cIndexFile     /* Index File Name */
   DATA cExpression    /* Index Expression */
   DATA iMode          /* iOption: IDX_NONE=0 IDX_UNIQUE=1 IDX_EMPTY=2  */
   DATA lDescending    /* BOOL lDescending */
   DATA cCondition     /* FOR Condition */
   DATA	cAlias

   METHOD New( cIndexFile, cExpression, cMode, lDescending, cCondition, cAlias )
   METHOD Create()

ENDCLASS

METHOD New( cIndexFile, cExpression, iMode, lDescending, cCondition, cAlias ) CLASS TIndex

   ::cIndexFile   := cIndexFile     /* Index File Name */
   ::cExpression  := cExpression    /* Index Expression */
   ::iMode        := iMode          /* iOption: IDX_NONE=0 IDX_UNIQUE=1 IDX_EMPTY=2  */
   ::lDescending  := lDescending    /* BOOL lDescending */
   ::cCondition   := cCondition     /* FOR Condition */
   ::cAlias       := cAlias         /* Alias */

   Return Self

METHOD Create() CLASS TIndex

   // This is For Single TAG Index File : DBFNTX
   // DBFCDX and DBFNSX Should Use TAG
   return sx_Index( ::cIndexFile, ::cExpression, ::iMode, ::lDescending, ::cCondition, ::cAlias )

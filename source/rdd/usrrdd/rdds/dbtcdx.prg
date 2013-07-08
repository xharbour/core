/*
 * $Id: dbtcdx.prg 9723 2012-10-02 09:18:08Z andijahja $
 */

/*
 * Harbour Project source code:
 *    DBTCDX RDD
 *
 * Copyright 2006 Przemyslaw Czerpak <druzus / at / priv.onet.pl>
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

#include "rddsys.ch"
#include "usrrdd.ch"
#include "dbinfo.ch"

/*
 * DBTCDX RDD
 * Very simple RDD which inherits from DBFCDX and
 * set default memo type to DBT
 */

   /* Force linking DBFCDX and DBFFPT from which our RDD inherits */
   REQUEST DBFCDX
   REQUEST DBFFPT

   /* Announce our RDD for foreign REQUESTs */
   ANNOUNCE DBTCDX

PROC DBTCDX(); RETURN

FUNCTION DBTCDX_GETFUNCTABLE( pFuncCount, pFuncTable, pSuperTable, nRddID )

   RETURN USRRDD_GETFUNCTABLE( pFuncCount, pFuncTable, pSuperTable, nRddID, ;
      "DBFCDX", {} ) /* We are inheriting from DBFCDX */

   INIT PROCEDURE DBTCDX_INIT()
   rddRegister( "DBTCDX", RDT_FULL )
   rddInfo( RDDI_MEMOTYPE, DB_MEMO_DBT, "DBTCDX" )

   RETURN

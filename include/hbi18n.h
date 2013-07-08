/*
 * $Id: hbi18n.h 9279 2011-02-14 18:06:32Z druzus $
 */

/*
 * xHarbour Project source code:
 * Internationalization routines
 *
 * Copyright 2003 Giancarlo Niccolai <giancarlo@niccolai.ws>
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
 */

#ifndef HB_I18N_H
#define HB_I18N_H

#include "hbapifs.h"

#define HB_DEFAULT_I18N_PATH     "i18n"
#define HB_I18N_LIST_EXT         "hil"
#define HB_I18N_TAB_EXT          "hit"
#define HB_INTERNATIONAL_CODE    "en_US"
#define HB_INTERNATIONAL_NAME    "American English"
#define HB_I18N_CODELEN          15
#define HB_I18N_NAMELEN          50

typedef struct tag_hb_i18n_tab_header
{
   char signature[5];
   char author[50];
   char language[50];
   char language_int[50];
   char language_code[10];
   int entries;
} HB_I18N_TAB_HEADER;

HB_EXTERN_BEGIN

/** VM Interface **/
BOOL hb_i18nInit( char *i18n_dir, char *language );
void hb_i18nExit( void );

/** Low level API routines **/
PHB_ITEM hb_i18n_scan_table( PHB_ITEM pStr, PHB_ITEM pTable, ULONG * ulIndex );
char * hb_i18n_build_table_filename( char *i18n_dir, char *language );
PHB_ITEM hb_i18n_read_table_header( FHANDLE handle );
BOOL hb_i18n_write_table_header( FHANDLE handle, PHB_ITEM pHeader );
PHB_ITEM hb_i18n_read_table( FHANDLE handle, int count );
PHB_ITEM hb_i18n_read_memory_table( BYTE* pData, int count );
BOOL hb_i18n_write_table( FHANDLE handle, PHB_ITEM pTable );
BOOL hb_i18n_load_language( char *language );

HB_EXTERN_END

#endif

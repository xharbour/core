/*
 * $Id: hbapilng.h 9279 2011-02-14 18:06:32Z druzus $
 */

/*
 * Harbour Project source code:
 * Header file for the Language API
 *
 * Copyright 1999-2001 Viktor Szakats <viktor.szakats@syenar.hu>
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

#ifndef HB_APILNG_H_
#define HB_APILNG_H_

#include "hbdefs.h"
#include "hbvmpub.h"
#include "hbinit.h"

#include "hblang.ch" /* Base values for the unified language item table */

HB_EXTERN_BEGIN

/* This hack is needed to force preprocessing if id is also a macro */
#define HB_LANG_REQUEST( id )           HB_LANG_REQUEST_( id )
#define HB_LANG_REQUEST_( id )          HB_FUNC_EXTERN( HB_LANG_##id ); \
                                        void hb_lang_ForceLink( void ) \
                                        { \
                                           HB_FUNC_EXEC( HB_LANG_##id ); \
                                        }

/* Macro to publish a specific language module, for both C and Harbour level */
#define HB_LANG_ANNOUNCE( id )          HB_LANG_ANNOUNCE_( id )
#define HB_LANG_ANNOUNCE_( id )         HB_FUNC( HB_LANG_##id ) {}

typedef struct _HB_LANG
{
   const void * pItemList[ HB_LANG_ITEM_MAX_ ];
} HB_LANG, * PHB_LANG, * HB_LANG_PTR;

/* Supported language list management */

extern HB_EXPORT BOOL      hb_langRegister         ( PHB_LANG lang );
extern HB_EXPORT BOOL      hb_langDeRegister       ( char * pszID );
extern HB_EXPORT PHB_LANG  hb_langFind             ( const char * pszID );

/* Default language selection and data query */

extern HB_EXPORT PHB_LANG  hb_langSelect           ( PHB_LANG lang );
extern HB_EXPORT char *    hb_langSelectID         ( const char * pszID );
extern HB_EXPORT char *    hb_langDGetItem         ( int iIndex );
extern HB_EXPORT char *    hb_langID               ( void );
extern HB_EXPORT char *    hb_langName             ( void );

/* Compatibility interface */

extern HB_EXPORT char *    hb_langDGetErrorDesc    ( ULONG ulIndex );

HB_EXTERN_END

#endif /* HB_APILNG_H_ */

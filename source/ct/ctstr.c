/*
 * $Id: ctstr.c 9784 2012-10-23 21:26:49Z andijahja $
 */

/*
 * Harbour Project source code:
 *   internal and switch functions for CT3 string functions
 *
 * Copyright 2001 IntTec GmbH, Neunlindenstr 32, 79106 Freiburg, Germany
 *        Author: Martin Vogel <vogel@inttec.de>
 *
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

#include "ct.h"

/* -------------- */
/* initialization */
/* -------------- */
int ct_str_init( void )
{
   HB_TRACE( HB_TR_DEBUG, ( "ctstr_init()" ) );
   return 1;
}

int ct_str_exit( void )
{
   HB_TRACE( HB_TR_DEBUG, ( "ctstr_exit()" ) );
   return 1;
}

/* -------------------------- */
/* search for exact substring */
/* -------------------------- */
const char * ct_at_exact_forward( const char * pcString, HB_SIZE sStrLen,
                                  const char * pcMatch, HB_SIZE sMatchLen,
                                  HB_SIZE * psMatchStrLen )
{
   HB_SIZE sPos;

   HB_TRACE( HB_TR_DEBUG, ( "ct_at_exact_forward (\"%s\", %u, \"%s\", %u, %p)",
                            pcString, sStrLen, pcMatch, sMatchLen, psMatchStrLen ) );

   if( ( sMatchLen == 0 ) || ( sStrLen < sMatchLen ) )
      return NULL;

   sPos = hb_strAt( pcMatch, sMatchLen, pcString, sStrLen );
   if( sPos == 0 )
   {
      return NULL;
   }
   else
   {
      if( psMatchStrLen != NULL )
         *psMatchStrLen = sMatchLen;
      return pcString + sPos - 1;
   }

}

/* ------------------------------------------------ */
/* search for exact substring in backward direction */
/* ------------------------------------------------ */
const char * ct_at_exact_backward( const char * pcString, HB_SIZE sStrLen,
                                   const char * pcMatch, HB_SIZE sMatchLen,
                                   HB_SIZE * psMatchStrLen )
{
   HB_SIZE         sIndex;
   const char *   pcRet;

   HB_TRACE( HB_TR_DEBUG, ( "ct_at_exact_backward (\"%s\", %u, \"%s\", %u, %p)",
                            pcString, sStrLen, pcMatch, sMatchLen, psMatchStrLen ) );

   if( ( sMatchLen == 0 ) || ( sStrLen < sMatchLen ) )
      return NULL;

   for( pcRet = pcString + sStrLen - sMatchLen; pcRet >= pcString; pcRet-- )
   {
      for( sIndex = 0; sIndex < sMatchLen; sIndex++ )
         if( *( pcRet + sIndex ) != *( pcMatch + sIndex ) )
            break;
      if( sIndex == sMatchLen )
      {
         /* last match found */
         if( psMatchStrLen != NULL )
            *psMatchStrLen = sMatchLen;
         return pcRet;
      }
   }

   return NULL;
}

/* ----------------------------------- */
/* search for substring using wildcard */
/* ----------------------------------- */
const char * ct_at_wildcard_forward( const char * pcString, HB_SIZE sStrLen,
                                     const char * pcMatch, HB_SIZE sMatchLen,
                                     char cWildCard, HB_SIZE * psMatchStrLen )
{
   HB_SIZE         sIndex;
   const char *   pcRet, * pcStop;

   HB_TRACE( HB_TR_DEBUG, ( "ct_at_wildcard_forward (\"%s\", %u, \"%s\", %u, \'%c\', %p)",
                            pcString, sStrLen, pcMatch, sMatchLen, cWildCard, psMatchStrLen ) );

   if( ( sMatchLen == 0 ) || ( sStrLen < sMatchLen ) )
      return NULL;

   pcStop = pcString + sStrLen - sMatchLen;
   for( pcRet = pcString; pcRet < pcStop; pcRet++ )
   {
      for( sIndex = 0; sIndex < sMatchLen; sIndex++ )
      {
         char c = *( pcMatch + sIndex );
         if( ( c != cWildCard ) && ( c != *( pcRet + sIndex ) ) )
            break;
      }
      if( sIndex == sMatchLen )
      {
         if( psMatchStrLen != NULL )
            *psMatchStrLen = sMatchLen;
         return pcRet;
      }
   }

   return NULL;
}

/* --------------------------------------------------------- */
/* search for substring using wildcard in backward direction */
/* --------------------------------------------------------- */
const char * ct_at_wildcard_backward( const char * pcString, HB_SIZE sStrLen,
                                      const char * pcMatch, HB_SIZE sMatchLen,
                                      char cWildCard, HB_SIZE * psMatchStrLen )
{
   HB_SIZE         sIndex;
   const char *   pcRet;

   HB_TRACE( HB_TR_DEBUG, ( "ct_at_wildcard_backward (\"%s\", %u, \"%s\", %u, \'%c\', %p)",
                            pcString, sStrLen, pcMatch, sMatchLen, cWildCard, psMatchStrLen ) );

   if( ( sMatchLen == 0 ) || ( sStrLen < sMatchLen ) )
      return NULL;

   for( pcRet = pcString + sStrLen - sMatchLen; pcRet >= pcString; pcRet-- )
   {
      for( sIndex = 0; sIndex < sMatchLen; sIndex++ )
      {
         char c = *( pcMatch + sIndex );
         if( ( c != cWildCard ) && ( c != *( pcRet + sIndex ) ) )
            break;
      }
      if( sIndex == sMatchLen )
      {
         /* last match found */
         if( psMatchStrLen != NULL )
            *psMatchStrLen = sMatchLen;
         return pcRet;
      }
   }

   return NULL;
}

/* ------------------------------- */
/* search for character from a set */
/* ------------------------------- */
const char * ct_at_charset_forward( const char * pcString, HB_SIZE sStrLen,
                                    const char * pcCharSet, HB_SIZE sCharSetLen,
                                    HB_SIZE * psMatchedCharPos )
{
   const char * pcRet, * pcSet, * pcStop1, * pcStop2;

   HB_TRACE( HB_TR_DEBUG, ( "ct_at_charset_forward (\"%s\", %u, \"%s\", %u, %p)",
                            pcString, sStrLen, pcCharSet, sCharSetLen, psMatchedCharPos ) );

   *( psMatchedCharPos ) = sCharSetLen;

   if( ( sCharSetLen == 0 ) || ( sStrLen == 0 ) )
      return NULL;

   pcStop1  = pcString + sStrLen;
   pcStop2  = pcCharSet + sCharSetLen;
   for( pcRet = pcString; pcRet < pcStop1; pcRet++ )
   {
      for( pcSet = pcCharSet; pcSet < pcStop2; pcSet++ )
         if( *pcSet == *pcRet )
         {
            if( psMatchedCharPos != NULL )
               *( psMatchedCharPos ) = pcSet - pcCharSet;
            return pcRet;
         }
   }

   return NULL;
}

/* ----------------------------------------------------- */
/* search for character from a set in backward direction */
/* ----------------------------------------------------- */
const char * ct_at_charset_backward( const char * pcString, HB_SIZE sStrLen,
                                     const char * pcCharSet, HB_SIZE sCharSetLen,
                                     HB_SIZE * psMatchedCharPos )
{
   const char * pcRet, * pcSet, * pcStop;

   HB_TRACE( HB_TR_DEBUG, ( "ct_at_charset_backward (\"%s\", %u, \"%s\", %u, %p)",
                            pcString, sStrLen, pcCharSet, sCharSetLen, psMatchedCharPos ) );

   *( psMatchedCharPos ) = sCharSetLen;

   if( ( sCharSetLen == 0 ) || ( sStrLen == 0 ) )
      return NULL;

   pcStop = pcCharSet + sCharSetLen;
   for( pcRet = pcString + sStrLen - 1; pcRet >= pcString; pcRet-- )
   {
      for( pcSet = pcCharSet; pcSet < pcStop; pcSet++ )
         if( *pcSet == *pcRet )
         {
            if( psMatchedCharPos != NULL )
               *( psMatchedCharPos ) = pcSet - pcCharSet;
            return pcRet;
         }
   }

   return NULL;
}

/*
 *  CSETREF() stuff
 */

static int siRefSwitch = 0; /* TODO: make this tread safe */

void ct_setref( int iNewSwitch )
{
   HB_TRACE( HB_TR_DEBUG, ( "ct_setref(%i)", iNewSwitch ) );
   siRefSwitch = iNewSwitch;
}

int ct_getref( void )
{
   HB_TRACE( HB_TR_DEBUG, ( "ct_getref()" ) );
   return siRefSwitch;
}

HB_FUNC( CSETREF )
{
   hb_retl( ct_getref() );

   if( ISLOG( 1 ) )
   {
      ct_setref( hb_parl( 1 ) );
   }
   else if( hb_pcount() > 0 )  /* 1 params, but is not logical ! */
   {
      int iArgErrorMode = ct_getargerrormode();
      if( iArgErrorMode != CT_ARGERR_IGNORE )
      {
         ct_error( ( USHORT ) iArgErrorMode, EG_ARG, CT_ERROR_CSETREF,
                   NULL, "CSETREF", 0, EF_CANDEFAULT, 1, hb_paramError( 1 ) );
      }
   }
}

/*
 * CSETATMUPA() stuff
 */

static int siAtMupaSwitch = 0; /* TODO: make this tread safe */

void ct_setatmupa( int iNewSwitch )
{
   HB_TRACE( HB_TR_DEBUG, ( "ct_setatmupa(%i)", iNewSwitch ) );
   siAtMupaSwitch = iNewSwitch;
}

int ct_getatmupa( void )
{
   HB_TRACE( HB_TR_DEBUG, ( "ct_getatmupa()" ) );
   return siAtMupaSwitch;
}

HB_FUNC( CSETATMUPA )
{
   hb_retl( ct_getatmupa() );

   if( ISLOG( 1 ) )
   {
      ct_setatmupa( hb_parl( 1 ) );
   }
   else if( hb_pcount() > 0 )  /* 1 params, but is not logical ! */
   {
      int iArgErrorMode = ct_getargerrormode();
      if( iArgErrorMode != CT_ARGERR_IGNORE )
      {
         ct_error( ( USHORT ) iArgErrorMode, EG_ARG, CT_ERROR_CSETATMUPA,
                   NULL, "CSETATMUPA", 0, EF_CANDEFAULT, 1, hb_paramError( 1 ) );
      }
   }
}

/*
 * SETATLIKE() stuff
 */

static int  siAtLikeMode   = 0;     /* TODO: make this tread safe */
static int  scAtLikeChar   = '?';   /* TODO: make this tread safe */

void ct_setatlike( int iNewMode )
{
   HB_TRACE( HB_TR_DEBUG, ( "ct_setatlike(%i)", iNewMode ) );
   siAtLikeMode = iNewMode;
}

int ct_getatlike( void )
{
   HB_TRACE( HB_TR_DEBUG, ( "ct_getatlike()" ) );
   return siAtLikeMode;
}

void ct_setatlikechar( char cNewChar )
{
   HB_TRACE( HB_TR_DEBUG, ( "ct_setatlikechar(\'%c\')", cNewChar ) );
   scAtLikeChar = cNewChar;
}

char ct_getatlikechar( void )
{
   HB_TRACE( HB_TR_DEBUG, ( "ct_getatlikechar()" ) );
   return ( char ) scAtLikeChar;
}

HB_FUNC( SETATLIKE )
{
   hb_retni( ct_getatlike() );

   /* set new mode if first parameter is CT_SETATLIKE_EXACT (==0)
                                      or CT_SETATLIKE_WILDCARD (==1) */
   if( ISNUM( 1 ) )
   {
      int iNewMode = hb_parni( 1 );
      if( ( iNewMode == CT_SETATLIKE_EXACT ) ||
          ( iNewMode == CT_SETATLIKE_WILDCARD ) )
      {
         ct_setatlike( iNewMode );
      }
      else
      {
         int iArgErrorMode = ct_getargerrormode();
         if( iArgErrorMode != CT_ARGERR_IGNORE )
         {
            ct_error( ( USHORT ) iArgErrorMode, EG_ARG, CT_ERROR_SETATLIKE,
                      NULL, "SETATLIKE", 0, EF_CANDEFAULT, 2,
                      hb_paramError( 1 ), hb_paramError( 2 ) );
         }
      }
   }

   /* set new wildcard character, if ISCHAR(2) but !ISBYREF(2) */
   if( ISCHAR( 2 ) )
   {
      if( ISBYREF( 2 ) )
      {
         /* new behaviour: store the current wildcard char in second parameter */
         char cResult;
         cResult = ct_getatlikechar();
         hb_storclen( &cResult, 1, 2 );
      }
      else
      {
         const char * pcNewChar = hb_parc( 2 );
         if( hb_parclen( 2 ) > 0 )
            ct_setatlikechar( *pcNewChar );
      }
   }
   else if( hb_pcount() > 1 )  /* more than 2 params, but second is not string ! */
   {
      int iArgErrorMode = ct_getargerrormode();
      if( iArgErrorMode != CT_ARGERR_IGNORE )
      {
         ct_error( ( USHORT ) iArgErrorMode, EG_ARG, CT_ERROR_SETATLIKE,
                   NULL, "SETATLIKE", 0, EF_CANDEFAULT, 2,
                   hb_paramError( 1 ), hb_paramError( 2 ) );
      }
   }
}


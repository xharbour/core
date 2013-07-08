/*
 * $Id: zipmemfile.cpp 9685 2012-09-16 05:42:35Z andijahja $
 */

////////////////////////////////////////////////////////////////////////////////
// $Workfile: ZipMemFile.cpp $
// $Archive: /ZipArchive/ZipMemFile.cpp $
// $Date: 2012-09-16 02:42:35 -0300 (dom, 16 set 2012) $ $Author: andijahja $
////////////////////////////////////////////////////////////////////////////////
// This source file is part of the ZipArchive library source distribution and
// is Copyright 2000-2003 by Tadeusz Dracz (http://www.artpol-software.com/)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// For the licensing details see the file License.txt
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "zipmemfile.h"
#include "zipexception.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

void CZipMemFile::Grow( size_t nGrowTo )
{
   if( m_nBufSize < ( UINT ) nGrowTo )
   {
      if( m_nGrowBy == 0 )
         CZipException::Throw( CZipException::memError );
      size_t   nNewSize = m_nBufSize;
      while( nNewSize < nGrowTo )
         nNewSize += m_nGrowBy;
      BYTE *   lpNew;
      if( m_lpBuf )
         lpNew = ( BYTE * ) hb_xrealloc( ( void * ) m_lpBuf, nNewSize );
      else
         lpNew = ( BYTE * ) hb_xalloc( nNewSize );

      if( ! lpNew )
         CZipException::Throw( CZipException::memError );
      m_nBufSize  = nNewSize;
      m_lpBuf     = lpNew;
   }
}

void CZipMemFile::SetLength( ZIP_ULONGLONG nNewLen )
{
   if( m_nBufSize < ( UINT ) nNewLen )
      Grow( ( size_t ) nNewLen );
   else
      m_nPos = ( size_t ) nNewLen;
   m_nDataSize = ( size_t ) nNewLen;
}

UINT CZipMemFile::Read( void * lpBuf, UINT nCount )
{
   if( m_nPos > m_nDataSize )
      return 0;
   UINT nToRead = ( m_nPos + nCount > m_nDataSize ) ? m_nDataSize - m_nPos : nCount;
   memcpy( lpBuf, m_lpBuf + m_nPos, nToRead );
   m_nPos += nToRead;
   return nToRead;

}

void CZipMemFile::Write( const void * lpBuf, UINT nCount )
{
   if( ! nCount )
      return;

   if( m_nPos + nCount > m_nBufSize )
      Grow( m_nPos + nCount );
   memcpy( m_lpBuf + m_nPos, lpBuf, nCount );
   m_nPos += nCount;
   if( m_nPos > m_nDataSize )
      m_nDataSize = m_nPos;
}

ZIP_ULONGLONG CZipMemFile::Seek( ZIP_LONGLONG lOff, int nFrom )
{
   ZIP_ULONGLONG lNew = m_nPos;

   if( nFrom == CZipAbstractFile::begin )
      lNew = lOff;
   else if( nFrom == CZipAbstractFile::current )
      lNew += lOff;
   else if( nFrom == CZipAbstractFile::end )
      lNew = m_nDataSize + lOff;
   else
      return lNew;

   if( lNew < 0 )
      CZipException::Throw( CZipException::memError );

   m_nPos = ( size_t ) lNew;
   return lNew;
}

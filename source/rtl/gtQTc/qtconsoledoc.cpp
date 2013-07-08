/***************************************************************************
                          qtconsoledoc.cpp  -  description
                             -------------------
    begin                : dom nov 17 13:34:51 CET 2002
    copyright            : (C) 2002 by Giancarlo
    email                :
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "qtconsoledoc.h"
#include <iostream>

QTconsoleDoc::QTconsoleDoc()
{
   modified       = false;
   _rows          = DEFROWS;
   _cols          = DEFCOLS;
   // set initial cursor position
   cursX          = 0;
   cursY          = 0;
   _rectCommited  = true;
   _changing      = 0;
   int size = _cols * 2 * _rows;
   buffer         = new char[ size ];
   setAttrib( 0x07 );
}

QTconsoleDoc::~QTconsoleDoc()
{
   delete buffer;
}

void QTconsoleDoc::newDoc()
{
   // just a test
}

bool QTconsoleDoc::save()
{
   return true;
}

bool QTconsoleDoc::saveAs( const QString&filename )
{
   return true;
}

bool QTconsoleDoc::load( const QString&filename )
{
   emit documentChanged( QRect( 0, 0, _cols, _rows ) );

   return true;
}

bool QTconsoleDoc::isModified() const
{
   return modified;
}
/** Clears virtual screen with a given background color (default black ) */
void QTconsoleDoc::clearScr( unsigned char attrib )
{
   char *   size  = buffer + ( _rows * _cols * 2 );
   char *   pos   = buffer;

   while( pos < size )
   {
      *pos  = ( char ) attrib;
      pos++;
      *pos  = 0;
      pos++;
   }
   rectChanging( QRect( 0, 0, _cols, _rows ) );
   setModified();
}

void QTconsoleDoc::clearScr( unsigned char attrib, int left, int top, int width, int height )
{
   for( int i = 0; i < height; i++ )
   {
      char *   pos   = buffer + ( ( i + top ) * _cols + left ) * 2;
      char *   size  = pos + width * 2;
      while( pos < size )
      {
         *pos  = ( char ) attrib;
         pos++;
         *pos  = 0;
         pos++;
      }
   }
   rectChanging( QRect( left, top, width, height ) );
   setModified();
}

/** Writes a string with the current attribute to the buffer; returns true if write has been done, or false if data can't be written. */
bool QTconsoleDoc::write( int x, int y, char * data, int len )
{
   if( x < 0 || x >= _cols || y < 0 || y >= _rows )
      return false;
   if( len < 0 )
      len = strlen( data );
   if( x + len >= _cols )
      len = _cols - x;
   if( len == 0 )
      return true;

   char *   pos   = buffer + ( ( _cols * y + x ) * 2 );
   char *   end   = buffer + ( ( _cols * y + x + len ) * 2 );
   while( pos < end )
   {
      *pos  = _attrib;
      pos++;
      *pos  = *data;
      pos++;
      data++;
   }
   rectChanging( QRect( x, y, len, 1 ) );
   setModified();
   return true;
}

/** Writes a char */
bool QTconsoleDoc::write( int x, int y, char c )
{
   if( x < 0 || x >= _cols - 1 || y < 0 || y >= _rows )
      return false;

   char * pos = buffer + ( _cols * y + x ) * 2;
   *pos  = _attrib;
   pos++;
   *pos  = c;
   rectChanging( QRect( x, y, 1, 1 ) );
   setModified();
   return true;
}

/** Reads a block of data from the buffer */
bool QTconsoleDoc::read( int x, int y, char * data, int width, int height )
{
   if( x < 0 || x >= _cols || y < 0 || y >= _rows )
      return false;
   if( width < 0 )
      width = strlen( data );
   if( x + width > _cols )
      width = _cols - x;
   if( width == 0 )
      return true;
   if( y + height > _rows )
      height = _rows - y;

   for( int i = 0; i < height; i++ )
   {
      char *   pos   = buffer + ( ( _cols * ( y + i ) + x ) * 2 ) + 1;
      char *   end   = buffer + ( ( _cols * ( y + i ) + x + width ) * 2 ) + 1;

      while( pos < end )
      {
         *data = *pos;
         pos   += 2;
         data++;
      }
   }
   return true;
}

/** Reads a single character*/
bool QTconsoleDoc::read( int x, int y, char * data )
{
   if( x < 0 || x >= _cols || y < 0 || y >= _rows )
      return false;

   *data = buffer[ ( ( _cols * y + x ) * 2 ) + 1 ];
   return true;
}

/** Reads a single attribute*/
bool QTconsoleDoc::readAttrib( int x, int y, char * attr )
{
   if( x < 0 || x >= _cols || y < 0 || y >= _rows )
      return false;

   *attr = buffer[ ( ( _cols * y + x ) * 2 ) ];
   return true;
}

/** Writes a single attribute*/
bool QTconsoleDoc::writeAttrib( int x, int y, char attr )
{
   if( x < 0 || x >= _cols || y < 0 || y >= _rows )
      return false;

   buffer[ ( ( _cols * y + x ) * 2 ) ] = attr;
   rectChanging( QRect( x, y, 1, 1 ) );
   setModified();
   return true;
}

/** Write Multiline data block of data from the buffer */
bool QTconsoleDoc::write( int x, int y, char * data, int width, int height )
{
   if( x < 0 || x >= _cols || y < 0 || y >= _rows )
      return false;
   if( width < 0 )
      width = strlen( data );
   if( x + width > _cols )
      width = _cols - x;
   if( width == 0 )
      return true;
   if( y + height > _rows )
      height = _rows - y;

   for( int i = 0; i < height; i++ )
   {
      char *   pos   = buffer + ( ( _cols * ( y + i ) + x ) * 2 ) + 1;
      char *   end   = buffer + ( ( _cols * ( y + i ) + x + width ) * 2 ) + 1;

      while( pos < end )
      {
         *pos  = *data;
         pos   += 2;
         data++;
      }
   }
   rectChanging( QRect( x, y, width, height ) );
   setModified();
   return true;
}

/** Reads a block of data from the buffer */
bool QTconsoleDoc::getMem( int x, int y, char * data, int width, int height )
{
   if( x < 0 || x >= _cols || y < 0 || y >= _rows )
      return false;
   if( width < 0 )
      width = strlen( data );
   if( x + width > _cols )
      width = _cols - x;
   if( width == 0 )
      return true;
   if( y + height > _rows )
      height = _rows - y;

   for( int i = 0; i < height; i++ )
   {
      char *   pos   = buffer + ( ( _cols * ( y + i ) + x ) * 2 );
      char *   end   = buffer + ( ( _cols * ( y + i ) + x + width - 1 ) * 2 ) + 1;

      while( pos <= end )
      {
         *data = *pos;
         pos++;
         data++;
      }
   }
   return true;
}

/** Writes a block of data from the buffer */
bool QTconsoleDoc::setMem( int x, int y, char * data, int width, int height )
{
   if( x < 0 || x >= _cols || y < 0 || y >= _rows )
      return false;
   if( width < 0 )
      width = strlen( data );
   if( x + width > _cols )
      width = _cols - x;
   if( width == 0 )
      return true;
   if( y + height > _rows )
      height = _rows - y;

   for( int i = 0; i < height; i++ )
   {
      char *   pos   = buffer + ( ( _cols * ( y + i ) + x ) * 2 );
      char *   end   = buffer + ( ( _cols * ( y + i ) + x + width - 1 ) * 2 ) + 1;

      while( pos <= end )
      {
         *pos = *data;
         pos++;
         data++;
      }
   }
   rectChanging( QRect( x, y, width, height ) );
   setModified();
   return true;
}

/** Adds a rectangle to the current modify area, that will be notified when the
   changes are comited to the views with setModified( true ) or endChanging() */
void QTconsoleDoc::rectChanging( QRect r )
{
   if( _rectCommited )
   {
      _rectMody      = r;
      _rectCommited  = false;
   }
   else
      _rectMody |= r;
}

/** Scroll the text of a defined number of lines... */
bool QTconsoleDoc::scroll( int lines )
{
   if( lines <= 0 )
      lines = _rows;
   if( lines > _rows )
      lines = _rows;

   int rest = _rows - lines;
   // only if we have to copy some content
   if( rest )
   {
      char * data = new char[ rest * _cols * 2 ];
      getMem( 0, lines, data, _cols, rest );
      clearScr( getAttrib() );
      setMem( 0, 0, data, _cols, rest );
      delete data;
   }
   else
      clearScr( getAttrib() );

   return true;
}

bool QTconsoleDoc::scroll( int top, int left, int bottom, int right, char attr,
                           int vert, int horiz )
{
   int   width    = right - left + 1;
   int   height   = bottom - top + 1;

   if( width < 0 || height < 0 )
      return false;

   if( vert == 0 && horiz == 0 )
   {
      if( height == _rows && width == _cols )
         clearScr( attr );
      else
         clearScr( attr, left, top, width, height );
      return true;
   }

   if( abs( vert ) >= height || abs( horiz ) >= width )
   {
      clearScr( attr, left, top, width, height );
      return true;
   }

   char * data = new char[ width * height * 2 ];
   getMem( left, top, data, width, height );
   startChanging();
   clearScr( attr, left, top, width, height );
   // select starting point
   height -= abs( vert );
   char * data1 = data;
   if( vert > 0 )
      data1 += vert * width * 2;
   else
      top += -vert;

   // for the moment, orizontal scrolling is not implemented
   setMem( left, top, data1, width, height );
   endChanging();
   delete data;

   return true;
}

/** Writes a "teletype" character, acting as a console */
bool QTconsoleDoc::write( char c )
{
   if( cursX < 0 || cursX >= _cols || cursY < 0 || cursY >= _rows )
      return false;

   int   oldCursX = cursX, oldCursY = cursY;
   //handle special characters
   bool  advance  = false;
   // signal that we "could" issue more than 1 modify command
   startChanging();
   switch( c )
   {
      case 13: cursX = 0; break;
      case 10: cursY++; break;
      case 9: cursX  += 8; break;
      case 12: clearScr( getAttrib() ); break;
      case 8:
         cursX--;
         if( cursX < 0 )
         {
            cursX = _cols - 1;
            if( cursY > 0 )
               cursY--;
         }
         buffer[ ( cursX + cursY * _cols ) * 2 + 1 ]  = 0;
         break;
      default:
         char * pos                                   = buffer + ( ( cursX + cursY * _cols ) * 2 );
         *pos                                         = _attrib;
         pos++;
         *pos                                         = c;
         advance                                      = true;
         break;
   }
   // now advance cursor
   if( advance )
      cursX++;

   // scroll if needed
   if( cursX >= _cols )
   {
      cursX = 0;
      cursY++;
   }
   if( cursY == _rows )
   {
      cursY--;
      scroll();
   }
   // add the character and following character (cursor) to be modified
   rectChanging( QRect( oldCursX, oldCursY, cursX - oldCursX + 1, cursY - oldCursY + 1 ) );
   endChanging();
   return true;
}

/** Writes a "teletype" string acting as a console */
bool QTconsoleDoc::write( char * data, int len )
{
   if( len == -1 )
      len = strlen( data );
   startChanging();
   for( int i = 0; i < len; i++ )
   {
      write( data[ i ] );
   }
   endChanging();
   return true;
}


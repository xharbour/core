/*
 * $Id: readme.txt 9279 2011-02-14 18:06:32Z druzus $
 */

                +---------------------------------------------------------------+
                | 22/november/2001 - Harbour MySQL access classes - readme file |
                +---------------------------------------------------------------+


This is work in progress, so it has to be fully tested and needs a few more methods to cover MySQL possibilities.


This set of files gives you a mean to access a MySQL server, I've developed and tested them on a OS/2 platform,
In their present state MySQL classes are made up of these files:

mysql.c     :  low level wrapper around MySQL client API. It requires libmysqlclient.a library
mysql.ch    :  [x]Harbour level defines of MySQL types
tmysql.prg  :  MySQL access classes
test.prg    :  a little test program which wont work for you :-) since it uses a .dbf.
               Use it as a small tutorial of tmysql.prg provided functions.
Makefile    :  my makefile for OS/2 gcc.
makefile.bc,
make_b32.bat:  batch file and makefile to build mysql.lib on win32

You will also need all .h files from include subdir of your installed MySQL server, at a bare minimum
they are:

      mysql.h, mysql_com.h, mysql_version.h and my_alloc.h in some new intallations.

(under OS/2 with OS/2 port of MySql you need to use the ones from 3.21.33b build which is the only one with
a single threaded libmysqlclient.a client library and works ok even with latest MySQL/2 availble).

To build this library on win32 (using freely available Borland C++ compiler) you
need to follow these steps:

1) go to www.mysql.com and download any version you like of mysql.
   So far I've tested only 3.x versions of MySQL. Install it.
   Now you have an include subdir with .h files with the same version
   number as your libmysql.dll.

   This is very important. You need to use .h files from the package you
   install.

2) Copy inside \contrib\mysql all .h files from mysql include subdir.

3) Do a "make_b32" inside \contrib\mysql to build mysql.lib.

4) Use implib from borland free bcc to create an import library for
   libmysql.dll and call it libmysql.lib

   implib libmysql.lib libmysql.dll

5) Link mysql.lib and libmysql.lib to your harbour program (you can try to
   recompile dbf2mysql inside \contrib\mysql just to test everything) and
   be sure to have libmysql.dll on your path.


                              +---------------------+
                              |  Available Classes  |
                              +---------------------+


tmysql.prg defines four classes:

TMySQLServer:  manages access to a MySQL server and returns an oServer object to which you'll send all your
               queries;
TMySQLQuery:   a standard query to an oServer with joins. Every query has a GetRow() method
               which on every call returns a TMySQLRow object which, in turn, contains requested fields.
               Query objects convert MySQL answers (which is an array of strings) to clipper level types.
               At present time N (with decimals), L, D, and C clipper types are supported.
TMySQLTable:   It's a descendant of a TMySQLQuery and you'll receive it when your query has no joins.
               It adds Update(), Append() and Delete() methods which receive a TMySQLRow object and
               reflect changes to the MySQL table from which they come.
               Please note that TMySQLQuery objects don't have these methods, so, if you want to change
               a row received from a TMySQLQuery object you need to construct a valid SQL query and submit
               it to an oServer object.
TMySQLRow:     Every row returned by a SELECT is converted to a TMySQLRow object. This object handles
               fields and has methods to access fields given a field name or position.

I'm aware that this brief document doesn't explain a lot about MySQL access classes and I'm sorry for that
(please read the souce code, it has quite a few comments which can help you understand what's going on)

I'll try to update it as work on these classes goes by and I'll like to receive feedbak and suggestions
from users (if any :-))

Excuse my poor english and happy selecting :-)

Maurilio Longo - <maurilio.longo@libero.it>

New Enhacemets

Added support to mediumint and mediumblob type on the follow classes
TMySqlRow:FieldType() will Return an "B" for mediumblob and an "I" Mediumint field
TMySQLServer:CreateTable(cTable, aStruct,cPrimaryKey,cUniqueKey,cAuto) Added tree new parameters
     cPrimarykey tell to use the field defined as an primaty key.
     cUniqueKey  tell that this field has unique values
     cAuto       Tell that this field should be auto_increment
     Also to create an Table with mediumint and mediumblob field types use
     {{"Data","B",1,0},{"Ammount","I",9,0}} in the astructure array
     To save an File to an mediumblob field use
     cBuffer:=FILETOSQLBINARY(cFile) . this will read up the file and return an string formated to mysql requeriments

Luiz Rafael Culik - <culik@sl.conex.net>


New Enhancements

TMySQLServer:DBExist( cDatabase ) return true if exist the database.
TMySQLServer:TableExist( cTable ) return true if exist the table in the database.

TMySQLQuery   Add as instance variable, the columns of table.
              This permit access to data with this format: oQuery:<column>
              Also is permited access to data with oQuery:aRow[i] format.

TMySQLServer:CreateTable() Support timestamp field types.
              {{"Stamp","T",14,0}}

Several optimizations of code with xHarbour enhancements, (FOR-EACH, SWITCH-CASE).
Order for more readable code.

Walter Negro <anegro@overnet.com.ar>

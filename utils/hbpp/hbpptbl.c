/*
 * $Id: hbpptbl.c 9279 2011-02-14 18:06:32Z druzus $
 */

/*
 * Harbour Project source code:
 * Preprocessor precompiled STD.CH and some additions ( mainly generated )
 *
 * Copyright 1999 Alexander S.Kresin <alex@belacy.belgorod.su>
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

/*
 * Avoid tracing in preprocessor/compiler.
 */
#if ! defined(HB_TRACE_UTILS)
   #if defined(HB_TRACE_LEVEL)
      #undef HB_TRACE_LEVEL
   #endif
#endif

#include <stdio.h>

#include "hbppdef.h"

COMMANDS * hb_pp_topCommand = NULL;
COMMANDS * hb_pp_topTranslate = NULL;
DEFINES * hb_pp_topDefine = NULL;

void hb_pp_Table( void )
{
   static DEFINES sD___01 = {"_SET_ALTERNATE",NULL,-1,"18", NULL };
   static DEFINES sD___02 = {"_SET_ALTFILE",NULL,-1,"19", &sD___01 };
   static DEFINES sD___03 = {"_SET_BELL",NULL,-1,"26", &sD___02 };
   static DEFINES sD___04 = {"_SET_CANCEL",NULL,-1,"12", &sD___03 };
   static DEFINES sD___05 = {"_SET_COLOR",NULL,-1,"15", &sD___04 };
   static DEFINES sD___06 = {"_SET_CONFIRM",NULL,-1,"27", &sD___05 };
   static DEFINES sD___07 = {"_SET_CONSOLE",NULL,-1,"17", &sD___06 };
   static DEFINES sD___08 = {"_SET_CURSOR",NULL,-1,"16", &sD___07 };
   static DEFINES sD___09 = {"_SET_DATEFORMAT",NULL,-1,"4", &sD___08 };
   static DEFINES sD___10 = {"_SET_DEBUG",NULL,-1,"13", &sD___09 };
   static DEFINES sD___11 = {"_SET_DECIMALS",NULL,-1,"3", &sD___10 };
   static DEFINES sD___12 = {"_SET_DEFAULT",NULL,-1,"7", &sD___11 };
   static DEFINES sD___13 = {"_SET_DELETED",NULL,-1,"11", &sD___12 };
   static DEFINES sD___14 = {"_SET_DELIMCHARS",NULL,-1,"34", &sD___13 };
   static DEFINES sD___15 = {"_SET_DELIMITERS",NULL,-1,"33", &sD___14 };
   static DEFINES sD___16 = {"_SET_DEVICE",NULL,-1,"20", &sD___15 };
   static DEFINES sD___17 = {"_SET_EPOCH",NULL,-1,"5", &sD___16 };
   static DEFINES sD___18 = {"_SET_ESCAPE",NULL,-1,"28", &sD___17 };
   static DEFINES sD___19 = {"_SET_EVENTMASK",NULL,-1,"39", &sD___18 };
   static DEFINES sD___20 = {"_SET_EXACT",NULL,-1,"1", &sD___19 };
   static DEFINES sD___21 = {"_SET_EXCLUSIVE",NULL,-1,"8", &sD___20 };
   static DEFINES sD___22 = {"_SET_EXIT",NULL,-1,"30", &sD___21 };
   static DEFINES sD___23 = {"_SET_EXTRA",NULL,-1,"21", &sD___22 };
   static DEFINES sD___24 = {"_SET_EXTRAFILE",NULL,-1,"22", &sD___23 };
   static DEFINES sD___25 = {"_SET_FIXED",NULL,-1,"2", &sD___24 };
   static DEFINES sD___26 = {"_SET_INSERT",NULL,-1,"29", &sD___25 };
   static DEFINES sD___27 = {"_SET_INTENSITY",NULL,-1,"31", &sD___26 };
   static DEFINES sD___28 = {"_SET_MARGIN",NULL,-1,"25", &sD___27 };
   static DEFINES sD___29 = {"_SET_MCENTER",NULL,-1,"37", &sD___28 };
   static DEFINES sD___30 = {"_SET_MESSAGE",NULL,-1,"36", &sD___29 };
   static DEFINES sD___31 = {"_SET_PATH",NULL,-1,"6", &sD___30 };
   static DEFINES sD___32 = {"_SET_PRINTER",NULL,-1,"23", &sD___31 };
   static DEFINES sD___33 = {"_SET_PRINTFILE",NULL,-1,"24", &sD___32 };
   static DEFINES sD___34 = {"_SET_SCOREBOARD",NULL,-1,"32", &sD___33 };
   static DEFINES sD___35 = {"_SET_SCROLLBREAK",NULL,-1,"38", &sD___34 };
   static DEFINES sD___36 = {"_SET_SOFTSEEK",NULL,-1,"9", &sD___35 };
   static DEFINES sD___37 = {"_SET_TYPEAHEAD",NULL,-1,"14", &sD___36 };
   static DEFINES sD___38 = {"_SET_UNIQUE",NULL,-1,"10", &sD___37 };
   static DEFINES sD___39 = {"_SET_WRAP",NULL,-1,"35", &sD___38 };
   static DEFINES sD___40 = {"_SET_COUNT",NULL,-1,"47", &sD___39 };
   static DEFINES sD___41 = {"_SET_CH",NULL,-1,NULL, &sD___40 };
   static DEFINES sD___42 = {"_DFSET","\1A,\1B",2,"Set( _SET_DATEFORMAT, IIF(__SetCentury(), \1A, \1B) )", &sD___41 };
   static DEFINES sD___43 = {"_SET_VIDEOMODE",NULL,-1,"40", &sD___42 };
   static DEFINES sD___44 = {"_SET_MBLOCKSIZE",NULL,-1,"41", &sD___43 };
   static DEFINES sD___45 = {"_SET_MFILEEXT",NULL,-1,"42", &sD___44 };
   static DEFINES sD___46 = {"_SET_STRICTREAD",NULL,-1,"43", &sD___45 };
   static DEFINES sD___47 = {"_SET_OPTIMIZE",NULL,-1,"44", &sD___46 };
   static DEFINES sD___48 = {"_SET_AUTOPEN",NULL,-1,"45", &sD___47 };
   static DEFINES sD___49 = {"_SET_AUTORDER",NULL,-1,"46", &sD___48 };
   static DEFINES sD___50 = {"_SET_AUTOSHARE",NULL,-1,"47", &sD___49 };
   static DEFINES sD___51 = {"_SET_LANGUAGE",NULL,-1,"100", &sD___50 };
   static DEFINES sD___52 = {"_SET_IDLEREPEAT",NULL,-1,"101", &sD___51 };
   static DEFINES sD___53 = {"_SET_TRACE",NULL,-1,"102", &sD___52 };
   static DEFINES sD___54 = {"_SET_TRACEFILE",NULL,-1,"103", &sD___53 };
   static DEFINES sD___55 = {"_SET_TRACESTACK",NULL,-1,"104", &sD___54 };
   static DEFINES sD___56 = {"_SET_FILECASE",NULL,-1,"105", &sD___55 };
   static DEFINES sD___57 = {"_SET_DIRCASE",NULL,-1,"106", &sD___56 };
   static DEFINES sD___58 = {"_SET_DIRSEPARATOR",NULL,-1,"107", &sD___57 };
   static DEFINES sD___59 = {"_SET_ERRORLOOP",NULL,-1,"108", &sD___58 };
   static DEFINES sD___60 = {"_SET_OUTPUTSAFETY",NULL,-1,"109", &sD___59 };
   static DEFINES sD___61 = {"_SET_DBFLOCKSCHEME",NULL,-1,"110", &sD___60 };
   static DEFINES sD___62 = {"_SET_BACKGROUNDTASKS",NULL,-1,"111", &sD___61 };
   static DEFINES sD___63 = {"_SET_TRIMFILENAME",NULL,-1,"112", &sD___62 };
   static DEFINES sD___64 = {"_SET_GTMODE",NULL,-1,"113", &sD___63 };
   static DEFINES sD___65 = {"_SET_BACKGROUNDTICK",NULL,-1,"114", &sD___64 };
   static DEFINES sD___66 = {"_SET_PRINTERJOB",NULL,-1,"115", &sD___65 };
   static DEFINES sD___67 = {"_SET_HARDCOMMIT",NULL,-1,"116", &sD___66 };
   static DEFINES sD___68 = {"_SET_FORCEOPT",NULL,-1,"117", &sD___67 };
   static DEFINES sD___69 = {"_SET_EOL",NULL,-1,"118", &sD___68 };
   static DEFINES sD___70 = {"_SET_ERRORLOG",NULL,-1,"119", &sD___69 };
   static DEFINES sD___71 = {"HB_SET_BASE",NULL,-1,"100", &sD___70 };
   static DEFINES sD___72 = {"HB_SET_COUNT",NULL,-1,"20", &sD___71 };

   static COMMANDS sC___1 = {0,"NOTE","\1A30",NULL,NULL };
   static COMMANDS sC___2 = {0,"DO","WHILE \1A00","while \1A00",&sC___1 };

   static COMMANDS sC___3 = {0,"END","\1A00","end",&sC___2 };
   static COMMANDS sC___4 = {0,"END","SEQUENCE","end",&sC___3 };
   static COMMANDS sC___5 = {0,"ENDSEQUENCE","","end",&sC___4 };

/*
   static COMMANDS sC___6 = {0,"ENDDO","\1A30","enddo",&sC___5 };
   static COMMANDS sC___7 = {0,"ENDIF","\1A30","endif",&sC___6 };
   static COMMANDS sC___8 = {0,"ENDCASE","\1A30","endcase",&sC___7 };

  Clipper rules are buggy in that they will HIDE lines following END* in lines ':' seperated
  multi-lins, such as:

     IF x; y(); ENDIF; z() // z() will be sent to lalaland

  This is extermly problematic with #directives generating multiple lines such as hbclass.ch

  The corrected rules are:

  // Using sTrangE cApitalizatioN to signify transformation when debugging PPO files.
  #command ENDDO   <any,...> [<anymore,...>] => eNddO
  #command ENDIF   <any,...> [<anymore,...>] => eNdiF
  #command ENDCASE <any,...> [<anymore,...>] => eNdcasE
*/
   static COMMANDS sC___6 = {0,"ENDDO","\1A10 \16\1B10\17","eNddO",&sC___5 };
   static COMMANDS sC___7 = {0,"ENDIF","\1A10 \16\1B10\17","eNdiF",&sC___6 };
   static COMMANDS sC___8 = {0,"ENDCASE","\1A10 \16\1B10\17","eNdcasE",&sC___7 };

   static COMMANDS sC___9 = {0,"ENDFOR","\16 \1A30 \17","next",&sC___8 };
   static COMMANDS sC___10 = {0,"NEXT","\1A00 \16TO \1B00\17 \16STEP \1C00\17","next",&sC___9 };
   static COMMANDS sC___11 = {0,"DO","\1A00.PRG \16WITH \1B10\17","do \1A00 \16 WITH \1B00\17",&sC___10 };
   static COMMANDS sC___12 = {0,"CALL","\1A00() \16WITH \1B10\17","call \1A00 \16 WITH \1B00\17",&sC___11 };
   static COMMANDS sC___13 = {0,"STORE","\1A00 TO \1B00 \16, \1C00 \17","\1B00 := \16 \1C00 := \17 \1A00",&sC___12 };
   static COMMANDS sC___14 = {0,"SET","ECHO \1A30",NULL,&sC___13 };
   static COMMANDS sC___15 = {0,"SET","HEADING \1A30",NULL,&sC___14 };
   static COMMANDS sC___16 = {0,"SET","MENU \1A30",NULL,&sC___15 };
   static COMMANDS sC___17 = {0,"SET","STATUS \1A30",NULL,&sC___16 };
   static COMMANDS sC___18 = {0,"SET","STEP \1A30",NULL,&sC___17 };
   static COMMANDS sC___19 = {0,"SET","SAFETY \1A30",NULL,&sC___18 };
   static COMMANDS sC___20 = {0,"SET","TALK \1A30",NULL,&sC___19 };
   static COMMANDS sC___21 = {0,"SET","PROCEDURE TO",NULL,&sC___20 };
   static COMMANDS sC___22 = {0,"SET","PROCEDURE TO \1A00","_ProcReq_( \1A30 )",&sC___21 };
   static COMMANDS sC___23 = {0,"SET","EXACT \1A20ON,OFF,&>","Set( _SET_EXACT, \1A30 )",&sC___22 };
   static COMMANDS sC___24 = {0,"SET","EXACT (\1A00)","Set( _SET_EXACT, \1A00 )",&sC___23 };
   static COMMANDS sC___25 = {0,"SET","FIXED \1A20ON,OFF,&>","Set( _SET_FIXED, \1A30 )",&sC___24 };
   static COMMANDS sC___26 = {0,"SET","FIXED (\1A00)","Set( _SET_FIXED, \1A00 )",&sC___25 };
   static COMMANDS sC___27 = {0,"SET","DECIMALS TO \1A00","Set( _SET_DECIMALS, \1A00 )",&sC___26 };
   static COMMANDS sC___28 = {0,"SET","DECIMALS TO","Set( _SET_DECIMALS, 0 )",&sC___27 };
   static COMMANDS sC___29 = {0,"SET","PATH TO \1A30","Set( _SET_PATH, \1A30 )",&sC___28 };
   static COMMANDS sC___30 = {0,"SET","PATH TO","Set( _SET_PATH, "" )",&sC___29 };
   static COMMANDS sC___31 = {0,"SET","DEFAULT TO \1A40","Set( _SET_DEFAULT, \1A30 )",&sC___30 };
   static COMMANDS sC___32 = {0,"SET","DEFAULT TO","Set( _SET_DEFAULT, "" )",&sC___31 };
   static COMMANDS sC___33 = {0,"SET","CENTURY \1A20ON,OFF,&>","__SetCentury( \1A30 )",&sC___32 };
   static COMMANDS sC___34 = {0,"SET","CENTURY (\1A00)","__SetCentury( \1A00 )",&sC___33 };
   static COMMANDS sC___35 = {0,"SET","EPOCH TO \1A00","Set( _SET_EPOCH, \1A00 )",&sC___34 };
   static COMMANDS sC___36 = {0,"SET","DATE FORMAT \16TO\17 \1A00","Set( _SET_DATEFORMAT, \1A00 )",&sC___35 };
   static COMMANDS sC___37 = {0,"SET","DATE \16TO\17 AMERICAN","_DFSET( 'mm/dd/yyyy', 'mm/dd/yy' )",&sC___36 };
   static COMMANDS sC___38 = {0,"SET","DATE \16TO\17 ANSI","_DFSET( 'yyyy.mm.dd', 'yy.mm.dd' )",&sC___37 };
   static COMMANDS sC___39 = {0,"SET","DATE \16TO\17 BRITISH","_DFSET( 'dd/mm/yyyy', 'dd/mm/yy' )",&sC___38 };
   static COMMANDS sC___40 = {0,"SET","DATE \16TO\17 FRENCH","_DFSET( 'dd/mm/yyyy', 'dd/mm/yy' )",&sC___39 };
   static COMMANDS sC___41 = {0,"SET","DATE \16TO\17 GERMAN","_DFSET( 'dd.mm.yyyy', 'dd.mm.yy' )",&sC___40 };
   static COMMANDS sC___42 = {0,"SET","DATE \16TO\17 ITALIAN","_DFSET( 'dd-mm-yyyy', 'dd-mm-yy' )",&sC___41 };
   static COMMANDS sC___43 = {0,"SET","DATE \16TO\17 JAPANESE","_DFSET( 'yyyy/mm/dd', 'yy/mm/dd' )",&sC___42 };
   static COMMANDS sC___44 = {0,"SET","DATE \16TO\17 USA","_DFSET( 'mm-dd-yyyy', 'mm-dd-yy' )",&sC___43 };
   static COMMANDS sC___45 = {0,"SET","ALTERNATE \1A20ON,OFF,&>","Set( _SET_ALTERNATE, \1A30 )",&sC___44 };
   static COMMANDS sC___46 = {0,"SET","ALTERNATE (\1A00)","Set( _SET_ALTERNATE, \1A00 )",&sC___45 };
   static COMMANDS sC___47 = {0,"SET","ALTERNATE TO","Set( _SET_ALTFILE, "" )",&sC___46 };
   static COMMANDS sC___48 = {0,"SET","ALTERNATE TO \1A40 \16\1B20 ADDITIVE>\17","Set( _SET_ALTFILE, \1A30, \1B50 )",&sC___47 };
   static COMMANDS sC___49 = {0,"SET","CONSOLE \1A20ON,OFF,&>","Set( _SET_CONSOLE, \1A30 )",&sC___48 };
   static COMMANDS sC___50 = {0,"SET","CONSOLE (\1A00)","Set( _SET_CONSOLE, \1A00 )",&sC___49 };
   static COMMANDS sC___51 = {0,"SET","MARGIN TO \1A00","Set( _SET_MARGIN, \1A00 )",&sC___50 };
   static COMMANDS sC___52 = {0,"SET","MARGIN TO","Set( _SET_MARGIN, 0 )",&sC___51 };
   static COMMANDS sC___53 = {0,"SET","PRINTER \1A20ON,OFF,&>","Set( _SET_PRINTER, \1A30 )",&sC___52 };
   static COMMANDS sC___54 = {0,"SET","PRINTER (\1A00)","Set( _SET_PRINTER, \1A00 )",&sC___53 };
   static COMMANDS sC___55 = {0,"SET","PRINTER TO","Set( _SET_PRINTFILE, '""' )",&sC___54 };
   static COMMANDS sC___56 = {0,"SET","PRINTER TO \1A40 \16\1B20 ADDITIVE>\17","Set( _SET_PRINTFILE, \1A30, \1B50 )",&sC___55 };
   static COMMANDS sC___57 = {0,"SET","DEVICE TO SCREEN","Set( _SET_DEVICE, 'SCREEN' )",&sC___56 };
   static COMMANDS sC___58 = {0,"SET","DEVICE TO PRINTER","Set( _SET_DEVICE, 'PRINTER' )",&sC___57 };
   static COMMANDS sC___59 = {0,"SET","COLOR TO \16\1A30\17","SetColor( \1A10 )",&sC___58 };
   static COMMANDS sC___60 = {0,"SET","COLOR TO ( \1A00 )","SetColor( \1A00 )",&sC___59 };
   static COMMANDS sC___61 = {0,"SET","COLOUR TO \16\1A30\17","SET COLOR TO \16\1A00\17",&sC___60 };
   static COMMANDS sC___62 = {0,"SET","CURSOR \1A20ON,OFF,&>","SetCursor( IIF(Upper(\1A30) == 'ON', 1, 0) )",&sC___61 };
   static COMMANDS sC___63 = {0,"SET","CURSOR (\1A00)","SetCursor( IIF(\1A00, 1, 0) )",&sC___62 };
   static COMMANDS sC___64 = {0,"?","\16 \1A10\17","QOut( \1A00 )",&sC___63 };
   static COMMANDS sC___65 = {0,"?","? \16 \1A10\17","QQOut( \1A00 )",&sC___64 };
   static COMMANDS sC___66 = {0,"EJECT","","__Eject()",&sC___65 };
   static COMMANDS sC___67 = {0,"TEXT","","text QOut, QQOut",&sC___66 };
   static COMMANDS sC___68 = {0,"TEXT","TO FILE \1A40","__TextSave( \1A30 ) ; text QOut, __TextRestore",&sC___67 };
   static COMMANDS sC___69 = {0,"TEXT","TO PRINTER","__TextSave('PRINTER') ; text QOut, __TextRestore",&sC___68 };
   static COMMANDS sC___70 = {0,"CLS","","Scroll() ; SetPos(0,0)",&sC___69 };
   static COMMANDS sC___71 = {0,"CLEAR","SCREEN","CLS",&sC___70 };
   static COMMANDS sC___72 = {0,"@","\1A00, \1B00","Scroll( \1A00, \1B00, \1A00 ) ; SetPos( \1A00, \1B00 )",&sC___71 };
   static COMMANDS sC___73 = {0,"@","\1A00, \1B00 CLEAR","Scroll( \1A00, \1B00 ) ; SetPos( \1A00, \1B00 )",&sC___72 };
   static COMMANDS sC___74 = {0,"@","\1A00, \1B00 CLEAR TO \1C00, \1D00",
       "Scroll( \1A00, \1B00, \1C00, \1D00 ) ; SetPos( \1A00, \1B00 )",&sC___73 };
   static COMMANDS sC___75 = {0,"@","\1A00, \1B00, \1C00, \1D00 BOX \1E00 \16COLOR \1F00\17",
       "DispBox( \1A00, \1B00, \1C00, \1D00, \1E00 \16, \1F00 \17 )",&sC___74 };
   static COMMANDS sC___76 = {0,"@","\1A00, \1B00 TO \1C00, \1D00 \16DOUBLE\17 \16COLOR \1E00\17",
       "DispBox( \1A00, \1B00, \1C00, \1D00, 2 \16, \1E00 \17 )",&sC___75 };
   static COMMANDS sC___77 = {0,"@","\1A00, \1B00 TO \1C00, \1D00 \16COLOR \1E00\17",
       "DispBox( \1A00, \1B00, \1C00, \1D00, 1 \16, \1E00 \17 )",&sC___76 };

   static COMMANDS sC___78 = {0,"@","\1A00, \1B00 SAY \1C00 \16PICTURE \1D00\17 \16COLOR \1E00\17",
       "DevPos( \1A00, \1B00 ); DevOutPict( \1C00, \1D00, \1E00  )",&sC___77 };
   static COMMANDS sC___79 = {0,"@","\1A00, \1B00 SAY \1C00 \16COLOR \1D00\17",
       "DevPos( \1A00, \1B00 ); DevOut( \1C00, \1D00 )",&sC___78 };

   /* This hack breaks compatibility with some proper Clipper code
      If someone wants to use it then should put it in your own .ch file */
   /*
   static COMMANDS sC___78 = {0,"@","\1A00, \1B00 SAY \1C00 \16PICTURE \1D00\17 \16COLOR \1E00\17",
       "DevOutPict( \1C00, \1D00, \1E00, \1A00, \1B00 )",&sC___77 };
   static COMMANDS sC___79 = {0,"@","\1A00, \1B00 SAY \1C00 \16COLOR \1D00\17",
       "DevOut( \1C00, \1D00, \1A00, \1B00 )",&sC___78 };
   */

   static COMMANDS sC___80 = {0,"SET","BELL \1A20ON,OFF,&>","Set( _SET_BELL, \1A30 )",&sC___79 };
   static COMMANDS sC___81 = {0,"SET","BELL (\1A00)","Set( _SET_BELL, \1A00 )",&sC___80 };
   static COMMANDS sC___82 = {0,"SET","CONFIRM \1A20ON,OFF,&>","Set( _SET_CONFIRM, \1A30 )",&sC___81 };
   static COMMANDS sC___83 = {0,"SET","CONFIRM (\1A00)","Set( _SET_CONFIRM, \1A00 )",&sC___82 };
   static COMMANDS sC___84 = {0,"SET","ESCAPE \1A20ON,OFF,&>","Set( _SET_ESCAPE, \1A30 )",&sC___83 };
   static COMMANDS sC___85 = {0,"SET","ESCAPE (\1A00)","Set( _SET_ESCAPE, \1A00 )",&sC___84 };
   static COMMANDS sC___86 = {0,"SET","INTENSITY \1A20ON,OFF,&>","Set( _SET_INTENSITY, \1A30 )",&sC___85 };
   static COMMANDS sC___87 = {0,"SET","INTENSITY (\1A00)","Set( _SET_INTENSITY, \1A00 )",&sC___86 };
   static COMMANDS sC___88 = {0,"SET","SCOREBOARD \1A20ON,OFF,&>","Set( _SET_SCOREBOARD, \1A30 )",&sC___87 };
   static COMMANDS sC___89 = {0,"SET","SCOREBOARD (\1A00)","Set( _SET_SCOREBOARD, \1A00 )",&sC___88 };
   static COMMANDS sC___90 = {0,"SET","DELIMITERS \1A20ON,OFF,&>","Set( _SET_DELIMITERS, \1A30 )",&sC___89 };
   static COMMANDS sC___91 = {0,"SET","DELIMITERS (\1A00)","Set( _SET_DELIMITERS, \1A00 )",&sC___90 };
   static COMMANDS sC___92 = {0,"SET","DELIMITERS TO \1A00","Set( _SET_DELIMCHARS, \1A00 )",&sC___91 };
   static COMMANDS sC___93 = {0,"SET","DELIMITERS TO DEFAULT","Set( _SET_DELIMCHARS, '::' )",&sC___92 };
   static COMMANDS sC___94 = {0,"SET","DELIMITERS TO","Set( _SET_DELIMCHARS, '::' )",&sC___93 };
   static COMMANDS sC___95 = {0,"SET","FORMAT TO \1A00","_ProcReq_( \1A30 + '.fmt' ) ; __SetFormat( {|| \1A00()} )",&sC___94 };
   static COMMANDS sC___96 = {0,"SET","FORMAT TO \1A00.\1B00",
       "_ProcReq_( \1A30 + '.' + \1B30 ) ; __SetFormat( {|| \1A00()} )",&sC___95 };
   static COMMANDS sC___97 = {0,"SET","FORMAT TO \1A20&>",
       "if ( Empty(\1A30) ) ;   SET FORMAT TO ; else ;   __SetFormat( &('{||' + \1A30 + '()}') ) ; end",&sC___96 };
   static COMMANDS sC___98 = {0,"SET","FORMAT TO","__SetFormat()",&sC___97 };
   static COMMANDS sC___99 = {0,"@","\1A00, \1B00 GET \1C00 \16PICTURE \1D00\17 \16VALID \1E00\17 \16WHEN \1F00\17 \16SEND \1G00\17",
       "SetPos( \1A00, \1B00 ) ; AAdd( GetList, _GET_( \1C00, \1C20, \1D00, \1E40, \1F40 ):display() ) \16; ATail(GetList):\1G00\17",&sC___98 };
   static COMMANDS sC___100 = {0,"@","\1A00, \1B00 SAY \1C00 \16\1D10\17 GET \1E00 \16\1F10\17",
       "@ \1A00, \1B00 SAY \1C00 \16\1D00\17 ; @ Row(), Col()+1 GET \1E00 \16\1F00\17",&sC___99 };
   static COMMANDS sC___101 = {0,"@","\1A00, \1B00 GET \1C00 \16\1D10\17 RANGE \1E00, \1F00 \16\1G10\17",
       "@ \1A00, \1B00 GET \1C00 \16\1D00\17 VALID {|_1| RangeCheck(_1,, \1E00, \1F00)} \16\1G00\17",&sC___100 };
   static COMMANDS sC___102 = {0,"@","\1A00, \1B00 GET \1C00 \16\1D10\17 COLOR \1E00 \16\1F10\17",
       "@ \1A00, \1B00 GET \1C00 \16\1D00\17 SEND colordisp( \1E00 ) \16\1F00\17",&sC___101 };
   static COMMANDS sC___103 = {0,"READ","SAVE","ReadModal(GetList)",&sC___102 };
   static COMMANDS sC___104 = {0,"READ","","ReadModal(GetList) ; GetList := {}",&sC___103 };
   static COMMANDS sC___105 = {0,"CLEAR","GETS","ReadKill(.T.) ; GetList := {}",&sC___104 };
   static COMMANDS sC___106 = {0,"@","\16\1A10\17 COLOUR \16\1B10\17","@ \16\1A00\17 COLOR \16\1B00\17",&sC___105 };
   static COMMANDS sC___107 = {0,"SET","WRAP \1A20ON,OFF,&>","Set( _SET_WRAP, \1A30 )",&sC___106 };
   static COMMANDS sC___108 = {0,"SET","WRAP (\1A00)","Set( _SET_WRAP, \1A00 )",&sC___107 };
   static COMMANDS sC___109 = {0,"SET","MESSAGE TO \1A00 \16\1B20 CENTER, CENTRE>\17",
       "Set( _SET_MESSAGE, \1A00 ) ; Set( _SET_MCENTER, \1B50 )",&sC___108 };
   static COMMANDS sC___110 = {0,"SET","MESSAGE TO","Set( _SET_MESSAGE, 0 ) ; Set( _SET_MCENTER, .f. )",&sC___109 };
   static COMMANDS sC___111 = {0,"@","\1A00, \1B00 PROMPT \1C00 \16MESSAGE \1D00\17 \16COLOR \1E00\17",
       "__AtPrompt( \1A00, \1B00, \1C00 , \1D00 ,\1E00 )",&sC___110 };
   static COMMANDS sC___112 = {0,"MENU","TO \1A00","\1A00 := __MenuTo( {|_1| IIF(PCount() == 0, \1A00, \1A00 := _1)}, \1A10 )",&sC___111 };
   static COMMANDS sC___113 = {0,"SAVE","SCREEN","__XSaveScreen()",&sC___112 };
   static COMMANDS sC___114 = {0,"RESTORE","SCREEN","__XRestScreen()",&sC___113 };
   static COMMANDS sC___115 = {0,"SAVE","SCREEN TO \1A00","\1A00 := SaveScreen( 0, 0, Maxrow(), Maxcol() )",&sC___114 };
   static COMMANDS sC___116 = {0,"RESTORE","SCREEN FROM \1A00","RestScreen( 0, 0, Maxrow(), Maxcol(), \1A00 )",&sC___115 };
   static COMMANDS sC___117 = {0,"WAIT","\16\1A00\17","__Wait( \1A00 )",&sC___116 };
   static COMMANDS sC___118 = {0,"WAIT","\16\1A00\17 TO \1B00","\1B00 := __Wait( \1A00 )",&sC___117 };
   static COMMANDS sC___119 = {0,"ACCEPT","\16\1A00\17 TO \1B00","\1B00 := __Accept( \1A00 )",&sC___118 };
   static COMMANDS sC___120 = {0,"INPUT","\16\1A00\17 TO \1B00",
       "if ( !Empty(__Accept(\1A00)) ) ;    \1B00 := &( __AcceptStr() ) ; end",&sC___119 };
   static COMMANDS sC___121 = {0,"KEYBOARD","\1A00","__Keyboard( \1A00 )",&sC___120 };
   static COMMANDS sC___122 = {0,"CLEAR","TYPEAHEAD","__Keyboard()",&sC___121 };
   static COMMANDS sC___123 = {0,"SET","TYPEAHEAD TO \1A00","Set( _SET_TYPEAHEAD, \1A00 )",&sC___122 };
   static COMMANDS sC___124 = {0,"SET","KEY \1A00 TO \1B00","SetKey( \1A00, {|p, l, v| \1B00(p, l, v)} )",&sC___123 };
   static COMMANDS sC___125 = {0,"SET","KEY \1A00 TO \1B00 ( \16\1C10\17 )","SET KEY \1A00 TO \1B00",&sC___124 };
   static COMMANDS sC___126 = {0,"SET","KEY \1A00 TO \1B20&>",
       "if ( Empty(\1B30) ) ;   SetKey( \1A00, NIL ) ; else ;   SetKey( \1A00, {|p, l, v| \1B00(p, l, v)} ) ; end",&sC___125 };
   static COMMANDS sC___127 = {0,"SET","KEY \1A00 \16TO\17","SetKey( \1A00, NIL )",&sC___126 };
   static COMMANDS sC___128 = {0,"SET","FUNCTION \1A00 \16TO\17 \16\1B00\17","__SetFunction( \1A00, \1B00 )",&sC___127 };
   static COMMANDS sC___129 = {0,"CLEAR","MEMORY","__MVClear()",&sC___128 };
   static COMMANDS sC___130 = {0,"RELEASE"," \1A10","__MVXRelease( \1A30 )",&sC___129 };
   static COMMANDS sC___131 = {0,"RELEASE","ALL","__MVRelease('*', .t.)",&sC___130 };
   static COMMANDS sC___132 = {0,"RELEASE","ALL LIKE \1A00","__MVRelease( \1A10, .t. )",&sC___131 };
   static COMMANDS sC___133 = {0,"RELEASE","ALL EXCEPT \1A00","__MVRelease( \1A10, .f. )",&sC___132 };
   static COMMANDS sC___134 = {0,"RESTORE","\16FROM \1A40\17 \16\1B20 ADDITIVE>\17 \16\1C20 EXTENDED>\17","__MVRestore( \1A30, \1B50, \1C50 )",&sC___133 };
   static COMMANDS sC___135 = {0,"SAVE","ALL LIKE \1A00 TO \1B40 \16\1C20 EXTENDED>\17","__MVSave( \1B30, \1A30, .t., \1C50 )",&sC___134 };
   static COMMANDS sC___136 = {0,"SAVE","TO \1A40 ALL LIKE \1B40 \16\1C20 EXTENDED>\17","__MVSave( \1A30, \1B30, .t., \1C50 )",&sC___135 };
   static COMMANDS sC___137 = {0,"SAVE","ALL EXCEPT \1A00 TO \1B40 \16\1C20 EXTENDED>\17","__MVSave( \1B30, \1A30, .f., \1C50 )",&sC___136 };
   static COMMANDS sC___138 = {0,"SAVE","TO \1A40 ALL EXCEPT \1B00 \16\1C20 EXTENDED>\17","__MVSave( \1A30, \1B30, .f., \1C50 )",&sC___137 };
   static COMMANDS sC___139 = {0,"SAVE","\16TO \1A40\17 \16ALL\17 \16\1C20 EXTENDED>\17","__MVSave( \1A30, '*', .t., \1C50 )",&sC___138 };
   static COMMANDS sC___140 = {0,"ERASE","\1A40","FErase( \1A30 )",&sC___139 };
   static COMMANDS sC___141 = {0,"DELETE","FILE \1A40","FErase( \1A30 )",&sC___140 };
   static COMMANDS sC___142 = {0,"RENAME","\1A40 TO \1B40","FRename( \1A30, \1B30 )",&sC___141 };
   static COMMANDS sC___143 = {0,"COPY","FILE \1A40 TO \1B40","__CopyFile( \1A30, \1B30 )",&sC___142 };
   static COMMANDS sC___144 = {0,"DIR","\16\1A40\17","__Dir( \1A30 )",&sC___143 };
   static COMMANDS sC___145 = {0,"TYPE","\1A40 \16\1B20 TO PRINTER>\17 \16TO FILE \1C40\17",
       "__TypeFile( \1A30, \1B50 ) \16; COPY FILE \1A30 TO \1C30 \17",&sC___144 };
   static COMMANDS sC___146 = {0,"TYPE","\1A40 \16\1B20 TO PRINTER>\17","__TypeFile( \1A30, \1B50 )",&sC___145 };
   static COMMANDS sC___147 = {0,"REQUEST","\1A10","EXTERNAL \1A00",&sC___146 };
   static COMMANDS sC___148 = {0,"CANCEL","","__Quit()",&sC___147 };
   static COMMANDS sC___149 = {0,"QUIT","","__Quit()",&sC___148 };
   static COMMANDS sC___150 = {0,"RUN","\1A30","__Run( \1A10 )",&sC___149 };
   static COMMANDS sC___151 = {0,"RUN","( \1A00 )","__Run( \1A00 )",&sC___150 };
   static COMMANDS sC___152 = {0,"!","\1A30","RUN \1A00",&sC___151 };
   static COMMANDS sC___153 = {0,"RUN","= \1A00","( run := \1A00 )",&sC___152 };
   static COMMANDS sC___154 = {0,"RUN",":= \1A00","( run := \1A00 )",&sC___153 };
   static COMMANDS sC___155 = {0,"SET","EXCLUSIVE \1A20ON,OFF,&>","Set( _SET_EXCLUSIVE, \1A30 )",&sC___154 };
   static COMMANDS sC___156 = {0,"SET","EXCLUSIVE (\1A00)","Set( _SET_EXCLUSIVE, \1A00 )",&sC___155 };
   static COMMANDS sC___157 = {0,"SET","SOFTSEEK \1A20ON,OFF,&>","Set( _SET_SOFTSEEK, \1A30 )",&sC___156 };
   static COMMANDS sC___158 = {0,"SET","SOFTSEEK (\1A00)","Set( _SET_SOFTSEEK, \1A00 )",&sC___157 };
   static COMMANDS sC___159 = {0,"SET","UNIQUE \1A20ON,OFF,&>","Set( _SET_UNIQUE, \1A30 )",&sC___158 };
   static COMMANDS sC___160 = {0,"SET","UNIQUE (\1A00)","Set( _SET_UNIQUE, \1A00 )",&sC___159 };
   static COMMANDS sC___161 = {0,"SET","DELETED \1A20ON,OFF,&>","Set( _SET_DELETED, \1A30 )",&sC___160 };
   static COMMANDS sC___162 = {0,"SET","DELETED (\1A00)","Set( _SET_DELETED, \1A00 )",&sC___161 };
   static COMMANDS sC___163 = {0,"SELECT","\1A00","dbSelectArea( \1A30 )",&sC___162 };
   static COMMANDS sC___164 = {0,"SELECT","\1A00(\16\1B10\17)","dbSelectArea( \1A00(\1B00) )",&sC___163 };
   static COMMANDS sC___165 = {0,"USE","","dbCloseArea()",&sC___164 };
   static COMMANDS sC___166 = {0,"USE","\1A40 \16VIA \1B00\17 \16ALIAS \1C00\17 \16\1D20 NEW>\17 \16\1E20 EXCLUSIVE>\17 \16\1F20 SHARED>\17 \16\1G20 READONLY>\17 \16CODEPAGE \1H00\17 \16CONNECTION \1I00\17 \16INDEX \1J40 \16, \1K40\17\17",
       "dbUseArea( \1D50, \1B00, \1A30, \1C30, IIF(\1F50 .or. \1E50, !\1E50, NIL), \1G50, \1H30, \1I00 ) \16; dbSetIndex( \1J30 )\17 \16; dbSetIndex( \1K30 )\17",&sC___165 };
   static COMMANDS sC___167 = {0,"APPEND","BLANK","dbAppend()",&sC___166 };
   static COMMANDS sC___168 = {0,"PACK","","__dbPack()",&sC___167 };
   static COMMANDS sC___169 = {0,"ZAP","","__dbZap()",&sC___168 };
   static COMMANDS sC___170 = {0,"UNLOCK","","dbUnlock()",&sC___169 };
   static COMMANDS sC___171 = {0,"UNLOCK","ALL","dbUnlockAll()",&sC___170 };
   static COMMANDS sC___172 = {0,"COMMIT","","dbCommitAll()",&sC___171 };
   static COMMANDS sC___173 = {0,"GOTO","\1A00","dbGoto(\1A00)",&sC___172 };
   static COMMANDS sC___174 = {0,"GO","\1A00","dbGoto(\1A00)",&sC___173 };
   static COMMANDS sC___175 = {0,"GOTO","TOP","dbGoTop()",&sC___174 };
   static COMMANDS sC___176 = {0,"GO","TOP","dbGoTop()",&sC___175 };
   static COMMANDS sC___177 = {0,"GOTO","BOTTOM","dbGoBottom()",&sC___176 };
   static COMMANDS sC___178 = {0,"GO","BOTTOM","dbGoBottom()",&sC___177 };
   static COMMANDS sC___179 = {0,"SKIP","","dbSkip(1)",&sC___178 };
   static COMMANDS sC___180 = {0,"SKIP","\1A00","dbSkip( \1A00 )",&sC___179 };
   static COMMANDS sC___181 = {0,"SKIP","ALIAS \1A00","\1A00 -> ( dbSkip(1) )",&sC___180 };
   static COMMANDS sC___182 = {0,"SKIP","\1A00 ALIAS \1B00","\1B00 -> ( dbSkip(\1A00) )",&sC___181 };
   static COMMANDS sC___183 = {0,"SEEK","\1A00 \16\1B20 SOFTSEEK>\17","dbSeek( \1A00, IIF( \1B50, .T., NIL ) )",&sC___182 };
   static COMMANDS sC___184 = {0,"FIND","\1A30","dbSeek( \1A30 )",&sC___183 };
   static COMMANDS sC___185 = {0,"FIND",":= \1A00","( find := \1A00 )",&sC___184 };
   static COMMANDS sC___186 = {0,"FIND","= \1A00","( find := \1A00 )",&sC___185 };
   static COMMANDS sC___187 = {0,"CONTINUE","","__dbContinue()",&sC___186 };
   static COMMANDS sC___188 = {0,"LOCATE","\16FOR \1A00\17 \16WHILE \1B00\17 \16NEXT \1C00\17 \16RECORD \1D00\17 \16\1E20REST>\17 \16ALL\17",
       "__dbLocate(\1A40,\1B40,\1C00,\1D00,\1E50)",&sC___187 };
   static COMMANDS sC___189 = {0,"SET","RELATION TO","dbClearRelation()",&sC___188 };
   static COMMANDS sC___190 = {0,"SET","RELATION \16\1A20 ADDITIVE>\17 \16TO \1C00 INTO \1D40 \16\1B20 SCOPED>\17 \16,\16TO\17 \1E00 INTO \1F40 \16\1G20 SCOPED>\17\17\17",
       "if (!\1A50 ) ; dbClearRelation() ; end ; dbSetRelation(\1D30,\1C40,\1C20,\1B50 ) \16; dbSetRelation(\1F30,\1E40,\1E20,\1G50 )\17",&sC___189 };
   static COMMANDS sC___191 = {0,"SET","FILTER TO","dbClearFilter(NIL)",&sC___190 };
   static COMMANDS sC___192 = {0,"SET","FILTER TO \1A00","dbSetFilter( \1A40, \1A20 )",&sC___191 };
   static COMMANDS sC___193 = {0,"SET","FILTER TO \1A20&>",
       "if ( Empty(\1A30) ) ;    dbClearFilter() ; else ;    dbSetFilter(\1A40,\1A30) ; end",&sC___192 };
   static COMMANDS sC___194 = {0,"REPLACE","\16 \1A00 WITH \1B00 \16, \1C00 WITH \1D00\17 \17 \16FOR \1E00\17 \16WHILE \1F00\17 \16NEXT \1G00\17 \16RECORD \1H00\17 \16\1I20REST>\17 \16ALL\17",
       "DBEval( {|| _FIELD->\1A00 := \1B00 \16,_FIELD->\1C00 := \1D00\17},\1E40,\1F40,\1G00,\1H00,\1I50)",&sC___193 };
   static COMMANDS sC___195 = {0,"REPLACE","\1A00 WITH \1B00 \16, \1C00 WITH \1D00 \17",
       "_FIELD->\1A00 := \1B00 \16; _FIELD->\1C00 := \1D00\17",&sC___194 };
   static COMMANDS sC___196 = {0,"DELETE","\16FOR \1A00\17 \16WHILE \1B00\17 \16NEXT \1C00\17 \16RECORD \1D00\17 \16\1E20REST>\17 \16ALL\17",
       "DBEval( {|| dbDelete()}, \1A40, \1B40, \1C00, \1D00, \1E50 )",&sC___195 };
   static COMMANDS sC___197 = {0,"RECALL","\16FOR \1A00\17 \16WHILE \1B00\17 \16NEXT \1C00\17 \16RECORD \1D00\17 \16\1E20REST>\17 \16ALL\17",
       "DBEval( {|| dbRecall()}, \1A40, \1B40, \1C00, \1D00, \1E50 )",&sC___196 };
   static COMMANDS sC___198 = {0,"DELETE","","dbDelete()",&sC___197 };
   static COMMANDS sC___199 = {0,"RECALL","","dbRecall()",&sC___198 };
   static COMMANDS sC___200 = {0,"CREATE","\1A40 \16FROM \1B40\17 \16VIA \1C00\17 \16ALIAS \1D00\17 \16\1E20 NEW>\17 \16CODEPAGE \1F00\17 \16CONNECTION \1G00\17",
       "__dbCreate( \1A30, \1B30, \1C00, \1E50, \1D30, \1F30, \1G00 )",&sC___199 };
   static COMMANDS sC___201 = {0,"COPY","\16STRUCTURE\17 \16EXTENDED\17 \16TO \1A40\17","__dbCopyXStruct( \1A30 )",&sC___200 };
   static COMMANDS sC___202 = {0,"COPY","\16STRUCTURE\17 \16TO \1A40\17 \16FIELDS \1B10\17","__dbCopyStruct( \1A30, { \1B30 } )",&sC___201 };
   static COMMANDS sC___203 = {0,"COPY","\16TO \1A40\17 \16DELIMITED \16WITH \1B30\17\17 \16FIELDS \1C10\17 \16FOR \1D00\17 \16WHILE \1E00\17 \16NEXT \1F00\17 \16RECORD \1G00\17 \16\1H20REST>\17 \16CODEPAGE \1I00\17 \16ALL\17",
       "__dbDelim( .T., \1A30, \1B30, { \1C30 }, \1D40, \1E40, \1F00, \1G00, \1H50, \1I30 )",&sC___202 };
   static COMMANDS sC___204 = {0,"COPY","\16TO \1A40\17 \16SDF\17 \16FIELDS \1B10\17 \16FOR \1C00\17 \16WHILE \1D00\17 \16NEXT \1E00\17 \16RECORD \1F00\17 \16\1G20REST>\17 \16CODEPAGE \1H00\17 \16ALL\17",
       "__dbSDF( .T., \1A30, { \1B30 }, \1C40, \1D40, \1E00, \1F00, \1G50, \1H30 )",&sC___203 };
   static COMMANDS sC___205 = {0,"COPY","\16TO \1A40\17 \16FIELDS \1B10\17 \16FOR \1C00\17 \16WHILE \1D00\17 \16NEXT \1E00\17 \16RECORD \1F00\17 \16\1G20REST>\17 \16VIA \1H00\17 \16CODEPAGE \1I00\17 \16CONNECTION \1J00\17 \16ALL\17",
       "__dbCopy( \1A30, { \1B30 }, \1C40, \1D40, \1E00, \1F00, \1G50, \1H00, \1J00, \1I30)",&sC___204 };
   static COMMANDS sC___206 = {0,"APPEND","\16FROM \1A40\17 \16DELIMITED \16WITH \1B30\17\17 \16FIELDS \1C10\17 \16FOR \1D00\17 \16WHILE \1E00\17 \16NEXT \1F00\17 \16RECORD \1G00\17 \16\1H20REST>\17 \16CODEPAGE \1I00\17 \16ALL\17",
       "__dbDelim( .F., \1A30, \1B30, { \1C30 }, \1D40, \1E40, \1F00, \1G00, \1H50, \1I30 )",&sC___205 };
   static COMMANDS sC___207 = {0,"APPEND","\16FROM \1A40\17 \16SDF\17 \16FIELDS \1B10\17 \16FOR \1C00\17 \16WHILE \1D00\17 \16NEXT \1E00\17 \16RECORD \1F00\17 \16\1G20REST>\17 \16CODEPAGE \1H00\17 \16ALL\17",
       "__dbSDF( .F., \1A30, { \1B30 }, \1C40, \1D40, \1E00, \1F00, \1G50, \1H30 )",&sC___206 };
   static COMMANDS sC___208 = {0,"APPEND","\16FROM \1A40\17 \16FIELDS \1B10\17 \16FOR \1C00\17 \16WHILE \1D00\17 \16NEXT \1E00\17 \16RECORD \1F00\17 \16\1G20REST>\17 \16VIA \1H00\17 \16CODEPAGE \1I00\17 \16CONNECTION \1J00\17 \16ALL\17",
       "__dbApp( \1A30, { \1B30 }, \1C40, \1D40, \1E00, \1F00, \1G50, \1H00, \1J00, \1I30 )",&sC___207 };
   static COMMANDS sC___209 = {0,"SORT","\16TO \1A40\17 \16ON \1B10\17 \16FOR \1C00\17 \16WHILE \1D00\17 \16NEXT \1E00\17 \16RECORD \1F00\17 \16\1G20REST>\17 \16VIA \1H00\17 \16CODEPAGE \1I00\17 \16CONNECTION \1J00\17 \16ALL\17",
       "__dbSort( \1A30, { \1B30 }, \1C40, \1D40, \1E00, \1F00, \1G50, \1H00, \1J00, \1I30 )",&sC___208 };
   static COMMANDS sC___210 = {0,"TOTAL","\16TO \1A40\17 \16ON \1B00\17 \16FIELDS \1C10\17 \16FOR \1D00\17 \16WHILE \1E00\17 \16NEXT \1F00\17 \16RECORD \1G00\17 \16\1H20REST>\17 \16VIA \1I00\17 \16CODEPAGE \1J00\17 \16CONNECTION \1K00\17 \16ALL\17",
       "__dbTotal( \1A30, \1B40, { \1C30 }, \1D40, \1E40, \1F00, \1G00, \1H50, \1I00, \1K00, \1J30 )",&sC___209 };
   static COMMANDS sC___211 = {0,"UPDATE","\16FROM \1A40\17 \16ON \1B00\17 \16REPLACE \1C00 WITH \1D00 \16, \1E00 WITH \1F00\17\17 \16\1G20RANDOM>\17",
       "__dbUpdate( \1A30, \1B40, \1G50, {|| _FIELD->\1C00 := \1D00 \16, _FIELD->\1E00 := \1F00\17} )",&sC___210 };
   static COMMANDS sC___212 = {0,"JOIN","\16WITH \1A40\17 \16TO \1B00\17 \16FIELDS \1C10\17 \16FOR \1D00\17 \16VIA \1E00\17 \16CODEPAGE \1F00\17 \16CONNECTION \1G00\17",
       "__dbJoin( \1A30, \1B30, { \1C30 }, \1D40, \1E00, \1G00, \1F30 )",&sC___211 };
   static COMMANDS sC___213 = {0,"COUNT","\16TO \1A00\17 \16FOR \1B00\17 \16WHILE \1C00\17 \16NEXT \1D00\17 \16RECORD \1E00\17 \16\1F20REST>\17 \16ALL\17",
       "\1A00 := 0 ; DBEval( {|| \1A00++}, \1B40, \1C40, \1D00, \1E00, \1F50 )",&sC___212 };
   static COMMANDS sC___214 = {0,"SUM","\16 \1A00 \16, \1B00\17  TO  \1C00 \16, \1D00\17 \17 \16FOR \1E00\17 \16WHILE \1F00\17 \16NEXT \1G00\17 \16RECORD \1H00\17 \16\1I20REST>\17 \16ALL\17",
       "\1C00 := \16 \1D00 := \17 0 ; DBEval( {|| \1C00 += \1A00 \16, \1D00 += \1B00 \17}, \1E40, \1F40, \1G00, \1H00, \1I50 )",&sC___213 };
   static COMMANDS sC___215 = {0,"AVERAGE","\16 \1A00 \16, \1B00\17  TO  \1C00 \16, \1D00\17 \17 \16FOR \1E00\17 \16WHILE \1F00\17 \16NEXT \1G00\17 \16RECORD \1H00\17 \16\1I20REST>\17 \16ALL\17",
       "M->__Avg := \1C00 := \16 \1D00 := \17 0 ; DBEval( {|| M->__Avg := M->__Avg + 1, \1C00 := \1C00 + \1A00 \16, \1D00 := \1D00 + \1B00\17 }, \1E40, \1F40, \1G00, \1H00, \1I50 ) ; \1C00 := \1C00 / M->__Avg \16; \1D00 := \1D00 / M->__Avg \17",&sC___214 };
   static COMMANDS sC___216 = {0,"LIST","\16\1A10\17 \16\1B20OFF>\17 \16\1C20 TO PRINTER>\17 \16TO FILE \1D40\17 \16FOR \1E00\17 \16WHILE \1F00\17 \16NEXT \1G00\17 \16RECORD \1H00\17 \16\1I20REST>\17 \16ALL\17",
       "__dbList( \1B50, { \1A40 }, .t., \1E40, \1F40, \1G00, \1H00, \1I50, \1C50, \1D30 )",&sC___215 };
   static COMMANDS sC___217 = {0,"DISPLAY","\16\1A10\17 \16\1B20OFF>\17 \16\1C20 TO PRINTER>\17 \16TO FILE \1D40\17 \16FOR \1E00\17 \16WHILE \1F00\17 \16NEXT \1G00\17 \16RECORD \1H00\17 \16\1I20REST>\17 \16\1J20ALL>\17",
       "__DBList( \1B50, { \1A40 }, \1J50, \1E40, \1F40, \1G00, \1H00, \1I50, \1C50, \1D30 )",&sC___216 };
   static COMMANDS sC___218 = {0,"REPORT","FORM \1A00 \16HEADING \1B00\17 \16\1C20 PLAIN>\17 \16\1D20 NOEJECT>\17 \16\1E20 SUMMARY>\17 \16\1F20 NOCONSOLE>\17 \16\1G20 TO PRINTER>\17 \16TO FILE \1H40\17 \16FOR \1I00\17 \16WHILE \1J00\17 \16NEXT \1K00\17 \16RECORD \1L00\17 \16\1M20REST>\17 \16ALL\17",
       "__ReportForm( \1A30, \1G50, \1H30, \1F50, \1I40, \1J40, \1K00, \1L00, \1M50, \1C50, \1B00, \1D50, \1E50 )",&sC___217 };
   static COMMANDS sC___219 = {0,"LABEL","FORM \1A00 \16\1B20 SAMPLE>\17 \16\1C20 NOCONSOLE>\17 \16\1D20 TO PRINTER>\17 \16TO FILE \1E40\17 \16FOR \1F00\17 \16WHILE \1G00\17 \16NEXT \1H00\17 \16RECORD \1I00\17 \16\1J20REST>\17 \16ALL\17",
       "__LabelForm( \1A30, \1D50, \1E30, \1C50, \1F40, \1G40, \1H00, \1I00, \1J50, \1B50 )",&sC___218 };
   static COMMANDS sC___220 = {0,"CLOSE","\1A00","\1A00->( dbCloseArea() )",&sC___219 };
   static COMMANDS sC___221 = {0,"CLOSE","","dbCloseArea()",&sC___220 };
   static COMMANDS sC___222 = {0,"CLOSE","DATABASES","dbCloseAll()",&sC___221 };
   static COMMANDS sC___223 = {0,"CLOSE","ALTERNATE","Set(_SET_ALTFILE, "")",&sC___222 };
   static COMMANDS sC___224 = {0,"CLOSE","FORMAT","__SetFormat(NIL)",&sC___223 };
   static COMMANDS sC___225 = {0,"CLOSE","INDEXES","dbClearIndex()",&sC___224 };
   static COMMANDS sC___226 = {0,"CLOSE","PROCEDURE",NULL,&sC___225 };
   static COMMANDS sC___227 = {0,"CLOSE","ALL","CLOSE DATABASES ; SELECT 1 ; CLOSE FORMAT",&sC___226 };
   static COMMANDS sC___228 = {0,"CLEAR","","CLEAR SCREEN ; CLEAR GETS",&sC___227 };
   static COMMANDS sC___229 = {0,"CLEAR","ALL",
       "CLOSE DATABASES ; CLOSE FORMAT ; CLEAR MEMORY ; CLEAR GETS ; SET ALTERNATE OFF ; SET ALTERNATE TO",&sC___228 };
#ifdef HB_EXTENSION
   static COMMANDS sC___230 = {0,"INDEX","ON \1A00 \16TAG \1B40\17 TO \1C40 \16FOR \1D00\17 \16\1E20ALL>\17 \16WHILE \1F00\17 \16NEXT \1G00\17 \16RECORD \1H00\17 \16\1I20REST>\17 \16EVAL \1J00\17 \16EVERY \1K00\17 \16\1L20 UNIQUE>\17 \16\1M20 ASCENDING>\17 \16\1N20 DESCENDING>\17 \16\1O20 USECURRENT>\17 \16\1P20 ADDITIVE>\17 \16\1R20 CUSTOM>\17 \16\1S20 NOOPTIMIZE>\17 \16\1T20  MEMORY>\17 \16\1T20 TEMPORARY>\17 \16\1U20 FILTERON>\17 \16\1V20 EXCLUSIVE>\17 \16CONSTRAINT \1W00\17 \16TARGET \1X00\17 \16KEY \1Y10\17",
       "ordCondSet( \1D20, \1D40, \16\1E50\17, \1F40, \1J40, \1K00, RECNO(), \1G00, \1H00, \16\1I50\17, \16\1N50\17,, \16\1P50\17, \16\1O50\17, \16\1R50\17, \16\1S50\17, \1F20, \16\1T50\17, \16\1U50\17, \16\1V50\17 ) ;  ordCreate(\1C30, \1B30, \1A20, \1A40, \16\1L50\17, \16\1W30\17, \16\1X30\17, { \16\1Y30\17 } )",&sC___229 };
   static COMMANDS sC___231 = {0,"INDEX","ON \1A00 TAG \1B40 \16TO \1C40\17 \16FOR \1D00\17 \16\1E20ALL>\17 \16WHILE \1F00\17 \16NEXT \1G00\17 \16RECORD \1H00\17 \16\1I20REST>\17 \16EVAL \1J00\17 \16EVERY \1K00\17 \16\1L20 UNIQUE>\17 \16\1M20 ASCENDING>\17 \16\1N20 DESCENDING>\17 \16\1O20 USECURRENT>\17 \16\1P20 ADDITIVE>\17 \16\1R20 CUSTOM>\17 \16\1S20 NOOPTIMIZE>\17 \16\1T20  MEMORY>\17 \16\1T20 TEMPORARY>\17 \16\1U20 FILTERON>\17 \16\1V20 EXCLUSIVE>\17 \16CONSTRAINT \1W00\17 \16TARGET \1X00\17 \16KEY \1Y10\17",
       "ordCondSet( \1D20, \1D40, \16\1E50\17, \1F40, \1J40, \1K00, RECNO(), \1G00, \1H00, \16\1I50\17, \16\1N50\17,, \16\1P50\17, \16\1O50\17, \16\1R50\17, \16\1S50\17, \1F20, \16\1T50\17, \16\1U50\17, \16\1V50\17 ) ;  ordCreate(\1C30, \1B30, \1A20, \1A40, \16\1L50\17, \16\1W30\17, \16\1X30\17, { \16\1Y30\17 } )",&sC___230 };
#else
   static COMMANDS sC___230 = {0,"INDEX","ON \1A00 \16TAG \1B40\17 TO \1C40 \16FOR \1D00\17 \16\1E20ALL>\17 \16WHILE \1F00\17 \16NEXT \1G00\17 \16RECORD \1H00\17 \16\1I20REST>\17 \16EVAL \1J00\17 \16EVERY \1K00\17 \16\1L20 UNIQUE>\17 \16\1M20 ASCENDING>\17 \16\1N20 DESCENDING>\17 \16\1O20 USECURRENT>\17 \16\1P20 ADDITIVE>\17 \16\1R20 CUSTOM>\17 \16\1S20 NOOPTIMIZE>\17",
       "ordCondSet( \1D20, \1D40, \16\1E50\17, \1F40, \1J40, \1K00, RECNO(), \1G00, \1H00, \16\1I50\17, \16\1N50\17,, \16\1P50\17, \16\1O50\17, \16\1R50\17, \16\1S50\17, \1F20 ) ;  ordCreate(\1C30, \1B30, \1A20, \1A40, \16\1L50\17 )",&sC___229 };
   static COMMANDS sC___231 = {0,"INDEX","ON \1A00 TAG \1B40 \16TO \1C40\17 \16FOR \1D00\17 \16\1E20ALL>\17 \16WHILE \1F00\17 \16NEXT \1G00\17 \16RECORD \1H00\17 \16\1I20REST>\17 \16EVAL \1J00\17 \16EVERY \1K00\17 \16\1L20 UNIQUE>\17 \16\1M20 ASCENDING>\17 \16\1N20 DESCENDING>\17 \16\1O20 USECURRENT>\17 \16\1P20 ADDITIVE>\17 \16\1R20 CUSTOM>\17 \16\1S20 NOOPTIMIZE>\17",
       "ordCondSet( \1D20, \1D40, \16\1E50\17, \1F40, \1J40, \1K00, RECNO(), \1G00, \1H00, \16\1I50\17, \16\1N50\17,, \16\1P50\17, \16\1O50\17, \16\1R50\17, \16\1S50\17, \1F20 ) ;  ordCreate(\1C30, \1B30, \1A20, \1A40, \16\1L50\17 )",&sC___230 };
#endif
   static COMMANDS sC___232 = {0,"INDEX","ON \1A00 TO \1B40 \16\1C20 UNIQUE>\17", "dbCreateIndex( \1B30, \1A20, \1A40, IIF( \1C50, .t., NIL ) )",&sC___231 };
   static COMMANDS sC___233 = {0,"DELETE","TAG \1A40 \16, \1B40 \17", "ordDestroy( \1A30 ) \16; ordDestroy( \1B30 ) \17",&sC___232 };
   static COMMANDS sC___234 = {0,"DELETE","TAG \1A40 IN \1B40 \16, \1C40 IN \1D40 \17", "ordDestroy( \1A30, \1B30 ) \16; ordDestroy( \1C30, \1D30 ) \17",&sC___233 };
   static COMMANDS sC___235 = {0,"REINDEX","\16EVAL \1A00\17 \16EVERY \1B00\17", "ordCondSet(,,,, \1A40, \1B00,,,,,,,) ;  ordListRebuild()",&sC___234 };
   static COMMANDS sC___236 = {0,"REINDEX","","ordListRebuild()",&sC___235 };
   static COMMANDS sC___237 = {0,"SET","INDEX TO \16 \1A40 \16, \1B40\17\17 \16\1C20 ADDITIVE>\17", "if !\1C50 ; ordListClear() ; end \16; ordListAdd( \1A30 )\17 \16; ordListAdd( \1B30 )\17",&sC___236 };
   static COMMANDS sC___238 = {0,"SET","ORDER TO \1A00","ordSetFocus( \1A00 )",&sC___237 };
   static COMMANDS sC___239 = {0,"SET","ORDER TO TAG \1A40","ordSetFocus( \1A30 )",&sC___238 };
   static COMMANDS sC___240 = {0,"SET","ORDER TO \1A00 IN \1B40","ordSetFocus( \1A00, \1B30 )",&sC___239 };
   static COMMANDS sC___241 = {0,"SET","ORDER TO TAG \1A40 IN \1B40","ordSetFocus( \1A30, \1B30 )",&sC___240 };
   static COMMANDS sC___242 = {0,"SET","ORDER TO","ordSetFocus(0)",&sC___241 };
   static COMMANDS sC___243 = {0,"SET","EVENTMASK TO \1A00","Set( _SET_EVENTMASK, \1A00 )",&sC___242 };
   static COMMANDS sC___244 = {0,"SET","OPTIMIZE \1A20ON,OFF,&>","Set( _SET_OPTIMIZE, \1A30 )",&sC___243 };
   static COMMANDS sC___245 = {0,"SET","OPTIMIZE (\1A00)","Set( _SET_OPTIMIZE, \1A00 )",&sC___244 };
   static COMMANDS sC___246 = {0,"SET","TRACE \1A20ON,OFF,&>","Set( _SET_TRACE, \1A30 )",&sC___245 };
   static COMMANDS sC___247 = {0,"SET","AUTOPEN \1A20 ON,OFF,&>","Set(_SET_AUTOPEN,\1A30 )",&sC___246 };
   static COMMANDS sC___248 = {0,"SET","AUTOPEN (\1A00)","Set(_SET_AUTOPEN,\1A00 )",&sC___247 };
   static COMMANDS sC___249 = {0,"SET","FILECASE \1A30","Set(_SET_FILECASE, \1A10 )",&sC___248 };
   static COMMANDS sC___250 = {0,"SET","FILECASE ( \1A00 )","Set(_SET_FILECASE, \1A00 )",&sC___249 };
   static COMMANDS sC___251 = {0,"SET","DIRCASE \1A30","Set(_SET_DIRCASE, \1A10 )",&sC___250 };
   static COMMANDS sC___252 = {0,"SET","DIRCASE ( \1A00 )","Set(_SET_DIRCASE, \1A00 )",&sC___251 };
   static COMMANDS sC___253 = {0,"SET","DIRSEPARATOR \1A30","Set(_SET_DIRSEPARATOR, \1A10 )",&sC___252 };
   static COMMANDS sC___254 = {0,"SET","DIRSEPARATOR ( \1A00 )","Set(_SET_DIRSEPARATOR, \1A00 )",&sC___253 };
   static COMMANDS sC___255 = {0,"SET","ERRORLOOP TO \1A00","Set(_SET_ERRORLOOP, \1A00 )",&sC___254 };
   static COMMANDS sC___256 = {0,"SET","ERRORLOOP TO","Set(_SET_ERRORLOOP, 0 )",&sC___255 };
   static COMMANDS sC___257 = {0,"SET","DBFLOCKSCHEME TO \1A00","Set(_SET_DBFLOCKSCHEME, \1A00 )",&sC___256 };
   static COMMANDS sC___258 = {0,"SET","DBFLOCKSCHEME TO","Set(_SET_DBFLOCKSCHEME, 0 )",&sC___257 };
   static COMMANDS sC___259 = {0,"SET","ERRORLOG TO \1A40 \16\1B20 ADDITIVE>\17","Set( _SET_ERRORLOG, \1A30, \1B50 )",&sC___258 };
   static COMMANDS sC___260 = {0,"TEXT","INTO \1A00", "\1A00 := \"\"; HB_SetWith({|_1| \1A00 += (_1 + HB_OSNewLine() ) } ) ; text __TextInto,HB_SetWith",&sC___259 };

   /*
    * NOTE: below section within #ifdef - MUST update hb_pp_topCommand in 2 places AND
    * MODIFY the sC___ numbers in the #ifdef!!!
    */

#if defined( HB_COMPAT_C53 )
   static COMMANDS sC___261 = {0,"@","\1A00,\1B00 GET \1C00 CHECKBOX \16VALID \1D00\17 \16WHEN \1E00\17 \16CAPTION \1F00\17 \16MESSAGE \1G00\17 \16COLOR \1H00\17 \16FOCUS \1I00\17 \16STATE \1J00\17 \16STYLE \1K00\17 \16SEND \1L00\17 \16GUISEND \1M00\17",
       "SetPos(\1A00,\1B00 ) ; AAdd(GetList,_GET_(\1C00,\1C30,NIL,\1D40,\1E40 ) ) ; ATail(GetList):Control := _CheckBox_(\1C00,\1F00,\1G00,\1H00,\1I40,\1J40,\1K00 ) ; ATail(GetList):reader := { | a,b,c,d | GuiReader(a,b,c,d ) } \16; ATail(GetList):\1L00\17 \16; ATail(GetList):Control:\1M00\17 ; ATail(GetList):Control:Display()",&sC___260 };
   static COMMANDS sC___262 = {0,"@","\1A00,\1B00,\1C00,\1D00 GET \1E00 RADIOGROUP \1F00 \16VALID \1G00\17 \16WHEN \1H00\17 \16CAPTION \1I00\17 \16MESSAGE \1J00\17 \16COLOR \1K00\17 \16FOCUS \1L00\17 \16STYLE \1M00\17 \16SEND \1N00\17 \16GUISEND \1O00\17",
       "SetPos(\1A00,\1B00 ) ; AAdd(GetList,_GET_(\1E00,\1E30,NIL,\1G40,\1H40 ) ) ; ATail(GetList):Control := _RadioGrp_(ATail(Getlist):row,ATail(Getlist):col,\1C00,\1D00,\1E00,\1F00,\1I00,\1J00,\1K00,\1L40,\1M00 ) ; ATail(GetList):reader := { | a,b,c,d | GuiReader(a,b,c,d ) } \16; ATail(GetList):\1N00\17 \16; ATail(GetList):Control:\1O00\17 ; ATail(GetList):Control:Display()",&sC___261 };
   static COMMANDS sC___263 = {0,"@","\1A00,\1B00 GET \1C00 PUSHBUTTON \16VALID \1D00\17 \16WHEN \1E00\17 \16CAPTION \1F00\17 \16MESSAGE \1G00\17 \16COLOR \1H00\17 \16FOCUS \1I00\17 \16STATE \1J00\17 \16STYLE \1K00\17 \16SEND \1L00\17 \16GUISEND \1M00\17",
       "SetPos(\1A00,\1B00 ) ; AAdd(GetList,_GET_(\1C00,\1C30,NIL,\1D40,\1E40 ) ) ; ATail(GetList):Control := _PushButt_(\1F00,\1G00,\1H00,\1I40,\1J40,\1K00,) ; ATail(GetList):reader := { | a,b,c,d | GuiReader(a,b,c,d ) } \16; ATail(GetList):\1L00\17 \16; ATail(GetList):Control:\1M00\17 ; ATail(GetList):Control:Display()",&sC___262 };
   static COMMANDS sC___264 = {0,"@","\1A00,\1B00,\1C00,\1D00 GET \1E00 LISTBOX \1F00 \16VALID \1G00\17 \16WHEN \1H00\17 \16CAPTION \1I00\17 \16MESSAGE \1J00\17 \16COLOR \1K00\17 \16FOCUS \1L00\17 \16STATE \1M00\17 \16\1N20  DROPDOWN>\17 \16\1O20  SCROLLBAR>\17 \16SEND \1P00\17 \16GUISEND \1Q00\17",
       "SetPos(\1A00,\1B00 ) ; AAdd(GetList,_GET_(\1E00,\1E30,NIL,\1G40,\1H40 ) ) ; ATail(GetList):Control := _ListBox_(ATail(Getlist):row,ATail(Getlist):col,\1C00,\1D00,\1E00,\1F00,\1I00,\1J00,\1K00,\1L40,\1M40,\1N50,\1O50 ) ; ATail(GetList):reader := { | a,b,c,d | GuiReader(a,b,c,d ) } \16; ATail(GetList):\1P00\17 \16; ATail(GetList):Control:\1Q00\17 ; ATail(GetList):Control:Display()",&sC___263 };
   static COMMANDS sC___265 = {0,"@","\1A00,\1B00,\1C00,\1D00 GET \1E00 TBROWSE \1F00 \16VALID \1G00\17 \16WHEN \1H00\17 \16MESSAGE \1I00\17 \16SEND \1J00\17 \16GUISEND \1K00\17",
       "SetPos(\1A00,\1B00 ) ; AAdd(GetList,_GET_(\1E00,\1E30,NIL,\1G40,\1H40 ) ) ; \1F00:ntop := ATail(Getlist):row ; \1F00:nleft := ATail(Getlist):col ; \1F00:nbottom := \1C00 ; \1F00:nright := \1D00 ; \1F00:Configure() ; ATail(GetList):Control := \1F00 ; ATail(GetList):reader := { | a,b,c,d | TBReader( a, b, c, d ) } \16; ATail(GetList):Control:Message := \1I00\17 \16; ATail(GetList):\1J00\17 \16; ATail(GetList):Control:\1K00\17",&sC___264 };
   static COMMANDS sC___266 = {0,"READ","\16MSG AT \1A00,\1B00,\1C00 \16MSG COLOR \1D00\17\17",
       "ReadModal(GetList,NIL,NIL,\1A00,\1B00,\1C00,\1D00 ) ; GetList := {}",&sC___265 };
   static COMMANDS sC___267 = {0,"@","\1A00,\1B00 GET \1C00 \16PICTURE \1D00\17 \16VALID \1E00\17 \16WHEN \1F00\17 \16CAPTION \1G00\17 \16MESSAGE \1H00\17 \16SEND \1I00\17",
       "SetPos(\1A00,\1B00 ) ; AAdd(GetList,_GET_(\1C00,\1C20,\1D00,\1E40,\1F40 ) ) \16; ATail(GetList):Caption := \1G00\17 \16; ATail(GetList):CapRow := ATail(Getlist):row ; ATail(GetList):CapCol := ATail(Getlist):col - __CapLength(\1G00) - 1\17 \16; ATail(GetList):message := \1H00\17 \16; ATail(GetList):\1I00\17 ; ATail(GetList):Display()",&sC___266 };
   static COMMANDS sC___268 = {0,"SET","MBLOCKSIZE TO \1A00","Set( _SET_MBLOCKSIZE, \1A00 )",&sC___267 };
   static COMMANDS sC___269 = {0,"SET","MEMOBLOCK TO \1A00","Set( _SET_MBLOCKSIZE, \1A00 )",&sC___268 };
   static COMMANDS sC___270 = {0,"SET","MFILEEXT TO \1A00","Set( _SET_MFILEEXT, \1A00 )",&sC___269 };
   static COMMANDS sC___271 = {0,"SET","AUTOSHARE TO \1A00","Set( _SET_AUTOSHARE, \1A00 )",&sC___270 };
   static COMMANDS sC___272 = {0,"SET","AUTOSHARE TO","Set( _SET_AUTOSHARE, 0 )",&sC___271 };
   static COMMANDS sC___273 = {0,"SET","AUTORDER TO \1A00","Set( _SET_AUTORDER, \1A00 )",&sC___272 };
   static COMMANDS sC___274 = {0,"SET","AUTORDER TO","Set( _SET_AUTORDER, 0 )",&sC___273 };
   static COMMANDS sC___275 = {0,"SET","STRICTREAD \1A20 ON,OFF,&>","Set(_SET_STRICTREAD,\1A30 )",&sC___274 };
   static COMMANDS sC___276 = {0,"SET","STRICTREAD (\1A00)","Set(_SET_STRICTREAD,\1A00 )",&sC___275 };
   static COMMANDS sC___277 = {0,"SET","OUTPUT SAFETY \1A20 ON,OFF,&>","Set(_SET_OUTPUTSAFETY,\1A30 )",&sC___276 };
   static COMMANDS sC___278 = {0,"SET","BACKGROUND TASKS \1A20 ON,OFF,&>","Set(_SET_BACKGROUNDTASKS,\1A30 )",&sC___277 };
   static COMMANDS sC___279 = {0,"SET","GTMODE TO \1A30","Set( _SET_GTMODE,\1A30 )",&sC___278 };
   static COMMANDS sC___280 = {0,"SET","BACKGROUNDTICK \1A00","Set( _SET_BACKGROUNDTICK, \1A00 )",&sC___279 };
   static COMMANDS sC___281 = {0,"SET","HARDCOMMIT \1A20 ON,OFF,&>","Set(_SET_HARDCOMMIT,\1A30 )",&sC___280 };
   static COMMANDS sC___282 = {0,"SET","HARDCOMMIT (\1A00)","Set(_SET_HARDCOMMIT,\1A00 )",&sC___281 };
   static COMMANDS sC___283 = {0,"SET","FORCE OPTIMIZATION \1A20 ON,OFF,&>","Set(_SET_FORCEOPT,\1A30 )",&sC___282 };
   static COMMANDS sC___284 = {0,"SET","EOL \1A00","Set( _SET_EOL, \1A00 )",&sC___283 };
#endif

   static COMMANDS sT___01 = {1,"_GET_","(\1A00,\1B00 )","_GET_(\1A00,\1B00,NIL,NIL,NIL )",NULL };
   static COMMANDS sT___02 = {1,"_GET_","(\1A00,\1B00,\1C00 )","_GET_(\1A00,\1B00,\1C00,NIL,NIL )",&sT___01 };
   static COMMANDS sT___03 = {1,"_GET_","(\1A00,\1B00,\1C00,\1D00 )","_GET_(\1A00,\1B00,\1C00,\1D00,NIL )",&sT___02 };

#if defined( HB_COMPAT_C53 )
   hb_pp_topCommand = &sC___284;
#else
   hb_pp_topCommand = &sC___260;
#endif

   hb_pp_topDefine = &sD___72;
   hb_pp_topTranslate = &sT___03;
}

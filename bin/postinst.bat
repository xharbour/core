@echo off
rem
rem $Id: postinst.bat 9279 2011-02-14 18:06:32Z druzus $
rem

rem ---------------------------------------------------------------
rem Copyright 2003 Przemyslaw Czerpak [druzus@polbox.com]
rem simple script run after xHarbour make install to finish install
rem process
rem
rem See doc/license.txt for licensing terms.
rem ---------------------------------------------------------------

goto inst_%HB_ARCHITECTURE%

:inst_dos
rem DOS post install part

goto end


:inst_w32
rem Windows post install part

goto end


:inst_
:end

@echo off
rem
rem $Id: pdllvc.bat 9888 2012-12-21 02:34:46Z andijahja $
rem
rem �����������������������������������������������Ŀ
rem � * this is a batch file for creating pCode DLL ��
REM � * ENVIRONMENT SHOULD BE ADJUSTED ACCORDINGLY  ��
rem ��������������������������������������������������
rem  �������������������������������������������������
SET SUB_DIR=VC10
SET CC_DIR=..\..\%SUB_DIR%
SET _MYLIB=codepage.lib rddads.lib zlib.lib common.lib dbffpt.lib dbfcdx.lib dbfntx.lib hbsix.lib debug.lib gtwin.lib macro.lib pp.lib rdd.lib rtl.lib vm.lib lang.lib pcrepos.lib ct.lib
SET HARBOURDIR=..
SET HARBOURLIB=%HARBOURDIR%\lib
SET HARBOUREXE=%HARBOURDIR%\bin\harbour.exe
SET ____CFL=/c /TP /MT /I%CC_DIR%\INCLUDE /I%HARBOURDIR%\INCLUDE /Ot2yb1 /Gs /GA /W3 /nologo
SET _PATH=%PATH%
SET PATH=%CC_DIR%\BIN;%PATH%
SET DEFFILE=%HARBOURDIR%\source\vm\export.def

%HARBOUREXE% pexec.prg  /n0 /i%HARBOURDIR%\include /gc2 /es2 /w
%HARBOUREXE% pcode.prg  /n1 /i%HARBOURDIR%\include /gc2 /es2 /w
%HARBOUREXE% pcode1.prg /n1 /i%HARBOURDIR%\include /gc2 /es2 /w
CL %____CFL% pexec.c
CL %____CFL% pcode.c
CL %____CFL% pcode1.c
CL %____CFL% %HARBOURDIR%\source\vm\maindllp.c
IF ERRORLEVEL 1 GOTO end
LINK /DEF:%DEFFILE% /NOLOGO /LIBPATH:%HARBOURLIB% /LIBPATH:%CC_DIR%\LIB /OUT:pexec.exe /FORCE:MULTIPLE /INCLUDE:__matherr /SUBSYSTEM:CONSOLE pexec.obj %_MYLIB%
LINK /DLL /NOLOGO /LIBPATH:%HARBOURLIB% /LIBPATH:%CC_DIR%\LIB /OUT:pcode.dll  /FORCE:MULTIPLE /INCLUDE:__matherr /SUBSYSTEM:WINDOWS pcode.obj  maindllp.obj
LINK /DLL /NOLOGO /LIBPATH:%HARBOURLIB% /LIBPATH:%CC_DIR%\LIB /OUT:pcode1.dll /FORCE:MULTIPLE /INCLUDE:__matherr /SUBSYSTEM:WINDOWS pcode1.obj maindllp.obj

:end
SET PATH=%_PATH%
SET _PATH=
SET HARBOUREXE=
SET HARBOURLIB=
SET HARBOURDIR=
SET _MYLIB=
SET CC_DIR=
SET ____CFL=
SET DEFFILE=

rem @echo off
rem
rem $Id: make_c5x.bat 9280 2011-02-14 18:16:32Z druzus $
rem

if not exist ..\..\contrib\hbclip\hbclip.lib goto NO_HBCLIP

rem set hb_clipopt=/DHAVE_HBCLIP
set hb_linkopt=li hbclip
set lib=..\..\contrib\hbclip\;%lib%

:NO_HBCLIP

clipper hbtest.prg   /w /n /p /i..\..\include %hb_clipopt%
clipper rt_array.prg /w /n /p %hb_clipopt%
clipper rt_date.prg  /w /n /p %hb_clipopt%
clipper rt_file.prg  /w /n /p /i..\..\include %hb_clipopt%
clipper rt_hvm.prg   /w /n /p %hb_clipopt%
clipper rt_hvma.prg  /w /n /p %hb_clipopt%
clipper rt_math.prg  /w /n /p %hb_clipopt%
clipper rt_misc.prg  /w /n /p %hb_clipopt%
clipper rt_str.prg   /w /n /p %hb_clipopt%
clipper rt_stra.prg  /w /n /p %hb_clipopt%
clipper rt_trans.prg /w /n /p %hb_clipopt%

if     "%1" == "" set hb_linker=rtlink 
if not "%1" == "" set hb_linker=exospace


%hb_linker% out hbtest5x fi hbtest,rt_array,rt_date,rt_file,rt_hvm,rt_hvma,rt_math,rt_misc,rt_str,rt_stra,rt_trans %hb_linkopt%

rem del *.obj

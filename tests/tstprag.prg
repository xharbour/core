//NOTEST
/*
 * $Id: tstprag.prg 9279 2011-02-14 18:06:32Z druzus $
 */

#pragma TracePragmas=On
#pragma ExitSeverity=1

/* Unknow pragmas will be ignored silently */
#pragma BadPragma=off
#pragma /Y+

function Main()

#pragma Shortcut=On
#pragma Shortcut= Off
#pragma Shortcut = On
#pragma Shortcut(OFF)
#pragma Shortcut( On)
#pragma Shortcut( OFF )
#pragma Shortcut( On )
#pragma Shortcut( OFF )
#pragma Shortcut( ON

/* or #pragma /Z+ */

  if .t. .and. .f.
    ? "Always"
  endif

  if .f. .and. .t.
    ? "Never"
  endif

#pragma /Z-
/* or #pragma Shortcut=Off */

#pragma Exitseverity=0
#pragma Exitseverity=1
#pragma Exitseverity(0)
#pragma Exitseverity( 1 )
#pragma Exitseverity( 0 )
#pragma Exitseverity= 2
#pragma Exitseverity= 1

/* Pragmas with bad values will cause an error  */
#pragma WarningLevel=8

#pragma TEXTHIDDEN(1)
? "This is a hidden text"
#pragma TEXTHIDDEN(0)

return nil

/*
 * $Id: langapi.prg 9279 2011-02-14 18:06:32Z druzus $
 */

// ; Donated to the public domain by
//   Viktor Szakats <viktor.szakats@syenar.hu>

REQUEST HB_LANG_HU852
REQUEST HB_LANG_KO

func main()

? "Prev:", hb_langselect()
? hb_langName()
? NationMsg( 1 )
? CMonth( Date() )
? CDOW( Date() )
? "---------"

? "Prev:", hb_langSelect( "HU852" )
? hb_langName()
? NationMsg( 1 )
? CMonth( Date() )
? CDOW( Date() )
? "---------"

? "Prev:", hb_langSelect( "NOTHERE" )
? hb_langName()
? NationMsg( 1 )
? CMonth( Date() )
? CDOW( Date() )
? "---------"

? "Prev:", hb_langSelect( "KO" )
? hb_langName()
? NationMsg( 1 )
? CMonth( Date() )
? CDOW( Date() )
? "---------"

return nil


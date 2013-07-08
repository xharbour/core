REQUEST FCOMMA

PROC MAIN()

   USE test.csv VIA "FCOMMA"
   ? "ALIAS", ALIAS(), "RECNO", RECNO(), ;
     "BOF", BOF(), "EOF", EOF(), "LASTREC", LASTREC()
   ? RECNO(), '"' + FIELD->LINE + '"'
   DBGOBOTTOM()
   ? RECNO(), '"' + FIELD->LINE + '"'
   DBGOTOP()
   ? RECNO(), '"' + FIELD->LINE + '"'
   WAIT
   WHILE !EOF()
      ? RECNO(), '"' + FIELD->LINE + '"'
      IF RECNO()==20
         INKEY(0)
      ENDIF
      DBSKIP()
   ENDDO
   ? "ALIAS", ALIAS(), "RECNO", RECNO(), ;
     "BOF", BOF(), "EOF", EOF(), "LASTREC", LASTREC()
   WAIT
   DBGOBOTTOM()
   ? "ALIAS", ALIAS(), "RECNO", RECNO(), ;
     "BOF", BOF(), "EOF", EOF(), "LASTREC", LASTREC()
   WAIT
   WHILE !BOF()
      ? RECNO(), '[' + FIELD->LINE + ']'
      IF RECNO()==LASTREC()-20
         INKEY(0)
      ENDIF
      DBSKIP(-1)
   ENDDO
   ? "ALIAS", ALIAS(), "RECNO", RECNO(), ;
     "BOF", BOF(), "EOF", EOF(), "LASTREC", LASTREC()
   WAIT
   BROWSE()

RETURN

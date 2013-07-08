##################################
# $Id: Rules.make,v 1.10 2004/08/21 17:27:13 lf_sfnet Exp $
#
# Rules for making simwin
#
# USAGE:
# Include from subdirectories defining a TARGET
# that can be a binary file, a library file or
# a shared file.
# HARBOUR flags can be defined using variable PRGFLAGS
# C flags are CFLAGS ;-)
# Use LIBDIR and LIBS to define extra libs.
#
# If variable SOURCES has been defined, the list of files
# is taken from here, else all the files are taken from
# *.prg.
#

#Generic make options
LINKER = ar
CC = gcc
CFLAGS += -Wall -fms-extensions -I.
ifeq ($(HB_COMPILER),mingw32)
   CFLAGS += -mno-cygwin -mms-bitfields -mwindows
endif
LIBRARIAN = ranlib

ifeq ($(HB_MULTI_GT),yes)
   ifeq ($(HB_COMPILER),mingw32)
      GT_LIBS=-lgtnul -lgtwin
   else
      GT_LIBS=-lgtnul -lgtcrs  -lncurses
   endif
else
   GT_LIBS += -lgtcgi
endif

#libraries for binary building
ifeq ($(HB_MT),MT)
LIBFILES_ = -ldebug -lvmmt -lrtlmt $(GT_LIBS) -lrddmt -lrtlmt -lvmmt -lmacro -lppmt -ldbfntxmt -ldbfcdx -ldbfdbt -lcommon -lm -lpthread
else
LIBFILES_ = -ldebug -lvm -lrtl $(GT_LIBS)  -llang -lrdd -lrtl -lvm -lmacro -lpp -ldbfntx -ldbfcdx -ldbfdbt -lcommon -lm
endif

ifeq ($(HB_COMPILER),mingw32)
   LIBFILES_ += -luser32 -lwinspool -lole32 -loleaut32 -luuid -lgdi32 -lcomctl32 -lcomdlg32 -lodbc32
   EXETYPE=.exe
else
   LIBFILES_ += -lgpm
   EXETYPE=
endif

LIBDIR_ = $(LIBDIR) -L$(HB_LIB_INSTALL)
LIBS_=  $(LIBS) $(LIBFILES_)

ifeq ($(strip $(SOURCE_TYPE)),)
SOURCE_TYPE=prg
endif

#Sources / object determination rule
#subidr might override this file by providing a makefile.sources
ifeq ($(strip $(SOURCES)),)
SOURCES=$(wildcard *.$(SOURCE_TYPE))
endif

ifeq ($(strip $(OBJECTS)),)
OBJECTS=$(patsubst %.$(SOURCE_TYPE),%.o,$(SOURCES))
ifneq ($(strip $(CSOURCES)),)
OBJECTS+=$(patsubst %.c,%.o,$(CSOURCES))
endif
endif


#COMMANDS
all:$(TARGET) $(TARGETS)

.PHONY: clean install

%$(EXETYPE):%.o
	$(CC) -o$@ $< $(LIBDIR_) $(LIBS_)

%.o: %.c
	$(CC) -c -o$@ $(CFLAGS) -I$(HB_INC_INSTALL) $<

%.c: %.prg
	$(HB_BIN_INSTALL)/harbour -q0 -gc0 -w2 -n $(PRGFLAGS) -I$(HB_INC_INSTALL)  -o$@ $<

$(TARGET): $(OBJECTS)
ifeq ( lib , $(patsubst %.a, lib, $(TARGET)))
	$(LINKER) -r $(TARGET) $(OBJECTS)
	$(LIBRARIAN) $(TARGET)
else
	$(CC) -o $(TARGET) $(OBJECTS) $(LIBDIR_) $(LIBS_)
endif

clean:
	rm -f *.o
	rm -f *~
	rm -f *.ppo
	rm -f $(TARGET)
	rm -f $(TARGET).exe
	rm -f $(TARGETS)

install: all
	cp -f *.a $(XWT_INSTALL)

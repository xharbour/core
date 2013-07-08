#
# $Id: Makefile 9279 2011-02-14 18:06:32Z druzus $
#

ROOT = ./

DIRS=\
	include \
	source \
	utils{source} \
#	tests \
#	samples \

ifeq ($(HB_POSTINST),)

ifneq ($(HB_ROOTPOSTINST),)
HB_POSTINST = $(HB_ROOTPOSTINST) $(HB_POSTINSTPARAM)
else
ifneq ($(SHLVL),)
HB_POSTINST = $(TOP)$(ROOT)bin/postinst.sh $(HB_POSTINSTPARAM)
else
ifeq ($(HB_ARCHITECTURE),dos)
HB_POSTINST = $(subst /,\,$(TOP)$(ROOT)bin/postinst.bat) $(HB_POSTINSTPARAM)
else
ifeq ($(HB_ARCHITECTURE),w32)
HB_POSTINST = $(subst /,\,$(TOP)$(ROOT)bin/postinst.bat) $(HB_POSTINSTPARAM)
endif
endif
endif
endif

endif

include $(ROOT)config/dir.cf

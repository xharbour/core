#!/bin/sh
#
# $Id: xhbgtmk.sh 9279 2011-02-14 18:06:32Z druzus $
#

# ---------------------------------------------------------------
# Copyright 2003 Przemyslaw Czerpak <druzus@polbox.com>
# This script checks you have all tools to build xHarbour binaries
# installed then takes current xHarbour sources from SourceForge CVS
# and build binary RPMs at your local host
#
# See doc/license.txt for licensing terms.
# ---------------------------------------------------------------

# ssh is not necessary for anonymous access on SourceForge
# export CVS_RSH=ssh
export CVSROOT=":pserver:anonymous@xharbour.cvs.sourceforge.net:/cvsroot/xharbour"
export PROJECT=xharbour

test_reqrpm()
{
    rpm -q --whatprovides "$1" &> /dev/null
}

TOINST_LST=""
for i in cvs make gcc binutils bison bash ncurses ncurses-devel
do
    test_reqrpm "$i" || TOINST_LST="${TOINST_LST} $i"
done

_cvs_RSH="${CVS_RSH}"
[ -n "${_cvs_RSH}" ] || _cvs_RSH="rsh"

if ! which ${_cvs_RSH} &>/dev/null
then
    if [ "${_cvs_RSH}" = "ssh" ]
    then
	TOINST_LST="${TOINST_LST} [open]ssh-clients"
    else
	TOINST_LST="${TOINST_LST} ${_cvs_RSH}"
    fi
fi

if [ -z "${TOINST_LST}" ] || [ "$1" = "--force" ]
then
    cd
    mkdir -p CVS
    cd CVS
    if cvs -z3 co "${PROJECT}"; then
	cd "${PROJECT}"
	./make_rpm.sh "$*"
    fi
else
    echo "If you want to build xHarbour compilers"
    echo "you have to install the folowing RPM files:"
    echo "${TOINST_LST}"
    echo ""
    echo "If you want to force installation run this script with --force paramter:"
    echo "$0 --force"
fi

#!/bin/sh
#
# $Id: make_tgz.sh 9279 2011-02-14 18:06:32Z druzus $
#

# ---------------------------------------------------------------
# Copyright 2003 Przemyslaw Czerpak <druzus@polbox.com>
# simple script to build binaries .tgz from xHarbour sources
#
# See doc/license.txt for licensing terms.
# ---------------------------------------------------------------

cd `dirname $0`
. bin/hb-func.sh

name="xharbour"
hb_ver=`get_hbver`
hb_platform=`get_hbplatform`
[ "${hb_platform}" = "" ] || hb_platform="-${hb_platform}${HB_BUILDSUF}"
[ "${HB_XBUILD}" = "" ] || hb_platform="-${HB_XBUILD}"
hb_archfile="${name}-${hb_ver}${hb_platform}.bin.tar.gz"
hb_instfile="${name}-${hb_ver}${hb_platform}.inst.sh"
hb_lnkso="yes"
hb_pref="xhb"
hb_contrib=""
hb_sysdir="yes"
hb_exesuf=""
export C_USR="$C_USR -DHB_FM_STATISTICS_OFF -O3"

[ -z "$HB_INSTALL_PREFIX" ] && [ -n "$PREFIX" ] && export HB_INSTALL_PREFIX="$PREFIX"

if [ -z "$TMPDIR" ]; then TMPDIR="/tmp"; fi
HB_INST_PREF="$TMPDIR/$name.bin.$USER.$$"

if [ -z "$HB_ARCHITECTURE" ]; then
    if [ "$OSTYPE" = "msdosdjgpp" ]; then
        hb_arch="dos"
    else
        hb_arch=`uname -s | tr -d "[-]" | tr '[A-Z]' '[a-z]' 2>/dev/null`
        case "$hb_arch" in
            *windows*|*mingw32*|msys*)    hb_arch="w32" ;;
            *dos)   hb_arch="dos" ;;
            *bsd)   hb_arch="bsd" ;;
        esac
    fi
    export HB_ARCHITECTURE="$hb_arch"
fi

if [ -z "$HB_COMPILER" ]; then
    case "$HB_ARCHITECTURE" in
        w32) HB_COMPILER="mingw32" ;;
        dos) HB_COMPILER="djgpp" ;;
        *)   HB_COMPILER="gcc" ;;
    esac
    export HB_COMPILER
fi

if [ -z "$HB_GT_LIB" ]; then
    case "$HB_ARCHITECTURE" in
        w32) HB_GT_LIB="gtwin" ;;
        dos) HB_GT_LIB="gtdos" ;;
        os2) HB_GT_LIB="gtos2" ;;
        *)   HB_GT_LIB="gttrm" ;;
    esac
    export HB_GT_LIB
fi

if [ -z "$HB_MT" ]; then
    case "$HB_ARCHITECTURE" in
        dos) HB_MT="" ;;
        *)   HB_MT="MT" ;;
    esac
    export HB_MT
fi

if [ -z "$HB_COMMERCE" ]; then export HB_COMMERCE=no; fi

# default lib dir name
HB_LIBDIRNAME="lib"

ETC="/etc"

HB_ARCH64=""
if [ "$HB_ARCHITECTURE" = "linux" ]
then
    HB_CPU=`uname -m`
    case "$HB_CPU" in
        *[_@]64)
            export C_USR="$C_USR -fPIC"
            HB_ARCH64="yes"
            ;;
        *)
            ;;
    esac
elif [ "$HB_ARCHITECTURE" = "hpux" ]
then
    export C_USR="$C_USR -fPIC"
fi

# Select the platform-specific installation prefix and ownership
HB_INSTALL_OWNER=root
case "$HB_ARCHITECTURE" in
    darwin)
        [ -z "$HB_INSTALL_PREFIX" ] && HB_INSTALL_PREFIX="/usr/local"
        HB_INSTALL_GROUP=wheel
        ETC="/private/etc"
        ;;
    linux)
        [ -z "$HB_INSTALL_PREFIX" ] && HB_INSTALL_PREFIX="/usr"
        [ -d "$HB_INSTALL_PREFIX/lib64" ] && [ "${HB_ARCH64}" = yes ] && HB_LIBDIRNAME="lib64"
        HB_INSTALL_GROUP=root
        ;;
    w32)
        [ -z "$HB_INSTALL_PREFIX" ] && HB_INSTALL_PREFIX="/usr/local"
        HB_INSTALL_GROUP=0
        hb_sysdir="no"
        hb_exesuf=".exe"
        hb_instfile=""
        ;;
    dos)
        [ -z "$HB_INSTALL_PREFIX" ] && HB_INSTALL_PREFIX="/${name}"
        HB_INSTALL_GROUP=root
        hb_lnkso="no"
        hb_sysdir="no"
        hb_exesuf=".exe"
        hb_instfile=""
        hb_archfile="${name}.tgz"
        HB_INST_PREF="$TMPDIR/hb-$$"
        ;;
    *)
        [ -z "$HB_INSTALL_PREFIX" ] && HB_INSTALL_PREFIX="/usr/local"
        HB_INSTALL_GROUP=wheel
        ;;
esac

# Select the platform-specific command names
MAKE=make
TAR=tar
hb_gnutar=yes
if gtar --version >/dev/null 2>&1; then
   TAR=gtar
elif ! tar --version >/dev/null 2>&1; then
   hb_gnutar=no
   echo "Warning!!! Cannot find GNU TAR"
fi
if gmake --version >/dev/null 2>&1; then
   MAKE=gmake
elif ! make --version >/dev/null 2>&1; then
   echo "Warning!!! Cannot find GNU MAKE"
fi

# Set other platform-specific build options
if [ -z "$HB_GPM_MOUSE" ]; then
    if [ "$HB_ARCHITECTURE" = "linux" ] && \
       ( [ -f /usr/include/gpm.h ] || [ -f /usr/local/include/gpm.h ]); then
        HB_GPM_MOUSE=yes
    else
        HB_GPM_MOUSE=no
    fi
    export HB_GPM_MOUSE
fi

if [ -z "${HB_WITHOUT_GTSLN}" ]; then
    HB_WITHOUT_GTSLN=yes
    case "$HB_ARCHITECTURE" in
        linux|bsd|darwin|hpux|sunos)
            for dir in /usr /usr/local /sw /opt/local
            do
                if [ -f ${dir}/include/slang.h ] || \
                   [ -f ${dir}/include/slang/slang.h ]; then
                    HB_WITHOUT_GTSLN=no
                fi
            done
            ;;
    esac
    export HB_WITHOUT_GTSLN
fi

case "$HB_ARCHITECTURE" in
    linux)
        ;;
    darwin)
        # Autodetect old Darwin versions and set appropriate build options
        if [ `uname -r | sed "s/\..*//g"` -lt 6 ]; then
            export HB_NCURSES_FINK=yes
        fi
        [ -z "$HB_WITHOUT_X11" ] && export HB_WITHOUT_X11=yes
        ;;
    dos|w32)
        [ -z "$HB_WITHOUT_X11" ] && export HB_WITHOUT_X11=yes
        ;;
    *)
        [ -z "$HB_WITHOUT_X11" ] && export HB_WITHOUT_X11=yes
        ;;
esac

if [ "$HB_COMMERCE" = yes ]
then
   export HB_GPM_MOUSE=no
   export HB_WITHOUT_GTSLN=yes
fi

if [ "${hb_sysdir}" = "yes" ]; then
    export HB_BIN_INSTALL="$HB_INSTALL_PREFIX/bin"
    export HB_INC_INSTALL="$HB_INSTALL_PREFIX/include/${name}"
    export HB_LIB_INSTALL="$HB_INSTALL_PREFIX/$HB_LIBDIRNAME/${name}"
else
    export HB_BIN_INSTALL="$HB_INSTALL_PREFIX/bin"
    export HB_INC_INSTALL="$HB_INSTALL_PREFIX/include"
    export HB_LIB_INSTALL="$HB_INSTALL_PREFIX/$HB_LIBDIRNAME"
fi

# build
umask 022
$MAKE -r clean
$MAKE -r
for l in ${hb_contrib}
do
    (cd "contrib/$l"
     $MAKE -r clean
     $MAKE -r)
done

# install
rm -fR "${HB_INST_PREF}"

export _DEFAULT_BIN_DIR=$HB_BIN_INSTALL
export _DEFAULT_INC_DIR=$HB_INC_INSTALL
export _DEFAULT_LIB_DIR=$HB_LIB_INSTALL
export HB_BIN_INSTALL="$HB_INST_PREF$HB_BIN_INSTALL"
export HB_INC_INSTALL="$HB_INST_PREF$HB_INC_INSTALL"
export HB_LIB_INSTALL="$HB_INST_PREF$HB_LIB_INSTALL"

mkdir -p $HB_BIN_INSTALL
mkdir -p $HB_INC_INSTALL
mkdir -p $HB_LIB_INSTALL
$MAKE -r -i install
for l in ${hb_contrib}
do
    (cd "contrib/$l"
     $MAKE -r -i install)
done

# Keep the size of the binaries to a minimim.
if [ -f $HB_BIN_INSTALL/harbour${hb_exesuf} ]; then
    ${CCPREFIX}strip $HB_BIN_INSTALL/harbour${hb_exesuf}
fi
if [ "$HB_ARCHITECTURE" != "hpux" ]; then
    # Keep the size of the libraries to a minimim, but don't try to strip symlinks.
    ${CCPREFIX}strip -S `find $HB_LIB_INSTALL -type f`
fi

if [ "${hb_sysdir}" = "yes" ]; then

mkdir -p $HB_INST_PREF$ETC/harbour
cp -f source/rtl/gtcrs/hb-charmap.def $HB_INST_PREF$ETC/harbour/hb-charmap.def
chmod 644 $HB_INST_PREF$ETC/harbour/hb-charmap.def

cat > $HB_INST_PREF$ETC/harbour.cfg <<EOF
CC=${CCPREFIX}gcc
CFLAGS=-c -I$_DEFAULT_INC_DIR -O3
VERBOSE=YES
DELTMP=YES
EOF

fi

# check if we should rebuild tools with shared libs
if [ "${hb_lnkso}" = yes ]
then
    case $HB_ARCHITECTURE in
        darwin)     ADD_LIBS="$ADD_LIBS -lncurses -L/opt/local/lib -L/sw/lib" ;;
        dos|w32)    ADD_LIBS="" ;;
        *)          ADD_LIBS="$ADD_LIBS -lncurses" ;;
    esac 
    [ "${HB_GPM_MOUSE}" = yes ] && ADD_LIBS="$ADD_LIBS -lgpm"
    [ "${HB_WITHOUT_GTSLN}" != yes ] && ADD_LIBS="$ADD_LIBS -lslang"
    [ "${HB_WITHOUT_X11}" != yes ] && ADD_LIBS="$ADD_LIBS -L/usr/X11R6/$HB_LIBDIRNAME -lX11"

    export L_USR="-L${HB_LIB_INSTALL} -l${name} ${ADD_LIBS} ${L_USR}"
    export PRG_USR="\"-D_DEFAULT_INC_DIR='${_DEFAULT_INC_DIR}'\" ${PRG_USR}"

    for utl in hbmake hbrun hbpp hbdoc hbtest hbdict xbscript
    do
        (cd "utils/${utl}"
         rm -fR "./${HB_ARCHITECTURE}/${HB_COMPILER}"
         $MAKE -r install
         ${CCPREFIX}strip "${HB_BIN_INSTALL}/${utl}${hb_exesuf}")
    done
fi

chmod 644 $HB_INC_INSTALL/*

(cd ${HB_BIN_INSTALL}
    if [ "$HB_ARCHITECTURE" = "w32" ]; then
        cp -f "${HB_LIB_INSTALL}/${name}.dll" .
        echo '#!/bin/sh' > xprompt.sh
        echo 'xbscript "$@"' >> xprompt.sh
    else
        ln -s xbscript${hb_exesuf} xprompt${hb_exesuf}
    fi
    ln -s xbscript${hb_exesuf} pprun${hb_exesuf}
)

CURDIR=$(pwd)
if [ $hb_gnutar = yes ]; then
    (cd "${HB_INST_PREF}"; $TAR czvf "${CURDIR}/${hb_archfile}" --owner=${HB_INSTALL_OWNER} --group=${HB_INSTALL_GROUP} .)
    UNTAR_OPT=xvpf
else
    (cd "${HB_INST_PREF}"; $TAR covf - . | gzip > "${CURDIR}/${hb_archfile}")
    UNTAR_OPT=xvf
fi
rm -fR "${HB_INST_PREF}"

if [ -n "${hb_instfile}" ]; then

   if [ "${HB_ARCHITECTURE}" = linux ]; then
      DO_LDCONFIG="&& ldconfig"
   else
      DO_LDCONFIG=""
   fi
   # In the generated script use tar instead of $TAR because we can't be sure
   # if $TAR exists in the installation environment
   cat > "${hb_instfile}" <<EOF
#!/bin/sh
if [ "\$1" = "--extract" ]; then
    sed -e '1,/^HB_INST_EOF\$/ d' "\$0" > "${hb_archfile}"
    exit
fi
if [ \`id -u\` != 0 ]; then
    echo "This package has to be installed from root account."
    exit 1
fi
echo "Do you want to install ${name} (y/n)"
read ASK
if [ "\${ASK}" != "y" ] && [ "\${ASK}" != "Y" ]; then
    exit 1
fi
(sed -e '1,/^HB_INST_EOF\$/ d' "\$0" | gzip -cd | tar ${UNTAR_OPT} - -C /) ${DO_LDCONFIG}
exit \$?
HB_INST_EOF
EOF
    cat "${hb_archfile}" >> "${hb_instfile}"
    chmod +x "${hb_instfile}"
    rm -f "${hb_archfile}"

fi

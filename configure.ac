
AC_INIT([gluster], [1.3], [gluster-devel@nongnu.org])
AM_INIT_AUTOMAKE

AC_CONFIG_FILES([Makefile 

])

#doc/Makefile 
#ramdisk/Makefile
#
#extensions/shutdown/Makefile
#extensions/GlusterHPC/Makefile
#extensions/GlusterEP/Makefile
#
#extensions/init/Makefile
#extensions/bootdisk-manager/Makefile
#extensions/bootp-manager/Makefile
#extensions/bootloader-manager/Makefile
#extensions/client-boot/Makefile
#extensions/client-reg/Makefile
#extensions/mac-collector/Makefile
#extensions/gutils/Makefile
#
#extensions/fs-utils/Makefile
#extensions/auth/Makefile
#extensions/net-utils/Makefile
#extensions/debug-utils/Makefile
#extensions/freeipmi/Makefile
#
#extensions/mpich2-pkg/Makefile
#extensions/ofed-pkg/Makefile
#extensions/torque-pkg/Makefile
#extensions/slurm-pkg/Makefile
#extensions/pdsh-pkg/Makefile
#extensions/c3-pkg/Makefile
#extensions/cerebro-pkg/Makefile
#extensions/genders-pkg/Makefile
#extensions/lam-pkg/Makefile
#extensions/ganglia-pkg/Makefile
#extensions/conman-pkg/Makefile
#extensions/hpc-base-pkg/Makefile
#extensions/autologin-pkg/Makefile
#

#AC_DEFINE_UNQUOTED(OS, "${host}", [cpu-machine-OS])
AC_PROG_CC
AC_PROG_MAKE_SET

AC_PROG_RANLIB

# check for dirname
AC_CHECK_PROG([DIRNAME], [dirname], [dirname])
# check for tar
AC_CHECK_PROG(TAR, [tar], [tar], none)
if test "$TAR" = "none"
then
AC_MSG_ERROR([no suitable tar found. Please install the 'tar' package.])
fi
# check for gzip
AC_CHECK_PROG(GZIP, [gzip], [gzip], none)  
if test "$GZIP" = "none"
then
AC_MSG_ERROR([no suitable gzip found. Please install the 'gzip' package.])
fi
# check for bzip2
AC_CHECK_PROG(BZIP2, [bzip2], [bzip2], none)
if test "$BZIP2" = "none"
then
AC_MSG_ERROR([no suitable bzip2 found. Please install the 'bzip2' package.])
fi
# check for patch
AC_CHECK_PROG(PATCH, [patch], [patch], none)  
if test "$PATCH" = "none"
then
AC_MSG_ERROR([no suitable "patch" found. Please install the 'patch' package.])
fi
AC_CHECK_PROG(LEX, [lex], [lex], none)
if test "$LEX" = "none"
then
AC_MSG_ERROR([no suitable lex/flex found. Please install the 'flex' package.])
fi
#
AC_CHECK_PROG(YACC, [yacc], [yacc], none)
if test "$YACC" = "none"
then
AC_MSG_ERROR([no suitable bison/yacc found. Please install the 'bison' package.])
fi
#
AC_CHECK_PROG(LIBTOOL, [libtool], [libtool], none)
if test "$LIBTOOL" = "none"
then
AC_MSG_ERROR([no suitable libtool found. Please install the 'libtool' package.])
fi
# check for make
#AC_CHECK_PROG([MAKE], [make], [make])  
# check for wget
AC_CHECK_PROG(WGET, [wget], [wget], none)  
if test "$WGET" = "none"
then
AC_MSG_ERROR([no suitable wget found. Please install the 'wget' package.])
fi
# check for cat
# check for cpio
AC_CHECK_PROG(CPIO, [cpio], [cpio], none)  
if test "$CPIO" = "none"
then
AC_MSG_ERROR([no suitable cpio found. Please install the 'cpio' package.])
fi
# check for file
AC_CHECK_PROG(FILE, [file], [file], none)
if test "$FILE" = "none"
then
AC_MSG_ERROR([no suitable "file" found. Please install the 'file' package.])
fi

# check for mkisofs, build cdrtools.native optionally
AC_CHECK_PROG([CDRTOOLS_NATIVE], [mkisofs], [] , [cdrtools.native])
AC_SUBST(CDRTOOLS_NATIVE)


# check for genext2fs
AC_CHECK_PROG([GENEXT2FS_NATIVE], [genext2fs], [], [genext2fs.native])
AC_SUBST(GENEXT2FS_NATIVE)

# fix tarballs dir

tarballsdir="\$(abs_top_srcdir)/tarballs"
AC_MSG_CHECKING([Source tarballs directory of external packages])
AC_ARG_WITH(tarballs-dir,
	AC_HELP_STRING([--with-tarballs-dir=DIR],[Directory to look for tarballs of external packages]),
	[ tarballsdir="$withval" ])

TARBALLS_DIR=$tarballsdir
AC_SUBST(TARBALLS_DIR)
AC_MSG_RESULT([$tarballsdir])


tarballsurl="ftp://ftp.zresearch.com/pub/gluster/external"
AC_MSG_CHECKING([tarballs repository of external packages])
AC_ARG_WITH(tarballs-url,
	AC_HELP_STRING([--with-tarballs-url=URL],[URL to download tarballs of external packages]),
	[ tarballsurl="$withval" ])

TARBALLS_URL=$tarballsurl
AC_SUBST(TARBALLS_URL)
AC_MSG_RESULT([$tarballsurl])


archs_arg=$(uname -m)

AC_ARG_WITH(archs,
	AC_HELP_STRING([--with-archs=ARCHLIST],[Comma seperated list of archs]),
	[ archs_arg="$withval" ])
archs_arg="$(echo $archs_arg | sed -e 's/,/ /')"
ARCHS=""
for arch in $archs_arg; do
    AC_MSG_CHECKING([gluster toolchain for $arch])
    got_it=$(which $arch-gluster-linux-gnu-gcc 2>/dev/null)
    got_it=${got_it:-"no"}
    AC_MSG_RESULT([$got_it])

    if test $got_it != "no" ; then
       ARCHS="$ARCHS $arch"
    fi
done

echo "Detected archs =${ARCHS}"

if test -z "$ARCHS" ; then
  AC_ERROR("No requested architectures have a gluster toolchain \$PATH !!!")
fi

AC_SUBST(ARCHS)
# fix toolchain
# fix toolbase (for lib/)

AC_OUTPUT

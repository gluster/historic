
AC_INIT([gluster], [1.0-rc1], [gluster-devel@nongnu.org])
AM_INIT_AUTOMAKE

AC_CONFIG_FILES([Makefile doc/Makefile ramdisk/Makefile
extensions/init/Makefile
extensions/bootdisk-manager/Makefile
extensions/bootp-manager/Makefile
extensions/client-boot/Makefile
extensions/client-reg/Makefile
extensions/mac-collector/Makefile
extensions/fs-utils/Makefile
extensions/ib-utils/Makefile
extensions/gutils/Makefile
extensions/auth/Makefile
extensions/net-utils/Makefile
extensions/debug-utils/Makefile
extensions/mpich2-pkg/Makefile
extensions/mvapich-gen2-pkg/Makefile
extensions/openib-pkg/Makefile
extensions/torque-pkg/Makefile
extensions/slurm-pkg/Makefile
extensions/pdsh-pkg/Makefile
extensions/c3-pkg/Makefile
extensions/cerebro-pkg/Makefile
extensions/genders-pkg/Makefile
extensions/lam-pkg/Makefile
extensions/ganglia-pkg/Makefile
extensions/conman-pkg/Makefile
extensions/hpc-base-pkg/Makefile
extensions/autologin-pkg/Makefile
extensions/shutdown/Makefile
extensions/GlusterHPC/Makefile
extensions/GlusterEP/Makefile
extensions/freeipmi/Makefile
])

AC_DEFINE_UNQUOTED(OS, "${host}", [cpu-machine-OS])
AC_PROG_CC
AC_PROG_LD
AC_PROG_MAKE_SET

AC_PROG_RANLIB

# check for dirname
AC_CHECK_PROG([DIRNAME], [dirname], [dirname])
# check for tar
AC_CHECK_PROG([TAR], [tar], [tar])  
# check for gzip
AC_CHECK_PROG([GZIP], [gzip], [gzip])  
# check for bzip2
AC_CHECK_PROG([BZIP2], [bzip2], [bzip2])
# check for patch
AC_CHECK_PROG([PATCH], [patch], [patch])  
# check for make
#AC_CHECK_PROG([MAKE], [make], [make])  
# check for wget
AC_CHECK_PROG([WGET], [wget], [wget])  
# check for cat
# check for cpio
AC_CHECK_PROG([CPIO], [cpio], [cpio])  
# check for file
AC_CHECK_PROG([FILE], [file], [file])

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

# fix toolchain
# fix toolbase (for lib/)

AC_OUTPUT

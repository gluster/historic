
grep.%: PACKAGE_NAME=grep-2.5
grep.%: LIVE_INSTALL_CMD=rm -f $1/usr/bin/{f,e}grep; make install \
		libdir=$1/usr/lib bindir=$1/usr/bin localedir=$1/usr/share \
		mandir=$1/usr/man datadir=$1/usr/share infodir=$1/usr/info \
		DESTDIR=

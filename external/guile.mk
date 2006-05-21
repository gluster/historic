guile.%: PACKAGE_NAME=guile-1.8.0
guile.%: LIVE_PREPARE_CMD=[ -f .prep ] || ( aclocal && autoconf && automake && touch .prep )
guile.%: LIVE_CONFIGURE_ENV=CC_FOR_BUILD=gcc $(DEFAULT_LIVE_CONFIGURE_ENV) LIBS=-lm
guile.%: LIVE_CONFIGURE_CMD=$(DEFAULT_LIVE_CONFIGURE_CMD) --with-threads=pthreads --disable-error-on-warning
guile.%: NATIVE_CONFIGURE_ENV=LIBS=-lm
guile.%: NATIVE_CONFIGURE_CMD=./configure --prefix=$(DESTDIR_NATIVE)/usr
guile.%: NATIVE_INSTALL_CMD=make install DESTDIR=

ncurses.%: PACKAGE_NAME=ncurses-5.4
ncurses.%: LIVE_CONFIGURE_CMD=$(DEFAULT_LIVE_CONFIGURE_CMD) --with-shared --without-ada --with-build-cc=gcc; sed -i '/cd man/d' Makefile
ncurses.%: NATIVE_CONFIGURE_CMD=$(DEFAULT_NATIVE_CONFIGURE_CMD) --with-shared --without-ada; sed -i '/cd man/d' Makefile
ncurses.%: PKG_CONFIGURE_CMD=$(DEFAULT_PKG_CONFIGURE_CMD) --with-shared -\-without-ada; sed -i '/cd man/d' Makefile
ncurses.%: PKG_INSTALL_CMD=$(DEFAULT_PKG_INSTALL_CMD); cd $1/opt/gluster/include && ln -sf ncurses/ncurses.h && ln -sf ncurses/curses.h
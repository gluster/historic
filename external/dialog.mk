
dialog.%: PACKAGE_NAME=dialog-1.0-20050306
dialog.%: LIVE_CONFIGURE_CMD=$(DEFAULT_LIVE_CONFIGURE_CMD) --with-curses-dir=$(DESTDIR_LIVE)/usr --with-ncurses

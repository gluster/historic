
screen.%: PACKAGE_NAME=screen-4.0.2
screen.%: LIVE_PREPARE_CMD=[ -f .configured ] || rm -rf Makefile config.cache && touch .configured
screen.%: LIVE_CONFIGURE_CMD=./configure --prefix=/usr --host=$(CROSS) --build=i686-pc-linux-gnu
screen.%: LIVE_INSTALL_CMD=$(DEFAULT_LIVE_INSTALL_CMD); ls ./etc/screenrc | cpio -puvd $1


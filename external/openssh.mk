
openssh.%: PACKAGE_NAME=openssh-4.2p1
openssh.%: LIVE_CONFIGURE_CMD=$(DEFAULT_LIVE_CONFIGURE_CMD) --disable-etc-default-login --sysconfdir=/etc/ssh
openssh.%: LIVE_BUILD_CMD=$(DEFAULT_LIVE_BUILD_CMD) LD=$(CROSS)-gcc


wget.%: PACKAGE_NAME=wget-1.9
wget.%: LIVE_CONFIGURE_CMD=$(PACKAGE_LIVE_CONFIGURE_CMD) --with-ssl --sysconfdir=/etc

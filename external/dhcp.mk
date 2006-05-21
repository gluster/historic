
dhcp.%: PACKAGE_NAME=dhcp-3.0.3
dhcp.%: LIVE_PREPARE_CMD=./configure
dhcp.%: LIVE_BUILD_CMD=cd work.linux-2.2 ; make all $(LIVE_BUILD_ENV)
dhcp.%: LIVE_INSTALL_CMD=cd work.linux-2.2 ; $(DEFAULT_LIVE_INSTALL_CMD)

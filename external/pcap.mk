
pcap.%: PACKAGE_NAME=libpcap-0.9.4
pcap.%: LIVE_CONFIGURE_CMD=ac_cv_linux_vers=2 $(DEFAULT_LIVE_CONFIGURE_CMD) --with-pcap=linux


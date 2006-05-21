nfs-utils.%: PACKAGE_NAME=nfs-utils-1.0.8-rc4
nfs-utils.%: LIVE_PREPARE_CMD=[ -f .prep ] || (sh autogen.sh && touch .prep)
nfs-utils.%: LIVE_CONFIGURE_CMD=$(DEFAULT_LIVE_CONFIGURE_CMD) --disable-nfsv4 --disable-gss

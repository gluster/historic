module-init-tools.%: PACKAGE_NAME=module-init-tools-3.3-pre1
module-init-tools.%: LIVE_CONFIGURE_CMD=./configure --host=$(CROSS) --prefix=/ --build=$(GLUSTER_BUILD) --enable-zlib

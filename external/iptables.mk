iptables.%: PACKAGE_NAME=iptables-1.3.5
iptables.%: LIVE_CONFIGURE_CMD=
iptables.%: LIVE_BUILD_CMD=make all $(LIVE_BUILD_ENV) KERNEL_DIR=${abs_top_builddir}/live_kernel_src PREFIX=/usr
iptables.%: LIVE_INSTALL_CMD=make install PREFIX=/usr DESTDIR=$1
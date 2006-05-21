discover.%: PACKAGE_NAME=discover-2.0.7
discover.%: LIVE_CONFIGURE_CMD=$(DEFAULT_LIVE_CONFIGURE_CMD) \
                                      --sbindir=/sbin \
                                      --sysconfdir=/etc \
                                      --localstatedir=/var \
                                      --with-default-url=file:///lib/discover/list.xml \
                                      --disable-curl
discover.%: LIVE_INSTALL_CMD=make install DESTDIR=$1 INSTALL_LIB='${LIBTOOL} --mode=install ${INSTALL} -m 644'
discover.%: NATIVE_CONFIGURE_CMD=./configure --prefix=/usr \
                                      --sbindir=/sbin \
                                      --sysconfdir=/etc \
                                      --localstatedir=/var \
                                      --with-default-url=file:///lib/discover/list.xml \
                                      --disable-curl

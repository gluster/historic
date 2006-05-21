torque.%: PACKAGE_NAME=torque-2.0.0p8
torque.%: PKG_CONFIGURE_CMD=$(PKG_BUILD_ENV) ./configure --prefix=/opt/gluster --set-default-server=master-node --with-scp
torque.%: PKG_INSTALL_CMD=make install prefix=$1/opt/gluster; ls torque.setup | cpio -puvd $1/opt/gluster/setup
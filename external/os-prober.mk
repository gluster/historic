os-prober.%: PACKAGE_NAME=os-prober-1.07
os-prober.%: LIVE_CONFIGURE_CMD=
os-prober.%: LIVE_BUILD_CMD=
os-prober.%: LIVE_INSTALL_CMD=find os-probes linux-boot-probes common.sh | cpio -puvd $1/usr/share/os-prober ; find os-prober linux-boot-prober | cpio -puvd $1/usr/bin

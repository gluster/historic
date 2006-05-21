
pcre.%: PACKAGE_NAME=pcre-6.3
pcre.%: LIVE_CONFIGURE_CMD=./configure --prefix=/usr \
+               --disable-cpp --host=$(CROSS) --build=$(GLUSTER_BUILD)

## NOTE : pcre-config is used by apps which depend on pcre for determining
##  the library and include paths of PCRE. The system's /usr/bin/pcre-config
## will confuse the cross build of grep. Below is a workaround by putting
## a dummy pcre-config into $(DESTDIR_NATIVE)/usr/bin which overrides all 
## other PATHS
## pcre.%: LIVE_INSTALL_CMD=$(DEFAULT_LIVE_INSTALL_CMD); mkdir -p $(DESTDIR_NATIVE)/usr/bin; echo > $(DESTDIR_NATIVE)/usr/bin/pcre-config; chmod +x $(DESTDIR_NATIVE)/usr/bin/pcre-config

pcre.%: LIVE_INSTALL_CMD=make install DESTDIR=$1; \
		ls pcre-config | cpio -puvd $(DESTDIR_NATIVE)/usr/bin ; \
mkdir -p $(DESTDIR_NATIVE)/usr/bin; echo > $(DESTDIR_NATIVE)/usr/bin/pcre-config; chmod +x $(DESTDIR_NATIVE)/usr/bin/pcre-config


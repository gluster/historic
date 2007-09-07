
netdevice.py.%: PACKAGE_NAME="netdevice.py-0.0.1"
netdevice.py.%: LIVE_CONFIGURE_CMD=
netdevice.py.%: LIVE_BUILD_CMD=$(CROSS)-gcc -pthread -fno-strict-aliasing -DNDEBUG -g -O2 -Wall -Wstrict-prototypes -fPIC -I$(DESTDIR_LIVE)/usr/include -I$(DESTDIR_LIVE)/usr/include/python2.4 -c netdevicemodule.c -o netdevicemodule.o && $(CROSS)-gcc -pthread -shared -L$(DESTDIR_LIVE)/usr/lib netdevicemodule.o -o netdevice.so
netdevice.py.%: LIVE_INSTALL_CMD=cp -f netdevice.so $1/usr/lib/python2.4/site-packages/



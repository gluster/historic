
python.%: PACKAGE_NAME=Python-2.4.1
#python.native: PACKAGE_CONFIGURE_CMD=$(DEFAULT_PACKAGE_CONFIGURE_CMD)
python.%: NATIVE_BUILD_CMD=make python Parser/pgen
python.%: NATIVE_INSTALL_CMD=mkdir -p $(DESTDIR_NATIVE)/usr/bin; cp python Parser/pgen $(DESTDIR_NATIVE)/usr/bin

python.%: LIVE_CONFIGURE_CMD=CXX=$(CROSS)-c++ BASECFLAGS=-I$(DESTDIR_LIVE)/usr/include BLDSHARED="$(CROSS)-gcc -shared" $(DEFAULT_LIVE_CONFIGURE_CMD); cp $(DESTDIR_NATIVE)/usr/bin/python ./hostpython; cp $(DESTDIR_NATIVE)/usr/bin/pgen ./hostpgen
python.%: LIVE_BUILD_CMD=$(DEFAULT_LIVE_BUILD_CMD) CROSS_COMPILE=yes HOSTPYTHON=./hostpython HOSTPGEN=./hostpgen BLDSHARED="$(CROSS)-gcc -shared" LDFLAGS=-L$(DESTDIR_LIVE)/usr/lib
python.%: LIVE_INSTALL_CMD=$(DEFAULT_LIVE_INSTALL_CMD) CROSS_COMPILE=yes HOSTPYTHON=./hostpython HOSTPGEN=./hostpgen; find $1/usr/lib/python2.4 -name '*.py' -exec rm -vf {} \; ; find $1/usr/lib/python2.4 -name '*.pyo' -exec rm -vf {} \;



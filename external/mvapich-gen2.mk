mvapich-gen2.%: PACKAGE_NAME=mvapich-gen2-1.0-106
ifeq ($(ARCH),x86_64)
mvapich-gen2.%: PKG_CONFIGURE_ENV=$(DEFAULT_PKG_CONFIGURE_ENV) CFLAGS="-D_SMP_ -DLAZY_MEM_UNREGISTER -DRDMA_FAST_PATH -D_X86_64_ -O3 -fno-strict-aliasing -I$(DESTDIR_PKG)/opt/gluster/include" CCFLAGS="$CFLAGS -Wno-deprecated" FFLAGS="-L$(DESTDIR_PKG)/opt/gluster/lib" FC=gfortran ac_cv_prog_AR=$(CROSS)-ar
else
mvapich-gen2.%: PKG_CONFIGURE_ENV=$(DEFAULT_PKG_CONFIGURE_ENV) CFLAGS="-D_SMP_ -DLAZY_MEM_UNREGISTER -DRDMA_FAST_PATH -D_IA32_ -O3 -fno-strict-aliasing -I$(DESTDIR_PKG)/opt/gluster/include" CCFLAGS="$CFLAGS -Wno-deprecated" FFLAGS="-L$(DESTDIR_PKG)/opt/gluster/lib" FC=gfortran ac_cv_prog_AR=$(CROSS)-ar
endif
mvapich-gen2.%: PKG_CONFIGURE_CMD=./configure --with-device=ch_p4 --with-arch=LINUX -prefix=/opt/gluster/$(PACKAGE_NAME) -lib="-L$(DESTDIR_PKG)/opt/gluster/lib -Wl,-rpath=/opt/gluster/lib -libverbs -lpthread -lsysfs" --without-mpe --enable-f77 --c++=$(CROSS)-g++ --disable-devdebug -cc=$(CROSS)-gcc -fc=gfortran -clinker=$(CROSS)-gcc -c++linker=$(CROSS)-g++ -with-cross
mvapich-gen2.%: PKG_BUILD_CMD=make RANLIB=$(CROSS)-ranlib

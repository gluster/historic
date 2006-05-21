IB_TOOLS_SUBDIRS=userspace/tvflash userspace/mstflint userspace/perftest

ib-tools.%: PACKAGE_NAME=openib-userspace-svn3640-1
ib-tools.%: LIVE_PREPARE_CMD=cd userspace/tvflash && \
		[ -f configure ] || ./autogen.sh
ib-tools.%: LIVE_CONFIGURE_CMD=;cd userspace/tvflash && \
		[ -f Makefile ] || \
		./configure --prefix=/usr \
		--host=$(CROSS) --build=$(GLUSTER_BUILD)
ib-tools.%: LIVE_BUILD_CMD= $(foreach d, $(IB_TOOLS_SUBDIRS), \
		cd $d && make all $(LIVE_BUILD_ENV) LD="$(CC)" ARCH= \
		GLUSTER_EXTRA_CFLAGS=-I$(DESTDIR_LIVE)/usr/include \
		GLUSTER_EXTRA_LDFLAGS="-L$(DESTDIR_LIVE)/usr/lib -lsysfs" && \
		cd - ;) 

ib-tools.%: LIVE_INSTALL_CMD=(cd userspace/tvflash && \
		make install DESTDIR=$1 $(LIVE_BUILD_ENV) \
		LD="$(CC)" ARCH= ) && \
		(cd userspace/mstflint && \
		cp mread mwrite mstflint $1/usr/sbin) && \
		(cd userspace/perftest && \
		cp runme rdma_lat rdma_bw $1/usr/sbin)


### hpc bundling

ib-tools.%: PKG_PREPARE_CMD=cd userspace/tvflash && \
		[ -f configure ] || ./autogen.sh
ib-tools.%: PKG_CONFIGURE_CMD=;cd userspace/tvflash && \
		[ -f Makefile ] || \
		./configure --prefix=/opt/gluster \
		--host=$(CROSS) --build=$(GLUSTER_BUILD)
ib-tools.%: PKG_BUILD_CMD= $(foreach d, $(IB_TOOLS_SUBDIRS), \
		cd $d && make all $(PKG_BUILD_ENV) LD="$(CC)" ARCH= \
		GLUSTER_EXTRA_CFLAGS=-I$(DESTDIR_PKG)/opt/gluster/include \
		GLUSTER_EXTRA_LDFLAGS="-L$(DESTDIR_PKG)/opt/gluster/lib -lsysfs" && \
		cd - ;) 

ib-tools.%: PKG_INSTALL_CMD=(cd userspace/tvflash && \
		make install DESTDIR=$1 $(PKG_BUILD_ENV) \
		LD="$(CC)" ARCH= ) && \
		(cd userspace/mstflint && \
		cp mread mwrite mstflint $1/opt/gluster/sbin) && \
		(cd userspace/perftest && \
		cp runme rdma_lat rdma_bw $1/opt/gluster/sbin)

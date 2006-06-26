
all:

install:
	mkdir -p $(DESTDIR)/tftpboot
	cp $(top_builddir)/../../iso_fs_$(ARCH)/boot/linux.slave.$(ARCH) \
		$(DESTDIR)/tftpboot
	cp $(top_builddir)/../../iso_fs_$(ARCH)/boot/initrd.slave.$(ARCH).gz \
		$(DESTDIR)/tftpboot
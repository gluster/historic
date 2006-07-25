
all:

install:
	mkdir -p $(DESTDIR)/tftpboot
	cp $(top_builddir)/../../iso_fs_$(ARCH)/boot/linux.master.$(ARCH) \
		$(DESTDIR)/tftpboot
	cp $(top_builddir)/../../iso_fs_$(ARCH)/boot/initrd.master.$(ARCH).gz \
		$(DESTDIR)/tftpboot
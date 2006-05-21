
all:

install:
	mkdir -p $(DESTDIR)/tftpboot
	cp $(top_builddir)/iso_fs/boot/linux.slave \
		$(DESTDIR)/tftpboot
	cp $(top_builddir)/iso_fs/boot/initrd.slave.gz \
		$(DESTDIR)/tftpboot
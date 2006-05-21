
kernel-images: kernel-bzimage.live
	mkdir $(top_builddir)/iso_fs/boot -p
	cp $(DESTDIR_LIVE)/boot/vmlinuz $(top_builddir)/iso_fs/boot/linux.master
	cp $(DESTDIR_LIVE)/boot/vmlinuz $(top_builddir)/iso_fs/boot/linux.slave

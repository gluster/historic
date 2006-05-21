
.PHONY: boot

boot:
	cd $(abs_top_srcdir)/boot/$(ARCH) && (find . | grep -v CVS | \
		cpio -puvd $(abs_top_builddir)/iso_fs/boot )
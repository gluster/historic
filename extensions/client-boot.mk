
client-boot.gex: EXTENSION=client-boot
client-boot.gex: $(abs_top_builddir)/iso_fs/boot/linux.slave $(abs_top_builddir)/iso_fs/boot/initrd.slave.gz

$(abs_top_builddir)/iso_fs/boot/linux.slave: kernel-bzimage.live
$(abs_top_builddir)/iso_fs/boot/initrd.slave.gz: initrd.slave


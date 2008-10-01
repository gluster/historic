CLEANFILES = 

install-data-am:
	[ "${GF_HOST_OS}" == "GF_DARWIN_HOST_OS" ] && cp mount_glusterfs /sbin/ || cp mount.glusterfs /sbin/
	[ "${GF_HOST_OS}" == "GF_DARWIN_HOST_OS" ] && chmod +x /sbin/mount_glusterfs || chmod +x /sbin/mount.glusterfs


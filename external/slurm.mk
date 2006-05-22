slurm.%: PACKAGE_NAME=slurm-1.0.11
slurm.%: PKG_CONFIGURE_CMD=ac_cv_openssldir=$(DESTDIR_PKG)/opt/gluster $(DEFAULT_PKG_CONFIGURE_CMD); sed -i 's/doc//' Makefile
slurm.%: PKG_BUILD_CMD=make all CPPFLAGS="-DBLUEGENE_CONFIG_FILE=\\\"/opt/gluster/etc/bluegene.conf\\\" -DSLURM_PLUGIN_PATH=\\\"/opt/gluster/lib\\\" -DSLURM_CONFIG_FILE=\\\"/opt/gluster/etc/slurm.conf\\\" -I$(DESTDIR_PKG)/opt/gluster/include"

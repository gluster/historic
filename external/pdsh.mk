pdsh.%: PACKAGE_NAME="pdsh-2.10"
pdsh.%: PKG_CONFIGURE_CMD=$(DEFAULT_PKG_CONFIGURE_CMD) --with-ssh --with-slurm --with-readline
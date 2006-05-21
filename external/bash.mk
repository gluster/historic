
bash.%: PACKAGE_NAME=bash-3.0
bash.%: LIVE_INSTALL_CMD=$(DEFAULT_LIVE_INSTALL_CMD); cd $1/usr/bin && ls bash | cpio -puvd $1/bin && cd -

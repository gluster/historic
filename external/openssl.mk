
openssl.%: PACKAGE_NAME=openssl-0.9.8

# passing linux-elf instead of linux-generic64 to Configure for 64bit compile 
# causes ssh-keygen to hang in runtime

ifeq ($(ARCH),x86_64)
openssl.%: LIVE_PREPARE_CMD=./Configure --prefix=/usr --openssldir=/etc/ssl linux-generic64 shared && touch .configured
openssl.%: PKG_PREPARE_CMD=./Configure --prefix=/opt/gluster linux-generic64 no-asm && touch .configured
else
openssl.%: LIVE_PREPARE_CMD=./Configure --prefix=/usr --openssldir=/etc/ssl linux-elf no-asm shared && touch .configured
openssl.%: PKG_PREPARE_CMD=./Configure --prefix=/opt/gluster linux-elf no-asm && touch .configured
endif

openssl.%: LIVE_BUILD_CMD=make all $(LIVE_BUILD_ENV) AR="$(CROSS)-ar -r" CFLAG="-O -fPIC"
openssl.%: LIVE_INSTALL_CMD=make install_sw INSTALL_PREFIX=$1
openssl.%: PKG_BUILD_CMD=make all $(PKG_BUILD_ENV) AR="$(CROSS)-ar -r" CFLAG="-O -fPIC"
openssl.%: PKG_INSTALL_CMD=make install_sw INSTALL_PREFIX=$1

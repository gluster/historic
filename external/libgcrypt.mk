libgcrypt.%: PACKAGE_NAME=libgcrypt-1.2.2
libgcrypt.%: LIVE_CONFIGURE_ENV=CC_FOR_BUILD=gcc LDFLAGS="-L$(DESTDIR_LIVE)/usr/lib -lgpg-error" CPPFLAGS=-I$(DESTDIR_LIVE)/usr/include
libgcrypt.%: LIVE_CONFIGURE_CMD=$(DEFAULT_LIVE_CONFIGURE_CMD) --disable-asm

freeipmi.%: PACKAGE_NAME=freeipmi-0.2.0
freeipmi.%: LIVE_PREPARE_CMD=[ -f .prep ] || (./autogen.sh && touch .prep)
freeipmi.%: LIVE_CONFIGURE_ENV=CC_FOR_BUILD=gcc $(LIVE_BUILD_ENV) LDFLAGS="-L$(DESTDIR_LIVE)/usr/lib -lgmp -pthread -lltdl -lgpg-error" CPPFLAGS=-I$(DESTDIR_LIVE)/usr/include

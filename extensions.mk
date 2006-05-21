
APPLICATIONS = GlusterHPC GlusterEP

BUNDLES = mpich2-pkg openib-pkg mvapich-gen2-pkg torque-pkg

EXTENSIONS= \
	init \
	bootdisk-manager \
	bootp-manager \
	client-boot \
	client-reg \
	mac-collector \
	fs-utils \
	gutils \
	auth \
	net-utils \
        debug-utils \
	shutdown \
	$(BUNDLES) \
	$(APPLICATIONS)
#	freeipmi \
#	ib-utils

%.stage3: %.live
	cd $(BUILD_LIVE)/$(PACKAGE_NAME) ; \
	export $(LIVE_INSTALL_ENV) ; \
	$(call LIVE_INSTALL_CMD,$(DESTDIR_STAGE3)/$(EXTENSION)/destdir)


include $(EXTENSIONS:%=$(top_srcdir)/extensions/%.mk)

.PHONY: extensions

extensions: $(EXTENSIONS:%=%.gex) 
	mkdir -p $(top_builddir)/iso_fs/extensions
	cp $(DESTDIR_STAGE3)/*.{gex,tgz} $(top_builddir)/iso_fs/extensions


$(EXTENSIONS:%=%.gex):
	$(MAKE) -C $(abs_top_builddir)/extensions/$(EXTENSION) all
	$(MAKE) -C $(abs_top_builddir)/extensions/$(EXTENSION) install \
		DESTDIR=$(DESTDIR_LIVE)
	mkdir -p $(DESTDIR_STAGE3)/$(EXTENSION)/destdir
	$(MAKE) -C $(abs_top_builddir)/extensions/$(EXTENSION) install \
		DESTDIR=$(DESTDIR_STAGE3)/$(EXTENSION)/destdir
	cp $(abs_top_srcdir)/extensions/$(EXTENSION)/runme \
		$(DESTDIR_STAGE3)/$(EXTENSION)
	cp $(abs_top_srcdir)/extensions/$(EXTENSION).gex \
		$(DESTDIR_STAGE3)/$(EXTENSION).gex
	$(abs_top_srcdir)/cleanup.sh $(DESTDIR_STAGE3)/$(EXTENSION)/destdir \
		STRIP=$(CROSS)-strip
	tar -C $(DESTDIR_STAGE3)/$(EXTENSION) \
		-czf $(DESTDIR_STAGE3)/$(EXTENSION).tgz .

clean.extensions:
	rm -rf destdir_stage3

clean: clean.extensions
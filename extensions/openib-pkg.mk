openib-pkg.gex: EXTENSION=openib-pkg
openib-pkg.gex: $(APPS_OPENIB:%=%.dist) sysfs.dist pciutils.dist zlib.dist

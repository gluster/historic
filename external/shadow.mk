shadow.%: PACKAGE_NAME=shadow-4.0.15
shadow.%: LIVE_CONFIGURE_CMD=ac_cv_func_setpgrp_void=yes $(DEFAULT_LIVE_CONFIGURE_CMD) --without-selinux

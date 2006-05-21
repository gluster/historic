tar.%: PACKAGE_NAME=tar-1.15.1
tar.%: LIVE_CONFIGURE_ENV=$(DEFAULT_LIVE_CONFIGURE_ENV) ac_cv_func_chown_works=yes gl_cv_func_chown_follows_symlink=yes

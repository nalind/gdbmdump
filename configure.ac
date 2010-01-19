AC_INIT(gdbmdump,0.0)
AM_INIT_AUTOMAKE(foreign)
AC_PROG_CC
AC_CHECK_FUNCS(gdbm_open)
if test x$ac_cv_func_gdbm_open = xno ; then
	AC_CHECK_LIB(gdbm,gdbm_open)
fi
AC_CONFIG_HEADER(config.h)
AC_OUTPUT(Makefile)

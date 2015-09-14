dnl Process this file with autoconf to produce a configure script.
AC_INIT([pxtools], [0.0.21])
AC_CONFIG_SRCDIR([src/pxparse.c])
AC_CONFIG_HEADERS([config.h])
AC_USE_SYSTEM_EXTENSIONS
AM_INIT_AUTOMAKE([foreign dist-bzip2])

AM_MAINTAINER_MODE
AM_SILENT_RULES([yes])

dnl Checks for programs.
AC_PROG_CC
AC_PROG_MAKE_SET
AC_PROG_INSTALL

AM_GNU_GETTEXT([external])
AC_SUBST(INTLINC)
AC_SUBST(INTLLIBS)

AC_USE_SYSTEM_EXTENSIONS

dnl Checks for libraries.

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(fcntl.h unistd.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_BIGENDIAN
AC_STRUCT_TM

ALL_LINGUAS="de"

AC_ARG_ENABLE(y2k,
  [  --enable-y2k            enables a Y2k workaround for paradox < 7.0],
  [case "${enableval}" in
	yes) y2k=1;;
	no) y2k=0;;
	*) AC_MSG_ERROR(bad value ${enableval} for --enable-y2k) ;;
esac],
[y2k=0]
)
AC_DEFINE_UNQUOTED(Y2K_WORKAROUND, [$y2k],
    [Whether to enable the paradox < 7.0 Y2K workaround or not])

AC_CHECK_HEADERS([getopt.h])
AC_CHECK_FUNCS([getopt_long])

localedir='${datadir}/locale'
AC_SUBST(localedir)

dnl Checks for library functions.

AC_CONFIG_FILES([
	src/Makefile
	doc/Makefile
	po/Makefile
	Makefile
	pxtools.spec
	])
AC_OUTPUT

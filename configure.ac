AC_INIT([display-float], [1.0], [kennedyri@gmail.com], [display-float], [http://github.com/rkennedy/exact-float])

AM_INIT_AUTOMAKE([1.13 -Wall no-define foreign std-options subdir-objects])
AC_CONFIG_MACRO_DIR([m4])

AC_CONFIG_HEADERS([config.h])
AC_LANG([C++])

RK_LIBEXT

AC_PROG_CXX

AX_CHECK_COMPILE_FLAG([-Werror=unused-command-line-argument], [
    extra_flags=-Werror=unused-command-line-argument
])
AX_APPEND_COMPILE_FLAGS([-fexceptions -pedantic], [CXXFLAGS], [$extra_flags])

# This macro is used in AC_CHECK_HEADER, but it just adds checks for
# headers that either exist everywhere or we don't use. override it here
# to avoid checking them.
m4_pushdef([AC_INCLUDES_DEFAULT], [])

BOOST_REQUIRE
BOOST_TEST

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

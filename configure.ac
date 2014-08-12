AC_INIT([Exact floating-point display], [1.0], [kennedyri@gmail.com], [display-float], [http://github.com/rkennedy/exact-float])

AM_INIT_AUTOMAKE([1.13 -Wall no-define foreign std-options subdir-objects])
AC_CONFIG_MACRO_DIR([m4])

AC_CONFIG_HEADERS([config.h])
AC_LANG([C++])

RK_LIBEXT

AC_PROG_CXX
AC_PROG_RANLIB
AM_PROG_AR
AX_CXX_COMPILE_STDCXX_11([noext], [mandatory])

# Some compilers will ignore options they don't recognize, but we don't want
# to add -fexceptions or -pedantic when they're not necessary. Check whether
# the compiler supports complaining about unrecognized options, and if it does,
# then make sure that's active when checking for other options.
AX_CHECK_COMPILE_FLAG([-Werror=unused-command-line-argument], [
    extra_flags=-Werror=unused-command-line-argument
])
AX_APPEND_COMPILE_FLAGS([-fexceptions -pedantic], [CXXFLAGS], [$extra_flags])

# This macro is used in AC_CHECK_HEADER, but it just adds checks for
# headers that either exist everywhere or we don't use. override it here
# to avoid checking them.
m4_pushdef([AC_INCLUDES_DEFAULT], [])

RK_LIB_GMOCK([
    AC_CONFIG_LINKS([tests/gtest-all.cc:$ac_cv_file_gtest_all_cc
                     tests/gmock-all.cc:$ac_cv_file_gmock_all_cc])
])

BOOST_REQUIRE
BOOST_PROGRAM_OPTIONS

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

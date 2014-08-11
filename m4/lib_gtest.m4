# Although the Google Test makefile generates libgtest.a, it also recommends
# against installing that library to a common location. Instead, it wants
# each project to use a local copy of the gtest source and build a library
# from that. Therefore, this macro finds the location of GTEST_ROOT, makes
# a local symlink for the source code, and arranges to build a local
# libgtest library that the project will link with.
AC_DEFUN([RK_LIB_GTEST], [
    AC_REQUIRE([AC_PROG_CXX])

    AC_ARG_VAR([GTEST_ROOT],
               [Location of Google Test installation; src and include directories should be accessible from here.])

    AC_ARG_WITH([gtest],
                [AS_HELP_STRING([--with-gtest=DIR],
                                [location of Google Test installation])])

    AS_CASE([x$with_gtest],
            [x], [gtest_search_fatal=no],
            [xyes], [gtest_search_fatal=yes],
            [xno], [AS_UNSET(GTEST_ROOT)],
            [
                gtest_search_fatal=yes
                GTEST_ROOT=$with_gtest
            ])
    AS_IF([test x"$GTEST_ROOT" != x], [
        AC_ARG_VAR([GTEST_CPPFLAGS],
                   [Preprocessor flags for Google Test])
        AC_ARG_VAR([GTEST_CXXFLAGS],
                   [C++ compiler flags for Google Test])
        AC_ARG_VAR([GTEST_LDFLAGS],
                   [Linker flags for Google Test])
        AC_ARG_VAR([GTEST_LIBS],
                   [Additional libraries to include when using Google Test])
        AS_VAR_SET_IF([GTEST_CPPFLAGS], [
            AS_VAR_SET([test_GTEST_CPPFLAGS], [$GTEST_CPPFLAGS])
        ], [
            AS_VAR_SET([test_GTEST_CPPFLAGS], ["-isystem $GTEST_ROOT/include"])
        ])
        AS_VAR_SET_IF([GTEST_CXXFLAGS],
    .                 [],
                      [AS_VAR_SET([GTEST_CXXFLAGS], [-pthread])])
        AC_LANG_PUSH([C++])
        AX_SAVE_FLAGS([gtest])
        AS_VAR_SET([CPPFLAGS], [$test_GTEST_CPPFLAGS])
        AS_VAR_SET([CXXFLAGS], [$GTEST_CXXFLAGS])
        AC_CHECK_HEADER([gtest/gtest.h])
        AX_RESTORE_FLAGS([gtest])
        AC_LANG_POP([C++])
    ])
    AM_CONDITIONAL([USE_GTEST], [test x"$ac_cv_header_gtest_gtest_h" = xyes])
    AM_COND_IF([USE_GTEST], [
        AC_CHECK_FILE([$GTEST_ROOT/src/gtest-all.cc], [
            AS_VAR_SET([GTEST_CPPFLAGS], [$test_GTEST_CPPFLAGS])
            AS_VAR_SET([ac_cv_file_gtest_all_cc],
                       [$GTEST_ROOT/src/gtest-all.cc])
            $1
        ], [
            $2
            AC_MSG_FAILURE([Could not find src/gtest-all.cc in $GTEST_ROOT.])
        ])
    ], [
        $2
        AS_IF([test x"$gtest_search_fatal" = xyes], [
            AC_MSG_FAILURE([Could not include gtest/gtest.h; try setting --with-gtest.])
        ])
    ])
])

AC_DEFUN([RK_LIB_GMOCK], [
    AC_REQUIRE([AC_PROG_CXX])
    AC_REQUIRE([RK_LIB_GTEST])

    AC_ARG_VAR([GMOCK_ROOT],
               [Location of Google Mock installation; src and include directories should be accessible from here.])

    AC_ARG_WITH([gmock],
                [AS_HELP_STRING([--with-gmock=DIR],
                                [location of Google Mock installation; this overrides GMOCK_ROOT])])

    AS_CASE([x$with_gmock],
            [x], [gmock_search_fatal=no],
            [xyes], [gmock_search_fatal=yes],
            [xno], [AS_UNSET(GMOCK_ROOT)],
            [
                gmock_search_fatal=yes
                GMOCK_ROOT=$with_gmock
            ])

    # Google Mock depends on Google Test. If --with-gtest=x is provided, or
    # $GTEST_ROOT is set, then use that here. Otherwise, use the gtest.h
    # provided with gmock.

    AS_IF([test x"$GMOCK_ROOT" != x], [
        AS_IF([test x"$ac_cv_header_gtest_gtest_h" != xyes], [
            GTEST_ROOT=$GMOCK_ROOT/gtest
            AS_UNSET(ac_cv_header_gtest_gtest_h)
            RK_LIB_GTEST
        ])
        AC_ARG_VAR([GMOCK_CPPFLAGS], [Preprocessor flags for Google Mock])
        AC_ARG_VAR([GMOCK_CXXFLAGS], [C++ compiler flags for Google Mock])
        AC_ARG_VAR([GMOCK_LDFLAGS], [Linker flags for Google Mock])
        AC_ARG_VAR([GMOCK_LIBS], [Additional libraries to include when using Google Mock])
        : ${GMOCK_CPPFLAGS:=-isystem $GMOCK_ROOT/include}
        : ${GMOCK_CXXFLAGS:=-pthread}
        AC_LANG_PUSH([C++])
        AX_SAVE_FLAGS([gmock])
        CPPFLAGS="$GMOCK_CPPFLAGS $GTEST_CPPFLAGS"
        CXXFLAGS="$GMOCK_CXXFLAGS $GTEST_CXXFLAGS"
        AC_CHECK_HEADER([gmock/gmock.h])
        AX_RESTORE_FLAGS([gmock])
        AC_LANG_POP([C++])
    ])
    AM_CONDITIONAL([USE_GMOCK], [test x"$ac_cv_header_gmock_gmock_h" = xyes])
    AM_COND_IF([USE_GMOCK], [
        AC_CHECK_FILE([$GMOCK_ROOT/src/gmock-all.cc], [
            ac_cv_file_gmock_all_cc=$GMOCK_ROOT/src/gmock-all.cc
            $1
        ], [
            $2
            AC_MSG_FAILURE([Could not find src/gmock-all.cc in $GMOCK_ROOT.])
        ])
    ], [
        $2
        AS_IF([test x"$gmock_search_fatal" = xyes], [
            AC_MSG_FAILURE([Could not include gmock/gmock.h; try setting --with-gmock.])
        ])
    ])
])

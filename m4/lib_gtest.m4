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
            [x], [
                # --with-gtest is not on command line, so the library is
                # optional.
                gtest_search_fatal=no
                AS_IF([test x"$GTEST_ROOT" != x], [
                    AC_MSG_NOTICE([Detected GTEST_ROOT=$GTEST_ROOT])
                    gtest_search=yes
                ])
            ],
            [xyes], [
                gtest_search=yes
                gtest_search_fatal=yes
                AS_IF([test x"$GTEST_ROOT" = x], [
                    AC_MSG_FAILURE([GTEST_ROOT is not set. Set it or provide path with --with-gtest.])
                ])
            ],
            [xno], [gtest_search=no],
            [
                gtest_search=yes
                gtest_search_fatal=yes
                AS_IF([test x"$GTEST_ROOT" != x], [
                    AC_MSG_NOTICE([Detected GTEST_ROOT=$GTEST_ROOT, but ignoring in favor of --with-gtest])
                ])
                GTEST_ROOT=$with_gtest
            ])
    AS_IF([test x"$gtest_search" = xyes], [
        AC_ARG_VAR([GTEST_CPPFLAGS], [Preprocessor flags for Google Test])
        AC_ARG_VAR([GTEST_CXXFLAGS], [C++ compiler flags for Google Test])
        AC_ARG_VAR([GTEST_LDFLAGS], [Linker flags for Google Test])
        AC_ARG_VAR([GTEST_LIBS], [Additional libraries to include when using Google Test])
        : ${GTEST_CPPFLAGS:=-I$GTEST_ROOT/include}
        : ${GTEST_CXXFLAGS:=-pthread}
        AC_LANG_PUSH([C++])
        AX_SAVE_FLAGS([gtest])
        CPPFLAGS=$GTEST_CPPFLAGS
        CXXFLAGS=$GTEST_CXXFLAGS
        LDFLAGS=$GTEST_LDFLAGS
        LDADD=$GTEST_LIBS
        AC_CHECK_HEADER([gtest/gtest.h], [], [
            AS_IF([test x"$gtest_search_fatal" = xyes], [
                AC_MSG_FAILURE([Could not find gtest.h; maybe set GTEST_ROOT to where the include directory is.])
            ], [
                gtest_search=no
            ])
        ])
        AX_RESTORE_FLAGS([gtest])
        AC_LANG_POP([C++])
    ])
    AM_CONDITIONAL([USE_GTEST], [test x"$gtest_search" = xyes])
    AS_IF([test x"$gtest_search" = xyes], [
        AC_CHECK_FILE([$GTEST_ROOT/src/gtest-all.cc], [
            AC_CONFIG_LINKS([tests/gtest-all.cc:$GTEST_ROOT/src/gtest-all.cc])
        ], [
            AC_MSG_FAILURE([Could not find src/gtest-all.cc in $GTEST_ROOT.])
        ])
    ])
])

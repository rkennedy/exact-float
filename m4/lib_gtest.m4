AC_DEFUN([RK_LIB_GTEST], [
    AC_REQUIRE([AC_PROG_CXX])

    AC_ARG_VAR([GTEST_ROOT],
               [Location of Google Test installation; src and include directories should be accessible from here.])

    AC_ARG_WITH([gtest],
                [AS_HELP_STRING([--with-gtest=DIR],
                                [location of Google Test installation])],
                [
                    # If GTEST_ROOT is set, but --with-gtest is not, then
                    # pretend we got --with-gtest=$GTEST_ROOT.
                    AS_IF([test x"$GTEST_ROOT" != x], [
                        AC_MSG_NOTICE([Detected GTEST_ROOT=$GTEST_ROOT, but overridden by --with-gtest=$with_gtest])
                        GTEST_ROOT=$with_gtest
                    ])
                    AS_CASE([$with_gtest],
                            [yes], [gtest_search=yes],
                            [no], [gtest_search=no],
                            [GTEST_ROOT=$with_gtest; gtest_search=yes])
                    gtest_search_fatal=yes
                ], [
                    gtest_search=yes
                    gtest_search_fatal=no
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

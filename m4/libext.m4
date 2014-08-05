AC_DEFUN([RK_LIBEXT], [
    AC_REQUIRE([AC_CANONICAL_HOST])
    AC_BEFORE([$0], [BOOST_REQUIRE])
    AC_CACHE_CHECK([library extension for $host_os], [rk_cv_libext], [
        AS_CASE([$host_os],
            [cygwin* | mingw* | pw32* | cegcc*], [rk_cv_libext=lib],
            [rk_cv_libext=a])
        ])
    libext=$rk_cv_libext
])

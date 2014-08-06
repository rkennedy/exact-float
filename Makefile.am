ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS} -I m4

bin_PROGRAMS = display-float
display_float_SOURCES = src/display-float.cpp src/analyze-float.cpp
display_float_CPPFLAGS = -I$(srcdir)/include $(BOOST_CPPFLAGS)
display_float_LDFLAGS =
display_float_LDADD =

if USE_GTEST
noinst_LIBRARIES = libgtest-all.a
nodist_libgtest_all_a_SOURCES = tests/gtest-all.cc
libgtest_all_a_CPPFLAGS = $(GTEST_CPPFLAGS) -I$(GTEST_ROOT)
libgtest_all_a_CXXFLAGS = $(GTEST_CXXFLAGS)

noinst_PROGRAMS = test-analyze-float
test_analyze_float_SOURCES = tests/test-analyze-float.cpp
test_analyze_float_CPPFLAGS = -I$(srcdir)/include $(BOOST_CPPFLAGS) $(GTEST_CPPFLAGS)
test_analyze_float_CXXFLAGS = -I$(srcdir)/include $(BOOST_CPPFLAGS) $(GTEST_CXXFLAGS)
test_analyze_float_LDFLAGS = $(BOOST_UNIT_TEST_FRAMEWORK_LDFLAGS) $(GTEST_LDFLAGS)
test_analyze_float_LDADD = $(BOOST_UNIT_TEST_FRAMEWORK_LIBS) $(GTEST_LIBS) libgtest-all.a

TESTS = test-analyze-float
endif

dist_noinst_SCRIPTS = autogen.sh

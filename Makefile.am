ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS} -I m4

bin_PROGRAMS = display-float
display_float_SOURCES = src/display-float.cpp src/analyze-float.cpp
display_float_CPPFLAGS = -I$(srcdir)/include $(BOOST_CPPFLAGS)
display_float_LDFLAGS =
display_float_LDADD =

noinst_PROGRAMS = test-analyze-float
test_analyze_float_SOURCES = tests/test-analyze-float.cpp
test_analyze_float_CPPFLAGS = -I$(srcdir)/include $(BOOST_CPPFLAGS)
test_analyze_float_LDFLAGS = $(BOOST_UNIT_TEST_FRAMEWORK_LDFLAGS)
test_analyze_float_LDADD = $(BOOST_UNIT_TEST_FRAMEWORK_LIBS)

TESTS = test-analyze-float

dist_noinst_SCRIPTS = autogen.sh

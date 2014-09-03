ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS} -I m4

bin_PROGRAMS = display-float
display_float_SOURCES = src/display-float.cpp src/analyze-float.cpp
display_float_CPPFLAGS = -I$(srcdir)/include \
			 $(BOOST_CPPFLAGS) \
			 $(BOOST_FORMAT_CPPFLAGS) \
			 $(BOOST_CONVERSION_CPPFLAGS) \
			 $(BOOST_PROGRAM_OPTIONS_CPPFLAGS)
display_float_LDFLAGS = $(BOOST_FORMAT_LDFLAGS) \
			$(BOOST_CONVERSION_LDFLAGS) \
			$(BOOST_PROGRAM_OPTIONS_LDFLAGS)
display_float_LDADD = $(BOOST_FORMAT_LIBS) \
		      $(BOOST_CONVERSION_LIBS) \
		      $(BOOST_PROGRAM_OPTIONS_LIBS)

if USE_GMOCK
noinst_LIBRARIES = libgtest.a libgmock.a
nodist_libgtest_a_SOURCES = tests/gtest-all.cc
libgtest_a_CPPFLAGS = $(GTEST_CPPFLAGS) -I$(GTEST_ROOT)
libgtest_a_CXXFLAGS = $(GTEST_CXXFLAGS)
nodist_libgmock_a_SOURCES = tests/gmock-all.cc
libgmock_a_CPPFLAGS = $(GMOCK_CPPFLAGS) -I$(GMOCK_ROOT) $(GTEST_CPPFLAGS)
libgmock_a_CXXFLAGS = $(GMOCK_CXXFLAGS) $(GTEST_CXXFLAGS)

noinst_PROGRAMS = test-analyze-float
test_analyze_float_SOURCES = tests/main.cpp tests/test-analyze-float.cpp tests/bit-tests.cpp
test_analyze_float_CPPFLAGS = -I$(srcdir)/include $(BOOST_CPPFLAGS) $(GTEST_CPPFLAGS) $(GMOCK_CPPFLAGS)
test_analyze_float_CXXFLAGS = -I$(srcdir)/include $(BOOST_CPPFLAGS) $(GTEST_CXXFLAGS) $(GMOCK_CXXFLAGS)
test_analyze_float_LDFLAGS = $(GTEST_LDFLAGS) $(GMOCK_LDFLAGS)
test_analyze_float_LDADD = $(GTEST_LIBS) $(GMOCK_LIBS) libgtest.a libgmock.a

TESTS = test-analyze-float
endif

dist_noinst_SCRIPTS = autogen.sh

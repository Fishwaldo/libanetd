ACLOCAL_AMFLAGS = -I autotools
include ../autotools/am_prog_doxygen.am
lib_LTLIBRARIES = libanetd.la
libanetd_la_SOURCES = http_engine.cpp http_response.cpp LogClass.cpp
libanetd_la_CXXFLAGS = -I$(top_srcdir)/include -I$(top_builddir)/include/ $(BOOST_CPPFLAGS) $(OPENSSL_INCLUDES)
libanetd_la_LIBADD = $(BOOST_LDFLAGS) $(BOOST_ASIO_LIB) $(BOOST_PROGRAM_OPTIONS_LIB) $(BOOST_SIGNALS_LIB) $(BOOST_SYSTEM_LIB) $(BOOST_FILESYSTEM_LIB) $(BOOST_DATE_TIME_LIB) $(BOOST_SERIALIZATION_LIB) $(BOOST_THREAD_LIB) $(BOOST_REGEX_LIB) $(OPENSSL_LIBS) 
libanetd_la_LDFLAGS = $(OPENSSL_LDFLAGS) -version-info 1:0:0 -no-undefined

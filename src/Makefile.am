ACLOCAL_AMFLAGS = -I autotools
include ../autotools/am_prog_doxygen.am
lib_LTLIBRARIES = libHTTPClient.la
libHTTPClient_la_SOURCES = http_request.cpp http_response.cpp
libHTTPClient_la_CXXFLAGS = -I$(top_srcdir)/include $(BOOST_CPPFLAGS) $(OPENSSL_INCLUDES)
libHTTPClient_la_LIBADD = $(BOOST_LDFLAGS) $(BOOST_ASIO_LIB) $(BOOST_PROGRAM_OPTIONS_LIB) $(BOOST_SIGNALS_LIB) $(BOOST_SYSTEM_LIB) $(BOOST_FILESYSTEM_LIB) $(BOOST_DATE_TIME_LIB) $(BOOST_SERIALIZATION_LIB) $(BOOST_THREAD_LIB) $(BOOST_REGEX_LIB) $(OPENSSL_LIBS) 
libHTTPClient_la_LDFLAGS = $(OPENSSL_LDFLAGS) -version-info 1:0:0 -no-undefined

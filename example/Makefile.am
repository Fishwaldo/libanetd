ACLOCAL_AMFLAGS = -I autotools
include ../autotools/am_prog_doxygen.am
bin_PROGRAMS = httpclient
httpclient_SOURCES = httpclient.cpp
httpclient_CXXFLAGS = -I$(top_srcdir)/include $(BOOST_CPPFLAGS) $(OPENSSL_INCLUDES)
httpclient_LDADD =  $(top_builddir)/src/libHTTPClient.la
#httpclient_LDFLAGS = -L$(top_builddir)/src/

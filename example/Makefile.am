ACLOCAL_AMFLAGS = -I autotools
include ../autotools/am_prog_doxygen.am
bin_PROGRAMS = anetd
anetd_SOURCES = anetd.cpp
anetd_CXXFLAGS = -I$(top_srcdir)/include $(BOOST_CPPFLAGS) $(OPENSSL_INCLUDES)
anetd_LDADD =  $(top_builddir)/src/libHTTPClient.la
#anetd_LDFLAGS = -L$(top_builddir)/src/

#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
dnl AC_CONFIG_MACRO_DIR([autotools])
AC_INIT([Libanetd], [0.3], [bugs@dynam.ac])
AM_INIT_AUTOMAKE([1.10 no-define foreign])
AC_PROG_LIBTOOL
AC_CONFIG_SRCDIR([src])
AC_CONFIG_HEADERS([include/anetd/anetdConfig.h])
AC_CONFIG_MACRO_DIR([autotools])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
DX_DOXYGEN_FEATURE(ON)
DX_DOT_FEATURE(ON)
DX_HTML_FEATURE(ON)
DX_CHM_FEATURE(OFF)
DX_CHI_FEATURE(OFF)
DX_MAN_FEATURE(OFF)
DX_RTF_FEATURE(OFF)
DX_XML_FEATURE(OFF)
DX_PDF_FEATURE(OFF)
DX_PS_FEATURE(OFF)
DX_INIT_DOXYGEN([libanetd],[Doxyfile],[doc])

# Checks for libraries.
AX_BOOST_BASE(1.37)

AX_BOOST_SYSTEM

AX_BOOST_ASIO

AX_BOOST_PROGRAM_OPTIONS

AX_BOOST_FILESYSTEM

AX_BOOST_DATE_TIME

AX_BOOST_SERIALIZATION

AX_BOOST_THREAD

AX_BOOST_REGEX

AX_CHECK_OPENSSL([], [AC_MSG_ERROR(OpenSSL Not Installed)])

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
# AC_CHECK_HEADER_STDBOOL
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([socket])

CXXFLAGS="-g -O0"


AC_CONFIG_FILES([Makefile src/Makefile example/Makefile Doxyfile include/Makefile libanetd.pc])

AC_OUTPUT

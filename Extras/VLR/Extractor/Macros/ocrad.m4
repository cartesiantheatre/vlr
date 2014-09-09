dnl ocrad.m4
dnl
dnl    M4 macros for OCRAD related tasks.
dnl    Copyright (C) 2010-2014 Cartesian Theatre <info@cartesiantheatre.com>.
dnl    
dnl    Public discussion on IRC available at #avaneya (irc.freenode.net) 
dnl    or on the mailing list <avaneya@lists.avaneya.com>.
dnl
dnl    This program is free software: you can redistribute it and/or modify
dnl    it under the terms of the GNU General Public License as published by
dnl    the Free Software Foundation, either version 3 of the License, or
dnl    (at your option) any later version.
dnl
dnl    This program is distributed in the hope that it will be useful,
dnl    but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl    GNU General Public License for more details.
dnl
dnl    You should have received a copy of the GNU General Public License
dnl    along with this program.  If not, see <http://www.gnu.org/licenses/>.


# Verify that the OCRAD library is at least the given version. Although it is
#  usually better to test for features rather than specific versions, the 
#  problem is that OCRAD's internal algorithms have been improved with time even
#  though its API has sometimes remained the same which can make testing for 
#  features difficult...
#
#  Usage: AC_CHECK_OCRAD(MIN_VERSION)
#
AC_DEFUN([AC_CHECK_OCRAD],
[
    # Sanity check arguments...
    m4_if([$1], [], [m4_fatal([$0 expected missing minimum version argument])]) dnl

    # This test requires the C++ compiler and preprocessor...
    AC_REQUIRE([AC_PROG_CXX])
    AC_REQUIRE_CPP

    # Perform tests using the C++ compiler. Even though OCRAD's native bindings 
    #  are available as C interfaces, their implementations require a standard
    #  C++ runtime which the C++ compiler should automatically link against...
    AC_LANG_PUSH([C++])

    # Make sure header is present and usable...
    AC_CHECK_HEADERS([ocradlib.h], [], 
        [AC_MSG_ERROR([GNU OCRAD header is required, but unavailable...])])

    # Make sure library is present and usable...
    AC_CHECK_LIB([ocrad], [OCRAD_version], [], 
        [AC_MSG_ERROR([GNU OCRAD library is required, but unavailable...])])

    # Now check the library version...
    AC_MSG_CHECKING([GNU OCRAD >= $1])
    AC_RUN_IFELSE(
        [
            AC_LANG_PROGRAM(
                [
                    #include <ocradlib.h>
                    #include <stdlib.h>
                ], 
                [ 
                    const char *version = OCRAD_version();
                    return (atof(version) >= $1) ? 0 : 1;
                ])
        ], 
        [
            AC_MSG_RESULT([yes])
        ],
        [
            AC_MSG_RESULT([no])
            AC_MSG_ERROR([GNU OCRAD library is too old... (need >= $1)])
        ])
    
    # Done with the C++ compiler for this test...
    AC_LANG_POP([C++])
])



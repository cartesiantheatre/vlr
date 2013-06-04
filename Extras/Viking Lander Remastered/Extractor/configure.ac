# VikingExtractor, to recover images from Viking Lander operations. 
# Copyright (C) 2010-2013 Cartesian Theatre <info@cartesiantheatre.com>.
#    
# Public discussion on IRC available at #avaneya (irc.freenode.net) or on the 
# mailing list <avaneya@lists.avaneya.com>.
#
# This program is free software: you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation, either version 3 of the License, or (at your option) any later 
# version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT 
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more 
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program.  If not, see <http://www.gnu.org/licenses/>.
#
#
# Maintainer: Process this file with autoconf to produce a configure script...

# Minimum version of Autoconf used to successfully process configure.ac...
AC_PREREQ([2.67])

# Copyright notice...
AC_COPYRIGHT([Copyright (C) 2010-2013 Cartesian Theatre <info@cartesiantheatre.com>.])

# Initialize Autoconf...
AC_INIT(
    [VikingExtractor],
    [0.9],
    [https://bugs.launchpad.net/avaneya],
    [viking-extractor],
    [https://www.avaneya.com])

# Location to find auxiliary automake scripts...
AC_CONFIG_AUX_DIR([build-aux])

# Location to find Autoconf plugins...
AC_CONFIG_MACRO_DIR([Macros])

# Preserve all arguments for products that want them into config.h...
config_flags="$0 $*"
AC_DEFINE_UNQUOTED(
    [CONFIGURATION_FLAGS], 
    ["$config_flags"], 
    [All flags that were passed to configure at configuration time.])

# Initialize Automake...
    
    # Setup flags...
    AM_INIT_AUTOMAKE([color-tests dist-lzip no-dist-gzip silent-rules \
         subdir-objects -Wall -Werror foreign])

    # Set by default to use silent build rules...
    AM_SILENT_RULES([yes])

# Prepare canonical values for the platform we are compiling on and the platform
#  to host the generated files on...
AC_CANONICAL_BUILD
AC_CANONICAL_HOST

# Sanity check to ensure generated ./configure script knows the directory on 
#  which it is being executed is actually the project directory by providing any
#  unique source file...
AC_CONFIG_SRCDIR([Source/VikingExtractor.cpp])

# Configure config.h...

    # Location to generate headers from templates containing Autoconf 
    #  substitution variables...
    AC_CONFIG_HEADERS([Source/config.h])

# Check for command-line options...

    # D-Bus interface...
    AC_ARG_ENABLE([dbus-interface],
        [AS_HELP_STRING([--enable-dbus-interface],
            [enable D-Bus interface (needed by VLR GUI) @<:@default: no@:>@])],
        [dbus_interface=${enableval}], 
        [dbus_interface=no])

    # D-Bus interface...
    AC_ARG_ENABLE([static],
        [AS_HELP_STRING([--enable-static],
            [statically compile against what can be @<:@default: no@:>@])],
        [static=${enableval}], 
        [static=no])

# Initialize i18n from GNU gettext...
AM_GNU_GETTEXT_VERSION([0.18.1])
AM_GNU_GETTEXT([external], [need-ngettext])

# Checks for programs needed to build and test VikingExtractor...

    # C++ preprocessor...
    AC_PROG_CXXCPP

    # C++ compiler...
    AC_PROG_CXX

    # Install utility to copy files and set attributes...
    AC_PROG_INSTALL
    
    # MD5 checksum tool required to perform test suite...
    AC_PATH_PROG([MD5SUM], [md5sum])
    if test "x$MD5SUM" = "x"; then
        AC_MSG_ERROR([md5sum tool is required to perform test suite...])
    fi

    # D-Bus interface feature was requested...
    if test "x${dbus_interface}" = xyes; then
        
        # We need gdbus tool which can be used during the test suite to verify 
        #  d-bus interface is sane...
        AC_PATH_PROG([GDBUS], [gdbus])
        if test "x$GDBUS" = "x"; then
            AC_MSG_ERROR([gdbus tool is required to perform test suite...])
        fi

    fi

# Select our native language of C++ to perform tests in...
AC_LANG([C++])

# Substitute autoconfig macros in various non-make related files...

    # System manual page...
    AC_CONFIG_FILES([Manual/viking-extractor.man])
    
    # Update version string...
    AC_CONFIG_FILES([Source/VikingExtractor.h])

# Checks for headers, libraries, and features...

    # Locates pkg-config tool on system and checks compatibility...
    PKG_PROG_PKG_CONFIG([0.25])

    # Standard C++ library headers...
    AC_CHECK_HEADERS([algorithm cassert cstdlib errno.h iomanip limits map ostream set sstream vector], [],
        [AC_MSG_ERROR([missing some required standard C++ headers...])])
    
    # POSIX headers...
    AC_LANG_PUSH([C])
    AC_CHECK_HEADERS([dirent.h fnmatch.h getopt.h sys/stat.h unistd.h], [],
        [AC_MSG_ERROR([missing a required POSIX header...])])
    AC_LANG_POP([C])

    # D-Bus interface feature was requested...
    if test "x${dbus_interface}" = xyes; then

        # Check for the D-Bus bindings via GDBus provided by the GIO library...
        PKG_CHECK_MODULES([dbus], [gio-2.0], [have_dbus=yes], 
            [have_dbus=no])

        # D-Bus interface was requested, but API isn't available...
        if test "x${have_dbus}" = xno; then
            AC_MSG_ERROR([D-Bus interface was requested, but gio-2.0 was not available...])
        fi

        # D-Bus interface available...
        if test "x${have_dbus}" = xyes; then

            # Alert user and update config.h of the result of the check...
            AC_DEFINE([USE_DBUS_INTERFACE], 1, [D-Bus interface available and enabled.])
            
            # Store the needed compiler and linker flags for automake since this
            #  doesn't happen automatically like with AC_CHECK_LIB...
            CXXFLAGS="$CXXFLAGS $dbus_CFLAGS"
            LIBS="$LIBS $dbus_LIBS"
        fi

        # Provide automake with a flag to know whether to conditionally compile 
        #  D-Bus related code...
        AM_CONDITIONAL(USE_DBUS_INTERFACE, test "$dbus_interface" = yes && test "$have_dbus" = yes)

    # D-Bus interface was not requested. Provide automake with a flag to know to
    #  not compile D-Bus related code...
    else
        AM_CONDITIONAL(USE_DBUS_INTERFACE, false)
    fi

    # GNU OCRAD minimum required version needs to be at least 0.21...
    AC_CHECK_OCRAD([0.21])

    # Government...
    AC_MSG_CHECKING([if government is])
    AC_MSG_RESULT([good])

    # zziplib...

        # Check for C header and library...
        if test "$static" = yes; then
            PKG_CHECK_MODULES_STATIC([libzzip], [zziplib], [have_zzip=yes], [have_zzip=no])
        else
            PKG_CHECK_MODULES([libzzip], [zziplib], [have_zzip=yes], [have_zzip=no])
        fi
        if test "x${have_zzip}" = xno; then
            AC_MSG_ERROR([zziplib runtime library is required, but was not detected...])
        fi

        # Store the needed compiler flags for automake since this doesn't happen
        #  automatically like with AC_CHECK_LIB. We will take care of linker
        #  flags later...
        CXXFLAGS="$CXXFLAGS $libzzip_CFLAGS"

    # Portable network graphics...
    
        # Check for C++ interface header...
        AC_CHECK_HEADERS([png++/png.hpp], [have_png_cxx=yes], [have_png_cxx=no])
        if test "x${have_png_cxx}" = xno; then
            AC_MSG_ERROR([libpng++ headers are required, but were not detected...])
        fi

        # Check for C header and static library...
        if test "$static" = yes; then
            PKG_CHECK_MODULES_STATIC([libpng], [libpng12], [have_png=yes], [have_png=no])
        else
            PKG_CHECK_MODULES([libpng], [libpng12], [have_png=yes], [have_png=no])
        fi
        if test "x${have_png}" = xno; then
            AC_MSG_ERROR([libpng runtime library is required, but was not detected...])
        fi

        # Store the needed compiler flags for automake since this doesn't happen
        #  automatically like with AC_CHECK_LIB. We will take care of linker
        #  flags later...
        CXXFLAGS="$CXXFLAGS $libpng_CFLAGS"

# Checks for typedefs, structures, and compiler characteristics...

    # Endianness...
    AC_C_BIGENDIAN

    # Fixed width primitives...
    AC_TYPE_INT8_T
    AC_TYPE_INT16_T
    AC_TYPE_INT32_T
    AC_TYPE_UINT8_T
    AC_TYPE_UINT16_T
    AC_TYPE_UINT32_T
    
    # Other basic types...
    AC_TYPE_SIZE_T

    # Check for M_PI since some compilers still don't define it in math.h / cmath...

        # Alert user...
        AC_MSG_CHECKING(for M_PI in cmath)
        
        # Check if cmath has it already by trying to compile this prologue and body. 
        #  If it compiles, toggle have_m_pi...
        have_m_pi=no
        AC_COMPILE_IFELSE(
            [AC_LANG_PROGRAM([#include <cmath>], [ double f = M_PI ])], 
            [have_m_pi=yes])
            
        # Did not compile...
        if test "$have_m_pi" = no; then
        
            # Tweak compiler flag and try again...
            SAVE_CXXFLAGS="$CXXFLAGS"
            CXXFLAGS="$CXXFLAGS -U__STRICT_ANSI__"
            AC_COMPILE_IFELSE(
                [AC_LANG_PROGRAM([#include <cmath>], [ double f = M_PI ])],
                [mpi="yes, with -U__STRICT_ANSI__"],
                [CXXFLAGS="$SAVE_CXXFLAGS"])

        fi

        # Alert user of test result...
        AC_MSG_RESULT([$have_m_pi])

        # Still didn't work, bail...
        if test "$have_m_pi" = no; then
            AC_MSG_ERROR([can't work out how to make <cmath> define M_PI])
        fi

# Checks for library functions...

    # Standard C and GNU C library extensions...
    AC_CHECK_FUNCS([access fnmatch getcwd getopt_long memset mkdir sqrt], [], 
        [AC_MSG_ERROR([missing some needed standard C or GNU C library functions...])])

# Set additional compilation flags...

    # Enable all warnings and treat them as errors...
    CXXFLAGS="$CXXFLAGS -Wall -Werror -Wshadow -Wextra"

# Set additional linker flags...

    # If static compilation is enabled, update linker...
    if test "$static" = yes; then
        
        # libpng and libzzip statically link against...
        LIBS="$LIBS -Wl,-Bstatic $libpng_LIBS $libzzip_LIBS -Wl,-Bdynamic"

        # Static linking against GCC's runtimes and the standard C++ library...
        LDFLAGS="$LDFLAGS -static-libgcc -static-libstdc++"

    # Otherwise insert vanilla linker flags...
    else
        LIBS="$LIBS $libpng_LIBS $libzzip_LIBS"
    fi

# Generate makefiles from templates containing Autoconf substitution variables...
AC_CONFIG_FILES([Makefile Translations/Makefile.in])
                   
# Generates config.status which generates all files from templates...
AC_OUTPUT

# Show a user summary of the configuration...
echo \
"-----------------------------------------------------

${PACKAGE_NAME} ${PACKAGE_VERSION}

Prefix............: $prefix
C++ Compiler......: $CXX $CFLAGS $CXXFLAGS $CPPFLAGS
Linker............: $LD $LDFLAGS $LIBS

Optional package features:
    D-Bus interface...........: $dbus_interface
    Native language support...: $USE_NLS
    Static binary.............: $static

Now type 'make @<:@<target>@:>@' where the 
optional <target> is:
    all            ...builds all binaries
    check          ...run all diagnostics
    dist           ...build a redistributable archive
    install        ...install everything

-----------------------------------------------------"


#
#    AresEngine, a 3D game engine.
#    Copyright (C) 2010-2013 Cartesian Theatre <kip@thevertigo.com>.
#    
#    Public discussion on IRC available at #avaneya (irc.freenode.net) 
#    or on the mailing list <avaneya@lists.avaneya.com>.
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

# Options for aclocal so it can find our custom M4 macros...
ACLOCAL_AMFLAGS = -I Macros

# Product list containing just viking-extractor to be destined for the binary 
#  prefix...
bin_PROGRAMS = viking-extractor

# viking-extractor product option variables containing list of sources...
viking_extractor_SOURCES = \
    Source/Console.cpp \
    Source/Console.h \
    Source/ExplicitSingleton.h \
    Source/LogicalRecord.cpp \
    Source/LogicalRecord.h \
    Source/Miscellaneous.cpp \
    Source/Miscellaneous.h \
    Source/Options.cpp \
    Source/Options.h \
    Source/ReconstructableImage.cpp \
    Source/ReconstructableImage.h \
    Source/VicarImageAssembler.cpp \
    Source/VicarImageAssembler.h \
    Source/VicarImageBand.cpp \
    Source/VicarImageBand.h \
    Source/VikingExtractor.cpp \
    Source/ZZipFileDescriptor.cpp \
    Source/ZZipFileDescriptor.h

# If D-Bus interface was enabled through Autoconf, then we also need to compile
#  these sources too...
if USE_DBUS_INTERFACE
viking_extractor_SOURCES += \
    Source/DBusInterface.h \
    Source/DBusInterface.cpp
endif

# These files must exist before anything is compiled. Can be machine generated...
BUILT_SOURCES = \
    Source/VikingExtractor.h

# viking-extractor product option variables for list of source not to be 
#  distributed because they are machine generated...
nodist_viking_extractor_SOURCES = \
    Source/config.h.in

# Miscellaneous data files...
dist_pkgdata_DATA = \
    Authors \
    Copying \
    News    \
    ReadMe

# System manual pages...
man1_MANS = $(top_builddir)/Manual/viking-extractor.man

# Manually add these things to distribution package when dist target is run...
EXTRA_DIST = \
	Authors \
	Copying \
	Install \
	News    \
	ReadMe  \
    Tests   \
	ToDo

# Product list of scripts to generate during check target...
check_SCRIPTS = GrepTest.sh RecoveryTest.sh

# Additional C++ preprocessor flags...
AM_CPPFLAGS=
AM_CPPFLAGS+=-I$(top_srcdir)/Source

# Targets to execute for check target...
TESTS = $(check_SCRIPTS)

# Rule to create the viking-extractor test script. Just tests to make sure it 
#  compiled and linked correctly...
GrepTest.sh: Makefile.am
	@echo './viking-extractor --version | grep -q ".*Cartesian Theatre."' > $@
	@chmod +x $@

# Checksums to for recovery_test.sh target to check against...
RecoveryChecksums.md5: Makefile.am
	@echo '5e37828fe338c8a12ef387ca2bbb2b85  Tests/Recovery/22D180.png' > $@
	@echo 'b4c641d7b0c30079e73298e05522cae6  Tests/Recovery/22D180.txt' >> $@

# Rule to test a sample viking-extractor recovery against one known to have been 
#  successful...
RecoveryTest.sh: RecoveryChecksums.md5 Makefile.am
	@echo './viking-extractor --recursive --no-ansi-colours --overwrite --generate-metadata $(top_srcdir)/Tests/Recovery/ Tests/Recovery/' > $@
	@echo '$(MD5SUM) --warn --check $<' >> $@
	@chmod +x $@

# Actually clean all derived and maintainer files...
MAINTAINERCLEANFILES =              \
	$(srcdir)/Source/config.h.in    \
	$(srcdir)/aclocal.m4            \
	$(srcdir)/config.guess          \
	$(srcdir)/config.sub            \
	$(srcdir)/configure             \
	$(srcdir)/depcomp               \
	$(srcdir)/install-sh            \
	$(srcdir)/Makefile.in           \
	$(srcdir)/missing

# Additional files left behind during dist target that need to be cleaned...
DISTCLEANFILES = \
	_configs.sed \
    $(PACKAGE_TARNAME)-$(PACKAGE_VERSION).tar.*

# List of files to clean up during clean target...
CLEANFILES =                    \
    Source/VikingExtractor.h    \
    Tests/Recovery/22D180.png   \
    Tests/Recovery/22D180.txt   \
    GrepTest.sh                 \
    RecoveryChecksums.md5       \
    RecoveryTest.sh


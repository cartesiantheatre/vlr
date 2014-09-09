#
#    VikingExtractor, to recover images from Viking Lander operations.
#    Copyright (C) 2010-2014 Cartesian Theatre <info@cartesiantheatre.com>.
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

# Additional libraries to link against...
viking_extractor_LDADD = $(LIBINTL)

# viking-extractor product option variables containing list of sources...
viking_extractor_SOURCES = \
    Source/Console.cpp \
    Source/Console.h \
    Source/ExplicitSingleton.h \
    Source/gettext.h \
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

RECOVERY_TEST_ARGUMENTS	= \
    --recursive \
    --no-ansi-colours \
    --overwrite \
    --generate-metadata 

# If D-Bus interface was enabled through Autoconf...
if USE_DBUS_INTERFACE

# Compile relavent sources too...
viking_extractor_SOURCES += \
    Source/DBusInterface.h \
    Source/DBusInterface.cpp

RECOVERY_TEST_ARGUMENTS += \
    --remote-start
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
EXTRA_DIST =        \
    Tests           \
	Authors         \
	ChangeLog       \
	Copying         \
	Install         \
	News            \
	ReadMe          \
	ToDo

# Product list of scripts to generate during check target...
check_SCRIPTS = GrepTest.sh RecoveryTest.sh

# Additional C++ preprocessor flags...
AM_CPPFLAGS=
AM_CPPFLAGS+=-I$(top_srcdir)/Source

# Locale directory...
localedir = $(datadir)/locale

# Preprocessor definitions to pass down to the compiler...
DEFS=
DEFS+= -DLOCALEDIR="\"$(localedir)\"" @DEFS@

# Options to pass on to ./configure when running distcheck...
#AM_DISTCHECK_CONFIGURE_FLAGS= \
#    --enable-dbus-interface

# Targets to execute for check target...
TESTS = $(check_SCRIPTS)

# Make sure gettext is present...
check-gettext:
	@if test x$(USE_NLS) != "xyes" ; then echo "Missing gettext. Rerun configure and check for" \
	"'checking whether to use NLS... yes'!" ; exit 1 ; fi

# Force an update to the machine dependent message catalogs...
force-update-gmo: check-gettext
	find Translations/ -type f -name "*.po" -execdir touch {} \;
	cd Translations && $(MAKE) $(AM_MAKEFLAGS) update-gmo

# Force a rebuild of the machine dependent message catalog for a specific 
#  language...
force-update-gmo-%: check-gettext
	@language=`echo $@ | sed s/force-update-gmo-//` ; \
	if test ! -f Translations/$$language.po ; then echo "file Translations/$$language.po does not exist" ; exit 1 ; fi ; \
	touch Translations/$$language.po ; \
	cd Translations && $(MAKE) $(AM_MAKEFLAGS) update-gmo

# Rule to create the viking-extractor test script. Just tests to make sure it 
#  compiled and linked correctly...
GrepTest.sh: Makefile.am
	@echo './viking-extractor --version | grep -q ".*Cartesian Theatre."' > $@
	@chmod +x $@

# Checksums for RecoveryTest.sh target to check against...
RecoveryChecksums.md5: Makefile.am
	@echo '5e37828fe338c8a12ef387ca2bbb2b85  Tests/Recovery/22D180.png' > $@
	@if [[ "$(LANGUAGE)" == "en_CA"* ]]; then \
	echo 'b4c641d7b0c30079e73298e05522cae6  Tests/Recovery/22D180.txt' >> $@ ; \
	fi ;

# If D-Bus interface was enabled through Autoconf, then include testing of d-bus 
#  functionality when verifying checksums of recovered sample mission data. For
#  headless systems needing a dbus session, see...
#       <http://lists.freedesktop.org/archives/dbus/2009-September/011746.html>
if USE_DBUS_INTERFACE
RecoveryTest.sh: RecoveryChecksums.md5 Makefile.am
	@echo '' > $@
	@echo '# Remove previous artifacts we check against in case of crash during test run...' >> $@
	@echo 'rm -f Tests/Recovery/22D180.png Tests/Recovery/22D180.txt' >> $@
	@echo '' >> $@
	@echo '# Launch the viking extractor and capture process ID...' >> $@
	@echo './viking-extractor $(RECOVERY_TEST_ARGUMENTS) $(top_srcdir)/Tests/Recovery/ Tests/Recovery/ &' >> $@
	@echo 'VE_PID=$$!' >> $@
	@echo '' >> $@
	@echo '# Keep trying to initiate the extraction remotely over d-bus until it' >> $@
	@echo '#  becomes available on the session bus...' >> $@
	@echo 'while [ ! $$(kill -s 0 $$VE_PID &> /dev/null) ]' >> $@
	@echo 'do' >> $@
	@echo '' >> $@
	@echo '    # Try to execute the Start() method...' >> $@
	@echo '    $(GDBUS) call --session --dest com.cartesiantheatre.VikingExtractorService --object-path /com/cartesiantheatre/VikingExtractorObject --method com.cartesiantheatre.VikingExtractorInterface.Start &> /dev/null' >> $@
	@echo '' >> $@
	@echo '    # Succeeded. Do not try again...' >> $@
	@echo '    if [ $$? == 0 ] ; then' >> $@
	@echo '        break' >> $@
	@echo '    fi' >> $@
	@echo '    sleep 1s' >> $@
	@echo 'done' >> $@
	@echo '' >> $@
	@echo '# Wait for the extractor to exit...' >> $@
	@echo 'wait $$VE_PID' >> $@
	@echo '' >> $@
	@echo '# Verify checksums of extracted photographs...' >> $@
	@echo '$(MD5SUM) --warn --check $<' >> $@
	@echo '' >> $@
	@chmod +x $@

# Otherwise verifying checksums of recovered sample mission data without using
#  d-bus invocation...
else
RecoveryTest.sh: RecoveryChecksums.md5 Makefile.am
	@echo 'rm -f Tests/Recovery/22D180.png Tests/Recovery/22D180.txt' > $@
	@echo './viking-extractor $(RECOVERY_TEST_ARGUMENTS) $(top_srcdir)/Tests/Recovery/ Tests/Recovery/' >> $@
	@echo '$(MD5SUM) --warn --check $<' >> $@
	@chmod +x $@

endif

# Update the machine dependent message catalogs...
update-gmo: check-gettext
	cd Translations && $(MAKE) $(AM_MAKEFLAGS) update-gmo

# Create the .pot file..
update-po: check-gettext
	@find $(srcdir) -iname "*.cpp" -print | sort > $(srcdir)/Translations/POTFILES.in.2 ; \
	@find $(srcdir) -iname "*.cpp.in" -print | sort >> $(srcdir)/Translations/POTFILES.in.2 ; \
	@find $(srcdir) -iname "*.h" -print | sort >> $(srcdir)/Translations/POTFILES.in.2 ; \
	@find $(srcdir) -iname "*.h.in" -print | sort >> $(srcdir)/Translations/POTFILES.in.2 ; \
	if diff $(srcdir)/Translations/POTFILES.in $(srcdir)/Translations/POTFILES.in.2 >/dev/null 2>&1 ; then \
		rm -f $(srcdir)/Translations/POTFILES.in.2 ; \
	else \
		mv $(srcdir)/Translations/POTFILES.in.2 $(srcdir)/Translations/POTFILES.in ; \
	fi
#	sed --in-place $(srcdir)/Translations/$(PACKAGE).pot --expression=s/CHARSET/UTF-8/ # CHARSET warning hack
	cd Translations && $(MAKE) $(AM_MAKEFLAGS) update-po

# Actually clean all derived and maintainer files...
MAINTAINERCLEANFILES =                          \
	$(srcdir)/build-aux/config.h.in             \
	$(srcdir)/build-aux/config.guess            \
	$(srcdir)/build-aux/config.rpath            \
	$(srcdir)/build-aux/config.sub              \
	$(srcdir)/build-aux/depcomp                 \
	$(srcdir)/build-aux/install-sh              \
	$(srcdir)/build-aux/missing                 \
	$(srcdir)/Translations/Makevars.template    \
	$(srcdir)/ABOUT-NLS                         \
	$(srcdir)/Source/config.h.in                \
	$(srcdir)/aclocal.m4                        \
	$(srcdir)/config.guess                      \
	$(srcdir)/config.sub                        \
	$(srcdir)/configure                         \
	$(srcdir)/depcomp                           \
	$(srcdir)/install-sh                        \
	$(srcdir)/Makefile.in                       \
	$(srcdir)/missing

# Additional files left behind during dist target that need to be cleaned...
DISTCLEANFILES = \
	_configs.sed \
    $(PACKAGE_TARNAME)-$(PACKAGE_VERSION).tar.*

# List of files to clean up during clean target...
CLEANFILES =                    \
    Translations/*.gmo          \
    Source/VikingExtractor.h    \
    Tests/Recovery/22D180.png   \
    Tests/Recovery/22D180.txt   \
    GrepTest.sh                 \
    RecoveryChecksums.md5       \
    RecoveryTest.sh

# Other directories containing Makefiles, such as translations...
SUBDIRS = Translations

# Targets which aren't actually files...
.PHONY: check-gettext update-po update-gmo force-update-gmo


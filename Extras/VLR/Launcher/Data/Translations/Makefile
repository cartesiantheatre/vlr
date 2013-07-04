#
# VikingExtractor, to recover images from Viking Lander operations.
# Copyright (C) 2010-2013 Cartesian Theatre <info@cartesiantheatre.com>.
#
# Public discussion on IRC available at #avaneya (irc.freenode.net) or
# on the mailing list <avaneya@lists.avaneya.com>.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or 
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but 
# WITHOUT ANY WARRANTY; without even the implied warranty of 
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#

# Location of source code...
SRCDIR = ../../Source

# Root data directory...
DATADIR = ../

# Directory containing all message translation catalogues...
PODIR = .

# The message catalogue template...
POTFILE = $(PODIR)/vlr.pot

# Location to install compiled binary message catalogues to. Default, if not
#  overriden via user environment, to current directory...
INSTALL_DIR ?= .

# Command to install data...
INSTALL_DATA = install --mode=644

# Message catalogue domain...
DOMAIN = "vlr"

# Options for gettext...
GETTEXT_OPTIONS = --add-comments --add-location --from-code=utf-8

# List of all translation catalogues...
POFILES =       \
    en_CA.po    \
    en_GB.po    \
    it.po       \
    uk.po       \
    ru.po

# List of all translation catalogues in compiled form...
GMOFILES=$(POFILES:.po=.gmo)

# Default target is to update all machine dependent message catalogs...
all: update-gmo

# Check the translations...
check:
	@echo Checking translation files...
	@for file in ${POFILES}; do \
		msgfmt --check -vv $$file -o /dev/null; \
	done

# Cleanup all derived files...
clean:
	@for file in ${GMOFILES}; do \
		rm -fv $$file; \
	done

# Compile a specific message catalogue into optimized binary format. According
#  to sect 10.1.9 of the gettext manual, binary message catalogues actually are
#  machine portable. Endianness is swapped at runtime with a negligible 
#  performance penalty... <http://rod.gs/t2o>
%.gmo:
	@rm -fv $@
	@msgfmt --check --statistics --verbose -o $@ $*.po

# Force an update to the machine dependent message catalogs...
force-update-gmo: update-gmo
	@find . -maxdepth 1 -type f -iname "*.po" -execdir touch {} \;
	$(MAKE) update-gmo

# Force a rebuild of the machine dependent message catalog for a specific 
#  language...
force-update-gmo-%:
	@language=`echo $@ | sed s/force-update-gmo-//` ; \
	if test ! -f $(PODIR)/$$language.po ; then \
		echo "file $(PODIR)/$$language.po does not exist" ; \
		exit 1 ; \
	fi ; \
	touch $(PODIR)/$$language.po ; \
	$(MAKE) update-gmo

# Install the compiled object files into the locale directory...
install-po: all
	@for GMOFILE in ${GMOFILES}; do \
		GMOFILE=`basename $$GMOFILE`; \
		LANG=`echo $$GMOFILE | sed -e 's/\.gmo//'`; \
		FINAL_DIR=$(INSTALL_DIR)/$$LANG/LC_MESSAGES; \
		mkdir -p $$FINAL_DIR; \
		$(INSTALL_DATA) $$GMOFILE $$FINAL_DIR/$(DOMAIN).mo; \
		echo "installing $$GMOFILE as $$FINAL_DIR/$(DOMAIN).mo"; \
	done

# Uninstall the compiled object files from the locale directory...
uninstall-po:
	@for GMOFILE in ${GMOFILES}; do \
		GMOFILE=`basename $$GMOFILE`; \
		LANG=`echo $$GMOFILE | sed -e 's/\.gmo//'`; \
		FINAL_DIR=$(INSTALL_DIR)/$$LANG/LC_MESSAGES; \
		if test -f $$FINAL_DIR/$(DOMAIN).mo ; then \
			rm -fv $$FINAL_DIR/$(DOMAIN).mo ; \
	    fi ; \
	    if test -d $(INSTALL_DIR)/$$LANG/LC_MESSAGES ; then \
			rmdir $(INSTALL_DIR)/$$LANG/LC_MESSAGES ; \
			rmdir $(INSTALL_DIR)/$$LANG/ ; \
		fi ; \
	done

# Update all machine dependent message catalogs...
update-gmo: ${GMOFILES}

# Create the .pot message catalogue template..
update-pot:
	echo '' > $(PODIR)/messages.po # xgettext looks for this file
	@find $(SRCDIR) -iname "*.py" | xgettext $(GETTEXT_OPTIONS) --language Python --join-existing -f - 
	sed --in-place messages.po --expression=s/CHARSET/UTF-8/ # CHARSET warning hack
	@find $(DATADIR) -iname "*.glade" | xgettext $(GETTEXT_OPTIONS) --language Glade --join-existing -f - 
	msgmerge --lang=en_CA --no-fuzzy-matching $(POTFILE) messages.po > new.pot
	mv new.pot $(POTFILE)
	rm messages.po

# Directive to make to let it know that these targets don't generate filesystem 
#  objects / products and therefore no need to check time stamps...
.PHONY:                 \
    all                 \
    clean               \
    force-update-gmo    \
    install-po          \
    uninstall-po        \
    update-gmo          \
    update-pot


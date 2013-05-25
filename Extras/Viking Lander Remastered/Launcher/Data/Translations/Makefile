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

SRCDIR = ../../Source
DATADIR = ../
PODIR = .
POTFILE = $(PODIR)/vlr.pot

# List of all translation catalogues...
POFILES = #ru.po

# List of all translation catalogues in compiled form...
GMOFILES=$(POFILES:.po=.gmo)

# Default target is to update all machine dependent message catalogs...
all: update-gmo

# Cleanup all derived files...
clean:
	@for file in ${GMOFILES}; do \
		rm -fv $$file; \
	done

# Compile a specific message catalogue into machine dependent binary...
%.gmo:
	@rm -fv $@
	msgfmt --check --statistics --verbose -o $@ $*.po

# Force an update to the machine dependent message catalogs...
force-update-gmo: update-gmo
	find . -type f -name "*.po" -execdir touch {} \;
	$(MAKE) update-gmo

# Force a rebuild of the machine dependent message catalog for a specific 
#  language...
force-update-gmo-%:
	@language=`echo $@ | sed s/force-update-gmo-//` ; \
	if test ! -f $(PODIR)/$$language.po ; then echo "file $(PODIR)/$$language.po does not exist" ; exit 1 ; fi ; \
	touch $(PODIR)/$$language.po ; \
	$(MAKE) update-gmo

# Update all machine dependent message catalogs...
update-gmo: ${GMOFILES}

# Create the .pot file..
update-pot:
	echo '' > $(PODIR)/messages.po # xgettext looks for this file
	find $(SRCDIR) -iname "*.py" | xgettext --from-code=utf-8 --language Python --join-existing -f - 
	sed --in-place messages.po --expression=s/CHARSET/UTF-8/ # CHARSET warning hack
	find $(DATADIR) -iname "*.glade" | xgettext --from-code=utf-8 --language Glade --join-existing -f - 
	msgmerge --lang=en_CA --no-fuzzy-matching $(POTFILE) messages.po > new.pot
	mv new.pot $(POTFILE)
	rm messages.po

# Directive to make to let it know that these targets don't generate filesystem 
#  objects / products and therefore no need to check time stamps...
.PHONY: all clean update-pot update-gmo force-update-gmo


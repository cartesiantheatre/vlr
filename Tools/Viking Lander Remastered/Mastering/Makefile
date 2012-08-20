# VikingExtractor, to recover images from Viking Lander operations.
# Copyright (C) 2010, 2011, 2012 Cartesian Theatre <kip@thevertigo.com>.
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

ISO                 = Avaneya_Viking_Lander_Remastered.iso
PREPARER_ID         = "Cartesian Theatre"
PUBLISHER_ID        = "Cartesian Theatre"
VOLUME_ID           = "Avaneya_VLR"
MANIFEST            = ./Manifest
LAYOUT_OPTIMIZATION = ./LayoutOptimization
GENISOIMAGE_PARAMETERS \
                    = -graft-points \
                      -input-charset utf-8 \
                      -iso-level 3 \
                      -J \
                      -no-bak \
                      -path-list ./Manifest \
                      -p $(PREPARER_ID) \
                      -publisher $(PUBLISHER_ID) \
                      -r \
                      -sort $(LAYOUT_OPTIMIZATION) \
                      -udf \
                      -volid $(VOLUME_ID)

# Disable checking for files by these names...
#.PHONY: all clean check

# Default target is to built the complete ISO...
all: iso

# Build the ISO...
# TODO: This iso isn't exclusively udf, but appears to be iso9660. We need to
#        be exclusively udf to ensure exec bits are always correctly set...
iso: $(ISO)
$(ISO): viking-extractor Manifest Makefile
	chmod +x -c ../Launcher/Code/autorun.sh
	genisoimage $(GENISOIMAGE_PARAMETERS) -o $(ISO)

# Recurse into viking-extractor's makefile for supported targets...
all check viking-extractor:
	cd ../Extractor && $(MAKE) $@

# Clean up...
clean:
	-@$(RM) -v $(ISO)
	cd ../Extractor && $(MAKE) $@

# Directive to make to let it know that these targets don't generate filesystem 
#  objects / products and therefore no need to check time stamps...
.PHONY: all check clean iso


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

# Image metadata...
PUBLISHER_ID        = "Cartesian Theatre <info@cartesiantheatre.com>"
PREPARER_ID         = $(PUBLISHER)
VOLUME_ID           = "AVANEYA_VLR"

# Image file name...
ISO                 = Avaneya_Viking_Lander_Remastered.iso

# Premastering contents, exclusion, and optimization list...
INCLUDE_LIST        = Lists/Manifest
EXCLUDE_LIST        = Lists/Exclude
OPTIMIZATION_LIST   = Lists/Optimization

# Extractor and launcher source roots...
EXTRACTOR_DIR       = ../Extractor/
LAUNCHER_DIR        = ../Launcher/

GENISOIMAGE_PARAMETERS  =           \
    -copyright Copying              \
    -graft-points                   \
    -J                              \
    -exclude-list $(EXCLUDE_LIST)   \
    -no-bak                         \
    -o $(ISO)                       \
    -path-list $(INCLUDE_LIST)      \
    -publisher "$(PUBLISHER_ID)"    \
    -p "$(PREPARER_ID)"             \
    -r                              \
    -sort $(OPTIMIZATION_LIST)      \
    -V "$(VOLUME_ID)"

# Default target is to built the complete ISO...
all: $(ISO)

# Build the image by coalescing everything needed from various places...
$(ISO): $(INCLUDE_LIST) $(EXCLUDE_LIST) $(OPTIMIZATION_LIST) Makefile
	chmod +x -c ../Launcher/Source/autorun.sh

# Recurse into viking-extractor's to build it...
all check viking-extractor:
	if test -f $(EXTRACTOR_DIR)/configure ; then \
		$(MAKE) maintainer-clean
	fi ; \
	cd ../Extractor && $(MAKE) $@

# Clean up...
clean:
	-@$(RM) -v $(ISO)
	cd ../Extractor && $(MAKE) $@

# Directive to make to let it know that these targets don't generate filesystem 
#  objects / products and therefore no need to check time stamps...
.PHONY: all clean iso


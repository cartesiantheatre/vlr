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

# Generated image file name...
ISO                 = Avaneya_Viking_Lander_Remastered.iso

# Image contents root prefix...
IMAGE_DIR           = $(abspath ./ImageRoot/)

# Build root prefix...
BUILD_DIR           = $(abspath ./Build/)

# Premastering contents, exclusion, and optimization list...
INCLUDE_LIST        = Lists/Manifest
EXCLUDE_LIST        = Lists/Exclude
OPTIMIZATION_LIST   = Lists/Optimization

# Extractor and launcher source roots...
EXTRACTOR_DIR       = $(abspath ../Extractor/)
LAUNCHER_DIR        = $(abspath ../Launcher/)

GENISOIMAGE_PARAMETERS  =           \
    -copyright Copying              \
    -graft-points                   \
    -J                              \
    -exclude-list $(EXCLUDE_LIST)   \
    -no-bak                         \
    -path-list $(INCLUDE_LIST)      \
    -publisher $(PUBLISHER_ID)    \
    -p $(PREPARER_ID)             \
    -r                              \
    -sort $(OPTIMIZATION_LIST)      \
    -V $(VOLUME_ID)

# Default target is to built the complete ISO...
all: $(ISO)

# Build the image by coalescing everything needed from various places...
$(ISO): $(INCLUDE_LIST) $(EXCLUDE_LIST) $(OPTIMIZATION_LIST) Makefile viking-extractor
	chmod +x -c XDG/autorun.sh
	#genisoimage -o $@ $(GENISOIMAGE_PARAMETERS) $(IMAGE_DIR)

# VikingExtractor depends on all compiled architectures...
viking-extractor: viking-extractor-gnu-amd64

# Configure VikingExtractor for a given architecture...
$(BUILD_DIR)/%/config.status: $(EXTRACTOR_DIR)/configure
#TODO: Finish this.
	mkdir -p $(@D) && \
	cd $(@D) && \
	$(EXTRACTOR_DIR)/configure \
	    --enable-dbus-interface \
	    --enable-static \
	    --prefix=$(IMAGE_DIR)/Extractor/%

# Prepare viking-extractor maintainer files...
$(EXTRACTOR_DIR)/configure:
	cd $(@D) && autoreconf -v -i

# Compile and install viking-extractor for any given architecture...
viking-extractor-%: $(BUILD_DIR)/%/config.status
	@platform=`echo $@ | sed s/viking-extractor-//` ; \
	cd $(BUILD_DIR)/$$platform/ && \
	$(MAKE) check && \
	$(MAKE) install

# Clean up...
clean:
	-@$(RM) -v $(ISO)

# Directive to make to let it know that these targets don't generate filesystem 
#  objects / products and therefore no need to check time stamps...
.PHONY: all clean


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
PUBLISHER_ID            = "Cartesian Theatre <info@cartesiantheatre.com>"
PREPARER_ID             = $(PUBLISHER_ID)
VOLUME_ID               = "AVANEYA_VLR"

# Generated image file name...
ISO                     = Avaneya_Viking_Lander_Remastered.iso

# Image staging root prefix...
STAGING_DIR             = $(abspath ./Staging/)

# Extractor location...
EXTRACTOR_DIR           = $(abspath ../Extractor)

# VLR location...
VLR_DIR                 = $(abspath ../Launcher)

# Woe32 autorun stub...
WOE32STUB_DIR           = $(abspath ./Woe32/Stub/)

# Premastering contents, exclusion, and optimization list...
INCLUDE_LIST            = Lists/AdditionalFiles
EXCLUDE_LIST            = Lists/ExcludedFiles
OPTIMIZATION_LIST       = Lists/LayoutOptimization

# Image generation parameters. Rock Ridge, Joliet, UTF-8, metadata, etc...
GENISOIMAGE_PARAMETERS  =           \
    -copyright Copying              \
    -exclude-list $(EXCLUDE_LIST)   \
    -graft-points                   \
    -input-charset UTF-8            \
    -J                              \
    -no-bak                         \
    -path-list $(INCLUDE_LIST)      \
    -p $(PREPARER_ID)               \
    -publisher $(PUBLISHER_ID)      \
    -r                              \
    -sort $(OPTIMIZATION_LIST)      \
    -V $(VOLUME_ID)

# Default target is to built the complete ISO...
all: $(ISO) MD5SUM

# Build the image by coalescing everything needed from various places...
$(ISO): \
  $(INCLUDE_LIST) \
  $(EXCLUDE_LIST) \
  $(OPTIMIZATION_LIST) \
  XDG/* \
  vlr.stamp \
  $(WOE32STUB_DIR)/Woe32Stub.exe
	@chmod +x -c XDG/autorun
	genisoimage -o $@ $(GENISOIMAGE_PARAMETERS) $(STAGING_DIR)

# Woe32 autorun stub...
$(WOE32STUB_DIR)/Woe32Stub.exe:
	cd $(WOE32STUB_DIR)/ && make

# Generate checksum...
MD5SUM: $(ISO)
	@echo "Generating image checksum. Please wait..."
	@md5sum $(ISO) > $@

# Check what we can...
check: $(ISO)
	@isovfy -i $(ISO)
	@echo "Verifying image checksum. Please wait..."
	@md5sum --warn --check MD5SUM

# Clean up...
clean:
	-@$(RM) -v $(ISO) vlr.stamp MD5SUM
	cd $(VLR_DIR)/Data/Translations && \
	make clean && \
	INSTALL_DIR=$(STAGING_DIR)/Launcher/Data/Translations make uninstall-po
	cd $(WOE32STUB_DIR)/ && make clean

# Copy the VLR into the image with translations built...
vlr.stamp: $(VLR_DIR)/Source/*.py
	@rsync --delete --delete-excluded -avz --exclude-from=$(EXCLUDE_LIST) --progress --partial $(VLR_DIR) $(STAGING_DIR)/
	cd $(VLR_DIR)/Data/Translations && make && INSTALL_DIR=$(STAGING_DIR)/Launcher/Data/Translations make install-po
	touch vlr.stamp

# Directive to make to let it know that these targets don't generate filesystem 
#  objects / products and therefore no need to check time stamps...
.PHONY: \
  all   \
  check \
  clean


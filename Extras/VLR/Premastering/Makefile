# VikingExtractor, to recover images from Viking Lander operations.
# Copyright (C) 2010-2014 Cartesian Theatre <info@cartesiantheatre.com>.
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
all: $(ISO)

# Build the image by coalescing everything needed from various places. End by
#  injecting a series of MD5 checksums into the image. These are stored in the 
#  ISO 9660's 'Application Use' area in byte positions 884 to 1395. See section 
#  8.4.32 of the Standard ECMA-119 - Volume and File Structure of CDROM for 
#  Information Interchange on how this can be done safely.
#  <http://www.ecma-international.org/publications/standards/Ecma-119.htm>
$(ISO):                             \
  $(INCLUDE_LIST)                   \
  $(EXCLUDE_LIST)                   \
  $(OPTIMIZATION_LIST)              \
  XDG/*                             \
  ve_l10n.stamp                     \
  vlr.stamp                         \
  $(WOE32STUB_DIR)/Woe32Stub.exe
	@chmod +x -c XDG/autorun
	genisoimage -o $@ $(GENISOIMAGE_PARAMETERS) $(STAGING_DIR)
	@echo "Preparing injectable MD5 checksum, please wait..."
	@implantisomd5 $(ISO)

# Woe32 autorun stub...
$(WOE32STUB_DIR)/Woe32Stub.exe:
	cd $(WOE32STUB_DIR)/ && make

# Generate a detached MD5 checksum. Note that this is calculated differently 
#  than the embedded MD5 checksum because the latter is calculated via isomd5sum
#  tools which skips the last 15 sectors for compatibility reasons...
MD5SUM: $(ISO)
	@echo "Generating detached MD5 image checksum, please wait..."
	@#pv --progress -B 8m $(ISO) | md5sum > $@
	@#@sed 's/-/$(ISO)/g' $@
	@md5sum $(ISO) > $@

# Generate a digital signature on the detached MD5 checksum...
sign: MD5SUM.gpg
MD5SUM.gpg: MD5SUM
	gpg2 --output $@ --armour --detach-sign $<

# Check VLR / VE translations that were staged, general iso9660 image structure, 
#  and finally verify detached and embedded image checksums...
check: $(ISO) MD5SUM.gpg
	cd $(EXTRACTOR_DIR)/Translations && make installcheck localedir=$(STAGING_DIR)/Extractor/locale
	cd $(STAGING_DIR)/Launcher/Data/Translations && make check
	@echo "Verifying general iso9660 structure, please wait..."
	@isovfy -i $(ISO)
	@echo "Verifying detached MD5 image checksum, please wait..."
	@md5sum --warn --check MD5SUM
	@echo "Verifying embedded MD5 image checksum, please wait..."
	@checkisomd5 --verbose $(ISO)
	@echo "Verifying maintainer's digital signature..."
	@gpg2 --verify MD5SUM.gpg MD5SUM

# Clean up everything...
clean: vlr.clean ve_l10n.clean
	-@$(RM) -v $(ISO) MD5SUM MD5SUM.gpg
	cd $(WOE32STUB_DIR)/ && make $@

# Inject VikingExtractor message catalogues into staging area...
ve_l10n.stamp:
	cd $(EXTRACTOR_DIR)/Translations && \
	make update-gmo && \
	make install localedir=$(STAGING_DIR)/Extractor/locale
	touch $@

# Inject VLR into staging area, compiling message catalogues in place...
vlr.stamp: $(VLR_DIR)/Source/*.py
	@rsync --delete --delete-excluded -avz --exclude-from=$(EXCLUDE_LIST) --progress --partial $(VLR_DIR) $(STAGING_DIR)/
	cd $(VLR_DIR)/Data/Translations && \
	make update-gmo && \
	make install-po localedir=$(STAGING_DIR)/Launcher/Data/Translations
	touch $@

# Clean the above...
ve_l10n.clean:
	cd $(EXTRACTOR_DIR)/Translations && \
	make clean && \
	make uninstall localedir=$(STAGING_DIR)/Extractor/locale
	cd $(STAGING_DIR)/Extractor/locale && find . -print -type d -empty -delete
	-@$(RM) -v ve_l10n.stamp

# Clean the above...
vlr.clean:
	cd $(VLR_DIR)/Data/Translations && \
	make clean
	-@$(RM) -Rfv $(STAGING_DIR)/Launcher
	-@$(RM) -v vlr.stamp 

# Directive to make to let it know that these targets don't generate filesystem 
#  objects / products and therefore no need to check time stamps...
.PHONY:         \
  all           \
  check         \
  clean         \
  sign          \
  vlr.clean     \
  ve_l10n.clean


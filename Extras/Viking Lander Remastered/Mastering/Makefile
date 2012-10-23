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

UDF                 = Avaneya_Viking_Lander_Remastered.udf
DVD_9_SIZE          = 8540000000 # Single sided, dual layer, 8.54 billion bytes...
DISC_SIZE           = DVD_9_SIZE
PREPARER_ID         = "Cartesian Theatre"
PUBLISHER_ID        = "Cartesian Theatre"
VOLUME_ID           = "Avaneya VLR"
MANIFEST            = ./Manifest
LAYOUT_OPTIMIZATION = ./LayoutOptimization
MKUDFFS_PARAMETERS  = --udfrev=0x0201 \
                      --media-type=dvd \
                      --blocksize=2048 \
                      --utf8
USER_ID             = $(USER)

# Disable checking for files by these names...
#.PHONY: all clean check

# Default target is to built the complete ISO...
all: udf

# Build the image...
udf: $(UDF)
$(UDF): viking-extractor Manifest Makefile
	chmod +x -c ../Launcher/Source/autorun.sh
	#truncate -s $(DISC_SIZE)
	mkudffs $(MKUDFFS_PARAMETERS) $(UDF)
	mkdir -vp udfmount/
	sudo mount -t udf -o loop,user,rw,uid=$(USER_ID) $(UDF) udfmount/
	touch udfmount/foo
	sudo umount /dev/loop

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


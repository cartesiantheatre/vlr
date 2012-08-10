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

ISO = Viking_Lander_Remastered.iso
GENISOIMAGE_PARAMETERS = -graft-points -input-charset utf-8 -r -J -udf \
    -iso-level 3 -path-list Manifest -volid "VIKING_LANDER_REMASTERED"

# Disable checking for files by these names...
#.PHONY: all clean check

# Default target is to built the complete ISO...
all: $(ISO)

# Build the ISO...
$(ISO): Extractor/viking-extractor autorun.sh Manifest Makefile
	chmod +x -c ./autorun.sh
	genisoimage $(GENISOIMAGE_PARAMETERS) -o $(ISO)

# Build viking-extractor...
Extractor/viking-extractor:
	@(cd Extractor; $(MAKE))

# Clean up...
clean:
	-@$(RM) $(ISO)
	@(cd Extractor; $(MAKE) clean) 


# VikingExtractor, to recover images from Viking Lander operations.
# Copyright (C) 2010, 2011 Kshatra Corp <kip@thevertigo.com>.
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

CPP                 = g++
LIBS                = `libpng-config --ldflags` -locrad
BIN                 = viking-extractor
SOURCE_DIRECTORIES  := Source
CXXFLAGS            = -g3 -Wall -Werror `libpng-config --cflags`
SOURCE_FILES        := $(shell find $(SOURCE_DIRECTORIES) -type f -name "*.cpp")
DEPENDENCY_FILES    := $(patsubst %.cpp, %.d, $(SOURCE_FILES))
OBJECT_FILES        := $(patsubst %.cpp, %.o, $(SOURCE_FILES))

# Disable checking for files by these names...
#.PHONY: all clean check

# Default target is to built the executable...
all: $(BIN)

# Include the list of dependencies, the - carries on if an error 
#  occured because one of them might not exist yet...
-include $(DEPENDENCY_FILES)

# Build the executable...
$(BIN): $(OBJECT_FILES)
	@echo "Linking $@"
	@$(CPP) $(OBJECT_FILES) -o $(BIN) $(LIBS)

# Build a single object file...
%.o: %.cpp
	@echo "Compiling $<"
	@$(CPP) $(CXXFLAGS) -MMD -MP -c $< -o $@

# Clean up...
clean:
	-@$(RM) $(wildcard $(OBJECT_FILES) $(DEPENDENCY_FILES) $(BIN))


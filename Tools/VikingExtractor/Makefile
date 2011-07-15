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

CPP         = g++
OBJ         = LogicalRecord.o           \
              VikingExtractor.o         \
              VicarImageAssembler.o     \
              VicarImageBand.o
LIBS        = `libpng-config --ldflags`
INCS        =
CXXINCS     =
BIN         = VikingExtractor
CXXFLAGS    = $(CXXINCS) $(INCS) -g3 -Wall -Werror `libpng-config --cflags`
SOURCES     = LogicalRecord.cpp         \
              VicarImageAssembler.cpp   \
              VicarImageBand.cpp        \
              VikingExtractor.cpp

.PHONY: all
all: $(BIN)

.SUFFIXES: .cpp

# Take the list of sources, replace extension with .d to include as
#  additional makefiles...
-include $(subst .cpp,.d,$(SOURCES))

%.d: %.cpp
	@$(CPP) -M $(CXXFLAGS) $< > $@.$$$$;                    \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@;     \
	rm -f $@.$$$$

.cpp.o:
	$(CPP) -c $< $(CXXFLAGS)

clean:
	@rm -f *.d
	@rm -fv $(OBJ) $(BIN) *~

$(BIN): $(OBJ)
	$(CPP) $(OBJ) -o $(BIN) $(LIBS)


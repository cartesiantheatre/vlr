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

# Makefile to generate a Woe32 executable stub...

CPP         = i586-mingw32msvc-g++
RC          = i586-mingw32msvc-windres
OBJ         = Main.o \
              Resources.o
LIBS        = 
INCS        =
CXXINCS     =

# Executable name taken from GNU Coding Standards section 5.5, Portability 
#  between System Types...
BIN         = Woe32Stub.exe

CXXFLAGS    = $(CXXINCS) $(INCS) -g3 -Wall -Werror

SOURCES     = Main.cpp

.PHONY: all
all: $(BIN)

.SUFFIXES: .cpp

-include $(subst .cpp,.d,$(SOURCES))

%.d: %.cpp
	@$(CPP) -M $(CXXFLAGS) $< > $@.$$$$;                \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

Resources.o: Resources.rc
	$(RC) --input-format rc --input $< --output-format coff --output $@

.cpp.o:
	$(CPP) -c $< $(CXXFLAGS)

clean:
	@rm -fv $(OBJ) $(BIN) *~ *.d

$(BIN): $(OBJ)
	$(CPP) $(OBJ) -o $(BIN) $(LIBS) -mwindows


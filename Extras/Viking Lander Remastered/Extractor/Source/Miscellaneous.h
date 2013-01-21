/*
    VikingExtractor, to recover images from Viking Lander operations.
    Copyright (C) 2010-2013 Cartesian Theatre <kip@thevertigo.com>.
    
    Public discussion on IRC available at #avaneya (irc.freenode.net) 
    or on the mailing list <avaneya@lists.avaneya.com>.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// Multiple include protection...
#ifndef _MISCELLANEOUS_H_
#define _MISCELLANEOUS_H_

// Includes...
#include <string>
#include <algorithm>

// Create a directory and all of its parents, if necessary...
bool CreateDirectoryRecursively(const std::string &Path);

// Three way min() / max() function templates...
template<class T> const T &min3(const T &A, const T &B, const T &C) { return min(A, min(B, C)); }
template<class T> const T &max3(const T &A, const T &B, const T &C) { return max(A, max(B, C)); }

// Convert a given Martian solar day in the range [1 .. n] to Ls angle...
float SolarDayToLs(const size_t SolarDay);

// Helpful macros...
#define SetErrorAndReturn(Message)              { SetErrorMessage((Message)); return; }
#define SetErrorAndReturnNullStream(Message)    { SetErrorMessage((Message)); return ostream(0); }
#define SetErrorAndReturnFalse(Message)         { SetErrorMessage((Message)); return false; }

// fnmatch() shell matching patterns...
#define FNMATCH_ANY_ZIP     "*.[Zz][Ii][Pp]"
#define FNMATCH_ANY_VICAR   "*vl_[0-9][0-9][0-9][0-9].[0-9][0-9][0-9]"

// Multiple include protection...
#endif


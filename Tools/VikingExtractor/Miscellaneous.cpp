/*
    VikingExtractor, to recover images from Viking Lander operations.
    Copyright (C) 2010, 2011 Kshatra Corp <kip@thevertigo.com>.
    
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

// Includes...
#include "Miscellaneous.h"
#include <sys/stat.h>
#include <string>

// Using the standard name space...
using namespace std;

// Create a directory and all of its parents, if necessary...
bool CreateDirectoryRecursively(const std::string &Path)
{
    // Make a copy of the user's string, since we need to change it...
    string PathCopy = Path;

    // Remove trailing path separator...
    if(PathCopy.at(PathCopy.length() - 1) == '/')
        PathCopy.erase(PathCopy.length() - 1);

    // Create the directories from the parent to child order...
    for(string::const_iterator Iterator = PathCopy.begin(); Iterator != PathCopy.end(); ++Iterator)
    {
        // Found the next partial path, create...
        if(*Iterator == '/')
        {
            // Get path up to this point...
            const string PartialPath(PathCopy, 0, Iterator - PathCopy.begin());

            // If it doesn't already exist, create it...
            if(access(PartialPath.c_str(), F_OK) != 0)
                mkdir(PartialPath.c_str(), S_IRWXU);
        }
    }

    // Check if final nested directory exists, if not, create...
    if(access(PathCopy.c_str(), F_OK) != 0)
        return (mkdir(PathCopy.c_str(), S_IRWXU) == 0);
    
    // Otherwise path already exists...
    else
        return true;
}


/*
    VikingExtractor, to recover images from Viking Lander operations.
    Copyright (C) 2010, 2011, 2012 Cartesian Theatre <kip@thevertigo.com>.
    
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
#ifndef _ZZIP_FILE_HANDLE_H_
#define _ZZIP_FILE_HANDLE_H_

// Includes...

    // zziplib...
    #include <zzip/zzip.h>
    
    // System headers...
    #include <cassert>

// ZZip file handle with safe RIAA...
class ZZipFileHandle
{
    // Public methods...
    public:
    
        // Constructor...
        ZZipFileHandle(const ZZIP_DIR *ArchiveHandle, const ZZIP_FILE *FileHandle);

        // Check if the handle is valid...
        bool IsGood() const;
        
        // Get the internal file handle...
        operator ZZIP_FILE *() const { assert(IsGood()); return m_FileHandle; }

        // Destructor...
       ~ZZipFileHandle();

    // Protected methods...
    protected:
        
        // Archive handle...
        ZZIP_DIR *m_ArchiveHandle;
        
        // File within the archive handle...
        ZZIP_FILE *m_FileHandle;
};

// Multiple include protection...
#endif


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
#ifndef _ZZIP_FILE_DESCRIPTOR_H_
#define _ZZIP_FILE_DESCRIPTOR_H_

// Includes...

    // zziplib...
    #include <zzip/zzip.h>
    
    // System headers...
    #include <cassert>

// ZZip file descriptor with safe RIAA...
class ZZipFileDescriptor
{
    // Public methods...
    public:
    
        // Constructor...
        ZZipFileDescriptor(const ZZIP_FILE *FileDescriptor);

        // Check if the handle is valid, not EOF, etc...
        bool IsGood() const;
        
        // Get the internal file handle...
        operator ZZIP_FILE *() const { assert(IsGood()); return m_FileDescriptor; }

        // Destructor...
       ~ZZipFileDescriptor();

    // Protected methods...
    protected:
        
        // Archive descriptor...
        ZZIP_DIR *m_ArchiveDescriptor;
        
        // File within the archive descriptor...
        ZZIP_FILE *m_FileDescriptor;
};

// Multiple include protection...
#endif


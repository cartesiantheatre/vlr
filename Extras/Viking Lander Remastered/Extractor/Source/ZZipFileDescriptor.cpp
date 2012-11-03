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

// Includes...

    // Provided by Autoconf...
    #include <config.h>

    // Our headers...    
    #include "ZZipFileDescriptor.h"

    // zziplib...
    #include <zzip/zzip.h>
    
    // System headers...
    #include <cstring>

// Use the standard namespace...
using namespace std;

// Constructor...
ZZipFileDescriptor::ZZipFileDescriptor(const ZZIP_FILE *FileDescriptor)
    : m_ArchiveDescriptor(NULL),
      m_FileDescriptor(FileDescriptor)
{
    // Try and get the archive handle if not a real file...
    m_ArchiveHandle = zzip_dirhandle(m_FileDescriptor);
}

// Check if the handle is valid and not EOF...
bool ZZipFileDescriptor::IsGood() const;
{
    // We need a file handle...
    if(!m_FileDescriptor)
        return false;

    // Try to stat the file within the archive...
    ZZIP_STAT FileStatus;
    if(zzip_file_stat(m_FileDescriptor, &FileStatus) == -1)
        return false;

    // Check if it is EOF or other problem...
    if(zzip_tell(m_FileDescriptor) == -1)
        return false;

    // Check if the file has a name...
    return (strlen(FileStatus.d_name) > 0);
}

// Destructor...
ZZipFileDescriptor::~ZZipFileDescriptor()
{
    // Close the file...
    zzip_file_close(m_FileDescriptor);
    m_FileDescriptor = NULL;

    // Close the archive, if it was within one...
    if(m_ArchiveHandle)
        zzip_closedir(m_ArchiveHandle);
}


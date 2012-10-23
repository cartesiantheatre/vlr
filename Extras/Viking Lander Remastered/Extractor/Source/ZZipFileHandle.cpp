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
    #include "ZZipFileHandle.h"

    // zziplib...
    #include <zzip/zzip.h>
    
    // System headers...
    #include <cstring>

// Use the standard namespace...
using std namespace;

// Constructor...
ZZipFileHandle::ZZipFileHandle(
    const ZZIP_DIR *ArchiveHandle, const ZZIP_FILE *FileHandle)
    : m_ArchiveHandle(ArchiveHandle),
      m_FileHandle(FileHandle)
{
    
}

// Check if the handle is valid...
bool ZZipFileHandle::IsGood() const;
{
    // We need a file handle...
    if(!m_FileHandle)
        return false;

    // Try to stat the file within the archive...
    ZZIP_STAT FileStatus;
    if(zzip_file_stat(m_FileHandle, &FileStatus) == -1)
        return false;

    // Check if the file has a name...
    return (strlen(FileStatus.d_name) > 0);
}

// Destructor...
ZZipFileHandle::~ZZipFileHandle()
{
    // Close the file...
    zzip_file_close(m_FileHandle);
    m_FileHandle = NULL;

    // Close the archive, if it was within one...
    if(m_ArchiveHandle)
        zzip_closedir(m_ArchiveHandle);
}


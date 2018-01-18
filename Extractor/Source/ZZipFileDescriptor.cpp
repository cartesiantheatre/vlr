/*
    VikingExtractor, to recover images from Viking Lander operations.
    Copyright (C) 2010-2018 Cartesian Theatre™ <info@cartesiantheatre.com>.
    
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

// Use the standard namespace...
using namespace std;

// Construct around a real file...
ZZipFileDescriptor::ZZipFileDescriptor(const std::string &RealFileName)
    : m_RealFileName(RealFileName),
      m_ArchiveDescriptor(NULL),
      m_FileDescriptor(NULL)
{
    // Open the real file...
    m_FileDescriptor = zzip_fopen(RealFileName.c_str(), O_RDONLY);
}

// Construct around a compressed file within an archive...
ZZipFileDescriptor::ZZipFileDescriptor(
    const std::string &ArchiveFileName, const std::string &CompressedFileName)
    : m_ArchiveFileName(ArchiveFileName),
      m_CompressedFileName(CompressedFileName),
      m_ArchiveDescriptor(NULL),
      m_FileDescriptor(NULL)
{
    // Open archive...
    m_ArchiveDescriptor = zzip_dir_open(m_ArchiveFileName.c_str(), NULL);

        // Failed...
        if(!m_ArchiveDescriptor)
            return;

    // Open the compressed file within the archive...
    m_FileDescriptor = zzip_file_open(m_ArchiveDescriptor, CompressedFileName.c_str(), 0);

        // Failed...
        if(!m_FileDescriptor)
        {
            // Cleanup, abort...
            zzip_closedir(m_ArchiveDescriptor);
            return;
        }
}

// Copy constructor...
ZZipFileDescriptor::ZZipFileDescriptor(const ZZipFileDescriptor &Source)
    : m_RealFileName(Source.m_RealFileName),
      m_ArchiveFileName(Source.m_ArchiveFileName),
      m_CompressedFileName(Source.m_CompressedFileName),
      m_ArchiveDescriptor(NULL),
      m_FileDescriptor(NULL)
{
    // The source wraps a real file...
    if(zzip_file_real(Source.m_FileDescriptor))
        m_FileDescriptor = zzip_fopen(m_RealFileName.c_str(), O_RDONLY);

    // The source wraps a compressed file within an archive...
    else
    {
        // Open archive...
        m_ArchiveDescriptor = zzip_dir_open(m_ArchiveFileName.c_str(), NULL);

            // Failed...
            if(!m_ArchiveDescriptor)
                return;

        // Open the compressed file within the archive...
        m_FileDescriptor = zzip_file_open(
            m_ArchiveDescriptor, m_CompressedFileName.c_str(), 0);

            // Failed...
            if(!m_FileDescriptor)
            {
                // Cleanup, abort...
                zzip_closedir(m_ArchiveDescriptor);
                m_ArchiveDescriptor = NULL;
                return;
            }
    }
    
    // Restore the archive descriptor pointer...
    const zzip_off_t ArchiveReadPointer = zzip_telldir(Source.m_ArchiveDescriptor);
    zzip_seekdir(m_ArchiveDescriptor, ArchiveReadPointer);

    // Restore the file descriptor pointer...
    const zzip_off_t CompressedFileReadPointer = 
        zzip_tell(Source.m_FileDescriptor);
    zzip_seek(m_FileDescriptor, CompressedFileReadPointer, SEEK_SET);
}

// Check if the handle is valid and not EOF...
bool ZZipFileDescriptor::IsGood() const
{
    // We need a file handle...
    if(!m_FileDescriptor)
        return false;

    // Try to stat the file... (possibly within the archive)
    ZZIP_STAT FileStatus;
    if(zzip_fstat(m_FileDescriptor, &FileStatus) == -1)
        return false;

    // Check if it is EOF or other problem...
    if(zzip_tell(m_FileDescriptor) == -1)
        return false;

    // File descriptor probably points to a valid file...
    return true;
}

// Destructor...
ZZipFileDescriptor::~ZZipFileDescriptor()
{
    // Close the file, if opened...
    if(m_FileDescriptor)
        zzip_close(m_FileDescriptor);

    // Close the containing archive, if it was within one...
    if(m_ArchiveDescriptor)
    {
        zzip_closedir(m_ArchiveDescriptor);
    }
}


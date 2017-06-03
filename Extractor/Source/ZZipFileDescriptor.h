/*
    VikingExtractor, to recover images from Viking Lander operations.
    Copyright (C) 2010-2017 Cartesian Theatre™ <info@cartesiantheatre.com>.
    
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
    #include <string>
    #include <clocale>

    // i18n...
    #include "gettext.h"
    #define _(str) gettext (str)
    #define N_(str) gettext_noop (str)

// ZZip file descriptor with safe RIAA...
class ZZipFileDescriptor
{
    // Public methods...
    public:
    
        // Construct around a real file...
        ZZipFileDescriptor(const std::string &RealFileName);

        // Construct around a compressed file within an archive...
        ZZipFileDescriptor(
            const std::string &ArchiveFileName, 
            const std::string &CompressedFileName);

        // Copy constructor...
        ZZipFileDescriptor(const ZZipFileDescriptor &Source);

        // Check if the handle is valid, not EOF, etc...
        bool IsGood() const;
        
        // Get the file descriptor...
        operator ZZIP_FILE *() const { assert(IsGood()); return m_FileDescriptor; }

        // Destructor...
       ~ZZipFileDescriptor();

    // Protected methods...
    protected:

        // If this wraps a real file, this is its file name...
        const std::string   m_RealFileName;

        // If this wraps a compressed file, this is the archive file name and 
        //  the file name of the compressed file within it...
        const std::string   m_ArchiveFileName;
        const std::string   m_CompressedFileName;

        // Archive and file descriptors...
        ZZIP_DIR           *m_ArchiveDescriptor;
        ZZIP_FILE          *m_FileDescriptor;
};

// Multiple include protection...
#endif


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

// Multiple include protection...
#ifndef _LOGICAL_RECORD_H_
#define _LOGICAL_RECORD_H_

// Includes...

    // Our headers...
    #include "ZZipFileDescriptor.h"

    // System headers...
    #include <string>
    #include <stdint.h>
    #include <clocale>

    // i18n...
    #include "gettext.h"
    #define _(str) gettext (str)
    #define N_(str) gettext_noop (str)

// Logical buffer size...
#define LOGICAL_RECORD_SIZE 72

/*

    Great diagram of how this works here. Wish I had found this back when I had
    started writing this.
    
        <http://archive.stsci.edu/iue/manual/iuesips/section8.html#fig8-4>

*/

// Logical record...
class LogicalRecord
{
    // Public methods...
    public:
        
        // Default constructor...
        LogicalRecord();
        
        // Constructor from an input stream...
        LogicalRecord(ZZipFileDescriptor &FileDescriptor);

        // Get a string or substring, stripping non-friendly bytes. If
        //  trim is true will strip leading and trailing whitespace 
        //  and logical record markers...
        std::string GetString(
            const bool Trim = false,
            const size_t Start = 0, 
            const size_t Size = 0) const;

        // Is this the last label or does more follow? Throws error...
        bool IsLastLabel() const;

        // Is this a valid label?
        bool IsValidLabel() const;

        // Load the buffer from a stream and decode, or throw an error...
        void operator<<(ZZipFileDescriptor &FileDescriptor);
        
        // Convert to a string...
        operator std::string() const { return GetString(); }
        
        // Index operator...
        char &operator[](const size_t Index);
        const char &operator[](const size_t Index) const;
        
        // Reset the buffer...
        void Reset();

    // Protected methods...
    protected:

        // Convert ASCII to EBCDIC encoded character...
        char AsciiToEbcdic(const uint8_t AsciiCharacter) const;

        // Convert EBCDIC to ASCII encoded character...
        char EbcdicToAscii(const uint8_t EbcdicCharacter) const;

    // Protected constants...
    protected:
    
        // ASCII to EBCDIC table provided by Bob Stout...
        static const uint8_t    ms_AsciiToEbcdicTable[256];

        // EBCDIC to ASCII table provided by Bob Stout...
        static const uint8_t    ms_EbcdicToAsciiTable[256];
        
        // Label sentinal signatures...
        typedef enum
        {
            ContinuationSentinal    = 'C',  /* Continuation code */
            EndSentinal             = 'L'   /* Can occur in any logical record 
                                               and marks end of physical 
                                               record */
        }LabelSentinalMarker_t;
        
    // Protected data...
    protected:

        // Buffer is always 72 bytes long, but add one so always
        //  safely NULL terminated...
        char m_Buffer[LOGICAL_RECORD_SIZE + 1];
};

// Multiple include protection...
#endif


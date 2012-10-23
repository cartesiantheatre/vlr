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
    #include "LogicalRecord.h"
    
    // System headers...
    #include <iostream>
    #include <cassert>
    #include <cstring>

// Using the standard namespace...
using namespace std;

// Initialize constants...

    // ASCII to EBCDIC table provided by Bob Stout...
    const uint8_t LogicalRecord::ms_AsciiToEbcdicTable[256] = {
          0,  1,  2,  3, 55, 45, 46, 47, 22,  5, 37, 11, 12, 13, 14, 15,
         16, 17, 18, 19, 60, 61, 50, 38, 24, 25, 63, 39, 28, 29, 30, 31,
         64, 79,127,123, 91,108, 80,125, 77, 93, 92, 78,107, 96, 75, 97,
        240,241,242,243,244,245,246,247,248,249,122, 94, 76,126,110,111,
        124,193,194,195,196,197,198,199,200,201,209,210,211,212,213,214,
        215,216,217,226,227,228,229,230,231,232,233, 74,224, 90, 95,109,
        121,129,130,131,132,133,134,135,136,137,145,146,147,148,149,150,
        151,152,153,162,163,164,165,166,167,168,169,192,106,208,161,  7,
         32, 33, 34, 35, 36, 21,  6, 23, 40, 41, 42, 43, 44,  9, 10, 27,
         48, 49, 26, 51, 52, 53, 54,  8, 56, 57, 58, 59,  4, 20, 62,225,
         65, 66, 67, 68, 69, 70, 71, 72, 73, 81, 82, 83, 84, 85, 86, 87,
         88, 89, 98, 99,100,101,102,103,104,105,112,113,114,115,116,117,
        118,119,120,128,138,139,140,141,142,143,144,154,155,156,157,158,
        159,160,170,171,172,173,174,175,176,177,178,179,180,181,182,183,
        184,185,186,187,188,189,190,191,202,203,204,205,206,207,218,219,
        220,221,222,223,234,235,236,237,238,239,250,251,252,253,254,255
    };

    // EBCDIC to ASCII table provided by Bob Stout...
    const uint8_t LogicalRecord::ms_EbcdicToAsciiTable[256] = {
          0,  1,  2,  3,156,  9,134,127,151,141,142, 11, 12, 13, 14, 15,
         16, 17, 18, 19,157,133,  8,135, 24, 25,146,143, 28, 29, 30, 31,
        128,129,130,131,132, 10, 23, 27,136,137,138,139,140,  5,  6,  7,
        144,145, 22,147,148,149,150,  4,152,153,154,155, 20, 21,158, 26,
         32,160,161,162,163,164,165,166,167,168, 91, 46, 60, 40, 43, 33,
         38,169,170,171,172,173,174,175,176,177, 93, 36, 42, 41, 59, 94,
         45, 47,178,179,180,181,182,183,184,185,124, 44, 37, 95, 62, 63,
        186,187,188,189,190,191,192,193,194, 96, 58, 35, 64, 39, 61, 34,
        195, 97, 98, 99,100,101,102,103,104,105,196,197,198,199,200,201,
        202,106,107,108,109,110,111,112,113,114,203,204,205,206,207,208,
        209,126,115,116,117,118,119,120,121,122,210,211,212,213,214,215,
        216,217,218,219,220,221,222,223,224,225,226,227,228,229,230,231,
        123, 65, 66, 67, 68, 69, 70, 71, 72, 73,232,233,234,235,236,237,
        125, 74, 75, 76, 77, 78, 79, 80, 81, 82,238,239,240,241,242,243,
         92,159, 83, 84, 85, 86, 87, 88, 89, 90,244,245,246,247,248,249,
         48, 49, 50, 51, 52, 53, 54, 55, 56, 57,250,251,252,253,254,255
    };

// Default constructor...
LogicalRecord::LogicalRecord()
{
    // Clear the buffer...
    Reset();
}

// Constructor from an input stream...
LogicalRecord::LogicalRecord(std::istream &InputStream)
{
    // Clear the buffer...
    Reset();

    // Load from stream...
   *this << InputStream;
}

// Get a string or substring, stripping non-friendly bytes. If trim is
//  true will strip leading and trailing whitespace and logical record 
//  markers...
string LogicalRecord::GetString(
    const bool Trim, const size_t Start, const size_t Size) const
{
    // Calculate last byte index...
    const size_t End = (Size == 0) ? LOGICAL_RECORD_SIZE - 1 
                                   : Size;

    // Build string...
    std::string Selection;
    for(size_t Index = Start; Index <= End; ++Index)
    {
        // If trim was requested, skip trailing two byte logical 
        //  record markers...
        if(Trim && (Index >= LOGICAL_RECORD_SIZE - 2))
            continue;

        // Only want printable characters...
        if(isprint(m_Buffer[Index]))
            Selection += m_Buffer[Index];
    }

    // If trim was enabled, trim leading and trailing white space...
    if(Trim)
    {
        // Trim leading white space if some was found...
        size_t StartIndex = Selection.find_first_not_of(" \t");
        if(string::npos != StartIndex)
            Selection = Selection.substr(StartIndex, Selection.length() - StartIndex);

        // Trim trailing white space if some was found...
        size_t EndIndex = Selection.find_last_not_of(" \t");
        if(string::npos != EndIndex)
            Selection = Selection.substr(0, EndIndex + 1);
    }

    // Done...
    return Selection;
}

// Convert ASCII to EBCDIC encoded character...
char LogicalRecord::AsciiToEbcdic(const uint8_t AsciiCharacter) const
{
    // Lookup...
    return ms_AsciiToEbcdicTable[AsciiCharacter];
}

// Convert EBCDIC to ASCII encoded character...
char LogicalRecord::EbcdicToAscii(const uint8_t EbcdicCharacter) const
{
    // Lookup
    return ms_EbcdicToAsciiTable[EbcdicCharacter];
}

// Is this the last label or does more follow? Throws error...
bool LogicalRecord::IsLastLabel() const
{
    // Check...
    switch(m_Buffer[LOGICAL_RECORD_SIZE - 1])
    {
        // No...
        case 'C':
            return false;
        
        // Yes...
        case 'L':
            return true;

        // Not a valid label...
        default:
            throw std::string("invalid logical record label");
    }
}

// Is this a valid label?
bool LogicalRecord::IsValidLabel() const
{
    // It should only contain printable characters past the first two bytes...
    for(size_t Index = 2; Index < LOGICAL_RECORD_SIZE; ++Index)
    {
        // Binary junk found, probably not a valid label record...
        if(!isprint(m_Buffer[Index]))
            return false;
    }

    // Check for delimeter...
    switch(m_Buffer[LOGICAL_RECORD_SIZE - 1])
    {
        // Yes...
        case 'C':
        case 'L':
            return true;
        
        // No...
        default:
            return false;
    }
}

// Load the buffer from a stream and decode, or throw an error. Always 
//  consumes exactly LOGICAL_RECORD_SIZE bytes when successful...
void LogicalRecord::operator<<(std::istream &InputStream)
{
    // Fill the whole buffer and check for error...
    if(LOGICAL_RECORD_SIZE != InputStream.readsome(m_Buffer, LOGICAL_RECORD_SIZE))
        throw std::string("failed to read from input stream");

    // Decode...
    for(size_t Index = 0; Index < LOGICAL_RECORD_SIZE; ++Index)
        m_Buffer[Index] = EbcdicToAscii(m_Buffer[Index]);
}

// Index operator...
char &LogicalRecord::operator[](const size_t Index)
{
    // Bounds check...
    assert(Index < LOGICAL_RECORD_SIZE);

    // Return reference...
    return m_Buffer[Index];
}

// Read only index operator...
const char &LogicalRecord::operator[](const size_t Index) const
{
    // Bounds check...
    assert(Index < LOGICAL_RECORD_SIZE);

    // Return reference...
    return m_Buffer[Index];
}

// Reset the buffer...
void LogicalRecord::Reset()
{
    // Buffer is always 72 bytes long, but add one so always
    //  safely NULL terminated...
    memset(m_Buffer, '\x0', LOGICAL_RECORD_SIZE + 1);                
}


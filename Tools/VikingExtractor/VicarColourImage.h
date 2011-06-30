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

// Multiple include protection...
#ifndef _VICAR_COLOUR_IMAGE_H_
#define _VICAR_COLOUR_IMAGE_H_

// Includes...
#include <string>
#include <stdint.h>

// 1970s era VICAR colour image class...
class VicarColourImage
{
    // Public methods...
    public:

        // Constructor...
        VicarColourImage(const std::string &InputFile, const bool Verbose = false);
        
        // Is the file accessible and the header ok?
        bool IsOk() const { return m_Ok; }
        
        // Is verbosity set...
        bool IsVerbose() { return m_Verbose; }
        
        // Set verbosity flag...
        void SetVerbose(const bool Verbose = true) { m_Verbose = Verbose; }

    // Protected methods...
    protected:
    
        // Read VICAR image header, or throw an error...
        void ReadHeader();

    // Protected data...
    protected:

        // Input file name...
        const std::string   m_InputFile;

        // Number of colour channel bands. e.g 3 means RGB...
        int                 m_Bands;
        
        // Image height and width in pixels...
        int                 m_Height;
        int                 m_Width;
        
        // Bytes per pixel...
        int                 m_BytesPerPixel;
        
        // True if the header appears to be ok...
        bool                m_Ok;
        
        // Verbosity flag...
        bool                m_Verbose;
};

// Multiple include protection...
#endif


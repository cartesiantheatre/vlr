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
#include "VicarColourImage.h"
#include <iostream>
#include <fstream>

// Using the standard namespace...
using namespace std;

// Constructor...
VicarColourImage::VicarColourImage(
    const std::string &InputFile, const bool Verbose)
    : m_InputFile(InputFile),
      m_Bands(0), 
      m_Height(0),
      m_Width(0),
      m_BytesPerPixel(0),
      m_Ok(false),
      m_Verbose(Verbose)
{
    // Try to read the VICAR image header...
    try
    {
        ReadHeader();
    }
        // Failed...
        catch(const std::string & ErrorMessage)
        {
            // Alert...
            cerr << ErrorMessage << endl;
        }
}

// Read VICAR image header, or throw an error...
void VicarColourImage::ReadHeader()
{
    // Open the file...
    ifstream InputFileStream(m_InputFile.c_str(), ifstream::in | ifstream::binary);
    
        // Failed...
        if(!InputFileStream.is_open())
            throw std::string("Could not open input for reading...");

    // Loaded ok...
    m_Ok = true;
}


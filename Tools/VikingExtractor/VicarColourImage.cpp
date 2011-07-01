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
    
    // Ourself...    
    #include "VicarColourImage.h"
    
    // Logical record access...
    #include "LogicalRecord.h"
    
    // PNG writing...
    #include <png++/png.hpp>
    
    // Streams...
    #include <iostream>
    #include <sstream>
    #include <fstream>
    
    // String access and manipulation...
    #include <cstring>
    
    // max()...
    #include <algorithm>

// Using the standard namespace...
using namespace std;

// Construct and read the header, or throw an error...
VicarColourImage::VicarColourImage(
    const std::string &InputFile, const bool Verbose)
    : m_InputFile(InputFile),
      m_Bands(0), 
      m_Height(0),
      m_Width(0),
      m_BytesPerColour(0),
      m_Ok(false),
      m_Verbose(Verbose)
{
    // Try to load the file...
    try
    {
        // Read the VICAR image header...
        ReadHeader();
    }
        // Failed...
        catch(const std::string & ErrorMessage)
        {
            // Alert...
            cerr << "Error: " << ErrorMessage << endl;
        }
}

// Read VICAR image header, or throw an error...
void VicarColourImage::ReadHeader()
{
    // Objects and variables...
    LogicalRecord   Record;
    char            DummyCharacter;
    char            PixelFormat;

    // If verbose, tell user we are opening the file...
    if(m_Verbose)
        clog << "Opening " << m_InputFile << endl;

    // Open the file...
    ifstream InputFileStream(m_InputFile.c_str(), ifstream::in | ifstream::binary);
    
        // Failed...
        if(!InputFileStream.is_open())
            throw std::string("Could not open input for reading...");

    // There isn't any "typical" file magic signature, so check for general...

        // Load the first logical record...
        const LogicalRecord HeaderRecord(InputFileStream);
    
        // First two bytes should be a zero and a one, and should have more records...
        if(HeaderRecord[0] != '\x0' || HeaderRecord[1] != '\x1' || HeaderRecord[71] != 'C')
            throw std::string("Input does not appear to be a 1970s era VICAR format...");

    // Check second record just to double check that this is actually from
    //  the Viking Lander...

        // Load a record...
        Record << InputFileStream;
        
        // Check...
        if(Record.GetString().compare(0, strlen("VIKING LANDER "), "VIKING LANDER ") != 0)
            throw std::string("Input does not appear to be from a Viking Lander...");

    // Extract image metadata from header logical record, seeking 
    //  passed two byte marker...
        
        // Initialize tokenizer...
        stringstream Tokenizer(HeaderRecord.GetString(2));
        
        // Extract...
        Tokenizer >> m_Bands;
        Tokenizer >> DummyCharacter;
        Tokenizer >> m_Height;
        Tokenizer >> m_Width;
        Tokenizer >> PixelFormat;
        Tokenizer >> m_BytesPerColour;

        // Calculate the physical record size which is either 5 logical 
        //  records or the image width, whichever is greater...
        m_PhysicalRecordSize = max(m_Width, 5 * LOGICAL_RECORD_SIZE);

    // Perform sanity check on basic metadata...
    
        // Check bands...
        if(m_Bands != 1 && m_Bands != 3)
            throw std::string("Unsupported number of image bands...");

        // Check height...
        if(m_Height <= 0)
            throw std::string("Expected positive image height...");
            
        // Check width...
        if(m_Width <= 0)
            throw std::string("Expected positive image width...");

        // Check pixel format...
        if(PixelFormat != 'I')
            throw std::string("Unsupported pixel format...");

        // Check bytes per colour...
        if(m_BytesPerColour != 1 && m_BytesPerColour != 3)
            throw std::string("Unsupported bytes per colour...");

    // If verbosity is set, display basic metadata...
    if(m_Verbose)
        clog << "  Bands:\t\t"              << m_Bands << endl
             << "  Height:\t\t"             << m_Height << endl
             << "  Width:\t\t"              << m_Width << endl
             << "  Format:\t\t"             << "integral" << endl
             << "  Bytes Per Colour:\t"     << m_BytesPerColour << endl
             << "  Physical record size:\t" << m_PhysicalRecordSize << endl;

    // Loaded ok...
    m_Ok = true;
}

// Write the image out as a PNG, or throw an error...
void VicarColourImage::Write(const std::string &OutputFile) const
{
    // Check if file was loaded ok...
    if(!IsOk())
        cerr << "Input was not loaded...";

    // Allocate PNG object of the right size...

        /*
            TODO: Figure out where the raw data starts and then pass 
                  through PNG compressor.
        */
}


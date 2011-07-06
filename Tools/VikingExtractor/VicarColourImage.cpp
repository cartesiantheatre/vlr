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
    #include <iomanip>
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
      m_PhysicalRecordSize(0),
      m_PhysicalRecordPadding(0),
      m_Ok(false),
      m_SaveLabels(false),
      m_Verbose(Verbose)
{
    // Read the VICAR image header...
    ReadHeader();
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
        clog << "opening " << m_InputFile << endl;

    // Open the file...
    ifstream InputFileStream(m_InputFile.c_str(), ifstream::in | ifstream::binary);
    
        // Failed...
        if(!InputFileStream.is_open())
            throw std::string("could not open input for reading");

    // There isn't any "typical" file magic signature, so check for general...

        // Load the first logical record...
        const LogicalRecord HeaderRecord(InputFileStream);
    
        // Check for first end of logical record marker......
        if(HeaderRecord[71] != 'C')
            throw std::string("input does not appear to be a 1970s era VICAR format");

    // Check second record just to double check that this is actually from
    //  the Viking Lander...

        // Load a record...
        Record << InputFileStream;
        
        // Check...
        if(Record.GetString().compare(0, strlen("VIKING LANDER "), "VIKING LANDER ") != 0)
            throw std::string("input does not appear to be from a Viking Lander");

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

            // Image width is greater than 5 logical records...
            if(m_Width > 5 * LOGICAL_RECORD_SIZE)
            {
                // Use the width...
                m_PhysicalRecordSize = m_Width;
                
                // But anything passed the last logical record is just padding...
                m_PhysicalRecordPadding = m_Width - (5 * LOGICAL_RECORD_SIZE);
            }
            
            else
            {
                // Only five logical records...
                m_PhysicalRecordSize = 5 * LOGICAL_RECORD_SIZE;
                
                // With no following padding...
                m_PhysicalRecordPadding = 0;
            }

    // Perform sanity check on basic metadata...
    
        // Check bands...
        if(m_Bands != 1 && m_Bands != 3)
            throw std::string("unsupported number of image bands");

        // Check height...
        if(m_Height <= 0)
            throw std::string("expected positive image height");
            
        // Check width...
        if(m_Width <= 0)
            throw std::string("expected positive image width");

        // Check pixel format...
        if(PixelFormat != 'I')
            throw std::string("unsupported pixel format");

        // Check bytes per colour...
        if(m_BytesPerColour != 1)
            throw std::string("unsupported colour bit depth");

    // If verbosity is set, display basic metadata...
    if(m_Verbose)
    {
        clog << "  bands:\t\t\t" << m_Bands << endl;
        clog << "  height:\t\t\t" << m_Height << endl;
        clog << "  width:\t\t\t" << m_Width << endl;
        clog << "  format:\t\t\t" << "integral" << endl;
        clog << "  bytes per colour:\t\t" << m_BytesPerColour << endl;
        clog << "  physical record size:\t\t" << m_PhysicalRecordSize << hex << showbase << " (" << m_PhysicalRecordSize<< ")" << endl;
        clog << "  physical record padding:\t"  << dec << m_PhysicalRecordPadding << hex << showbase << " (" << m_PhysicalRecordPadding<< ")" << dec << endl;
    }

    // Loaded ok...
    m_Ok = true;
}

// Write the image out as a PNG, or throw an error...
void VicarColourImage::Write(const std::string &OutputFile, const bool Interlace) const
{
    // Objects...
    LogicalRecord   Record;
    std::string     SavedLabelsBuffer;

    // Check if file was loaded ok...
    if(!IsOk())
        throw std::string("input was not loaded");

    // Open the input file...
    ifstream InputFileStream(m_InputFile.c_str(), ifstream::in | ifstream::binary);
    
        // Failed...
        if(!InputFileStream.is_open())
            throw std::string("could not open input for reading");

    // Seek to start of raw image data's physical record boundary...
    while(InputFileStream.good())
    {
        // Local offset into the current physical record...
        streampos LocalPhysicalRecordOffset = 0;
        
        // True if the raw image data was found...
        bool RawImageDataFound = false;

        // Check each logical record of this physical record's five...
        for(size_t Index = 0; Index < 5; ++Index)
        {
            // Extract a logical record...
            Record << InputFileStream;
            
            // Is it valid?
            if(!Record.IsValidLabel())
                throw std::string("invalid logical record label");

            // Add to saved labels buffer...
            SavedLabelsBuffer += Record.GetString() + '\n';

            // Update local offset into the current physical record...
            LocalPhysicalRecordOffset += LOGICAL_RECORD_SIZE;

            // This is the last logical record of the record labels...
            if(Record.IsLastLabel())
            {
                // Calculate the offset necessary to seek past any 
                //  remaining logical records in this physical record, 
                //  along with any padding to the next physical record 
                //  boundary...
                const streamoff RawImageDataRelativeOffset = 
                    ((LOGICAL_RECORD_SIZE * 5) - LocalPhysicalRecordOffset) + 
                    m_PhysicalRecordPadding;

                // Seek to the raw image data...
                InputFileStream.seekg(
                    RawImageDataRelativeOffset, ios_base::cur);

                // Done...
                RawImageDataFound = true;
                break;
            }
        }
        
        // Raw image data was found, stop looking...
        if(RawImageDataFound)
            break;      

        // Seek passed any padding that may have followed the logical 
        //  record set to the next physical record boundary...
        InputFileStream.seekg(m_PhysicalRecordPadding, ios_base::cur);
    }

    // Got to the end of the file and did not find the last label record...
    if(!InputFileStream.good())
        throw std::string("unable to locate last logical record label");

    // Show the image offset if verbose mode enabled...
    if(m_Verbose)
        clog << "  raw image offset:\t\t" << InputFileStream.tellg() << endl;

    // Write the saved label out, if user selected...
    if(m_SaveLabels)
    {
        // Create label file name...
        const std::string LabelFileName = m_InputFile + string(".txt");

        // Open...
        ofstream SavedLabelsStream(LabelFileName.c_str());
        
            // Failed...
            if(!SavedLabelsStream.good())
                throw std::string("unable to save record label");

        // Write...
        SavedLabelsStream << SavedLabelsBuffer;
        
        // Done...
        SavedLabelsStream.close();
    }
    
    // Single channel image...
    if(m_Bands == 1)
    {
        // Allocate...
        png::image<png::gray_pixel> PngImage(m_Width, m_Height);

        // Toggle interlacing, if user selected...
        if(Interlace)
            PngImage.set_interlace_type(png::interlace_adam7);
        else
            PngImage.set_interlace_type(png::interlace_none);

        // Pass raw image data through encoder, row by row...
        for(size_t Y = 0; Y < PngImage.get_height(); ++Y)
        {
            // Pass raw image data through encoder, column by column...
            for(size_t X = 0; X < PngImage.get_width(); ++X)
            {
                // Read a pixel / byte...
                char Byte = '\x0';
                InputFileStream.read(&Byte, 1);
                
                // Encode...
                PngImage.set_pixel(X, Y, Byte);
            }
        }

        // Write out the file and alert the user...
        PngImage.write(OutputFile);
        clog << "writing " << OutputFile << endl;
    }
    
    // Unsupported colour format...
    else
        throw std::string("unsupported number of image bands");
}


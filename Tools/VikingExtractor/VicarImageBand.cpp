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
    #include "VicarImageBand.h"
    
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
VicarImageBand::VicarImageBand(
    const std::string &InputFile, const bool Verbose)
    : m_InputFile(InputFile),
      m_Height(0),
      m_Width(0),
      m_BytesPerColour(0),
      m_PhysicalRecordSize(0),
      m_PhysicalRecordPadding(0),
      m_RawImageOffset(0),
      m_BandType(Invalid),
      m_Ok(false),
      m_SaveLabels(false),
      m_Verbose(Verbose)
{
    // Read the VICAR image header...
    ReadHeader();
}

// Extract the image out as a PNG, or throw an error...
void VicarImageBand::Extract(const std::string &OutputFile, const bool Interlace) const
{
    // Objects...
    LogicalRecord   Record;

    // Check if file was loaded ok...
    if(!IsOk())
        throw std::string("input was not loaded");

    // Open the input file...
    ifstream InputFileStream(m_InputFile.c_str(), ifstream::in | ifstream::binary);
    
        // Failed...
        if(!InputFileStream.is_open())
            throw std::string("could not open input for reading");

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
        SavedLabelsStream << m_SavedLabelsBuffer;
        
        // Done...
        SavedLabelsStream.close();
    }
    
    // Seek to raw image offset and make sure it was successful...
    if(!InputFileStream.seekg(m_RawImageOffset, ios_base::beg).good())
        throw std::string("file ended prematurely before raw image");
    
    // Write out the image...

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
                // Read a pixel / byte and check for error...
                char Byte = '\x0';
                if(!InputFileStream.read(&Byte, 1).good())
                    throw std::string("raw image data ended prematurely");
                
                // Encode...
                PngImage.set_pixel(X, Y, Byte);
            }
        }

        // Write out the file and alert the user...
        PngImage.write(OutputFile);
        clog << "writing " << OutputFile << endl;
}

// Parse basic metadata, or throw an error...
void VicarImageBand::ParseBasicMetadata(const LogicalRecord &Record)
{
    // Variables...
    int     Bands           = 0;
    char    DummyCharacter  = 0;
    char    PixelFormat     = 0;

    // Initialize a tokenizer, seeking passed two byte binary marker...
    stringstream BasicMetadataTokenizer(Record.GetString(false, 2));
    
    // Extract...
    BasicMetadataTokenizer >> Bands;
    BasicMetadataTokenizer >> DummyCharacter;
    BasicMetadataTokenizer >> m_Height;
    BasicMetadataTokenizer >> m_Width;
    BasicMetadataTokenizer >> PixelFormat;
    BasicMetadataTokenizer >> m_BytesPerColour;

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
        if(Bands != 1)
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
    Verbose() << "  bands:\t\t\t" << Bands << endl;
    Verbose() << "  height:\t\t\t" << m_Height << endl;
    Verbose() << "  width:\t\t\t" << m_Width << endl;
    Verbose() << "  size:\t\t\t\t" << m_Width * m_Height * m_BytesPerColour << " bytes" << endl;
    Verbose() << "  format:\t\t\t" << "integral" << endl;
    Verbose() << "  bytes per colour:\t\t" << m_BytesPerColour << endl;
    Verbose() << "  physical record size:\t\t" << m_PhysicalRecordSize << hex << showbase << " (" << m_PhysicalRecordSize<< ")" << endl;
    Verbose() << "  physical record padding:\t"  << dec << m_PhysicalRecordPadding << hex << showbase << " (" << m_PhysicalRecordPadding<< ")" << dec << endl;
}

// Parse extended metadata, if any, or throw an error...
void VicarImageBand::ParseExtendedMetadata(const LogicalRecord &Record)
{
    // Variables...
    std::string Token;
    
    // Is it valid?
    if(!Record.IsValidLabel())
        throw std::string("invalid logical record label");

    // Initialize tokenizer...
    stringstream Tokenizer(Record);
    
    // Get first token...
    Tokenizer >> Token;

    // Found band type...
    if(Token == "DIODE")
    {
        // Extract band...
        Tokenizer >> Token;
        
        // Narrow band for colours...
        if(Token == "RED")      { m_BandType = Red; Verbose() << "  photosensor diode:\tred" << endl; }
        else if(Token == "GRN") { m_BandType = Green; Verbose() << "  photosensor diode:\tgreen" << endl; }
        else if(Token == "BLU") { m_BandType = Blue; Verbose() << "  photosensor diode:\tblue" << endl; }

        // Narrow band for infrared...
        else if(Token == "IR1") { m_BandType = Infrared1; Verbose() << "  photosensor diode:\tinfrared 1" << endl; }
        else if(Token == "IR2") { m_BandType = Infrared2; Verbose() << "  photosensor diode:\tinfrared 2" << endl; }
        else if(Token == "IR3") { m_BandType = Infrared3; Verbose() << "  photosensor diode:\tinfrared 3" << endl; }

        // Narrow band for the Sun...
        else if(Token == "SUN") { m_BandType = Sun; Verbose() << "  photosensor diode:\tsun" << endl; }

        // Unknown...
        else { m_BandType = Invalid; Verbose() << "  photosensor diode:\t\tnon-imaging (" << Token << ")" << endl; }
    }

    // Found azimuth and elevation...
    else if(Token == "AZIMUTH")
    {
        // Extract without surrounding whitespace...
        m_AzimuthElevation = Record.GetString(true);
        string FriendlyAzimuthElevation = m_AzimuthElevation;

        // Make a friendly version for printing out to console in verbose mode...
        size_t ElevationIndex = FriendlyAzimuthElevation.find("ELEVATION");
        if(ElevationIndex != string::npos)
            FriendlyAzimuthElevation.insert(ElevationIndex, "\n\t\t\t\t");

        // Alert user if verbose mode enabled...
        Verbose() << "  PSA directional vector:\t" << FriendlyAzimuthElevation << endl;
    }
}

// Read VICAR header and calling all parse methods, or throw an error...
void VicarImageBand::ReadHeader()
{
    // Objects and variables...
    LogicalRecord   Record;

    // If verbose, tell user we are opening the file...
    Verbose() << "opening " << m_InputFile << endl;

    // Open the file...
    ifstream InputFileStream(m_InputFile.c_str(), ifstream::in | ifstream::binary);
    
        // Failed...
        if(!InputFileStream.is_open())
            throw std::string("could not open input for reading");

    // There isn't any "typical" file magic signature, so check for general...

        // Load and save the first logical record...
        const LogicalRecord HeaderRecord(InputFileStream);
    
        // Check for first end of logical record marker......
        if(HeaderRecord[71] != 'C')
            throw std::string("input does not appear to be a 1970s era VICAR format");

    // Check second record just to double check that this is actually 
    //  from the Viking Lander EDR...

        // Load record and save...
        Record << InputFileStream;
        
        // Check...
        if(Record.GetString().compare(0, strlen("VIKING LANDER "), "VIKING LANDER ") != 0)
            throw std::string("input does not appear to be from a Viking Lander");

    // Extract basic image metadata from the very first label record...
    ParseBasicMetadata(HeaderRecord);

    // Now rewind to start of file...
    InputFileStream.seekg(0, ios_base::beg);

    // Clear saved labels buffer, in case it already had data in it...
    m_SavedLabelsBuffer.clear();

    // Go through all physical / logical records and make note of the 
    //  raw image data's physical record boundary start...
    while(InputFileStream.good())
    {
        // Local offset into the current physical record...
        streampos LocalPhysicalRecordOffset = 0;
        
        // True if the raw image data was found...
        bool RawImageDataFound = false;

        // Check each logical record of this physical record's five.
        for(size_t LocalLogicalRecordIndex = 0; 
            LocalLogicalRecordIndex < 5; 
          ++LocalLogicalRecordIndex)
        {
            // Extract a logical record...
            Record << InputFileStream;
            
            // Is it valid?
            if(!Record.IsValidLabel())
                throw std::string("invalid logical record label");

            // Parse the extended metadata, if any...
            ParseExtendedMetadata(Record);

            // Add to saved labels buffer...
            m_SavedLabelsBuffer += Record.GetString() + '\n';

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

    // Store raw image offset...
    m_RawImageOffset = InputFileStream.tellg();

    // Show user, if requested...
    Verbose() << "  raw image offset:\t\t" << m_RawImageOffset << hex << showbase << " (" << m_RawImageOffset<< ")" << dec << endl;

    // Loaded ok...
    m_Ok = true;
}

// Get the output stream to be verbose, if enabled...
std::ostream &VicarImageBand::Verbose() const
{
    // Not enabled. Return the null stream...
    if(!m_Verbose)
        return m_DummyOutputStream;

    // Otherwise use the usual standard logging stream...
    else
        return clog;
}


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
    const string &InputFile, const bool Verbose)
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
void VicarImageBand::Extract(const string &OutputFile, const bool Interlace) const
{
    // Objects...
    LogicalRecord   Record;

    // Check if file was loaded ok...
    if(!IsOk())
        throw string("input was not loaded");

    // Open the input file...
    ifstream InputFileStream(m_InputFile.c_str(), ifstream::in | ifstream::binary);
    
        // Failed...
        if(!InputFileStream.is_open())
            throw string("could not open input for reading");

    // Write the saved label out, if user selected...
    if(m_SaveLabels)
    {
        // Create label file name...
        const string LabelFileName = m_InputFile + string(".txt");

        // Open...
        ofstream SavedLabelsStream(LabelFileName.c_str());
        
            // Failed...
            if(!SavedLabelsStream.good())
                throw string("unable to save record label");

        // Write...
        SavedLabelsStream << m_SavedLabelsBuffer;
        
        // Done...
        SavedLabelsStream.close();
    }
    
    // Seek to raw image offset and make sure it was successful...
    if(!InputFileStream.seekg(m_RawImageOffset, ios_base::beg).good())
        throw string("file ended prematurely before raw image");
    
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
                    throw string("raw image data ended prematurely");
                
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
    size_t  TokenCount      = 0;
    string  Token;
    char    Buffer[1024]    = {0};
    int     Bands           = 0;
    char    DummyCharacter  = 0;
    char    PixelFormat     = 0;

    // Count how many tokens are there, seeking passed two byte binary 
    //  marker. This is necessary to know since different label formats
    //  can be distinguished by the number of whitespace separated 
    //  tokens...
    stringstream TokenCounter(Record.GetString(true, 2));
    while(TokenCounter.good())
    {
        // Extract and count...
        TokenCounter >> Token;
      ++TokenCount;
    }

    // Initialize a tokenizer, seeking passed two byte binary marker...
    stringstream BasicMetadataTokenizer(Record.GetString(true, 2));

    // Most common format should have five tokens...
    if(TokenCount != 5)
        throw string("unrecognized VICAR header label format");

    // Extract number of image bands...
    BasicMetadataTokenizer >> Token;
    Bands = atoi(Token.c_str());

    // Way more than is reasonable, assume alternate header representation...
    if(Bands > 9)
    {
        // Bands is really just one and was actually the image height...
        Bands = 1;
        m_Height = Bands;
        
        // Next token is the width and height coallesced. Skip...
        BasicMetadataTokenizer >> Token;
        
        // Next token should be the width...
        BasicMetadataTokenizer >> m_Width;
    }
    
    // Otherwise use more common representation...
    else
    {
        // We don't know what this byte is for...
        BasicMetadataTokenizer >> DummyCharacter;

        // Next token may be the height and width coallesced...
        BasicMetadataTokenizer >> Token;
        
        // It's too long to be the height, so split. The reasoning being
        //  that it's doubtful the PSA captured an image greater than 
        //  9999 pixels in width or height...
        if(Token.length() > 4)
        {
            // Calculate length of first half... (height)
            const size_t HeightLength = Token.length() / 2;

            // Let the first half be the height...
            Token.copy(Buffer, HeightLength);
            Buffer[HeightLength] = '\x0';
            m_Height = atoi(Buffer);
            
            // Let the second half be the width...
            Token.copy(Buffer, Token.length() - HeightLength, HeightLength);
            Buffer[Token.length() - HeightLength] = '\x0';
            m_Width = atoi(Buffer);
        }
        
        // It's short enough to be the actual height...
        else
        {
            // Extract image height...
            m_Height = atoi(Token.c_str());

            // Extract image width...
            BasicMetadataTokenizer >> m_Width;
        }

        // Calculate the physical record size which is either 5 logical 
        //  records or the image width, whichever is greater...
        if(Token.length() <= 4)
        {
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
        }
    }
    
    // Extract pixel format...
    BasicMetadataTokenizer >> PixelFormat;
    
    // Extract bytes per colour...
    BasicMetadataTokenizer >> m_BytesPerColour;

    // Perform sanity check on basic metadata...

    // If verbosity is set, display basic metadata...
    Verbose() << "  bands:\t\t\t" << Bands << endl;
    Verbose() << "  height:\t\t\t" << m_Height << endl;
    Verbose() << "  width:\t\t\t" << m_Width << endl;
    Verbose() << "  size:\t\t\t\t" << m_Width * m_Height * m_BytesPerColour << " bytes" << endl;
    Verbose() << "  format:\t\t\t" << "integral" << endl;
    Verbose() << "  bytes per colour:\t\t" << m_BytesPerColour << endl;
    Verbose() << "  physical record size:\t\t" << m_PhysicalRecordSize << hex << showbase << " (" << m_PhysicalRecordSize<< ")" << endl;
    Verbose() << "  physical record padding:\t"  << dec << m_PhysicalRecordPadding << hex << showbase << " (" << m_PhysicalRecordPadding<< ")" << dec << endl;

        // Check bands...
        if(Bands != 1)
            throw string("unsupported number of image bands");

        // Check height...
        if(m_Height <= 0)
            throw string("expected positive image height");

        // Check width...
        if(m_Width <= 0)
            throw string("expected positive image width");

        // Check pixel format is integral...
        if(PixelFormat != 'I' && PixelFormat != 'L')
            throw string("unsupported pixel format");

        // Check bytes per colour...
        if(m_BytesPerColour != 1)
            throw string("unsupported colour bit depth");
}

// Parse extended metadata, if any, or throw an error...
void VicarImageBand::ParseExtendedMetadata(const LogicalRecord &Record)
{
    // Variables...
    string Token;
    
    // Is it valid?
    if(!Record.IsValidLabel())
        throw string("invalid logical record label while parsing extended metadata");

    // Initialize tokenizer...
    stringstream Tokenizer(Record);
    
    // Get first token...
    Tokenizer >> Token;

    // Found band, check photosensor type...
    if(Token == "DIODE")
    {
        // Extract band...
        Tokenizer >> Token;
        
        /*
            Note: Sometimes the narrow band photosensor array diodes
            in the VICAR label were given inconsistent names when
            taken as part of a triplet (e.g. RGB colour bands).
            Sometimes you might see RED, sometimes RED/T (red channel
            as part of triplet), and sometimes RED/S. We're not sure
            what the /S might have stood for, but probably not
            "single" since VICAR images vl_1553.00{7-9}, for instance,
            have PSA diodes of colour/S form but appear to be separate
            channels of the same image. We will assume colour ==
            colour/S == colour/T for now since they were probably
            added later in an inconsistent and hectic early work
            environment.
        */
        
        // Narrow band for colours...
            
            // Red triplet...
            if(Token == "RED" || Token == "RED/S" || Token == "RED/T")
                { m_BandType = Red; Verbose() << "  photosensor diode:\tred" << endl; }
            
            // Green triplet...
            else if(Token == "GRN" || Token == "GRN/S" || Token == "GRN/T")
                { m_BandType = Green; Verbose() << "  photosensor diode:\tgreen" << endl; }

            // Blue triplet...
            else if(Token == "BLU" || Token == "BLU/S" || Token == "BLU/T")
                { m_BandType = Blue; Verbose() << "  photosensor diode:\tblue" << endl; }

        // Narrow band for infrared...
        
            // Infrared one...
            else if(Token == "IR1" || Token == "IR1/S" || Token == "IR1/T")
                { m_BandType = Infrared1; Verbose() << "  photosensor diode:\tinfrared 1" << endl; }
                
            // Infrared one...
            else if(Token == "IR2" || Token == "IR2/S" || Token == "IR2/T")
                { m_BandType = Infrared2; Verbose() << "  photosensor diode:\tinfrared 2" << endl; }
            
            // Infrared one...
            else if(Token == "IR3" || Token == "IR3/S" || Token == "IR3/T")
                { m_BandType = Infrared3; Verbose() << "  photosensor diode:\tinfrared 3" << endl; }

        // Narrow band for the Sun...
        else if(Token == "SUN")
            { m_BandType = Sun; Verbose() << "  photosensor diode:\tsun" << endl; }

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
            throw string("could not open input for reading");

    // There isn't any "typical" file magic signature, so check for general...

        // Load and save the first logical record...
        const LogicalRecord HeaderRecord(InputFileStream);
    
        // Check for first end of logical record marker......
        if(HeaderRecord[71] != 'C')
            throw string("input does not appear to be a 1970s era VICAR format");

    // Check second record just to double check that this is actually 
    //  from the Viking Lander EDR...

        // Load record and save...
        Record << InputFileStream;
        
        // Check...
        if(Record.GetString().compare(0, strlen("VIKING LANDER "), "VIKING LANDER ") != 0)
            throw string("input does not appear to be from a Viking Lander");

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
//Verbose() << "tellg() " << static_cast<int>(InputFileStream.tellg()) << endl;
            
            // Is it valid?
            if(!Record.IsValidLabel())
            {
                Verbose() << "bad logical record terminator " << LocalLogicalRecordIndex << "/5 at " << static_cast<int>(InputFileStream.tellg()) + 71 << endl;
                throw string("invalid logical record label while reading header at ");
            }

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
//Verbose() << "seeking passed " << m_PhysicalRecordPadding << " physical record padding" << endl;
        InputFileStream.seekg(m_PhysicalRecordPadding, ios_base::cur);
    }

    // Got to the end of the file and did not find the last label record...
    if(!InputFileStream.good())
        throw string("unable to locate last logical record label");

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


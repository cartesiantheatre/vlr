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

// Construct or throw an error...
VicarImageBand::VicarImageBand(
    const string &InputFile, const bool Verbose)
    : m_InputFile(InputFile),
      m_Bands(0),
      m_Height(0),
      m_Width(0),
      m_PixelFormat(0),
      m_BytesPerColour(0),
      m_PhysicalRecordSize(0),
      m_PhysicalRecordPadding(0),
      m_RawImageOffset(0),
      m_DiodeBandType(Unknown),
      m_Ok(false),
      m_SaveLabels(false),
      m_Verbose(Verbose)
{

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

// Get the diode band type as a human friendly string...
string VicarImageBand::GetDiodeBandTypeString() const
{
    switch(m_DiodeBandType)
    {
        // Unknown...
        case Unknown:   return string("unknown");
        
        // Narrow band for colours...
        case Red:       return string("red");
        case Green:     return string("green");
        case Blue:      return string("blue");
        
        // Narrow band for infrared...
        case Infrared1: return string("infrared 1");
        case Infrared2: return string("infrared 2");
        case Infrared3: return string("infrared 3");

        // Narrow band for the Sun...
        case Sun:       return string("sun");
        
        // Invalid... (fall through)
        case Invalid:
        default:        return string("invalid");
    }
}

// Get the file size or throw an error...
int VicarImageBand::GetFileSize() const
{
    // Open the file...
    ifstream InputFileStream(m_InputFile.c_str(), ifstream::in | ifstream::binary);
    
        // Failed...
        if(!InputFileStream.is_open())
            throw string("could not open input for reading");

    // Seek to the end and return the size...
    InputFileStream.seekg(0, ios_base::end);
    return InputFileStream.tellg();
}

// Get the input file name only without path...
string VicarImageBand::GetInputFileNameOnly() const
{
    // There should always be a file name already...
    assert(!m_InputFile.empty());
    
    // Make a copy of the complete input file name and path...
    string FileNameOnly = m_InputFile;
    
    // Strip the path so only file name remains...
    const size_t PathIndex = FileNameOnly.find_last_of("\\/");
    if(PathIndex != string::npos)
        FileNameOnly.erase(0, PathIndex + 1);

    // Done...
    return FileNameOnly;
}

// Check if the image band data is most likely present by file size...
bool VicarImageBand::IsBandDataPresent() const
{
    // Basic metadata should have been read already...
    assert(m_Ok);

    // Probably, if the file is at least large enough to contain the 
    //  band data and one physical record...
    if(GetFileSize() >= ((m_Width * m_Height * m_BytesPerColour) + (5 * LOGICAL_RECORD_SIZE)))
        return true;
    
    // Otherwise probably not...
    else
        return false;
}

// Check if the file is loadable. Note that this does a shallow
//  file integrity check and so it may succeed even though 
//  LoadHeader() fails later...
bool VicarImageBand::IsHeaderIntact() const
{
    // Check to make sure it contains at least three logical records...
    if(GetFileSize() < (3 * LOGICAL_RECORD_SIZE))
        return false;

    // Open the file...
    ifstream InputFileStream(m_InputFile.c_str(), ifstream::in | ifstream::binary);
    
        // Failed...
        if(!InputFileStream.is_open())
            throw string("could not open input for reading");

    // There isn't any "typical" file magic signature, so check for general...

        // Load the first logical record...
        const LogicalRecord HeaderRecord(InputFileStream);
    
        // Check if valid first end of logical record marker......
        if(HeaderRecord.IsValidLabel())
            return true;

        // Nope...
        else
            return false;
}

// Check second record just to double check that this is actually 
//  from the Viking Lander EDR...
bool VicarImageBand::IsFromVikingLander() const
{
    // A logical record...
    LogicalRecord   Record;

    // Open the file...
    ifstream InputFileStream(m_InputFile.c_str(), ifstream::in | ifstream::binary);
    
        // Failed...
        if(!InputFileStream.is_open())
            throw string("could not open input for reading");

    // Load the second record...
    Record << InputFileStream;
    Record << InputFileStream;
    
    // Check...
    if(Record.GetString().compare(0, strlen("VIKING LANDER "), "VIKING LANDER ") != 0)
        return false;
    else
        return true;
}

// Read VICAR header, calling all parse methods, or throw an error...
void VicarImageBand::LoadHeader()
{
    // Objects and variables...
    LogicalRecord   Record;

    // Check if the header is intact...
    if(!IsHeaderIntact())
        throw string("header is not intact");

    // Open the file...
    ifstream InputFileStream(m_InputFile.c_str(), ifstream::in | ifstream::binary);
    
        // Failed...
        if(!InputFileStream.is_open())
            throw string("could not open input for reading");

    // Extract the basic image metadata...
    ParseBasicMetadata(InputFileStream);

    // Now rewind again to start of file...
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
                throw string("invalid logical record label");
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

        // Deal with padding...
        
            // Remember the current read pointer offset in case we decide to rewind...
            const streampos CurrentPosition = InputFileStream.tellg();
            
            // Check to see if next physical record boundary was tangential...
            Record << InputFileStream;
            if(Record.IsValidLabel())
            {
                // It was, so rewind and carry on since there is no 
                //  physical record padding...
//                Verbose() << "tangential physical record boundary detected, ignoring padding" << endl;
                InputFileStream.seekg(CurrentPosition);
            }
            
            // Otherwise, seek passed any padding that may have followed
            //  the logical record set to the next physical record boundary...
            else
            {
                // Alert and seek...
//                Verbose() << "seeking passed " << m_PhysicalRecordPadding << " physical record padding" << endl;
                InputFileStream.seekg(CurrentPosition);
                InputFileStream.seekg(m_PhysicalRecordPadding, ios_base::cur);
            }
    }

    // Got to the end of the file and did not find the last label record...
    if(!InputFileStream.good())
        throw string("unable to locate last logical record label");

    // Store raw image offset...
    m_RawImageOffset = InputFileStream.tellg();

    // Show user, if requested...
    Verbose() << "  raw image offset:\t\t\t" << m_RawImageOffset << hex << showbase << " (" << m_RawImageOffset<< ")" << dec << endl;

    // Loaded ok...
    m_Ok = true;
}

// Read VICAR header's basic metadata only. Does not throw an error, 
//  but reads the most of what it can...
void VicarImageBand::LoadHeaderShallow()
{
    // Objects and variables...
    LogicalRecord   Record;

    // Open the file...
    ifstream InputFileStream(m_InputFile.c_str(), ifstream::in | ifstream::binary);
    
        // Failed...
        if(!InputFileStream.is_open())
            throw string("could not open input for reading");

    // Extract the basic image metadata...
    ParseBasicMetadata(InputFileStream);
}

// Parse basic metadata, or throw an error. Basic metadata includes 
//  bands, dimensions, pixel format, bytes per colour, photosensor 
//  diode band type, etc...
void VicarImageBand::ParseBasicMetadata(ifstream &InputFileStream)
{
    // Variables...
    LogicalRecord   Record;
    string          Token;
    size_t          TokenIndex          = 0;
    size_t          TokenLength[32];

    // Stream should have already been validated...
    assert(InputFileStream.good());

    // There isn't any "typical" file magic signature, so check for general...

        // Load and save the first logical record...
        const LogicalRecord HeaderRecord(InputFileStream);
    
        // Check for first end of logical record marker......
        if(!HeaderRecord.IsValidLabel())
            throw string("input not a valid 1970s era VICAR format");

    // Check that it is from a Viking Lander...
    if(!IsFromVikingLander())
        throw string("input does not appear to be from a Viking Lander");

    // Clear token length buffer...
    memset(TokenLength, 0, sizeof(TokenLength));

    // Count how many tokens are there, seeking passed two byte binary 
    //  marker. This is necessary to know since different label formats
    //  can be distinguished by the number of whitespace separated 
    //  tokens...
    stringstream TokenCounter(HeaderRecord.GetString(true, 2));
    for(TokenIndex = 0; 
        TokenCounter.good() && TokenIndex < sizeof(TokenLength) / sizeof(TokenLength[0]); 
      ++TokenIndex)
    {
        // Extract token...
        TokenCounter >> Token;
        
        // Remember its length...
        TokenLength[TokenIndex] = Token.length();
    }
    
    // Calculate total number of tokens found...
    const size_t TotalTokens = TokenIndex;

    // Select method to parse basic header metadata based on heuristics...
    
        /* 
            (Most common format)
            Header: "1   11151 586 I 1"
            Legend:  A   BC    D   E F  
                
            A: Number of image bands (1)
            B: Unknown (1)
            C: Height (1151)
            D: Width (586)
            E: Pixel format (integral)
            F: Bytes per pixel (1)
        */
        if(TotalTokens == 5 && 
           TokenLength[0] == 1 &&
           TokenLength[1] <= 5 &&
           TokenLength[2] <= 4 &&
           TokenLength[3] == 1 &&
           TokenLength[4] == 1)
            ParseBasicMetadataImplementation_Format1(HeaderRecord);

        /* 
            Example: vl_0387.021
            Header:  "1   1 5122001 I 1"
            Legend:   A   B C  D    E F  
                
            A: Number of image bands (1)
            B: Unknown (1)
            C: Height (512)
            D: Width (2001)
            E: Pixel format (integral)
            F: Bytes per pixel (1)
        */
        else if(TotalTokens == 5 && 
           TokenLength[0] == 1 &&
           TokenLength[1] == 1 &&
           TokenLength[2] <= 8 &&
           TokenLength[3] == 1 &&
           TokenLength[4] == 1)
            ParseBasicMetadataImplementation_Format2(HeaderRecord);

        /* 
            Example: vl_1474.001
            Header:  "715    1955 7151955 I 1"
            Legend:   B      C    B  C    D E
                
            A: Number of image bands (implicitly 1)
            B: Height (715)
            C: Width (1955)
            D: Pixel format (integral)
            E: Bytes per pixel (1)
        */
        else if(TotalTokens == 5 && 
           TokenLength[0] >  1 &&
           TokenLength[0] <= 4 &&
           TokenLength[1] >  1 &&
           TokenLength[1] <= 4 &&
           TokenLength[2] >= 4 &&
           TokenLength[3] == 1 &&
           TokenLength[4] == 1)
            ParseBasicMetadataImplementation_Format5(HeaderRecord);

        /* 
            Example: vl_1529.008
            Header:  "1151     5861151 586 L 1"
            Legend:   B        C  B    C   D E  

            A: Number of image bands (implicitly 1)
            B: Height (1151)
            C: Width (586)
            D: Pixel format (integral)
            E: Bytes per pixel (1)
        */
        else if(TotalTokens == 5 && 
           TokenLength[0] <= 4 &&
           TokenLength[1] <= 8 &&
           TokenLength[2] <= 4 &&
           TokenLength[3] == 1 &&
           TokenLength[4] == 1)
            ParseBasicMetadataImplementation_Format3(HeaderRecord);

        /* 
            Example: vl_0514.004
            Header:  "1   116402250 L 1"
            Legend:   A   BC   D    E F  
                
            A: Number of image bands (1)
            B: Unknown (1)
            C: Height (1640)
            D: Width (2250)
            E: Pixel format (integral)
            F: Bytes per pixel (1)
        */
        else if(TotalTokens == 4 && 
           TokenLength[0] == 1 &&
           TokenLength[1] <= 9 &&
           TokenLength[2] == 1 &&
           TokenLength[3] == 1)
            ParseBasicMetadataImplementation_Format2(HeaderRecord);

        /* 
            Example: vl_2044.001
            Header:  "2000    410020004100 L 1"
            Legend:   B       C   B   C    D E
                
            A: Number of image bands (implicitly 1)
            B: Height (2000)
            C: Width (4100)
            D: Pixel format (integral)
            E: Bytes per pixel (1)
        */
        else if(TotalTokens == 4 && 
           TokenLength[0] >= 2 &&
           TokenLength[0] <= 4 &&
           TokenLength[1] >= 6 &&
           TokenLength[2] == 1 &&
           TokenLength[3] == 1)
            ParseBasicMetadataImplementation_Format6(HeaderRecord);

        /* 
            Example: vl_1105.006
            Header:  "1   1 512  42 I 1"
            Legend:   A   B C    D  E F  
                
            A: Number of image bands (1)
            B: Unknown (1)
            C: Height (512)
            D: Width (42)
            E: Pixel format (integral)
            F: Bytes per pixel (1)
        */
        else if(TotalTokens == 6 && 
           TokenLength[0] == 1 &&
           TokenLength[1] == 1 &&
           TokenLength[2] <= 4 &&
           TokenLength[3] <= 4 &&
           TokenLength[4] == 1 &&
           TokenLength[5] == 1)
            ParseBasicMetadataImplementation_Format1(HeaderRecord);

        /* 
            Example: vl_2003.002
            Header:  "512     253 512 253 I 1"
            Legend:   B       C   B   C   D E
                
            A: Number of image bands (implicitly 1)
            B: Height (512)
            C: Width (253)
            D: Pixel format (integral)
            E: Bytes per pixel (1)
        */
        else if(TotalTokens == 6 && 
           TokenLength[0] >= 2 &&
           TokenLength[0] <= 4 &&
           TokenLength[1] >= 2 &&
           TokenLength[1] <= 4 &&
           TokenLength[2] >= 2 &&
           TokenLength[2] <= 4 &&
           TokenLength[3] >= 2 &&
           TokenLength[3] <= 4 &&
           TokenLength[4] == 1 &&
           TokenLength[5] == 1)
            ParseBasicMetadataImplementation_Format4(HeaderRecord);

        // Unknown format...
        else
            throw string("heuristics couldn't determine how to parse basic metadata");

    // Third record usually contains the photosensor diode band type...

        // Load it...
        Record << InputFileStream;  /* Skips second one which was checked in IsFromVikingLander() */
        Record << InputFileStream;
        
        // Is it valid?
        if(!Record.IsValidLabel())
            throw string("invalid logical record label while looking for diode band type");

        // Initialize tokenizer...
        stringstream DiodeTokenizer(Record);
        
        // Get first token...
        DiodeTokenizer >> Token;

        // Not the logical record beginning with the diode photosensor type...
        if(Token != "DIODE")
            clog << GetInputFileNameOnly() << "\033[1;31m" << ": warning: photosensor diode band type missing" << "\033[0m" << endl;
        
        // Otherwise set it...
        else
        {
            // Extract and set photosensor band type...
            DiodeTokenizer >> Token;
            SetDiodeBandTypeFromVicarToken(Token);
        }

    // Perform sanity check on basic metadata...

        // Check bands...
        if(m_Bands != 1)
            throw string("unsupported number of image bands");

        // Check height...
        if(m_Height <= 0)
            throw string("expected positive image height");

        // Check width...
        if(m_Width <= 0)
            throw string("expected positive image width");

        // Check pixel format is integral...
        if(m_PixelFormat != 'I' && /* Definitely integral */
           m_PixelFormat != 'L')   /* Guessing integral */
            throw string("unsupported pixel format");

        // Check bytes per colour...
        if(m_BytesPerColour != 1)
            throw string("unsupported colour bit depth");

    // If verbosity is set, display basic metadata...
    Verbose() << "  bands:\t\t\t\t" << m_Bands << endl;
    Verbose() << "  height:\t\t\t\t" << m_Height << endl;
    Verbose() << "  width:\t\t\t\t" << m_Width << endl;
    Verbose() << "  raw band data size:\t\t\t" << m_Width * m_Height * m_BytesPerColour << " bytes" << endl;
    Verbose() << "  file size:\t\t\t\t" << GetFileSize() << " bytes" << endl;
    Verbose() << "  format:\t\t\t\t" << "integral" << endl;
    Verbose() << "  bytes per colour:\t\t\t" << m_BytesPerColour << endl;
    Verbose() << "  photosensor diode band type:\t\t" << GetDiodeBandTypeString() << endl;
    Verbose() << "  physical record size:\t\t\t" << m_PhysicalRecordSize << hex << showbase << " (" << m_PhysicalRecordSize<< ")" << endl;
    Verbose() << "  possible physical record padding:\t"  << dec << m_PhysicalRecordPadding << hex << showbase << " (" << m_PhysicalRecordPadding<< ")" << dec << endl;

    // Basic metadata in theory should be enough to extract the band data...
    m_Ok = true;
}

// Parse the basic metadata using format 1...
void VicarImageBand::ParseBasicMetadataImplementation_Format1(
    const LogicalRecord &HeaderRecord)
{
    /* 
        (Most common format)
        Header: "1   11151 586 I 1"
        Legend:  A   BC    D   E F  
            
        A: Number of image bands (1)
        B: Unknown (1)
        C: Height (1151)
        D: Width (586)
        E: Pixel format (integral)
        F: Bytes per pixel (1)
    */

    // Variables...
    char    DummyCharacter  = 0;

    // Alert user if verbose enabled...
    Verbose() << "heuristics selected format 1 basic metadata parser" << endl;

    // Initialize a tokenizer, seeking passed two byte binary marker...
    stringstream Tokenizer(HeaderRecord.GetString(true, 2));

    // Extract number of image bands...
    Tokenizer >> m_Bands;
    
    // We don't know what this byte is for...
    Tokenizer >> DummyCharacter;

    // Extract image height...
    Tokenizer >> m_Height;

    // Extract image width...
    Tokenizer >> m_Width;

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
        
        // Image width less than five logical records, meaning physical
        //  records are exactly five logical records...
        else
        {
            // Only five logical records...
            m_PhysicalRecordSize = 5 * LOGICAL_RECORD_SIZE;

            // With no following padding...
            m_PhysicalRecordPadding = 0;
        }
    
    // Extract pixel format...
    Tokenizer >> m_PixelFormat;
    
    // Extract bytes per colour...
    Tokenizer >> m_BytesPerColour;
}

// Parse the basic metadata using format 2...
void VicarImageBand::ParseBasicMetadataImplementation_Format2(
    const LogicalRecord &HeaderRecord)
{
    /* 
        Example: vl_0387.021
        Header:  "1   1 5122001 I 1"
        Legend:   A   B C  D    E F  
            
        A: Number of image bands (1)
        B: Unknown (1)
        C: Height (512)
        D: Width (2001)
        E: Pixel format (integral)
        F: Bytes per pixel (1)
    */

    // Variables...
    string  Token;
    char    Buffer[1024]    = {0};
    char    DummyCharacter  = 0;

    // Alert user if verbose enabled...
    Verbose() << "heuristics selected format 2 basic metadata parser" << endl;

    // Initialize a tokenizer, seeking passed two byte binary marker...
    stringstream Tokenizer(HeaderRecord.GetString(true, 2));

    // Extract number of image bands...
    Tokenizer >> m_Bands;

    // We don't know what this byte is for...
    Tokenizer >> DummyCharacter;

    // Next token is the height and width coallesced...
    Tokenizer >> Token;

    /*
        It's too long to be the height, so split. The reasoning being
        that it's doubtful the PSA captured an image greater than 9999
        pixels in width or height.
    */

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
        
        // Image width less than five logical records, meaning physical
        //  records are exactly five logical records...
        else
        {
            // Only five logical records...
            m_PhysicalRecordSize = 5 * LOGICAL_RECORD_SIZE;

            // With no following padding...
            m_PhysicalRecordPadding = 0;
        }
    
    // Extract pixel format...
    Tokenizer >> m_PixelFormat;
    
    // Extract bytes per colour...
    Tokenizer >> m_BytesPerColour;
}

// Parse the basic metadata using format 3...
void VicarImageBand::ParseBasicMetadataImplementation_Format3(
    const LogicalRecord &HeaderRecord)
{
    /* 
        Example: vl_1529.008
        Header:  "1151     5861151 586 L 1"
        Legend:   B        C  B    C   D E  

        A: Number of image bands (implicitly 1)
        B: Height (1151)
        C: Width (586)
        D: Pixel format (integral)
        E: Bytes per pixel (1)
    */

    // Variables...
    string  Token;

    // Alert user if verbose enabled...
    Verbose() << "heuristics selected format 3 basic metadata parser" << endl;

    // Initialize a tokenizer, seeking passed two byte binary marker...
    stringstream Tokenizer(HeaderRecord.GetString(true, 2));

    // Number of image bands implicitly one...
    m_Bands = 1;

    // Extract image height...
    Tokenizer >> m_Height;

    // Next token is the height and width coallesced. Ignore...
    Tokenizer >> Token;
    
    // Extract width...
    Tokenizer >> m_Width;

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
        
        // Image width less than five logical records, meaning physical
        //  records are exactly five logical records...
        else
        {
            // Only five logical records...
            m_PhysicalRecordSize = 5 * LOGICAL_RECORD_SIZE;

            // With no following padding...
            m_PhysicalRecordPadding = 0;
        }
    
    // Extract pixel format...
    Tokenizer >> m_PixelFormat;
    
    // Extract bytes per colour...
    Tokenizer >> m_BytesPerColour;
}

// Parse the basic metadata using format 4...
void VicarImageBand::ParseBasicMetadataImplementation_Format4(
    const LogicalRecord &HeaderRecord)
{
    /* 
        Example: vl_2003.002
        Header:  "512     253 512 253 I 1"
        Legend:   B       C   B   C   D E
            
        A: Number of image bands (implicitly 1)
        B: Height (512)
        C: Width (253)
        D: Pixel format (integral)
        E: Bytes per pixel (1)
    */

    // Variables...
    string  Token;

    // Alert user if verbose enabled...
    Verbose() << "heuristics selected format 4 basic metadata parser" << endl;

    // Initialize a tokenizer, seeking passed two byte binary marker...
    stringstream Tokenizer(HeaderRecord.GetString(true, 2));

    // Number of image bands implicitly one...
    m_Bands = 1;

    // Extract image height...
    Tokenizer >> m_Height;
    
    // Extract width...
    Tokenizer >> m_Width;
    
    // Next two are height and width again, skip...
    Tokenizer >> Token;
    Tokenizer >> Token;

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
        
        // Image width less than five logical records, meaning physical
        //  records are exactly five logical records...
        else
        {
            // Only five logical records...
            m_PhysicalRecordSize = 5 * LOGICAL_RECORD_SIZE;

            // With no following padding...
            m_PhysicalRecordPadding = 0;
        }
    
    // Extract pixel format...
    Tokenizer >> m_PixelFormat;
    
    // Extract bytes per colour...
    Tokenizer >> m_BytesPerColour;
}

// Parse the basic metadata using format 5...
void VicarImageBand::ParseBasicMetadataImplementation_Format5(
    const LogicalRecord &HeaderRecord)
{
    /* 
        Example: vl_1474.001
        Header:  "715    1955 7151955 I 1"
        Legend:   B      C    B  C    D E
            
        A: Number of image bands (implicitly 1)
        B: Height (715)
        C: Width (1955)
        D: Pixel format (integral)
        E: Bytes per pixel (1)
    */

    // Variables...
    string  Token;

    // Alert user if verbose enabled...
    Verbose() << "heuristics selected format 5 basic metadata parser" << endl;

    // Initialize a tokenizer, seeking passed two byte binary marker...
    stringstream Tokenizer(HeaderRecord.GetString(true, 2));

    // Number of image bands implicitly one...
    m_Bands = 1;

    // Extract image height...
    Tokenizer >> m_Height;

    // Extract width...
    Tokenizer >> m_Width;

    // Next token is the height and width coallesced. Ignore...
    Tokenizer >> Token;

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
        
        // Image width less than five logical records, meaning physical
        //  records are exactly five logical records...
        else
        {
            // Only five logical records...
            m_PhysicalRecordSize = 5 * LOGICAL_RECORD_SIZE;

            // With no following padding...
            m_PhysicalRecordPadding = 0;
        }
    
    // Extract pixel format...
    Tokenizer >> m_PixelFormat;
    
    // Extract bytes per colour...
    Tokenizer >> m_BytesPerColour;
}

// Parse the basic metadata using format 6...
void VicarImageBand::ParseBasicMetadataImplementation_Format6(
    const LogicalRecord &HeaderRecord)
{
    /* 
        Example: vl_2044.001
        Header:  "2000    410020004100 L 1"
        Legend:   B       C   B   C    D E
            
        A: Number of image bands (implicitly 1)
        B: Height (2000)
        C: Width (4100)
        D: Pixel format (integral)
        E: Bytes per pixel (1)
    */

    // Variables...
    string  Token;
    char    Buffer[1024] = {0};

    // Alert user if verbose enabled...
    Verbose() << "heuristics selected format 6 basic metadata parser" << endl;

    // Initialize a tokenizer, seeking passed two byte binary marker...
    stringstream Tokenizer(HeaderRecord.GetString(true, 2));

    // Number of image bands implicitly one...
    m_Bands = 1;

    // Extract image height...
    Tokenizer >> Token;
    const size_t HeightLength = Token.length();
    m_Height = atoi(Token.c_str());

    // Next token is the height, width, and height again coallesced...
    Tokenizer >> Token;
    
    // The width we can calculate because it follows immediately after height...
    Token.copy(Buffer, Token.length() - HeightLength, HeightLength);
    Buffer[Token.length() - HeightLength] = '\x0';
    m_Width = atoi(Buffer);

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
        
        // Image width less than five logical records, meaning physical
        //  records are exactly five logical records...
        else
        {
            // Only five logical records...
            m_PhysicalRecordSize = 5 * LOGICAL_RECORD_SIZE;

            // With no following padding...
            m_PhysicalRecordPadding = 0;
        }
    
    // Extract pixel format...
    Tokenizer >> m_PixelFormat;
    
    // Extract bytes per colour...
    Tokenizer >> m_BytesPerColour;
}

// Parse extended metadata, if any, or throw an error. Extended metadata
//  includes the azimuth and elevation...
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

    // Found azimuth and elevation...
    if(Token == "AZIMUTH")
    {
        // Extract without surrounding whitespace...
        m_AzimuthElevation = Record.GetString(true);
        string FriendlyAzimuthElevation = m_AzimuthElevation;

        // Make a friendly version for printing out to console in verbose mode...
        size_t ElevationIndex = FriendlyAzimuthElevation.find("ELEVATION");
        if(ElevationIndex != string::npos)
            FriendlyAzimuthElevation.insert(ElevationIndex, "\n\t\t\t\t\t");

        // Alert user if verbose mode enabled...
        Verbose() << "  PSA directional vector:\t\t" << FriendlyAzimuthElevation << endl;
    }
}

// Set the photosensor diode band type from VICAR style 
//  string, or throw an error... (e.g. "RED/T")
void VicarImageBand::SetDiodeBandTypeFromVicarToken(
    const std::string &DiodeBandType)
{
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
        if(DiodeBandType == "RED" || DiodeBandType == "RED/S" || DiodeBandType == "RED/T")
            m_DiodeBandType = Red;
        
        // Green triplet...
        else if(DiodeBandType == "GRN" || DiodeBandType == "GRN/S" || DiodeBandType == "GRN/T")
            m_DiodeBandType = Green;

        // Blue triplet...
        else if(DiodeBandType == "BLU" || DiodeBandType == "BLU/S" || DiodeBandType == "BLU/T")
            m_DiodeBandType = Blue;

    // Narrow band for infrared...
    
        // Infrared one...
        else if(DiodeBandType == "IR1" || DiodeBandType == "IR1/S" || DiodeBandType == "IR1/T")
            m_DiodeBandType = Infrared1;
            
        // Infrared one...
        else if(DiodeBandType == "IR2" || DiodeBandType == "IR2/S" || DiodeBandType == "IR2/T")
            m_DiodeBandType = Infrared2;
        
        // Infrared one...
        else if(DiodeBandType == "IR3" || DiodeBandType == "IR3/S" || DiodeBandType == "IR3/T")
            m_DiodeBandType = Infrared3;

    // Narrow band for the Sun...
    else if(DiodeBandType == "SUN")
        m_DiodeBandType = Sun;

    // Invalid...
    else 
        m_DiodeBandType = Invalid;
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


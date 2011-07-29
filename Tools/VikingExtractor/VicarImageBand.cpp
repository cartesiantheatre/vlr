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
    
    // Console output...
    #include "Console.h"

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

    // Connecting and operating on useful containers...
    #include <algorithm>
    #include <iterator>

// Helpful macro...
#define SetErrorAndReturn(Message)      { SetErrorMessage((Message)); return; }
#define SetErrorAndReturnFalse(Message) { SetErrorMessage((Message)); return false; }

// Using the standard namespace...
using namespace std;

// Construct...
VicarImageBand::VicarImageBand(
    const string &InputFile)
    : m_InputFile(InputFile),
      m_PhaseOffsetRequired(0),
      m_Bands(0),
      m_Height(0),
      m_Width(0),
      m_PixelFormat(0),
      m_BytesPerColour(0),
      m_PhysicalRecordSize(0),
      m_PhysicalRecordPadding(0),
      m_RawImageOffset(0),
      m_DiodeBandType(Unknown),
      m_Ok(false)
{
    // Initialize the token to diode band type dictionary...

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

        // Narrow band for colour...

            // Red...
            m_TokenToBandTypeMap["RED"]         = Red;
            m_TokenToBandTypeMap["RED/S"]       = Red;
            m_TokenToBandTypeMap["RED/T"]       = Red;
            
            // Green...
            m_TokenToBandTypeMap["GRN"]         = Green;
            m_TokenToBandTypeMap["GREEN"]       = Green;
            m_TokenToBandTypeMap["GRN/S"]       = Green;
            m_TokenToBandTypeMap["GRN/T"]       = Green;
            
            // Blue...
            m_TokenToBandTypeMap["BLU"]         = Blue;
            m_TokenToBandTypeMap["BLUE"]        = Blue;
            m_TokenToBandTypeMap["BLU/S"]       = Blue;
            m_TokenToBandTypeMap["BLU/T"]       = Blue;

        // Narrow band for infrared...
        
            // Infrared one...
            m_TokenToBandTypeMap["IR1"]         = Infrared1;
            m_TokenToBandTypeMap["IR1/T"]       = Infrared1;
                
            // Infrared two...
            m_TokenToBandTypeMap["IR2"]         = Infrared2;
            m_TokenToBandTypeMap["IR2/T"]       = Infrared2;
            
            // Infrared three...
            m_TokenToBandTypeMap["IR3"]         = Infrared3;
            m_TokenToBandTypeMap["IR3/T"]       = Infrared3;

        // Narrow band for the Sun...
        m_TokenToBandTypeMap["SUN"]             = Sun;
    
        // Broad band for survey...
        m_TokenToBandTypeMap["SUR"]             = Survey;
        m_TokenToBandTypeMap["SURV"]            = Survey;
        m_TokenToBandTypeMap["SURV/S"]          = Survey;
        m_TokenToBandTypeMap["SURVEY"]          = Survey;
        
        // Identifiable, but unsupported broad band diodes...
        m_TokenToBandTypeMap["BB1"]             = Unknown;
        m_TokenToBandTypeMap["BB1/S"]           = Unknown;
        m_TokenToBandTypeMap["BB2"]             = Unknown;
        m_TokenToBandTypeMap["BB2/S"]           = Unknown;
        m_TokenToBandTypeMap["BB3"]             = Unknown;
        m_TokenToBandTypeMap["BB3/S"]           = Unknown;
        m_TokenToBandTypeMap["BB4"]             = Unknown;
        m_TokenToBandTypeMap["BB4/S"]           = Unknown;

    // Initialize the diode band to friendly dictionary...

        // Unknown...
        m_BandTypeToFriendlyMap[Unknown]    = "unknown";

        // Narrow band for colour...
        m_BandTypeToFriendlyMap[Red]        = "red";
        m_BandTypeToFriendlyMap[Green]      = "green";
        m_BandTypeToFriendlyMap[Blue]       = "blue";
            
        // Narrow band for infrared...
        m_BandTypeToFriendlyMap[Infrared1]  = "infrared 1";
        m_BandTypeToFriendlyMap[Infrared2]  = "infrared 2";
        m_BandTypeToFriendlyMap[Infrared3]  = "infrared 3";

        // Narrow band for the Sun...
        m_BandTypeToFriendlyMap[Sun]        = "sun";
    
        // Broad band for survey...
        m_BandTypeToFriendlyMap[Survey]     = "survey";
}

// Get the diode band type as a human friendly string...
const string &VicarImageBand::GetDiodeBandTypeFriendlyString() const
{
    // Lookup the diode band type in the band type to friendly map...
    const BandTypeToFriendlyMap::const_iterator Iterator =
        m_BandTypeToFriendlyMap.find(GetDiodeBandType());

    // Should always have been found...
    assert(Iterator != m_BandTypeToFriendlyMap.end());

    // Return the friendly string...
    return Iterator->second;
}

// Get a stream opened and ready to extract where raw image data 
//  begins. Must be loaded first. Argument is streams to use...
bool VicarImageBand::GetExtractionStream(std::ifstream &ExtractionStream)
{
    // Check if file was loaded ok...
    if(!IsOk())
        SetErrorAndReturnFalse("input was not loaded")

    // Stream should not be opened...
    assert(!ExtractionStream.is_open());

    // Open the input file...
    ExtractionStream.open(m_InputFile.c_str(), ifstream::in | ifstream::binary);
    
        // Failed...
        if(!ExtractionStream.is_open())
        {
            // Alert, cleanup, and abort...
            SetErrorMessage("could not open input for reading");
            ExtractionStream.close();
            return false;
        }

    // Seek to raw image offset and make sure it was successful...
    if(!ExtractionStream.seekg(m_RawImageOffset, ios_base::beg).good())
        SetErrorAndReturnFalse("file ended prematurely before raw image");

    // Done...
    return true;
}

// Get the file size, or -1 on error...
int VicarImageBand::GetFileSize() const
{
    // Open the file...
    ifstream InputFileStream(m_InputFile.c_str(), ifstream::in | ifstream::binary);
    
        // Failed...
        if(!InputFileStream.is_open())
            return -1;

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

// Is the token a valid VICAR diode band type?
bool VicarImageBand::IsVicarTokenDiodeBandType(const string &DiodeBandTypeToken) const
{
    // Lookup the VICAR token in the token to band type map...
    const TokenToBandTypeMap::const_iterator Iterator =
        m_TokenToBandTypeMap.find(DiodeBandTypeToken);

    // Check if found...
    return (Iterator != m_TokenToBandTypeMap.end());
}

// Check if the header is at least readable, and if so, phase offset 
//  required to decode file...
bool VicarImageBand::IsHeaderIntact(size_t &PhaseOffsetRequired) const
{
    // Open the file...
    ifstream InputFileStream(m_InputFile.c_str(), ifstream::in | ifstream::binary);

        // Load() already succeeded in opening, so this shouldn't ever happen...
        assert(InputFileStream.is_open());

    // Sometimes the records are out of phase due to being preceeded 
    //  with VAX/VMS prefix bytes, so check for threshold of at most
    //  four bytes...
    for(PhaseOffsetRequired = 0; PhaseOffsetRequired < 4; ++PhaseOffsetRequired)
    {
        // Seek to offset...
        InputFileStream.seekg(PhaseOffsetRequired);

        // Load the first logical record...
        const LogicalRecord HeaderRecord(InputFileStream);

        // Check if valid first end of logical record marker......
        if(HeaderRecord.IsValidLabel())
            return true;
    }

    // Tried even with a phase offset and still didn't work...
    PhaseOffsetRequired = 0;
    return false;
}

// Check ifthis is actually from the Viking Lander EDR...
bool VicarImageBand::IsVikingLanderOrigin() const
{
    // Buffer to hold first 256 bytes of file...
    vector<char>    Buffer(256, 0x00);

    // Open the file...
    ifstream InputFileStream(m_InputFile.c_str(), ifstream::in | ifstream::binary);

        // Load() already succeeded in opening, so this shouldn't ever happen...
        assert(InputFileStream.is_open());

    // Don't skip white space... (implicit if opening in binary mode?)
    InputFileStream >> std::noskipws;

    // Fully fill the buffer with the first 256 bytes of the file...
    InputFileStream.read(&Buffer.front(), Buffer.size());
    assert(InputFileStream.gcount() == static_cast<signed>(Buffer.size()));

    // Signature to search for in EBCDIC ("VIKING LANDER " in ASCII)
    const char Signature[] = {
        0xE5, 0xC9, 0xD2, 0xC9, 0xD5, 0xC7, 0x40, 
        0xD3, 0xC1, 0xD5, 0xC4, 0xC5, 0xD9, 0x40
    };

    // Scan for Viking Lander signature...
    vector<char>::const_iterator SearchIterator = search(
        Buffer.begin(), Buffer.end(), Signature, Signature + sizeof(Signature));

    // Found...
    if(SearchIterator != Buffer.end())
        return true;

    // Not found...
    else
        return false;
}

// Load as much of the file as possible, setting error on failure...
void VicarImageBand::Load()
{
    // Objects and variables...
    LogicalRecord   Record;

    // Set the file name for console messages to be preceded with...
    Console::GetInstance().SetCurrentFileName(GetInputFileNameOnly());

    // Be verbose...
    Message(Console::Verbose) << "opening" << endl;

    // Open the file...
    ifstream InputFileStream(m_InputFile.c_str(), ifstream::in | ifstream::binary);

        // Failed...
        if(!InputFileStream.is_open())
            SetErrorAndReturn("could not open input for reading")

    // Check size...
    const int FileSize = GetFileSize();

        // Empty...
        if(FileSize == 0)
            SetErrorAndReturn("empty file, probably blank magnetic tape")

        // Check to make sure it is at least four kilobytes...
        else if(FileSize < (4 * 1024))
            SetErrorAndReturn("too small to be interesting (< 4 KB)")

    // Check if the header is at least readable, and if so, retrieve
    //  phase offset required to decode the file...
    if(!IsHeaderIntact(m_PhaseOffsetRequired))
        SetErrorAndReturn("header is not intact, or not a VICAR file")
    else if(m_PhaseOffsetRequired > 0)
        Message(Console::Verbose) << "header intact, but requires " << m_PhaseOffsetRequired << " byte phase offset" << endl;

    // Verify it's from one of the Viking Landers...
    if(!IsVikingLanderOrigin())
        SetErrorAndReturn("did not originate from a Viking Lander")

    // Extract the basic image metadata...
    ParseBasicMetadata(InputFileStream);
    
        // Error occured, stop...
        if(IsError())
            return;

    // Now rewind again to start of file, plus any phase offset necessary...
    InputFileStream.seekg(0 + m_PhaseOffsetRequired, ios_base::beg);

    // Clear saved labels buffer, in case it already had data in it...
    m_SavedLabelsBuffer.clear();

    // Go through all physical records, parsing extended metadata, skipping past
    //  padding between physical records, and calculating the raw image data's 
    //  absolute offset...
    for(size_t PhysicalRecordIndex = 0; InputFileStream.good(); ++PhysicalRecordIndex)
    {
        // Verbosity...
        Message(Console::Verbose)
            << "entering physical record " 
            << PhysicalRecordIndex + 1 
            << " starting at " 
            << static_cast<int>(InputFileStream.tellg()) 
            << endl;

        // Local offset into the current physical record...
        streampos LocalPhysicalRecordOffset = 0;
        
        // True if the raw image data was found...
        bool RawImageDataFound = false;

        // Examine each of the five logical records of this physical record...
        for(size_t LocalLogicalRecordIndex = 0; 
            LocalLogicalRecordIndex < 5; 
          ++LocalLogicalRecordIndex)
        {
            // Verbosity...
            Message(Console::Verbose) 
                << "extracting logical record " 
                << LocalLogicalRecordIndex + 1 
                << "/5 starting at " 
                << static_cast<int>(InputFileStream.tellg()) 
                << endl;

            // Extract a logical record...
            Record << InputFileStream;
            
            // Record isn't valid...
            if(!Record.IsValidLabel())
            {
                // Verbosity...
                Message(Console::Error) 
                    << "bad logical record terminator " 
                    << LocalLogicalRecordIndex + 1 
                    << "/5 starting at " 
                    << static_cast<int>(InputFileStream.tellg()) 
                    << endl;
                
                // Give a hint if this was suppose to be a physical record boundary...
                if(LocalLogicalRecordIndex == 0)
                    SetErrorAndReturn("invalid logical record label possibly from out of phase physical boundary")
                else
                    SetErrorAndReturn("invalid logical record label")
            }

            // Parse the extended metadata, if any...
            ParseExtendedMetadata(Record);
            
                // Error occured, stop...
                if(IsError())
                    return;

            // Add to saved labels buffer...
            m_SavedLabelsBuffer += Record.GetString() + '\n';

            // Update local offset into the current physical record...
            LocalPhysicalRecordOffset += LOGICAL_RECORD_SIZE;

            // This is the last logical record of all record labels...
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
                Message(Console::Verbose) << "tangential physical record boundary detected, ignoring padding" << endl;
                InputFileStream.seekg(CurrentPosition);
            }
            
            // Otherwise, seek passed any padding that may have followed
            //  the logical record set to the next physical record boundary...
            else
            {
                // Alert and seek...
                Message(Console::Verbose) << "seeking passed " << m_PhysicalRecordPadding << " physical record padding" << endl;
                InputFileStream.seekg(CurrentPosition);
                InputFileStream.seekg(m_PhysicalRecordPadding, ios_base::cur);
            }
    }

    // Got to the end of the file and did not find the last label record...
    if(!InputFileStream.good())
        SetErrorAndReturn("unable to locate last logical record label")

    // Store raw image offset...
    m_RawImageOffset = InputFileStream.tellg();

    // Show user, if requested...
    Message(Console::Verbose) << "raw image offset: " << m_RawImageOffset << hex << showbase << " (" << m_RawImageOffset<< ")" << dec << endl;

    // Now we know an absolute lower bound for file size, check...
    const int RequiredMinimumSize = m_RawImageOffset + (m_Bands * m_Height * m_Width * m_BytesPerColour);
    if(FileSize < RequiredMinimumSize)
        SetErrorAndReturn("file too small to contain self described band data payload");

    // Loaded ok...
    m_Ok = true;
}

// For comparing quality between images of the same camera event 
//  and same band type...
bool VicarImageBand::operator<(const VicarImageBand &RightSide) const
{
    /*
        Comparison Rules:

            Rule 1: Indiscernable images or ones without scanline axis
            are either corrupt or there is a better image with the 
            scanline axis somewhere in the image set.

                Method: Check for scanline axis presence by looking
                for "LINE" text.

            Rule 2: Images without a histogram are better than ones 
            with them because they have more image space.
                
                Method: Check for histogram text (e.g. "VIKING LANDER")

    */

    // Stub for now...
    return true;
}

// Parse basic metadata. Basic metadata includes bands, dimensions, 
//  pixel format, bytes per colour, photosensor diode band type, etc...
void VicarImageBand::ParseBasicMetadata(ifstream &InputFileStream)
{
    // Variables...
    string          Token;
    string          DiodeBandTypeHint;
    size_t          TokenIndex          = 0;
    size_t          TokenLength[32];

    // Stream should have already been validated...
    assert(InputFileStream.good());

    // Probe for the photosensor diode band type...
    m_DiodeBandType = ProbeDiodeBandType(DiodeBandTypeHint);

        // Not a supported band type...
        if(m_DiodeBandType == Unknown)
        {
            // This was probably just an internal radiometric / geometric 
            //  calibration shot and not meant to be interesting...
            if(DiodeBandTypeHint.find("CAL") != string::npos)
                SetErrorAndReturn(
                    string("internal radio/geometric calibration (") + 
                    DiodeBandTypeHint + 
                    string(")"))

            // Some other...
            else
                SetErrorAndReturn(string("unsupported photosensor diode band type (") + 
                    DiodeBandTypeHint + 
                    string(")"))
        }

    // Extract the header record...
    const LogicalRecord HeaderRecord(InputFileStream);

    // Clear token length buffer...
    memset(TokenLength, 0, sizeof(TokenLength));

    // Initialize the token counter, skipping past the first two 
    //  magnetic tape marker bytes...
    stringstream TokenCounter(HeaderRecord.GetString(true, 2));

    // Count how many tokens are there, seeking passed two byte binary 
    //  marker. This is necessary to know since different label formats
    //  can be distinguished by the number of whitespace separated 
    //  tokens...
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
            SetErrorAndReturn("exhausted basic metadata parser heuristics")

    // Perform sanity check on basic metadata...

        // Check bands...
        if(m_Bands != 1)
            SetErrorAndReturn("unsupported number of image bands")

        // Check height...
        if(m_Height <= 0)
            SetErrorAndReturn("expected positive image height")

        // Check width...
        if(m_Width <= 0)
            SetErrorAndReturn("expected positive image width")

        // Check pixel format is integral...
        if(m_PixelFormat != 'I' && /* Definitely integral */
           m_PixelFormat != 'L')   /* Guessing integral */
            SetErrorAndReturn("unsupported pixel format")

        // Check bytes per colour...
        if(m_BytesPerColour != 1)
            SetErrorAndReturn("unsupported colour bit depth")

    // If verbosity is set, display basic metadata...
    Message(Console::Verbose) << "bands: " << m_Bands << endl;
    Message(Console::Verbose) << "height: " << m_Height << endl;
    Message(Console::Verbose) << "width: " << m_Width << endl;
    Message(Console::Verbose) << "raw band data size: " << m_Width * m_Height * m_BytesPerColour << " bytes" << endl;
    Message(Console::Verbose) << "file size: " << GetFileSize() << " bytes" << endl;
    Message(Console::Verbose) << "format: " << "integral" << endl;
    Message(Console::Verbose) << "bytes per colour: " << m_BytesPerColour << endl;
    Message(Console::Verbose) << "photosensor diode band type: " << GetDiodeBandTypeFriendlyString() << endl;
    Message(Console::Verbose) << "physical record size: " << m_PhysicalRecordSize << hex << showbase << " (" << m_PhysicalRecordSize<< ")" << endl;
    Message(Console::Verbose) << "possible physical record padding: "  << dec << m_PhysicalRecordPadding << hex << showbase << " (" << m_PhysicalRecordPadding<< ")" << dec << endl;

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
    Message(Console::Verbose) << "heuristics selected format 1 basic metadata parser" << endl;

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
    Message(Console::Verbose) << "heuristics selected format 2 basic metadata parser" << endl;

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
    Message(Console::Verbose) << "heuristics selected format 3 basic metadata parser" << endl;

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
    Message(Console::Verbose) << "heuristics selected format 4 basic metadata parser" << endl;

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
    Message(Console::Verbose) << "heuristics selected format 5 basic metadata parser" << endl;

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
    Message(Console::Verbose) << "heuristics selected format 6 basic metadata parser" << endl;

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

// Parse extended metadata, if any. Extended metadata includes the 
//  azimuth and elevation...
void VicarImageBand::ParseExtendedMetadata(const LogicalRecord &Record)
{
    // Variables...
    string Token;
    
    // Is it valid?
    if(!Record.IsValidLabel())
        SetErrorAndReturn("invalid logical record label while parsing extended metadata")

    // Initialize tokenizer...
    stringstream Tokenizer(Record);
    
    // Keep scanning until no more tokens...
    for(size_t TokenIndex = 0; Tokenizer.good(); ++TokenIndex)
    {
        // Get first token...
        Tokenizer >> Token;

        // Found azimuth and elevation, which seems to always occupy whole record...
        if(Token == "AZIMUTH" && (TokenIndex == 0))
        {
            // Extract without surrounding whitespace...
            m_AzimuthElevation = Record.GetString(true);

            // Alert user if verbose mode enabled...
            Message(Console::Verbose) << "psa directional vector: " << m_AzimuthElevation << endl;
        }
        
        // Possibly camera event label...
        else if(Token == "CE")
        {
            // Check...
            Tokenizer >> Token;
            
            // Confirmed...
            if(Token == "LABEL")
            {
                // Store the label...
                Tokenizer >> m_CameraEventLabel;
                Message(Console::Verbose) << "camera event label: " << m_CameraEventLabel << endl;
            }
            
            // Not a camera event, restore the token...
            else
                Tokenizer << Token;
        }
    }
}

// Perform a deep probe on the file to check for the photosensor diode band type, 
//  returning Unknown if couldn't detect it or unsupported. The parameter can be
//  used for callee to store for caller the token that probably denotes an 
//  unsupported diode type...
VicarImageBand::PSADiode VicarImageBand::ProbeDiodeBandType(string &DiodeBandTypeHint) const
{
    // Open the file...
    ifstream InputFileStream(m_InputFile.c_str(), ifstream::in | ifstream::binary);
    
        // Should have already been openable, since we did so in Load()...
        assert(InputFileStream.is_open());

    // Account for any required phase offset...
    InputFileStream.seekg(m_PhaseOffsetRequired);

    // Setup caller's default return value...
    DiodeBandTypeHint = "unknown";

    // Check anywhere within the first physical record...
    for(size_t LogicalRecordIndex = 0; LogicalRecordIndex < 5; ++LogicalRecordIndex)
    {
        // Variables...
        string  PreviousToken;
        string  CurrentToken;

        // Extract record...
        LogicalRecord Record(InputFileStream);

        // Initialize tokenizer, skipping first two magnetic tape marker 
        //  bytes if first record...
        stringstream Tokenizer(Record.GetString(false, LogicalRecordIndex > 0 ? 0 : 2));

        // Keep extracting tokens while there are some in this record...
        for(size_t TokenIndex = 0; Tokenizer.good(); ++TokenIndex)
        {
            // Remember the last read token...
            PreviousToken = CurrentToken;
            
            // Extract a new token...
            Tokenizer >> CurrentToken;

            // We don't know how to deal with these yet...
            if(CurrentToken == "MONOCOLOR")
            {
                // Set caller hint and return unsupported...
                DiodeBandTypeHint = "monocolour unsupported";
                return Unknown;            
            }

            // Some kind of broad band diode that may or may not identify itself...
            if(CurrentToken == "BROADBAND")
            {
                // Set caller hint and return unsupported...
                DiodeBandTypeHint = "unidenfiable broadband";
                return Unknown;
            }

            // Not a diode marker token, skip...
            if(CurrentToken != "DIODE")
                continue;

            // End of token stream...
            if(!Tokenizer.good())
            {
                // Recognized the token before DIODE marker...
                if(IsVicarTokenDiodeBandType(PreviousToken))
                {
                    DiodeBandTypeHint = PreviousToken;
                    return GetDiodeBandTypeFromVicarToken(PreviousToken);
                }

                // Otherwise we got to the end of this logical record and 
                //  found nothing. Try next one...
                else
                    break;
            }

            // Otherwise extract next token which may be the diode type...
            Tokenizer >> CurrentToken;

            // Check if it is a diode type...
            if(IsVicarTokenDiodeBandType(CurrentToken))
            {
                DiodeBandTypeHint = CurrentToken;
                return GetDiodeBandTypeFromVicarToken(CurrentToken);
            }

            // Otherwise try the token before the diode marker...
            else if(IsVicarTokenDiodeBandType(PreviousToken))
            {
                DiodeBandTypeHint = PreviousToken;
                return GetDiodeBandTypeFromVicarToken(PreviousToken);
            }
            
            // Neither recognized on either side of the diode marker...
            else
            {
                // Save hint for caller and return unknown...
                DiodeBandTypeHint = CurrentToken;
                return Unknown;
            }
        }
    }

    // Set hint and return unknown...
    DiodeBandTypeHint = "none detected";
    return Unknown;
}

// Set the photosensor diode band type from VICAR token... (e.g. "RED/T")
VicarImageBand::PSADiode VicarImageBand::GetDiodeBandTypeFromVicarToken(
    const string &DiodeBandTypeToken) const
{
    // Lookup the VICAR token in the token to band type map...
    const TokenToBandTypeMap::const_iterator Iterator =
        m_TokenToBandTypeMap.find(DiodeBandTypeToken);

    // Found...
    if(Iterator != m_TokenToBandTypeMap.end())
        return Iterator->second;

    // Not found...
    else
        return Unknown;
}


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

// Includes...

    // Provided by Autoconf...
    #include <config.h>

    // Our headers...    
    #include "Console.h"
    #include "LogicalRecord.h"
    #include "Miscellaneous.h"
    #include "Options.h"
    #include "VicarImageBand.h"
    #include "ZZipFileDescriptor.h"

    // PNG writing...
    #include <png++/png.hpp>
    
    // Optical character recognition...
    #include <ocradlib.h>

    // zziplib...
    #include <zzip/zzip.h>

    // POSIX headers...
    #include <fnmatch.h>

    // Standard system headers...
    #include <algorithm>
    #include <cmath>
    #include <cstring>
    #include <iomanip>
    #include <iostream>
    #include <iterator>
    #include <limits>
    #include <sstream>

// Using the standard namespace...
using namespace std;

// Construct...
VicarImageBand::VicarImageBand(
    const string &InputFile)
    : m_AxisPresent(false),
      m_Bands(0),
      m_BasicMetadataParserHeuristic(0),
      m_BytesPerColour(0),
      m_DiodeBandType(Unknown),
      m_FileOrdinalOnMagneticTape(0),
      m_FullHistogramPresent(false),
      m_InputFile(InputFile),
      m_LanderNumber(0),
      m_MagneticTapeNumber(0),
      m_MeanPixelValue(0.0f),
      m_Ok(false),
      m_OriginalHeight(0),
      m_OriginalWidth(0),
      m_PhaseOffsetRequired(0),
      m_PhysicalRecordPadding(0),
      m_PhysicalRecordSize(0),
      m_PixelFormat(0),
      m_RawImageOffset(0),
      m_Rotation(None),
      m_SolarDay(99999)
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
        m_TokenToBandTypeMap["BB1"]             = Broadband1;
        m_TokenToBandTypeMap["BB1/S"]           = Broadband1;
        m_TokenToBandTypeMap["BB2"]             = Broadband2;
        m_TokenToBandTypeMap["BB2/S"]           = Broadband2;
        m_TokenToBandTypeMap["BB3"]             = Broadband3;
        m_TokenToBandTypeMap["BB3/S"]           = Broadband3;
        m_TokenToBandTypeMap["BB4"]             = Broadband4;
        m_TokenToBandTypeMap["BB4/S"]           = Broadband4;

    // Initialize the diode band to friendly dictionary...

        // Unknown...
        m_BandTypeToFriendlyMap[Unknown]    = "unknown";

        // High resolution broadband...
        m_BandTypeToFriendlyMap[Broadband1] = "broadband 1";
        m_BandTypeToFriendlyMap[Broadband2] = "broadband 2";
        m_BandTypeToFriendlyMap[Broadband3] = "broadband 3";
        m_BandTypeToFriendlyMap[Broadband4] = "broadband 4";

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

    // Get the magnetic tape and file on tape number...
    
        // Get just the input file name...
        const string InputFileNameOnly = GetInputFileNameOnly();

        // Appears to be properly formatted...
        if(InputFileNameOnly.length() > 3 && InputFileNameOnly.compare(0, 3, "vl_") == 0)
        {
            // Find the separator between magnetic tape and file number...
            const size_t SeparatorIndex = InputFileNameOnly.find(".");
            if(SeparatorIndex != string::npos)
            {
                // Extract the magnetic tape number...
                const string MagneticTapeNumber(InputFileNameOnly, 3, SeparatorIndex - 3);
                m_MagneticTapeNumber = atoi(MagneticTapeNumber.c_str());

                // Extract the file ordinal on the original magnetic tape...
                const string FileOrdinalOnMagneticTape(InputFileNameOnly, SeparatorIndex + 1);
                m_FileOrdinalOnMagneticTape = atoi(FileOrdinalOnMagneticTape.c_str());
            }
        }
}

// Check if the raw band image data, rotated as requested, 
//  contains text usually found in an image with azimuth / 
//  elevation axes oriented properly. If so, return true
//  and store extracted text in buffer...
bool VicarImageBand::CheckForHorizontalAxisAndExtractText(
    const RawBandDataType &RawBandData, 
    const RotationType Rotation, 
    string &OCRBuffer)
{
    // Space for rotated raw band data...
    RawBandDataType RotatedRawBandData;

    // Rotated as requested...
    Rotate(Rotation, RawBandData, RotatedRawBandData);

    // Extract the OCR text and check for error...
    if(!ExtractOCR(RotatedRawBandData, OCRBuffer, Rotation))
        return false;

    // Look for words we would expect to see if oriented properly...
    if(OCRBuffer.find("AZ") != string::npos) return true;
    else if(OCRBuffer.find("CAMERA") != string::npos) return true;
    else if(OCRBuffer.find("SCAN") != string::npos) return true;
    else if(OCRBuffer.find("LINE") != string::npos) return true;
    else if(OCRBuffer.find("IPL") != string::npos) return true;
    else if(OCRBuffer.find("SAMPLE") != string::npos) return true;
    else return false;
}

// Check if the raw band image data, rotated as requested, 
//  contains text usually found in an image with with a large 
//  histogram present. If so, return true and store extracted 
//  text in buffer...
bool VicarImageBand::CheckForLargeHistogramAndExtractText(
    const RawBandDataType &RawBandData, 
    const RotationType Rotation, 
    string &OCRBuffer)
{
    // Space for rotated raw band data...
    RawBandDataType RotatedRawBandData;

    // Rotated as requested...
    Rotate(Rotation, RawBandData, RotatedRawBandData);

    // Extract the OCR text and check for error...
    if(!ExtractOCR(RotatedRawBandData, OCRBuffer, Rotation))
        return false;

    // Look for words we would expect to see if oriented properly...
    if(OCRBuffer.find("VIKING") != string::npos) return true;
    else if(OCRBuffer.find("LANDER") != string::npos) return true;
    else if(OCRBuffer.find("LABEL") != string::npos) return true;
    else if(OCRBuffer.find("DIODE") != string::npos) return true;
    else if(OCRBuffer.find("CHANNEL") != string::npos) return true;
    else if(OCRBuffer.find("AZIMUTH") != string::npos) return true;
    else if(OCRBuffer.find("ELEVATION") != string::npos) return true;
    else if(OCRBuffer.find("OFFSET") != string::npos) return true;
    else if(OCRBuffer.find("RESCAN") != string::npos) return true;
    else if(OCRBuffer.find("SEGMENT") != string::npos) return true;
    else if(OCRBuffer.find("MEAN") != string::npos) return true;
    else return false;
}

// Examine image visually to determine things like suggested 
//  orientation, optical character recognition, and histogram 
//  detection, or set an error...
bool VicarImageBand::ExamineImageVisually()
{
    // Space for the original unrotated as well as the rotated image band data...
    RawBandDataType RawBandData;
    RawBandDataType RotatedBandData;

    // Get the raw band data and check for error. No need to set an 
    //  error since callee does this...
    if(!GetRawBandData(RawBandData))
        return false;

    // Check orientation by looking for large histogram's text which 
    //  is always 90 degrees counterclockwise rotated away from normal 
    //  image orientation...

        // Image needs to be rotated 90 degrees counterclockwise...
        if(CheckForLargeHistogramAndExtractText(RawBandData, None, m_OCRBuffer))
        {
            Message(Console::Verbose) << _("image should be rotated 90 counterclockwise") << endl;
            m_Rotation = Rotate90;
            m_FullHistogramPresent = true;
            m_AxisPresent = true;
        }
        
        // Image needs to be rotated 180 degrees counterclockwise...
        else if(CheckForLargeHistogramAndExtractText(RawBandData, Rotate90, m_OCRBuffer))
        {
            Message(Console::Verbose) << _("image should be rotated 180 counterclockwise") << endl;
            m_Rotation = Rotate180;
            m_FullHistogramPresent = true;
            m_AxisPresent = true;
        }
        
        // Image needs to be rotated 270 degrees counterclockwise...
        else if(CheckForLargeHistogramAndExtractText(RawBandData, Rotate180, m_OCRBuffer))
        {
            Message(Console::Verbose) << _("image should be rotated 270 counterclockwise") << endl;
            m_Rotation = Rotate270;
            m_FullHistogramPresent = true;
            m_AxisPresent = true;
        }
        
        // Image does not need be rotated...
        else if(CheckForLargeHistogramAndExtractText(RawBandData, Rotate270, m_OCRBuffer))
        {
            Message(Console::Verbose) << _("image does not need to be rotated") << endl;
            m_Rotation = None;
            m_FullHistogramPresent = true;
            m_AxisPresent = true;
        }
        
        // No large large histogram found. Check for properly oriented 
        //  azimuth / elevation axes...
        else
        {
            // Image does not need be rotated...
            if(CheckForHorizontalAxisAndExtractText(RawBandData, None, m_OCRBuffer))
            {
                Message(Console::Verbose) << _("image does not need to be rotated") << endl;
                m_Rotation = None;
                m_AxisPresent = true;
            }

            // Image needs to be rotated 90 degrees counterclockwise...
            else if(CheckForHorizontalAxisAndExtractText(RawBandData, Rotate90, m_OCRBuffer))
            {
                Message(Console::Verbose) << _("image should be rotated 90 counterclockwise") << endl;
                m_Rotation = Rotate90;
                m_AxisPresent = true;
            }
            
            // Image needs to be rotated 180 degrees counterclockwise...
            else if(CheckForHorizontalAxisAndExtractText(RawBandData, Rotate180, m_OCRBuffer))
            {
                Message(Console::Verbose) << _("image should be rotated 180 counterclockwise") << endl;
                m_Rotation = Rotate180;
                m_AxisPresent = true;
            }
            
            // Image needs to be rotated 270 degrees counterclockwise...
            else if(CheckForHorizontalAxisAndExtractText(RawBandData, Rotate270, m_OCRBuffer))
            {
                Message(Console::Verbose) << _("image should be rotated 270 counterclockwise") << endl;
                m_Rotation = Rotate270;
                m_AxisPresent = true;
            }
            
            // No legible text hints found. Probably image without any axis or histogram overlay...
            else
            {
                Message(Console::Warning) << _("could not guess image rotation") << endl;
                m_Rotation = None;
                m_OCRBuffer.clear();
            }
        }

    // If autorotation isn't enabled, then leave rotation as none...
    if(!Options::GetInstance().GetAutoRotate())
        m_Rotation = None;

    // Done...
    return true;
}

// Extract OCR within image band data to buffer...
bool VicarImageBand::ExtractOCR(
    const RawBandDataType &RawBandData, 
    string &Extracted, 
    const RotationType RotationHint)
{
    // Check some assumptions...
    assert(!RawBandData.empty());

    // Clear the output buffer...
    Extracted.clear();

    // Check if we've already performed this computation...
    RotationOCRCacheIterator CacheIterator 
        = m_RotationOCRCache.find(RotationHint);

        // Hit...
        if(CacheIterator != m_RotationOCRCache.end())
        {
            // Alert user...
            Message(Console::Verbose) << _("annotation cache hit optimization") << endl;
            
            // Return the cached result...
            Extracted = CacheIterator->second;
            return true;
        }

    // Get the width and height of this raw band data...
    const size_t Height = RawBandData.size();
    const size_t Width  = RawBandData.at(0).size();

    // Initialize OCR library...
    OCRAD_Descriptor *LibraryDescriptor = OCRAD_open();
    
        // Fucked...
        if(OCRAD_get_errno(LibraryDescriptor) != OCRAD_ok)
            SetErrorAndReturnFalse(_("GNU Ocrad failed to initialize"));

    // Load the raw image band data...

        // OCRAD_greymap only works with single byte per pixel...
        assert(m_BytesPerColour == 1);

        // Space for flattened linear version of the raw band data...
        vector<uint8_t> FlattenedRawBandData;
        
        // Preallocate buffer...
        FlattenedRawBandData.reserve(Width * Height);
        FlattenedRawBandData.resize(Width * Height);

        // Collapse by flattening each row...
        for(size_t Y = 0; Y < Height; ++Y)
        {
            // Flatten each column in this row...
            memcpy(
                &FlattenedRawBandData.at(Y * Width), 
                &RawBandData.at(Y).front(),
                Width);
        }

        // Get direct address to flattened raw band data vector...
        vector<uint8_t>::const_iterator Iterator = FlattenedRawBandData.begin();
        const uint8_t *DataAddress = &(*Iterator);

        // Initialize the OCR image structure with the raw image data...
        OCRAD_Pixmap OcrImage;
        OcrImage.height = Height;
        OcrImage.width  = Width;
        OcrImage.mode   = OCRAD_greymap;
        OcrImage.data   = DataAddress;

    // Pass the image into the OCR library, inverted, and check for error...
    if(OCRAD_set_image(LibraryDescriptor, &OcrImage, true) != 0)
    {
        // Cleanup...
        OCRAD_close(LibraryDescriptor);

        // Set error message...
        SetErrorAndReturnFalse(_("could not set OCR image"));
    }

    // Algorithm seems to recognize VICAR text overlay better when the original 
    //  image is re-scaled by a factor of three and the threshhold is at 70. We
    //  can hardcode these constants since the Viking lander data set isn't 
    //  going to change...
    OCRAD_scale(LibraryDescriptor, 3);
    OCRAD_set_threshold(LibraryDescriptor, 70);
    
    // Perform optical character recognition and check for error...
    if(OCRAD_recognize(LibraryDescriptor, true) != 0)
    {
        // Cleanup...
        OCRAD_close(LibraryDescriptor);

        // Set error message...
        SetErrorAndReturnFalse(_("OCR pass failed"));
    }

    // Grab the text from each text block...
    for(int CurrentTextBlock = 0; 
        CurrentTextBlock < OCRAD_result_blocks(LibraryDescriptor); 
      ++CurrentTextBlock)
    {
        // Grab each line in this text block...
        for(int CurrentTextLine = 0;
            CurrentTextLine < OCRAD_result_lines(LibraryDescriptor, CurrentTextBlock);
          ++CurrentTextLine)
        {
            // Get the current text line from the current text block...
            Extracted += OCRAD_result_line(
                LibraryDescriptor, CurrentTextBlock, CurrentTextLine);
        }
    }

    // Cache this result...
    m_RotationOCRCache.insert(
        CacheIterator, RotationOCRCachePair(RotationHint, Extracted));

    // Be verbose...
    Message(Console::Verbose) 
        << Extracted.size() 
        << _(" potential character annotations detected")
        << endl;

    // Cleanup...
    OCRAD_close(LibraryDescriptor);

    // Return ok...
    return true;
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

// Get the Martian month of this camera event...
string VicarImageBand::GetMonth() const
{
    // We need to know relative to beginning of the Martian year the 
    //  absolute solar day of the local midnight immediately preceding 
    //  the lander's touch down. Lander 1 landed on June 20, 1976 
    //  (Virgo 6 = 198) and Lander 2 on Sept 30 1976... (Virgo 49 = 241)
    const size_t LanderTouchdownAbsoluteSolarDay = m_LanderNumber == 1 ? 199 : 242;

    // Calculate Ls... (solar longitude)
    const float MartianSolsPerYear  = 668.5991f;
    const float Ls = 
        SolarDayToLs(1 + fmodf(LanderTouchdownAbsoluteSolarDay + m_SolarDay, MartianSolsPerYear));

    // Convert Ls to month...
    if(Ls <= 30.0f)                         return string("Gemini");
    else if(30.0f < Ls && Ls <= 60.0f)      return string("Cancer");
    else if(60.0f < Ls && Ls <= 90.0f)      return string("Leo");
    else if(90.0f < Ls && Ls <= 120.0f)     return string("Virgo");
    else if(120.0f < Ls && Ls <= 150.0f)    return string("Libra");
    else if(150.0f < Ls && Ls <= 180.0f)    return string("Scorpius");
    else if(180.0f < Ls && Ls <= 210.0f)    return string("Sagittarius");
    else if(210.0f < Ls && Ls <= 240.0f)    return string("Capricorn");
    else if(240.0f < Ls && Ls <= 270.0f)    return string("Aquarius");
    else if(270.0f < Ls && Ls <= 300.0f)    return string("Pisces");
    else if(300.0f < Ls && Ls <= 330.0f)    return string("Aries");
    else                                    return string("Taurus");
}

// Get the file size, or -1 on error...
int VicarImageBand::GetFileSize() const
{
    // Open the file...
    ZZipFileDescriptor FileDescriptor(Open());
    
        // Failed...
        if(!FileDescriptor.IsGood())
            return -1;

    // Seek to the end and return the size...
    zzip_seek(FileDescriptor, 0, SEEK_END);
    return zzip_tell(FileDescriptor);
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

// Return a friendly human readable location of lander...
string VicarImageBand::GetLanderLocation() const
{
    // Determine base on lander number...
    switch(m_LanderNumber)
    {
        case 1: return string("Chryse Planitia");
        case 2: return string("Arcadia Planitia");
        default: return string("?");
    }
}

// Get the raw band data transformed if autorotate was enabled. Use 
//  GetTransformedWidth() / ...Height() to know adapted dimensions...
bool VicarImageBand::GetRawBandData(VicarImageBand::RawBandDataType &RawBandData)
{
    // Clear caller's band data...
    RawBandData.clear();

    // Check if file was loaded ok...
    if(!IsOk())
        SetErrorAndReturnFalse(_("input was not loaded"))

    // Open the input file...
    ZZipFileDescriptor FileDescriptor(Open());

        // Failed...
        if(!FileDescriptor.IsGood())
            SetErrorAndReturnFalse(_("could not open input for reading"));

    // Seek to raw image offset and make sure it was successful...
    if(zzip_seek(FileDescriptor, m_RawImageOffset, SEEK_SET) == -1)
        SetErrorAndReturnFalse(_("file ended prematurely before raw image"));

    // Clear the mean pixel value...
    m_MeanPixelValue = 0.0f;

    // Calculate inner bounding rectangle 1/3 image width and height to
    //  sample mean pixel value. We do this to prevent sampling from
    //  outside in the image overlay and histogram region...
    const size_t    SampleRegion_Top    = m_OriginalHeight / 3;
    const size_t    SampleRegion_Left   = m_OriginalWidth / 3;
    const size_t    SampleRegion_Right  = (m_OriginalWidth / 3) * 2;
    const size_t    SampleRegion_Bottom = (m_OriginalHeight / 3) * 2;
    register size_t SampleRegionSize    = 0;

    // Preallocate the band data buffer since we should know already dimensions
    //  of image...
    RawBandData.reserve(m_OriginalHeight);

    // Read the whole image, row by row...
    for(size_t Y = 0; Y < m_OriginalHeight; ++Y)
    {
        // The current row...
        vector<uint8_t> CurrentRow;
        
        // Preallocate the row...
        CurrentRow.reserve(m_OriginalWidth);
        CurrentRow.resize(m_OriginalWidth, 0xff);

        // Try to read the whole row in one pass...
        const size_t BytesRead = 
            zzip_read(FileDescriptor, &CurrentRow.front(), m_OriginalWidth);

            // Failed...
            if((BytesRead != m_OriginalWidth) || !FileDescriptor.IsGood())
                SetErrorAndReturnFalse(_("band data extraction i/o error"));

        // Update mean pixel value...
        for(size_t X = 0; X < m_OriginalWidth; ++X)
        {
            // Extract the current pixel value...
            const uint8_t CurrentByte = CurrentRow.at(X);

            // Check if within the pixel sample region...
            if(X >= SampleRegion_Left  && 
               X <= SampleRegion_Right &&
               Y >= SampleRegion_Top   &&
               Y <= SampleRegion_Bottom)
            {
                // Add to mean pixel value accumulator...
                m_MeanPixelValue += CurrentByte;
                SampleRegionSize++;
            }
        }

        /* Read each pixel in this row, byte by byte...
        for(size_t X = 0; X < m_OriginalWidth; ++X)
        {
            // Extract the current pixel value...
            register uint8_t CurrentByte = 0xff;

            // Check for error...
            if((zzip_read(FileDescriptor, &CurrentByte, sizeof(CurrentByte)) 
                    != sizeof(CurrentByte)) || !FileDescriptor.IsGood())
                SetErrorAndReturnFalse(_("band data extraction i/o error"));

            // Check if within the pixel sample region...
            if(X >= SampleRegion_Left  && 
               X <= SampleRegion_Right &&
               Y >= SampleRegion_Top   &&
               Y <= SampleRegion_Bottom)
            {
                // Add to mean pixel value accumulator...
                m_MeanPixelValue += CurrentByte;
                SampleRegionSize++;
            }

            // Add to row...
            CurrentRow.push_back(CurrentByte);
        }*/

        // Add row to list of columns...
        RawBandData.push_back(CurrentRow);
    }

    // Calculate the mean pixel value by dividing accumulator with 
    //  total rectangle size...
    m_MeanPixelValue /= SampleRegionSize;

    // Alert user if verbose mode enabled...
    Message(Console::Verbose) << _("mean pixel value: ") << m_MeanPixelValue << endl;

    // Auto rotate was requested and requires a rotation...
    if(Options::GetInstance().GetAutoRotate() && m_Rotation != None)
    {
        // Space for the rotated raw band data...
        RawBandDataType RotatedRawBandData;

        // Perform rotation...
        Rotate(m_Rotation, RawBandData, RotatedRawBandData);
        
        // Store result for caller...
        RawBandData = RotatedRawBandData;
    }

    // Done...
    return true;
}

// Get image height, accounting for transformations like rotation...
size_t VicarImageBand::GetTransformedHeight() const
{
    // Depending on the rotation applied, if any, width and height can be swapped...
    if(Options::GetInstance().GetAutoRotate() && (m_Rotation == Rotate90 || m_Rotation == Rotate270))
        return m_OriginalWidth;
    else
        return m_OriginalHeight;
}

// Get image width, accounting for transformations like rotation...
size_t VicarImageBand::GetTransformedWidth() const
{
    // Depending on the rotation applied, if any, width and height can be swapped...
    if(Options::GetInstance().GetAutoRotate() && (m_Rotation == Rotate90 || m_Rotation == Rotate270))
        return m_OriginalHeight;
    else
        return m_OriginalWidth;
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
    ZZipFileDescriptor FileDescriptor(Open());

        // Load() already succeeded in opening, so this shouldn't ever happen...
        assert(FileDescriptor.IsGood());

    // Sometimes the records are out of phase due to being preceeded 
    //  with VAX/VMS prefix bytes, so check for threshold of at most
    //  four bytes...
    for(PhaseOffsetRequired = 0; PhaseOffsetRequired < 4; ++PhaseOffsetRequired)
    {
        // Seek to offset and check for error...
        if(zzip_seek(FileDescriptor, PhaseOffsetRequired, SEEK_SET) == -1)
            return false;

        // Load the first logical record...
        const LogicalRecord HeaderRecord(FileDescriptor);

        // Check if valid first end of logical record marker......
        if(HeaderRecord.IsValidLabel())
            return true;
    }

    // Tried even with a phase offset and still didn't work...
    PhaseOffsetRequired = 0;
    return false;
}

// Check if this is actually from the Viking Lander EDR...
bool VicarImageBand::IsVikingLanderOrigin() const
{
    // Buffer to hold first 256 bytes of file...
    vector<char>    Buffer(256, 0x00);

    // Open the file...
    ZZipFileDescriptor FileDescriptor(Open());

        // Load() already succeeded in opening, so this shouldn't ever happen...
        assert(FileDescriptor.IsGood());

    /* Don't skip white space... (implicit if opening in binary mode?)
   *InputFileStream >> std::noskipws;*/

    // Fully fill the buffer with the first 256 bytes of the file...
    const size_t BytesRead = zzip_read(
        FileDescriptor, &Buffer.front(), Buffer.size());
    assert(BytesRead == Buffer.size());

    // Signature to search for in EBCDIC ("VIKING LANDER " in ASCII)
    const char Signature[] = {
        static_cast<char>(0xE5), 
        static_cast<char>(0xC9), 
        static_cast<char>(0xD2), 
        static_cast<char>(0xC9), 
        static_cast<char>(0xD5), 
        static_cast<char>(0xC7), 
        static_cast<char>(0x40), 
        static_cast<char>(0xD3), 
        static_cast<char>(0xC1), 
        static_cast<char>(0xD5), 
        static_cast<char>(0xC4), 
        static_cast<char>(0xC5), 
        static_cast<char>(0xD9), 
        static_cast<char>(0x40)
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
    Message(Console::Verbose) << _("loading") << endl;

    // Open the file...
    ZZipFileDescriptor FileDescriptor(Open());

        // Failed...
        if(!FileDescriptor.IsGood())
            SetErrorAndReturn(_("could not open input for reading"))

    // Check size...
    const int FileSize = GetFileSize();

        // Empty...
        if(FileSize == 0)
            SetErrorAndReturn(_("empty file, probably blank magnetic tape or not received back on Earth"))

        // Check to make sure it is at least four kilobytes...
        else if(FileSize < (4 * 1024))
            SetErrorAndReturn(_("too small to be interesting (< 4 KB)"))

    // Check if the header is at least readable, and if so, retrieve phase 
    //  offset required to decode the file...
    if(!IsHeaderIntact(m_PhaseOffsetRequired))
        SetErrorAndReturn(_("header is not intact, or not a VICAR file"))
    else if(m_PhaseOffsetRequired > 0)
        Message(Console::Verbose) << _("header intact, but requires ") << m_PhaseOffsetRequired << _(" byte phase offset") << endl;

    // Verify it's from one of the Viking Landers...
    if(!IsVikingLanderOrigin())
        SetErrorAndReturn(_("did not originate from a Viking Lander"))

    // Extract the basic image metadata...
    ParseBasicMetadata(FileDescriptor);
    
        // Error occured, stop...
        if(IsError())
            return;

    // Now rewind again to start of file, plus any phase offset necessary...
    zzip_seek(FileDescriptor, 0 + m_PhaseOffsetRequired, SEEK_SET);

    // Clear saved labels buffer, in case it already had data in it...
    m_SavedLabelsBuffer.clear();

    // Go through all physical records, parsing extended metadata, skipping past
    //  padding between physical records, and calculating the raw image data's 
    //  absolute offset...
    for(size_t PhysicalRecordIndex = 0; FileDescriptor.IsGood(); ++PhysicalRecordIndex)
    {
        // Verbosity...
        Message(Console::Verbose)
            << _("entering physical record ")
            << PhysicalRecordIndex + 1 
            << _(" starting at ")
            << static_cast<int>(zzip_tell(FileDescriptor)) 
            << hex 
                << showbase << " (" 
                << static_cast<int>(zzip_tell(FileDescriptor)) 
                << ")" << dec 
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
                << _("extracting logical record ") 
                << LocalLogicalRecordIndex + 1 
                << _("/5 starting at ")
                << static_cast<int>(zzip_tell(FileDescriptor)) 
                << hex 
                    << showbase << " (" 
                    << static_cast<int>(zzip_tell(FileDescriptor))
                    << ")" << dec 
                << endl;

            // Extract a logical record...
            Record << FileDescriptor;
            
            // Record isn't valid...
            if(!Record.IsValidLabel())
            {
                // Verbosity...
                Message(Console::Error) 
                    << _("bad logical record terminator ")
                    << LocalLogicalRecordIndex + 1 
                    << _("/5 starting at ")
                    << static_cast<int>(zzip_tell(FileDescriptor)) 
                    << endl;
                
                // Give a hint if this was suppose to be a physical record boundary...
                if(LocalLogicalRecordIndex == 0)
                    SetErrorAndReturn(_("invalid logical record label possibly from out of phase physical boundary"))
                else
                    SetErrorAndReturn(_("invalid logical record label"))
            }

            // Parse the extended metadata, if any...
            ParseExtendedMetadata(Record, LocalLogicalRecordIndex);
            
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
                // Calculate the offset necessary to seek past any remaining 
                //  logical records in this physical record, along with any 
                //  padding to the next physical record boundary...
                const streamoff RawImageDataRelativeOffset = 
                    ((LOGICAL_RECORD_SIZE * 5) - LocalPhysicalRecordOffset) + 
                    m_PhysicalRecordPadding;

                // Seek to the raw image data...
                zzip_seek(FileDescriptor, RawImageDataRelativeOffset, SEEK_CUR);

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
            const streampos CurrentPosition = zzip_tell(FileDescriptor);
            
            // Check to see if next physical record boundary was tangential...
            Record << FileDescriptor;
            if(Record.IsValidLabel())
            {
                // It was, so rewind and carry on since there is no 
                //  physical record padding...
                Message(Console::Verbose) << _("tangential physical record boundary detected, ignoring padding") << endl;
                zzip_seek(FileDescriptor, CurrentPosition, SEEK_SET);
            }
            
            // Otherwise, seek passed any padding that may have followed
            //  the logical record set to the next physical record boundary...
            else
            {
                // Alert and seek...
                Message(Console::Verbose) << _("seeking passed ") << m_PhysicalRecordPadding << _(" physical record padding") << endl;
                zzip_seek(FileDescriptor, CurrentPosition, SEEK_SET);
                zzip_seek(FileDescriptor, m_PhysicalRecordPadding, SEEK_CUR);
            }
    }

    // Got to the end of the file and did not find the last label record...
    if(!FileDescriptor.IsGood())
        SetErrorAndReturn(_("unable to locate last logical record label"))

    // Store raw image offset...
    m_RawImageOffset = zzip_tell(FileDescriptor);

    // Show user, if requested...
    Message(Console::Verbose) << _("raw image offset: ") << m_RawImageOffset << hex << showbase << " (" << m_RawImageOffset << ")" << dec << endl;

    // Calculate an absolute lower bound for file size...
    const int RequiredMinimumSize = 
        m_RawImageOffset + 
        (m_Bands * m_OriginalHeight * m_OriginalWidth * m_BytesPerColour);

    // Check that we are within the previous calculation...
    if(FileSize < RequiredMinimumSize)
    {
        // Compose error message...
        stringstream ErrorStream;
        ErrorStream 
            << _("file too small to contain claimed payload ")
            << FileSize
            << " < "
            << RequiredMinimumSize;

        // Set the error message and abort...
        SetErrorAndReturn(ErrorStream.str());
    }

    // Peform additional examination looking for things like histograms or other
    //  annotations, unless originating from broadband or solar PSAs which 
    //  didn't appear to contain any...
    if((m_DiodeBandType != Broadband1) && 
       (m_DiodeBandType != Broadband2) &&
       (m_DiodeBandType != Broadband3) && 
       (m_DiodeBandType != Broadband4) &&
       (m_DiodeBandType != Sun) &&
       (m_DiodeBandType != Survey))
    {
        // Examine image visually to determine things like suggested 
        //  orientation, optical character recognition, and histogram 
        //  detection...
        if(!ExamineImageVisually())
        {
            // Cleanup cache and abort...
            m_RotationOCRCache.clear();
            return;
        }
    }

    // Cleanup cache...
    m_RotationOCRCache.clear();

    // Loaded ok...
    m_Ok = true;
}

// Get a file stream to access the raw file. Returned handle tests as false if 
//  error...
ZZipFileDescriptor VicarImageBand::Open() const
{
    // Is this an archive? If so, open it as such...
    if(fnmatch(FNMATCH_ANY_ZIP ":/*", m_InputFile.c_str(), 0) == 0)
    {
        // Create the path to just the archive portion...
        const size_t Index = m_InputFile.find(":/");
        assert(Index != string::npos);
        const string ArchiveFileName(m_InputFile, 0, Index);

        // Check that path to compressed file within archive is sane...
        if(Index + 2 >= m_InputFile.size())
            return ZZipFileDescriptor(NULL);

        // Create the path to just the compressed file within the archive...
        const string CompressedFileName(m_InputFile, Index + 2);

        // Return the handle...
        return ZZipFileDescriptor(ArchiveFileName, CompressedFileName);
    }

    // Not an archive, open as a real file...
    else
        return ZZipFileDescriptor(m_InputFile.c_str());
}

// For comparing quality between images of same camera event / band type. Think
//  of these as a series of matching rules...
bool VicarImageBand::operator<(const VicarImageBand &RightSide) const
{
    // These should always be true...
    assert(m_DiodeBandType == RightSide.m_DiodeBandType);
    assert(m_CameraEventLabel == RightSide.m_CameraEventLabel);
    /*assert(m_Rotation == RightSide.m_Rotation);
    assert(GetTransformedHeight() == RightSide.GetTransformedHeight());
    assert(GetTransformedWidth() == RightSide.GetTransformedWidth());*/

    // If only one of the images has an axis present, the one with it
    //  we consider better...
    if(m_AxisPresent != RightSide.m_AxisPresent)
        return !m_AxisPresent;
    
    // If only one of the images has a full histogram present (which 
    //  implies the presence of an axis as well), the one with it is 
    //  inferior to the one without, since the one without has more image 
    //  space...
    else if(m_FullHistogramPresent != RightSide.m_FullHistogramPresent)
        return m_FullHistogramPresent;

    // If neither has an axis nor full histogram, or both do, the one
    //  that is brighter is the one we consider better, but only when not 
    //  dealing with survey and solar PSA...
    else if((m_DiodeBandType != Broadband1) &&
        (m_DiodeBandType != Broadband2) && 
        (m_DiodeBandType != Broadband3) && 
        (m_DiodeBandType != Broadband4) &&  
        (m_DiodeBandType != Survey) && 
        (m_DiodeBandType != Sun))
        return (m_MeanPixelValue < RightSide.m_MeanPixelValue);

    // If the PSA is broadband, survey, or solar, pick the one that is better...
    else
    {
        // If they both cover the same number of pixels, select the one that is
        //  brighter...
        if(GetTotalOriginalPixelSpace() == RightSide.GetTotalOriginalPixelSpace())
            return (m_MeanPixelValue < RightSide.m_MeanPixelValue);
        
        // Otherwise select the one that covers a greater pixel area...
        else
            return (GetTotalOriginalPixelSpace() < RightSide.GetTotalOriginalPixelSpace());
    }

    /*
    else
    {
        cout << "Left: " << m_InputFile << endl;
        cout << "Right: " << RightSide.m_InputFile << endl;
        return true;
    }*/
}

// Parse basic metadata. Basic metadata includes bands, dimensions, 
//  pixel format, bytes per colour, photosensor diode band type, etc...
void VicarImageBand::ParseBasicMetadata(ZZipFileDescriptor &FileDescriptor)
{
    // Variables...
    string          Token;
    string          DiodeBandTypeHint;
    size_t          TokenIndex          = 0;
    size_t          TokenLength[32];

    // Stream should have already been validated...
    assert(FileDescriptor.IsGood());

    // Probe for the photosensor diode band type...
    m_DiodeBandType = ProbeDiodeBandType(DiodeBandTypeHint);

        // Not a supported band type...
        if(m_DiodeBandType == Unknown)
        {
            // This was probably just an internal radiometric / geometric 
            //  calibration shot and not meant to be interesting...
            if(DiodeBandTypeHint.find("CAL") != string::npos)
                SetErrorAndReturn(
                    string(_("internal radio/geometric calibration (")) + 
                    DiodeBandTypeHint + 
                    string(")"))

            // Some other...
            else
                SetErrorAndReturn(string(_("unsupported photosensor diode band type (")) + 
                    DiodeBandTypeHint + 
                    string(")"))
        }

    // Extract the header record...
    const LogicalRecord HeaderRecord(FileDescriptor);

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
        {
            ParseBasicMetadataImplementation_Format1(HeaderRecord);
            m_BasicMetadataParserHeuristic = 1;
        }

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
        {
            ParseBasicMetadataImplementation_Format2(HeaderRecord);
            m_BasicMetadataParserHeuristic = 2;
        }

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
        {
            ParseBasicMetadataImplementation_Format5(HeaderRecord);
            m_BasicMetadataParserHeuristic = 5;
        }

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
        {
            ParseBasicMetadataImplementation_Format3(HeaderRecord);
            m_BasicMetadataParserHeuristic = 3;
        }

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
        {
            ParseBasicMetadataImplementation_Format2(HeaderRecord);
            m_BasicMetadataParserHeuristic = 2;
        }

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
        {
            ParseBasicMetadataImplementation_Format6(HeaderRecord);
            m_BasicMetadataParserHeuristic = 6;
        }

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
        {
            ParseBasicMetadataImplementation_Format1(HeaderRecord);
            m_BasicMetadataParserHeuristic = 1;
        }

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
        {
            ParseBasicMetadataImplementation_Format4(HeaderRecord);
            m_BasicMetadataParserHeuristic = 4;
        }

        // Unknown format...
        else
            SetErrorAndReturn(_("exhausted basic metadata parser heuristics"))

    if(m_DiodeBandType == Sun && m_OriginalWidth == 512)
        m_OriginalWidth += 2;

    // Perform sanity check on basic metadata...

        // Check bands...
        if(m_Bands != 1)
            SetErrorAndReturn(_("unsupported number of image bands"))

        // Check height...
        if(m_OriginalHeight <= 0 || m_OriginalHeight >= 99999)
            SetErrorAndReturn(_("corrupt image height"))

        // Check width...
        if(m_OriginalWidth <= 0 || m_OriginalWidth >= 99999)
            SetErrorAndReturn(_("corrupt image width"))

        // Check pixel format is integral...
        if(m_PixelFormat != 'I' && /* Definitely integral */
           m_PixelFormat != 'L')   /* Guessing integral */
            SetErrorAndReturn(_("unsupported pixel format"))

        // Check bytes per colour...
        if(m_BytesPerColour != 1)
            SetErrorAndReturn(_("unsupported colour bit depth"))

    // If verbosity is set, display basic metadata...
    Message(Console::Verbose) << _("basic metadata parser heuristic: ") << m_BasicMetadataParserHeuristic << endl;
    Message(Console::Verbose) << _("bands: ") << m_Bands << endl;
    Message(Console::Verbose) << _("height: ") << m_OriginalHeight << endl;
    Message(Console::Verbose) << _("width: ") << m_OriginalWidth << endl;
    Message(Console::Verbose) << _("raw band data size: ") << m_OriginalWidth * m_OriginalHeight * m_BytesPerColour << _(" bytes") << endl;
    Message(Console::Verbose) << _("file size: ") << GetFileSize() << _(" bytes") << endl;
    Message(Console::Verbose) << _("format: integral") << endl;
    Message(Console::Verbose) << _("bytes per colour: ") << m_BytesPerColour << endl;
    Message(Console::Verbose) << _("photosensor diode band type: ") << GetDiodeBandTypeFriendlyString() << endl;
    Message(Console::Verbose) << _("physical record size: ") << m_PhysicalRecordSize << hex << showbase << " (" << m_PhysicalRecordSize<< ")" << endl;
    Message(Console::Verbose) << _("possible physical record padding: ")  << dec << m_PhysicalRecordPadding << hex << showbase << " (" << m_PhysicalRecordPadding<< ")" << dec << endl;

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
    Message(Console::Verbose) << _("heuristics selected format 1 basic metadata parser") << endl;

    // Initialize a tokenizer, seeking passed two byte binary marker...
    stringstream Tokenizer(HeaderRecord.GetString(true, 2));

    // Extract number of image bands...
    Tokenizer >> m_Bands;
    
    // We don't know what this byte is for...
    Tokenizer >> DummyCharacter;

    // Extract image height...
    Tokenizer >> m_OriginalHeight;

    // Extract image width...
    Tokenizer >> m_OriginalWidth;

    // Calculate the physical record size which is either 5 logical 
    //  records or the image width, whichever is greater...

        // Image width is greater than 5 logical records...
        if(m_OriginalWidth > 5 * LOGICAL_RECORD_SIZE)
        {
            // Use the width...
            m_PhysicalRecordSize = m_OriginalWidth;

            // But anything passed the last logical record is just padding...
            m_PhysicalRecordPadding = m_OriginalWidth - (5 * LOGICAL_RECORD_SIZE);
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
    Message(Console::Verbose) << _("heuristics selected format 2 basic metadata parser") << endl;

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
    m_OriginalHeight = atoi(Buffer);
    
    // Let the second half be the width...
    Token.copy(Buffer, Token.length() - HeightLength, HeightLength);
    Buffer[Token.length() - HeightLength] = '\x0';
    m_OriginalWidth = atoi(Buffer);

    // Calculate the physical record size which is either 5 logical 
    //  records or the image width, whichever is greater...

        // Image width is greater than 5 logical records...
        if(m_OriginalWidth > 5 * LOGICAL_RECORD_SIZE)
        {
            // Use the width...
            m_PhysicalRecordSize = m_OriginalWidth;

            // But anything passed the last logical record is just padding...
            m_PhysicalRecordPadding = m_OriginalWidth - (5 * LOGICAL_RECORD_SIZE);
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
    Message(Console::Verbose) << _("heuristics selected format 3 basic metadata parser") << endl;

    // Initialize a tokenizer, seeking passed two byte binary marker...
    stringstream Tokenizer(HeaderRecord.GetString(true, 2));

    // Number of image bands implicitly one...
    m_Bands = 1;

    // Extract image height...
    Tokenizer >> m_OriginalHeight;

    // Next token is the height and width coallesced. Ignore...
    Tokenizer >> Token;
    
    // Extract width...
    Tokenizer >> m_OriginalWidth;

    // Calculate the physical record size which is either 5 logical 
    //  records or the image width, whichever is greater...

        // Image width is greater than 5 logical records...
        if(m_OriginalWidth > 5 * LOGICAL_RECORD_SIZE)
        {
            // Use the width...
            m_PhysicalRecordSize = m_OriginalWidth;

            // But anything passed the last logical record is just padding...
            m_PhysicalRecordPadding = m_OriginalWidth - (5 * LOGICAL_RECORD_SIZE);
        }
        
        // Image width less than five logical records, meaning physical records
        //  are exactly five logical records...
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
    Message(Console::Verbose) << _("heuristics selected format 4 basic metadata parser") << endl;

    // Initialize a tokenizer, seeking passed two byte binary marker...
    stringstream Tokenizer(HeaderRecord.GetString(true, 2));

    // Number of image bands implicitly one...
    m_Bands = 1;

    // Extract image height...
    Tokenizer >> m_OriginalHeight;
    
    // Extract width...
    Tokenizer >> m_OriginalWidth;
    
    // Next two are height and width again, skip...
    Tokenizer >> Token;
    Tokenizer >> Token;

    // Calculate the physical record size which is either 5 logical 
    //  records or the image width, whichever is greater...

        // Image width is greater than 5 logical records...
        if(m_OriginalWidth > 5 * LOGICAL_RECORD_SIZE)
        {
            // Use the width...
            m_PhysicalRecordSize = m_OriginalWidth;

            // But anything passed the last logical record is just padding...
            m_PhysicalRecordPadding = m_OriginalWidth - (5 * LOGICAL_RECORD_SIZE);
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
    Message(Console::Verbose) << _("heuristics selected format 5 basic metadata parser") << endl;

    // Initialize a tokenizer, seeking passed two byte binary marker...
    stringstream Tokenizer(HeaderRecord.GetString(true, 2));

    // Number of image bands implicitly one...
    m_Bands = 1;

    // Extract image height...
    Tokenizer >> m_OriginalHeight;

    // Extract width...
    Tokenizer >> m_OriginalWidth;

    // Next token is the height and width coallesced. Ignore...
    Tokenizer >> Token;

    // Calculate the physical record size which is either 5 logical 
    //  records or the image width, whichever is greater...

        // Image width is greater than 5 logical records...
        if(m_OriginalWidth > 5 * LOGICAL_RECORD_SIZE)
        {
            // Use the width...
            m_PhysicalRecordSize = m_OriginalWidth;

            // But anything passed the last logical record is just padding...
            m_PhysicalRecordPadding = m_OriginalWidth - (5 * LOGICAL_RECORD_SIZE);
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
    Message(Console::Verbose) << _("heuristics selected format 6 basic metadata parser") << endl;

    // Initialize a tokenizer, seeking passed two byte binary marker...
    stringstream Tokenizer(HeaderRecord.GetString(true, 2));

    // Number of image bands implicitly one...
    m_Bands = 1;

    // Extract image height...
    Tokenizer >> Token;
    const size_t HeightLength = Token.length();
    m_OriginalHeight = atoi(Token.c_str());

    // Next token is the width, height, and width again coallesced...
    Tokenizer >> Token;
    
    // The height we can calculate because it is flanked by the width...
    Token.copy(Buffer, (Token.length() - HeightLength) / 2, 0);
    Buffer[(Token.length() - HeightLength) / 2] = '\x0';
    m_OriginalWidth = atoi(Buffer);

    // Calculate the physical record size which is either 5 logical 
    //  records or the image width, whichever is greater...

        // Image width is greater than 5 logical records...
        if(m_OriginalWidth > 5 * LOGICAL_RECORD_SIZE)
        {
            // Use the width...
            m_PhysicalRecordSize = m_OriginalWidth;

            // But anything passed the last logical record is just padding...
            m_PhysicalRecordPadding = m_OriginalWidth - (5 * LOGICAL_RECORD_SIZE);
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
void VicarImageBand::ParseExtendedMetadata(
    const LogicalRecord &Record, 
    const size_t LocalLogicalRecordIndexHint)
{
    // Variables...
    string Token;
    
    // Is it valid?
    if(!Record.IsValidLabel())
        SetErrorAndReturn(_("invalid logical record label while parsing extended metadata"))

    // Get the record as a string...
    const string RecordString = Record;

    // Initialize tokenizer...
    stringstream Tokenizer(RecordString);

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
            Message(Console::Verbose) << _("psa directional vector: ") << m_AzimuthElevation << endl;
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
                Tokenizer >> Token;
                SetCameraEventLabel(Token);
                Message(Console::Verbose) << _("camera event label: ") << m_CameraEventLabel << endl;
            }

            // Not a camera event, restore the token...
            else
                Tokenizer << Token;
        }
        
        // Possibly lander number, and always in the second local
        //  logical record if present at all...
        if(LocalLogicalRecordIndexHint == 1 && Token == "VIKING")
        {
            // Check...
            Tokenizer >> Token;
            
            // Confirmed...
            if(Token == "LANDER")
            {
                // Store the lander number...
                Tokenizer >> m_LanderNumber;

                // Check validity...
                if(m_LanderNumber > 2)
                    Message(Console::Warning) << _("bad lander number") << endl;

                // Be verbose if requested...
                Message(Console::Verbose) 
                    << _("lander number: ") << m_LanderNumber 
                    << " (" << GetLanderLocation() << ")" << endl;

                // Check for matching lander number, if user filtered...
                if(Options::GetInstance().GetFilterLander() != 0 &&
                   Options::GetInstance().GetFilterLander() != m_LanderNumber)
                    SetErrorAndReturn(_("filtering non-matching lander"))
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
VicarImageBand::PSADiode VicarImageBand::ProbeDiodeBandType(
    string &DiodeBandTypeHint) const
{
    // Open the file...
    ZZipFileDescriptor FileDescriptor(Open());
    
        // Should have already been openable, since we did so in Load()...
        assert(FileDescriptor.IsGood());

    // Setup caller's default return value...
    DiodeBandTypeHint = "unknown";

    // Account for any required phase offset...
    if(zzip_seek(FileDescriptor, m_PhaseOffsetRequired, SEEK_SET) == -1)
        return Unknown;

    // Check anywhere within the first physical record...
    for(size_t LogicalRecordIndex = 0; LogicalRecordIndex < 5; ++LogicalRecordIndex)
    {
        // Variables...
        string  PreviousToken;
        string  CurrentToken;

        // Extract record...
        LogicalRecord Record(FileDescriptor);

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
                DiodeBandTypeHint = _("monocolour unsupported");
                return Unknown;            
            }

            // Some kind of broad band diode that may or may not identify itself...
            if(CurrentToken == "BROADBAND")
            {
                // Set caller hint and return unsupported...
                DiodeBandTypeHint = _("unidentifiable broadband");
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
    DiodeBandTypeHint = _("none detected");
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

// Set the camera event label, along with the solar day and camera 
//  event identifier without the solar day...
void VicarImageBand::SetCameraEventLabel(const string &CameraEventLabel)
{
    // Store the camera event label...
    m_CameraEventLabel = CameraEventLabel;

    // Extract solar day out of camera event label...
    const size_t SolarDayOffset = m_CameraEventLabel.find_last_of("/\\");
    
    // Should always have a sol separator...
    assert(SolarDayOffset != string::npos && (SolarDayOffset + 1 < m_CameraEventLabel.length()));
    
    // Get the identifier without the solar day out of the label...
    m_CameraEventLabelNoSol.assign(m_CameraEventLabel, 0, SolarDayOffset);

    // Get the solar day...
        
        // Store whole thing as a string...
        string SolarDay;
        SolarDay.assign(m_CameraEventLabel, SolarDayOffset + 1, 4);

        // Convert to integer...
        m_SolarDay = atoi(SolarDay.c_str());

    // Check for matching solar day, if user filtered...
    if(Options::GetInstance().GetFilterSolarDay() != numeric_limits<size_t>::max() && 
       Options::GetInstance().GetFilterSolarDay() != m_SolarDay)
        SetErrorAndReturn(_("filtering non-matching solar day"))

    // Check for matching camera event, if user filtered...
    if(!Options::GetInstance().GetFilterCameraEvent().empty() && 
       Options::GetInstance().GetFilterCameraEvent() != m_CameraEventLabelNoSol)
        SetErrorAndReturn(_("filtering non-matching camera event"))
}

// Mirror the band data from left to right...
void VicarImageBand::MirrorLeftRight(
    const RawBandDataType &RawBandData, 
    RawBandDataType &TransformedRawBandData)
{
    // Make an editable copy of the band data...
    TransformedRawBandData = RawBandData;

    // Get height...
    const size_t Height = RawBandData.size();
    
    // Reverse each scanline...
    for(size_t CurrentRow = 0; CurrentRow < Height; ++CurrentRow)
        reverse(TransformedRawBandData[CurrentRow].begin(), TransformedRawBandData[CurrentRow].end());
}

// Mirror the top and bottom...
void VicarImageBand::MirrorTopBottom(
    const RawBandDataType &RawBandData, 
    RawBandDataType &TransformedRawBandData)
{
    // Make a writable copy of the band data...
    TransformedRawBandData = RawBandData;

    // Reverse from top to bottom...
    for(size_t Upper = 0, Bottom = RawBandData.size() - 1; Upper < Bottom; ++Upper, --Bottom)
        TransformedRawBandData[Upper].swap(TransformedRawBandData[Bottom]);
}

// Mirror diagonaly...
void VicarImageBand::MirrorDiagonal(
    const RawBandDataType &RawBandData, 
    RawBandDataType &TransformedRawBandData)
{
    // Make a writable copy of the band data...
    TransformedRawBandData = RawBandData;

    // Remember the original height and width before transforming...
    const size_t OldHeight = TransformedRawBandData.size();
    const size_t OldWidth  = TransformedRawBandData.at(0).size();

    // Get the larger of either the height or the width...
    const size_t LargerDimension = max(OldHeight, OldWidth);

    // Resize image dimensions...

        // Height less than width... (wide)
        if(OldHeight < LargerDimension)
        {
            // Make tall enough since new height will be what the width was...
            TransformedRawBandData.resize(LargerDimension);

            // Make each row (formally column) large enough...
            for(size_t CurrentRow = OldHeight; CurrentRow < LargerDimension; ++CurrentRow)
                TransformedRawBandData.at(CurrentRow).resize(LargerDimension);
        }

        // Width less than height... (narrow)
        else if(OldWidth < LargerDimension)
        {
            // Make each row large enough...
            for(size_t CurrentRow = 0; CurrentRow < OldHeight; ++CurrentRow)
                TransformedRawBandData.at(CurrentRow).resize(LargerDimension);
        }

    // Mirror the data along the diagonal...
    for(size_t CurrentRow = 0; CurrentRow < LargerDimension; ++CurrentRow)
    {
        // Grab the current row's data...
        vector<uint8_t> &CurrentRowData = TransformedRawBandData.at(CurrentRow);
        
        // Perform the pixel swap...
        for(size_t CurrentColumn = 0; CurrentColumn < CurrentRow; ++CurrentColumn)
            swap(CurrentRowData.at(CurrentColumn), TransformedRawBandData.at(CurrentColumn).at(CurrentRow));
    }

    // Since it was mirrored over the diagonal, the width becomes the height 
    //  and vise versa...
    const size_t NewHeight  = OldWidth;
    const size_t NewWidth   = OldHeight;

    // Trim excess height, if necessary...
    if(NewHeight < LargerDimension)
        TransformedRawBandData.resize(NewHeight);

    // Trim excess width, if necessary...
    else if(NewWidth < LargerDimension)
    {
        for(size_t CurrentRow = 0; CurrentRow < NewHeight; ++CurrentRow)
            TransformedRawBandData.at(CurrentRow).resize(NewWidth);
    }
}

// Rotate image band data as requested...
void VicarImageBand::Rotate(
    const RotationType Rotation, 
    const RawBandDataType &RawBandData, 
    RawBandDataType &TransformedRawBandData)
{
    // Bounds check...
    assert(Rotation == None || Rotation == Rotate90 || 
           Rotation == Rotate180 || Rotation == Rotate270);
    assert(!RawBandData.empty());
    assert(RawBandData != TransformedRawBandData);

    // No rotation to perform...
    if(Rotation == None)
        TransformedRawBandData = RawBandData;

    // Rotate counterclockwise 90...
    else if(Rotation == Rotate90)
    {
        MirrorDiagonal(RawBandData, TransformedRawBandData); 
        MirrorTopBottom(TransformedRawBandData, TransformedRawBandData);
    }
    
    // Rotate counterclockwise 180...
    else if(Rotation == Rotate180)
    {
        MirrorLeftRight(RawBandData, TransformedRawBandData); 
        MirrorTopBottom(TransformedRawBandData, TransformedRawBandData);
    }
    
    // Rotate counterclockwise 270...
    else if(Rotation == Rotate270)
    {
        MirrorDiagonal(RawBandData, TransformedRawBandData);
        MirrorLeftRight(TransformedRawBandData, TransformedRawBandData);
    }
}


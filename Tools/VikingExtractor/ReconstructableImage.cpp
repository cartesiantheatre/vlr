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
#include "ReconstructableImage.h"
#include "Miscellaneous.h"
#include "Console.h"
#include <cassert>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <climits>
#include <png++/png.hpp>

// Using the standard namespace...
using namespace std;

// Constructor...
ReconstructableImage::ReconstructableImage(
    const std::string &OutputRootDirectory, 
    const std::string &CameraEventLabel)
    : m_OutputRootDirectory(OutputRootDirectory),
      m_CameraEventLabel(CameraEventLabel),
      m_SolarDay(0)
{
    // Always need an label...
    assert(!CameraEventLabel.empty());

    // Make sure output root directory ends with a path separator...
    if(m_OutputRootDirectory.find_last_of("/\\") != m_OutputRootDirectory.length() - 1)
        m_OutputRootDirectory += '/';

    // Extract solar day out of camera event label...
    const size_t SolarDayOffset = m_CameraEventLabel.find_last_of("/\\");
    
    // Should always have a sol separator...
    assert(SolarDayOffset != string::npos && (SolarDayOffset + 1 < m_CameraEventLabel.length()));
    
    // Get the identifier without the solar day out of the label...
    m_CameraEventNoSol.assign(m_CameraEventLabel, 0, SolarDayOffset);
    
    // Get the solar day...
        
        // Store whole thing as a string...
        string SolarDay;
        SolarDay.assign(m_CameraEventLabel, SolarDayOffset + 1, 4);

        // Convert to integer...
        m_SolarDay = atoi(SolarDay.c_str());
}

// Add an image band...
void ReconstructableImage::AddImageBand(const VicarImageBand &ImageBand)
{
    // Make sure this is for the right event...
    assert(ImageBand.GetCameraEventLabel() == m_CameraEventLabel);

    // Add to the appropriate band type list...
    switch(ImageBand.GetDiodeBandType())
    {
        // Red...
        case VicarImageBand::Red: 
            m_RedImageBandList.push_back(ImageBand);
            break;
        
        // Green...
        case VicarImageBand::Green: 
            m_GreenImageBandList.push_back(ImageBand);
            break;
        
        // Blue...
        case VicarImageBand::Blue: 
            m_BlueImageBandList.push_back(ImageBand);
            break;
        
        // Infrared 1...
        case VicarImageBand::Infrared1:
            m_Infrared1ImageBandList.push_back(ImageBand);
            break;
        
        // Infrared 2...
        case VicarImageBand::Infrared2:
            m_Infrared2ImageBandList.push_back(ImageBand);
            break;
        
        // Infrared 3...
        case VicarImageBand::Infrared3:
            m_Infrared3ImageBandList.push_back(ImageBand);
            break;
        
        // Grayscale...
        case VicarImageBand::Sun:
        case VicarImageBand::Survey:
            m_GrayImageBandList.push_back(ImageBand);
            break;

        // Unknown... (shouldn't ever happen, but here for completeness)
        default:
            SetErrorAndReturn("cannot reconstruct image from unsupported diode band type");
    }
}

// Create the necessary path to the output file and return a path, 
//  placing the output file within an option subdirectory and appending
//  an optional name suffix e.g. file_suffix.png...
string ReconstructableImage::CreateOutputFileName()
{

    // Will contain the full directory, not including the file name...
    stringstream FullDirectory;
    FullDirectory << m_OutputRootDirectory;

    // Images are reconstructed in subfolder of solar day it was 
    //  taken on, so create the subfolder, if enabled......
    if(Options::GetInstance().GetSolDirectorize())
        FullDirectory << m_SolarDay << '/';

    /* Otherwise put inside camera event identifier folder...
    else
        FullDirectory << m_CameraEventNoSol << '/';*/

    // Create and check for error...
    if(!CreateDirectoryRecursively(FullDirectory.str()))
    {
        // Set the error message and abort...
        SetErrorMessage("could not create output subdirectory for solar day");
        return string();
    }

    // Return the full path to a file ready to be written to...
    return FullDirectory.str() + m_CameraEventNoSol + ".png";
}

// Create the necessary path to a single component of an image, 
//  distinguished with a name suffix and ordinal. These end up in the
//  Unreconstructable directory...
string ReconstructableImage::CreateUnreconstructableOutputFileName(
    const string &BandType, const size_t Ordinal)
{

    // Will contain the full directory, not including the file name...
    stringstream FullDirectory;
    FullDirectory << m_OutputRootDirectory;

    // Unreconstructables go in the Incomplete directory...
    FullDirectory 
        << "Unreconstructable/"
        << m_CameraEventNoSol 
        << "/";

    // Create and check for error...
    if(!CreateDirectoryRecursively(FullDirectory.str()))
    {
        // Set the error message and abort...
        SetErrorMessage("could not create output subdirectory for solar day");
        return string();
    }

    // Now append the file name...
    FullDirectory
        << BandType
        << "_" << Ordinal
        << ".png";

    // Return the full path to a file ready to be written to...
    return FullDirectory.str();
}

// Dump all images within given image band to the output directory in 
//  a subdirectory Incomplete under the camera event identifier...
bool ReconstructableImage::DumpUnreconstructable(
    ImageBandListType &ImageBandList, const string &BandTypeSuffix)
{
    // Dump all images within this image band...
    for(ImageBandListIterator Iterator = ImageBandList.begin(); 
        Iterator != ImageBandList.end(); 
      ++Iterator)
    {
        // Get the image band...
        VicarImageBand &ImageBand = *Iterator;

        // Format suffix to contain sort order identifier to distinguish
        //  from other images of this same band type of this same camera 
        //  event...
        const string FullDirectory = CreateUnreconstructableOutputFileName(
            BandTypeSuffix, 
            Iterator - ImageBandList.begin());
        
        // Dump the single channel as grayscale...
        ReconstructGrayscaleImage(FullDirectory, ImageBand);
    }
    
    // Done...
    return true;
}

// Extract the image out as a PNG, or return false if failed...
bool ReconstructableImage::Reconstruct()
{
    // Sort each band lists from lowest to best quality...
    sort(m_RedImageBandList.begin(),        m_RedImageBandList.end());
    sort(m_GreenImageBandList.begin(),      m_GreenImageBandList.end());
    sort(m_BlueImageBandList.begin(),       m_BlueImageBandList.end());
    sort(m_Infrared1ImageBandList.begin(),  m_Infrared1ImageBandList.end());
    sort(m_Infrared2ImageBandList.begin(),  m_Infrared2ImageBandList.end());
    sort(m_Infrared3ImageBandList.begin(),  m_Infrared3ImageBandList.end());
    sort(m_GrayImageBandList.begin(),       m_GrayImageBandList.end());

    // Setup shorthand sizes to image band vectors...
    const size_t Reds       = m_RedImageBandList.size();
    const size_t Greens     = m_GreenImageBandList.size();
    const size_t Blues      = m_BlueImageBandList.size();
    const size_t Infrareds1 = m_Infrared1ImageBandList.size();
    const size_t Infrareds2 = m_Infrared2ImageBandList.size();
    const size_t Infrareds3 = m_Infrared3ImageBandList.size();
    const size_t Grays      = m_GrayImageBandList.size();

// Colour image... (only all colour bands present)
if((Reds == 1 && Greens == 1 && Blues == 1) && 
   (Infrareds1 + Infrareds2 + Infrareds3 + Grays == 0))
{
    // Create full path to output file and create containing 
    // directory, if necessary...
    const string OutputFileName = CreateOutputFileName();

        // Failed...
        if(IsError())
            return false;

    // Attempt to reconstruct...
    return ReconstructColourImage(
            OutputFileName, 
            m_RedImageBandList.back(), 
            m_GreenImageBandList.back(), 
            m_BlueImageBandList.back());
}

    /* Colour image... (only all colour bands present)
    if((min3(Reds, Greens, Blues) >= 1) && 
       (Infrareds1 + Infrareds2 + Infrareds3 + Grays == 0))
    {
        // Create full path to output file and create containing 
        // directory, if necessary...
        const string OutputFileName = CreateOutputFileName();

            // Failed...
            if(IsError())
                return false;

        // Attempt to reconstruct...
        return ReconstructColourImage(
                OutputFileName, 
                m_RedImageBandList.back(), 
                m_GreenImageBandList.back(), 
                m_BlueImageBandList.back());
    }*/

    /* Infrared image... (only all infrared bands present)
    else if((Reds + Greens + Blues + Grays == 0) && 
            (min(Infrareds1, Infrareds2, Infrareds3) >= 1))
    {
        Message(Console::Info) << "reconstructed infrared image successfully" << endl;
    }
    
    // Grayscale image... (only grayscale image bands)
    else if((Reds + Greens + Blues + Infrareds1 + Infrareds2 + Infrareds3 == 0) &&
            Grays >= 1)
    {
        Message(Console::Info) << "reconstructed grayscale image successfully" << endl;
    }*/
    
    // Unknown...
    else
    {
        // Dump...
        DumpUnreconstructable(m_RedImageBandList, "red");
        DumpUnreconstructable(m_GreenImageBandList, "green");
        DumpUnreconstructable(m_BlueImageBandList, "blue");
        DumpUnreconstructable(m_Infrared1ImageBandList, "ir1");
        DumpUnreconstructable(m_Infrared2ImageBandList, "ir2");
        DumpUnreconstructable(m_Infrared3ImageBandList, "ir3");
        DumpUnreconstructable(m_GrayImageBandList, "gray");

        // This doesn't count as a successful reconstruction since it wasn't reassembled...
        SetErrorAndReturnFalse("cannot reconstruct, dumped all bands");
        return false;
    }
}

// Reconstruct a colour image from requested image bands which can be NULL...
bool ReconstructableImage::ReconstructColourImage(
    const string &OutputFileName, 
    VicarImageBand &BestRedImageBand, 
    VicarImageBand &BestGreenImageBand, 
    VicarImageBand &BestBlueImageBand)
{
    // Set file name for console messages to begin with...
    Console::GetInstance().SetCurrentFileName(OutputFileName);

    // Overwrite not enabled and file already existed, don't overwrite...
    if(!Options::GetInstance().GetOverwrite() && 
       (access(OutputFileName.c_str(), F_OK) == 0))
        SetErrorAndReturnFalse("output already exists, not overwriting (use --overwrite to override)");

    // Raw band data...
    VicarImageBand::RawBandDataType RedRawBandData;
    VicarImageBand::RawBandDataType GreenRawBandData;
    VicarImageBand::RawBandDataType BlueRawBandData;
    
    // Get the raw band data of each colour band...
    
        // Red...

            // Get the raw band data and check for error...
            if(!BestRedImageBand.GetRawBandData(RedRawBandData))
                SetErrorAndReturnFalse(BestRedImageBand.GetErrorMessage());

            // Get width and height...
            const size_t RedWidth  = BestRedImageBand.GetTransformedWidth();
            const size_t RedHeight = BestRedImageBand.GetTransformedHeight();
        
        // Green...

            // Get the raw band data and check for error...
            if(!BestGreenImageBand.GetRawBandData(GreenRawBandData))
                SetErrorAndReturnFalse(BestGreenImageBand.GetErrorMessage());

            // Get width and height...
            const size_t GreenWidth  = BestGreenImageBand.GetTransformedWidth();
            const size_t GreenHeight = BestGreenImageBand.GetTransformedHeight();

        // Blue...

            // Initialize extraction stream and check for error...
            if(!BestBlueImageBand.GetRawBandData(BlueRawBandData))
                SetErrorAndReturnFalse(BestBlueImageBand.GetErrorMessage());

            // Get width and height...
            const size_t BlueWidth  = BestBlueImageBand.GetTransformedWidth();
            const size_t BlueHeight = BestBlueImageBand.GetTransformedHeight();

    // If the widths don't match or the heights don't match, we have a problem...
    if((min3(RedWidth, GreenWidth, BlueWidth) != max3(RedWidth, GreenWidth, BlueWidth)) ||
       (min3(RedHeight, GreenHeight, BlueHeight) != max3(RedHeight, GreenHeight, BlueHeight)))
        SetErrorAndReturnFalse("image bands not all the same size, may be missing scanlines");

    // Get the final image width and height...
    const size_t Width  = RedWidth;
    const size_t Height = RedHeight;

    // Allocate png storage...
    png::image<png::rgb_pixel> PngImage(Width, Height);

    // Toggle interlacing, if user selected...
    if(Options::GetInstance().GetInterlace())
        PngImage.set_interlace_type(png::interlace_adam7);
    else
        PngImage.set_interlace_type(png::interlace_none);

    // Pass raw image data through encoder, row by row...
    for(size_t Y = 0; Y < PngImage.get_height(); ++Y)
    {
        // Pass raw image data through encoder, column by column...
        for(size_t X = 0; X < PngImage.get_width(); ++X)
        {
            // Storage for this pixel's colours...
            const char RedByte    = RedRawBandData.at(Y).at(X);
            const char GreenByte  = GreenRawBandData.at(Y).at(X);
            const char BlueByte   = BlueRawBandData.at(Y).at(X);

            // Encode...
            PngImage.set_pixel(X, Y, png::rgb_pixel(RedByte, GreenByte, BlueByte));
        }
    }

    // Write out...
    PngImage.write(OutputFileName);

    // Done...
    return true;
}

// Reconstruct a grayscale image from requested image band...
bool ReconstructableImage::ReconstructGrayscaleImage(
    const string &OutputFileName, 
    VicarImageBand &BestGrayscaleImageBand)
{
    // Set file name for console messages to begin with...
    Console::GetInstance().SetCurrentFileName(OutputFileName);

    // Overwrite not enabled and file already existed, don't overwrite...
    if(!Options::GetInstance().GetOverwrite() && 
       (access(OutputFileName.c_str(), F_OK) == 0))
        SetErrorAndReturnFalse("output already exists, not overwriting (use --overwrite to override)");

    // Extraction raw band data...
    VicarImageBand::RawBandDataType RawBandData;

    // Get the raw band data and check for error...
    if(!BestGrayscaleImageBand.GetRawBandData(RawBandData))
        SetErrorAndReturnFalse(BestGrayscaleImageBand.GetErrorMessage());

    // Get width and height...
    const int Width   = BestGrayscaleImageBand.GetTransformedWidth();
    const int Height  = BestGrayscaleImageBand.GetTransformedHeight();

    // Allocate png storage...
    png::image<png::gray_pixel> PngImage(Width, Height);

    // Toggle interlacing, if user selected...
    if(Options::GetInstance().GetInterlace())
        PngImage.set_interlace_type(png::interlace_adam7);
    else
        PngImage.set_interlace_type(png::interlace_none);

    // Pass raw image data through encoder, row by row...
    for(size_t Y = 0; Y < PngImage.get_height(); ++Y)
    {
        // Pass raw image data through encoder, column by column...
        for(size_t X = 0; X < PngImage.get_width(); ++X)
        {
            // Get this pixel's grayscale value...
            char GrayByte = RawBandData.at(Y).at(X);

            // Encode...
            PngImage.set_pixel(X, Y, GrayByte);
        }
    }
    
    // Write out...
    PngImage.write(OutputFileName);

    // Done...
    return true;
}


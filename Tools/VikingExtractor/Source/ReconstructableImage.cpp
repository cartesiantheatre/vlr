/*
    VikingExtractor, to recover images from Viking Lander operations.
    Copyright (C) 2010, 2011, 2012 Kshatra Corp <kip@thevertigo.com>.
    
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
      m_DumpedImagesCount(0),
      m_LanderNumber(0),
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

    // If this image knows the lander number, remember it...
    if(ImageBand.GetLanderNumber())
        m_LanderNumber = ImageBand.GetLanderNumber();

    // Remember the month it was taken on...
    m_Month = ImageBand.GetMonth();

    // Add to the appropriate band type list...
    switch(ImageBand.GetDiodeBandType())
    {
        // Red...
        case VicarImageBand::Red: 
            m_RedImageBandList.push_back(ImageBand);
            m_BandTypeClass = "Colour";
            break;
        
        // Green...
        case VicarImageBand::Green: 
            m_GreenImageBandList.push_back(ImageBand);
            m_BandTypeClass = "Colour";
            break;
        
        // Blue...
        case VicarImageBand::Blue: 
            m_BlueImageBandList.push_back(ImageBand);
            m_BandTypeClass = "Colour";
            break;
        
        // Infrared 1...
        case VicarImageBand::Infrared1:
            m_Infrared1ImageBandList.push_back(ImageBand);
            m_BandTypeClass = "Infrared";
            break;
        
        // Infrared 2...
        case VicarImageBand::Infrared2:
            m_Infrared2ImageBandList.push_back(ImageBand);
            m_BandTypeClass = "Infrared";
            break;
        
        // Infrared 3...
        case VicarImageBand::Infrared3:
            m_Infrared3ImageBandList.push_back(ImageBand);
            m_BandTypeClass = "Infrared";
            break;
        
        // Sun...
        case VicarImageBand::Sun:
            m_GrayImageBandList.push_back(ImageBand);
            m_BandTypeClass = "Sun";
            break;

        // Survey...
        case VicarImageBand::Survey:
            m_GrayImageBandList.push_back(ImageBand);
            m_BandTypeClass = "Survey";
            break;

        // Unknown... (shouldn't ever happen, but here for completeness)
        default:
            SetErrorAndReturn("cannot reconstruct image from unsupported diode band type");
    }
}

// Create the necessary path to the output file and return a path. 
//  The file will have the provided file extension, and if it is
//  not a reconstructable image, then it will use specified name
//  preceding the file extension...
string ReconstructableImage::CreateOutputFileName(
    const bool Unreconstructable,
    const string &Extension,
    const string &UnreconstructableName)
{

    // Will contain the full directory, not including the file name...
    stringstream FullDirectory;
    FullDirectory << m_OutputRootDirectory;

    // If this isn't part of a reconstructable image, prefix...
    if(Unreconstructable)
        FullDirectory << "Unreconstructable/";

    // Images are reconstructed in subfolder of location taken in if enabled...
    if(Options::GetInstance().GetDirectorizeLocation())
    {
        // Location determined by lander image was taken from...
        switch(m_LanderNumber)
        {
            // Chryse Planitia...
            case 1:
                FullDirectory << "Chryse Planitia/";
                break;
            
            // Utopia Planitia...
            case 2:
                FullDirectory << "Utopia Planitia/";
                break;

            // Unknown...
            default:
                FullDirectory << "Location Unknown/"; 
                break; 
        }
    }

    // Images are reconstructed in subfolder of month if enabled...
    if(Options::GetInstance().GetDirectorizeMonth() && !m_Month.empty())
        FullDirectory << m_Month << '/';

    // Images are reconstructed in subfolder of band type class if enabled...
    if(Options::GetInstance().GetDirectorizeBandTypeClass() && !m_BandTypeClass.empty())
        FullDirectory << m_BandTypeClass << '/';

    // Images are reconstructed in subfolder of solar day it was 
    //  taken on, so create the subfolder, if enabled...
    if(Options::GetInstance().GetDirectorizeSol())
        FullDirectory << m_SolarDay << '/';

    // Put inside of directory for this camera event, but only if it's 
    //  part of an unreconstructable image set, otherwise file name is
    //  unique enough to distinguish it...
    if(Unreconstructable)
        FullDirectory << m_CameraEventNoSol << '/';

    // Create and check for error, if not a dry run...
    if(!Options::GetInstance().GetDryRun() && 
       !CreateDirectoryRecursively(FullDirectory.str()))
    {
        // Set the error message and abort...
        SetErrorMessage("could not create output directory for file");
        return string();
    }

    // Compose the file name portion...
    
        // Image is a reconstructed image, only needing a name of the 
        //  camera event without the solar day...
        if(!Unreconstructable)
            FullDirectory << m_CameraEventNoSol;
        
        // Otherwise, if image is unreconstructable...
        else
        {
            // The unreconstructable's name should always have been provided...
            assert(!UnreconstructableName.empty());
            
            // Use it...
            FullDirectory << UnreconstructableName;
        }
        
        // Add the file name extension...
        FullDirectory << "." + Extension;

    // Return the full path to a file ready to be written to...
    return FullDirectory.str();
}

// Dump all images within given image band to the output directory in 
//  a subdirectory Unreconstructable under the camera event identifier...
bool ReconstructableImage::DumpUnreconstructable(ImageBandListType &ImageBandList)
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
        stringstream UnreconstructableName;
        UnreconstructableName 
            << ImageBand.GetDiodeBandTypeFriendlyString()
            << "_"
            << Iterator - ImageBandList.begin();

        // Create the path to the file...
        const string FullDirectory = CreateOutputFileName(true, "png", UnreconstructableName.str());
        
        // Dump the single channel as grayscale...
        if(ReconstructGrayscaleImage(FullDirectory, ImageBand))
        {
            // Remember this...
          ++m_DumpedImagesCount;

            // Saved successfully, now if saving metadata is 
            //  enabled, dump it...
            if(Options::GetInstance().GetSaveMetadata())
                SaveMetadata(CreateOutputFileName(true, "txt", UnreconstructableName.str()), ImageBandList);
        }
    }
    
    // Done...
    return true;
}

// Find the best image in the band list with a full histogram, 
//  or return rend iterator...
ReconstructableImage::ImageBandListReverseIterator 
ReconstructableImage::FindBestImageBandWithFullHistogram(
    ImageBandListType &ImageBandList, 
    ImageBandListReverseIterator &ReverseIterator) const
{
    // Scan backwards, starting with best quality image...
    for(ImageBandListReverseIterator Current = ReverseIterator;
        Current != ImageBandList.rend();
      ++Current)
    {
        // Get the current image band...
        const VicarImageBand &ImageBand = *Current;
        
        // This one has only an axis and no full histogram...
        if(ImageBand.IsFullHistogramPresent())
            return Current;
    }
    
    // Nothing found, return rend...
    return ImageBandList.rend();
}

// Find the best image in the band list with no axis, but just 
//  vanilla image, or return rend iterator...
ReconstructableImage::ImageBandListReverseIterator 
ReconstructableImage::FindBestImageBandWithNoAxis(
    ImageBandListType &ImageBandList, 
    ImageBandListReverseIterator &ReverseIterator) const
{
    // Scan backwards, starting with best quality image...
    for(ImageBandListReverseIterator Current = ReverseIterator;
        Current != ImageBandList.rend();
      ++Current)
    {
        // Get the current image band...
        const VicarImageBand &ImageBand = *Current;
        
        // This one has no axis present, use...
        if(!ImageBand.IsAxisPresent())
            return Current;
    }
    
    // Nothing found, return rend...
    return ImageBandList.rend();
}

// Extract the image out as a PNG, or return false if failed...
bool ReconstructableImage::Reconstruct()
{
    // Sort each band lists from lowest to best quality. Note that this
    //  does not necessarily mean that the best image of each band list
    //  will form the best matching set, since the best of one band list
    //  might have a full histogram and another might not...
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

    // We haven't dumped any images until we do so directed from in here...
    m_DumpedImagesCount = 0;

    // Colour image... (only all colour bands present)
    if(!Options::GetInstance().GetNoReconstruct() &&
       (min3(Reds, Greens, Blues) >= 1) && 
       (Infrareds1 + Infrareds2 + Infrareds3 + Grays == 0))
    {
        // Create full path to output file and create containing 
        // directory, if necessary...
        const string OutputFileName = CreateOutputFileName(false, "png");

            // Failed...
            if(IsError())
                return false;

        // Attempt to reconstruct...
        return ReconstructColourImage(
                OutputFileName, 
                m_RedImageBandList, 
                m_GreenImageBandList, 
                m_BlueImageBandList);
    }

    // Grayscale image... (only grayscale image bands)
    else if(!Options::GetInstance().GetNoReconstruct() &&
            (Reds + Greens + Blues + Infrareds1 + Infrareds2 + Infrareds3 == 0) &&
            Grays >= 1)
    {
        // Create full path to output file and create containing 
        // directory, if necessary...
        const string OutputFileName = CreateOutputFileName(false, "png");

            // Failed...
            if(IsError())
                return false;

        // Save best...
        return ReconstructGrayscaleImage(
            OutputFileName, 
            m_GrayImageBandList.back());
    }

    /* Infrared image... (only all infrared bands present)
    else if(!Options::GetInstance().GetNoReconstruct() &&
            (Reds + Greens + Blues + Grays == 0) && 
            (min(Infrareds1, Infrareds2, Infrareds3) >= 1))
    {
        Message(Console::Info) << "reconstructed infrared image successfully" << endl;
    }*/

    // Unknown...
    else
    {
        // Dump...
        DumpUnreconstructable(m_RedImageBandList);
        DumpUnreconstructable(m_GreenImageBandList);
        DumpUnreconstructable(m_BlueImageBandList);
        DumpUnreconstructable(m_Infrared1ImageBandList);
        DumpUnreconstructable(m_Infrared2ImageBandList);
        DumpUnreconstructable(m_Infrared3ImageBandList);
        DumpUnreconstructable(m_GrayImageBandList);

        // This doesn't count as a successful reconstruction since it wasn't reassembled...
        if(!Options::GetInstance().GetNoReconstruct())
            SetErrorAndReturnFalse("cannot reconstruct, dumped all bands");
        return false;
    }
}

// Reconstruct a colour image from requested image bands...
bool ReconstructableImage::ReconstructColourImage(
    const string &OutputFileName, 
    ImageBandListType &RedImageBandList, 
    ImageBandListType &GreenImageBandList, 
    ImageBandListType &BlueImageBandList)
{
    // We should have always been presented with non-empty lists...
    assert(!RedImageBandList.empty());
    assert(!GreenImageBandList.empty());
    assert(!BlueImageBandList.empty());

    // Set file name for console messages to begin with...
    Console::GetInstance().SetCurrentFileName(OutputFileName);

    // Overwrite not enabled and file already existed, don't overwrite...
    if(!Options::GetInstance().GetOverwrite() && 
       (access(OutputFileName.c_str(), F_OK) == 0))
        SetErrorAndReturnFalse("output already exists, not overwriting (use --overwrite to override)");

    // Form the best image set from each band list...

        // Iterators to point to best of each, beginning with the best 
        //  of each, but not necessarily forming the final set. e.g.
        //  some might have a full histogram, others just an axis, and
        //  sometimes maybe neither with just the vanilla image...
        ImageBandListReverseIterator BestRedIterator   = RedImageBandList.rbegin();
//VicarImageBand RedImage = *BestRedIterator;
        ImageBandListReverseIterator BestGreenIterator = GreenImageBandList.rbegin();
//VicarImageBand GreenImage = *BestGreenIterator;
        ImageBandListReverseIterator BestBlueIterator  = BlueImageBandList.rbegin();
//VicarImageBand BlueImage = *BestBlueIterator;

        // If the best image of each band list has some with an
        //  axis present, no full histogram, and some without...
        const size_t AxesOnlyPresent = 
            (*BestRedIterator).IsAxisOnlyPresent() +
            (*BestGreenIterator).IsAxisOnlyPresent() +
            (*BestBlueIterator).IsAxisOnlyPresent();
        if(AxesOnlyPresent >= 1 && AxesOnlyPresent < 3)
        {
            // ...see if you can find ones with full histograms then, 
            //  which are next best option...
            BestRedIterator = FindBestImageBandWithFullHistogram(RedImageBandList, BestRedIterator);
            BestGreenIterator = FindBestImageBandWithFullHistogram(GreenImageBandList, BestGreenIterator);
            BestBlueIterator = FindBestImageBandWithFullHistogram(BlueImageBandList, BestBlueIterator);
        }

        // At least one of them didn't match up...
        if(BestRedIterator == RedImageBandList.rend() || 
           BestGreenIterator == GreenImageBandList.rend() ||
           BestBlueIterator == BlueImageBandList.rend())
            SetErrorAndReturnFalse("images for each band present, but no matching set of full histogram variants available");

        // If the best image of each band list has some with a full
        //  histogram present and some without...
        const size_t FullHistogramsPresent = 
            (*BestRedIterator).IsFullHistogramPresent() +
            (*BestGreenIterator).IsFullHistogramPresent() +
            (*BestBlueIterator).IsFullHistogramPresent();
        if(FullHistogramsPresent >= 1 && FullHistogramsPresent < 3)
        {
            // ...try without any full histogram or axis at all, just vanilla image...
            BestRedIterator = FindBestImageBandWithNoAxis(RedImageBandList, BestRedIterator);
            BestGreenIterator = FindBestImageBandWithNoAxis(GreenImageBandList, BestGreenIterator);
            BestBlueIterator = FindBestImageBandWithNoAxis(BlueImageBandList, BestBlueIterator);
        }

        // At least one of them didn't match up...
        if(BestRedIterator == RedImageBandList.rend() || 
           BestGreenIterator == GreenImageBandList.rend() ||
           BestBlueIterator == BlueImageBandList.rend())
            SetErrorAndReturnFalse("images for each band present, but no matching set of non-overlayed variants available");

    // Get the raw band data of each colour band...

        // Space for the raw band data...
        VicarImageBand::RawBandDataType RedRawBandData;
        VicarImageBand::RawBandDataType GreenRawBandData;
        VicarImageBand::RawBandDataType BlueRawBandData;
    
        // Red...

            // Get the best red image band...
            VicarImageBand &BestRedImageBand = *BestRedIterator;

            // Get the raw band data and check for error...
            if(!BestRedImageBand.GetRawBandData(RedRawBandData))
                SetErrorAndReturnFalse(BestRedImageBand.GetErrorMessage());

            // Get width and height...
            const size_t RedWidth  = BestRedImageBand.GetTransformedWidth();
            const size_t RedHeight = BestRedImageBand.GetTransformedHeight();
        
        // Green...
        
            // Get the best green image band...
            VicarImageBand &BestGreenImageBand = *BestGreenIterator;

            // Get the raw band data and check for error...
            if(!BestGreenImageBand.GetRawBandData(GreenRawBandData))
                SetErrorAndReturnFalse(BestGreenImageBand.GetErrorMessage());

            // Get width and height...
            const size_t GreenWidth  = BestGreenImageBand.GetTransformedWidth();
            const size_t GreenHeight = BestGreenImageBand.GetTransformedHeight();

        // Blue...
            
            // Get the best blue image band...
            VicarImageBand &BestBlueImageBand = *BestBlueIterator;

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
            const uint8_t RedByte    = RedRawBandData.at(Y).at(X);
            const uint8_t GreenByte  = GreenRawBandData.at(Y).at(X);
            const uint8_t BlueByte   = BlueRawBandData.at(Y).at(X);

            // Encode...
            PngImage.set_pixel(X, Y, png::rgb_pixel(RedByte, GreenByte, BlueByte));
        }
    }

    // Write out image, if not a dry run...
    if(!Options::GetInstance().GetDryRun())
        PngImage.write(OutputFileName);

    // If saving metadata is enabled, dump it...
    if(Options::GetInstance().GetSaveMetadata())
    {
        // Prepare list of components...
        ImageBandListType ImageBandList;
        ImageBandList.push_back(*BestRedIterator);
        ImageBandList.push_back(*BestGreenIterator);
        ImageBandList.push_back(*BestBlueIterator);
        SaveMetadata(CreateOutputFileName(false, "txt"), ImageBandList);
    }

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
            uint8_t GrayByte = RawBandData.at(Y).at(X);

            // Encode...
            PngImage.set_pixel(X, Y, GrayByte);
        }
    }
    
    // Write out, if not a dry run...
    if(!Options::GetInstance().GetDryRun())
        PngImage.write(OutputFileName);

    // Done...
    return true;
}

// Save metadata for file...
void ReconstructableImage::SaveMetadata(
    const string &OutputFileName, 
    const ImageBandListType &ImageBandList)
{
    // Check for dry run...
    if(Options::GetInstance().GetDryRun())
        return;

    // Overwrite not enabled and file already existed, don't overwrite...
    if(!Options::GetInstance().GetOverwrite() && 
       (access(OutputFileName.c_str(), F_OK) == 0))
    {
        Message(Console::Warning) << "output metadata already exists, not overwriting (use --overwrite to override)";
        return;
    }

    // Create the metadata text file...
    ofstream OutputFileStream(OutputFileName.c_str());
    
        // Failed...
        if(!OutputFileStream.is_open())
        {
            // Just give a warning and abort...
            Message(Console::Warning) << "couldn't save metadata" << endl;
            return;
        }

    // Give user some basic information about the metadata...
    OutputFileStream
        << "The following is a machine generated collection of metadata of each of" << endl
        << "the image bands used to reconstruct a colour image." << endl
        << endl;

    // Dump each section...
    for(ImageBandListConstIterator Iterator = ImageBandList.begin(); 
        Iterator != ImageBandList.end(); 
      ++Iterator)
    {
        // Get image...
        const VicarImageBand &ImageBand = *Iterator;

        // Dump metadata...
        OutputFileStream 
            << "basic heuristic method: " << ImageBand.GetBasicMetadataParserHeuristic() << endl
            << "camera azimuth / elevation: " << ImageBand.GetAzimuthElevation() << endl
            << "camera event: " << ImageBand.GetCameraEventLabelNoSol() << endl
            << "camera event solar day: " << ImageBand.GetSolarDay() << endl
            << "diode band type: " << ImageBand.GetDiodeBandTypeFriendlyString() << endl
            << "file size: " << ImageBand.GetFileSize() << endl
            << "input file: " << ImageBand.GetInputFileNameOnly() << endl
            << "magnetic tape: " << ImageBand.GetMagneticTapeNumber() << endl
            << "magnetic tape file ordinal: " << ImageBand.GetFileOnMagneticTapeOrdinal() << endl
            << "mean pixel value: " << ImageBand.GetMeanPixelValue() << endl
            << "month: " << ImageBand.GetMonth() << endl
            << "overlay axis present: " << ImageBand.IsAxisPresent() << endl
            << "overlay full histogram present: " << ImageBand.IsFullHistogramPresent() << endl
            << "physical record size: " << ImageBand.GetPhysicalRecordSize() << endl
            << "physical record padding: " << ImageBand.GetPhysicalRecordPadding() << endl
            << "phase offset required: " << ImageBand.GetPhaseOffsetRequired() << endl
            << "raw image offset: " << ImageBand.GetRawImageOffset() << endl
            << endl 
            << endl;
    }
}


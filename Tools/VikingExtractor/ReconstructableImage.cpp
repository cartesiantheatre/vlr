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
#include <algorithm>
#include <png++/png.hpp>

// Using the standard namespace...
using namespace std;

// Helpful macro...
#define SetErrorAndReturn(Message)              { SetErrorMessage((Message)); return; }
#define SetErrorAndReturnNullStream(Message)    { SetErrorMessage((Message)); return ostream(0); }
#define SetErrorAndReturnFalse(Message)         { SetErrorMessage((Message)); return false; }

// Three way min() / max function templates...
template<class T> const T &min(const T &A, const T &B, const T &C) { return min(A, min(B, C)); }
template<class T> const T &max(const T &A, const T &B, const T &C) { return max(A, max(B, C)); }

// Constructor...
ReconstructableImage::ReconstructableImage(
    const std::string &OutputRootDirectory, 
    const std::string &CameraEventLabel)
    : m_OutputRootDirectory(OutputRootDirectory),
      m_CameraEventLabel(CameraEventLabel),
      m_AutoRotate(true),
      m_Interlace(false),
      m_SolDirectorize(false)
{
    // Always need an label...
    assert(!CameraEventLabel.empty());
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

// Create the necessary path to the output file and return a path...
string ReconstructableImage::GetOutputFileName()
{
    // Identifier and solar day this image was taken on...
    string Identifier   = "unknown";
    string SolarDay     = "unknown";
    
    // Extract solar day out of camera event label...
    const size_t SolarDayOffset = m_CameraEventLabel.find_last_of("/\\");
    if(SolarDayOffset != string::npos && (SolarDayOffset + 1 < m_CameraEventLabel.length()))
    {
        // Copy the identifier and solar day out of the label...
        Identifier.assign(m_CameraEventLabel, 0, SolarDayOffset);
        SolarDay.assign(m_CameraEventLabel, SolarDayOffset + 1, 4);

        // Remove initial zeros, if any, from solar day...
        while(SolarDay.length() > 1 && SolarDay.at(0) == '0')
            SolarDay.erase(0, 1);
    }

    // Images are reconstructed in subfolder of solar day it was 
    //  taken on, so create the subfolder, if enabled......
    if(m_SolDirectorize)
    {
        // Create full path to subfolder...
        const string FullDirectory = m_OutputRootDirectory + "/" + SolarDay + "/";
        
        // Create and check for error...
        if(!CreateDirectoryRecursively(FullDirectory))
        {
            // Set the error message and abort...
            SetErrorMessage("could not create output subdirectory for solar day");
            return string();
        }
        
        // Now have enough information to create full path to output file name...
        return FullDirectory + Identifier + ".png";
    }
    
    // Otherwise, if sol directory mode is not enabled...
    else
        return m_OutputRootDirectory + "/" + Identifier + ".png";
    
}

// Extract the image out as a PNG, or return false if failed...
bool ReconstructableImage::Reconstruct()
{
    // Create full path to output file...
    const string OutputFileName = GetOutputFileName();

        // Failed...
        if(IsError())
            return false;

    // Set file name for console messages to begin with...
    Console::GetInstance().SetCurrentFileName(OutputFileName);

    // Sort each band lists from lowest to best quality...
    sort(m_RedImageBandList.begin(), m_RedImageBandList.end());
    sort(m_GreenImageBandList.begin(), m_GreenImageBandList.end());
    sort(m_BlueImageBandList.begin(), m_BlueImageBandList.end());
    sort(m_Infrared1ImageBandList.begin(), m_Infrared1ImageBandList.end());
    sort(m_Infrared2ImageBandList.begin(), m_Infrared2ImageBandList.end());
    sort(m_Infrared3ImageBandList.begin(), m_Infrared3ImageBandList.end());
    sort(m_GrayImageBandList.begin(), m_GrayImageBandList.end());

    // Setup shorthand sizes to image band vectors...
    const size_t Reds       = m_RedImageBandList.size();
    const size_t Greens     = m_GreenImageBandList.size();
    const size_t Blues      = m_BlueImageBandList.size();
    const size_t Infrareds1 = m_Infrared1ImageBandList.size();
    const size_t Infrareds2 = m_Infrared2ImageBandList.size();
    const size_t Infrareds3 = m_Infrared3ImageBandList.size();
    const size_t Grays      = m_GrayImageBandList.size();

    // Get the best band data of each type of band type...
    
        // Pointers to the best of each type, or NULL if none available...
        VicarImageBand *BestRed         = NULL;
        VicarImageBand *BestGreen       = NULL;
        VicarImageBand *BestBlue        = NULL;
        VicarImageBand *BestInfrared1   = NULL;
        VicarImageBand *BestInfrared2   = NULL;
        VicarImageBand *BestInfrared3   = NULL;
        VicarImageBand *BestGrayscale   = NULL;

        // Get pointers to the best of each type, if available...
        if(Reds) 
            BestRed = &m_RedImageBandList.back();
        if(Greens)
            BestGreen = &m_GreenImageBandList.back();
        if(Blues) 
            BestBlue = &m_BlueImageBandList.back();
        if(Infrareds1)
            BestInfrared1 = &m_Infrared1ImageBandList.back();
        if(Infrareds2)
            BestInfrared2 = &m_Infrared2ImageBandList.back();
        if(Infrareds3)
            BestInfrared3 = &m_Infrared3ImageBandList.back();
        if(Grays)
            BestGrayscale = &m_GrayImageBandList.back();

    // Colour image... (only all colour bands present)
    if((Reds == 1 && Greens == 1 && Blues == 1) && 
       (Infrareds1 + Infrareds2 + Infrareds3 + Grays == 0))
    {
        // Attempt to reconstruct...
        return ReconstructColourImage(OutputFileName, BestRed, BestGreen, BestBlue);
    }

/*
    TODO: Complete reconstruction recipes for below.
*/

    /* Colour image... (only all colour bands present)
    if((min(Reds, Greens, Blues) >= 1) && 
       (Infrareds1 + Infrareds2 + Infrareds3 + Grays == 0))
    {
        // Attempt to reconstruct...
        if(!ReconstructColourImage(OutputFileName, BestRed, BestGreen, BestBlue))
            Message(Console::Error) << GetErrorMessage() << endl;
        else
            Message(Console::Info) << "reconstructed colour image successfully" << endl;
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
Message(Console::Info)
     << m_RedImageBandList.size() << " "
     << m_GreenImageBandList.size() << " "
     << m_BlueImageBandList.size() << " "
     << m_Infrared1ImageBandList.size() << " "
     << m_Infrared2ImageBandList.size() << " "
     << m_Infrared3ImageBandList.size() << " "
     << m_GrayImageBandList.size() << endl;

        Message(Console::Error) << "no known reconstruction recipe available" << endl;
        return false;
    }
}

// Reconstruct a colour image from requested image bands which can be NULL...
bool ReconstructableImage::ReconstructColourImage(
    const string &OutputFileName, 
    VicarImageBand *BestRedImageBand, 
    VicarImageBand *BestGreenImageBand, 
    VicarImageBand *BestBlueImageBand)
{
    // At least one of the image bands should be non-null...
    assert(BestRedImageBand || BestGreenImageBand || BestBlueImageBand);

    // Extraction streams...
    ifstream RedExtractionStream;
    ifstream GreenExtractionStream;
    ifstream BlueExtractionStream;
    
    // Width and height...
    int Width   = 0;
    int Height  = 0;

    // Open all available band type extraction streams, and width and 
    //  height from whichever of them are available... (should be all 
    //  the same)
    
        // Red...
        if(BestRedImageBand)
        {
            // Initialize extraction stream and check for error...
            if(!BestRedImageBand->GetExtractionStream(RedExtractionStream))
                SetErrorAndReturnFalse(BestRedImageBand->GetErrorMessage());

            // Get width and height...
            Width   = BestRedImageBand->GetWidth();
            Height  = BestRedImageBand->GetHeight();
        }
        
        // Green...
        if(BestGreenImageBand)
        {
            // Initialize extraction stream and check for error...
            if(!BestGreenImageBand->GetExtractionStream(GreenExtractionStream))
                SetErrorAndReturnFalse(BestGreenImageBand->GetErrorMessage());

            // Get width and height...
            Width   = BestGreenImageBand->GetWidth();
            Height  = BestGreenImageBand->GetHeight();
        }
        
        // Blue...
        if(BestBlueImageBand)
        {
            // Initialize extraction stream and check for error...
            if(!BestBlueImageBand->GetExtractionStream(BlueExtractionStream))
                SetErrorAndReturnFalse(BestBlueImageBand->GetErrorMessage());

            // Get width and height...
            Width   = BestBlueImageBand->GetWidth();
            Height  = BestBlueImageBand->GetHeight();
        }

    // Allocate png storage...
    png::image<png::rgb_pixel> PngImage(Width, Height);

    // Toggle interlacing, if user selected...
    if(m_Interlace)
        PngImage.set_interlace_type(png::interlace_adam7);
    else
        PngImage.set_interlace_type(png::interlace_none);

    // Pass raw image data through encoder, row by row...
    for(size_t Y = 0; Y < PngImage.get_height(); ++Y)
    {
        // Pass raw image data through encoder, column by column...
        for(size_t X = 0; X < PngImage.get_width(); ++X)
        {
            // Storage for this pixels colour...
            char RedByte    = '\x0';
            char GreenByte  = '\x0';
            char BlueByte   = '\x0';

            // Red extraction stream available, use...
            if(BestRedImageBand)
            {
                // Read byte and check for error ...
                if(!RedExtractionStream.read(&RedByte, 1).good())
                    SetErrorAndReturnFalse("raw red channel's source image band data i/o error")
            }

            // Green extraction stream available, use...
            if(BestGreenImageBand)
            {
                // Read byte and check for error ...
                if(!GreenExtractionStream.read(&GreenByte, 1).good())
                    SetErrorAndReturnFalse("raw green channel's source image band data i/o error")
            }

            // Blue extraction stream available, use...
            if(BestBlueImageBand)
            {
                // Read byte and check for error ...
                if(!BlueExtractionStream.read(&BlueByte, 1).good())
                    SetErrorAndReturnFalse("raw blue channel's source image band data i/o error")
            }
            
            // Encode...
            PngImage.set_pixel(X, Y, png::rgb_pixel(RedByte, GreenByte, BlueByte));
        }
    }

    // Requested to auto rotate...
    if(m_AutoRotate)
    {
        // Allocate png storage of swapped dimensions...
        png::image<png::rgb_pixel> RotatedPngImage(Height, Width);
        
        // Transform each row...
        for(size_t Y = 0; Y < RotatedPngImage.get_height(); ++Y)
        {
            // Transform each column...
            for(size_t X = 0; X < RotatedPngImage.get_width(); ++X)
                RotatedPngImage.set_pixel(X, Y, PngImage.get_pixel(Width - Y - 1, X));
        }
        
        // Write out...
        RotatedPngImage.write(OutputFileName);
    }
    
    // No auto rotation requested, write out normally...
    else
        PngImage.write(OutputFileName);

    // Done...
    return true;
}

/* Reconstruct a grayscale image from requested image band...
bool ReconstructableImage::ReconstructGrayscaleImage(
    const string &OutputFileName, 
    VicarImageBand &BestGrayscaleImageBand)
{
    // Allocate...
    png::image<png::gray_pixel> PngImage(m_Width, m_Height);

    // Toggle interlacing, if user selected...
    if(m_Interlace)
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
                SetErrorAndReturn("raw image data ended prematurely")
            
            // Encode...
            PngImage.set_pixel(X, Y, Byte);
        }
    }

    // Write out the file and alert the user...
    PngImage.write(OutputFile);
    clog << "writing " << OutputFile << endl;
}*/

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
#include "Console.h"
#include <cassert>
#include <iostream>
#include <fstream>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

// Using the standard namespace...
using namespace std;

// Helpful macro...
#define SetErrorAndReturn(Message)              { SetErrorMessage((Message)); return; }
#define SetErrorAndReturnNullStream(Message)    { SetErrorMessage((Message)); return ostream(0); }
#define SetErrorAndReturnFalse(Message)         { SetErrorMessage((Message)); return false; }

// Constructor...
ReconstructableImage::ReconstructableImage(
    const std::string &OutputRootDirectory, 
    const std::string &CameraEventLabel)
    : m_OutputRootDirectory(OutputRootDirectory),
      m_CameraEventLabel(CameraEventLabel)
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
    //  taken on, so create the subfolder...
    
        // Create full path to subfolder...
        const string FullDirectory = m_OutputRootDirectory + "/" + SolarDay + "/";
        
        // Create and check for error...
        if(mkdir(FullDirectory.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) != 0 && errno != EEXIST)
        {
            // Set the error message and abort...
            SetErrorMessage("could not create output subdirectory for solar day");
            return string();
        }

    // Now have enough information to create full path to output file name...
    return FullDirectory + Identifier + ".png";
}

// Extract the image out as a PNG, or return false if failed...
bool ReconstructableImage::Reconstruct()
{
    // Create full path to output file...
    const string OutputFileName = GetOutputFileName();

        // Failed...
        if(IsError())
            return false;

    // Open the stream...
    ofstream OutputFileStream(OutputFileName.c_str(), ios_base::binary | ios_base::trunc);

        // Failed...
        if(!OutputFileStream.good())
            SetErrorAndReturnNullStream("could not create output image");

    // Set file name for console messages to begin with...
    Console::GetInstance().SetCurrentFileName(OutputFileName);

Message(Console::Info)
     << m_RedImageBandList.size() << " "
     << m_GreenImageBandList.size() << " "
     << m_BlueImageBandList.size() << " "
     << m_Infrared1ImageBandList.size() << " "
     << m_Infrared2ImageBandList.size() << " "
     << m_Infrared3ImageBandList.size() << " "
     << m_GrayImageBandList.size() << endl;

    // Alert user...
//    Message(Console::Info) << "reconstructed successfully" << endl;
    
    // Done reconstructing image...
    return true;
}


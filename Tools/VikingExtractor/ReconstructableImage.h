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

// Multiple include protection...
#ifndef _RECONSTRUCTED_IMAGE_H_
#define _RECONSTRUCTED_IMAGE_H_

// Includes...
#include <ostream>
#include <vector>
#include <string>
#include <fstream>
#include "Options.h"
#include "VicarImageBand.h"

// Reconstructable image...
class ReconstructableImage
{
    // Public methods...
    public:

        // Constructor...
        ReconstructableImage(
            const std::string &OutputRootDirectory, 
            const std::string &CameraEventLabel);

        // Add an image band...
        void AddImageBand(const VicarImageBand &ImageBand);

        // Get an error message, if set...
        const std::string &GetErrorMessage() const 
            { return m_ErrorMessage; }

        // Was an error set?
        bool IsError() const
            { return !m_ErrorMessage.empty(); }
        
        // Extract the image out as a PNG, or return false if failed...
        bool Reconstruct();

    // Protected types...
    protected:

        // Image band list type and iterator...
        typedef std::vector<VicarImageBand>         ImageBandListType;
        typedef ImageBandListType::iterator         ImageBandListIterator;
        typedef ImageBandListType::reverse_iterator ImageBandListReverseIterator;

    // Protected methods...
    protected:

        // Create the necessary path to the output file and return a path. 
        //  The file will have the provided file extension...
        std::string CreateOutputFileName(const std::string &Extension);

        // Create the necessary path to a single component of an image, 
        //  distinguished with a name suffix and ordinal. These end up in the
        //  Unreconstructable directory...
        std::string CreateUnreconstructableOutputFileName(
            const std::string &BandType, 
            const size_t Ordinal);

        // Dump all images within given image band to the output directory in 
        //  a subdirectory Incomplete under the camera event identifier...
        bool DumpUnreconstructable(
            ImageBandListType &ImageBandList, 
            const std::string &BandTypeSuffix);

        // Find the best image in the band list with a full histogram, 
        //  or return rend iterator...
        ImageBandListReverseIterator FindBestImageBandWithFullHistogram(
            ImageBandListType &ImageBandList, 
            ImageBandListReverseIterator &ReverseIterator) const;

        // Find the best image in the band list with no axis, but just 
        //  vanilla image, or return rend iterator...
        ImageBandListReverseIterator FindBestImageBandWithNoAxis(
            ImageBandListType &ImageBandList, 
            ImageBandListReverseIterator &ReverseIterator) const;

        // Reconstruct a colour image from requested image bands...
        bool ReconstructColourImage(
            const std::string &OutputFileName, 
            ImageBandListType &RedImageBandList, 
            ImageBandListType &GreenImageBandList, 
            ImageBandListType &BlueImageBandList);

        // Reconstruct a grayscale image from requested image band...
        bool ReconstructGrayscaleImage(
            const std::string &OutputFileName, 
            VicarImageBand &BestGrayscaleImageBand);

        // Save metadata for file...
        void SaveMetadata(
            const std::string &OutputFileName, 
            const VicarImageBand &RedImageBand, 
            const VicarImageBand &GreenImageBand, 
            const VicarImageBand &BlueImageBand);

        // Set the error message...
        void SetErrorMessage(const std::string &ErrorMessage)
            { m_ErrorMessage = ErrorMessage; }

    // Protected data...
    protected:

        // Root output directory...
        std::string         m_OutputRootDirectory;

        // Image band lists for different colour and infrared types...
        ImageBandListType   m_RedImageBandList;
        ImageBandListType   m_GreenImageBandList;
        ImageBandListType   m_BlueImageBandList;
        ImageBandListType   m_Infrared1ImageBandList;
        ImageBandListType   m_Infrared2ImageBandList;
        ImageBandListType   m_Infrared3ImageBandList;
        ImageBandListType   m_GrayImageBandList;
        
        // Camera event label and same thing without the sol day
        //  and then the solar day itself...
        const std::string   m_CameraEventLabel;
        std::string         m_CameraEventNoSol;
        size_t              m_SolarDay;

        // Error message...
        std::string         m_ErrorMessage;
};

// Multiple include protection...
#endif


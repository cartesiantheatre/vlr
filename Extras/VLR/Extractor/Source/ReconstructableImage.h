/*
    VikingExtractor, to recover images from Viking Lander operations.
    Copyright (C) 2010-2013 Cartesian Theatre <info@cartesiantheatre.com>.
    
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

    // Our headers...
    #include "Options.h"
    #include "VicarImageBand.h"

    // System headers...
    #include <ostream>
    #include <vector>
    #include <string>
    #include <fstream>
    #include <clocale>

    // i18n...
    #include "gettext.h"
    #define _(str) gettext (str)
    #define N_(str) gettext_noop (str)

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

        // If the image wasn't reconstructed successfully, this is the
        //  number of component images that were dumped...
        size_t GetDumpedImagesCount() const { return m_DumpedImagesCount; }

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
        typedef ImageBandListType::const_iterator   ImageBandListConstIterator;
        typedef ImageBandListType::reverse_iterator ImageBandListReverseIterator;

    // Protected methods...
    protected:

        // Create the necessary path to the output file and return a path. 
        //  The file will have the provided file extension, and if it is
        //  not a reconstructable image, then it will use specified name
        //  preceding the file extension...
        std::string CreateOutputFileName(
            const bool Unreconstructable,
            const std::string &Extension,
            const std::string &UnreconstructableName = "");

        // Dump all images within given image band to the output directory in 
        //  a subdirectory Unreconstructable under the camera event identifier...
        bool DumpUnreconstructable(ImageBandListType &ImageBandList);

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

        // Generate metadata for file...
        void GenerateMetadata(
            const std::string &OutputFileName, 
            const ImageBandListType &ImageBandList);

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
        
        // Camera event label and same thing without the solar day...
        const std::string   m_CameraEventLabel;
        std::string         m_CameraEventNoSol;

        // Band type class... (e.g. Colour)
        std::string         m_BandTypeClass;

        // If the image wasn't reconstructed successfully, this is the
        //  number of component images that were dumped...
        size_t              m_DumpedImagesCount;

        // Error message...
        std::string         m_ErrorMessage;

        // Lander number image was taken from, or zero if unknown...
        size_t              m_LanderNumber;
        
        // Month image was taken on... (e.g. Libra)
        std::string         m_Month;

        // Solar day the image was taken on...
        size_t              m_SolarDay;
};

// Multiple include protection...
#endif


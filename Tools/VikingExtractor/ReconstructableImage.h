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

        // Get the camera event label...
        const std::string &GetCameraEventLabel() const 
            { return m_CameraEventLabel; }

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
        typedef std::vector<VicarImageBand> ImageBandListType;
        typedef ImageBandListType::iterator ImageBandListIterator;

    // Protected methods...
    protected:

        // Set the error message...
        void SetErrorMessage(const std::string &ErrorMessage)
            { m_ErrorMessage = ErrorMessage; }

    // Protected data...
    protected:

        // Root output directory...
        const std::string   m_OutputRootDirectory;

        // Image band lists for different colour and infrared types...
        ImageBandListType   m_RedImageBandList;
        ImageBandListType   m_GreenImageBandList;
        ImageBandListType   m_BlueImageBandList;
        ImageBandListType   m_Infrared1ImageBandList;
        ImageBandListType   m_Infrared2ImageBandList;
        ImageBandListType   m_Infrared3ImageBandList;
        ImageBandListType   m_GrayImageBandList;
        
        // Camera event label...
        const std::string   m_CameraEventLabel;
        
        // Error message...
        std::string         m_ErrorMessage;
};

// Multiple include protection...
#endif


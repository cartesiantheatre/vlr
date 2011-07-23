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
        ReconstructableImage(const std::string &CameraEventIdentifier);

        // Add an image band...
        void AddImageBand(const VicarImageBand &ImageBand);

        // Get the camera event identifier...
        const std::string &GetCameraEventIdentifier() const 
            { return m_CameraEventIdentifier; }

        // Get an error message, if set...
        const std::string &GetErrorMessage() const 
            { return m_ErrorMessage; }

        // Get a suggested file name for the reconstructed image...
        std::string GetSuggestedFileName() const;

        // Was an error set?
        bool IsError() const
            { return !m_ErrorMessage.empty(); }
        
        // Extract the image out as a PNG, or return false if failed...
        bool Reconstruct(const string &OutputFile) const;

    // Protected types...
    protected:

        // Image band list type and iterator...
        typedef std::vector<const VicarImageBand>   ImageBandListType;
        typedef ImageBandListType::iterator         ImageBandListIterator;

    // Protected methods...
    protected:

    // Protected data...
    protected:

        // Image band lists for different colour types...
        ImageBandListType   m_RedImageBandList;
        ImageBandListType   m_GreenImageBandList;
        ImageBandListType   m_BlueImageBandList;
        ImageBandListType   m_GrayImageBandList;
        
        // Camera event identifier...
        std::string         m_CameraEventIdentifier;
        
        // Error message...
        std::string         m_ErrorMessage;
};

// Multiple include protection...
#endif


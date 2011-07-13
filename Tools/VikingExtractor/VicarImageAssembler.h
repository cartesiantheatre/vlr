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
#ifndef _VICAR_IMAGE_ASSEMBLER_H_
#define _VICAR_IMAGE_ASSEMBLER_H_

// Includes...
#include "VicarImageBand.h"

// Assemble 1970s era VICAR colour images from individual VICAR images...
class VicarImageAssembler
{
    // Public types...
    public:
    
        // Lander's diode in the photosensor array...
        typedef enum
        {
            Invalid = 0,
            
            // Narrow band low resolution colour diodes...
            Blue,
            Green,
            Red,
            
            // Narrow band low resolution infrared diodes. These 
            //  deteriorated slowly due to neutron radiation from the 
            //  RTG...
            Infrared1,
            Infrared2,
            Infrared3,
            
            // Narrow band low resolution sun diode...
            Sun

        }PSADiode;
        
        // Null output stream...
        struct NullOutputStream : std::ostream
        {
            NullOutputStream() : std::ostream(0) { }
        };

    // Public methods...
    public:

        VicarImageAssembler(
            const std::string &InputDirectory, const bool Verbose = false);

        // Index the contents of the directory returning number of
        //  potentially reconstructable images or throw an error...
        size_t Index();
        
        // Reconstruct the ith image or throw an error...
        void Reconstruct(const size_t Index, const std::string 

    // Protected methods...
    protected:

        // Get the output stream to be verbose, if enabled...
        std::ostream &Verbose() const;

    // Protected data...
    protected:

        // Input directory...
        const std::string           m_InputDirectory;

        // Usage flags...
        bool                        m_Verbose;
        
        // Dummy output stream...
        mutable NullOutputStream    m_DummyOutputStream;
};

// Multiple include protection...
#endif


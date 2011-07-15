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
#ifndef _VICAR_COLOUR_IMAGE_H_
#define _VICAR_COLOUR_IMAGE_H_

// Includes...
#include <string>
#include <ostream>
#include "LogicalRecord.h"

// 1970s era VICAR image class...
class VicarImageBand
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

        // Construct grayscale, or throw an error... (grayscale)
        VicarImageBand(const std::string &InputFile, const bool Verbose = false);

        // Get the azimuth / elevation string...
        const std::string &GetAzimuthElevation() const;

        // Get the band type...
        PSADiode GetBandType() const { return m_BandType; };

        // Is the file accessible and the header ok?
        bool IsOk() const { return m_Ok; }
        
        // Is verbosity set...
        bool IsVerbose() { return m_Verbose; }

        // Set the save labels flag...
        void SetSaveLabels(const bool SaveLabels = true) { m_SaveLabels = SaveLabels; }
        
        // Set verbosity flag...
        void SetVerbose(const bool Verbose = true) { m_Verbose = Verbose; }

        // Extract the image out as a PNG, or throw an error...
        void Extract(const std::string &OutputFile, const bool Interlace = true) const;

    // Protected methods...
    protected:

        // Parse basic metadata, or throw an error. Calls one of the 
        //  implementations below...
        void ParseBasicMetadata(const LogicalRecord &Record);
        
        void ParseBasicMetadataImplementation_FormatA(const LogicalRecord &Record);
        void ParseBasicMetadataImplementation_FormatB(const LogicalRecord &Record);
        void ParseBasicMetadataImplementation_FormatC(const LogicalRecord &Record);

        // Parse extended metadata, if any, or throw an error...
        void ParseExtendedMetadata(const LogicalRecord &Record);
        
        // Read VICAR header and calling all parse methods, or throw an error...
        void ReadHeader();
        
        // Get the output stream to be verbose, if enabled...
        std::ostream &Verbose() const;

    // Protected data...
    protected:

        // Input file name...
        const std::string           m_InputFile;
        
        // Image height and width in pixels...
        int                         m_Height;
        int                         m_Width;

        // Bytes per pixel...
        int                         m_BytesPerColour;
        
        // Size of a physical record and padding...
        size_t                      m_PhysicalRecordSize;
        size_t                      m_PhysicalRecordPadding;
        
        // Raw image offset...
        size_t                      m_RawImageOffset;
        
        // Azimuth / elevation string...
        std::string                 m_AzimuthElevation;
        
        // Band type...
        PSADiode                    m_BandType;

        // True if the header appears to be ok...
        bool                        m_Ok;

        // Saved labels buffer...
        std::string                 m_SavedLabelsBuffer;
        
        // Usage flags...
        bool                        m_SaveLabels;
        bool                        m_Verbose;
        
        // Dummy output stream...
        mutable NullOutputStream    m_DummyOutputStream;
};

// Multiple include protection...
#endif


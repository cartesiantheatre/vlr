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
#include <fstream>
#include <ostream>
#include <set>
#include "LogicalRecord.h"

// 1970s era VICAR image class...
class VicarImageBand
{
    // Public types...
    public:
    
        // Lander's diode in the photosensor array...
        typedef enum
        {
            // Not known... (e.g. not read)
            Unknown = 0,
            
            // Detected, but unsupported...
            Invalid,
            
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
        
        // A set of photosensor array diode band types...
        typedef std::set<PSADiode>  DiodeBandFilterSet;
        
        // Null output stream...
        struct NullOutputStream : std::ostream
        {
            NullOutputStream() : std::ostream(0) { }
        };

    // Public methods...
    public:

        // Construct or throw an error...
        VicarImageBand(const std::string &InputFile, const bool Verbose = false);

        // Get the azimuth / elevation string...
        const std::string &GetAzimuthElevation() const;

        // Get the diode band type...
        PSADiode GetDiodeBandType() const { return m_DiodeBandType; };

        // Get the diode band type as a human friendly string...
        std::string GetDiodeBandTypeString() const;

        // Get the original file on the magnetic tape number, or zero if unknown...
        size_t GetFileOnMagneticTapeNumber() const;

        // Get the file size or throw an error...
        int GetFileSize() const;

        // Get the input file name only without path...
        std::string GetInputFileNameOnly() const;
        
        // Return the label identifier...
        const std::string &GetLabelIdentifier() const;

        // Get the original magnetic tape number, or zero if unknown...
        size_t GetMagneticTapeNumber() const;

        // Check if the image band data is most likely present by file size...
        bool IsBandDataPresent() const;

        // Check second record just to double check that this is actually 
        //  from the Viking Lander EDR...
        bool IsFromVikingLander() const;

        // Check if the file is loadable. Note that this does a shallow
        //  file integrity check and so it may succeed even though Load()
        //  fails later...
        bool IsHeaderIntact() const;

        // Is the file accessible and the header ok?
        bool IsOk() const { return m_Ok; }
        
        // Is verbosity set...
        bool IsVerbose() { return m_Verbose; }

        // Read VICAR header, calling all parse methods, or throw an error...
        void LoadHeader();

        // Read VICAR header's basic metadata only. Does not throw an 
        //  error, but reads the most of what it can...
        void LoadHeaderShallow();

        // Set the photosensor diode band type from VICAR style 
        //  string, or throw an error... (e.g. "RED/T")
        void SetDiodeBandTypeFromVicarToken(const std::string &DiodeBandType);

        // Set the save labels flag...
        void SetSaveLabels(const bool SaveLabels = true) { m_SaveLabels = SaveLabels; }
        
        // Set verbosity flag...
        void SetVerbose(const bool Verbose = true) { m_Verbose = Verbose; }

        // Extract the image out as a PNG, or throw an error...
        void Extract(const std::string &OutputFile, const bool Interlace = true) const;

    // Protected methods...
    protected:

        // Parse basic metadata, or throw an error. Calls one of the 
        //  implementations below based on its formatting. Basic
        //  metadata includes bands, dimensions, pixel format, bytes
        //  per colour, photosensor diode band type, etc...
        void ParseBasicMetadata(std::ifstream &InputFileStream);
        void ParseBasicMetadataImplementation_Format1(const LogicalRecord &HeaderRecord);
        void ParseBasicMetadataImplementation_Format2(const LogicalRecord &HeaderRecord);
        void ParseBasicMetadataImplementation_Format3(const LogicalRecord &HeaderRecord);
        void ParseBasicMetadataImplementation_Format4(const LogicalRecord &HeaderRecord);
        void ParseBasicMetadataImplementation_Format5(const LogicalRecord &HeaderRecord);
        void ParseBasicMetadataImplementation_Format6(const LogicalRecord &HeaderRecord);

        // Parse extended metadata, if any, or throw an error. Extended 
        //  metadata includes the azimuth and elevation...
        void ParseExtendedMetadata(const LogicalRecord &Record);
        
        // Get the output stream to be verbose, if enabled...
        std::ostream &Verbose() const;

    // Protected data...
    protected:

        // Input file name...
        const std::string           m_InputFile;
        
        // Number of image bands in this file. Should always be one...
        size_t                      m_Bands;
        
        // Image height and width in pixels...
        int                         m_Height;
        int                         m_Width;

        // Pixel format... (e.g. 'I' -> integral)
        char                        m_PixelFormat;

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
        PSADiode                    m_DiodeBandType;

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


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
#include <cassert>
#include <string>
#include <fstream>
#include <ostream>
#include <set>
#include <map>
#include "LogicalRecord.h"

// 1970s era VICAR image class...
class VicarImageBand
{
    // Public types...
    public:
    
        // Lander's diode in the photosensor array...
        typedef enum
        {
            // Not known... (e.g. or could be just not read)
            Unknown = 0,
            
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
            Sun,
            
            // Survey diode...
            Survey

        }PSADiode;

        // Token to band type map and pair types...
        typedef std::map<const std::string, PSADiode>   TokenToBandTypeMap;
        typedef std::pair<const std::string, PSADiode>  TokenToBandTypeMapPair;

        // Band type to friendly map and pair types...
        typedef std::map<const PSADiode, std::string>   BandTypeToFriendlyMap;
        typedef std::pair<const PSADiode, std::string>  BandTypeToFriendlyMapPair;

        // A set of photosensor array diode band types...
        typedef std::set<PSADiode>  DiodeBandFilterSet;

    // Public methods...
    public:

        // Construct...
        VicarImageBand(const std::string &InputFile);

        // Extract the image out as a PNG, but most be loaded first...
        void Extract(
            const std::string &OutputFile, const size_t DataBandIndex = 0);

        // Get the azimuth / elevation string...
        const std::string &GetAzimuthElevation() const;

        // Get the camera event identifier...
        const std::string &GetCameraEventIdentifier() const { return m_CameraEventIdentifier; }

        // Get the diode band type...
        PSADiode GetDiodeBandType() const { return m_DiodeBandType; };

        // Get the diode band type as a human friendly string...
        const std::string &GetDiodeBandTypeFriendlyString() const;

        // Get the original file on the magnetic tape number, or zero if unknown...
        size_t GetFileOnMagneticTapeNumber() const;

        // Get the error message...
        const std::string &GetErrorMessage() const { return m_ErrorMessage; }

        // Get the file size, or -1 on error...
        int GetFileSize() const;

        // Get the input file name only without path...
        std::string GetInputFileNameOnly() const;
        
        // Get the original magnetic tape number, or zero if unknown...
        size_t GetMagneticTapeNumber() const;

        // Check if the file came with a camera event identifier...
        bool IsCameraEventIdentifierPresent() const 
            { assert(m_Ok); return !m_CameraEventIdentifier.empty(); }
        
        // Check if an error is present...
        bool IsError() const
            { return !m_ErrorMessage.empty(); }

        // Is the file accessible and the header ok?
        bool IsOk() const { return m_Ok; }
        
        // Load as much of the file as possible, setting error on 
        //  failure...
        void Load();

        // Set Adam7 interlacing...
        void SetInterlace(const bool Interlace = true) { m_Interlace = Interlace; }

        // Set the save labels flag...
        void SetSaveLabels(const bool SaveLabels = true) { m_SaveLabels = SaveLabels; }
        
    // Protected methods...
    protected:

        // Set the photosensor diode band type from VICAR token... (e.g. "RED/T")
        PSADiode GetDiodeBandTypeFromVicarToken(const std::string &DiodeBandTypeToken) const;

        // Check if the header is at least readable, and if so, phase offset 
        //  required to decode file...
        bool IsHeaderIntact(size_t &PhaseOffsetRequired) const;

        // Is the token a valid VICAR diode band type?
        bool IsVicarTokenDiodeBandType(const std::string &DiodeBandTypeToken) const;

        // Check ifthis is actually from the Viking Lander EDR...
        bool IsVikingLanderOrigin() const;

        // Parse basic metadata. Calls one of the implementations below based on its 
        //  formatting. Basic metadata includes bands, dimensions, pixel format, 
        //  bytes per colour, photosensor diode band type, etc...
        void ParseBasicMetadata(std::ifstream &InputFileStream);
        void ParseBasicMetadataImplementation_Format1(const LogicalRecord &HeaderRecord);
        void ParseBasicMetadataImplementation_Format2(const LogicalRecord &HeaderRecord);
        void ParseBasicMetadataImplementation_Format3(const LogicalRecord &HeaderRecord);
        void ParseBasicMetadataImplementation_Format4(const LogicalRecord &HeaderRecord);
        void ParseBasicMetadataImplementation_Format5(const LogicalRecord &HeaderRecord);
        void ParseBasicMetadataImplementation_Format6(const LogicalRecord &HeaderRecord);

        // Parse extended metadata, if any. Extended metadata includes the
        //  azimuth and elevation...
        void ParseExtendedMetadata(const LogicalRecord &Record);

        // Perform a deep probe on the file to check for the photosensor diode
        //  band type, returning Unknown if couldn't detect it or unsupported.
        //  The parameter can be used for callee to store for caller the token
        //  that probably denotes an unsupported diode type...
        PSADiode ProbeDiodeBandType(std::string &DiodeBandTypeHint) const;

        // Set the error message...
        void SetErrorMessage(const std::string &ErrorMessage) { m_Ok = false; m_ErrorMessage = ErrorMessage; }

    // Protected data...
    protected:

        // Token to band type map...
        TokenToBandTypeMap          m_TokenToBandTypeMap;

        // Band type to friendly map...
        BandTypeToFriendlyMap       m_BandTypeToFriendlyMap;

        // Input file name...
        const std::string           m_InputFile;

        // Sometimes the records are out of phase due to being preceeded 
        //  with VAX/VMS prefix bytes. This is the offset required to 
        //  decode file...
        size_t                      m_PhaseOffsetRequired;

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
        
        // Camera event identifier...
        std::string                 m_CameraEventIdentifier;
        
        // Band type...
        PSADiode                    m_DiodeBandType;

        // True if the file is probably extractable...
        bool                        m_Ok;
        
        // If m_Ok is false, this is the error message...
        std::string                 m_ErrorMessage;

        // Saved labels buffer...
        std::string                 m_SavedLabelsBuffer;
        
        // Usage flags...
        bool                        m_Interlace;
        bool                        m_SaveLabels;
};

// Multiple include protection...
#endif


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
#ifndef _VICAR_IMAGE_BAND_H_
#define _VICAR_IMAGE_BAND_H_

// Includes...
#include "LogicalRecord.h"
#include <cassert>
#include <fstream>
#include <map>
#include <ostream>
#include <set>
#include <stdint.h>
#include <string>
#include <vector>

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
        
        // Image rotation...
        typedef enum
        {
            None,
            Rotate90,   /* Angles expressed in degrees and counterclockwise */
            Rotate180,
            Rotate270
        }RotationType;

        // Token to band type map and pair types...
        typedef std::map<const std::string, PSADiode>   TokenToBandTypeMap;
        typedef std::pair<const std::string, PSADiode>  TokenToBandTypeMapPair;

        // Band type to friendly map and pair types...
        typedef std::map<const PSADiode, std::string>   BandTypeToFriendlyMap;
        typedef std::pair<const PSADiode, std::string>  BandTypeToFriendlyMapPair;
        
        // Raw image band data, each nested vector is a row containing column data...
        typedef std::vector< std::vector<uint8_t> >     RawBandDataType;

    // Public methods...
    public:

        // Construct...
        VicarImageBand(const std::string &InputFile);

        // Get the azimuth / elevation string...
        const std::string &GetAzimuthElevation() const { return m_AzimuthElevation; }

        // Get the camera event label with and without the solar day...
        const std::string &GetCameraEventLabel() const { return m_CameraEventLabel; } 
        const std::string &GetCameraEventLabelNoSol() const { return m_CameraEventLabelNoSol; }

        // Get the diode band type...
        PSADiode GetDiodeBandType() const { return m_DiodeBandType; };

        // Get the diode band type as a human friendly string...
        const std::string &GetDiodeBandTypeFriendlyString() const;

        // Get the error message...
        const std::string &GetErrorMessage() const { return m_ErrorMessage; }

        // Get the original file on the magnetic tape number, or zero if unknown...
        size_t GetFileOnMagneticTapeOrdinal() const { return m_FileOnMagneticTapeOrdinal; }

        // Get the file size, or -1 on error...
        int GetFileSize() const; 

        // Get the input file name with full path...
        const std::string &GetInputFileName() const { return m_InputFile; }

        // Get the input file name only without path...
        std::string GetInputFileNameOnly() const; 

        // Get the original magnetic tape number, or zero if unknown...
        size_t GetMagneticTapeNumber() const { return m_MagneticTapeNumber; }

        // Get the mean pixel value of the inner rectangle...
        float GetMeanPixelValue() const { return m_MeanPixelValue; }
        
        // Get the Martian month of this camera event...
        std::string GetMonth() const;

        // Get the OCR buffer...        
        const std::string &GetOCRBuffer() const { return m_OCRBuffer; }

        // Get original image width and height, not accounting for rotation...
        size_t GetOriginalHeight() const { return m_OriginalHeight; } 
        size_t GetOriginalWidth() const { return m_OriginalWidth; }

        // Sometimes the records are out of phase due to being preceeded with VAX/VMS prefix bytes. This is the offset required to decode file...
        size_t GetPhaseOffsetRequired() const { return m_PhaseOffsetRequired; }

        // Get size of a physical record and padding...
        size_t GetPhysicalRecordPadding() const { return m_PhysicalRecordPadding; } 
        size_t GetPhysicalRecordSize() const { return m_PhysicalRecordSize; }

        // Get the raw band data transformed if autorotate was enabled. Use GetTransformedWidth()/Height() to know adapted dimensions...
        bool GetRawBandData(VicarImageBand::RawBandDataType &RawBandData); 

        // Get the solar day the image was taken on...
        size_t GetSolarDay() const { return m_SolarDay; } 

        // Get image height and width, accounting for transformations like rotation...
        size_t GetTransformedHeight() const; 
        size_t GetTransformedWidth() const;

        // Check if the image has an axis overlay present only, but no full histogram...
        bool IsAxisOnlyPresent() const { return (m_AxisPresent && !m_FullHistogramPresent); } 

        // Check if the image has an axis overlay present...
        bool IsAxisPresent() const { return m_AxisPresent; } 

        // Check if the file came with a camera event label...
        bool IsCameraEventLabelPresent() const { assert(m_Ok); return !m_CameraEventLabel.empty(); } 
        
        // Check if an error is present...
        bool IsError() const { return !m_ErrorMessage.empty(); } 
        
        // Check if a full histogram legend is present...
        bool IsFullHistogramPresent() const { return m_FullHistogramPresent; } 
        
        // Is the file accessible and the header ok?
        bool IsOk() const { return m_Ok; } 
        
        // Load as much of the file as possible, setting error on failure...
        void Load(); 
        
        // For comparing quality between images of the same camera event and same band type...
        bool operator<(const VicarImageBand &RightSide) const; 

    // Protected methods...
    protected:

        // Check if the raw band image data, rotated as requested, 
        //  contains text usually found in an image with azimuth / 
        //  elevation axes oriented properly. If so, return true
        //  and store extracted text in buffer...
        bool CheckForHorizontalAxisAndExtractText(
            const RawBandDataType &RawBandData, 
            const RotationType Rotation, 
            std::string &OCRBuffer);

        // Check if the raw band image data, rotated as requested, 
        //  contains text usually found in an image with with a large 
        //  histogram present. If so, return true and store extracted 
        //  text in buffer...
        bool CheckForLargeHistogramAndExtractText(
            const RawBandDataType &RawBandData, 
            const RotationType Rotation, 
            std::string &OCRBuffer);

        // Examine image visually to determine things like suggested 
        //  orientation, optical character recognition, and histogram 
        //  detection, or set an error...
        bool ExamineImageVisually();

        // Extract OCR within image band data...
        std::string ExtractOCR(const RawBandDataType &RawBandData);

        // Get the photosensor diode band type from VICAR token... (e.g. "RED/T")
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

        // Set the camera event label, along with the solar day and camera 
        //  event identifier without the solar day...
        void SetCameraEventLabel(const std::string &CameraEventLabel);

        // Set the error message...
        void SetErrorMessage(const std::string &ErrorMessage) { m_Ok = false; m_ErrorMessage = ErrorMessage; }

    // Static protected methods...
    protected:

        // Mirror the band data from left to right...
        static void MirrorLeftRight(
            const RawBandDataType &RawBandData, 
            RawBandDataType &TransformedRawBandData);

        // Mirror the top and bottom...
        static void MirrorTopBottom(
            const RawBandDataType &RawBandData, 
            RawBandDataType &TransformedRawBandData);

        // Mirror diagonaly...
        static void MirrorDiagonal(
            const RawBandDataType &RawBandData, 
            VicarImageBand::RawBandDataType &TransformedRawBandData);

        // Rotate image band data as requested...
        static void Rotate(
            const RotationType Rotation, 
            const RawBandDataType &RawBandData, 
            RawBandDataType &TransformedRawBandData);

    // Protected data...
    protected:

        // True if the image has an axis overlay present...
        bool                    m_AxisPresent;

        // Azimuth / elevation string...
        std::string             m_AzimuthElevation;

        // Number of image bands in this file. Should always be one...
        size_t                  m_Bands;

        // Band type to friendly map...
        BandTypeToFriendlyMap   m_BandTypeToFriendlyMap;

        // Bytes per pixel...
        int                     m_BytesPerColour;
        
        // Camera event label...
        std::string             m_CameraEventLabel;

        // Camera event identifier without solar day...
        std::string             m_CameraEventLabelNoSol;
        
        // Band type...
        PSADiode                m_DiodeBandType;

        // If m_Ok is false, this is the error message...
        std::string             m_ErrorMessage;

        // File on magnetic tape ordinal...
        size_t                  m_FileOnMagneticTapeOrdinal;

        // True if the image has a full histogram present...
        bool                    m_FullHistogramPresent;

        // Input file name...
        std::string             m_InputFile;

        // Magnetic tape number this originated on...
        size_t                  m_MagneticTapeNumber;

        // Pixel mean value of centre rectange which is 1/3 length and 
        //  width of image. We do this to prevent sampling from outside
        //  in the image overlay and histogram region...
        float                   m_MeanPixelValue;

        // Any OCR text that happened to be extracted...
        std::string             m_OCRBuffer;

        // True if the file is probably extractable...
        bool                    m_Ok;

        // Image height and width in pixels before being rotated...
        size_t                  m_OriginalHeight;
        size_t                  m_OriginalWidth;

        // Sometimes the records are out of phase due to being preceeded 
        //  with VAX/VMS prefix bytes. This is the offset required to 
        //  decode file...
        size_t                  m_PhaseOffsetRequired;

        // Size of a padding between physical records and their size...
        size_t                  m_PhysicalRecordPadding;
        size_t                  m_PhysicalRecordSize;

        // Pixel format... (e.g. 'I' -> integral)
        char                    m_PixelFormat;

        // Raw image offset...
        size_t                  m_RawImageOffset;

        // Counterclockwise rotation to orient image properly which is
        //  always 0, 90, 180, or 270...
        RotationType            m_Rotation;
        
        // Saved labels buffer...
        std::string             m_SavedLabelsBuffer;

        // Solar day image was taken on...
        size_t                  m_SolarDay;
        
        // Token to band type map...
        TokenToBandTypeMap      m_TokenToBandTypeMap;
};

// Multiple include protection...
#endif


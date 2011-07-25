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
#include <ostream>
#include <vector>
#include <set>
#include <map>
#include <string>
#include "VicarImageBand.h"
#include "ReconstructableImage.h"

// Assemble 1970s era VICAR colour images from individual VICAR images...
class VicarImageAssembler
{
    // Public methods...
    public:

        // Constructor...
        VicarImageAssembler(
            const std::string &InputDirectory,
            const std::string &OutputDirectory);

        // Index the contents of the directory returning number of
        //  potentially reconstructable images or throw an error...
        void Index();

        // Set usage switches...
        void SetDiodeFilterClass(const std::string &DiodeFilterClass);
        void SetIgnoreBadFiles(const bool IgnoreBadFiles = true) { m_IgnoreBadFiles = IgnoreBadFiles; }
        void SetLanderFilter(const std::string &LanderFilter);

        // Deconstructor...
       ~VicarImageAssembler();

    // Protected methods...
    protected:

        // Reset the assembler state...
        void Reset();

    // Protected types...
    protected:

        // Camera event label to image band map...
        typedef std::map<std::string, ReconstructableImage *>   CameraEventDictionaryType;
        typedef CameraEventDictionaryType::iterator             CameraEventDictionaryIterator;
        typedef std::pair<std::string, ReconstructableImage *>  CameraEventDictionaryPair;
        
        // A set of photosensor array diode band types...
        typedef std::set<VicarImageBand::PSADiode>              DiodeBandFilterSet;

    // Protected data...
    protected:

        // Camera event dictionary multimap...
        CameraEventDictionaryType       m_CameraEventDictionary;

        // Input directory...
        std::string                     m_InputDirectory;
        
        // Output directory...
        std::string                     m_OutputDirectory;

        // Acceptable diode band filter set...
        DiodeBandFilterSet              m_DiodeBandFilterSet;

        // Usage flags...
        bool                            m_IgnoreBadFiles;
        int                             m_LanderFilter;
};

// Multiple include protection...
#endif


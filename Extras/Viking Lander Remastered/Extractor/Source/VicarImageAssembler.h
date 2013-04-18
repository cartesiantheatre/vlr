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
#ifndef _VICAR_IMAGE_ASSEMBLER_H_
#define _VICAR_IMAGE_ASSEMBLER_H_

// Includes...

    // Our headers...
    #include "Options.h"
    #include "VicarImageBand.h"
    #include "ReconstructableImage.h"
    
    // System headers...
    #include <ostream>
    #include <vector>
    #include <set>
    #include <map>
    #include <string>

// Assemble 1970s era VICAR colour images from individual VICAR images...
class VicarImageAssembler
{
    // Public methods...
    public:

        // Constructor...
        VicarImageAssembler(
            const std::string &InputFileOrRootDirectory,
            const std::string &OutputRootDirectory);

        // Reconstruct all possible images found of either the input 
        //  file or a directory into the output directory...
        void Reconstruct();

        // Deconstructor...
       ~VicarImageAssembler();

    // Protected methods...
    protected:

        // Add the given file to the list of prospective files to examine later...
        void AddProspectiveFile(const std::string &InputFile);

        // Index archive contents into list of prospective files, or throw an
        //  error...
        void IndexArchive(const std::string &InputArchiveFile);

        // Generate input file list from the input directory, or throw an 
        //  error... (recursive)
        void IndexDirectory(const std::string &InputDirectory);

        // Index file into list of prospective files, or throw an error...
        void IndexFile(const std::string &InputFile);

        // Reset the assembler state...
        void Reset();

    // Protected types...
    protected:

        // Camera event label to image band map...
        typedef std::map<std::string, ReconstructableImage *>   CameraEventDictionaryType;
        typedef CameraEventDictionaryType::iterator             CameraEventDictionaryIterator;
        typedef std::pair<std::string, ReconstructableImage *>  CameraEventDictionaryPair;

        // Archived file name... (archive name, file in archive)
        typedef std::pair<std::string, std::string>             ArchivedFileNameType;

    // Protected data...
    protected:

        // Camera event dictionary multimap...
        CameraEventDictionaryType           m_CameraEventDictionary;

        // Input file or directory...
        std::string                         m_InputFileOrRootDirectory;
        
        // List of all potential files to examine...
        std::vector<std::string>            m_ProspectiveFiles;
        std::vector<ArchivedFileNameType>   m_ProspectiveArchivedFiles;
        
        // Output root directory...
        std::string                         m_OutputRootDirectory;
};

// Multiple include protection...
#endif


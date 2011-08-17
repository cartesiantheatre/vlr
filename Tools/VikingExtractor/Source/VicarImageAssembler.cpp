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

// Includes...
#include "VicarImageAssembler.h"
#include "Console.h"
#include <cassert>
#include <iostream>
#include <iomanip>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/types.h>
#include <fnmatch.h>

// Using the standard namespace...
using namespace std;

// Construct and read the header, or throw an error...
VicarImageAssembler::VicarImageAssembler(
    const string &InputFileOrRootDirectory,
    const string &OutputRootDirectory)
    : m_InputFileOrRootDirectory(InputFileOrRootDirectory),
      m_OutputRootDirectory(OutputRootDirectory)
{
    // We should have been provided with an input directory...
    assert(!m_InputFileOrRootDirectory.empty());

    // Output root directory not provided...
    if(m_OutputRootDirectory.empty())
    {
        // Use current working directory...
        char Temp[1024];
        m_OutputRootDirectory = getcwd(Temp, 1024);
    }

    // Make sure output root directory ends with a path separator...
    if(m_OutputRootDirectory.find_last_of("/\\") != m_OutputRootDirectory.length() - 1)
        m_OutputRootDirectory += '/';
}

// Generate input file list from the input file or directory, or throw an error... (recursive)
void VicarImageAssembler::GenerateProspectiveFileList(const string &InputFileOrDirectory)
{
    // Variables...
    DIR            *Directory       = NULL;
    struct dirent  *DirectoryEntry  = NULL;
    
    // Is this just a file?

        // Fetch attributes...
        struct stat FileAttributes;
        if(stat(InputFileOrDirectory.c_str(), &FileAttributes) != 0)
            throw string("could not stat ") + InputFileOrDirectory;

        // Yes, just a file...
        if(S_ISREG(FileAttributes.st_mode))
        {
            // Add to list and done...
            m_ProspectiveFiles.push_back(InputFileOrDirectory);
            return;
        }

    // Make an editable copy of the original input file or directory...
    string InputDirectory = InputFileOrDirectory;

    // Path should end with path separator which is needed for recursing...
    if(InputDirectory.at(InputDirectory.length() - 1) != '/')
        InputDirectory += '/';

    // Open directory and check for error...
    if(!(Directory = opendir(InputDirectory.c_str())))
        throw string("unable to open input directory for indexing ") + InputDirectory;

    // Add all files found and recurse through subdirectories...
    while((DirectoryEntry = readdir(Directory)))
    {
        // Regular file...
        if(DirectoryEntry->d_type == DT_REG)
        {
            // Skip if extension doesn't match that of a potential Viking 
            //  Lander VICAR file...
            if(fnmatch("vl_*.[0-9][0-9][0-9]", DirectoryEntry->d_name, 0) != 0)
                continue;

            // Otherwise, add it...
            else
                m_ProspectiveFiles.push_back(InputDirectory + DirectoryEntry->d_name);
        }

        // Directory, recurse...
        else if((DirectoryEntry->d_type == DT_DIR) && 
                Options::GetInstance().GetRecursive() &&
                (string(".") != DirectoryEntry->d_name) && 
                (string("..") != DirectoryEntry->d_name))
        {
            // Get the subdirectory...
            string SubDirectory = DirectoryEntry->d_name;

            // Path should end with path separator...
            if(SubDirectory.at(SubDirectory.length() - 1) != '/')
                SubDirectory += '/';

            // Recurse and scan subdirectory...
            GenerateProspectiveFileList(InputDirectory + SubDirectory);
        }
    }

    // Done with the directory, unwind stack...
    closedir(Directory);
}

// Reconstruct all possible images found of either the input 
//  file or a directory into the output directory...
void VicarImageAssembler::Reconstruct()
{
    // Variables...
    string ErrorMessage;

    // Try to index the directory...
    try
    {
        // Alert user...
        Message(Console::Summary) << "preparing list of prospective files to catalogue, please wait" << endl;

        // If summarize only mode is enabled, mute current file name, warnings, info, and errors...
        if(Options::GetInstance().GetSummarizeOnly())
        {
            Console::GetInstance().SetUseCurrentFileName(false);
            Console::GetInstance().SetChannelEnabled(Console::Error, false);
            Console::GetInstance().SetChannelEnabled(Console::Info, false);
            Console::GetInstance().SetChannelEnabled(Console::Warning, false);
        }

        // Reset assembler state...
        Reset();

        // Generate input file list from the input file or directory...
        GenerateProspectiveFileList(m_InputFileOrRootDirectory);

        // No prospective files to examine...
        if(m_ProspectiveFiles.empty())
        {
            // Alert...
            Message(Console::Summary) 
                << "no prospective files found"
                << endl;

            // Done...
            return;
        }

        // Keep reading entries while there are some...
        for(vector<string>::iterator CurrentFileIterator = m_ProspectiveFiles.begin();
            CurrentFileIterator != m_ProspectiveFiles.end();
          ++CurrentFileIterator)
        {
            // Get the full path...
            const string CurrentFile = *CurrentFileIterator;

            // Construct an image band object...
            VicarImageBand ImageBand(CurrentFile);

            // Calculate progress...
            const size_t ProspectiveFilesExamined = CurrentFileIterator - m_ProspectiveFiles.begin() + 1;
            const size_t TotalProspectiveFiles = m_ProspectiveFiles.size();
            const float  PercentageExamined = static_cast<float>(ProspectiveFilesExamined) / TotalProspectiveFiles * 100.0f;

            // Update summary, if enabled...
            if(Options::GetInstance().GetSummarizeOnly())
            {
                Message(Console::Summary) 
                    << "\rstudying catalogue of " 
                    << ProspectiveFilesExamined << "/" << TotalProspectiveFiles 
                    << " (" << PercentageExamined << " %)";
            }

            // Otherwise update console so it knows what current file we are 
            //  working with...
            else
                Console::GetInstance().SetCurrentFileName(ImageBand.GetInputFileNameOnly());
            
            // Attempt to load the file...
            ImageBand.Load();

                // Failed...
                if(ImageBand.IsError())
                {
                    // User requested we just skip over bad files....
                    if(Options::GetInstance().GetIgnoreBadFiles())
                    {
                        // Alert and skip...
                        Message(Console::Warning)
                            << ImageBand.GetErrorMessage() 
                            << ", skipping"
                            << endl;
                        continue;
                    }
                    
                    // Otherwise raise an error...
                    else
                    {
                        // Alert and abort...
                        ErrorMessage = 
                            ImageBand.GetErrorMessage() +
                            string(" (-b to skip)");
                        throw ErrorMessage;
                    }
                }

            // Get user selected diode band filter set...
            const Options::FilterDiodeBandSet &DiodeBandSet = 
                Options::GetInstance().GetFilterDiodeBandSet();

            // Not part of the diode filter set...
            if(!DiodeBandSet.empty() && 
               (DiodeBandSet.find(ImageBand.GetDiodeBandType()) == 
                DiodeBandSet.end()))
            {
                // Alert, skip...
                Message(Console::Info) 
                    << "filtering " 
                    << ImageBand.GetDiodeBandTypeFriendlyString()
                    << " type diode bands (-f to change)"
                    << endl;
                continue;
            }

            // Drop if no camera event label...
            if(!ImageBand.IsCameraEventLabelPresent())
            {
                // Alert user, skip...
                Message(Console::Error)
                    << "camera event doesn't identify itself, cannot index" 
                    << endl;
                continue;
            }
            
            // Get the camera event label...
            const string CameraEventLabel = ImageBand.GetCameraEventLabel();

            // Check if a reconstructable object already exists for this event...
            CameraEventDictionaryIterator EventIterator = 
                m_CameraEventDictionary.find(CameraEventLabel);

            // Place for the reconstructable image object...
            ReconstructableImage *Reconstructable = NULL;

                // No, construct a new one...
                if(EventIterator == m_CameraEventDictionary.end())
                {
                    // Alert user...
                    Message(Console::Info)
                        << CameraEventLabel
                        << " is a new camera event, indexing" 
                        << endl;

                    // Construct a new reconstructable image...
                    Reconstructable = new ReconstructableImage(
                        m_OutputRootDirectory, 
                        CameraEventLabel);

                    // Insert the reconstructable image into the event dictionary.
                    //  We use the previous failed find iterator as a possible 
                    //  amortized constant performance optimization...
                    m_CameraEventDictionary.insert(EventIterator,
                        CameraEventDictionaryPair(CameraEventLabel, Reconstructable));
                }

                // Yes, add image band to existing one...
                else
                {
                    // Alert user...
                    Message(Console::Info)
                        << CameraEventLabel
                        << " is a known camera event, indexing"
                        << endl;

                    // Get the reconstructable image object...
                    Reconstructable = EventIterator->second;
                    assert(Reconstructable);
                }

            // Add the image band to the reconstructable image...
            Reconstructable->AddImageBand(ImageBand);

            // Check for error...
            if(Reconstructable->IsError())
            {
                // User requested we just skip over bad files....
                if(Options::GetInstance().GetIgnoreBadFiles())
                {
                    // Alert and skip...
                    Message(Console::Warning)
                        << Reconstructable->GetErrorMessage() 
                        << ", skipping"
                        << endl;
                    continue;
                }
                
                // Otherwise raise an error...
                else
                {
                    // Alert and abort...
                    ErrorMessage = 
                        Reconstructable->GetErrorMessage() +
                        string(" (-b to skip)");
                    throw ErrorMessage;
                }
            }
        }

        // Update summary, if enabled, beginning with new line since last was \r only...
        if(Options::GetInstance().GetSummarizeOnly())
            Message(Console::Summary) << endl;

        // Total attempted reconstructions, total successful, and total just dumped...
        size_t AttemptedReconstruction      = 0;
        size_t SuccessfullyReconstructed    = 0;
        size_t DumpedImages                 = 0;

        // Reconstruct each image...
        for(CameraEventDictionaryIterator EventIterator = m_CameraEventDictionary.begin();
            EventIterator != m_CameraEventDictionary.end();
          ++EventIterator)
        {
            // Note one more attempted reconstruction effort...
          ++AttemptedReconstruction;

            // Update summary, if enabled...
            if(Options::GetInstance().GetSummarizeOnly())
            {
                // Trying to reconstruct...
                if(!Options::GetInstance().GetNoReconstruct())
                    Message(Console::Summary) 
                        << "\rattempting reconstruction " 
                        << AttemptedReconstruction << "/" << m_CameraEventDictionary.size()
                        << " (" << static_cast<float>(AttemptedReconstruction) / m_CameraEventDictionary.size() * 100.0f << " %)";
                
                // Just dumping components...
                else
                    Message(Console::Summary) 
                        << "\rdumping components from " 
                        << AttemptedReconstruction << "/" << m_CameraEventDictionary.size()
                        << " (" << static_cast<float>(AttemptedReconstruction) / m_CameraEventDictionary.size() * 100.0f << " %)";
            }

            // Get the reconstructable image object...
            ReconstructableImage *Reconstructable = EventIterator->second;
            assert(Reconstructable);

            // Reconstruct the image object and check for error...
            if(!Reconstructable->Reconstruct())
            {
                // Since the image wasn't reconstructed successfully, 
                //  this is the number of component images that were dumped...
                DumpedImages += Reconstructable->GetDumpedImagesCount();

                // User requested we just skip over bad files....
                if(Options::GetInstance().GetIgnoreBadFiles())
                {
                    // Alert and skip...
                    Message(Console::Warning)
                        << Reconstructable->GetErrorMessage() 
                        << ", skipping"
                        << endl;
                    continue;
                }

                // Otherwise raise an error...
                else
                {
                    // Alert and abort...
                    ErrorMessage = 
                        Reconstructable->GetErrorMessage() +
                        string(" (-b to skip)");
                    throw ErrorMessage;
                }
            }

            // Otherwise, take note that we recovered one more...
            else
              ++SuccessfullyReconstructed;
        }

        // Update summary, if enabled, beginning with new line since last was \r only...
        if(Options::GetInstance().GetSummarizeOnly())
        {
            // We were trying to reconstruct...
            if(!Options::GetInstance().GetNoReconstruct())
                Message(Console::Summary) 
                    << endl
                    << "successfully reconstructed "
                    << SuccessfullyReconstructed << "/" << m_CameraEventDictionary.size()
                    << ", " << DumpedImages << " unreconstructable components dumped"
                    << endl;
            
            // We were not trying to reconstruct...
            else
                Message(Console::Summary) 
                    << endl
                    << "dumped " << DumpedImages << " image components without reconstruction"
                    << endl;
        }
    }

        // Failed...
        catch(const string &ErrorMessage)
        {
            // Update summary, if enabled, beginning with new line since last was \r only...
            if(Options::GetInstance().GetSummarizeOnly())
                Message(Console::Summary) << endl;
            
            // Reset assembler state...
            Reset();

            // Propagate up the chain...
            throw ErrorMessage;
        }
}

// Reset the assembler state...
void VicarImageAssembler::Reset()
{
    // Empty the prospective file list...
    m_ProspectiveFiles.clear();

    // Cleanup camera event dictionary multi...
    for(CameraEventDictionaryIterator Iterator = m_CameraEventDictionary.begin();
        Iterator != m_CameraEventDictionary.end();
      ++Iterator)
    {
        // Deconstruct the reconstructable image object...
        delete Iterator->second;        
    }
    
    // Cleanup dangling pointers...
    m_CameraEventDictionary.clear();
}

// Deconstructor...
VicarImageAssembler::~VicarImageAssembler()
{
    // Cleanup assembler...
    Reset();
}


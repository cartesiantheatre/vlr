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
#include <dirent.h>
#include <sys/types.h>
#include <fnmatch.h>

// Using the standard namespace...
using namespace std;

// Construct and read the header, or throw an error...
VicarImageAssembler::VicarImageAssembler(
    const string &InputDirectory,
    const string &OutputDirectory)
    : m_InputDirectory(InputDirectory),
      m_OutputDirectory(OutputDirectory),
      m_AutoRotate(true),
      m_IgnoreBadFiles(false),
      m_LanderFilter(0),
      m_SolDirectorize(false),
      m_SummarizeOnly(false)
{
    // We should have been provided with an input directory...
    assert(!m_InputDirectory.empty());
    
    // The input directory should end with a path delimeter...
    if(m_InputDirectory.find_last_of("/\\") != (m_InputDirectory.length() - 1))
        m_InputDirectory += "/";
}

// Index the contents of the directory returning number of potentially
//  reconstructable images or throw an error...
void VicarImageAssembler::Index()
{
    // Variables...
    size_t          TotalFiles      = 0;
    DIR            *Directory       = NULL;
    struct  dirent *DirectoryEntry  = NULL;
    string          CurrentFile;
    string          FileNameOnly;
    string          ErrorMessage;

    // If summarize only mode is enabled, mute current file name, warnings, info, and errors...
    if(m_SummarizeOnly)
    {
        Console::GetInstance().SetUseCurrentFileName(false);
        Console::GetInstance().SetChannelEnabled(Console::Error, false);
        Console::GetInstance().SetChannelEnabled(Console::Info, false);
        Console::GetInstance().SetChannelEnabled(Console::Warning, false);
    }

    // Reset assembler state...
    Reset();

    // Open directory and check for error...
    if(!(Directory = opendir(m_InputDirectory.c_str())))
        throw string("unable to open input directory for indexing: ") + m_InputDirectory;

    // Try to index the directory...
    try
    {
        // Count total number of files...
        while((DirectoryEntry = readdir(Directory)))
        {
            // Not a regular file, skip...
            if(DirectoryEntry->d_type != DT_REG)
                continue;
            
            // Increment...
          ++TotalFiles;
        }
        
        // Rewind directory back to the beginning now...
        rewinddir(Directory);

        // Total files examined so far...
        size_t FilesExamined = 0;

        // Keep reading entries while there are some...
        while((DirectoryEntry = readdir(Directory)))
        {
            // Not a regular file, skip...
            if(DirectoryEntry->d_type != DT_REG)
                continue;
            
            // Update number of files examined...
          ++FilesExamined;

            // Skip if extension doesn't match...
            if(fnmatch("*.[0-9][0-9][0-9]", DirectoryEntry->d_name, 0) != 0)
                continue;

            // Update summary, if enabled...
            if(m_SummarizeOnly)
                Message(Console::Summary) 
                    << "\rexamined " 
                    << FilesExamined << "/" << TotalFiles 
                    << " files, please wait...";
            
            // Get the full path...
            CurrentFile = m_InputDirectory + DirectoryEntry->d_name;

            // Construct an image band object...
            VicarImageBand ImageBand(CurrentFile);

            // Get just the file name as well...
            FileNameOnly = ImageBand.GetInputFileNameOnly();
            
            // Attempt to load the file...
            ImageBand.Load();

                // Failed...
                if(ImageBand.IsError())
                {
                    // User requested we just skip over bad files....
                    if(m_IgnoreBadFiles)
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

            // Not part of the diode filter set...
            if(m_DiodeBandFilterSet.find(ImageBand.GetDiodeBandType()) == 
                m_DiodeBandFilterSet.end())
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
                Message(Console::Info)
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
                        m_AutoRotate, 
                        m_Interlace, 
                        m_SolDirectorize, 
                        m_OutputDirectory, 
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
                if(m_IgnoreBadFiles)
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
        if(m_SummarizeOnly)
            Message(Console::Summary) << endl;

        // Done with the directory...
        closedir(Directory);

        // Total attempted reconstructions and total successful...
        size_t AttemptedReconstruction      = 0;
        size_t SuccessfullyReconstructed    = 0;

        // Reconstruct each image...
        for(CameraEventDictionaryIterator EventIterator = m_CameraEventDictionary.begin();
            EventIterator != m_CameraEventDictionary.end();
          ++EventIterator)
        {
            // Note one more attempted reconstruction effort...
          ++AttemptedReconstruction;

            // Update summary, if enabled...
            if(m_SummarizeOnly)
                Message(Console::Summary) 
                    << "\rattempted reconstruction " 
                    << AttemptedReconstruction << "/" << m_CameraEventDictionary.size()
                    << " files, please wait...";

            // Get the reconstructable image object...
            ReconstructableImage *Reconstructable = EventIterator->second;
            assert(Reconstructable);

            // Reconstruct the image object and check for error...
            if(!Reconstructable->Reconstruct())
            {
                // User requested we just skip over bad files....
                if(m_IgnoreBadFiles)
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
        if(m_SummarizeOnly)
            Message(Console::Summary) 
                << endl
                << "successfully reconstructed "
                << SuccessfullyReconstructed << "/" << m_CameraEventDictionary.size()
                << endl;
    }

        // Failed...
        catch(const string &ErrorMessage)
        {
            // Update summary, if enabled, beginning with new line since last was \r only...
            if(m_SummarizeOnly)
                Message(Console::Summary) << endl;

            // Close the directory...
            closedir(Directory);
            
            // Reset assembler state...
            Reset();

            // Propagate up the chain...
            throw ErrorMessage;
        }
}

// Reset the assembler state...
void VicarImageAssembler::Reset()
{
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

// Set the diode filter type or throw an error...
void VicarImageAssembler::SetDiodeFilterClass(const string &DiodeFilter)
{
    // Clear the old set...
    m_DiodeBandFilterSet.clear();

    // Use any supported type...
    if(DiodeFilter.empty() || DiodeFilter == "any")
    {
        Message(Console::Info) << "using any supported diode filter" << endl;
        m_DiodeBandFilterSet.insert(VicarImageBand::Blue);
        m_DiodeBandFilterSet.insert(VicarImageBand::Green);
        m_DiodeBandFilterSet.insert(VicarImageBand::Red);
        m_DiodeBandFilterSet.insert(VicarImageBand::Infrared1);
        m_DiodeBandFilterSet.insert(VicarImageBand::Infrared2);
        m_DiodeBandFilterSet.insert(VicarImageBand::Infrared3);
        m_DiodeBandFilterSet.insert(VicarImageBand::Sun);
        m_DiodeBandFilterSet.insert(VicarImageBand::Survey);
    }
    
    // Colour band...
    else if(DiodeFilter == "colour")
    {
        Message(Console::Info) << "using colour diode filter" << endl;
        m_DiodeBandFilterSet.insert(VicarImageBand::Blue);
        m_DiodeBandFilterSet.insert(VicarImageBand::Green);
        m_DiodeBandFilterSet.insert(VicarImageBand::Red);
    }
    
    // Infrared band...
    else if(DiodeFilter == "infrared")
    {
        Message(Console::Info) << "using infrared diode filter" << endl;
        m_DiodeBandFilterSet.insert(VicarImageBand::Infrared1);
        m_DiodeBandFilterSet.insert(VicarImageBand::Infrared2);
        m_DiodeBandFilterSet.insert(VicarImageBand::Infrared3);
    }
    
    // Sun...
    else if(DiodeFilter == "sun")
    {
        Message(Console::Info) << "using sun diode filter" << endl;
        m_DiodeBandFilterSet.insert(VicarImageBand::Sun);    
    }
    
    // Survey...
    else if(DiodeFilter == "survey")
    {
        Message(Console::Info) << "using survey diode filter" << endl;
        m_DiodeBandFilterSet.insert(VicarImageBand::Survey);
    }
    
    // Unsupported...
    else
       throw string("unsupported diode filter class: ") + DiodeFilter;
}

// Set the lander filter or throw an error...
void VicarImageAssembler::SetLanderFilter(const string &LanderFilter)
{
    // Use any...
    if(LanderFilter.empty() || LanderFilter == "0" || LanderFilter == "any")
    {
        Message(Console::Info) << "filtering for either Viking Lander" << endl;
        m_LanderFilter = 0;
    }
    
    // Viking 1 lander...
    else if(LanderFilter == "1")
    {
        Message(Console::Info) << "filtering for either Viking Lander 1" << endl;
        m_LanderFilter = 1;
    }

    // Viking 2 lander...
    else if(LanderFilter == "2")
    {
        Message(Console::Info) << "filtering for either Viking Lander 2" << endl;
        m_LanderFilter = 2;
    }
    
    // Unknown...
    else
        throw string("unknown lander filter: ") + LanderFilter;
}

// Deconstructor...
VicarImageAssembler::~VicarImageAssembler()
{
    // Cleanup assembler...
    Reset();
}


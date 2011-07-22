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
    const string &InputDirectory)
    : m_InputDirectory(InputDirectory),
      m_IgnoreBadFiles(false),
      m_LanderFilter(0)
{
    // We should have been provided with an input directory...
    assert(!m_InputDirectory.empty());
    
    // The input directory should end with a path delimeter...
    if(m_InputDirectory.find_last_of("/\\") != (m_InputDirectory.length() - 1))
        m_InputDirectory += "/";

    // Set the diode filter class...
    SetDiodeFilterClass("any");
}

// Index the contents of the directory returning number of potentially
//  reconstructable images or throw an error...
void VicarImageAssembler::Index()
{
    // Variables...
    DIR            *Directory       = NULL;
    struct  dirent *DirectoryEntry  = NULL;
    string          CurrentFile;
    string          FileNameOnly;
    string          ErrorMessage;
//    size_t          FilesIndexed    = 0;

    // Open directory and check for error...
    if(!(Directory = opendir(m_InputDirectory.c_str())))
        throw string("unable to open input directory for indexing: ") + m_InputDirectory;

    // Try to index the directory...
    try
    {
        // Keep reading entries while there are some...
        while((DirectoryEntry = readdir(Directory)))
        {
            // Not a regular file, skip...
            if(DirectoryEntry->d_type != DT_REG)
                continue;
            
            // Skip if extension doesn't match...
            if(fnmatch("*.[0-9][0-9][0-9]", DirectoryEntry->d_name, 0) != 0)
                continue;
            
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
                // Alert and skip...
                Message(Console::Info) 
                    << "filtering " 
                    << ImageBand.GetDiodeBandTypeFriendlyString()
                    << " type diode bands (-f to change)"
                    << endl;
                continue;
            }

            // Alert user...
            Message(Console::Info)
                << "ok " 
                << ImageBand.GetCameraEventIdentifier() 
                << endl;
            
            // Show us indexing the VICAR files...
//            Message(Console::Info) << "\rfiles indexed " << ++FilesIndexed;
        }
        
        // End last message with a new line since it only had a carriage return...
//        Message(Console::Verbose) << endl;
        
        // Done with the directory...
        closedir(Directory);
    }

        // Failed...
        catch(const string &ErrorMessage)
        {
            // Close the directory...
            closedir(Directory);

            // Propagate up the chain...
            throw ErrorMessage;
        }
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


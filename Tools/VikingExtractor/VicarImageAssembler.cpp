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
      m_DiodeFilter("any"),
      m_IgnoreBadFiles(false)
{

}

// Index the contents of the directory returning number of potentially
//  reconstructable images or throw an error...
void VicarImageAssembler::Index()
{
    DIR            *Directory       = NULL;
    struct  dirent *DirectoryEntry  = NULL;
    string          CurrentFile;
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
            
            // Otherwise get the file name...
            else
                CurrentFile = m_InputDirectory + DirectoryEntry->d_name;

            // Construct an image band object...
            VicarImageBand ImageBand(CurrentFile, m_Verbose);
            
            // Shallow integrity check to see if it is probably a loadable VICAR image...
            if(!ImageBand.IsLoadable())
            {
                // User requested we just skip over bad files....
                if(m_IgnoreBadFiles)
                {
                    // Alert and skip...
                    Verbose() << CurrentFile << ": warning: bad file, skipping" << endl;
                    continue;
                }
                
                // Otherwise raise an error...
                else
                    throw string("input not a valid 1970s era VICAR format (-b to skip)");
            }

            // Load it...
            ImageBand.Load();
            
            // Show us indexing the VICAR files...
//            Verbose() << "\rfiles indexed " << ++FilesIndexed;
        }
        
        // End last message with a new line since it only had a carriage return...
        Verbose() << endl;
        
        // Done with the directory...
        closedir(Directory);
    }

        // Failed...
        catch(const string &ErrorMessage)
        {
            // Close the directory...
            closedir(Directory);

            // Propagate up the chain...
            throw CurrentFile + ": error: " + ErrorMessage;;
        }
}

// Set the diode filter type or throw an error...
void VicarImageAssembler::SetDiodeFilter(const string &DiodeFilter)
{
    // Verify it matches one of the acceptable types...
    if(!DiodeFilter.empty()         && 
       DiodeFilter != "any"         &&
       DiodeFilter != "colour"      &&
       DiodeFilter != "infrared"    &&
       DiodeFilter != "sun")
        throw string("unknown diode filter type: ") + DiodeFilter;

    // Store...
    m_DiodeFilter = DiodeFilter.empty() ? "any" : DiodeFilter;
    
    // Alert user...
    Verbose() << "using diode filter for " << m_DiodeFilter << endl;
}

// Get the output stream to be verbose, if enabled...
ostream &VicarImageAssembler::Verbose() const
{
    // Not enabled. Return the null stream...
    if(!m_Verbose)
        return m_DummyOutputStream;

    // Otherwise use the usual standard logging stream...
    else
        return clog;
}


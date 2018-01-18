/*
    VikingExtractor, to recover images from Viking Lander operations.
    Copyright (C) 2010-2018 Cartesian Theatre™ <info@cartesiantheatre.com>.
    
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

    // Provided by Autoconf...
    #include <config.h>
    
    // Our headers...
    #include "Options.h"
    
    // System headers...
    #include <algorithm>
    #include <cctype>
    #include <limits>
    #include <iostream>
    
// Using the standard namespace...
using namespace std;

// Default constructor...
Options::Options()
    :   m_AutoRotate(true),
        m_DirectorizeBandTypeClass(false),
        m_DirectorizeLocation(false),
        m_DirectorizeMonth(false),
        m_DirectorizeSol(false),
        m_DryRun(false),
        m_FilterLander(0),
        m_FilterSolarDay(numeric_limits<size_t>::max()),
        m_IgnoreBadFiles(false),
        m_Interlace(false),
        m_Jobs(1),
        m_NoReconstruct(false),
        m_Overwrite(false),
        m_Recursive(false),
#ifdef USE_DBUS_INTERFACE
        m_RemoteStart(false),
#endif
        m_GenerateMetadata(false),
        m_SummarizeOnly(false)
{
    // Set the default diode filter class to any...
    SetFilterDiodeClass("any");
}

// Set the camera event filter...
void Options::SetFilterCameraEvent(const std::string &CameraEvent)
{
    // Store in upper case version...
    transform(
        CameraEvent.begin(), 
        CameraEvent.end(), 
        back_inserter(m_FilterCameraEvent), 
      ::toupper);
}

// Set the diode filter type or throw an error...
void Options::SetFilterDiodeClass(const string &DiodeClass)
{
    // Clear the old set...
    m_FilterDiodeBandSet.clear();

    // Use any supported type...
    if(DiodeClass.empty() || DiodeClass == "any")
    {
        m_FilterDiodeBandSet.insert(VicarImageBand::Blue);
        m_FilterDiodeBandSet.insert(VicarImageBand::Green);
        m_FilterDiodeBandSet.insert(VicarImageBand::Red);
        m_FilterDiodeBandSet.insert(VicarImageBand::Broadband1);
        m_FilterDiodeBandSet.insert(VicarImageBand::Broadband2);
        m_FilterDiodeBandSet.insert(VicarImageBand::Broadband3);
        m_FilterDiodeBandSet.insert(VicarImageBand::Broadband4);
        m_FilterDiodeBandSet.insert(VicarImageBand::Infrared1);
        m_FilterDiodeBandSet.insert(VicarImageBand::Infrared2);
        m_FilterDiodeBandSet.insert(VicarImageBand::Infrared3);
    /*
        Note: For the time being, disabling solar band type unless explicitly 
        selected. Every label header record examined appears corrupt. Can be 
        manually overridden, however, with --filter-diode=sun switch.

        m_FilterDiodeBandSet.insert(VicarImageBand::Sun);
    */

        m_FilterDiodeBandSet.insert(VicarImageBand::Survey);
    }

    // Broadband...
    else if(DiodeClass == "broadband")
    {
        m_FilterDiodeBandSet.insert(VicarImageBand::Broadband1);
        m_FilterDiodeBandSet.insert(VicarImageBand::Broadband2);
        m_FilterDiodeBandSet.insert(VicarImageBand::Broadband3);
        m_FilterDiodeBandSet.insert(VicarImageBand::Broadband4);
    }

    // Colour band...
    else if(DiodeClass == "colour")
    {
        m_FilterDiodeBandSet.insert(VicarImageBand::Blue);
        m_FilterDiodeBandSet.insert(VicarImageBand::Green);
        m_FilterDiodeBandSet.insert(VicarImageBand::Red);
    }
    
    // Infrared band...
    else if(DiodeClass == "infrared")
    {
        m_FilterDiodeBandSet.insert(VicarImageBand::Infrared1);
        m_FilterDiodeBandSet.insert(VicarImageBand::Infrared2);
        m_FilterDiodeBandSet.insert(VicarImageBand::Infrared3);
    }
    
    // Sun...
    else if(DiodeClass == "sun")
        m_FilterDiodeBandSet.insert(VicarImageBand::Sun);    
    
    // Survey...
    else if(DiodeClass == "survey")
    {
//        Message(Console::Info) << "using survey diode filter" << endl;
        m_FilterDiodeBandSet.insert(VicarImageBand::Survey);
    }
    
    // Unsupported...
    else
       throw string(_("unsupported diode filter class: ")) + DiodeClass;
}

// Set the lander filter or throw an error...
void Options::SetFilterLander(const size_t Lander)
{
    // Bounds check...
    if(Lander > 2)
        throw string(_("invalid lander filter"));
    
    // Remember lander number, 0 being any...
    m_FilterLander = Lander;
}

// Deconstructor...
Options::~Options()
{

}


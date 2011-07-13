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
    
    // Ourself...    
    #include "VicarColourImage.h"

// Using the standard namespace...
using namespace std;

// Construct and read the header, or throw an error...
VicarImageAssembler::VicarImageAssembler(
    const std::string &InputDirectory, const bool Verbose)
    : m_InputDirectory(InputDirectory),
      m_Verbose(Verbose)
{

}

// Get the output stream to be verbose, if enabled...
std::ostream &VicarImageAssembler::Verbose() const
{
    // Not enabled. Return the null stream...
    if(!m_Verbose)
        return m_DummyOutputStream;

    // Otherwise use the usual standard logging stream...
    else
        return clog;
}


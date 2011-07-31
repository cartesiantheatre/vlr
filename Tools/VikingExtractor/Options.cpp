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
#include "Options.h"

// Using the standard namespace...
using namespace std;

// Options singleton instance...
Options *Options::m_SingletonInstance = NULL;

// Default constructor...
Options::Options()
    :   m_AutoRotate(false),
        m_DryRun(false),
        m_IgnoreBadFiles(false),
        m_Interlace(false),
        m_Jobs(1),
        m_Recursive(false),
        m_SaveLabels(false),
        m_SolDirectorize(false),
        m_SummarizeOnly(false)
{

}

// Get the singleton instance...
Options &Options::GetInstance()
{
    // Unconstructed, construct...
    if(!m_SingletonInstance)
        m_SingletonInstance = new Options;

    // Return the only instance...
    return *m_SingletonInstance;
}

// Deconstructor...
Options::~Options()
{

}

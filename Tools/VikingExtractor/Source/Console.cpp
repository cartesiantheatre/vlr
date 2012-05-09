/*
    VikingExtractor, to recover images from Viking Lander operations.
    Copyright (C) 2010, 2011, 2012 Cartesian Theatre <kip@thevertigo.com>.
    
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
#include "Console.h"
#include <iostream>
#include <cassert>

// Using the standard namespace...
using namespace std;

// Console singleton instance...
Console *Console::m_SingletonInstance = NULL;

// Default constructor...
Console::Console()
    : m_UseColours(true),
      m_UseCurrentFileName(true)
{
    // Initialize the channel map...
    m_ChannelMap[Error]     = new Channel(Red,     "error: ");
    m_ChannelMap[Info]      = new Channel(Blue,    "info: ");
    m_ChannelMap[Summary]   = new Channel(Blue,    "");
    m_ChannelMap[Warning]   = new Channel(Yellow,  "warning: ");
    m_ChannelMap[Verbose]   = new Channel(Default, "");

    // Set default floating point settings...
    cout.setf(ios::fixed, ios::floatfield);
    cout.precision(1);
}

// Get the singleton instance...
Console &Console::GetInstance()
{
    // Unconstructed, construct...
    if(!m_SingletonInstance)
        m_SingletonInstance = new Console;

    // Return the only instance...
    return *m_SingletonInstance;
}

// Get an output stream, if enabled, or dummy stream otherwise...
ostream &Console::Message(const Console::ChannelID ID)
{
    // Lookup the channel ID...
    ChannelMapType::const_iterator Iterator = m_ChannelMap.find(ID);

    // Should always be found...
    assert(Iterator != m_ChannelMap.end());

    // Reset the terminal settings...
    if(m_UseColours)
        cout << "\033[0m";
    
    // Get the channel structure...
    Channel &RequestedChannel = *(Iterator->second);

    // Not enabled. Return the null stream...
    if(!RequestedChannel.m_Enabled)
        return m_DummyOutputStream;

    // Flush the last message, if this isn't the verbose stream which is noisy...
    if(ID != Verbose)
        cout.flush();

    // Set to the requested foreground colour...
    if(m_UseColours)
        cout << "\033[1;" << RequestedChannel.m_ForegroundColour << "m";
    
    // Begin message with current file name, if known, and enabled...
    if(!m_CurrentFileName.empty() && m_UseCurrentFileName)
        cout << m_CurrentFileName << ": ";
    
    // Prepend client's message with channel prefix...
    cout << RequestedChannel.m_Prefix;

    // Now return the stream...
    return cout;
}

// Get an output stream, if enabled, or dummy stream otherwise...
ostream &Message(const Console::ChannelID ID)
{
    // Return it...
    return Console::GetInstance().Message(ID);
}

// Enable or suppress a given channel...
void Console::SetChannelEnabled(const Console::ChannelID ID, const bool Enabled)
{
    // Lookup the channel ID...
    ChannelMapType::const_iterator Iterator = m_ChannelMap.find(ID);

    // Should always be found...
    assert(Iterator != m_ChannelMap.end());
    
    // Set the channel enabled bit accordingly...
    Channel &RequestedChannel  = *(Iterator->second);
    RequestedChannel.m_Enabled = Enabled;
}

// Deconstructor...
Console::~Console()
{
    // Clear the channel map...
    ChannelMapType::iterator Iterator = m_ChannelMap.begin();
    while(Iterator != m_ChannelMap.end())
    {
        delete Iterator->second;
        m_ChannelMap.erase(Iterator);
    }
}

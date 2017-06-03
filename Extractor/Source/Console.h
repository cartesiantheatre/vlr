/*
    VikingExtractor, to recover images from Viking Lander operations.
    Copyright (C) 2010-2017 Cartesian Theatre™ <info@cartesiantheatre.com>.
    
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
#ifndef _CONSOLE_H_
#define _CONSOLE_H_

// Includes...

    // Our headers...
    #include "ExplicitSingleton.h"

    // System headers...
    #include <ostream>
    #include <map>
    #include <clocale>

    // i18n...
    #include "gettext.h"
    #define _(str) gettext (str)
    #define N_(str) gettext_noop (str)

// Console explicit singleton class...
class Console : public ExplicitSingleton<Console>
{
    // Because we are a singleton, only ExplicitSingleton can control our 
    //  creation...
    friend class ExplicitSingleton<Console>;

    // Public types...
    public:

        // Channel ID..
        typedef enum
        {
            Error,
            Info,
            Summary,
            Warning,
            Verbose
        }ChannelID;

    // Public methods...
    public:

        // Get an output stream, if enabled, or dummy stream otherwise...
        std::ostream &Message(const Console::ChannelID ID);

        // Enable or disable the use of VT100 ANSI colours...
        void SetUseColours(const bool UseColours = true) 
            { m_UseColours = UseColours; }

        // Enable or disable preceding output with current file name...
        void SetUseCurrentFileName(const bool UseCurrentFileName = true)
            { m_UseCurrentFileName = UseCurrentFileName; }
        
        // Enable or suppress a given channel...
        void SetChannelEnabled(const ChannelID ID, const bool Enabled);

        // Set the current file name...
        void SetCurrentFileName(const std::string &CurrentFileName)
            { m_CurrentFileName = CurrentFileName; }

    // Private methods...
    private:

        // Default constructor...
        Console();
        
        // Deconstructor...
       ~Console();

    // Protected types...
    protected:

        // Foreground colours...
        typedef enum
        {
            Default = 0,
            Black   = 30,
            Red,
            Green,
            Yellow,
            Blue,
            Magenta,
            Cyan,
            White
        }ForegroundColour;

        // Channel structure...
        struct Channel
        {
            // Constructor initializer...
            Channel(const ForegroundColour ForeColour,
                    const std::string &Prefix)
                : m_Enabled(true),
                  m_ForegroundColour(ForeColour),
                  m_Prefix(Prefix)
            { }

            // Enabled flag...
            bool                m_Enabled;

            // Foreground colour...
            ForegroundColour    m_ForegroundColour;
            
            // Prefix string... (e.g. "warning: ")
            std::string         m_Prefix;
        };

        // Channel map type...
        typedef std::map<ChannelID, Channel *>  ChannelMapType;

        // Null output stream...
        struct NullOutputStream : std::ostream
        {
            NullOutputStream() : std::ostream(0) { }
        };

    // Protected data...
    protected:
        
        // Current file name...
        std::string         m_CurrentFileName;
        
        // Channel map...
        ChannelMapType      m_ChannelMap;
        
        // Dummy output stream...
        NullOutputStream    m_DummyOutputStream;
        
        // Use VT100 ANSI colours or not...
        bool                m_UseColours;
        
        // Precede output with current file name...
        bool                m_UseCurrentFileName;
};

// Wrapper around the global singleton instance...
std::ostream &Message(const Console::ChannelID ID);

// Multiple include protection...
#endif


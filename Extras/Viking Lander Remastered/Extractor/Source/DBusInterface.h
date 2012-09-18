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

// Multiple include protection...
#ifndef _DBUS_INTERFACE_H_
#define _DBUS_INTERFACE_H_

// Includes...

    // Provided by Autoconf...
    #include <config.h>

    // Our headers...
    #include "ExplicitSingleton.h"

    // System headers...
    #include <string>

// Sanity check...
#ifndef USE_DBUS_INTERFACE
    #error "D-Bus interface was disabled at configure time. This header should not have been used."
#endif

/*
    Notes: To monitor the session bus for any of the aforementioned signals...

        $ dbus-monitor "type='signal',sender='com.cartesiantheatre.VikingExtractorService',interface='com.cartesiantheatre.VikingExtractorInterface'"
*/

// DBus interface singleton class...
class DBusInterface : public ExplicitSingleton<DBusInterface>
{
    // Because we are a singleton, only ExplicitSingleton can control our 
    //  creation...
    friend class ExplicitSingleton<DBusInterface>;

    // Public methods...
    public:

        // Emit initialized signal...
        void EmitInitializedSignal() const;

        // Emit progress signal with Progress clamped to [0, 1]...
        void EmitSignalProgress(const float Progress) const;

        // Emit terminating signal...
        void EmitTerminatingSignal() const;

        // Get the singleton instance...
        static DBusInterface &GetInstance();

    // Private methods...
    private:

        // Default constructor...
        DBusInterface();

        // Deconstructor...
       ~DBusInterface();

    // Protected data...
    protected:

        // Service bus name...
        const std::string   m_BusName;

        // Object path on the aforementioned bus name...
        const std::string   m_ObjectPath;

        // Interface available on the aforementioned object...
        const std::string   m_Interface;

        // Signals we emit...
        
            // Initialized...
            const std::string   m_SignalInitialized;
        
            // Some progress to report...
            const std::string   m_SignalProgress;
            
            // Terminating...
            const std::string   m_SignalTerminating;
};

// Multiple include protection...
#endif


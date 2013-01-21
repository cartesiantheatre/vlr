/*
    VikingExtractor, to recover images from Viking Lander operations.
    Copyright (C) 2010-2013 Cartesian Theatre <kip@thevertigo.com>.
    
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

    // Sanity check...
    #ifndef USE_DBUS_INTERFACE
        #error "D-Bus interface was disabled at configure time. This header should not have been used."
    #endif

    // Our headers...
    #include "ExplicitSingleton.h"

    // D-Bus...
    #include <dbus/dbus.h>

    // System headers...
    #include <string>

/*
    To monitor the session bus for any of the aforementioned signals...
        $ dbus-monitor "type='signal',sender='com.cartesiantheatre.VikingExtractorService',interface='com.cartesiantheatre.VikingExtractorInterface'"

    To send a Ready signal manually...
        $ dbus-send --print-reply --type=signal --session --dest=com.cartesiantheatre.VikingExtractorService /com/cartesiantheatre/VikingExtractorObject com.cartesiantheatre.VikingExtractorService.Ready
*/

// DBus interface singleton class...
class DBusInterface : public ExplicitSingleton<DBusInterface>
{
    // Because we are a singleton, only ExplicitSingleton can control our 
    //  creation...
    friend class ExplicitSingleton<DBusInterface>;

    // Public methods...
    public:

        // Emit notification signal with string available to be displayed 
        //  through GUI or throw an error...
        void EmitNotificationSignal(const std::string &Message);

        // Emit progress signal with progress clamped to [0.0, 100.0] or throw 
        //  an error...
        void EmitProgressSignal(const double Progress);
        
        // Wait for a D-Bus signal before unblocking or throw an error...
        void WaitRemoteStart();

    // Private methods...
    private:

        // Default constructor...
        DBusInterface();

        // Deconstructor...
       ~DBusInterface();

    // Protected methods...
    protected:

        // Register on the session bus...
        void RegisterOnSessionBus();

    // Protected constants...
    protected:

        // Service bus name...
        const std::string   m_BusName;

        // Object path on the aforementioned bus name...
        const std::string   m_ObjectPath;

        // Interface available on the aforementioned object...
        const std::string   m_Interface;

        // Signals we emit...

            // We emit this with string parameter when some new information to 
            //  be displayed through GUI during long operations
            const std::string   m_NotificationSignal;

            // We emit this when we have some progress to report...
            const std::string   m_ProgressSignal;

            // We receive this signal when the assembler is ready to start. This
            //  provides a mechanism to ensure the GUI doesn't try to wait for 
            //  GUI extractor signals before the extractor has even registered
            //  itself on the session bus...
            const std::string   m_ReadySignal;

    // Protected data...
    protected:

        // Connection to the bus...
        DBusConnection *m_Connection;
        
        // Any D-Bus related error message...
        DBusError       m_Error;
};

// Multiple include protection...
#endif


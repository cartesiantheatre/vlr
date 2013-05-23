/*
    VikingExtractor, to recover images from Viking Lander operations.
    Copyright (C) 2010-2013 Cartesian Theatre <info@cartesiantheatre.com>.
    
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
    #include <gio/gio.h>

    // System headers...
    #include <string>
    #include <clocale>

    // i18n...
    #include "gettext.h"
    #define _(str) gettext (str)
    #define N_(str) gettext_noop (str)

/*
    To monitor the session bus for any of the aforementioned signals...
        $ dbus-monitor "type='signal',sender='com.cartesiantheatre.VikingExtractorService',interface='com.cartesiantheatre.VikingExtractorInterface'"

    To invoke the Start method manually...
        $ gdbus call --session --dest com.cartesiantheatre.VikingExtractorService --object-path /com/cartesiantheatre/VikingExtractorObject --method com.cartesiantheatre.VikingExtractorInterface.Start
*/

// D-Bus name, object path, interface, signal, and method constants...
#define VIKING_EXTRACTOR_DBUS_BUS_NAME               "com.cartesiantheatre.VikingExtractorService"
#define VIKING_EXTRACTOR_DBUS_OBJECT_PATH            "/com/cartesiantheatre/VikingExtractorObject"
#define VIKING_EXTRACTOR_DBUS_INTERFACE              "com.cartesiantheatre.VikingExtractorInterface"
#define VIKING_EXTRACTOR_DBUS_SIGNAL_NOTIFICATION    "Notification"
#define VIKING_EXTRACTOR_DBUS_SIGNAL_PROGRESS        "Progress"
#define VIKING_EXTRACTOR_DBUS_METHOD_START           "Start"

// D-Bus interface singleton class...
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
        
        // D-Bus interface method callback...
        static void MethodCallback(
            GDBusConnection *Connection,
            const gchar *Sender,
            const gchar *ObjectPath,
            const gchar *InterfaceName,
            const gchar *MethodName,
            GVariant *Parameters,
            GDBusMethodInvocation *Invocation,
            gpointer UserData);

        // Connection to the session message bus has been obtained...
        static void OnBusAcquired(
            GDBusConnection *Connection, const gchar *Name, gpointer UserData);

        // Name on the session bus has been obtained...
        static void OnNameAcquired(
            GDBusConnection *Connection, const gchar *Name, gpointer UserData);

        // The name on the bus or the connection itself has been lost...
        static void OnNameLost(
            GDBusConnection *Connection, const gchar *Name, gpointer UserData);

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

        // Introspection XML specification...
        static const gchar ms_IntrospectionXML[];
        
        // Introspection virtual table...
        static const GDBusInterfaceVTable ms_InterfaceVirtualTable;

    // Protected data...
    protected:

        // GLib main event loop needed for gdbus events...
        GMainLoop          *m_MainLoop;

        // When flag set, remote start successfully initiated...
        bool                m_RemoteStart;

        // Introspection data...
        GDBusNodeInfo      *m_IntrospectionData;

        // Connection to the bus...
        GDBusConnection    *m_Connection;
        guint               m_BusID;
        
        // Registration ID...
        guint               m_RegistrationID;
        
        // Any D-Bus related error message...
        GError             *m_Error;
};

// Multiple include protection...
#endif


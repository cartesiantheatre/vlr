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

// Includes...

    // Provided by Autoconf...
    #include <config.h>

    // Our headers...    
    #include "DBusInterface.h"
    #include "Console.h"
    
    // System headers...
    #include <cassert>
    #include <cstdlib>
    #include <sstream>

// Using the standard namespace...
using namespace std;

// Initialize static constant members...

    // Introspection XML specification...
    const gchar DBusInterface::ms_IntrospectionXML[] =
        "<node name='" VIKING_EXTRACTOR_DBUS_OBJECT_PATH "'>"
        "    <interface name='" VIKING_EXTRACTOR_DBUS_INTERFACE "'>"
        "        <method name='" VIKING_EXTRACTOR_DBUS_METHOD_START "'></method>"
        "        <signal name='" VIKING_EXTRACTOR_DBUS_SIGNAL_PROGRESS "'>"
        "            <arg name='Fraction' type='d'></arg>"
        "        </signal>"
        "        <signal name='" VIKING_EXTRACTOR_DBUS_SIGNAL_NOTIFICATION "'>"
        "            <arg name='Message' type='s'></arg>"
        "        </signal>"
        "    </interface>"
        "</node>";

    // Introspection virtual table...
    const GDBusInterfaceVTable DBusInterface::ms_InterfaceVirtualTable =
    {
        DBusInterface::MethodCallback,
        NULL,
        NULL,
        {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}
    };

// Default constructor...
DBusInterface::DBusInterface()
  : m_BusName(VIKING_EXTRACTOR_DBUS_BUS_NAME),
    m_ObjectPath(VIKING_EXTRACTOR_DBUS_OBJECT_PATH),
    m_Interface(VIKING_EXTRACTOR_DBUS_INTERFACE),
    m_NotificationSignal(VIKING_EXTRACTOR_DBUS_SIGNAL_NOTIFICATION),
    m_ProgressSignal(VIKING_EXTRACTOR_DBUS_SIGNAL_PROGRESS),
    m_MainLoop(NULL),
    m_RemoteStart(false),
    m_IntrospectionData(NULL),
    m_Connection(NULL),
    m_BusID(0),
    m_RegistrationID(0),
    m_Error(NULL)
{
    // Initialize type system...
    g_type_init();

    // Register on the session bus...
    RegisterOnSessionBus();
}

// Emit notification signal with string available to be displayed through GUI 
//  or throw an error...
void DBusInterface::EmitNotificationSignal(const string &Notification)
{
    // Sanity check...
    assert(!g_dbus_connection_is_closed(m_Connection));

    // Prepare signal parameters...
    GVariant *SignalParameters = g_variant_new("(s)", Notification.c_str());

    // Emit the signal...
    const bool Result = g_dbus_connection_emit_signal(
        m_Connection, 
        m_BusName.c_str(), 
        m_ObjectPath.c_str(), 
        m_Interface.c_str(), 
        m_NotificationSignal.c_str(), 
        SignalParameters, 
       &m_Error);

        // Failed...
        if(!Result)
        {
            // Alert user and terminate...
            Message(Console::Error) 
                << "d-bus: could not emit 'Notification' signal... (" 
                << m_Error->message << ")" << endl;

            // Cleanup and terminate...
            g_clear_error(&m_Error);
            exit(EXIT_FAILURE);
        }

    // Make sure the signal actually was dispatched...
    g_dbus_connection_flush_sync(m_Connection, NULL, NULL);
}

// Emit progress signal with progress clamped to [0.0, 100.0] or throw an error...
void DBusInterface::EmitProgressSignal(const double Progress)
{
    // Sanity check...
    assert(!g_dbus_connection_is_closed(m_Connection));

    // Prepare signal parameters...
    GVariant *SignalParameters = g_variant_new("(d)", Progress);

    // Emit the signal...
    const bool Result = g_dbus_connection_emit_signal(
        m_Connection, 
        m_BusName.c_str(), 
        m_ObjectPath.c_str(), 
        m_Interface.c_str(), 
        m_ProgressSignal.c_str(), 
        SignalParameters, 
       &m_Error);

        // Failed...
        if(!Result)
        {
            // Alert user and terminate...
            Message(Console::Error) 
                << "d-bus: could not emit 'Progress' signal... (" 
                << m_Error->message << ")" << endl;

            // Cleanup and terminate...
            g_clear_error(&m_Error);
            exit(EXIT_FAILURE);
        }

    // Make sure the signal actually was dispatched...
    g_dbus_connection_flush_sync(m_Connection, NULL, NULL);
}

// D-Bus interface method callback...
void DBusInterface::MethodCallback(
    GDBusConnection *,                  /* Connection */
    const gchar *,                      /* Sender */
    const gchar *,                      /* ObjectPath */
    const gchar *InterfaceName,
    const gchar *MethodName,
    GVariant *,                         /* Parameters */
    GDBusMethodInvocation *Invocation,
    gpointer UserData)
{
    // Retrieve this pointer from user data...
    DBusInterface &Context(*static_cast<DBusInterface *>(UserData));
    
    // Start method...
    if(g_strcmp0(MethodName, VIKING_EXTRACTOR_DBUS_METHOD_START) == 0)
    {
        // Inform WaitRemoteStart() that it can stop blocking now...
        Context.m_RemoteStart = true;
        
        // GLib's main loop no longer needs to continue checking for events...
        g_main_loop_quit(Context.m_MainLoop);

        // Acknowledge to client that we are done processing their request...
        g_dbus_method_invocation_return_value(Invocation, NULL);
    }
    
    // Some other method that we specified in the virtual table, but haven't
    //  implemented yet...
    else
    {
        // Inform client that the method isn't implemented yet...
        g_dbus_method_invocation_return_error(
            Invocation, G_DBUS_ERROR, G_DBUS_ERROR_NOT_SUPPORTED,
            "Method %s.%s is not implemented.", InterfaceName, MethodName);
    }
}

// Connection to the session message bus has been obtained...
void DBusInterface::OnBusAcquired(
    GDBusConnection *,  /* Connection */
    const gchar *,      /* Name */
    gpointer UserData)
{
    // Retrieve this pointer from user data...
    DBusInterface &Context(*static_cast<DBusInterface *>(UserData));

    // Register our interfaces on the session bus...
    Context.m_RegistrationID = g_dbus_connection_register_object(
        Context.m_Connection, 
        Context.m_ObjectPath.c_str(), 
        Context.m_IntrospectionData->interfaces[0], 
        &ms_InterfaceVirtualTable,
        static_cast<gpointer>(&Context),
        NULL, 
        &Context.m_Error);

        // Failed...
        if(!Context.m_RegistrationID)
        {
            // Alert user and terminate...
            Message(Console::Error) 
                << "d-bus: could not register service with session bus... (" 
                << Context.m_Error->message << ")" << endl;

            // Cleanup and terminate...
            g_object_unref(Context.m_Connection);
            g_clear_error(&Context.m_Error);
            exit(EXIT_FAILURE);
        }
}

// Name on the session bus has been obtained...
void DBusInterface::OnNameAcquired(
    GDBusConnection *,  /* Connection */
    const gchar *,      /* Name */
    gpointer)           /* UserData */
{

}

// The name on the bus or the connection itself has been lost...
void DBusInterface::OnNameLost(
    GDBusConnection *,  /* Connection */
    const gchar *,      /* Name */
    gpointer UserData)
{
    // Retrieve this pointer from user data...
    DBusInterface &Context(*static_cast<DBusInterface *>(UserData));

    // Unregister our interface, if registered...
    if(Context.m_RegistrationID)
        g_dbus_connection_unregister_object(
            Context.m_Connection, Context.m_RegistrationID);
}

// Register on the session bus...
void DBusInterface::RegisterOnSessionBus()
{
    // Alert user...
    Message(Console::Info) << "d-bus: registering on the session bus..." << endl;

    // Clear the bus error variable, if there is one...
    g_clear_error(&m_Error);

    // Initialize the introspection data...
    m_IntrospectionData = g_dbus_node_info_new_for_xml(
        ms_IntrospectionXML, &m_Error);

        // Failed...
        if(!m_IntrospectionData)
        {
            // Alert user and terminate...
            Message(Console::Error) 
                << "d-bus: could not parse introspection XML (" 
                << m_Error->message << ")" << endl;
            g_clear_error(&m_Error);
            exit(EXIT_FAILURE);
        }

    // Open a connection the session bus...
    m_Connection = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, &m_Error);

        // Failed...        
        if(!m_Connection)
        {
            // Alert user and terminate...
            Message(Console::Error) 
                << "d-bus: could not connect with the session bus (" 
                << m_Error->message << ")" << endl;
            g_clear_error(&m_Error);
            exit(EXIT_FAILURE);
        }

    // Register our bus name...
    m_BusID = g_bus_own_name(
        G_BUS_TYPE_SESSION,
        m_BusName.c_str(),
        static_cast<GBusNameOwnerFlags>(
            G_BUS_NAME_OWNER_FLAGS_ALLOW_REPLACEMENT |
            G_BUS_NAME_OWNER_FLAGS_REPLACE),
        DBusInterface::OnBusAcquired,
        DBusInterface::OnNameAcquired,
        DBusInterface::OnNameLost,
        static_cast<gpointer>(this),
        NULL);
}

// Wait for a D-Bus signal before unblocking or throw an error...
void DBusInterface::WaitRemoteStart()
{
    // Wait for the remote start method to be called by the client...
    m_MainLoop = g_main_loop_new (NULL, FALSE);
    g_main_loop_run(m_MainLoop);

    // Cleanup the main loop...
    g_main_loop_unref(m_MainLoop);

    // It wasn't called, but that should have been the only acceptable reason
    //  why the loop would terminate...
    if(!m_RemoteStart)
    {
        // Alert user...
        Message(Console::Info) 
            << "d-bus: glib main loop quit unexpectedly..." << endl;

        // Cleanup and terminate...
        g_object_unref(m_Connection);
        g_clear_error(&m_Error);
        exit(EXIT_FAILURE);
    }
}

// Deconstructor...
DBusInterface::~DBusInterface()
{
    // Cleanup if a connection to the session bus is open...
    if(m_Connection)
    {
        // Now close the connection...
        g_object_unref(m_Connection);
        m_Connection = 0;
    }

    // Cleanup the introspection data, if allocated...
    if(m_IntrospectionData)
        g_dbus_node_info_unref(m_IntrospectionData);

    // If an error was set, clear it...
    g_clear_error(&m_Error);
}


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

    // Provided by Autoconf...
    #include <config.h>

    // Our headers...    
    #include "DBusInterface.h"
    #include "Console.h"
    
    // System headers...
    #include <cassert>
    #include <cstdlib>

// Using the standard namespace...
using namespace std;

// Default constructor...
DBusInterface::DBusInterface()
  : m_BusName("com.cartesiantheatre.VikingExtractorService"),
    m_ObjectPath("/com/cartesiantheatre/VikingExtractorObject"),
    m_Interface("com.cartesiantheatre.VikingExtractorInterface"),
    m_NotificationSignal("Message"),
    m_ProgressSignal("Progress"),
    m_Connection(NULL)
{

    // Clear the bus error variable...
    dbus_error_init(&m_Error);

    // Open a connection the session bus...
    m_Connection = dbus_bus_get(DBUS_BUS_SESSION, &m_Error);

        // Failed...        
        if(!m_Connection)
        {
            // Alert user and terminate...
            Message(Console::Error) 
                << "d-bus: could not connect with the session bus (" 
                << m_Error.message << ")" << endl;
            exit(EXIT_FAILURE);
        }

    // Register ourselves on the session bus...
    const int Result = dbus_bus_request_name(
        m_Connection, m_BusName.c_str(), DBUS_NAME_FLAG_REPLACE_EXISTING, &m_Error);

        // Failed...
        if(dbus_error_is_set(&m_Error))
        {
            // Alert user and terminate...
            Message(Console::Error) 
                << "d-bus: could not register service with session bus... (" 
                << m_Error.message << ")" << endl;

            // Cleanup and terminate...
            dbus_error_free(&m_Error);
            exit(EXIT_FAILURE);
        }

    // There's probably some other instance already running...
    if(DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != Result)
    {
        // Alert user and terminate...
        Message(Console::Error) 
            << "d-bus: already running instance detected..." << endl;

        // Cleanup and terminate...
        exit(EXIT_FAILURE);
    }
}

// Emit notification signal with string available to be displayed through GUI...
void DBusInterface::EmitNotificationSignal(const string &Notification)
{
    // Sanity check...
    assert(dbus_connection_get_is_connected(m_Connection));

    // Allocate signal...
    DBusMessage *Signal = dbus_message_new_signal(
        m_ObjectPath.c_str(), m_Interface.c_str(), m_NotificationSignal.c_str());

        // Failed...
        if(dbus_error_is_set(&m_Error))
        {
            // Alert user and terminate...
            Message(Console::Error) 
                << "d-bus: could not register service with session bus... ("
                << m_Error.message << ")" << endl;

            // Cleanup and terminate...
            dbus_error_free(&m_Error);
            exit(EXIT_FAILURE);
        }

    // Prepare arguments...
    const char *NotificationArgument = Notification.c_str();
    dbus_message_append_args(Signal, 
        DBUS_TYPE_STRING, &NotificationArgument,
        DBUS_TYPE_INVALID);

    // Emit the signal and flush the message queue...
    dbus_connection_send(m_Connection, Signal, NULL);
    dbus_connection_flush(m_Connection);

        // There was a problem...
        if(dbus_error_is_set(&m_Error))
        {
            // Alert user and terminate...
            Message(Console::Error)
                << "d-bus: could not emit message signal... (" 
                << m_Error.message << ")" << endl;

            // Cleanup...
            dbus_error_free(&m_Error);
        }

    // Dereference the signal message so D-Bus can free it...
    dbus_message_unref(Signal);
}

// Emit progress signal with progress clamped to [0.0, 100.0]...
void DBusInterface::EmitProgressSignal(const double Progress)
{
    // Sanity check...
    assert(dbus_connection_get_is_connected(m_Connection));

    // Allocate signal...
    DBusMessage *Signal = dbus_message_new_signal(
        m_ObjectPath.c_str(), m_Interface.c_str(), m_ProgressSignal.c_str());

        // Failed...
        if(dbus_error_is_set(&m_Error))
        {
            // Alert user and terminate...
            Message(Console::Error) 
                << "d-bus: could not register service with session bus... (" 
                << m_Error.message << ")" << endl;

            // Cleanup and terminate...
            dbus_error_free(&m_Error);
            exit(EXIT_FAILURE);
        }

    // Prepare arguments...
    dbus_message_append_args(Signal, 
        DBUS_TYPE_DOUBLE, &Progress,
        DBUS_TYPE_INVALID);

    // Emit the signal and flush the message queue...
    dbus_connection_send(m_Connection, Signal, NULL);
    dbus_connection_flush(m_Connection);

        // There was a problem...
        if(dbus_error_is_set(&m_Error))
        {
            // Alert user and terminate...
            Message(Console::Error)
                << "d-bus: could not emit initialized signal... (" 
                << m_Error.message << ")" << endl;

            // Cleanup...
            dbus_error_free(&m_Error);
        }

    // Dereference the signal message so D-Bus can free it...
    dbus_message_unref(Signal);
}

// Deconstructor...
DBusInterface::~DBusInterface()
{
    // If an error was set, clear it...
    if(dbus_error_is_set(&m_Error))
        dbus_error_free(&m_Error);
}


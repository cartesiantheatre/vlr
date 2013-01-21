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

// Default constructor...
DBusInterface::DBusInterface()
  : m_BusName("com.cartesiantheatre.VikingExtractorService"),
    m_ObjectPath("/com/cartesiantheatre/VikingExtractorObject"),
    m_Interface("com.cartesiantheatre.VikingExtractorInterface"),
    m_NotificationSignal("Notification"),
    m_ProgressSignal("Progress"),
    m_ReadySignal("Ready"),
    m_Connection(NULL)
{
    // Clear the bus error variable...
    dbus_error_init(&m_Error);

    // Register on the session bus...
    RegisterOnSessionBus();
}

// Emit notification signal with string available to be displayed through GUI 
//  or throw an error...
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

    // Signal does not require a reply...
    dbus_message_set_no_reply(Signal, true);

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

// Emit progress signal with progress clamped to [0.0, 100.0] or throw an error...
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

    // Signal does not require a reply...
    dbus_message_set_no_reply(Signal, true);

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

// Register on the session bus...
void DBusInterface::RegisterOnSessionBus()
{
    // Alert user...
    Message(Console::Info) << "d-bus: registering on the session bus..." << endl;

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
        m_Connection, m_BusName.c_str(), 
        DBUS_NAME_FLAG_ALLOW_REPLACEMENT | 
        DBUS_NAME_FLAG_REPLACE_EXISTING |
        DBUS_NAME_FLAG_DO_NOT_QUEUE, 
        &m_Error);

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

    // There's probably some other instance already running and it's not 
    //  ourselves...
    if(DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != Result &&
       DBUS_REQUEST_NAME_REPLY_ALREADY_OWNER != Result)
    {
        // Alert user and terminate...
        Message(Console::Error) 
            << "d-bus: already running instance detected that could not be replaced..." 
            << endl;

        // Cleanup and terminate...
        exit(EXIT_FAILURE);
    }
    
    // Add a rule so we can receive signals of interest...
    
        // Format rule...
        stringstream MatchRule;
        //MatchRule << "type='signal'," << "interface='" << m_Interface << "'";
        MatchRule << "type='signal'";

        // Register...
        dbus_bus_add_match(m_Connection, MatchRule.str().c_str(), &m_Error);
        dbus_connection_flush(m_Connection);

        // Failed...
        if(dbus_error_is_set(&m_Error))
        {
            // Alert user and terminate...
            Message(Console::Error) 
                << "d-bus: match error... (" 
                << m_Error.message << ")" << endl;

            // Cleanup and terminate...
            dbus_error_free(&m_Error);
            exit(EXIT_FAILURE);
        }
}

// Wait for a D-Bus signal before unblocking or throw an error...
void DBusInterface::WaitRemoteStart()
{
/*
<node>
    <interface name="org.freedesktop.DBus.Introspectable">
        <method name="Introspect">
            <arg direction="out" name="data" type="s"></arg>
        </method>
    </interface>
    <interface name="com.cartesiantheatre.VikingExtractorInterface">
        <signal name="Progress">
            <arg name="Fraction" type="d"></arg>
        </signal>
        <signal name="Notification">
            <arg name="Message" type="s"></arg>
        </signal>
    </interface>
</node>
*/

    // Flag toggled when we received successfully the Ready signal...
    bool Ready = false;

    // Keep checking the message queue, without hammering the CPU, until the
    //  Ready signal arrives...
    while(!Ready)
    {
        // Alert user...
        Message(Console::Info) 
            << "d-bus: awaiting ready signal, please wait..." << endl;

        // Block until next message becomes available and check for error...
        if(!dbus_connection_read_write(m_Connection, -1))
            throw string("d-bus: connection closed prematurely...");

        // Retrieve the message...
        DBusMessage *IncomingMessage = dbus_connection_pop_message(m_Connection);

            // Failed...
            if(!IncomingMessage)
            {
                // Alert user, then try again...
                Message(Console::Error) 
                    << "d-bus: empty message queue, trying again..." << endl;
                continue;
            }

        /* Which message name was it?
        const char *MessageNameTemp = dbus_message_get_member(IncomingMessage);
        const string MessageName(MessageNameTemp ? MessageNameTemp : "?");

        // Display info about the message...
        Message(Console::Info) 
            << "d-bus: received message " 
            << MessageName 
            << ", needs reply (" 
            << !dbus_message_get_no_reply(IncomingMessage) 
            << "), is signal ("
            << (DBUS_MESSAGE_TYPE_SIGNAL == dbus_message_get_type(IncomingMessage))
            << ")" << endl;*/
        
        /*if(!)
            Message(Console::Error) << "d-bus: MESSAGE EXPECTED A REPLY..." << endl;

        if(DBUS_MESSAGE_TYPE_SIGNAL != dbus_message_get_type(IncomingMessage))
        {
            dbus_message_unref(IncomingMessage);
            continue;
        }*/

        // Verify this was the Ready signal...
        Ready = dbus_message_is_signal(
            IncomingMessage, m_Interface.c_str(), m_ReadySignal.c_str());

            // Not the Ready signal, ignore...
            if(!Ready)
            {
                /* Something else. Alert user...
                Message(Console::Error) 
                    << "Unrecognized signal... (" 
                    << dbus_message_get_member(IncomingMessage) << "), skipping..." 
                    << endl;

                Message(Console::Error) 
                    << "d-bus: expected interface \"" 
                    << m_Interface
                    << "\", but got \""
                    << dbus_message_get_interface(IncomingMessage)
                    << "\""
                    << endl;

                DBusMessageIter Arguments;

                // read the parameters
                if(!dbus_message_iter_init(IncomingMessage, &Arguments))
                    Message(Console::Error) << "message has no arguments!" << endl;

                else if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&Arguments))
                    Message(Console::Error) << "argument is not string!" << endl;

                else
                {
                    char *sigvalue = NULL;
                    dbus_message_iter_get_basic(&Arguments, &sigvalue);
                    Message(Console::Error) << "got signal with value " << sigvalue << endl;
                }*/
            }

        // Cleanup...
        dbus_message_unref(IncomingMessage);
    }
}

// Deconstructor...
DBusInterface::~DBusInterface()
{
    // If an error was set, clear it...
    if(dbus_error_is_set(&m_Error))
        dbus_error_free(&m_Error);
}


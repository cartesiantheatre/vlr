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

    // D-Bus...
    #include <dbus/dbus.h>

// Using the standard namespace...
using namespace std;

// Default constructor...
DBusInterface::DBusInterface()
  : m_BusName("com.cartesiantheatre.VikingExtractorService"),
    m_ObjectPath("/com/cartesiantheatre/VikingExtractorObject"),
    m_Interface("com.cartesiantheatre.VikingExtractorInterface"),
    m_SignalInitialized("Initialized"),
    m_SignalProgress("Progress"),
    m_SignalTerminating("Terminating")
{
    Message(Console::Summary) << "DBusInterface::DBusInterface()" << endl;
}

// Deconstructor...
DBusInterface::~DBusInterface()
{
    Message(Console::Summary) << "DBusInterface::~DBusInterface()" << endl;
}


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

    // Sanity check...
    #ifndef VIKING_EXTRACTOR_USE_DBUS_INTERFACE
        #error "D-Bus interface was disabled at configure time. This header should not have been used."
    #endif

    // D-Bus C++ API...
    #include <dbus-c++/dbus.h>

// D-Bus VikingExtractor interface constants...

    // D-Bus service name...
    #define DBUS_SERVICE_NAME   "org.CartesianTheatre.VikingExtractor"
    
    // D-Bus object on the aforementioned service...
    #define DBUS_OBJECT         "/org/CartesianTheatre/VikingExtractor"

// Multiple include protection...
#endif


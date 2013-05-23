# VikingExtractor, to recover images from Viking Lander operations.
# Copyright (C) 2010-2013 Cartesian Theatre <info@cartesiantheatre.com>.
#
# Public discussion on IRC available at #avaneya (irc.freenode.net) or
# on the mailing list <avaneya@lists.avaneya.com>.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or 
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but 
# WITHOUT ANY WARRANTY; without even the implied warranty of 
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

"""Miscellaneous: Random routines that didn't belong anywhere else"""

# System imports...
from gi.repository import Gdk
from gi.repository import Gtk
from gi.repository import Gio
import os
import subprocess
import platform
import sys

# i18n...
import gettext
_ = gettext.gettext

# Privates...
_applicationName    = "Avaneya: Viking Lander Remastered DVD"
_versionMajor       = 0
_versionMinor       = 2
_versionShortString = "{0}.{1}".format(_versionMajor, _versionMinor)
_versionLongString  = "{0}, v{1}".format(_applicationName, _versionShortString)

# Get the machine architecture...
def getMachineArchitecture():

    # Retrieve...
    machineArchitecture = platform.machine()
    assert(machineArchitecture)
    
    # Adapt to Debian's sane nomenclature...
    if machineArchitecture == "x86_64":
        machineArchitecture = "amd64"
    elif machineArchitecture == "x86":
        machineArchitecture = "i386"

    # Return the machine architecture to the caller...
    return machineArchitecture

# Get the size of the monitor the caller's window is mostly visible on, or the
#  default monitor of the default screen of the root window if none given...
def getMonitorWithCursorSize():

    # Get the cursor position...
    display = Gdk.Display.get_default()
    (screen, cursorX, cursorY, rootWindow) = display.get_pointer()

    # Which monitor the user's cursor?
    monitorId = screen.get_monitor_at_point(cursorX, cursorY)

    # Get the geometry of that monitor...
    monitorRectangle = screen.get_monitor_geometry(monitorId)
    
    # Return the monitor's width and height...
    return (monitorRectangle.width, monitorRectangle.height)

# Get the operating system name...
def getOperatingSystem():

    # Retrieve...
    operatingSystem = platform.system()
    assert(operatingSystem)
    
    # Correct common misnomer. Kernel mistaken for entire OS...
    #  <https://www.gnu.org/gnu/gnu-linux-faq.html>
    if operatingSystem == "Linux":
        operatingSystem = "GNU"

    # Return operating system name to caller...
    return operatingSystem

# Get the application name and version...
def getApplicationName(): return _applicationName
def getLongVersionString(): return _versionLongString
def getShortVersionString(): return _versionShortString

# Check network connectivity to the internet and alert user if caller requests 
#  if no connection available...
def hasInternetConnection(alertUser, parent, unqueriableDefault):

    # D-Bus service name, object, and interface to the NetworkManager...
    NETWORK_MANAGER_DBUS_SERVICE_NAME   = "org.freedesktop.NetworkManager"
    NETWORK_MANAGER_DBUS_OBJECT_PATH    = "/org/freedesktop/NetworkManager"
    NETWORK_MANAGER_DBUS_INTERFACE      = "org.freedesktop.NetworkManager"

    # D-Bus NetworkManager state constants...
    NM_STATE_CONNECTED              = 3     # For 0.8 interface
    NM_STATE_CONNECTED_GLOBAL       = 70    # For 0.9 interface

    # Try to get the NetworkManager remote proxy object...
    try:
        
        # Bind to the remote object...
        networkManagerProxy = None
        networkManagerProxy = Gio.DBusProxy.new_for_bus_sync(
            Gio.BusType.SYSTEM,
            Gio.DBusProxyFlags.NONE, 
            None,
            NETWORK_MANAGER_DBUS_SERVICE_NAME,
            NETWORK_MANAGER_DBUS_OBJECT_PATH,
            NETWORK_MANAGER_DBUS_INTERFACE, 
            None)

        # Get the connection state...
        connectionState = networkManagerProxy.state()

    # Something went wrong, but most likely the service isn't available...
    except:
        print(_("Warning: NetworkManager service not found..."))
        return unqueriableDefault

    # Connected...
    if connectionState == NM_STATE_CONNECTED or \
       connectionState == NM_STATE_CONNECTED_GLOBAL:

        # Inform caller...
        return True
    
    # Not connected...
    else:

        # Alert user, if caller requested...
        if alertUser:
            messageDialog = Gtk.MessageDialog(
                parent, Gtk.DialogFlags.MODAL, Gtk.MessageType.ERROR, 
                Gtk.ButtonsType.OK, 
                _("You need a working internet connection in order to do this. "
                "You don't appear to have one right now."))
            messageDialog.run()
            messageDialog.destroy()

        # Inform caller...
        return False

# Open a file or URL as though the user had launched it themselves using their 
#  preferred application...
def launchResource(resourcePath):

    # Winblows...
    if platform.system().lower().find("win") == 0:
        os.startfile(resourcePath)

    # OS X...
    elif platform.system().lower().find("darwin") == 0: 
        subprocess.Popen(["open", format(resourcePath)])
    elif platform.system().lower().find("macosx") == 0: 
        subprocess.Popen(["open", format(resourcePath)])

    # Some other OS...
    else:

        # Try freedesktop approach common on most GNU machines...
        try:
            subprocess.Popen(["xdg-open", format(resourcePath)])
        except OSError:
            print(_("Cannot launch {0}. Platform unknown...").format(resourcePath))

# Open a directory as though the user had launched it themselves using their 
#  preferred shell...
def exploreDirectory(directory):

    # Winblows Explorer...
    if platform.system().lower().find("win") == 0:
        os.startfile(directory, "explore")

    # OS X Finder...
    elif platform.system().lower().find("darwin") == 0: 
        subprocess.Popen(["open", format(directory)])
    elif platform.system().lower().find("macosx") == 0: 
        subprocess.Popen(["open", format(directory)])

    # Some other OS...
    else:

        # Try freedesktop approach common on most GNU machines. This should open
        #  Thunar, Nautilus, or whatever their default shell happens to be...
        try:
            subprocess.Popen(["xdg-open", format(directory)])
        except OSError:
            print(_("Cannot explore {0}. Platform unknown...").format(directory))


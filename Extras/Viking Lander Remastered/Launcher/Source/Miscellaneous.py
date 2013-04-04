# VikingExtractor, to recover images from Viking Lander operations.
# Copyright (C) 2010-2013 Cartesian Theatre <kip@thevertigo.com>.
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

# Imports...
from gi.repository import Gdk
import os
import platform
import sys

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
    
    # Correct common misnomer...
    if operatingSystem == "Linux":
        operatingSystem = "GNU"

    # Return operating system name to caller...
    return operatingSystem

# Driver for testing monitor geometry detection...
if __name__ == '__main__':
    print(getMonitorWithCursorSize())


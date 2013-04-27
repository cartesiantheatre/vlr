#!/usr/bin/env python3
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

# System imports...
import gi
import signal
from gi.repository import GObject, Gdk

# Launcher...
from LauncherApp import LauncherApp

# Entry point...
if __name__ == '__main__':

    # Kill on keyboard interrupt...
    signal.signal(signal.SIGINT, signal.SIG_DFL)

    # Initialize threading environment...
    GObject.threads_init()
    Gdk.threads_init()

    # Start the launcher GUI...
    launcher = LauncherApp()
    launcher.run()


#!/usr/bin/env python3
# VikingExtractor, to recover images from Viking Lander operations.
# Copyright (C) 2010, 2011, 2012 Cartesian Theatre <kip@thevertigo.com>.
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

# Imports...
from gi.repository import Gtk, GObject

# Our support modules...
import Options

# Recovery window proxy class...
class RecoveryWindowProxy():

    # Constructor...
    def __init__(self, launcherApp):
        
        # Initialize...
        self._launcherApp = launcherApp
        self._assistant = launcherApp.assistant
        self._builder = launcherApp.builder

        # Find the window and needed widgets...
        self.recoveryWindow = self._builder.get_object("recoveryWindow")

        # Toggle fullscreen...
        self.recoveryWindow.fullscreen(True)

        # Connect the signals...
        #self.splashWindow.connect("key-press-event", self.onSplashPressed)



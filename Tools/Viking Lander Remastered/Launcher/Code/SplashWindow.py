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
import LauncherOptions

# Splash window proxy class...
class SplashWindowProxy():

    # Constructor...
    def __init__(self, launcherApp):

        # Initialize...
        self._launcherApp = launcherApp
        self._assistant = launcherApp.assistant
        self._builder = launcherApp.builder

        # Find the window widgets...
        self.splashWindow = self._builder.get_object("splashWindow")
        self.splashEventBox = self._builder.get_object("splashEventBox")
        
        # Connect the signals...
        self.splashWindow.connect("key-press-event", self.onSplashPressed)
        self.splashEventBox.connect("button-press-event", self.onSplashPressed)

    # Keyboard or mouse pressed on the splash......
    def onSplashPressed(self, *arguments):
        
        # Cleanup the window...
        self.splashTimerDone(None)

    # Show the splash window...
    def showSplash(self):

        # Set timer for three seconds, or zero time if disabled...
        if not LauncherOptions.getOptions().noSplash:
            self.splashTimeoutEvent = GObject.timeout_add(3000, self.splashTimerDone, None)
        else:
            self.splashTimeoutEvent = GObject.timeout_add(0, self.splashTimerDone, None)

        # Set it to be always on top...
        self.splashWindow.set_keep_above(True)

        # Display the window...
        self.splashWindow.show_all()

    # Destroy the splash window after splash timer elapses...
    def splashTimerDone(self, userData):

        # Destroy the splash window...
        self.splashWindow.destroy()
        
        # Make the main window visible now...
        self._assistant.show_all()

        # Kill the timer...
        GObject.source_remove(self.splashTimeoutEvent)
        return False


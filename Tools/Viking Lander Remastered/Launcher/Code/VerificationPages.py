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

from VerificationThread import VerificationThread

# NavigatorSplash superclass...
class VerificationPages():

    # Constructor...
    def __init__(self, assistant):
        print("NavigatorVerificationPage constructing...")

        # Initialize...
        self._assistant = assistant
        self.verificationThread = None

        # Find the window widgets...
        stopVerificationButton = self._assistant.builder.get_object("stopVerificationButton")
        
        # Connect the signals...
        stopVerificationButton.connect("pressed", self.onStopVerificationPressed)

    # Start the disc verification...
    def startDiscVerification(self):

        # Already running...
        if self.verificationThread and self.verificationThread.isAlive():
            print("Disc verification thread already in progress...")
            return

        # Allocate and start the thread...
        self.verificationThread = VerificationThread(self.builder)
        self.verificationThread.start()

    # User requested to stop disc verification...
    def onStopVerificationPressed(self, button):

        # Signal the thread to quit...
        self.verificationThread.setQuit()
        
        # Wait for it to quit...
        self.verificationThread.join()
        
        # Mark as dead...
        self.verificationThread = None
        
        # Alert user...
        messageDialog = Gtk.MessageDialog(
            self.assistant, Gtk.DialogFlags.MODAL, Gtk.MessageType.WARNING, 
            Gtk.ButtonsType.OK, "Disc verification was cancelled.")
        messageDialog.run()
        messageDialog.destroy()

        # Mark page as complete...
        self.assistant.set_page_complete(self.verificationProgressPageBox, True)
        
        # Advance to the next page...
        currentPageIndex = self.assistant.get_current_page()
        self.assistant.set_current_page(currentPageIndex + 1)


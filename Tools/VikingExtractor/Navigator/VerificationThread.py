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
import threading
import time
import hashlib
from gi.repository import Gtk, GObject

# This thread is responsible for verifying data integrity of the disc...
class VerificationThread(threading.Thread):

    # Constructor...
    def __init__(self, builder):

        # Call the base class's constructor...
        super(VerificationThread, self).__init__()

        # Initialize state...
        self._builder = builder
        self._assistant = self._builder.get_object("AssistantInstance")

        # When this is true, the thread is done...
        self._finished = False

        # Find the progress bar...
        self._integrityProgressBar = self._builder.get_object("integrityProgressBar")

    # Update the GUI. This callback is safe to update the GUI because it has
    #  been scheduled to execute safely...
    def updateGUI(self, fraction):

        # Update the progress bar...
        self._integrityProgressBar.set_fraction(fraction)

        # Done...
        if fraction >= 1.0:

            # Alert user...
            messageDialog = Gtk.MessageDialog(
                self._assistant, Gtk.DialogFlags.MODAL, Gtk.MessageType.INFO, 
                Gtk.ButtonsType.OK, 
                "Disc verification was successful. Your disc appears to be fully intact.")
            messageDialog.run()
            messageDialog.destroy()

            # Advance to the next page...
            currentPageIndex = self._assistant.get_current_page()
            self._assistant.set_current_page(currentPageIndex + 1)

        # Remove function from list of event sources and don't call again until 
        #  needed...
        return False

    # Thread entry point...
    def run(self):
        fraction = 0.0
        while not self._finished and fraction < 1.0:
            fraction += 0.01
            GObject.idle_add(self.updateGUI, fraction)
            time.sleep(0.1)

    # Quit the thread...
    def setQuit(self):
        self._finished = True


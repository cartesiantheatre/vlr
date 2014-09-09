# VikingExtractor, to recover images from Viking Lander operations.
# Copyright (C) 2010-2014 Cartesian Theatre <info@cartesiantheatre.com>.
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
from gi.repository import GLib, Gtk, Gdk, GObject, GdkPixbuf

# Our support modules...
import LauncherArguments

# Assistant proxy page base class...
from PageProxyBase import *

# Class containing behaviour for the disc verification prompt page...
class VerificationPromptPageProxy(PageProxyBase):

    # Constructor...
    def __init__(self, launcherApp):

        # Initialize...
        super(VerificationPromptPageProxy, self).__init__(launcherApp)

        # Find the window widgets...
        self._verificationImage = self._builder.get_object("verificationImage")
        self._yesToggle = self._builder.get_object("performVerificationCheckRadio")
        self._noToggle = self._builder.get_object("skipVerificationCheckRadio")

        # Add the verification prompt page to the assistant...
        self.registerPage(
            "verificationPromptPageBox", 
            _("Verification Prompt"), 
            Gtk.AssistantPageType.CONFIRM, 
            True)

        # Connect the signals...
        self._yesToggle.connect("toggled", self.onToggle)
        self._noToggle.connect("toggled", self.onToggle)

    # Apply button hit...
    def onApply(self):
        pass

    # Our pages in the assistent are being constructed, but not visible yet...
    def onPrepare(self):
        pass

    # Verification toggle...
    def onToggle(self, button, userData=None):
        
        # Check whether requested...
        toggled = self._yesToggle.get_active()

        # Verification requested...
        if toggled:
        
            # Change page type to requiring an Apply button...
            self._assistant.set_page_type(
                self.getPageInGroup(0), Gtk.AssistantPageType.CONFIRM)

        # Verification not requested...
        else:

            # Change page type to only require a Next button...
            self._assistant.set_page_type(
                self.getPageInGroup(0), Gtk.AssistantPageType.CONTENT)


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

# System imports...
from gi.repository import Gtk

# Confirm page proxy class...
class ConfirmPageProxy():

    # Constructor...
    def __init__(self, navigatorApp):
    
        # For debugging purposes...
        print("ConfirmPageProxy constructing...")

        # Initialize...
        self._assistant     = navigatorApp.assistant
        self._builder       = navigatorApp.builder

        # Add the confirm page to the assistant...
        self._confirmPageBox = self._builder.get_object("confirmPageBox")
        self._confirmPageBox.set_border_width(5)
        self._assistant.append_page(self._confirmPageBox)
        self._assistant.set_page_title(self._confirmPageBox, "Confirmation")
        self._assistant.set_page_type(self._confirmPageBox, Gtk.AssistantPageType.CONFIRM)
        self._assistant.set_page_complete(self._confirmPageBox, True)

    # Get the page box...
    def getPageBox(self):
        return self._confirmPageBox


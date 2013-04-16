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
from gi.repository import Gtk

# Select recovery page proxy class...
class SelectRecoveryPageProxy():

    # Constructor...
    def __init__(self, launcherApp):

        # Initialize...
        self._assistant     = launcherApp.assistant
        self._builder       = launcherApp.builder

        # Add the select recovery page to the assistant...
        self._selectRecoveryFolderPageBox = self._builder.get_object("selectRecoveryFolderPageBox")
        self._selectRecoveryFolderPageBox.set_border_width(5)
        self._assistant.append_page(self._selectRecoveryFolderPageBox)
        self._assistant.set_page_title(self._selectRecoveryFolderPageBox, "Select Recovery Folder")
        self._assistant.set_page_type(self._selectRecoveryFolderPageBox, Gtk.AssistantPageType.CONTENT)
        self._assistant.set_page_complete(self._selectRecoveryFolderPageBox, True)


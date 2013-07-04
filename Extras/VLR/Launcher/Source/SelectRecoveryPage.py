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

# Assistant proxy page base class...
from PageProxyBase import *

# Select recovery page proxy class...
class SelectRecoveryPageProxy(PageProxyBase):

    # Constructor...
    def __init__(self, launcherApp):

        # Initialize...
        super(SelectRecoveryPageProxy, self).__init__(launcherApp)

        # Add the select recovery page to the assistant...
        self.registerPage(
            "selectRecoveryFolderPageBox",
            _("Select Recovery Folder"),
            Gtk.AssistantPageType.CONTENT,
            True)

        # Shortcuts to our widgets...
        self._recoveryFolder = self._builder.get_object("recoveryFolderChooser")

    # Our page in the assistent is being constructed, but not visible yet...
    def onPrepare(self):

        # Default to user's desktop, if we can find it...
        desktop = os.path.join(os.path.expanduser('~'), 'Desktop')
        if os.path.isdir(desktop):
            self._recoveryFolder.set_current_folder(desktop)


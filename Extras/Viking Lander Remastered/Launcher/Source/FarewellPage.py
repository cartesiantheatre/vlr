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

# Our modules...
from Miscellaneous import *

# Assistant proxy page base class...
from PageProxyBase import *

# Farewell page proxy class...
class FarewellPageProxy(PageProxyBase):

    # Constructor...
    def __init__(self, launcherApp):

        # Initialize...
        super(FarewellPageProxy, self).__init__(launcherApp)

        # Find window and widgets...
        self._farewellPageBox = self._builder.get_object("farewellPageBox")
        exploreDataButton = self._builder.get_object("exploreDataButton")
        exploreSourceCodeButton = self._builder.get_object("exploreSourceCodeButton")

        # Add the farewell page to the assistant...
        self._farewellPageBox.set_border_width(5)
        self._assistant.append_page(self._farewellPageBox)
        self._assistant.set_page_title(self._farewellPageBox, "Farewell")
        self._assistant.set_page_type(self._farewellPageBox, Gtk.AssistantPageType.SUMMARY)
        self._assistant.set_page_complete(self._farewellPageBox, True)

        # Set the explore recovered data button's icon...
        exploreDataButtonImage = Gtk.Image()
        exploreDataButtonImage.set_from_stock(Gtk.STOCK_OPEN, Gtk.IconSize.BUTTON)
        exploreDataButton.set_image(exploreDataButtonImage)
        
        # Set the explore source code button's icon...
        exploreSourceCodeButtonImage = Gtk.Image()
        exploreSourceCodeButtonImage.set_from_stock(Gtk.STOCK_FIND, Gtk.IconSize.BUTTON)
        exploreSourceCodeButton.set_image(exploreSourceCodeButtonImage)

        # Connect the signals...
        exploreDataButton.connect("clicked", self.onExploreDataButtonPressed)
        exploreSourceCodeButton.connect("clicked", self.onExploreSourceCodeButton)

    # Explore recovered data button pressed...
    def onExploreDataButtonPressed(self, button):
        
        # Find the recovery folder from the widget back on the recovery page...
        recoveryFolder = self._builder.get_object("recoveryFolderChooser").get_filename()

        # Explore the folder through the platform's native shell...
        exploreDirectory(recoveryFolder)

    # Explore source code button pressed. Open browser...
    def onExploreSourceCodeButton(self, button):
        launchResource("https://bazaar.launchpad.net/~avaneya/avaneya/trunk/files/head:/Extras/Viking%20Lander%20Remastered/Extractor/")


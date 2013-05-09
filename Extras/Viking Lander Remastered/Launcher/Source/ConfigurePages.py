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
import os

# Assistant proxy page base class...
from PageProxyBase import *

# Configure pages proxy class...
class ConfigurePagesProxy(PageProxyBase):

    # Constructor...
    def __init__(self, launcherApp):

        # Initialize...
        super(ConfigurePagesProxy, self).__init__(launcherApp)

        # Add the configure intro page to the assistant...
        self._configureIntroPageBox = self._builder.get_object("configureIntroPageBox")
        self._configureIntroPageBox.set_border_width(5)
        self._assistant.append_page(self._configureIntroPageBox)
        self._assistant.set_page_title(self._configureIntroPageBox, "Configure Introduction")
        self._assistant.set_page_type(self._configureIntroPageBox, Gtk.AssistantPageType.CONTENT)
        self._assistant.set_page_complete(self._configureIntroPageBox, True)

        # Add the configure output layout page to the assistant...
        self._configureOutputLayoutPageBox = self._builder.get_object("configureOutputLayoutPageBox")
        self._configureOutputLayoutPageBox.set_border_width(5)
        self._assistant.append_page(self._configureOutputLayoutPageBox)
        self._assistant.set_page_title(self._configureOutputLayoutPageBox, "Configure Layout")
        self._assistant.set_page_type(self._configureOutputLayoutPageBox, Gtk.AssistantPageType.CONTENT)
        self._assistant.set_page_complete(self._configureOutputLayoutPageBox, True)

        # Add the configure recovery page to the assistant...
        self._configureRecoveryPageBox = self._builder.get_object("configureRecoveryPageBox")
        self._configureRecoveryPageBox.set_border_width(5)
        self._assistant.append_page(self._configureRecoveryPageBox)
        self._assistant.set_page_title(self._configureRecoveryPageBox, "Configure Recovery")
        self._assistant.set_page_type(self._configureRecoveryPageBox, Gtk.AssistantPageType.CONTENT)
        self._assistant.set_page_complete(self._configureRecoveryPageBox, True)

        # Add the configure filters page to the assistant...
        self._configureFiltersPageBox = self._builder.get_object("configureFiltersPageBox")
        self._configureFiltersPageBox.set_border_width(5)
        self._assistant.append_page(self._configureFiltersPageBox)
        self._assistant.set_page_title(self._configureFiltersPageBox, "Configure Filters")
        self._assistant.set_page_type(self._configureFiltersPageBox, Gtk.AssistantPageType.CONTENT)
        self._assistant.set_page_complete(self._configureFiltersPageBox, True)

        # Add the configure advanced page to the assistant...
        self._configureAdvancedPageBox = self._builder.get_object("configureAdvancedPageBox")
        self._configureAdvancedPageBox.set_border_width(5)
        self._assistant.append_page(self._configureAdvancedPageBox)
        self._assistant.set_page_title(self._configureAdvancedPageBox, "Configure Advanced")
        self._assistant.set_page_type(self._configureAdvancedPageBox, Gtk.AssistantPageType.CONTENT)
        self._assistant.set_page_complete(self._configureAdvancedPageBox, False)
        
        # Find widgets...
        self.directorizeBandTypeClassCheckButton = self._builder.get_object("directorizeBandTypeClassCheckButton")
        self.directorizeLocationCheckButton = self._builder.get_object("directorizeLocationCheckButton")
        self.directorizeMonthCheckButton = self._builder.get_object("directorizeMonthCheckButton")
        self.directorizeSolCheckButton = self._builder.get_object("directorizeSolCheckButton")
        self.examplePathLabel = self._builder.get_object("examplePathLabel")

        # Connect the signals...
        self.directorizeBandTypeClassCheckButton.connect("toggled", self.onDirectorizeToggle)
        self.directorizeLocationCheckButton.connect("toggled", self.onDirectorizeToggle)
        self.directorizeMonthCheckButton.connect("toggled", self.onDirectorizeToggle)
        self.directorizeSolCheckButton.connect("toggled", self.onDirectorizeToggle)

        # Update the example path...
        self._updateExamplePath()

    # Assistant has reached the end of its current page and is transitioning to
    #  this page, though it is not visible yet...
    def onPrepare(self):

        # Now that the page is about to be visible, mark as complete so the
        #  final button doesn't appear prematurely in the assistant...
        self._assistant.set_page_complete(self._configureAdvancedPageBox, True)

    # Any of the directorize buttons were toggled. Update the example path...
    def onDirectorizeToggle(self, toggleButton):
        self._updateExamplePath()

    # Get the configure advanced page box...
    def getConfigureAdvancedPageBox(self):
        return self._configureAdvancedPageBox

    # Update the example path...
    def _updateExamplePath(self):

        # Get the toggle states of the different directorize flags...
        directorizeBandType = self._builder.get_object("directorizeBandTypeClassCheckButton").get_active()
        directorizeLocation = self._builder.get_object("directorizeLocationCheckButton").get_active()
        directorizeMonth = self._builder.get_object("directorizeMonthCheckButton").get_active()
        directorizeSol = self._builder.get_object("directorizeSolCheckButton").get_active()

        # Format the example path...
        #Output/Utopia Planitia/Scorpius/Colour/778/
        examplePath = "e.g. "
        if directorizeLocation:
            examplePath += "Utopia Planitia/"
        if directorizeMonth:
            examplePath += "Scorpius/"
        if directorizeBandType:
            examplePath += "Colour/"
        if directorizeSol:
            examplePath += "778/"
        examplePath += "22D180.png"

        # Show it...
        self.examplePathLabel.set_text(examplePath)


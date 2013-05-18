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
        self.registerPage(
            "configureIntroPageBox", 
            "Configure Introduction", 
            Gtk.AssistantPageType.CONTENT, 
            True)

        # Add the configure output layout page to the assistant...
        self.registerPage(
            "configureOutputLayoutPageBox", 
            "Configure Layout", 
            Gtk.AssistantPageType.CONTENT, 
            True)

        # Add the configure recovery page to the assistant...
        self.registerPage(
            "configureRecoveryPageBox", 
            "Configure Recovery", 
            Gtk.AssistantPageType.CONTENT, 
            True)

        # Add the configure filters page to the assistant...
        self.registerPage(
            "configureFiltersPageBox", 
            "Configure Filters", 
            Gtk.AssistantPageType.CONTENT, 
            True)

        # Add the configure advanced page to the assistant...
        self.registerPage(
            "configureAdvancedPageBox", 
            "Configure Advanced", 
            Gtk.AssistantPageType.CONTENT, 
            False)

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
    #  these pages, though current one not visible yet...
    def onPrepare(self):

        # Get the current page's absolute index...
        currentPageIndex = self._assistant.get_current_page()
       
        # If it's the last configuration page, mark it as complete so final
        #  button isn't visible prematurely in the assistant...
        if currentPageIndex == self.getAbsoluteIndex(4):
            self._assistant.set_page_complete(self.getPageInGroup(4), True)

    # Any of the directorize buttons were toggled. Update the example path...
    def onDirectorizeToggle(self, toggleButton):
        self._updateExamplePath()

    # Update the example path...
    def _updateExamplePath(self):

        # Get the toggle states of the different directorize flags...
        directorizeBandType = self._builder.get_object("directorizeBandTypeClassCheckButton").get_active()
        directorizeLocation = self._builder.get_object("directorizeLocationCheckButton").get_active()
        directorizeMonth = self._builder.get_object("directorizeMonthCheckButton").get_active()
        directorizeSol = self._builder.get_object("directorizeSolCheckButton").get_active()

        # Format the example path...
        #Output/Utopia Planitia/Scorpius/Colour/778/
        annotatedPath = ""
        if directorizeLocation:
            annotatedPath += "Location/"
        if directorizeMonth:
            annotatedPath += "Martian Month/"
        if directorizeBandType:
            annotatedPath += "Sensor Band Type/"
        if directorizeSol:
            annotatedPath += "Mission Solar Day/"
        annotatedPath += "Photograph.png"

        # Show it...
        self.examplePathLabel.set_markup(
            "e.g. <tt>{0}</tt>".format(annotatedPath))


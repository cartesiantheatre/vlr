#!/usr/bin/env python3
# VikingExtractor, to recover images from Viking Lander operations.
# Copyright (C) 2010-2017 Cartesian Theatre™ <info@cartesiantheatre.com>.
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
import re

# Our support modules...
import LauncherArguments

# Assistant proxy page base class...
from PageProxyBase import *

# Confirm page proxy class...
class ConfirmPageProxy(PageProxyBase):

    # Constructor...
    def __init__(self, launcherApp):
    
        # Initialize...
        super(ConfirmPageProxy, self).__init__(launcherApp)

        # Add the confirm page to the assistant...
        self.registerPage(
            "confirmPageBox",
            _("Confirmation"),
            Gtk.AssistantPageType.CONFIRM,
            True)

        # List that will contain all VikingExtractor command line options...
        self._commandLineInterface = []

    # Get the arguments to pass to VikingExtractor...
    def getVikingExtractorArguments(self):
        return self._commandLineInterface

    # Get the page box...
    def getPageBox(self):
        return self._confirmPageBox

    # Apply button hit...
    def onApply(self):
        
        # Start the recovery...
        self._launcher.recoveryPageProxy.startRecovery()

    # Assistant has reached the end of its current page and is transitioning to
    #  this page, though it is not visible yet. Prepare the VikingExtractor 
    #  arguments based on user selected configuration and show a summary of the
    #  user's selected configuration...
    def onPrepare(self):

        # List that will contain all VikingExtractor command line options...
        self._commandLineInterface = []

        # Get the confirmation text buffer...
        confirmTextBuffer = self._builder.get_object("confirmTextBuffer")
        
        # Clear it, if not already...
        confirmTextBuffer.set_text("")

        # Recovery folder...
        self.recoveryFolder = self._builder.get_object("recoveryFolderChooser").get_filename()
        confirmTextBuffer.insert(confirmTextBuffer.get_end_iter(), 
            _("Save to: {0}\n").format(self.recoveryFolder))

        # Overwrite...
        active = self._builder.get_object("overwriteOutputCheckButton").get_active()
        confirmTextBuffer.insert(confirmTextBuffer.get_end_iter(), 
            _("Output will be overwritten: {0}\n").format(_boolToYesNo(active)))
        if active:
            self._commandLineInterface.append("--overwrite")

        # Directorize by...
        confirmTextBuffer.insert(confirmTextBuffer.get_end_iter(), _("Directorize by...\n"))
        
        # ...diode band class...
        active = self._builder.get_object("directorizeBandTypeClassCheckButton").get_active()
        confirmTextBuffer.insert(confirmTextBuffer.get_end_iter(), 
            _("\t...diode band class: {0}\n").format(_boolToYesNo(active)))
        if active:
            self._commandLineInterface.append("--directorize-band-class")

        # ...Martian location...
        active = self._builder.get_object("directorizeLocationCheckButton").get_active()
        confirmTextBuffer.insert(confirmTextBuffer.get_end_iter(), 
            _("\t...Martian location: {0}\n").format(_boolToYesNo(active)))
        if active:
            self._commandLineInterface.append("--directorize-location")

        # ...Martian month...
        active = self._builder.get_object("directorizeMonthCheckButton").get_active()
        confirmTextBuffer.insert(confirmTextBuffer.get_end_iter(), 
            _("\t...Martian month: {0}\n").format(_boolToYesNo(active)))
        if active:
            self._commandLineInterface.append("--directorize-month")

        # ...mission solar day...
        active = self._builder.get_object("directorizeSolCheckButton").get_active()
        confirmTextBuffer.insert(confirmTextBuffer.get_end_iter(), 
            _("\t...mission solar day: {0}\n").format(_boolToYesNo(active)))
        if active:
            self._commandLineInterface.append("--directorize-sol")

        # No automatic rotation...
        active = self._builder.get_object("noAutomaticRotationCheckButton").get_active()
        confirmTextBuffer.insert(confirmTextBuffer.get_end_iter(), 
            _("Disable automatic image component orientation: {0}\n").format(_boolToYesNo(active)))
        if active:
            self._commandLineInterface.append("--no-auto-rotate")

        # No reconstruction...
        active = self._builder.get_object("noReconstructionCheckButton").get_active()
        confirmTextBuffer.insert(confirmTextBuffer.get_end_iter(), 
            _("Dump components without reconstruction: {0}\n").format(_boolToYesNo(active)))
        if active:
            self._commandLineInterface.append("--no-reconstruct")

        # Filters...
        confirmTextBuffer.insert(confirmTextBuffer.get_end_iter(), _("Filters...\n"))

        # ...band type class...
        diodeFilterComboBox = self._builder.get_object("diodeFilterComboBox")
        
        iterator = diodeFilterComboBox.get_active_iter()
        
        model = diodeFilterComboBox.get_model()
        humanValue = model.get_value(iterator, 1)
        commandLineValue = model.get_value(iterator, 1)

        confirmTextBuffer.insert(confirmTextBuffer.get_end_iter(), 
            _("\t...diode band type class: {0}\n").format(model.get_value(iterator, 0)))
        self._commandLineInterface.append("--filter-diode={0}".format(commandLineValue))
        
        # ...lander...
        landerFilterComboBox = self._builder.get_object("landerFilterComboBox")
        
        iterator = landerFilterComboBox.get_active_iter()
        
        model = landerFilterComboBox.get_model()
        humanValue = model.get_value(iterator, 0)
        humanValue = re.sub('<[^>]*>', '', humanValue)
        commandLineValue = model.get_value(iterator, 1)

        confirmTextBuffer.insert(confirmTextBuffer.get_end_iter(), 
            _("\t...lander: {0}\n").format(humanValue))
        self._commandLineInterface.append("--filter-lander={0}".format(commandLineValue))

        # Use image interlacing...
        active = self._builder.get_object("useInterlacingCheckButton").get_active()
        confirmTextBuffer.insert(confirmTextBuffer.get_end_iter(), 
            _("Interlace images: {0}\n").format(_boolToYesNo(active)))
        if active:
            self._commandLineInterface.append("--interlace")

        # Generate metadata...
        active = self._builder.get_object("generateMetadataCheckButton").get_active()
        confirmTextBuffer.insert(confirmTextBuffer.get_end_iter(), 
            _("Generate metadata: {0}\n").format(_boolToYesNo(active)))
        if active:
            self._commandLineInterface.append("--generate-metadata")

        # Verbosity...
        active = self._builder.get_object("verbosityCheckButton").get_active()
        confirmTextBuffer.insert(confirmTextBuffer.get_end_iter(), 
            _("Be verbose: {0}\n").format(_boolToYesNo(active)))
        if active:
            self._commandLineInterface.append("--verbose")

        # Multithreading...
        active = self._builder.get_object("multithreadingCheckButton").get_active()
        confirmTextBuffer.insert(confirmTextBuffer.get_end_iter(), 
            _("Use multithreading: {0}\n").format(_boolToYesNo(active)))
        if active:
            self._commandLineInterface.append("--jobs")

        # Other switches implicitly added...
        self._commandLineInterface.append("--recursive")
        self._commandLineInterface.append("--ignore-bad-files")
        self._commandLineInterface.append("--remote-start")
        self._commandLineInterface.append("--suppress")

        # Location of input...
        self._commandLineInterface.append(
            LauncherArguments.getArguments().missionDataRoot)

        # Location of output...
        self._commandLineInterface.append(self.recoveryFolder)
        
        # Display the full command line in the confirmation summary window...
        commandLine = " ".join(self._commandLineInterface)
        confirmTextBuffer.insert(confirmTextBuffer.get_end_iter(), 
            _("VikingExtractor arguments: {0}\n").format(commandLine))

# Helper function takes a boolean value and converts to a yes or no string...
def _boolToYesNo(value):
    
    # Type check...
    assert(type(value) is bool)

    # Convert to string...
    if value is True:
        return _("Yes")
    else:
        return _("No")


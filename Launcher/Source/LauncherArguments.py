# VikingExtractor, to recover images from Viking Lander operations.
# Copyright (C) 2010-2018 Cartesian Theatre™ <info@cartesiantheatre.com>.
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

"""LauncherArguments: Contains routines for parsing command line switches to the launcher"""

# Imports...
from gi.repository import Gtk, Gdk, GObject, GLib
import argparse
import os
import platform
import sys
from Miscellaneous import *

# i18n...
import gettext
_ = gettext.gettext

# Privates...
_arguments = None

# Parse command line...
def _initializeArguments():

    # For debugging purposes...
    #print("Initializing {0} module...".format(__name__))

    # Initialize the argument parser...
    argumentParser = argparse.ArgumentParser(
        description=_("Graphical user interface for the Avaneya: Viking Lander Remastered..."))
    
    # Define behaviour for --data-root...
    argumentParser.add_argument("--data-root", 
        action="store", 
        default=os.path.normpath(os.path.join(_getSourceDirectory(), "../Data/")),
        dest="dataRoot", 
        help=_("Override path to user interface data root."))

    # Define behaviour for --mission-data-root...
    argumentParser.add_argument("--mission-data-root", 
        action="store", 
        default=os.path.normpath(os.path.join(_getSourceDirectory(), "../Mission Data/")),
        dest="missionDataRoot", 
        help=_("Override path to Viking lander mission data root."))

    # Define behaviour for --no-splash...
    argumentParser.add_argument("--no-splash", 
        action="store_true", 
        dest="noSplash", 
        help=_("Do not show the splash and opening video."), 
        default=False)

    # Define behaviour for --no-music...
    argumentParser.add_argument("--no-background-music", 
        action="store_true", 
        dest="noBackgroundMusic", 
        help=_("Do not play background music."), 
        default=False)

    # Define behaviour for --version...
    argumentParser.add_argument("--version", 
        action="version", 
        version=print(getLongVersionString()),
        help=_("Display application version."))

    # Define behaviour for --viking-extractor-bin...
    argumentParser.add_argument("--viking-extractor-bin", 
        action="store", 
        default=_getDefaultVikingExtractorPath(),
        dest="vikingExtractorBinaryPath", 
        help=_("Override path to VikingExtractor executable."))

    # Parse the command line...
    global _arguments
    _arguments = argumentParser.parse_args()

    # Check if the resulting path to the VikingExtractor is valid...
    extractorPath = getArguments().vikingExtractorBinaryPath
    if not os.path.isfile(extractorPath):

        # Show debugging info...
        print(_("Warning: No executable detected... \"{0}\"").
            format(os.path.abspath(extractorPath)))

        # Alert user...
        messageDialog = Gtk.MessageDialog(
            None, Gtk.DialogFlags.MODAL, Gtk.MessageType.ERROR, 
            Gtk.ButtonsType.OK, 
            "Avaneya: Viking Lander Remastered")
        messageDialog.format_secondary_text(
          _("Unfortunately your platform is not yet supported. A required "
            "executable was not detected.\n\n"
            "\tOperating System: {0}\n"
            "\tMachine Architecture: {1}").
                format(getOperatingSystem(), getMachineArchitecture()))
        messageDialog.run()
        messageDialog.destroy()
        
        # Terminate...
        exit(1)

# Return the arguments object to the caller...
def getArguments():
    
    # Check assumptions...
    assert(_arguments is not None)

    # Return the object...
    return _arguments

# Create the default value for the path to an appropriate VikingExtractor binary
#  for the user's platform...
def _getDefaultVikingExtractorPath():

    # Get the operating system name...
    operatingSystem = getOperatingSystem()

    # Get the machine architecture and make it sane...
    machineArchitecture = getMachineArchitecture()

    # Get the executable name...
    executableName = "viking-extractor"
    if operatingSystem == "Windows":
        executableName += ".exe"

    # Construct a path to an appropriate executable...
    extractorPath = os.path.abspath(os.path.join(
        _getSourceDirectory(), "../../Extractor/", getOperatingSystem(), 
        getMachineArchitecture(), executableName))

    # Dead. Probably no executable built for this platform. Return nothing 
    #  because this is an error...
    if not os.path.isfile(extractorPath):
        return ""

    # Otherwise, return the path to the caller...
    else:
        return extractorPath

# Return the absolute path to the directory containing this source file...
def _getSourceDirectory():

    # Calculate paths to our needed files...
    # TODO: Implement this. Remember that __file__ may not be available if
    #        running via py2exe. In which case use os.path.dirname(sys.argv[0])
    #print(os.path.realpath(__file__))

    # Get the directory containing this source file and handle the case where
    #  realpath will fail because running through py2exe...
    sourceFile = os.path.realpath(__file__)
    if len(sourceFile) == 0:
        sourceFile = os.path.dirname(sys.argv[0])
    sourceDirectory = os.path.dirname(sourceFile)
    
    # Return to caller...
    return sourceDirectory

# Called on module initialization...
_initializeArguments()


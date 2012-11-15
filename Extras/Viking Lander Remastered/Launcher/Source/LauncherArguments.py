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

"""LauncherArguments: Contains routines for parsing command line switches to the launcher"""

# Imports...
import argparse
import os
import sys

# Privates...
_arguments    = None
_arguments  = None

# Parse command line...
def _initializeArguments():

    # For debugging purposes...
    #print("Initializing {0} module...".format(__name__))

    # Initialize the argument parser...
    argumentParser = argparse.ArgumentParser(
        description="Graphical user interface for the Avaneya: Viking Lander Remastered...")
    
    # Define behaviour for --no-splash...
    argumentParser.add_argument("--no-splash", 
        action="store_true", 
        dest="noSplash", 
        help="Do not show the splash screen.", 
        default=False)

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

    # Define behaviour for --glade-xml...
    argumentParser.add_argument("--glade-xml", 
        action="store", 
        default=os.path.normpath(os.path.join(sourceDirectory, "../Data/Launcher.glade")),
        dest="gladeXMLPath", 
        help="Path to Glade user interface XML file.")

    # Define behaviour for --mission-data-root...
    argumentParser.add_argument("--mission-data-root", 
        action="store", 
        default=os.path.normpath(os.path.join(sourceDirectory, "../Mission Data/")),
        dest="missionDataRoot", 
        help="Path to Viking lander mission data root.")

    # Define behaviour for --viking-extractor-bin...
    argumentParser.add_argument("--viking-extractor-bin", 
        action="store", 
        default=os.path.normpath(os.path.join(sourceDirectory, "../Executables/viking-extractor")),
        dest="vikingExtractorBinaryPath", 
        help="Path to VikingExtractor executable.")

    # Parse the command line...
    global _arguments
    _arguments = argumentParser.parse_args()

# Return the arguments object to the caller...
def getArguments():
    
    # Check assumptions...
    assert(_arguments is not None)

    # Return the object...
    return _arguments

# Called on module initialization...
_initializeArguments()


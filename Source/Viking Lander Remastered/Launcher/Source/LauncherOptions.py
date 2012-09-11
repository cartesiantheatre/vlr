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

"""LauncherOptions: Contains routines for parsing command line switches to the launcher"""

# Imports...
import optparse

# Privates...
_options    = None
_arguments  = None

# Parse command line...
def _initializeOptions():

    # For debugging purposes...
    #print("Initializing {0} module...".format(__name__))

    # Initialize the option parser...
    optionParser = optparse.OptionParser()
    
    # Define behaviour for --no-splash...
    optionParser.add_option("--no-splash", 
        action="store_true", 
        dest="noSplash", 
        help="Do not show the splash screen.", 
        default=False)
    
    # Parse the command line...
    global _options
    global _arguments
    (_options, _arguments) = optionParser.parse_args()

# Return the options object to the caller...
def getOptions():
    
    # Check assumptions...
    assert(_options is not None)

    # Return the object...
    return _options

# Called on module initialization...
_initializeOptions()


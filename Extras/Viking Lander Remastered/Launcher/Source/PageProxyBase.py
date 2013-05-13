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

# Arguments...
import LauncherArguments

# Page pages proxy class. Common code for all assistant pages...
class PageProxyBase(object):

    # Constructor...
    def __init__(self, launcherApp):

        # Initialize...
        self._launcher  = launcherApp
        self._assistant = launcherApp.assistant
        self._builder   = launcherApp.builder

    # Decorate the page with the common features to all assistant pages...
    def decoratePage(self, sizer):
        
        # Insert banner at top...
        banner = Gtk.Image()
        banner.set_from_file(
            os.path.join(
                LauncherArguments.
                    getArguments().dataRoot, "CartesianTheatre.png"))
        sizer.pack_start(banner, False, False, 0)
        sizer.reorder_child(banner, 0)


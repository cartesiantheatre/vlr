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

# Page pages proxy class. Common code for all assistant pages...
class PageProxyBase(object):

    # Constructor...
    def __init__(self, launcherApp):

        # Initialize...
        self._launcher  = launcherApp
        self._assistant = launcherApp.assistant
        self._builder   = launcherApp.builder

    def decoratePage(self, sizer):
        
        #sizer.pack_start(Gtk.Button(stock=Gtk.STOCK_OPEN), True, True, 0)
        pass
        
        # Add the header image...
        #animation = GdkPixbuf.PixbufAnimation.new_from_file(
        #    os.path.join(
        #        LauncherArguments.getArguments().dataRoot, "Animations", 
        #        "Verification", "Verification.gif"))
        #self._verificationImage.set_from_animation(animation)


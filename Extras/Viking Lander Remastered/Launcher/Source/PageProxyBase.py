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
        self._pages     = []

    # Decorate the page with the common features to all assistant pages...
    def decoratePage(self, page):
        
        # Insert banner at top...
        #banner = Gtk.Image()
        #banner.set_from_file(
        #    os.path.join(
        #        LauncherArguments.
        #            getArguments().dataRoot, "CartesianTheatre.png"))
        #sizer.pack_start(banner, False, False, 0)
        #sizer.reorder_child(banner, 0)
        pass

    # Get the page's absolute index in the assistant...
    def getAbsoluteIndex(self, groupIndex):
        page, absoluteIndex = self._pages[groupIndex]
        return absoluteIndex

    # Get a page by index for this specific group of pages...
    def getPageInGroup(self, groupIndex):
        page, absoluteIndex = self._pages[groupIndex]
        return page

    # Apply button was hit. Needs to be overridden in CONFIRM type assistant
    #  pages...
    def onApply(self):
        raise NotImplementedError()

    # End of current page. Next page is being constructed but not visible yet.
    #  Give it a chance to prepare if overridden...
    def onPrepare(self):
        pass

    # Register the page with the assistant...
    def registerPage(self, objectName, title, pageType, complete):
        
        # Find the page...
        page = self._builder.get_object(objectName)
        assert(page)
        
        # Add a reference to this proxy handler in the Gtk sizer object...
        page.pageProxyBase = self
        
        # Decorate the page...
        self.decoratePage(page)

        # Add subscribe page to assistant...
        absoluteIndex = self._assistant.append_page(page)
        
        # Add it to internal list...
        entry = (page, absoluteIndex)
        self._pages.append(entry)

        # Customize the page...
        #page.set_border_width(5)
        self._assistant.set_page_title(page, title)
        self._assistant.set_page_type(page, pageType)
        self._assistant.set_page_complete(page, complete)


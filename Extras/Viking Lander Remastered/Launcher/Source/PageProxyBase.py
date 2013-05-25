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
from gi.repository import GdkPixbuf
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
        
        # Load the banner image pixels...
        self._bannerPixelBuffer = GdkPixbuf.Pixbuf.new_from_file(
            os.path.join(
                LauncherArguments.
                    getArguments().dataRoot, "Banner.png"))

        # Fetch the image dimensions and aspect ratio...
        self._bannerWidth = self._bannerPixelBuffer.get_width()
        self._bannerHeight = self._bannerPixelBuffer.get_height()
        self._bannerAspect = self._bannerWidth / self._bannerHeight

        # Set the image from the pixel buffer, rescaling later...
        self._bannerImage = Gtk.Image()
        self._bannerImage.set_from_pixbuf(self._bannerPixelBuffer)
        self._bannerImage.set_alignment(0.5, 0.0)

        # Put the banner image inside of a viewport...
        self._viewport = Gtk.Viewport()
        self._viewport.add(self._bannerImage)

        # ...in turn inside of a scrolled window...
        self._scrolledWindow = Gtk.ScrolledWindow()
        self._scrolledWindow.set_policy(
            Gtk.PolicyType.AUTOMATIC, 
            Gtk.PolicyType.AUTOMATIC)
        self._scrolledWindow.add(self._viewport)

        # We need to update the image size every time the page is resized, and 
        #  consequently, redrawn... (Gtk+ 3 'expose-event' -> 'draw')
        self._viewport.connect("draw", self.onBannerImageResize, page)

        # Add the image to the top of the GUI...
        page.pack_start(self._scrolledWindow, False, False, 0)
        page.reorder_child(self._scrolledWindow, 0)

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

    # We need to update the image size every time the page is resized, and 
    #  consequently, redrawn...
    def onBannerImageResize(self, imageWidget, event, page):

        # Get the new size of the image widget...
        allocation = imageWidget.get_allocation()
        
        # If the requested dimensions are different than what's in the pixel
        #  buffer, then resize...
        if self._bannerWidth != allocation.width:

            # Remember new banner dimensions...
            self._bannerWidth = allocation.width
            self._bannerHeight = allocation.width / self._bannerAspect
            
            # Now rescale it to fit the GUI using the best tradeoff between speed
            #  and quality...
            rescaledBannerPixelBuffer = self._bannerPixelBuffer.scale_simple(
                self._bannerWidth, 
                self._bannerHeight, 
                GdkPixbuf.InterpType.BILINEAR)
            
            # Update the image in the widget with the rescaled one...
            self._bannerImage.set_from_pixbuf(rescaledBannerPixelBuffer)

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


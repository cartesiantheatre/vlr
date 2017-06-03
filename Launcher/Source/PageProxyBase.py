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
from gi.repository import Gtk, Gdk, GdkPixbuf
import os
import cairo

# Arguments...
import LauncherArguments

# A rescalable image that fits the allocation while maintaining its aspect 
#  ratio...
class ScalableImage(Gtk.DrawingArea):

    # Constructor...
    def __init__(self, filename):
    
        # Initialize GtkDrawingArea base class...
        super(ScalableImage, self).__init__()
        
        # Load the pixels from the requested file...
        self._pixelBuffer = GdkPixbuf.Pixbuf.new_from_file(filename)

    # Override the preferred width...
    def do_get_preferred_width(self):
        naturalWidth = self._pixelBuffer.get_width()
        return (0, naturalWidth)

    # Override the preferred height...
    def do_get_preferred_height(self):
        naturalHeight = self._pixelBuffer.get_height()
        return (0, naturalHeight)

    # Note here that the minimum request is set to the natural height of the 
    #  input pixel buffer. This may not be the desired behavior in all
    #  circumstances...
    def do_get_preferred_height_for_width(self, width):
        preferredHeight = width / self.get_aspect_ratio()
        return (0, preferredHeight)

    # Prefer a height for width resize mode...
    def do_get_request_mode(self):
        return Gtk.SizeRequestMode.HEIGHT_FOR_WIDTH

    # Get the aspect ratio of the image...
    def get_aspect_ratio(self):
        return self._pixelBuffer.get_width() / self._pixelBuffer.get_height()

    # Render the image onto the Cairo context...
    def do_draw(self, cairoContext):
        
        # Get the natural dimensions of the original image...
        pixelBufferWidth    = self._pixelBuffer.get_width()
        pixelBufferHeight   = self._pixelBuffer.get_height()

        # Get allocation for drawing area widget in the precision needed...
        allocation          = self.get_allocation()
        allocationWidth     = float(allocation.width)
        allocationHeight    = float(allocation.height)

        # Modify the transformation matrix to rescale the image. When scaling an 
        #  image, both its height and width are changing. In order to fit the 
        #  allocated rectangle without clipping, one must figure out which 
        #  dimension is smallest (normalized to the aspect ratio). For example, 
        #  if a 200x100 image is allocated 200x50, the allocation is 
        #  height-limited and the image must be scaled to 100x50. OTOH if 
        #  allocated 100x100, the image is width-limited and must be scaled to 
        #  100x50. Thanks Andrew Potter...
        rescale = min(allocationWidth / pixelBufferWidth, allocationHeight / pixelBufferHeight)
        cairoContext.scale(rescale, rescale)

        # Reload the pixels into the rendering context and repaint...
        Gdk.cairo_set_source_pixbuf(cairoContext, self._pixelBuffer, 0.0, 0.0)
        cairoContext.paint()
        
        # Allow the event to propagate further...
        return False

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

        # Create a drawing area to render the banner image onto...
        page._bannerDrawingArea = ScalableImage(
            os.path.join(LauncherArguments.getArguments().dataRoot, "Banner.png"))

        # Add the image to the top of the GUI...
        page.pack_start(page._bannerDrawingArea, False, False, 5)
        page.reorder_child(page._bannerDrawingArea, 0)

        # Centre its horizontal alignment...
        page._bannerDrawingArea.set_halign(Gtk.Align.CENTER)

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
        
        # Add a reference to this base proxy handler in the Gtk sizer object...
        page.pageProxyBase = self
        
        # Decorate the page...
        self.decoratePage(page)

        # Add page to assistant...
        absoluteIndex = self._assistant.append_page(page)
        
        # Add it to internal list...
        entry = (page, absoluteIndex)
        self._pages.append(entry)

        # Customize the page...
        #page.set_border_width(5)
        self._assistant.set_page_title(page, title)
        self._assistant.set_page_type(page, pageType)
        self._assistant.set_page_complete(page, complete)


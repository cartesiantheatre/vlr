#!/usr/bin/env python3
# VikingExtractor, to recover images from Viking Lander operations.
# Copyright (C) 2010-2014 Cartesian Theatre <info@cartesiantheatre.com>.
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
import os
import sys

# i18n...
import gettext
_ = gettext.gettext

# GStreamer...
haveGStreamer = True
try:
    import gi
    gi.require_version("GstVideo", "1.0")
    from gi.repository import GstVideo, Gst
    haveGStreamer = True
except ValueError:
    print(_("GStreamer >= 1.0 not found. Will not use for opening video..."))
    haveGStreamer = False

# Gtk...
from gi.repository import GObject, Gtk, Gdk

# This is a hack to be able to use the set_window_handle...
try:
    from gi.repository import GdkX11, GstVideo
except ImportError:
    pass

# Our support modules...
import LauncherArguments
from SplashWindow import SplashWindowProxy
from Miscellaneous import *

# Opening video window proxy class...
class OpeningVideoWindowProxy(object):

    # Constructor...
    def __init__(self, launcherApp):

        # Initialize...
        self._launcherApp = launcherApp
        self._assistant = launcherApp.assistant
        self._builder = launcherApp.builder

        # Find the window widgets...
        self._videoWindow = self._builder.get_object("openingVideoWindow")
        self._videoEventBox = self._builder.get_object("openingVideoEventBox")
        self._videoDrawingArea = self._builder.get_object("openingVideoDrawingArea")
        
        # Connect the Gtk+ signals...
        self._videoWindow.connect("key-press-event", self.onVideoPressed)
        self._videoEventBox.connect("button-press-event", self.onVideoPressed)
        self._videoDrawingArea.connect('realize', self.onDrawingAreaRealized)

        # Initialize GStreamer, if GStreamer is available...
        if haveGStreamer:
            Gst.init(None)

        # Otherwise just silently skip the opening video...
        else:
            self.videoDone()
            return

        # Create the GStreamer bin containing needed elements...
        self._playBin = None
        self._playBin = Gst.ElementFactory.make("playbin", "MultimediaPlayer")
        assert(self._playBin)

        # Get the bus for the GStreamer pipeline and prepare signal handlers
        #  for asynchronous and synchronous messages...
        bus = self._playBin.get_bus()
        bus.add_signal_watch()
        bus.enable_sync_message_emission()
        bus.connect("message", self.onBusMessage)
        bus.connect("sync-message::element", self.onBusSyncMessage)

        # Create the GStreamer sink...
        self._videoSink = None
        self._videoSink = Gst.ElementFactory.make("autovideosink", "sink")
        assert(self._videoSink)

        # Initialize the sink's properties and bind it to the pipeline...
        self._videoSink.force_aspect_ratio  = True
        self._videoSink.double_buffer       = True
        self._videoSink.draw_borders        = True
        self._videoSink.handle_expose       = True

        # Load the intro video and set to playback state...
        self._playBin.set_property('uri', 
            "file://" + 
            os.path.join(
                LauncherArguments.getArguments().dataRoot, 
                "Opening.mkv"))

        # Set the volume...
        self._playBin.set_property("volume", 0.6)

        # Force our realize callback to be invoked so we can capture the 
        #  drawing area's window handle which we need before playing back the
        #  video...
        self._videoDrawingArea.realize()

        # Toggle the pause state and block until transition complete...
        self._playBin.set_state(Gst.State.PAUSED)
        self._playBin.get_state(5000000000)
        
        # Get the video pad...
        videoPad = None
        videoPad = self._playBin.emit("get-video-pad", 0)
        
        # Video never loaded...
        if videoPad is None:
            return

        # Request video metadata...
        videoPadCapabilities = None
        videoPadCapabilities=videoPad.get_current_caps()
        
        # Got video metadata...
        if videoPadCapabilities:
            (success, videoWidth) = videoPadCapabilities.get_structure(0).get_int("width")
            (success, videoHeight) = videoPadCapabilities.get_structure(0).get_int("height")
            assert(success)
        
        # Didn't get video metadata, fallback to guessing dimensions...
        else:
            videoWidth  = 1920
            videoHeight = 1200

        # Resize drawing area so video is 3/4 screen width...
        (monitorWidth, monitorHeight) = getMonitorWithCursorSize()
        aspectRatio = videoWidth / videoHeight
        drawingAreaWidth = (3 / 4) * monitorWidth
        self._videoDrawingArea.set_size_request(
            drawingAreaWidth, drawingAreaWidth / aspectRatio)

        # Start playing now...
        self._playBin.set_state(Gst.State.PLAYING)

    # GStreamer is trying to tell us something asynchronously...
    def onBusMessage(self, bus, message):

        # Stream has finished. Destroy the window...
        if message.type == Gst.MessageType.EOS:
            self.videoDone()

        # Some kind of error occured...
        elif message.type == Gst.MessageType.ERROR:
        
            # Retrieve the error and dump on the console...
            (errorMessage, debugMessage) = message.parse_error()
            print(_("Error:"), errorMessage, debugMessage)

            # Just load the rest of the application anyways...
            self.videoDone()

    # GStreamer is trying to tell us something synchronously...
    def onBusSyncMessage(self, bus, message):

        # Get the message's structure...
        messageStructure = None
        messageStructure = message.get_structure();

        # No structure, skip...
        if messageStructure is None:
            return False

        # Get the name of the message...
        messageName = messageStructure.get_name()

        # We can only bind the render window to the drawing area from within
        #  the main thread, hence why we handle this in sync-message and not
        #  the message signal. prepare-xwindow-id became prepare-window-handle
        #  with GStreamer 0.10...
        if messageName == "prepare-xwindow-id" or \
           messageName == "prepare-window-handle":

            # Get the video sink...
            videoSink = message.src
            assert(videoSink)

            # Make sure we got the window handle during the widget 
            #  realization...
            assert(self._drawingAreaWindowHandle)

            # Set the drawing area widget as the image sink...
            Gdk.threads_enter()
            Gdk.Display.get_default().sync()
            videoSink.set_window_handle(self._drawingAreaWindowHandle)
            Gdk.threads_leave()

    # Drawing area is created and visible. Capture the window handle...
    def onDrawingAreaRealized(self, sender):

        # If we're running a legacy w32 system, use its platform's native
        #  semantics for retrieving the window handle...
        if sys.platform == "win32":
            self._drawingAreaWindowHandle = self._videoDrawingArea.get_window().get_handle()

        # Otherwise, if using the Xorg server, retrieve its window handle
        #  via its native semantics...
        else:
            self._drawingAreaWindowHandle = self._videoDrawingArea.get_window().get_xid()

        # Make sure the window handle was retrieved...
        assert(self._drawingAreaWindowHandle)

    # Keyboard or mouse pressed on the video......
    def onVideoPressed(self, *arguments):

        # Cleanup the window...
        self.videoDone()

    # Show the video window...
    def showVideo(self):

        # If GStreamer isn't available, skip the video...
        if not haveGStreamer:
            return

        # Set it to be always on top...
        self._videoWindow.set_keep_above(True)

        # Display the window...
        self._videoWindow.show_all()

    # Destroy the video window after video is done...
    def videoDone(self):

        # Unload the video, if using one...
        if haveGStreamer:
            self._playBin.set_state(Gst.State.NULL)

        # Destroy the video window...
        self._videoWindow.destroy()

        # Display the splash window now, if enabled...
        if not LauncherArguments.getArguments().noSplash:
            self.splashWindowProxy = SplashWindowProxy(self._launcherApp)
            self.splashWindowProxy.showSplash()
        
        # Otherwise just show the main window now...
        else:
            self._assistant.show_all()


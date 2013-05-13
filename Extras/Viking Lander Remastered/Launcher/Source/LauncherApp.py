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
import sys
import os
from gi.repository import Gtk, Gdk, GObject, Vte

# GStreamer...
haveGStreamer = True
try:
    import gi
    gi.require_version("GstVideo", "1.0")
    from gi.repository import GstVideo, Gst
    haveGStreamer = True
except ValueError:
    print("GStreamer >= 1.0 not found. Will not use for music...")
    haveGStreamer = False

# Arguments...
import LauncherArguments

# Splash and opening video windows...
from OpeningVideoWindow import OpeningVideoWindowProxy

# Assistant pages and related logic...
from IntroductionPage import IntroductionPageProxy
from VerificationPages import VerificationPagesProxy
from HandbookPage import HandbookPageProxy
from SelectRecoveryPage import SelectRecoveryPageProxy
from ConfigurePages import ConfigurePagesProxy
from ConfirmPage import ConfirmPageProxy
from RecoveryPage import RecoveryPageProxy
from FarewellPage import FarewellPageProxy

# Miscellaneous routines...
from Miscellaneous import *

# Launcher class...
class LauncherApp(object):

    # Constructor...
    def __init__(self):

        # Set the application name...
        GObject.set_application_name(getApplicationName())

        # Workaround for <https://bugzilla.gnome.org/show_bug.cgi?id=688767>
        GObject.type_register(Vte.Terminal)

        # Initialize Glade builder...
        self.builder = Gtk.Builder()
        self.builder.add_from_file(os.path.join(
            LauncherArguments.getArguments().dataRoot, "Launcher.glade"))

        # Find the assistant...
        self.assistant = self.builder.get_object("assistantWindow")

        # Construct pages and register them with the assistant in correct order...
        self.introductionPageProxy = IntroductionPageProxy(self)
        self.verificationPagesProxy = VerificationPagesProxy(self)
        self.handbookPageProxy = HandbookPageProxy(self)
        self.selectRecoveryPageProxy = SelectRecoveryPageProxy(self)
        self.configurePagesProxy = ConfigurePagesProxy(self)
        self.confirmPageProxy = ConfirmPageProxy(self)
        self.recoveryPageProxy = RecoveryPageProxy(self)
        self.farewellPageProxy = FarewellPageProxy(self)

        # Set the forward function which determines next page to show...
        self.assistant.set_forward_page_func(self.forwardPage, None)

        # Add the about button to the assitant's action area...
        aboutButton = Gtk.Button(stock=Gtk.STOCK_ABOUT)
        self.assistant.add_action_widget(aboutButton)

        # Connect all other signals...
        aboutButton.connect("clicked", self.onAboutButtonPressed)
        self.assistant.connect("apply", self.onApplyEvent)
        self.assistant.connect("cancel", self.onCancelEvent)
        self.assistant.connect("close", self.onCloseEvent)
        self.assistant.connect("delete-event", self.onDeleteEvent)
        self.assistant.connect("prepare", self.onPrepareEvent)
        self.assistant.connect("show", self.onShow)
        
        # For wider screens, adjust the window size to 3/4 the width...
        (monitorWidth, monitorHeight) = getMonitorWithCursorSize()
        if monitorWidth > 1024:
            self.assistant.resize_to_geometry(monitorWidth * 3 / 4, 1)

        # If the user requested no background music, then disable GStreamer...
        if LauncherArguments.getArguments().noBackgroundMusic:
            global haveGStreamer
            haveGStreamer = False

        # Prepare background music if GStreamer available...
        if haveGStreamer:

            # Create volume button...
            self.volumeButton = Gtk.VolumeButton(size=Gtk.IconSize.BUTTON)
            self.volumeButton.set_orientation(Gtk.Orientation.HORIZONTAL)
            self.assistant.add_action_widget(self.volumeButton)

            # Connect its signal...
            self.volumeButton.connect("value-changed", self.onVolumeChanged)

            # Initialize GStreamer, if not already...
            Gst.init_check(None)

            # Create the bin containing needed elements...
            self._playBin = None
            self._playBin = Gst.ElementFactory.make("playbin", "player")
            assert(self._playBin)

            # Get the bus for the pipeline and prepare signal handlers and for
            #  asynchronous messages...
            bus = self._playBin.get_bus()
            bus.add_signal_watch()
            bus.connect("message", self.onBusMessage)

            # Create the dummy sink for video...
            fakeSink = None
            fakeSink = Gst.ElementFactory.make("fakesink", "fakesink")
            assert(fakeSink)
            self._playBin.set_property("video-sink", fakeSink)

        # Construct and show the opening video window, unless user disabled...
        if not LauncherArguments.getArguments().noSplash:
            self.openingVideoWindowProxy = OpeningVideoWindowProxy(self)
            self.openingVideoWindowProxy.showVideo()
        else:
            self.assistant.show_all()

    # End of current page. Calculate index of next page...
    def forwardPage(self, currentPageIndex, userData):

        # Get what would be the next page...
        nextPage = self.assistant.get_nth_page(currentPageIndex)

        # Transitioning into verification progress page...
        if nextPage is self.verificationPagesProxy.getProgressPageBox() and \
           self.builder.get_object("skipVerificationCheckRadio").get_active():

                # Skip the disc verification check...
                self.assistant.set_current_page(currentPageIndex + 1)
                return currentPageIndex + 2

        # Any other page just transition to the next one...
        else:
            return currentPageIndex + 1

    # About button pressed. Invoke dialog box...
    def onAboutButtonPressed(self, button):

        # Find the about dialog...
        aboutDialog = self.builder.get_object("aboutDialog")
        assert(aboutDialog)

        # Set the version...
        aboutDialog.set_version(getShortVersionString())
       
        # Set the authors...
        aboutDialog.set_authors([
            "<a href=\"mailto:kip@thevertigo.com\">Kip Warner</a>"])
        
        # Set the documenters...
        aboutDialog.set_documenters([
            "<a href=\"mailto:jtantogo-1@yahoo.com\">Johann Tang</a>",
            "<a href=\"mailto:kip@thevertigo.com\">Kip Warner</a>"])
        
        # Set artists...
        aboutDialog.set_artists([
            "<a href=\"mailto:jacob_vejvoda@hotmail.com\">Jacob Vejvoda</a>",
            "<a href=\"http://www.openclipart.org\">Open Clipart Library</a>",
            "<a href=\"http://www.paul-laberge.com\">Paul Laberge</a>\n\n"])

        # Add custom sections if introspection for add_credit_section() isn't
        #  busted to shit on this machine...
        try:

            # Beta testers...
            aboutDialog.add_credit_section("Beta Testers", [
                "<a href=\"mailto:adam@avaneya.com\">Adam Gornowicz</a>",
                "<a href=\"mailto:me@jesseknudsen.com\">Jesse Knudsen</a>",
                "<a href=\"mailto:kip@thevertigo.com\">Kip Warner</a>",
                "<a href=\"mailto:matt@thevertigo.com\">Matthew MacLennan</a>"])

            # Musicians...
            aboutDialog.add_credit_section("Musicians", [
                "<a href=\"http://www.josephliau.com\">Joseph Liau</a>"])

            # Shouts and thanks...
            aboutDialog.add_credit_section("Shouts / Thanks", [
                "<a href=\"mailto:adam@avaneya.com\">Adam Gornowicz</a>",
                "<a href=\"mailto:charles.amadeus@gmail.com\">Chuck Siegal</a>",
                "<a href=\"http://www.freedesktop.org\">freedesktop.org</a>",
                "<a href=\"http://freedomincluded.com/\">Freedom Included</a>",
                "<a href=\"mailto:matt@thevertigo.com\">Matthew MacLennan</a>",
                "<a href=\"http://pds.nasa.gov/\">NASA Planetary Data System</a>",
                "Project Bossanova",
                "<a href=\"mailto:randall@executiv.es\">Randall Ross</a>",
                "<a href=\"mailto:rms@gnu.org\">Richard Stallman</a>",
                "<a href=\"http://www.system76.com\">System76</a>",
                "<a href=\"http://www.meetup.com/ubuntuvancouver/\">Ubuntu Vancouver</a>",
                "<a href=\"mailto:vr@oracology.net\">Varun Ramraj</a>"])

            # Studio software...
            aboutDialog.add_credit_section("Studio Software", [
                "Bazaar",
                "Blender",
                "D-Feet",
                "GEdit",
                "GHex",
                "GNU Image Manipulation Toolkit",
                "Glade",
                "GNU Autotools",
                "GNU C++ compilers",
                "GNU OCRAD",
                "GNU Operating System",
                "GStreamer",
                "Gtk+",
                "Nemiver",
                "Python",
                "Theora",
                "Ubuntu",
                "Vorbis",
                "Xfce"])

            # Dedications...
            aboutDialog.add_credit_section("Dedications", [
                "<a href=\"https://en.wikipedia.org/wiki/Ahmad_Shah_Masoud\">Ahmad Shah Massoud</a> (1953–2001)",
                "<a href=\"https://en.wikipedia.org/wiki/David_Kelly_%28weapons_expert%29\">David Kelly</a> (1944–2003)",
                "<a href=\"http://splittingthesky.blogspot.ca/\">Splitting the Sky</a> (1952–2013)"])

        # Otherwise fallback if it is...
        except TypeError:
            
            # Append a note where people can find the complete credits...
            artistsList = aboutDialog.get_artists()
            artistsList.append(
                "All of the aforementioned contributed directly to this software. For a\n"
                "complete list of everyone who worked on the Avaneya project, including\n"
                "on this software, please see our master <a href=\"https://bazaar.launchpad.net/~avaneya/avaneya/trunk/view/head:/Credits\">Credits</a> file.")
            aboutDialog.set_artists(artistsList)

        # Show dialog as modal and hide after close...
        aboutDialog.run()
        aboutDialog.hide()

    # GStreamer is trying to tell us something asynchronously...
    def onBusMessage(self, bus, message):

        # Stream has finished...
        if message.type == Gst.MessageType.EOS:
            pass # TODO: Loop? 

        # Some kind of error occured...
        elif message.type == Gst.MessageType.ERROR:
        
            # Retrieve the error and dump on the console...
            (errorMessage, debugMessage) = message.parse_error()
            print("Error:", errorMessage, debugMessage)

    # Apply button clicked...
    def onApplyEvent(self, *args):
        
        # Start the recovery process...
        self.recoveryPageProxy.executeVikingExtractor()

    # Cancel signal emitted when cancel button clicked or assistant being 
    #  closed and cancel button is interactive...
    def onCancelEvent(self, assistant, *args):
        
        # For debugging purposes...
        #print("onCancelEvent...")

        # Prompt the user for if they'd really like to quit...
        messageDialog = Gtk.MessageDialog(
            self.assistant, Gtk.DialogFlags.MODAL, Gtk.MessageType.QUESTION, 
            Gtk.ButtonsType.YES_NO, 
            "Are you sure you'd like to quit?")
        messageDialog.set_default_response(Gtk.ResponseType.NO)
        userResponse = messageDialog.run()
        messageDialog.destroy()
        
        # User requested not to quit, cancel...
        if userResponse != Gtk.ResponseType.YES:
            return

        # Check if the verification thread is still running, and if so, signal
        #  it to quit and block until it does...
        self.verificationPagesProxy.waitThreadQuit()

        # Terminate...
        self.quit()

    # Either close button of summary page clicked or apply button in last page
    #  in flow of type CONFIRM clicked...
    def onCloseEvent(self, assistant, *args):
        
        # For debugging purposes...
        #print("onCloseEvent")
        
        # No threads should be running by this point because at the end of the 
        #  assistant's page flow, so safe to terminate...
        self.quit()

    # User requested that assistant be closed. The default signal handler would 
    #  just destroys the window. Cancel signal is sent automatically after this
    #  signal is handled...
    def onDeleteEvent(self, assistant, event, *junk):

        # For debugging purposes...
        #print("onDeleteEvent...")
        
        # If the verification thread is currently running, then trigger its
        #  abort logic...
        if self.verificationPagesProxy.isVerifying():
            stopVerificationButton = self.builder.get_object("stopVerificationButton")
            stopVerificationButton.emit("clicked")
        
        # If the handbook page is the current page, trigger a stop download in
        #  case it is currently downloading...
        currentPage = assistant.get_nth_page(assistant.get_current_page())
        if currentPage is self.handbookPageProxy.getHandbookPageBox():
            stopHandbookDownloadButton = self.builder.get_object("stopHandbookDownloadButton")
            stopHandbookDownloadButton.emit("clicked")

        # If the recovery thread is currently running, then trigger its abort
        #  logic...
        if self.recoveryPageProxy.processID != 0:
            abortRecoveryButton = self.builder.get_object("abortRecoveryButton")
            abortRecoveryButton.emit("clicked")

        # Let cancel handler determine whether to exit or not...

    # End of current page. Next page is being constructed but not visible yet.
    #  Give it a chance to prepare...
    def onPrepareEvent(self, assistant, currentPageIndex):

        # Reset the cursor to normal in case something changed it...
        self.setBusy(False)

        # Transitioning to verification progress page...
        if currentPageIndex is self.verificationPagesProxy.getProgressPageBox():
            self.verificationPagesProxy.onPrepare()

        # Transitioning to handbook page...
        if currentPageIndex is self.handbookPageProxy.getHandbookPageBox():
            self.handbookPageProxy.onPrepare()

        # Transitioning to final configuration page...
        elif currentPageIndex is self.configurePagesProxy.getConfigureAdvancedPageBox():
            self.configurePagesProxy.onPrepare()

        # Transitioning to confirm page...
        elif currentPageIndex is self.confirmPageProxy.getPageBox():
            self.confirmPageProxy.onPrepare()

    # Assistant visible...
    def onShow(self, assistant, *dummy):

        # GStreamer available...
        if haveGStreamer:

            # Start playing...
            musicPath = "file://" + os.path.join(
                LauncherArguments.getArguments().dataRoot, "Music.ogg")
            self._playBin.set_property("uri", musicPath)
            self._playBin.set_state(Gst.State.PLAYING)

            # Set the music volume to full...
            self.volumeButton.set_value(1.0)

        # GStreamer not available...
        else:
            pass

    # Set the music playback state...
    def onVolumeChanged(self, volumeButton, value):

        # Volume button should only be visible of GStreamer was available...
        assert(haveGStreamer)

        # Make sure volume is normalized to [0.0, 1.0] range...
        volume = max(min(value, 1.0), 0.0)
        
        # Set the volume...
        self._playBin.set_property("volume", volume)
        
    # Run the GUI...
    def run(self):

        # Start processing events...
        try:
            Gtk.main()
        
        # On unhandled exceptions, just quit...
        except:
            self.quit()

    # Toggle a busy state...
    def setBusy(self, Busy = True):

        # Toggle busy state...
        if Busy:

            # Change to busy cursor...
            cursorWatch = Gdk.Cursor.new(Gdk.CursorType.WATCH)
            self.assistant.get_root_window().set_cursor(cursorWatch)

        # Untoggle busy state...
        else:

            # Reset the cursor to normal in case something changed it...
            cursorArrow = Gdk.Cursor.new(Gdk.CursorType.ARROW)
            self.assistant.get_root_window().set_cursor(cursorArrow)

    # Terminate...
    def quit(self):
        Gtk.main_quit()


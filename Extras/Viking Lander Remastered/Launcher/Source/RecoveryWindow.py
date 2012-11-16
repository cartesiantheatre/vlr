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

# Imports...
from gi.repository import Gtk, Gdk, GObject, GLib, Vte
import os
import dbus
from dbus.exceptions import DBusException
from dbus.mainloop.glib import DBusGMainLoop
from time import sleep
import sys

# Our support modules...
import LauncherArguments

# Recovery window proxy class...
class RecoveryWindowProxy():

    # Constructor...
    def __init__(self, launcherApp):
        
        # Initialize...
        self._launcherApp       = launcherApp
        self._assistant         = launcherApp.assistant
        self._builder           = launcherApp.builder
        self._confirmPageProxy  = launcherApp.confirmPageProxy

        # Find the window and its widgets...
        self._recoveryWindow            = self._builder.get_object("recoveryWindow")
        self._recoveryExpander          = self._builder.get_object("recoveryExpander")
        self._recoveryScrolledWindow    = self._builder.get_object("recoveryScrolledWindow")
        self._recoveryProgressBar       = self._builder.get_object("recoveryProgressBar")
        self._cancelRecoveryButton      = self._builder.get_object("cancelRecoveryButton")
        
        # Create the terminal widget...
        self._terminal = Vte.Terminal()
        self._terminal.set_cursor_blink_mode(Vte.TerminalCursorBlinkMode.ON)
        #self._terminal.set_background_transparent(True)
        #self._terminal.set_opacity(0.2)
        self._terminal.set_cursor_shape(Vte.TerminalCursorShape.BLOCK)
        self._terminal.set_emulation("xterm")
        self._terminal.set_scroll_on_output(True)
        self._terminal.set_scroll_on_keystroke(True)
        self._terminal.set_color_foreground(Gdk.Color(red=1.0, green=0.0, blue=0.0))
        self._terminal.set_scrollback_lines(5000)
        self._terminal.set_encoding("UTF-8")

        # Add the terminal widget to the expander's scrolled window widget...
        self._recoveryScrolledWindow.add(self._terminal)

        # Set the window width to 1000 pixels, unless user's resolution too low
        #  in which case just use the maximum available...
        screen = self._recoveryWindow.get_screen()
        screenWidth = screen.get_width()
        if screenWidth >= 1000:
            self._recoveryWindow.resize(1000, 200)
        else:
            self._recoveryWindow.resize(screenWidth, 200)
        
        # Connect the Gtk+ signals...
        self._recoveryWindow.connect("delete-event", Gtk.main_quit)
        self._cancelRecoveryButton.connect("clicked", self.onCancelClicked)
        self._recoveryExpander.connect("notify::expanded", self.onExpanded)
        self._terminal.connect("child-exited", self.onChildProcessExit)

        # Get the path to the VikingExtractor binary...
        self._vikingExtractorBinaryPath = \
            LauncherArguments.getArguments().vikingExtractorBinaryPath
            
        # None found, bail...
        if not self._vikingExtractorBinaryPath:
            self._fatalLaunchError()

        # Try to start the VikingExtractor process...
        try:
            launchStatus = self._terminal.fork_command_full(
                Vte.PtyFlags.DEFAULT,
                None,
                [self._vikingExtractorBinaryPath] +
                    self._confirmPageProxy.getVikingExtractorArguments(),
                [],
                GLib.SpawnFlags.DO_NOT_REAP_CHILD, # This method automatically adds this flag anyways. Here for clarity...
                None,
                None)

        # Failed to launch. VTE's documentation is not clear on the way to check
        #  for this...
        except GLib.GError:
            self._fatalLaunchError()
        if launchStatus == False:
            self._fatalLaunchError()

        # Hide the assistant...
        self._assistant.iconify()

        # Set it to be always on top...
        self._recoveryWindow.set_keep_above(True)

        # Display the window...
        self._recoveryWindow.show_all()

        # Register our D-Bus signal handler callbacks...
        print("Waiting for VikingExtractor D-Bus service...")
        while not self.initializeDBus():

            # Pump the Gtk+ event handler while we wait...
            while Gtk.events_pending():
                Gtk.main_iteration()

            # Try again in a tenth of a second...
            sleep(0.1)

        # Debugging hint...
        print("Found VikingExtractor D-Bus service...")

    # VikingExtractor could not be executed...
    def _fatalLaunchError(self):

            # Alert user...
            messageDialog = Gtk.MessageDialog(
                self._assistant, Gtk.DialogFlags.MODAL, Gtk.MessageType.ERROR, 
                Gtk.ButtonsType.OK, 
                "Error")
            messageDialog.format_secondary_text(
                "The VikingExtractor could not be launched. " \
                "The following location was attempted:\n\n{0}".
                    format(self._vikingExtractorBinaryPath))
            messageDialog.run()
            messageDialog.destroy()
            
            # Terminate...
            sys.exit(1)

    # Initialize D-Bus. True if VikingExtractor is found, False otherwise...
    def initializeDBus(self):

        # Some constants to help find the VikingExtractor...
        DBUS_SERVICE_NAME           = "com.cartesiantheatre.VikingExtractorService"
        DBUS_OBJECT_PATH            = "/com/cartesiantheatre/VikingExtractorObject"
        DBUS_INTERFACE              = "com.cartesiantheatre.VikingExtractorInterface"
        DBUS_SIGNAL_NOTIFICATION    = "Notification"
        DBUS_SIGNAL_PROGRESS        = "Progress"

        # Connect to the session bus...
        dbus_loop = DBusGMainLoop()
        sessionBus = dbus.SessionBus(mainloop=dbus_loop)

        # Try to connect to the VikingExtractor...
        try:
            
            # Initialize the remote proxy...
            vikingExtractorProxy = sessionBus.get_object(
                DBUS_SERVICE_NAME, DBUS_OBJECT_PATH)

            # Connect D-Bus notification signal to callback...
            vikingExtractorProxy.connect_to_signal(
                DBUS_SIGNAL_NOTIFICATION, 
                self.onVikingExtractorNotificationDBusSignal, 
                dbus_interface=DBUS_INTERFACE)

            # Connect D-Bus progress signal to callback...
            vikingExtractorProxy.connect_to_signal(
                DBUS_SIGNAL_PROGRESS, 
                self.onVikingExtractorProgressDBusSignal, 
                dbus_interface=DBUS_INTERFACE)

        # VikingExtractor probably isn't running...
        except DBusException as error:
            return False

        # Ready to roll...
        return True

    # Cancel recovery button clicked...
    def onCancelClicked(self, button, *junk):

        # Stubbed...
        pass

    # VikingExtractor terminated...
    def onChildProcessExit(self, *junk):

        # Get the exit code...
        exitCode = self._terminal.get_child_exit_status()
        print("VikingExtractor terminated with status {0}...".format(exitCode))

        # Any other code than zero denotes an error...
        if exitCode is not 0:

            # Alert user...
            messageDialog = Gtk.MessageDialog(
                self._assistant, Gtk.DialogFlags.MODAL, Gtk.MessageType.ERROR, 
                Gtk.ButtonsType.OK, 
                "The recovery process was unsuccessful.")
            messageDialog.format_secondary_text(
                "The VikingExtractor ran, but reported an error. ({0})"
                    .format(exitCode))
            messageDialog.run()
            messageDialog.destroy()
            
            # Terminate...
            sys.exit(1)
        
        # Otherwise VikingExtractor completed successfully...
        else:

            # Destroy the recovery window...
            self._recoveryWindow.destroy()
            
            # Make the main window visible now...
            self._assistant.deiconify()

    # Expander toggled...
    def onExpanded(self, expander, *junk):

        # Expander is collapsing, but hasn't physically yet...
        if expander.get_expanded() is False:

            # Get the current width, which we will maintain, and remember the full
            #  expanded height as well so we can restore later if expanding again...
            (currentWidth, self._expandedHeight) = self._recoveryWindow.get_size()
            
            # Keep the whole window's width, but shrink height to minimal...
            self._recoveryWindow.resize(currentWidth, 1)

        # Expander is expanding, but hasn't physically yet...
        else:

            # Keep whole window's width, but expand whole window to previous height...
            (currentWidth, collapsedHeight) = self._recoveryWindow.get_size()
            self._recoveryWindow.resize(currentWidth, self._expandedHeight)

    # VikingExtractor is trying to tell us something in a human readable string...
    def onVikingExtractorNotificationDBusSignal(self, notification):
        self._recoveryProgressBar.set_text(notification)

    # VikingExtractor is telling us it has completed some work...
    def onVikingExtractorProgressDBusSignal(self, fraction):
        self._recoveryProgressBar.set_fraction(fraction / 100.0)


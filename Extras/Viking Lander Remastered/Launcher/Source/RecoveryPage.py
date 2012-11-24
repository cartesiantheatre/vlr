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

# Recovery page proxy class...
class RecoveryPageProxy():

    # Constructor...
    def __init__(self, launcherApp):
        
        # Initialize...
        self._launcherApp       = launcherApp
        self._assistant         = launcherApp.assistant
        self._builder           = launcherApp.builder
        self._confirmPageProxy  = launcherApp.confirmPageProxy

        # Add recovery page to assistant...
        self._recoveryPageBox = self._builder.get_object("recoveryPageBox")
        self._recoveryPageBox.set_border_width(5)
        self._assistant.append_page(self._recoveryPageBox)
        self._assistant.set_page_title(self._recoveryPageBox, "Recovery")
        self._assistant.set_page_type(self._recoveryPageBox, Gtk.AssistantPageType.PROGRESS)
        self._assistant.set_page_complete(self._recoveryPageBox, False)

        # Shortcuts to the page's widgets...
        self._terminal              = self._builder.get_object("recoveryTerminal")
        self._recoveryProgressBar   = self._builder.get_object("recoveryProgressBar")
        self._abortRecoveryButton   = self._builder.get_object("abortRecoveryButton")

        # Connect the signals...
        self._abortRecoveryButton.connect("clicked", self.onAbortClicked)
        self._terminal.connect("child-exited", self.onChildProcessExit)

    # Start the recovery process...
    def startRecovery(self, noProcessIDHack=False):

        # Get the path to the VikingExtractor binary...
        self._vikingExtractorBinaryPath = \
            LauncherArguments.getArguments().vikingExtractorBinaryPath
            
        # None found, bail...
        if not self._vikingExtractorBinaryPath:
            self._fatalLaunchError()

        # Try to start the VikingExtractor process. First way is with child_pid
        #  but some distros ship buggy version that doesn't provide it. Second
        #  way is without it...
        self._processID = 0
        try:
            if noProcessIDHack is False:
                launchStatus = self._terminal.fork_command_full(
                    Vte.PtyFlags.DEFAULT,
                    None,
                    [self._vikingExtractorBinaryPath] + self._confirmPageProxy.getVikingExtractorArguments(),
                    [],
                    GLib.SpawnFlags.DO_NOT_REAP_CHILD, # This method automatically adds this flag anyways. Here for clarity...
                    child_pid=self._processID)
            else:
                launchStatus = self._terminal.fork_command_full(
                    Vte.PtyFlags.DEFAULT,
                    None,
                    [self._vikingExtractorBinaryPath] + self._confirmPageProxy.getVikingExtractorArguments(),
                    [],
                    GLib.SpawnFlags.DO_NOT_REAP_CHILD,
                    None,
                    None)

        # Ugly ass hack...
        #  https://bugzilla.gnome.org/show_bug.cgi?id=649004
        except TypeError:
            self.startRecovery(noProcessIDHack=True)
            return

        # Failed to launch. VTE's documentation is not clear on the way to check
        #  for this...
        except GLib.GError:
            self._fatalLaunchError()
        if launchStatus == False:
            self._fatalLaunchError()

        print("Process ID: {0}...".format(self._processID))

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

    # Abort recovery button clicked...
    def onAbortClicked(self, button, *junk):

        # If we have the process ID, kill it...
        if self._processID:
            os.kill(self._processID, 9)

        # Otherwise, use this ugly non-portable hack...
        else:
            os.system("killall -9 viking-extractor")

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
                "Error")
            messageDialog.format_secondary_text(
                "The recovery process was unsuccessful. ({0})"
                    .format(exitCode))
            messageDialog.run()
            messageDialog.destroy()
            
            # Terminate...
            sys.exit(1)
        
        # Otherwise VikingExtractor completed successfully...
        else:

            # Notify assistant...
            self._assistant.set_page_complete(self._recoveryPageBox, True)

    # VikingExtractor is trying to tell us something in a human readable string...
    def onVikingExtractorNotificationDBusSignal(self, notification):
        self._recoveryProgressBar.set_text(notification)

    # VikingExtractor is telling us it has completed some work...
    def onVikingExtractorProgressDBusSignal(self, fraction):
        self._recoveryProgressBar.set_fraction(fraction / 100.0)


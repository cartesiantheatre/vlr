#!/usr/bin/env python3
# VikingExtractor, to recover images from Viking Lander operations.
# Copyright (C) 2010-2013 Cartesian Theatre <kip@thevertigo.com>.
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
from gi.repository import Gtk
from gi.repository import Gdk
from gi.repository import GObject
from gi.repository import GLib
from gi.repository import Vte
from gi.repository import Gio
import os
from time import sleep
import sys

# Our support modules...
import LauncherArguments

# Recovery page proxy class...
class RecoveryPageProxy():

    # Constructor...
    def __init__(self, launcherApp):

        # Initialize...
        self._launcherApp           = launcherApp
        self._assistant             = launcherApp.assistant
        self._builder               = launcherApp.builder
        self._confirmPageProxy      = launcherApp.confirmPageProxy
        self._recoveryProgressText  = ""

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
    def executeVikingExtractor(self, noProcessIDHack=False):

        # Get the path to the VikingExtractor binary...
        self._vikingExtractorBinaryPath = \
            LauncherArguments.getArguments().vikingExtractorBinaryPath
            
        # None found, bail...
        if not self._vikingExtractorBinaryPath:
            self._fatalLaunchError()

        # Try to start the VikingExtractor process. First way is with child_pid
        #  provided, but some distros ship buggy version that don't provide it. 
        #  Second way is without it, works, but doesn't provide child_pid...
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
            self.executeVikingExtractor(noProcessIDHack=True)
            return

        # Failed to launch. VTE's documentation is not clear on the way to check
        #  for this...
        except GLib.GError:
            self._fatalLaunchError()
        if launchStatus == False:
            self._fatalLaunchError()

        # We got the process ID, show it...
        if self._processID:
            print("Spawned process {0}...".format(self._processID))

        # Some constants to help find the VikingExtractor...
        VE_DBUS_SERVICE_NAME        = "com.cartesiantheatre.VikingExtractorService"
        VE_DBUS_OBJECT_PATH         = "/com/cartesiantheatre/VikingExtractorObject"
        VE_DBUS_INTERFACE           = "com.cartesiantheatre.VikingExtractorInterface"
        VE_DBUS_SIGNAL_NOTIFICATION = "Notification"
        VE_DBUS_SIGNAL_PROGRESS     = "Progress"
        VE_DBUS_METHOD_START        = "Start"

        # Alert user...
        sys.stdout.write("Waiting for VikingExtractor D-Bus service...")

        sessionConnection = None
        sessionConnection = Gio.bus_get_sync(
            Gio.BusType.SESSION,
            None)

        if sessionConnection is None:
            print("Dead session connection handle...")
            sys.exit(1)

        # Keep trying to connect until successful...
        while True:

            # Alert user with each dot being a new attempt...
            sys.stdout.write(".")
            sys.stdout.flush()

            # Try to connect to the VikingExtractor...
            try:

                # Bind to the remote object...
                vikingExtractorProxy = None
                vikingExtractorProxy = Gio.DBusProxy.new_sync(
                    sessionConnection,
                    Gio.DBusProxyFlags.NONE, 
                    None,
                    VE_DBUS_SERVICE_NAME,
                    VE_DBUS_OBJECT_PATH,
                    VE_DBUS_INTERFACE, 
                    None)

                # Not available...
                if vikingExtractorProxy is None:
                    raise Exception()

                # We're connected. End alert with new line...
                break

            # VikingExtractor probably isn't running...
            except:

                # Pump the Gtk+ event handler before we try again...
                while Gtk.events_pending():
                    Gtk.main_iteration()

                # Pause one second, then try again...
                sleep(0.1)
                continue

        # Tell the VikingExtractor to commence the recovery...
        # Keep trying to connect until successful...
        while True:

            try:

                # Connect to VikingExtractor's notification signal...
                sessionConnection.signal_subscribe(
                    VE_DBUS_SERVICE_NAME,
                    VE_DBUS_INTERFACE,
                    VE_DBUS_SIGNAL_NOTIFICATION,
                    VE_DBUS_OBJECT_PATH,
                    None,
                    Gio.DBusConnectionFlags.NONE,
                    self.onVikingExtractorNotificationSignal,
                    None)

                # Connect to VikingExtractor's progress signal...
                sessionConnection.signal_subscribe(
                    VE_DBUS_SERVICE_NAME,
                    VE_DBUS_INTERFACE,
                    VE_DBUS_SIGNAL_PROGRESS,
                    VE_DBUS_OBJECT_PATH,
                    None,
                    Gio.DBusConnectionFlags.NONE,
                    self.onVikingExtractorProgressSignal,
                    None)

                # Show some progress on the console...
                sys.stdout.write(".")
                sys.stdout.flush()
                
                # Begin the recovery...
                vikingExtractorProxy.Start()

            except:

                # Pause one second, then try again...
                sleep(0.1)
                continue

            print("ok")
            break

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

    # Abort recovery button clicked...
    def onAbortClicked(self, button, *junk):

        # Prompt the user for if they'd really like to quit...
        messageDialog = Gtk.MessageDialog(
            self._assistant, Gtk.DialogFlags.MODAL, Gtk.MessageType.QUESTION, 
            Gtk.ButtonsType.YES_NO, 
            "Are you sure you'd like to abort the recovery?")
        messageDialog.set_default_response(Gtk.ResponseType.NO)
        userResponse = messageDialog.run()
        messageDialog.destroy()
        
        # User requested not to abort, cancel...
        if userResponse != Gtk.ResponseType.YES:
            return

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

            # Notify assistant by marking page as complete...
            self._assistant.set_page_complete(self._recoveryPageBox, True)

            # Advance to the next page...
            currentPageIndex = self._assistant.get_current_page()
            self._assistant.set_current_page(currentPageIndex + 1)

    # VikingExtractor is trying to tell us something in a human readable string...
    def onVikingExtractorNotificationSignal(
        self, connection, senderName, objectPath, interfaceName, signalName, 
        parameters, userData):

        # Unpack the notification message from the signal's arguments...
        notificationMessage = parameters.unpack()[0]

        # Store the human readable extractor state string...
        self._recoveryProgressText = notificationMessage
        
        # Get the current progress...
        currentProgress = self._recoveryProgressBar.get_fraction()
        
        # Format and set the progress bar caption based on the aforementioned...
        self._recoveryProgressBar.set_text("{0} ({1:.0f}%)".
            format(notificationMessage, currentProgress / 100.0))

    # VikingExtractor is telling us it has completed some work...
    def onVikingExtractorProgressSignal(
        self, connection, senderName, objectPath, interfaceName, signalName, 
        parameters, userData):

        # Unpack the current progress from the signal's arguments...
        currentProgress = parameters.unpack()[0]
    
        # Format and set the progress bar caption...
        self._recoveryProgressBar.set_text("{0} ({1:.0f}%)".
            format(self._recoveryProgressText, currentProgress))
        
        # Move the progress bar's fraction...
        self._recoveryProgressBar.set_fraction(currentProgress / 100.0)


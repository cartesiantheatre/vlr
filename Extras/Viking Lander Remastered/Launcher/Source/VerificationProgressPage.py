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
from gi.repository import GLib
from gi.repository import Gtk
from gi.repository import Gdk
from gi.repository import GObject
from gi.repository import GdkPixbuf
import hashlib
import os
import threading
import time
from sys import exit

# Our support modules...
import LauncherArguments

# Assistant proxy page base class...
from PageProxyBase import *

# i18n...
import gettext
_ = gettext.gettext

# Class containing behaviour for the two disc verification pages...
class VerificationProgressPageProxy(PageProxyBase):

    # Constructor...
    def __init__(self, launcherApp):

        # Initialize...
        super(VerificationProgressPageProxy, self).__init__(launcherApp)
        self._thread = None

        # Find the window widgets...
        self._verificationImage = self._builder.get_object("verificationImage")

        # Add the verification progress page to the assistant...
        self.registerPage(
            "verificationProgressPageBox",
            _("Verification Progress"),
            Gtk.AssistantPageType.PROGRESS,
            False)

        # Connect the signals...
        stopVerificationButton = self._builder.get_object("stopVerificationButton")
        stopVerificationButton.connect("clicked", self.onStopVerificationPressed)

    # Our page in the assistent is being constructed, but not visible yet...
    def onPrepare(self):

        # If the verification toggle wasn't selected, skip the page...
        self._yesToggle = self._builder.get_object("performVerificationCheckRadio")
        if self._yesToggle.get_active() == False:
            self._assistant.next_page()
            return

        # Make sure the page is not marked complete until after the thread
        #  exits...
        self._assistant.set_page_complete(self.getPageInGroup(0), False)

        # Change to busy cursor...
        self._launcher.setBusy(True)

        # Load the verification animation...
        animation = GdkPixbuf.PixbufAnimation.new_from_file(
            os.path.join(
                LauncherArguments.getArguments().dataRoot, "Verification.gif"))
        self._verificationImage.set_from_animation(animation)
        self._verificationImage.show()

        # Launch the thread...
        self.startDiscVerification()

    # Check if the verification thread is already running...
    def isVerifying(self):

        # Running now...
        if self._thread and self._thread.isAlive():
            return True
        
        # Not running...
        else:
            return False

    # Start the disc verification...
    def startDiscVerification(self):

        # Already running...
        if self.isVerifying():
            print(_("Verification thread already running, not relaunching..."))
            return

        # Allocate and start the thread...
        self._thread = VerificationThread(self._builder)
        self._thread.start()

    # User requested to stop disc verification...
    def onStopVerificationPressed(self, button):

        # Signal the thread to quit...
        self._thread.setQuit()

        # Wait for it to quit...
        self._thread.join()

        # Mark as dead...
        self._thread = None

        # Alert user...
        messageDialog = Gtk.MessageDialog(
            self._assistant, Gtk.DialogFlags.MODAL, Gtk.MessageType.WARNING, 
            Gtk.ButtonsType.OK, _("Disc verification was cancelled."))
        messageDialog.run()
        messageDialog.destroy()

        # Mark page as complete...
        self._assistant.set_page_complete(self.getPageInGroup(0), True)

        # Advance to the next page...
        currentPageIndex = self._assistant.get_current_page()
        self._assistant.set_current_page(currentPageIndex + 1)

    # Check if the verification thread is still running, and if so, signal
    #  it to quit and block until it does...
    def waitThreadQuit(self):

        # Check if the thread exists, and if so, if it still running...
        if self.isVerifying():

            # Alert...
            print(_("Stopping disc verification thread..."))

            # Signal the thread to quit...
            self._thread.setQuit()
            
            # Wait for it to quit...
            self._thread.join()
            
            # Stop the verification animation...
            self._verificationImage.set_from_animation(None)

# This thread is responsible for verifying data verification of the disc...
class VerificationThread(threading.Thread):

    # Constructor...
    def __init__(self, builder):

        # Call the inherited threading class's constructor...
        super(VerificationThread, self).__init__()

        # Initialize state...
        self._builder = builder
        self._assistant = builder.get_object("assistantWindow")

        # Create list of file pairs (path, checksum) to check...
        self._parseChecksumManifest()
        
        # Counters for tracking progress...
        self._totalVerifiedSize = 0
        self._totalFileSize = 0
        
        # When this is true, the thread has been requested to terminate...
        self._terminateRequested = False

        # Find the user interface elements...
        self._verificationProgressPageBox = builder.get_object("verificationProgressPageBox")
        self._verificationProgressBar = builder.get_object("verificationProgressBar")

    # Calculate a file's MD5 checksum...
    def _calculateChecksum(self, filePath):

        # Try to open the file in readonly binary mode...
        try:
            fileDescriptor = open(filePath, 'rb')

        # Or check for failure...
        except OSError as Error:
        
            Gdk.threads_add_idle(GLib.PRIORITY_DEFAULT,
                self._setQuitWithError, 
                _("I was not able to check a file I require. {0}:\n\n{1}").
                    format(Error.strerror, currentFile))
            return 0

        # Construct a hash object for MD5 algorithm...
        md5Hash = hashlib.md5()

        # Calculate checksum...
        while True:

            # Read at most an 8K chunk...
            fileBuffer = fileDescriptor.read(8192)

            # I/O error or reached the end of the file...
            if not fileBuffer:
                break

            # Update the hash...
            md5Hash.update(fileBuffer)

            # Remember how much of the total size we've checked and calculate 
            #  the progress...
            self._totalVerifiedSize += len(fileBuffer)
            fraction = self._totalVerifiedSize / self._totalFileSize

            # Schedule to update the GUI...
            Gdk.threads_add_idle(
                GLib.PRIORITY_LOW,
                self._updateGUI, (filePath, fraction))

            # Something requested the thread quit...
            if self._terminateRequested:
                return 0

        # Return the calculated hash...
        return md5Hash.hexdigest()

    # Create list of file pairs (path, checksum) to check...
    def _parseChecksumManifest(self):

        # Reset the files list...
        self._files = []

        # Create path to file manifest...
        manifestPath = os.path.join(LauncherArguments.getArguments().missionDataRoot, "Checksums")

        # Try to open the checksum manifest file...
        try:
            manifestFile = open(manifestPath)

        # Or check for failure...
        except IOError as Error:

            # Alert the user...
            self._setQuitWithError(
                _("I could not locate the list of files to verify:\n\n{0}\n\n{1}").
                    format(manifestPath, Error.strerror))
            # Terminate...
            self.quit()

        # Parse each line...
        for line in manifestFile:
            
            # Check to make sure line is sane...
            assert(len(line) > 35)
            
            # First 32 characters, followed by two characters of white space, 
            #  then file path, then new line character...
            (fileChecksum, relativePath) = line.strip().split('  ', 1)
            
            # Prefix the file path with the mission data root...
            fullPath = os.path.join(
                LauncherArguments.getArguments().missionDataRoot, relativePath)
            
            # Add the checksum and path to the list...
            self._files.append((fileChecksum, fullPath))

        # Done...
        manifestFile.close()

    # Update the GUI. This callback is safe to update the GUI because it has
    #  been scheduled to execute safely...
    def _updateGUI(self, arguments):

        # Unpack arguments...
        currentFile, fraction = arguments

        # Update the progress bar...
        self._verificationProgressBar.set_fraction(fraction)
        self._verificationProgressBar.set_text("{0} ({1:.0f}%)".format(os.path.split(currentFile)[1], fraction * 100))

        # Remove function from list of event sources and don't call again until 
        #  needed...
        return False

    # Thread entry point...
    def run(self):
        
        print(_("Launching verification thread..."))
        
        # Calculate the total file size of all files...
        for (correctHexDigest, currentFile) in self._files:
            
            # Try to read it...
            try:
                fileSize = os.path.getsize(currentFile)
            
            # Something bad happened, bail...
            except OSError as Error:

                # Alert user from main thread...
                Gdk.threads_add_idle(
                    GLib.PRIORITY_DEFAULT,
                    self._setQuitWithError, 
                    _("I was not able to check a file I require. {0}:\n\n{1}").
                        format(Error.strerror, currentFile))
                
                # Exit the thread...
                exit(1)

            # Add the file's size to the total size...
            self._totalFileSize += fileSize

        # Check the checksums of all files...
        for (correctHexDigest, currentFile) in self._files:

            # Calculate checksum...
            currentHexDigest = self._calculateChecksum(currentFile)

            # Something requested the thread quit or there was a problem
            #  generating the checksum ...
            if self._terminateRequested or not currentHexDigest:
                return

            # Mismatch...
            if currentHexDigest != correctHexDigest:
                
                # Alert user from main thread...
                Gdk.threads_add_idle(
                    GLib.PRIORITY_DEFAULT,
                    self._setQuitWithError, 
                    _("Your disc might be damaged. It is recommended that you "
                    "replace it before continuing. The following file appeared "
                    "to be corrupt:\n\n<tt>{0}</tt>\n\n").format(currentFile))

                # Quit the thread...
                return

        # Alert user everything went fine from the main thread...
        Gdk.threads_add_idle(GLib.PRIORITY_DEFAULT, self._setQuitOk, None)

    # Quit the thread and show user everything went fine...
    def _setQuitOk(self, arguments):

        # Alert user...
        messageDialog = Gtk.MessageDialog(
            self._assistant, Gtk.DialogFlags.MODAL, Gtk.MessageType.INFO, 
            Gtk.ButtonsType.OK, 
            _("The disc verification completed successfully. Your disc is probably fine."))
        messageDialog.run()
        messageDialog.destroy()

        # Reset the cursor to normal in case something changed it...
        self._assistant.get_root_window().set_cursor(None)

        # Page is complete now...
        self._assistant.set_page_complete(self._verificationProgressPageBox, True)

        # Advance to the next page...
        currentPageIndex = self._assistant.get_current_page()
        self._assistant.set_current_page(currentPageIndex + 1)
        
        # Remove function from list of event sources and don't call again until 
        #  needed...
        return False

    # Quit the thread and show an error message...
    def _setQuitWithError(self, message):

        # Signal the thread should quit...
        self.setQuit()
        
        # Alert user...
        messageDialog = Gtk.MessageDialog(
            self._assistant, Gtk.DialogFlags.MODAL, Gtk.MessageType.ERROR, 
            Gtk.ButtonsType.OK, 
            _("Disc verification failed."))
        messageDialog.format_secondary_markup(message)
        messageDialog.set_default_response(Gtk.ResponseType.NO)
        messageDialog.run()
        messageDialog.destroy()

        # Advance to the next page...
        currentPageIndex = self._assistant.get_current_page()
        self._assistant.set_current_page(currentPageIndex + 1)

        # Remove function from list of event sources and don't call again until 
        #  needed...
        return False

    # Quit the thread...
    def setQuit(self):
        self._terminateRequested = True


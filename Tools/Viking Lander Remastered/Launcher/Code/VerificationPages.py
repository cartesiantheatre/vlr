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

# System imports...
from gi.repository import Gtk, Gdk, GObject
import hashlib
import os
import threading
import time

# Class containing behaviour for the two disc verification pages...
class VerificationPagesProxy():

    # Constructor...
    def __init__(self, launcherApp):

        # Initialize...
        self._launcher  = launcherApp
        self._assistant = launcherApp.assistant
        self._builder   = launcherApp.builder
        self._thread    = None

        # Add the verification info page to the assistant...
        self._verificationInfoPageBox = self._builder.get_object("verificationInfoPageBox")
        self._verificationInfoPageBox.set_border_width(5)
        self._assistant.append_page(self._verificationInfoPageBox)
        self._assistant.set_page_title(self._verificationInfoPageBox, "Verification Prompt")
        self._assistant.set_page_type(self._verificationInfoPageBox, Gtk.AssistantPageType.CONTENT)
        self._assistant.set_page_complete(self._verificationInfoPageBox, True)

        # Add the verification progress page to the assistant...
        self._verificationProgressPageBox = self._builder.get_object("verificationProgressPageBox")
        self._verificationProgressPageBox.set_border_width(5)
        self._assistant.append_page(self._verificationProgressPageBox)
        self._assistant.set_page_title(
            self._verificationProgressPageBox, "Verification Progress")
        self._assistant.set_page_type(
            self._verificationProgressPageBox, Gtk.AssistantPageType.PROGRESS)
        self._assistant.set_page_complete(self._verificationProgressPageBox, False)

        # Connect the signals...
        stopVerificationButton = self._builder.get_object("stopVerificationButton")
        stopVerificationButton.connect("pressed", self.onStopVerificationPressed)

    # Get the verification progress page box...
    def getProgressPageBox(self):
        return self._verificationProgressPageBox

    # Our page in the assistent is being constructed, but not visible yet...
    def onPrepare(self):

        # Don't start the disc verification thread if user requested to 
        #  skip it...
        if self._builder.get_object("skipVerificationCheckRadio").get_active():
            return

        # Otherwise begin the verification...
        else:

            # Change to busy cursor...
            self._launcher.setBusy(True)

            # Launch the thread...
            self.startDiscVerification()

    # Start the disc verification...
    def startDiscVerification(self):

        # Already running...
        if self._thread and self._thread.isAlive():
            print("Verification thread already running, not relaunching...")
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
            Gtk.ButtonsType.OK, "Disc verification was cancelled.")
        messageDialog.run()
        messageDialog.destroy()

        # Mark page as complete...
        self._assistant.set_page_complete(self._verificationProgressPageBox, True)
        
        # Advance to the next page...
        currentPageIndex = self._assistant.get_current_page()
        self._assistant.set_current_page(currentPageIndex + 1)

    # Check if the verification thread is still running, and if so, signal
    #  it to quit and block until it does...
    def waitThreadQuit(self):

        # Check if the thread exists, and if so, if it still running...
        if self._thread and self._thread.isAlive():

            # Alert...
            print("Stopping disc verification thread...")

            # Signal the thread to quit...
            self._thread.setQuit()
            
            # Wait for it to quit...
            self._thread.join()

# This thread is responsible for verifying data verification of the disc...
class VerificationThread(threading.Thread):

    # Constructor...
    def __init__(self, builder):

        # Call the inherited threading class's constructor...
        super(VerificationThread, self).__init__()

        # Initialize state...
        self._builder = builder
        self._assistant = self._builder.get_object("assistantWindow")

        # List of file pairs (path, checksum) to check...
        self._files = [
            ("/home/kip/Projects/Avaneya: Viking Lander Remastered/Mastered/Science Digital Data Preservation Task/Processed Images-9.7z", 
             "1569e25c8b159d32025f9ed4467adcc9")
        ]
        self._totalVerifiedSize = 0
        self._totalFileSize = 0
        
        # When this is true, the thread has been requested to terminate...
        self._terminateRequested = False

        # Find the progress bar...
        self._verificationProgressBar = self._builder.get_object("verificationProgressBar")

    # Calculate a file's MD5 checksum...
    def _calculateChecksum(self, filePath):

        # Try to open the file in readonly binary mode...
        try:
            fileHandle = open(filePath, 'rb')

        # Or check for failure...
        except IOError:
            GObject.idle_add(
                self._setQuitWithError, 
                "I was not able to check a file I require:\n\n{0}".format(filePath))
            return 0

        # Construct a hash object for MD5 algorithm...
        md5Hash = hashlib.md5()

        # Calculate checksum...
        while True:
            
            # Read at most an 8K chunk...
            fileBuffer = fileHandle.read(8192)
            
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
            GObject.idle_add(self._updateGUI, fraction, priority=GObject.PRIORITY_LOW)
            
            # Something requested the thread quit...
            if self._terminateRequested:
                return 0

        # Return the calculated hash...
        return md5Hash.hexdigest()

    # Update the GUI. This callback is safe to update the GUI because it has
    #  been scheduled to execute safely...
    def _updateGUI(self, fraction):

        # Update the progress bar...
        self._verificationProgressBar.set_fraction(fraction)

        # Remove function from list of event sources and don't call again until 
        #  needed...
        return False

    # Thread entry point...
    def run(self):
        
        # Calculate the total file size of all files...
        for (currentFile, correctHexDigest) in self._files:
            
            # Try to read it...
            try:
                fileSize = os.path.getsize(currentFile)
            
            # Something bad happened, bail...
            except OSError:

                # Alert user from main thread...
                GObject.idle_add(
                    self._setQuitWithError, 
                    "I was not able to check a file I require:\n\n{0}".format(filePath))
                
                # Exit the thread...
                return

            # Add the file's size to the total size...
            self._totalFileSize += fileSize

        # Check the checksums of all files...
        for (currentFile, correctHexDigest) in self._files:

            # Calculate checksum...
            currentHexDigest = self._calculateChecksum(currentFile)

            # Something requested the thread quit or there was a problem
            #  generating the checksum ...
            if self._terminateRequested or not currentHexDigest:
                return

            # Mismatch...
            if currentHexDigest != correctHexDigest:
                
                # Alert user from main thread...
                GObject.idle_add(
                    self._setQuitWithError, 
                    "The following file appears to be corrupt:\n\n{0}\n\n" \
                    "You should probably replace the disc.".format(currentFile))

                # Quit the thread...
                return

        # Reset the cursor to normal in case something changed it...
        self._assistant.get_root_window().set_cursor(None)

        # Mark page as complete...
        self._assistant.set_page_complete(
            self._builder.get_object("verificationProgressPageBox"), True)

        # Alert user everything went fine from the main thread...
        GObject.idle_add(self._setQuitOk)

    # Quit the thread and show user everything went fine...
    def _setQuitOk(self):

        # Alert user...
        messageDialog = Gtk.MessageDialog(
            self._assistant, Gtk.DialogFlags.MODAL, Gtk.MessageType.INFO, 
            Gtk.ButtonsType.OK, 
            "Disc verification was successful. Your disc is probably fine.")
        messageDialog.run()
        messageDialog.destroy()

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
            "Disc verification failed.")
        messageDialog.format_secondary_text(message)
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


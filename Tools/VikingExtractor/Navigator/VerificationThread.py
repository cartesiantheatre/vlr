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
import hashlib
import os
import threading
import time
from gi.repository import Gtk, GObject

# This thread is responsible for verifying data integrity of the disc...
class VerificationThread(threading.Thread):

    # Constructor...
    def __init__(self, builder):

        # Call the base class's constructor...
        super(VerificationThread, self).__init__()

        # Initialize state...
        self._builder = builder
        self._assistant = self._builder.get_object("AssistantInstance")

        # List of file pairs (path, checksum) to check...
        self._files = [("/home/kip/Projects/Avaneya: Viking Lander Remastered/Mastered/Science Digital Data Preservation Task/Processed Images-9.7z", "1569e25c8b159d32025f9ed4467adcc9")]
        self._totalVerifiedSize = 0
        self._totalFileSize = 0
        
        # When this is true, the thread is done...
        self._finished = False

        # Find the progress bar...
        self._integrityProgressBar = self._builder.get_object("integrityProgressBar")

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
            GObject.idle_add(self._updateGUI, fraction)
            
            # Something requested the thread quit...
            if self._finished:
                return 0

        # Return the calculated hash...
        return md5Hash.hexdigest()

    # Update the GUI. This callback is safe to update the GUI because it has
    #  been scheduled to execute safely...
    def _updateGUI(self, fraction):

        # Update the progress bar...
        self._integrityProgressBar.set_fraction(fraction)

        # Done...
        if fraction >= 1.0:

            # Alert user...
            messageDialog = Gtk.MessageDialog(
                self._assistant, Gtk.DialogFlags.MODAL, Gtk.MessageType.INFO, 
                Gtk.ButtonsType.OK, 
                "Disc verification was successful. Your disc appears to be fully intact.")
            messageDialog.run()
            messageDialog.destroy()

            # Advance to the next page...
            currentPageIndex = self._assistant.get_current_page()
            self._assistant.set_current_page(currentPageIndex + 1)

        # Remove function from list of event sources and don't call again until 
        #  needed...
        return False

    # Thread entry point...
    def run(self):
        print("Verification thread executing...")
        
        # Calculate the total file size of all files...
        for (currentFile, correctHexDigest) in self._files:
            
            # Try to read it...
            try:
                fileSize = os.path.getsize(currentFile)
            except OSError:
                self._setQuitWithError(
                    "I was not able to check a file I require:\n\n{0}".format(filePath))

            # Add the file's size to the total size...
            self._totalFileSize += fileSize

        # Check the checksums of all files...
        for (currentFile, correctHexDigest) in self._files:

            # Calculate checksum...
            currentHexDigest = self._calculateChecksum(currentFile)

            # Something requested the thread quit...
            if self._finished:
                return

            # Mismatch...
            if currentHexDigest != correctHexDigest:
                
                # Alert user...
                messageDialog = Gtk.MessageDialog(
                    self._assistant, Gtk.DialogFlags.MODAL, Gtk.MessageType.WARNING, 
                    Gtk.ButtonsType.YES_NO, 
                    "Disc verification failed.")
                messageDialog.format_secondary_text(
                    "The following file appears to be corrupt:\n\n{0}\n\nDo you wish to continue?".format(
                        currentFile))
                userResponse = messageDialog.run()
                messageDialog.destroy()

                # User wants to continue...
                if userResponse == Gtk.ResponseType.YES:
                    continue
                
                # User doesn't want to continue...
                else:
                    self._setQuitWithError("User requested to the verification process.")
                    return

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
        messageDialog.run()
        messageDialog.destroy()

    # Quit the thread...
    def setQuit(self):
        self._finished = True


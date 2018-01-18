# VikingExtractor, to recover images from Viking Lander operations.
# Copyright (C) 2010-2018 Cartesian Theatre™ <info@cartesiantheatre.com>.
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
import time
import errno
from sys import exit

# Our support modules...
import LauncherArguments

# Assistant proxy page base class...
from PageProxyBase import *

# Class containing behaviour for the two disc verification pages...
class VerificationProgressPageProxy(PageProxyBase):

    # Constructor...
    def __init__(self, launcherApp):

        # Initialize...
        super(VerificationProgressPageProxy, self).__init__(launcherApp)
        self._builder           = launcherApp.builder
        self._isVerifying       = False
        self._stopVerification  = False

        # Add the verification progress page to the assistant...
        self.registerPage(
            "verificationProgressPageBox",
            _("Verification Progress"),
            Gtk.AssistantPageType.PROGRESS,
            False)

        # Counters for tracking progress...
        self._totalVerifiedSize = 0
        self._totalFileSize = 0

        # Find the user interface elements...
        self._verificationImage = self._builder.get_object("verificationImage")
        self._verificationProgressPageBox = self._builder.get_object("verificationProgressPageBox")
        self._verificationProgressBar = self._builder.get_object("verificationProgressBar")

        # Connect the signals...
        stopVerificationButton = self._builder.get_object("stopVerificationButton")
        stopVerificationButton.connect("clicked", self.onStopVerificationPressed)

        # Note: See onMapEvent preamble...
        self._verificationProgressPageBox.connect_after("map", self.onMapEvent)

    # This is a stupid hack because neither calling startDiscVerification() via
    #  idle_add or directly in onPrepare() callback works properly. In the 
    #  former case, the code executes and works, but the entire UI does not 
    #  repaint during the job, even though we pump the message queue regularly.
    #  Some have said idle callbacks should never be used for long jobs. Be that
    #  as it may, if I call the method directly in the onPrepare() callback with 
    #  it regularly pumping the message queue, at the end of the verification 
    #  the entire main loop dies for a reason I cannot figure out and no one on
    #  the Gtk+ mailing list seems to either. This callback is invoked when
    #  show() is called on the widget...
    def onMapEvent(self, *dummy):

        # If the verification toggle wasn't selected, skip the page...
        self._yesToggle = self._builder.get_object("performVerificationCheckRadio")
        if self._yesToggle.get_active() == False:
            GObject.idle_add(self.nextPage)
            return

        # Make sure the page is not marked complete until after the thread
        #  exits...
        self._assistant.set_page_complete(self.getPageInGroup(0), False)
        self.startDiscVerification()

    # Calculate a file's MD5 checksum...
    def _calculateChecksum(self, filePath):

        # Try to open the file in readonly binary mode...
        try:
            fileDescriptor = open(filePath, 'rb')

        # Or check for failure...
        except OSError as Error:
            raise IOError(errno.ENOMSG, 
                _("I was not able to check a file I require. {0}:\n\n{1}").
                    format(Error.strerror, currentFile))

        # Construct a hash object for MD5 algorithm...
        md5Hash = hashlib.md5()

        # Calculate checksum...
        while True:

            # Read up to a 32K buffer. We could have set this to read as much
            #  as the system allows in one pass which would have overall taken
            #  less time, but the UI would lock and we need to take breaks to
            #  flush the message queue...
            fileBuffer = fileDescriptor.read(1024 * 32)

            # I/O error or reached the end of the file...
            if not fileBuffer:
                break

            # Update the hash...
            md5Hash.update(fileBuffer)

            # Remember how much of the total size we've checked and calculate 
            #  the progress...
            self._totalVerifiedSize += len(fileBuffer)
            fraction = self._totalVerifiedSize / self._totalFileSize

            # Update the GUI...
            self._updateGUI(filePath, fraction)

            # User requested we stop...
            if self._stopVerification:
                return 0

        # Return the calculated hash...
        return md5Hash.hexdigest()

    # Create list of file pairs (path, checksum) to check...
    def _parseChecksumManifest(self):

        # Reset the files list...
        self._files = []

        # Create path to file manifest...
        manifestPath = os.path.join(
            LauncherArguments.getArguments().missionDataRoot, 
            "Checksums")

        # Try to open the checksum manifest file...
        try:
            manifestFile = open(manifestPath)

        # Or check for failure...
        except IOError as Error:
            raise IOError(errno.ENOMSG, 
                _("I could not locate the list of files to verify:\n\n{0}\n\n{1}").
                    format(manifestPath, Error.strerror))

        # Parse each line...
        for line in manifestFile:
            
            # Check to make sure line is sane...
            if len(line) <= 35:
                continue
            
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

    # Our page in the assistent is being constructed, but not visible yet...
    def onPrepare(self):
        pass

    # Skip to the next page, called as an idle callback because doesn't work
    #  any other way it seems with some versions of Gtk+ 3...
    def nextPage(self):
        self._assistant.next_page()
        return False

    # Check if the verification is already running...
    def isVerifying(self):
        return self._isVerifying

    # Start the disc verification...
    def startDiscVerification(self):

        # Check if already running...
        if self.isVerifying():
            return False

        # Try to verify the disc...
        try:

            # Update state...
            self._isVerifying       = True
            self._stopVerification  = False

            # Change to busy cursor...
            self._launcher.setBusy(True)

            # Make sure usual assistant buttons are temporarily disabled...
            self._assistant.set_page_complete(self.getPageInGroup(0), False)
            self._assistant.update_buttons_state()

            # Load the verification animation...
            animation = GdkPixbuf.PixbufAnimation.new_from_file(
                os.path.join(
                    LauncherArguments.getArguments().dataRoot, "Verification.gif"))
            self._verificationImage.set_from_animation(animation)
            self._verificationImage.show()

            # Create list of file pairs (path, checksum) to check...
            self._parseChecksumManifest()

            # Calculate the total file size of all files...
            for (correctHexDigest, currentFile) in self._files:

                # Try to read it...
                try:
                    fileSize = os.path.getsize(currentFile)

                # Something bad happened, bail...
                except OSError as Error:
                    raise IOError(errno.ENOMSG, 
                        _("I was not able to check a file I require. {0}:\n\n{1}").
                            format(Error.strerror, currentFile))

                # Add the file's size to the total size...
                self._totalFileSize += fileSize

            # Check the checksums of all files...
            for (correctHexDigest, currentFile) in self._files:
                
                print("Verifying {0}".format(currentFile))

                # Calculate checksum...
                currentHexDigest = self._calculateChecksum(currentFile)

                # Something requested we stop or there was a problem generating the 
                #  checksum ...
                if self._stopVerification or not currentHexDigest:
                    self._resetState()
                    return False

                # Mismatch...
                if currentHexDigest != correctHexDigest:
                    raise IOError(errno.ENOMSG, 
                        _("Your disc might be damaged. It is recommended that you "
                        "replace it before continuing. The following file appeared "
                        "to be corrupt:\n\n<tt>{0}</tt>\n\n").format(currentFile))

        # Failed...
        except IOError as exception:

            # Show the error...
            self._setQuitWithError(exception.strerror)
            
            # Done and don't add to idle queue again...
            return False

        # No matter what, always make sure to reset state...
        finally:
            self._resetState()

        # Alert user everything was fine...
        self._setQuitOk()

        # Done and don't add to idle queue again...
        return False

    # User requested to stop disc verification...
    def onStopVerificationPressed(self, button):

        # Signal the verification to quit...
        self.setQuit()

        # Alert user...
        self._setQuitWithError(_("Disc verification was cancelled."))

    # Reset verification state...
    def _resetState(self):
        self._isVerifying       = False
        self._stopVerification  = False

    # Update the GUI...
    def _updateGUI(self, currentFile, fraction):

        # Update the progress bar...
        self._verificationProgressBar.set_fraction(fraction)
        self._verificationProgressBar.set_text("{0} ({1:.0f}%)".format(os.path.split(currentFile)[1], fraction * 100))

        # Flush the event queue so we don't block...
        while Gtk.events_pending():
            Gtk.main_iteration()

    # Verification done. Report success...
    def _setQuitOk(self):

        # Alert user...
        messageDialog = Gtk.MessageDialog(
            self._assistant, Gtk.DialogFlags.MODAL, Gtk.MessageType.INFO, 
            Gtk.ButtonsType.OK, 
            _("The disc verification completed successfully. Your disc is probably fine."))
        messageDialog.run()
        messageDialog.destroy()

        # Reset state...
        self._resetState()

        # Reset the cursor to normal in case something changed it...
        self._assistant.get_root_window().set_cursor(None)

        # Page is complete now...
        self._assistant.set_page_complete(self.getPageInGroup(0), True)

        # Advance to the next page...
        currentPageIndex = self._assistant.get_current_page()
        self._assistant.set_current_page(currentPageIndex + 1)

    # Quit the verification and show an error message...
    def _setQuitWithError(self, message):

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

    # Stop verification...
    def setQuit(self):
        self._stopVerification = True


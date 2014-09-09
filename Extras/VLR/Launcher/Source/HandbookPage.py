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
from gi.repository import Gtk
from gi.repository import Gdk
from gi.repository import GObject
import urllib.request
import urllib.error
import os
import errno
import platform

from Miscellaneous import *

# Assistant proxy page base class...
from PageProxyBase import *

# Handbook page proxy class...
class HandbookPageProxy(PageProxyBase):

    # Constructor...
    def __init__(self, launcherApp):

        # Initialize...
        super(HandbookPageProxy, self).__init__(launcherApp)
        self._handbookUrl   = "https://www.avaneya.com/downloads/Avaneya_Project_Crew_Handbook.pdf"
        self._stopDownload  = False
        self._handbookDownloaded = False;

        # Add handbook page to assistant...
        self.registerPage(
            "handbookPageBox",
            _("Download Handbook"),
            Gtk.AssistantPageType.PROGRESS,
            True)

        # Shortcuts to our widgets...
        self._downloadHandbookToggle = self._builder.get_object("downloadHandbookToggle")
        self._progressBar = self._builder.get_object("handbookDownloadProgress")
        self._stopHandbookDownloadButton = self._builder.get_object("stopHandbookDownloadButton")

        # Connect the signals...
        self._downloadHandbookToggle.connect("toggled", self.onDownloadHandbookToggled)
        self._stopHandbookDownloadButton.connect("clicked", self.onStopHandbookDownload)

    # Download handbook button toggled...
    def onDownloadHandbookToggled(self, toggleButton):

        # Reset stop download flag in case user sets it later...
        self._stopDownload = False

        # User requested to download the latest handbook...
        if toggleButton.get_active():

            # Update the GUI...
            toggleButton.set_sensitive(False)
            self._assistant.set_page_complete(self.getPageInGroup(0), False)

            # Check network connectivity to the internet and alert user if no
            #  connection available...
            if not hasInternetConnection(True, self._assistant, True):

                # Update GUI...
                self._progressBar.hide()
                self._stopHandbookDownloadButton.hide()
                toggleButton.set_sensitive(True)
                self._assistant.set_page_complete(self.getPageInGroup(0), True)
                
                # Deactivate the download button...
                toggleButton.set_active(False)

                # Don't do anything further...
                return
            
            # Get just the filename from the download URL...
            fileName = self._handbookUrl.split('/')[-1]

            # Prepare a Save As file chooser dialog to save the handbook...
            dialog = Gtk.FileChooserDialog(
                _("Please choose a download location..."), 
                self._assistant,
                Gtk.FileChooserAction.SAVE,
                (Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL, Gtk.STOCK_SAVE_AS, Gtk.ResponseType.OK))
            dialog.set_local_only(True)
            dialog.set_do_overwrite_confirmation(True)
            dialog.set_current_name(fileName)
            dialog.set_default_response(Gtk.ResponseType.OK)

            # Default to user's desktop, if we can find it...
            desktop = os.path.join(os.path.expanduser('~'), 'Desktop')
            if os.path.isdir(desktop):
                dialog.set_current_folder(desktop)

            # Run the dialog and capture the response...
            userResponse = dialog.run()

            # User cancelled...
            if userResponse != Gtk.ResponseType.OK:

                # Update GUI...
                self._progressBar.hide()
                self._stopHandbookDownloadButton.hide()
                toggleButton.set_sensitive(True)
                self._assistant.set_page_complete(self.getPageInGroup(0), True)

                # Deactivate the download button...
                toggleButton.set_active(False)

                # Cleanup the dialog...
                dialog.destroy()

                # Don't do anything further...
                return

            # Get the selected file name...
            fileName = dialog.get_filename()

            # Cleanup the dialog...
            dialog.destroy()

            # Show the download progress bar...
            self._progressBar.show()
            self._stopHandbookDownloadButton.show()
            self._progressBar.set_text(_("Contacting server, please wait..."))

            # Try to download...
            try:

                # Flush the event queue so we don't block...
                while Gtk.events_pending():
                    Gtk.main_iteration()

                # Create a URL stream with a 10 second timeout...
                urlStream = urllib.request.urlopen(self._handbookUrl, None, 10)
                
                # Create the file on disk...
                fileHandle = open(fileName, 'wb')
                
                # Get its length...
                fileSize = int(urlStream.headers["Content-Length"])
                
                # Remember how much we have downloaded so far so we can calculate 
                #  progress...
                fileSizeCompleted = 0
                
                # Download block size of 8K...
                blockSize = 8192

                # Receive loop...
                while True:

                    # User clicked the stop button...
                    if self._stopDownload:
                        raise IOError(errno.ENOMSG, 
                            _("The download of the handbook was stopped."))

                    # Fill the buffer...
                    fileBuffer = urlStream.read(blockSize)

                    # No more left...
                    if not fileBuffer:
                        break

                    # Save to disk...
                    fileHandle.write(fileBuffer)

                    # Calculate progress and update progress bar...
                    fileSizeCompleted += len(fileBuffer)
                    progress = fileSizeCompleted / fileSize
                    self._progressBar.set_text("{0:.1f} MB of {1:.1f} MB ({2:.0f}%)".
                        format(
                            fileSizeCompleted / (1024**2), 
                            fileSize / (1024**2), 
                            progress * 100))
                    self._progressBar.set_fraction(progress)

                    # Flush the event queue so we don't block...
                    while Gtk.events_pending():
                        Gtk.main_iteration()

                # Done. Close the stream...
                fileHandle.close()
                
                # Remember that download was successful...
                self._handbookDownloaded = True

                # Note that the page is ready to advance...
                self._assistant.set_page_complete(self.getPageInGroup(0), True)
                
                # Alert user that the download is done...
                messageDialog = Gtk.MessageDialog(
                    self._assistant, Gtk.DialogFlags.MODAL, Gtk.MessageType.QUESTION, 
                    Gtk.ButtonsType.YES_NO, 
                    _("The book downloaded successfully. Would you like to open it?"))
                messageDialog.set_default_response(Gtk.ResponseType.YES)
                userResponse = messageDialog.run()
                messageDialog.destroy()

                # User requested to open the book...
                if userResponse == Gtk.ResponseType.YES:
                    launchResource(fileName)

                # Actually advance to the next page...
                currentPageIndex = self._assistant.get_current_page()
                self._assistant.set_current_page(currentPageIndex + 1)

            # Problem finding the URL, e.g. 404...
            except urllib.error.URLError as exception:

                # Untoggle the download button...
                toggleButton.set_active(False)

                # Alert user...
                messageDialog = Gtk.MessageDialog(
                    self._assistant, Gtk.DialogFlags.MODAL, Gtk.MessageType.ERROR, 
                    Gtk.ButtonsType.OK, 
                    _("There was a problem communicating with the remote server. "
                    "Please try again later.\n\n\t{0}").
                        format(exception.reason))
                messageDialog.run()
                messageDialog.destroy()

            # Couldn't write to disk...
            except (OSError, IOError) as exception:

                # Untoggle download button...
                toggleButton.set_active(False)

                # Alert user...
                messageDialog = Gtk.MessageDialog(
                    self._assistant, Gtk.DialogFlags.MODAL, Gtk.MessageType.ERROR, 
                    Gtk.ButtonsType.OK, 
                    exception.strerror)
                messageDialog.run()
                messageDialog.destroy()

            # Regardless of whether there was an exception or not...
            finally:

                # Update the GUI...
                toggleButton.set_sensitive(not self._handbookDownloaded)
                self._progressBar.hide()
                self._progressBar.set_fraction(0.0)
                self._stopHandbookDownloadButton.hide()
                self._assistant.set_page_complete(self.getPageInGroup(0), True)

        # User did not want to download the handbook...
        else:

            # Update the GUI...
            toggleButton.set_sensitive(True)
            self._progressBar.hide()
            self._stopHandbookDownloadButton.hide()
            self._assistant.set_page_complete(self.getPageInGroup(0), True)

    # Stop handbook download button pressed...
    def onStopHandbookDownload(self, stopButton):
        self._stopDownload = True


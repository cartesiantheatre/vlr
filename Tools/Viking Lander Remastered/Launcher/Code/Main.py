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
import urllib.request
import os
import platform
import re
import dbus

# Our support modules...
import Options
from Splash import NavigatorSplash

from VerificationThread import VerificationThread
from VerificationPages import VerificationPages

# Navigator class...
class NavigatorApp():

    # Constructor...
    def __init__(self):

        # Initialize Glade builder...
        self.builder = Gtk.Builder()
        self.builder.add_from_file("../Gooey/Launcher.glade")

        # Find the assistant...
        self.assistant = self.builder.get_object("Assistant")

        # Construct composites objects...
        self.navigatorSplash = NavigatorSplash(self)
        self.verificationPages = VerificationPages(self)

        print("NavigatorApp constructing...")


        self.builder.connect_signals(self)

        # Show the splash window...
        self.navigatorSplash.showSplash()

        # Set the forward function which determines next page to show...
        self.assistant.set_forward_page_func(self.forwardPage, None)

        # Welcome page...
        self.welcomePageBox = self.builder.get_object("welcomePageBox")
        self.welcomePageBox.set_border_width(5)
        self.assistant.append_page(self.welcomePageBox)
        self.assistant.set_page_title(self.welcomePageBox, "Introduction")
        self.assistant.set_page_type(self.welcomePageBox, Gtk.AssistantPageType.INTRO)
        self.assistant.set_page_complete(self.welcomePageBox, True)
        
        # Verification info page...
        self.verificationInfoPageBox = self.builder.get_object("verificationInfoPageBox")
        self.verificationInfoPageBox.set_border_width(5)
        self.assistant.append_page(self.verificationInfoPageBox)
        self.assistant.set_page_title(self.verificationInfoPageBox, "Verification Prompt")
        self.assistant.set_page_type(self.verificationInfoPageBox, Gtk.AssistantPageType.CONTENT)
        self.assistant.set_page_complete(self.verificationInfoPageBox, True)

        # Verification progress page...
        self.verificationProgressPageBox = self.builder.get_object("verificationProgressPageBox")
        self.verificationProgressPageBox.set_border_width(5)
        self.assistant.append_page(self.verificationProgressPageBox)
        self.assistant.set_page_title(
            self.verificationProgressPageBox, "Verification Progress")
        self.assistant.set_page_type(
            self.verificationProgressPageBox, Gtk.AssistantPageType.PROGRESS)
        self.assistant.set_page_complete(self.verificationProgressPageBox, False)

        # Fetch handbook page...
        self.handbookPageBox = self.builder.get_object("handbookPageBox")
        self.handbookPageBox.set_border_width(5)
        self.assistant.append_page(self.handbookPageBox)
        self.assistant.set_page_title(self.handbookPageBox, "Download Handbook")
        self.assistant.set_page_type(self.handbookPageBox, Gtk.AssistantPageType.CONTENT)
        self.assistant.set_page_complete(self.handbookPageBox, True)

        # Select save folder page...
        self.selectSaveFolderPageBox = self.builder.get_object("selectSaveFolderPageBox")
        self.selectSaveFolderPageBox.set_border_width(5)
        self.assistant.append_page(self.selectSaveFolderPageBox)
        self.assistant.set_page_title(self.selectSaveFolderPageBox, "Select Save Folder")
        self.assistant.set_page_type(self.handbookPageBox, Gtk.AssistantPageType.CONTENT)
        self.assistant.set_page_complete(self.selectSaveFolderPageBox, True)

        # Configuration intro page...
        self.configureIntroPageBox = self.builder.get_object("configureIntroPageBox")
        self.configureIntroPageBox.set_border_width(5)
        self.assistant.append_page(self.configureIntroPageBox)
        self.assistant.set_page_title(self.configureIntroPageBox, "Configure Introduction")
        self.assistant.set_page_type(self.configureIntroPageBox, Gtk.AssistantPageType.CONTENT)
        self.assistant.set_page_complete(self.configureIntroPageBox, True)
        
        # Configure output layout page...
        self.configureOutputLayoutPageBox = self.builder.get_object("configureOutputLayoutPageBox")
        self.configureOutputLayoutPageBox.set_border_width(5)
        self.assistant.append_page(self.configureOutputLayoutPageBox)
        self.assistant.set_page_title(self.configureOutputLayoutPageBox, "Configure Layout")
        self.assistant.set_page_type(self.configureOutputLayoutPageBox, Gtk.AssistantPageType.CONTENT)
        self.assistant.set_page_complete(self.configureOutputLayoutPageBox, True)

        # Configure recovery page...
        self.configureRecoveryPageBox = self.builder.get_object("configureRecoveryPageBox")
        self.configureRecoveryPageBox.set_border_width(5)
        self.assistant.append_page(self.configureRecoveryPageBox)
        self.assistant.set_page_title(self.configureRecoveryPageBox, "Configure Recovery")
        self.assistant.set_page_type(self.configureRecoveryPageBox, Gtk.AssistantPageType.CONTENT)
        self.assistant.set_page_complete(self.configureRecoveryPageBox, True)

        # Configure filters page...
        self.configureFiltersPageBox = self.builder.get_object("configureFiltersPageBox")
        self.configureFiltersPageBox.set_border_width(5)
        self.assistant.append_page(self.configureFiltersPageBox)
        self.assistant.set_page_title(self.configureFiltersPageBox, "Configure Filters")
        self.assistant.set_page_type(self.configureFiltersPageBox, Gtk.AssistantPageType.CONTENT)
        self.assistant.set_page_complete(self.configureFiltersPageBox, True)

        # Configure advanced page...
        self.configureAdvancedPageBox = self.builder.get_object("configureAdvancedPageBox")
        self.configureAdvancedPageBox.set_border_width(5)
        self.assistant.append_page(self.configureAdvancedPageBox)
        self.assistant.set_page_title(self.configureAdvancedPageBox, "Configure Advanced")
        self.assistant.set_page_type(self.configureAdvancedPageBox, Gtk.AssistantPageType.CONTENT)
        self.assistant.set_page_complete(self.configureAdvancedPageBox, True)

        # Confirm page...
        self.confirmPageBox = self.builder.get_object("confirmPageBox")
        self.confirmPageBox.set_border_width(5)
        self.assistant.append_page(self.confirmPageBox)
        self.assistant.set_page_title(self.confirmPageBox, "Confirmation")
        self.assistant.set_page_type(self.confirmPageBox, Gtk.AssistantPageType.CONFIRM)
        self.assistant.set_page_complete(self.confirmPageBox, True)

        # Final page...
        self.finalPageBox = self.builder.get_object("finalPageBox")
        self.finalPageBox.set_border_width(5)
        self.assistant.append_page(self.finalPageBox)
        self.assistant.set_page_title(self.finalPageBox, "Goodbye")
        self.assistant.set_page_type(self.finalPageBox, Gtk.AssistantPageType.SUMMARY)
        self.assistant.set_page_complete(self.finalPageBox, True)

    # Download handbook button toggled...
    def onDownloadHandbookToggled(self, button):

        # Find the download progress bar...
        progressBar = self.builder.get_object("handbookDownloadProgress")

        # User requested to download the latest handbook...
        if button.get_active():

            # Update the GUI...
            button.set_sensitive(False)
            self.assistant.set_page_complete(self.handbookPageBox, False)

            # Check network connectivity to the internet and alert user if 
            #  caller requests if no connection available...
            if not isInternetConnectionDetected(True, self.assistant, True):

                # Update GUI...
                progressBar.hide()
                button.set_sensitive(True)
                self.assistant.set_page_complete(self.handbookPageBox, True)
                
                # Deactivate the download button...
                button.set_active(False)

                # Don't do anything further...
                return
            
            # The URL to the latest copy of the handbook and just the filename...
            handbookUrl = "https://www.avaneya.com/downloads/Avaneya_Project_Crew_Handbook.pdf"
            fileName = handbookUrl.split('/')[-1]

            # Prepare a save as file chooser dialog to save the handbook...
            dialog = Gtk.FileChooserDialog(
                "Please choose a download location...", 
                self.assistant,
                Gtk.FileChooserAction.SAVE,
                (Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL, Gtk.STOCK_SAVE_AS, Gtk.ResponseType.OK))
            dialog.set_local_only(True)
            dialog.set_do_overwrite_confirmation(True)
            dialog.set_current_name(fileName)
            dialog.set_default_response(Gtk.ResponseType.OK)

            # Run the dialog and capture the response...
            userResponse = dialog.run()
            
            # User cancelled...
            if userResponse != Gtk.ResponseType.OK:

                # Update GUI...
                progressBar.hide()
                button.set_sensitive(True)
                self.assistant.set_page_complete(self.handbookPageBox, True)
                
                # Deactivate the download button...
                button.set_active(False)

                # Cleanup the dialog...
                dialog.destroy()

                # Don't do anything further...
                return
            
            # Get the selected file name...
            fileName = dialog.get_filename()

            # Cleanup the dialog...
            dialog.destroy()

            # Show the download progress bar...
            progressBar.show()
            progressBar.set_text("Contacting server, please wait...")

            # Try to download...
            try:

                # Flush the event queue so we don't block...
                while Gtk.events_pending():
                    Gtk.main_iteration()

                # Create a URL stream with a 10 second timeout...
                urlStream = urllib.request.urlopen(handbookUrl, None, 10)
                
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
                
                    # Fill the buffer...
                    fileBuffer = urlStream.read(blockSize)

                    # No more left...
                    if not fileBuffer:
                        break

                    # Save to disk...
                    fileHandle.write(fileBuffer)

                    # Calculate progress and update progress bar...
                    fileSizeCompleted += len(fileBuffer)
                    progressBar.set_text(None)
                    progressBar.set_fraction(fileSizeCompleted / fileSize)

                    # Flush the event queue so we don't block...
                    while Gtk.events_pending():
                        Gtk.main_iteration()

                # Done. Close the stream and note that the page is ready to advance...
                fileHandle.close()
                self.assistant.set_page_complete(self.handbookPageBox, True)

                # Alert user that the download is done...
                messageDialog = Gtk.MessageDialog(
                    self.assistant, Gtk.DialogFlags.MODAL, Gtk.MessageType.QUESTION, 
                    Gtk.ButtonsType.YES_NO, 
                    "The book downloaded successfully. Would you like to open it?")
                messageDialog.set_default_response(Gtk.ResponseType.YES)
                userResponse = messageDialog.run()
                messageDialog.destroy()
                
                # User requested to open the book...
                if userResponse == Gtk.ResponseType.YES:

                    # GNU. Try via freedesktop method...
                    if platform.system() == "Linux":
                        os.system("xdg-open \"{0}\"".format(fileName))
                    
                    # Winblows system...
                    elif platform.system() == "Windows":
                        os.startfile(fileName)

            # Problem finding the URL, e.g. 404...
            except urllib.error.URLError as exception:

                # Deactivate the download button...
                button.set_active(False)

                # Alert user...
                messageDialog = Gtk.MessageDialog(
                    self.assistant, Gtk.DialogFlags.MODAL, Gtk.MessageType.ERROR, 
                    Gtk.ButtonsType.OK, 
                    "There was a problem communicating with the remote server. Please try again later.\n\n{0}".
                        format(exception.reason))
                messageDialog.run()
                messageDialog.destroy()

            # Couldn't write to disk...
            except IOError as exception:

                # Deactivate the download button...
                button.set_active(False)

                # Alert user...
                messageDialog = Gtk.MessageDialog(
                    self.assistant, Gtk.DialogFlags.MODAL, Gtk.MessageType.ERROR, 
                    Gtk.ButtonsType.OK, 
                    "Unable to save the handbook to the requested location:\n\n{0}".
                        format(exception.strerror))
                messageDialog.run()
                messageDialog.destroy()

            # Regardless of whether there was an exception or not...
            finally:
                
                # Update the GUI...
                button.set_sensitive(True)
                progressBar.hide()
                self.assistant.set_page_complete(self.handbookPageBox, True)

        # User did not want to download the handbook...
        else:

            # Update the GUI...
            button.set_sensitive(True)
            progressBar.hide()
            self.assistant.set_page_complete(self.handbookPageBox, True)

    # End of current page. Next page is being constructed but not visible yet...
    def onPrepareEvent(self, assistant, currentPage):

        # Reset the cursor to normal in case something changed it...
        self.assistant.get_root_window().set_cursor(None)

        # Transitioning into verification progress page...
        if currentPage is self.verificationProgressPageBox:

            # Don't start the disc verification thread if user requested to 
            #  skip it...
            if self.builder.get_object("skipVerificationCheckRadio").get_active():
                return

            # Change to busy cursor...
            cursorWatch = Gdk.Cursor.new(Gdk.CursorType.WATCH)
            self.assistant.get_root_window().set_cursor(cursorWatch)

            # Start the disc verification...
            self.startDiscVerification()
        
        # Transitioning into confirm page...
        elif currentPage is self.confirmPageBox:
            
            # Prepare the VikingExtractor arguments based on user selected
            #  configuration and show the summary...
            self.prepareVikingExtractor()

    # End of current page. Calculate index of next page...
    def forwardPage(self, currentPageIndex, userData):

        # Get what would be the next page...
        nextPage = self.assistant.get_nth_page(currentPageIndex)

        # Transitioning into verification progress page...
        if nextPage is self.verificationProgressPageBox and \
           self.builder.get_object("skipVerificationCheckRadio").get_active():

                # Skip the disc verification check...
                self.assistant.set_current_page(currentPageIndex + 1)
                return currentPageIndex + 2

        # Any other page just transition to the next one...
        else:
            return currentPageIndex + 1

    # Prepare the VikingExtractor arguments based on user selected
    #  configuration and show the summary...
    def prepareVikingExtractor(self):
    
        # List that will contain all VikingExtractor command line options...
        commandLineInterface = []

        # Get the confirmation text buffer...
        confirmTextBuffer = self.builder.get_object("confirmTextBuffer")
        
        # Clear it, if not already...
        confirmTextBuffer.set_text("")

        # Output folder...
        outputFolder = self.builder.get_object("saveFolderChooser").get_filename()
        confirmTextBuffer.insert(confirmTextBuffer.get_end_iter(), 
            "Save to: {0}\n".format(outputFolder))

        # Overwrite...
        active = self.builder.get_object("overwriteOutputCheckButton").get_active()
        confirmTextBuffer.insert(confirmTextBuffer.get_end_iter(), 
            "Output will be overwritten: {0}\n".format(boolToYesNo(active)))
        if active:
            commandLineInterface.append("--overwrite")

        # Directorize by...
        confirmTextBuffer.insert(confirmTextBuffer.get_end_iter(), "Directorize by...\n")
        
        # ...diode band class...
        active = self.builder.get_object("directorizeBandTypeClassCheckButton").get_active()
        confirmTextBuffer.insert(confirmTextBuffer.get_end_iter(), 
            "\t...diode band class: {0}\n".format(boolToYesNo(active)))
        if active:
            commandLineInterface.append("--directorize-band-class")

        # ...Martian location...
        active = self.builder.get_object("directorizeLocationCheckButton").get_active()
        confirmTextBuffer.insert(confirmTextBuffer.get_end_iter(), 
            "\t...Martian location: {0}\n".format(boolToYesNo(active)))
        if active:
            commandLineInterface.append("--directorize-location")

        # ...Martian month...
        active = self.builder.get_object("directorizeMonthCheckButton").get_active()
        confirmTextBuffer.insert(confirmTextBuffer.get_end_iter(), 
            "\t...Martian month: {0}\n".format(boolToYesNo(active)))
        if active:
            commandLineInterface.append("--directorize-month")

        # ...mission solar day...
        active = self.builder.get_object("directorizeSolCheckButton").get_active()
        confirmTextBuffer.insert(confirmTextBuffer.get_end_iter(), 
            "\t...mission solar day: {0}\n".format(boolToYesNo(active)))
        if active:
            commandLineInterface.append("--directorize-sol")

        # No automatic rotation...
        active = self.builder.get_object("noAutomaticRotationCheckButton").get_active()
        confirmTextBuffer.insert(confirmTextBuffer.get_end_iter(), 
            "Disable automatic image component orientation: {0}\n".format(boolToYesNo(active)))
        if active:
            commandLineInterface.append("--no-auto-rotate")

        # No reconstruction...
        active = self.builder.get_object("noReconstructionCheckButton").get_active()
        confirmTextBuffer.insert(confirmTextBuffer.get_end_iter(), 
            "Dump components without reconstruction: {0}\n".format(boolToYesNo(active)))
        if active:
            commandLineInterface.append("--no-reconstruct")

        # Filters...
        confirmTextBuffer.insert(confirmTextBuffer.get_end_iter(), "Filters...\n")

        # ...band type class...
        diodeFilterComboBox = self.builder.get_object("diodeFilterComboBox")
        
        iterator = diodeFilterComboBox.get_active_iter()
        
        model = diodeFilterComboBox.get_model()
        humanValue = model.get_value(iterator, 1)
        commandLineValue = model.get_value(iterator, 1)

        confirmTextBuffer.insert(confirmTextBuffer.get_end_iter(), 
            "\t...diode band type class: {0}\n".format(model.get_value(iterator, 0)))
        commandLineInterface.append("--filter-diode={0}".format(commandLineValue))
        
        # ...lander...
        landerFilterComboBox = self.builder.get_object("landerFilterComboBox")
        
        iterator = landerFilterComboBox.get_active_iter()
        
        model = landerFilterComboBox.get_model()
        humanValue = model.get_value(iterator, 0)
        humanValue = re.sub('<[^>]*>', '', humanValue)
        commandLineValue = model.get_value(iterator, 1)

        confirmTextBuffer.insert(confirmTextBuffer.get_end_iter(), 
            "\t...lander: {0}\n".format(humanValue))
        commandLineInterface.append("--filter-lander={0}".format(commandLineValue))

        # Use image interlacing...
        active = self.builder.get_object("useInterlacingCheckButton").get_active()
        confirmTextBuffer.insert(confirmTextBuffer.get_end_iter(), 
            "Interlace images: {0}\n".format(boolToYesNo(active)))
        if active:
            commandLineInterface.append("--interlace")

        # Generate metadata...
        active = self.builder.get_object("generateMetadataCheckButton").get_active()
        confirmTextBuffer.insert(confirmTextBuffer.get_end_iter(), 
            "Generate metadata: {0}\n".format(boolToYesNo(active)))
        if active:
            commandLineInterface.append("--generate-metadata")

        # Verbosity...
        active = self.builder.get_object("verbosityCheckButton").get_active()
        confirmTextBuffer.insert(confirmTextBuffer.get_end_iter(), 
            "Be verbose: {0}\n".format(boolToYesNo(active)))
        if active:
            commandLineInterface.append("--verbose")

        # Multithreading...
        active = self.builder.get_object("multithreadingCheckButton").get_active()
        confirmTextBuffer.insert(confirmTextBuffer.get_end_iter(), 
            "Use multithreading: {0}\n".format(boolToYesNo(active)))
        if active:
            commandLineInterface.append("--jobs")

        # Create full commandline to invoke VikingExtractor...
        commandLine = " ".join(commandLineInterface)
        confirmTextBuffer.insert(confirmTextBuffer.get_end_iter(), 
            "Will execute: $ {0}\n".format(commandLine))

    # Apply button clicked...
    def onApplyEvent(self, *args):
        
        # For debugging purposes...
        print("onApplyEvent")

    # Cancel signal emitted when cancel button clicked...
    def onCancelEvent(self, assistant, *args):
        
        # For debugging purposes...
        print("onCancelEvent...")

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

        # Check if the verification thread is still running...
        if self.verificationThread and self.verificationThread.isAlive():

            # Alert...
            print("Stopping disc verification thread...")

            # Signal the thread to quit...
            self.verificationThread.setQuit()
            
            # Wait for it to quit...
            self.verificationThread.join()
        
        # Terminate...
        self.quit()

    # User requested that assistant be closed. The default signal handler would 
    #  just destroys the window. Cancel signal is sent automatically after this
    #  signal is handled...
    def onDeleteEvent(self, *args):

        # For debugging purposes...
        print("onDeleteEvent...")
        
        # Let cancel handler determine whether to exit or not...

    # Either close button of summary page clicked or apply button in last page
    #  in flow of type CONFIRM clicked...
    def onCloseEvent(self, assistant, *args):
        
        # For debugging purposes...
        print("onCloseEvent")
        
        # No threads should be running by this point, so safe to terminate...
        self.quit()

    # Run the GUI...
    def run(self):

        # Start processing events...    
        try:
            Gtk.main()
        
        # TODO: This doesn't work. The exception is never raised on a ctrl-c...
        except KeyboardInterrupt:
            print("KeyboardInterrupt")
            raise

        except:
            print("SomeOtherException")
            Gtk.main_quit()

    # Terminate...
    def quit(self):
        Gtk.main_quit()

# Helper function takes a boolean value and converts to a yes or no string...
def boolToYesNo(value):
    
    # Type check...
    assert(type(value) is bool)

    if value is True:
        return "Yes"
    else:
        return "No"

# Check network connectivity to the internet and alert user if caller requests 
#  if no connection available...
def isInternetConnectionDetected(alertUser, parent, unqueriableDefault):

    # Some useful constants for DBus NetworkManager interface...
    NM_STATE_CONNECTED = 3          # For 0.8 interface
    NM_STATE_CONNECTED_GLOBAL = 70  # For 0.9 interface

    # Open the session bus...
    systemBus = dbus.SystemBus()
    
    # Try to get the NetworkManager remote proxy object...
    try:
        networkManagerRemoteProxy = systemBus.get_object(
            "org.freedesktop.NetworkManager",  # Service name
            "/org/freedesktop/NetworkManager") # Object on the service
    
    # Something went wrong, but most likely the service isn't available...
    except dbus.exceptions.DBusException as exception:
        print("NetworkManager service not found. Silently ignoring...")

        # Alert caller...
        return unqueriableDefault

    # Get the connection state...
    connectionState = networkManagerRemoteProxy.state()

    # Connected. We're good...
    if connectionState == NM_STATE_CONNECTED or \
       connectionState == NM_STATE_CONNECTED_GLOBAL:

        # Inform caller...
        return True
    
    # Not connected...
    else:

        # Alert user, if caller requested...
        if alertUser:
            messageDialog = Gtk.MessageDialog(
                parent, Gtk.DialogFlags.MODAL, Gtk.MessageType.ERROR, 
                Gtk.ButtonsType.OK, 
                "You need a working internet connection in order to do this. " \
                "You don't appear to have one right now.")
            messageDialog.run()
            messageDialog.destroy()

        # Inform caller...
        return False

# Entry point...
if __name__ == '__main__':

    # Initialize threading environment...
    GObject.threads_init()
    
    # Start the navigator GUI...
    navigator = NavigatorApp()
    navigator.run()


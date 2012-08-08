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
from gi.repository import Gtk, Gdk, GObject
from optparse import OptionParser
import urllib.request
import os
import platform

# Our other modules...
from VerificationThread import VerificationThread

# Navigator class...
class NavigatorApp():

    # Constructor...
    def __init__(self):

        # Parse the command line...
        self.parseCommandLine()

        # Initialize Glade builder and connect signals...
        self.builder = Gtk.Builder()
        self.builder.add_from_file("Navigator/Navigator.glade")
        self.builder.connect_signals(self)

        # Show the splash window...
        self.showSplash()

        # Setup the assistant...
        self.setupAssistant()
        
        # Disc verification thread not active yet...
        self.verificationThread = None

    # Setup the assistant window...
    def setupAssistant(self):

        # Find the assistant...
        self.assistant = self.builder.get_object("AssistantInstance")
        
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

        # Select recovery folder page...
        self.selectRecoveryFolderPageBox = self.builder.get_object("selectRecoveryFolderPageBox")
        self.selectRecoveryFolderPageBox.set_border_width(5)
        self.assistant.append_page(self.selectRecoveryFolderPageBox)
        self.assistant.set_page_title(self.selectRecoveryFolderPageBox, "Select Recovery Folder")
        self.assistant.set_page_type(self.handbookPageBox, Gtk.AssistantPageType.CONTENT)
        self.assistant.set_page_complete(self.selectRecoveryFolderPageBox, True)

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
        parametersList = []

        # Get the confirmation text buffer...
        confirmTextBuffer = self.builder.get_object("confirmTextBuffer")
        
        # Clear it, if not already...
        confirmTextBuffer.set_text("")

        # Output folder...
        outputFolder = self.builder.get_object("recoveryFolderChooser").get_filename()
        confirmTextBuffer.insert(confirmTextBuffer.get_end_iter(), 
            "Output folder:\n\t{0}\n".format(outputFolder))

        # Overwrite...
        active = self.builder.get_object("overwriteOutputCheckButton").get_active()
        confirmTextBuffer.insert(confirmTextBuffer.get_end_iter(), 
            "Output will be overwritten:\n\t{0}\n".format(active))
        if active:
            parametersList.append("--overwrite")

        # Directorize by diode band class...
        active = self.builder.get_object("directorizeBandTypeClassCheckButton").get_active()
        confirmTextBuffer.insert(confirmTextBuffer.get_end_iter(), 
            "Directorize by diode band class:\n\t{0}\n".format(active))
        if active:
            parametersList.append("--directorize-band-class")

        # Directorize by Martian location...
        active = self.builder.get_object("directorizeLocationCheckButton").get_active()
        confirmTextBuffer.insert(confirmTextBuffer.get_end_iter(), 
            "Directorize by Martian location:\n\t{0}\n".format(active))
        if active:
            parametersList.append("--directorize-location")

        # Directorize by Martian month...
        active = self.builder.get_object("directorizeMonthCheckButton").get_active()
        confirmTextBuffer.insert(confirmTextBuffer.get_end_iter(), 
            "Directorize by Martian month:\n\t{0}\n".format(active))
        if active:
            parametersList.append("--directorize-month")

        # Directorize by mission solar day...
        active = self.builder.get_object("directorizeSolCheckButton").get_active()
        confirmTextBuffer.insert(confirmTextBuffer.get_end_iter(), 
            "Directorize by mission solar day:\n\t{0}\n".format(active))
        if active:
            parametersList.append("--directorize-sol")

        # Create full commandline to invoke VikingExtractor...
        commandLine = " ".join(parametersList)
        confirmTextBuffer.insert(confirmTextBuffer.get_end_iter(), 
            "Resulting Command:\n\t{0}\n".format(commandLine))

    # Start the disc verification...
    def startDiscVerification(self):

        # Already running...
        if self.verificationThread and self.verificationThread.isAlive():
            print("Disc verification thread already in progress...")
            return

        # Allocate and start the thread...
        self.verificationThread = VerificationThread(self.builder)
        self.verificationThread.start()

    # User requested to stop disc verification...
    def onStopVerificationPressed(self, button):

        # Signal the thread to quit...
        self.verificationThread.setQuit()
        
        # Wait for it to quit...
        self.verificationThread.join()
        
        # Mark as dead...
        self.verificationThread = None
        
        # Alert user...
        messageDialog = Gtk.MessageDialog(
            self.assistant, Gtk.DialogFlags.MODAL, Gtk.MessageType.WARNING, 
            Gtk.ButtonsType.OK, "Disc verification was cancelled.")
        messageDialog.run()
        messageDialog.destroy()
        
        # Mark page as complete...
        self.assistant.set_page_complete(self.verificationProgressPageBox, True)
        
        # Advance to the next page...
        currentPageIndex = self.assistant.get_current_page()
        self.assistant.set_current_page(currentPageIndex + 1)

    # Parse command line...
    def parseCommandLine(self):

        # Initialize and parse...
        optionParser = OptionParser()
        optionParser.add_option("--skip-splash", 
            action="store_true", 
            dest="skipSplash", 
            help="skip the splash screen", 
            default=False)
        (self.options, arguments) = optionParser.parse_args()

    # Keyboard or mouse pressed on the splash......
    def onSplashPressed(self, *arguments):
        
        # Cleanup the window...
        self.splashTimerDone(None)

    # Apply button clicked...
    def onApplyEvent(self, *args):
        print("onApplyEvent stub")

    # Cancel button clicked...
    def onCancelEvent(self, *args):
        print("Cancelling...")
        Gtk.main_quit()

    # Window about to close...
    def onDestroyEvent(self, *args):
        
        # For debugging purposes...
        print("Quitting...")
        
        # Check if the verification thread is still running...
        if self.verificationThread:
        
            # Signal the thread to quit...
            self.verificationThread.setQuit()
            
            # Wait for it to quit...
            self.verificationThread.join()
        
        # Terminate...
        self.quit()

    # Destroy the splash window after splash timer elapses...
    def splashTimerDone(self, userData):

        # Destroy the splash window...
        self.splashWindow.destroy()
        
        # Make the main window visible now...
        self.assistant.show_all()

        # Kill the timer...
        GObject.source_remove(self.splashTimeoutEvent)

        # Kill the timer...
        return False

    # Run the GUI...
    def run(self):

        # Start processing events...    
        try:
            Gtk.main()
        except KeyboardInterrupt:
            print("KeyboardInterrupt")
            raise
        except:
            print("SomeOtherException")
            Gtk.main_quit()

    # Show the splash window...
    def showSplash(self):

        # Find the window instance...
        self.splashWindow = self.builder.get_object("SplashWindowInstance")

        # Set timer for three seconds, or zero time if disabled...
        if not self.options.skipSplash:
            self.splashTimeoutEvent = GObject.timeout_add(3000, self.splashTimerDone, None)
        else:
            self.splashTimeoutEvent = GObject.timeout_add(0, self.splashTimerDone, None)

        # Set it to be always on top...
        self.splashWindow.set_keep_above(True)

        # Display the window...
        self.splashWindow.show_all()

    # Terminate...
    def quit(self):
        Gtk.main_quit()

# Entry point...
if __name__ == '__main__':

    # Initialize threading environment...
    GObject.threads_init()

    # Start the navigator GUI...
    navigator = NavigatorApp()
    navigator.run()


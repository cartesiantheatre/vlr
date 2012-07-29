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
        self.assistant.set_forward_page_func(self.forwardPage, None)

        # Welcome page...
        self.welcomePageBox = self.builder.get_object("welcomePageBox")
        self.welcomePageBox.set_border_width(5)
        self.assistant.append_page(self.welcomePageBox)
        self.assistant.set_page_title(self.welcomePageBox, "Introduction")
        self.assistant.set_page_type(self.welcomePageBox, Gtk.AssistantPageType.INTRO)
        self.assistant.set_page_complete(self.welcomePageBox, True)
        
        # Integrity info page...
        self.integrityInfoPageBox = self.builder.get_object("integrityInfoPageBox")
        self.integrityInfoPageBox.set_border_width(5)
        self.assistant.append_page(self.integrityInfoPageBox)
        self.assistant.set_page_title(self.integrityInfoPageBox, "Disc Verification")
        self.assistant.set_page_type(self.integrityInfoPageBox, Gtk.AssistantPageType.CONTENT)
        self.assistant.set_page_complete(self.integrityInfoPageBox, True)

        # Integrity progress page...
        self.integrityProgressPageBox = self.builder.get_object("integrityProgressPageBox")
        self.integrityProgressPageBox.set_border_width(5)
        self.assistant.append_page(self.integrityProgressPageBox)
        self.assistant.set_page_title(
            self.integrityProgressPageBox, "Disc Verification\nProgress")
        self.assistant.set_page_type(
            self.integrityProgressPageBox, Gtk.AssistantPageType.PROGRESS)
        self.assistant.set_page_complete(self.integrityProgressPageBox, False)

        # Fetch handbook page...
        self.handbookPageBox = self.builder.get_object("handbookPageBox")
        fetchHandbookButton = self.builder.get_object("fetchHandbookButton")
        fetchHandbookButton=fetchHandbookButton.get_children()[0]
        fetchHandbookButton.get_children()[0].get_children()[1].set_label("Download")
        self.handbookPageBox.set_border_width(5)
        self.assistant.append_page(self.handbookPageBox)
        self.assistant.set_page_title(self.handbookPageBox, "Fetch Handbook")
        self.assistant.set_page_type(self.handbookPageBox, Gtk.AssistantPageType.CONTENT)
        self.assistant.set_page_complete(self.handbookPageBox, True)

    # End of current page. Next page is being constructed but not visible yet...
    def onPrepareEvent(self, assistant, currentPage):

        # Reset the cursor to normal in case something changed it...
        self.assistant.get_root_window().set_cursor(None)

        # Transitioning into integrity progress page...
        if currentPage is self.integrityProgressPageBox:

            # Change to busy cursor...
            cursorWatch = Gdk.Cursor.new(Gdk.CursorType.WATCH)
            self.assistant.get_root_window().set_cursor(cursorWatch)

            # Start the disc verification...
            self.startDiscVerification()

    # End of current page. Calculate index of next page...
    def forwardPage(self, currentPageIndex, userData):

        # Get what would be the next page...
        nextPage = self.assistant.get_nth_page(currentPageIndex)

        # Transitioning into integrity progress page...
        if nextPage is self.integrityProgressPageBox and \
           self.builder.get_object("skipIntegrityCheckRadio").get_active():

                # Skip the disc integrity check...
                self.assistant.set_current_page(currentPageIndex + 1)
                return currentPageIndex + 2

        # Any other page just transition to the next one...
        else:
            return currentPageIndex + 1

    # Start the disc verification...
    def startDiscVerification(self):

        # Already running...
        if self.verificationThread:
            print("Disc verification thread already in progress...")
            return

        # Allocate and start the thread...
        self.verificationThread = VerificationThread(self.builder)
        self.verificationThread.start()

    # User requested to stop disc verification...
    def onStopIntegrityPressed(self, button):

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
        self.assistant.set_page_complete(self.integrityProgressPageBox, True)
        
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

    # Cancel button clicked...
    def onCancelEvent(self, *args):
        Gtk.main_quit()

    # Window about to close...
    def onDestroyEvent(self, *args):
        print("Quitting...")
        self.quit()

    def onForwardClicked(self, button):
        print("onForwardPressed")

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
            raise
        except:
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


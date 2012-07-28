#!/usr/bin/env python3

# Imports...
from gi.repository import Gtk, Gdk, GObject
from optparse import OptionParser
import threading
import hashlib

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
        self.assistant.set_page_complete(self.integrityProgressPageBox, True)

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

    # Integrity progress page ready...
    def onIntegrityProgressPageRealizeEvent(self, page):
        # TODO: Perform md5 check of data files...
        pass

    # User requested to stop disc verification...
    def onStopIntegrityPressed(self, button):
        pass

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
    navigator = NavigatorApp()
    navigator.run()


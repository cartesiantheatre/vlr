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

# Splash window...
from SplashWindow import SplashWindowProxy

# Assistant pages and related logic...
from IntroductionPages import IntroductionPagesProxy
from VerificationPages import VerificationPagesProxy
from HandbookPage import HandbookPageProxy
from SelectRecoveryPage import SelectRecoveryPageProxy
from ConfigurePages import ConfigurePagesProxy
from ConfirmPage import ConfirmPageProxy
from FarewellPage import FarewellPageProxy

from RecoveryWindow import RecoveryWindowProxy

# Launcher class...
class LauncherApp():

    # Constructor...
    def __init__(self):

        # Initialize Glade builder...
        self.builder = Gtk.Builder()
        self.builder.add_from_file("../Gooey/Launcher.glade")

        # Find the assistant...
        self.assistant = self.builder.get_object("assistantWindow")

        # Construct and show the splash window...
        #self.splashWindowProxy = SplashWindowProxy(self)
        #self.splashWindowProxy.showSplash()
        self.recoveryWindowProxy = RecoveryWindowProxy(self)

        # Construct pages and register them with the assistant in order...
        self.introductionPagesProxy = IntroductionPagesProxy(self)
        self.verificationPagesProxy = VerificationPagesProxy(self)
        self.handbookPageProxy = HandbookPageProxy(self)
        self.selectRecoveryPageProxy = SelectRecoveryPageProxy(self)
        self.configurePagesProxy = ConfigurePagesProxy(self)
        self.confirmPageProxy = ConfirmPageProxy(self)
        self.farewellPageProxy = FarewellPageProxy(self)

        # Set the forward function which determines next page to show...
        self.assistant.set_forward_page_func(self.forwardPage, None)

        # Connect the assistant's signals...
        self.assistant.connect("close", self.onCloseEvent)
        self.assistant.connect("apply", self.onApplyEvent)
        self.assistant.connect("delete-event", self.onDeleteEvent)
        self.assistant.connect("cancel", self.onCancelEvent)
        self.assistant.connect("prepare", self.onPrepareEvent)

    # End of current page. Next page is being constructed but not visible yet...
    def onPrepareEvent(self, assistant, currentPage):

        # Reset the cursor to normal in case something changed it...
        self.setBusy(False)

        # Transitioning into verification progress page...
        if currentPage is self.verificationPagesProxy.getProgressPageBox():

            # Give a chance to prepare...
            self.verificationPagesProxy.onPrepare()

        # Transitioning into confirm page...
        elif currentPage is self.confirmPageProxy.getPageBox():
            
            # Prepare the VikingExtractor arguments based on user selected
            #  configuration and show the summary...
            self.confirmPageProxy.onPrepare()

    # End of current page. Calculate index of next page...
    def forwardPage(self, currentPageIndex, userData):

        # Get what would be the next page...
        nextPage = self.assistant.get_nth_page(currentPageIndex)

        # Transitioning into verification progress page...
        if nextPage is self.verificationPagesProxy.getProgressPageBox() and \
           self.builder.get_object("skipVerificationCheckRadio").get_active():

                # Skip the disc verification check...
                self.assistant.set_current_page(currentPageIndex + 1)
                return currentPageIndex + 2

        # Any other page just transition to the next one...
        else:
            return currentPageIndex + 1

    # Apply button clicked...
    def onApplyEvent(self, *args):
        
        # For debugging purposes...
        print("onApplyEvent")

    # Cancel signal emitted when cancel button clicked or assistant being 
    #  closed...
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

        # Check if the verification thread is still running, and if so, signal
        #  it to quit and block until it does...
        self.verificationPagesProxy.waitThreadQuit()

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
        
        # No threads should be running by this point because at the end of the 
        #  assistant's page flow, so safe to terminate...
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

    def setBusy(self, Busy = True):

        # Toggle busy state...
        if Busy:

            # Change to busy cursor...
            cursorWatch = Gdk.Cursor.new(Gdk.CursorType.WATCH)
            self.assistant.get_root_window().set_cursor(cursorWatch)

        # Untoggle busy state...
        else:

            # Reset the cursor to normal in case something changed it...
            cursorArrow = Gdk.Cursor.new(Gdk.CursorType.ARROW)
            self.assistant.get_root_window().set_cursor(cursorArrow)

    # Terminate...
    def quit(self):
        Gtk.main_quit()


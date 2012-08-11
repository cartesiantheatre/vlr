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
import re

# Our support modules...
import Options

# Splash window...
from Splash import NavigatorSplash

# Assistant pages and related logic...
from Introduction import IntroductionPage
from Verification import VerificationPages
from Handbook import HandbookPage

# Navigator class...
class NavigatorApp():

    # Constructor...
    def __init__(self):

        # Initialize Glade builder...
        self.builder = Gtk.Builder()
        self.builder.add_from_file("../Gooey/Launcher.glade")

        # Find the assistant...
        self.assistant = self.builder.get_object("Assistant")

        # Construct and show the splash window...
        self.navigatorSplash = NavigatorSplash(self)
        self.navigatorSplash.showSplash()

        # Construct pages and register them with the assistant...
        self.introductionPage = IntroductionPage(self)
        self.verificationPages = VerificationPages(self)
        self.handbookPage = HandbookPage(self)

        # Select save folder page...
        # TODO: Move the rest of this page logic into appropriate composite classes...
        self.selectSaveFolderPageBox = self.builder.get_object("selectSaveFolderPageBox")
        self.selectSaveFolderPageBox.set_border_width(5)
        self.assistant.append_page(self.selectSaveFolderPageBox)
        self.assistant.set_page_title(self.selectSaveFolderPageBox, "Select Save Folder")
        self.assistant.set_page_type(self.selectSaveFolderPageBox, Gtk.AssistantPageType.CONTENT)
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

        # Set the forward function which determines next page to show...
        self.assistant.set_forward_page_func(self.forwardPage, None)

        self.builder.connect_signals(self)

    # End of current page. Next page is being constructed but not visible yet...
    def onPrepareEvent(self, assistant, currentPage):

        # Reset the cursor to normal in case something changed it...
        self.assistant.get_root_window().set_cursor(None)

        # Transitioning into verification progress page...
        if currentPage is self.verificationPages.getProgressPageBox():

            # Give a chance to prepare...
            self.verificationPages.onPrepare()

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
        if nextPage is self.verificationPages.getProgressPageBox() and \
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
        self.verificationPages.waitThreadQuit()

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

# Entry point...
if __name__ == '__main__':

    # Initialize threading environment...
    GObject.threads_init()
    
    # Start the navigator GUI...
    navigator = NavigatorApp()
    navigator.run()


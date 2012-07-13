#!/usr/bin/env python3

from gi.repository import Gtk, GObject

# Signal window...
class SplashWindow:

    # Constructor...
    def __init__(self, builder, signalHandlerDictionary):

        # Modify the signal handler dictionary to notify us...
        signalHandlerDictionary['SplashWindow.onSkipSplash'] = self.onSkipSplash

        # Find the window instance...
        self.builder = builder        
        self.window = builder.get_object("SplashWindowInstance")

        splashTimeoutID = GObject.timeout_add(3000, self.killSplash, None)
        
        # Display the window...
        self.window.show_all()

    # Keyboard or mouse click, kill the splash...
    def onSkipSplash(self, *arguments):
        Gtk.main_quit()

    # Destroy the splash window after splash timer elapses...
    def killSplash(self, userData):

        # Destroy the splash window...
        self.window.destroy()
        
        # Kill the timer...
        #return False
        Gtk.main_quit()

# Entry point if run directly...
if __name__ == "__main__":

    # Initialize Glade builder...
    builder = Gtk.Builder()
    builder.add_from_file("Navigator/Navigator.glade")

    # Dictionary to use for signal handlers...
    signalHandlerDictionary = { }

    # Create the splash window...
    splashWindow = SplashWindow(builder, signalHandlerDictionary)

    # Use signal handlers from now on...
    builder.connect_signals(signalHandlerDictionary)

    # Start processing events...
    Gtk.main()


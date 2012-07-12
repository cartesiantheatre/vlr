#!/usr/bin/env python3

from gi.repository import Gtk, GObject

# Signal handler...
class SignalHandler:
    
    # Splash deleted...
    def onDeleteWindow(self, window, *args):
        print("onDeleteWindow")
        Gtk.main_quit()
    
    # Event box clicked...
    def onSplashPressed(self, eventbox, *args):
        print("onButtonPressed")
        Gtk.main_quit()

# Destroy the splash window after splash timer elapses...
def killSplash(userData):

    # Destroy the splash window...
    window = builder.get_object("SplashWindow")
    window.destroy()
    
    # Kill the timer...
    #return False
    Gtk.main_quit()

# Entry point if run directly...
if __name__ == "__main__":

    # Initialize Glade builder...
    builder = Gtk.Builder()
    builder.add_from_file("Navigator/Navigator.glade")

    # Setup signal handlers...
    builder.connect_signals(SignalHandler())

    # Show the splash screen...
    window = builder.get_object("SplashWindow")
    window.show_all()
    splashTimeoutID = GObject.timeout_add(3000, killSplash, None)
    
    # Start processing events...
    Gtk.main()


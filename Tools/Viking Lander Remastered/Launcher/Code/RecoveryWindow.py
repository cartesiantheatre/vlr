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
from gi.repository import Gtk, Gdk, GObject, GLib, Vte
import os

# Our support modules...

# Recovery window proxy class...
class RecoveryWindowProxy():

    # Constructor...
    def __init__(self, launcherApp):
        
        # Initialize...
        self._launcherApp = launcherApp
        self._assistant = launcherApp.assistant
        self._builder = launcherApp.builder

        # Find the window and its widgets...
        self._recoveryWindow = self._builder.get_object("recoveryWindow")
        self._recoveryExpander = self._builder.get_object("recoveryExpander")
        self._recoveryScrolledWindow = self._builder.get_object("recoveryScrolledWindow")
        self._recoveryProgressBar = self._builder.get_object("recoveryProgressBar")
        self._cancelRecoveryButton = self._builder.get_object("cancelRecoveryButton")
        
        # Create the terminal widget...
        self._terminal = Vte.Terminal()
        self._terminal.set_cursor_blink_mode(Vte.TerminalCursorBlinkMode.ON)
        #self._terminal.set_background_transparent(True)
        #self._terminal.set_opacity(0.2)
        self._terminal.set_cursor_shape(Vte.TerminalCursorShape.BLOCK)
        self._terminal.set_emulation("xterm")
        self._terminal.set_scroll_on_output(True)
        self._terminal.set_scroll_on_keystroke(True)
        self._terminal.set_color_foreground(Gdk.Color(red=1.0, green=0.0, blue=0.0))
        self._terminal.set_scrollback_lines(5000)
        self._terminal.set_encoding("UTF-8")

        # Add the terminal widget to the expander's scrolled window widget...
        self._recoveryScrolledWindow.add(self._terminal)

        # This is just for debugging purposes...
        self._terminal.fork_command_full(
            Vte.PtyFlags.DEFAULT,
            os.environ['HOME'],
            ["/usr/bin/find", "/etc"],
            [],
            GLib.SpawnFlags.DO_NOT_REAP_CHILD,
            None,
            None,
            )

        # Set the window width to 1000 pixels, unless user's resolution too low
        #  in which case just use the maximum available...
        screen = self._recoveryWindow.get_screen()
        screenWidth = screen.get_width()
        if screenWidth >= 1000:
            self._recoveryWindow.resize(1000, 200)
        else:
            self._recoveryWindow.resize(screenWidth, 200)
        
        # Connect the signals...
        self._recoveryWindow.connect("delete-event", Gtk.main_quit)
        self._cancelRecoveryButton.connect("clicked", self.onCancelClicked)
        self._recoveryExpander.connect("notify::expanded", self.onExpanded)
        self._terminal.connect("child-exited", self.onChildProcessExit)

        # Display the window...
        self._recoveryWindow.show_all()

    # Cancel recovery button clicked...
    def onCancelClicked(self, button, *junk):

        # Stubbed...
        pass

    # VikingExtractor terminated...
    def onChildProcessExit(self, *junk):

        # Stubbed...
        pass

    # Expander toggled...
    def onExpanded(self, expander, *junk):

        # Expander is collapsing, but hasn't physically yet...
        if expander.get_expanded() is False:

            # Get the current width, which we will maintain, and remember the full
            #  expanded height as well so we can restore later if expanding again...
            (currentWidth, self._expandedHeight) = self._recoveryWindow.get_size()
            
            # Keep the whole window's width, but shrink height to minimal...
            self._recoveryWindow.resize(currentWidth, 1)

        # Expander is expanding, but hasn't physically yet...
        else:

            # Keep whole window's width, but expand whole window to previous height...
            (currentWidth, collapsedHeight) = self._recoveryWindow.get_size()
            self._recoveryWindow.resize(currentWidth, self._expandedHeight)


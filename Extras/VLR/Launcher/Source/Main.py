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
import gi
import signal
from gi.repository import GObject, Gdk

# i18n...
import gettext
import locale
_ = gettext.gettext

# Arguments...
import LauncherArguments

# Miscellaneous routines...
from Miscellaneous import *

# Launcher...
from LauncherApp import LauncherApp

# Entry point...
if __name__ == '__main__':

    # Kill on keyboard interrupt...
    signal.signal(signal.SIGINT, signal.SIG_DFL)

    # Initialize threading environment...
    GObject.threads_init()
    Gdk.threads_init()

    # Initialize i18n...
    #  Note that we have to do this again for Gtk.Builder:
    #  <https://bugzilla.gnome.org/show_bug.cgi?id=574520>
    localeDirectory = os.path.join(
        LauncherArguments.getArguments().dataRoot, "Translations/")
    translation = gettext.translation(
        "vlr", 
        os.path.join(LauncherArguments.getArguments().dataRoot, "Translations/"),
        fallback=True)
    translation.install()
    locale.bindtextdomain("vlr", localeDirectory)
    locale.textdomain("vlr")

    # Start the launcher GUI...
    launcher = LauncherApp()
    launcher.run()


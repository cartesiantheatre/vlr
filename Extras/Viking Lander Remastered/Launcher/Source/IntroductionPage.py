#!/usr/bin/env python3
# VikingExtractor, to recover images from Viking Lander operations.
# Copyright (C) 2010-2013 Cartesian Theatre <info@cartesiantheatre.com>.
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
from gi.repository import Gtk
import os

# i18n...
import gettext
_ = gettext.gettext

# Arguments...
import LauncherArguments

# Assistant proxy page base class...
from PageProxyBase import *

# Introduction page proxy class...
class IntroductionPageProxy(PageProxyBase):

    # Constructor...
    def __init__(self, launcherApp):

        # Initialize...
        super(IntroductionPageProxy, self).__init__(launcherApp)

        # Add the introduction page to the assistant...
        self.registerPage(
            "introductionPageBox",
            _("Introduction"),
            Gtk.AssistantPageType.INTRO,
            True)

        # Find widgets...
        self._introductionLabel = self._builder.get_object("introductionLabel")

        # Load the introduction text from disk...
        introductionFile = open(os.path.join(
            LauncherArguments.getArguments().dataRoot, "Introduction.txt"))
        self._introductionLabel.set_markup(introductionFile.read())


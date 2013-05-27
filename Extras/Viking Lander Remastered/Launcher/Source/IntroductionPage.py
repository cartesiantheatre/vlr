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

        # Break up the introduction text by paragraphs to make it easier for 
        #  translators...
        introductionText = \
            _("Dear User,\n\n") + \
            _("The <i>Avaneya: Viking Lander Remastered</i> initiative has an interesting story behind it, one which began with the <a href=\"https://www.avaneya.com\">Avaneya</a> project. Avaneya is a grass roots collaborative undertaking to produce a libre, DRM free, cerebral, science fiction game primarily targeting, albeit not exclusively limited to, users of the <a href=\"https://www.gnu.org/gnu/gnu-users-never-heard-of-gnu.html\">GNU operating system</a>. This operating system has been made popular through its various distributions, such as <a href=\"http://www.ubuntu.com\">Ubuntu</a>. Although the game is still in its infancy at the time of writing, it would be naïve to conclude that it has not endured several years worth of critical research and design already, of which, a significant consequence you have before you now.\n\n") + \
            \
            _("Avaneya will take place in the not too distant future on Mars, predominantly in a real region known now as <i><a href=\"https://en.wikipedia.org/wiki/Arcadia_planitia\">Arcadia Planitia</a></i>. This is approximately mid-latitude in the Red Planet's northern hemisphere.\n\n") + \
            \
            _("While designing Avaneya, we realized just how important getting the game experience right was to us. We needed to replicate the visual environmental conditions of the Martian environment as carefully as possible. We could do much better than simply making it red, and users appreciate an earnest effort at a sincere visual experience. As such, our artists required access to large sets of high resolution reference material for study.\n\n") + \
            \
            _("The closest relevant photographs available at present were taken in a neighbouring region near <i>Arcadia Planitia</i> called <a href=\"https://en.wikipedia.org/wiki/Utopia_Planitia\"><i>Utopia Planitia</i></a> during the late 1970s by NASA's <a href=\"https://en.wikipedia.org/wiki/Viking_program\">Viking mission</a> to Mars. This region is very similar to <i>Arcadia Planitia</i> in appearance.\n\n") + \
            \
            _("In 1976 NASA successfully placed two Viking spacecraft into Martian orbit after many years of <a href=\"http://www.marsnews.com/missions/allmissions.html\">failed</a> American and Russian attempts. These initial failures became unofficially known as the <i>\"Mars Curse\"</i> owing to their high number. The Viking orbiters both carried their own landers which successfully landed, one of which in the aforementioned <i>Utopia Planitia</i> region.\n\n") + \
            \
            _("The landers were capable of capturing high resolution photographs and other data from the surface of the planet, buffering data temporarily within internal magnetic tapes and then relaying it back to Earth through one of the orbiters when in position or sometimes directly over a UHF uplink whenever possible. They provided us with large sets of data without any major issue for four years before finally succumbing to a battery failure on one and a software update bug which shutdown the communication antenna on the other.\n\n") + \
            \
            _("An <a href=\"http://pds-imaging.jpl.nasa.gov/volumes/viking.html\">online archive</a> of all of the mission data was released into the public domain. However, due to the way this data was encoded and archived with the technology of the 1970s, it is unfortunately virtually impossible for the layman to find any recognizably photographic data in the archive. We contacted NASA's <a href=\"http://pds.nasa.gov/about/about.shtml\">Planetary Data System</a> responsible for the archival of planetary missions. They informed us that the data was produced <i>\"during the mission for use in operations, probably in a rather hectic working environment, and were not intended for archiving. In those pre-PDS days nobody gave much thought to archiving,\"</i> (1 June 2011).\n\n") + \
            \
            _("To find a complete colour photograph is a non-trivial exercise. At the time of writing, NASA still had <a href=\"http://nssdc.gsfc.nasa.gov/planetary/planetaryfaq.html#Viking-lander\">not released</a> the complete set of high resolution user accessible images on disc. During our communication, NASA noted that they did <i>\"not know what format they would be in or whether there is any existing software to read them. There is not a complete index of individual color images, so it would require going through the [original archive] to extract the color data from all the rest of the material that has been saved. This is beyond the purview of [Planetary Data System]. Are you interested in doing this?\"</i> (24 May 2011).\n\n") + \
            \
            _("We did not know what to do at this point, so we asked NASA if they would reconsider tackling this problem. They were better qualified and it would have been beneficial to them during a time of fiscal austerity which presented new challenges to public space exploration programs. We reasoned that renewed public interest in the Red Planet was a good thing, and this archive in particular would be especially useful, given that it would enable anyone to relive the original breathtaking experience that captured the world's attention. It marked the first successful moment in history that man saw Mars - not as a distant and impersonal celestial body as it had been known for centuries prior through telescopes, but as if he had actually been standing there with his own eyes. They were, after all, the first successful photographs ever taken from the surface of Mars. Nevertheless, NASA indicated that the recovery effort was unfortunately not logistically feasible at the time. <i>\"We have to prioritize, and we just don't have the resources now to restore these images,\"</i> (25 May 2011).\n\n") + \
            \
            _("We contemplated undertaking the significant challenge ourselves, but were very much intimidated. <i>\"The images were created using old hardware and software and therefore will require some digital archaeology to read now. The software was VICAR, which is <a href=\"http://www-mipl.jpl.nasa.gov/external/vicar.html\">still used</a> at the JPL Multimission Instrument Processing Lab, although I doubt the current version can read these images,\"</i> (25 May 2011). They followed up several days later noting that <i>\"modern VICAR has evolved so much that it will most likely be unable to read these old files, and I think you have demonstrated that,\"</i> (1 June 2012).\n\n") + \
            \
            _("Needing the archive for our own creative purposes, yet willing to share our solution with everyone in response to a large degree of public interest, we set out to author a digital forensic archaeological tool to recover whatever we could of the landers' photographic mission data in a simple and easy to use manner. This software became known as the <i>Avaneya: Viking Lander Remastered</i>.\n\n") + \
            \
            _("True, we could have simply provided the recovered images alone, but we knew that the <i>libre</i> community, hobbyists in particular, would have found it far more intriguing to have taken part directly in their own digital forensic recovery effort. In addition, given that there has also been a public outcry for <i>libre</i> commercial gaming titles for the GNU operating system for more than a decade, we realized that our fans would understand that their support in purchasing a copy of this software on DVD from us would be critical in sustaining a project as ever wanting of high quality engineers and artists as it is in striving to minimize any reliance on charity.\n\n") + \
            \
            _("Nearly anything else you may wish to know about the Avaneya project you can find in our project bible, the <i>Avaneya Project Crew Handbook</i>. For example, there is a chapter titled <i>Viking Lander Remastered</i> which elaborates further on the technical details of this software for those who are curious. You will be given an opportunity momentarily to download a complementary copy of the latest revision of this book to enjoy at your leisure.\n\n") + \
            \
            _("On behalf of all of us, we would like to extend our gratitude for your having taken the time to support this project. Avaneya has an exciting future ahead of it. We hope that you will choose to be a part of it.\n\n") + \
            \
            _("\t<b><i>~ Avaneya Project Crew</i></b>\n")

        # Load the introduction text from disk...
        #introductionFile = open(os.path.join(
        #    LauncherArguments.getArguments().dataRoot, "Introduction.txt"))
        #self._introductionLabel.set_markup(introductionFile.read())
        self._introductionLabel.set_markup(introductionText)


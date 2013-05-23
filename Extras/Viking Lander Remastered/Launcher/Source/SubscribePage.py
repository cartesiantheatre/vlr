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
import urllib.request
import urllib.parse
import urllib.error
import errno
from html.parser import HTMLParser

# Helper functions...
from Miscellaneous import *

# Assistant proxy page base class...
from PageProxyBase import *

# i18n...
import gettext
_ = gettext.gettext

# HTML parser for GNU Mailman response...
class MailmanParser(HTMLParser):

    # Constructor...
    def __init__(self):

        # Initialize the base class...
        super(MailmanParser, self).__init__(HTMLParser)

        # Initialize...
        self._messageNext = False
        self._messageBody = None

    # Get the human readable message body...
    def getMessageBody(self):
        assert(len(self._messageBody))
        return self._messageBody

    # Start of tag... e.g. <div id="main">
    def handle_starttag(self, tag, attributes):
        pass

    # End of tag...
    def handle_endtag(self, tag):

        # Message seems to follow closing of </h1> tag in the body...
        if tag == "h1":
            self._messageNext = True

    # Data contained within a tag...
    def handle_data(self, data):
    
        # Expecting message...
        if self._messageNext:
            
            # Store it and strip leading and trailing white space and collapse
            #  new lines into one line...
            self._messageBody = data.strip().replace("\n", " ")
            
            # Done parsing...
            self._messageNext = False

    # True if the message body indicates the subscription request was 
    #  successful...
    def isSubscribeOk(self):
        
        # Parsing should have already occured...
        assert(len(self._messageBody))
        
        # Looks like it is ok...
        # TODO: What if response not in English?
        if self._messageBody.find("Your subscription request has been received") != -1:
            return True

        # Doesn't look like it...
        else:
            return False

# Subscribe page proxy class...
class SubscribePageProxy(PageProxyBase):

    # Constructor...
    def __init__(self, launcherApp):

        # Initialize...
        super(SubscribePageProxy, self).__init__(launcherApp)

        # Register the page with the assistant...
        self.registerPage(
            "subscribePageBox", 
            _("Subscribe to Newsletter"), 
            Gtk.AssistantPageType.CONFIRM, 
            True)

        # Shortcuts to our widgets...
        self._yesToggle = self._builder.get_object("yesSubscribeRadioButton")
        self._noToggle = self._builder.get_object("noSubscribeRadioButton")
        self._nameTextEntry = self._builder.get_object("subscribeNameTextEntry")
        self._emailTextEntry = self._builder.get_object("subscribeEmailTextEntry")
        self._passwordTextEntry = self._builder.get_object("subscribePasswordTextEntry")
        self._passwordAgainTextEntry = self._builder.get_object("subscribePasswordAgainTextEntry")

        # Connect the signals...
        self._yesToggle.connect("toggled", self.onSubscribeToggle)
        self._noToggle.connect("toggled", self.onSubscribeToggle)
        
    # Apply button was hit...
    def onApply(self):

        # If the subscription request was not successful, stay on current page...
        if not self.performSubscribe():
        
            # TODO: This makes the previous button transition to ghost pages
            #  however many times this code path is executed...
            subscribePageIndex = self.getAbsoluteIndex(0)
            self._assistant.set_current_page(subscribePageIndex - 1)
            #self._assistant.stop_emission("apply")

    # Our page in the assistent is being constructed, but not visible yet...
    def onPrepare(self):
        pass

    # Yes or no toggled for subscription...
    def onSubscribeToggle(self, button, userData=None):
        
        # Check whether subscription requested...
        subscribe = self._yesToggle.get_active()

        # Enable or disable child controls appropriately...
        self._subscribeGrid = self._builder.get_object("subscribeGrid")
        self._subscribeGrid.set_sensitive(subscribe)
        
        # Change page type, depending on whether we have a job to do or not...
        if subscribe:
            self._assistant.set_page_type(self.getPageInGroup(0), Gtk.AssistantPageType.CONFIRM)
        else:
            self._assistant.set_page_type(self.getPageInGroup(0), Gtk.AssistantPageType.CONTENT)

    # Subscribe button pressed...
    def performSubscribe(self):

        # Check network connectivity to the internet and alert user if no
        #  connection available...
        if not hasInternetConnection(True, self._assistant, True):

            # Don't do anything further...
            return False

        # Try to submit the request to the remote server...
        try:

            # Retrieve fields from GUI...
            fullName = self._nameTextEntry.get_text()
            email = self._emailTextEntry.get_text()
            password = self._passwordTextEntry.get_text()
            passwordAgain = self._passwordAgainTextEntry.get_text()

            # Missing one or more fields...
            if not len(fullName) or not len(email) or not len(password) \
               or not len(passwordAgain):
                raise IOError(errno.ENOMSG, _("You need to fill out the whole form."))

            # Email address not valid...
            if email.find("@") is -1:
                raise IOError(errno.ENOMSG, _("Invalid email address."))

            # Verify passwords match...
            if password != passwordAgain:
                raise IOError(errno.ENOMSG, _("Passwords do not match."))

            # Encode POST request parameters for GNU Mailman...
            postParameters = urllib.parse.urlencode({
                "email": email, 
                "fullname": fullName,
                "pw": password,
                "pw-conf": passwordAgain,
                "digest": 0})
            postParameters = postParameters.encode("utf-8")

            # Submit the POST request...
            urlStream = urllib.request.urlopen(
                "http://lists.avaneya.com/subscribe.cgi/announce-avaneya.com", 
                postParameters, timeout=10)

            # Something bad happened...
            if urlStream.getcode() is not 200:

                # Raise the error...
                raise IOError(
                    errno.ENOMSG, 
                    _("There was a problem processing your subscription request "
                    "on the remote server. Maybe try again later. ({0})").
                        format(urlStream.getcode()))

            # Get the mailman HTML response...
            mailmanMessage = urlStream.read(4096)

            # Decode the byte stream response into a string...
            mailmanMessage = mailmanMessage.decode("utf-8")
            
            # Create and initialize parser...
            parser = MailmanParser()
            parser.feed(mailmanMessage)
            
            # Determine whether this was an error or not so we know how to
            #  decorate the dialog box...
            dialogMessageType = None
            if parser.isSubscribeOk():
                dialogMessageType = Gtk.MessageType.INFO
            else:
                dialogMessageType = Gtk.MessageType.ERROR
            
            # Show server response in dialog box...
            messageDialog = Gtk.MessageDialog(
                self._assistant, Gtk.DialogFlags.MODAL, dialogMessageType, 
                Gtk.ButtonsType.OK, 
                parser.getMessageBody())
            messageDialog.run()
            messageDialog.destroy()
            
            # Done...
            return parser.isSubscribeOk()

        # Communication error...
        except urllib.error.URLError as exception:

            # Alert user...
            messageDialog = Gtk.MessageDialog(
                self._assistant, Gtk.DialogFlags.MODAL, Gtk.MessageType.ERROR, 
                Gtk.ButtonsType.OK, 
                _("There was a problem communicating with the remote server. "
                "Please try again later.\n\n\t{0}").
                    format(exception.reason))
            messageDialog.run()
            messageDialog.destroy()
            
            # Done...
            return False

        # Data entry or urllib error...
        except (OSError, IOError) as exception:

            # Alert user...
            messageDialog = Gtk.MessageDialog(
                self._assistant, Gtk.DialogFlags.MODAL, Gtk.MessageType.ERROR, 
                Gtk.ButtonsType.OK, 
                exception.strerror)
            messageDialog.run()
            messageDialog.destroy()
            
            # Done...
            return False


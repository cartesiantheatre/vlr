#!/usr/bin/env python3
# VikingExtractor, to recover images from Viking Lander operations.
# Copyright (C) 2010-2018 Cartesian Theatre™ <info@cartesiantheatre.com>.
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
import urllib.error
import json
import errno

# Helper functions...
from Miscellaneous import *

# Assistant proxy page base class...
from PageProxyBase import *

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

            # Prepare JSON request...
            jsonDictionary = {
                "email": email, 
                "fullname": fullName,
                "pw": password,
                "pw-conf": passwordAgain,
                "digest": 0,
                "user-agent": getLongVersionString()
            }
            
            # Convert to JSON data...
            jsonData = json.dumps(jsonDictionary)
            
            # Encode Python string as UTF-8 byte stream for POST request...
            postData = jsonData.encode("utf-8")
            
            # We should specify in the headers that the content type is JSON...
            headers = {}
            headers["Content-Type"] = "application/json; charset=utf-8"
            
            # Construct the request...
            request = urllib.request.Request(
                "https://www.avaneya.com/subscribe.php", 
                postData, 
                headers)

            # Submit the actual POST request...
            urlStream = urllib.request.urlopen(request, timeout=60)

            # This should always be a code 200, and if not, an exception should
            #  have been thrown...
            assert(urlStream.getcode() == 200)

            # Get the server response...
            serverMessage = urlStream.read(4096)

            # Decode the byte stream response into a string...
            serverMessage = serverMessage.decode("utf-8")

            # Show server response in dialog box...
            messageDialog = Gtk.MessageDialog(
                self._assistant, Gtk.DialogFlags.MODAL, Gtk.MessageType.INFO, 
                Gtk.ButtonsType.OK, 
                serverMessage)
            messageDialog.run()
            messageDialog.destroy()
            
            # Done...
            return True

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

            # Some debugging help...
            print("Exception of type \"{0}\"".format(type(exception)))
            print("Exception string \"{0}\"".format(exception.strerror))

            # Alert user...
            messageDialog = Gtk.MessageDialog(
                self._assistant, Gtk.DialogFlags.MODAL, Gtk.MessageType.ERROR, 
                Gtk.ButtonsType.OK, 
                exception.strerror)
            messageDialog.run()
            messageDialog.destroy()
            
            # Done...
            return False


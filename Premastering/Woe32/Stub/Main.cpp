/*
    VikingExtractor, to recover images from Viking Lander operations.
    Copyright (C) 2010-2017 Cartesian Theatre™ <info@cartesiantheatre.com>.
    
    Public discussion on IRC available at #avaneya (irc.freenode.net) 
    or on the mailing list <avaneya@lists.avaneya.com>.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// Includes...
#include <windows.h>

// Entry point...
int APIENTRY WinMain(
    HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // Prompt the user. Petzold will be pissed...
    const int Status = MessageBox(
        NULL, 
        "So there's good news and bad news. The bad news is that the\n"
        "Avaneya: Viking Lander Remastered software is not supported on\n"
        "legacy operating systems like Windows and OS X.\n\n"

        "But don't despair! The good news, however, is that upgrading to\n"
        "one that does is simple. Ubuntu is an alternative to proprietary\n"
        "operating systems like Windows and OS X. Built upon the freedom\n"
        "respecting GNU operating system, it is virus and malware free,\n"
        "safe, secure, costs nothing to download and use, is professionally\n"
        "supported, and already adopted worldwide by millions.\n\n"
        
        "No need to worry about switching cold turkey either. If Ubuntu is\n"
        "installed, you can choose when your computer starts to either boot\n"
        "into Ubuntu or Windows. But best of all, you won't need to worry\n"
        "about getting swindled anymore by Redmond's wretched\n"
        "monopolists or Cupertino's cryolite fruit.\n\n"

        "If you would like to be taken to the Ubuntu homepage to learn\n"
        "more about this contraband knowledge, press Ok now.",
        "Warning: Legacy Operating System", 
        MB_OKCANCEL | MB_ICONWARNING);

    // If they selected ok, open a browser to the Ubuntu home page...
    if(Status == IDOK)
        ShellExecute(NULL, "open", "http://www.ubuntu.com/desktop", NULL, NULL, SW_SHOWNORMAL);

    // Done...
    return 0;
}

